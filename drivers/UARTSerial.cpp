/* mbed Microcontroller Library
 * Copyright (c) 2006-2017 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if (DEVICE_SERIAL && DEVICE_INTERRUPTIN)

#include <errno.h>
#include "UARTSerial.h"
#include "platform/mbed_poll.h"

#if MBED_CONF_RTOS_PRESENT
#include "rtos/Thread.h"
#else
#include "platform/mbed_wait_api.h"
#endif

namespace mbed {

class UARTSerialWrite: public AsyncOp {
public:
    UARTSerialWrite(uint8_t *buf, uint32_t size): AsyncOp(NULL), tx_buf(buf), tx_size(size), result(0)
    {

    }
    uint8_t *tx_buf;
    uint32_t tx_size;
    ssize_t result;
};

class UARTSerialRead: public AsyncOp {
public:
    UARTSerialRead(uint8_t *buf, uint32_t size)
        :   AsyncOp(NULL), rx_buf(buf), rx_size(size), result(0)
    {

    }
    uint8_t *rx_buf;
    uint32_t rx_size;
    ssize_t result;
};

UARTSerial::UARTSerial(PinName tx, PinName rx, int baud) :
        SerialBase(tx, rx, baud),
        _tx_cur(NULL),
        _rx_cur(NULL),
        _blocking(true),
        _dcd_irq(NULL)
{
    /* Attatch IRQ routines to the serial device. */
    SerialBase::attach(callback(this, &UARTSerial::rx_irq), RxIrq);
}

UARTSerial::~UARTSerial()
{
    delete _dcd_irq;
}

void UARTSerial::dcd_irq()
{
    wake();
}

void UARTSerial::set_baud(int baud)
{
    SerialBase::baud(baud);
}

void UARTSerial::set_data_carrier_detect(PinName dcd_pin, bool active_high)
{
     delete _dcd_irq;
    _dcd_irq = NULL;

    if (dcd_pin != NC) {
        _dcd_irq = new InterruptIn(dcd_pin);
        if (active_high) {
            _dcd_irq->fall(callback(this, &UARTSerial::dcd_irq));
        } else {
            _dcd_irq->rise(callback(this, &UARTSerial::dcd_irq));
        }
    }
}

void UARTSerial::set_format(int bits, Parity parity, int stop_bits)
{
    lock();
    SerialBase::format(bits, parity, stop_bits);
    unlock();
}

#if DEVICE_SERIAL_FC
void UARTSerial::set_flow_control(Flow type, PinName flow1, PinName flow2)
{
    lock();
    SerialBase::set_flow_control(type, flow1, flow2);
    unlock();
}
#endif

int UARTSerial::close()
{
    /* Does not let us pass a file descriptor. So how to close ?
     * Also, does it make sense to close a device type file descriptor*/
    return 0;
}

int UARTSerial::isatty()
{
    return 1;

}

off_t UARTSerial::seek(off_t offset, int whence)
{
    /*XXX lseek can be done theoratically, but is it sane to mark positions on a dynamically growing/shrinking
     * buffer system (from an interrupt context) */
    return -ESPIPE;
}

int UARTSerial::sync()
{
    lock();

    while (!_txbuf.empty()) {
        unlock();
        // Doing better than wait would require TxIRQ to also do wake() when becoming empty. Worth it?
        wait_ms(1);
        lock();
    }

    unlock();

    return 0;
}

void UARTSerial::sigio(Callback<void()> func) {
    core_util_critical_section_enter();
    _sigio_cb = func;
    if (_sigio_cb) {
        short current_events = poll(0x7FFF);
        if (current_events) {
            _sigio_cb();
        }
    }
    core_util_critical_section_exit();
}

ssize_t UARTSerial::write(const void* buffer, size_t length)
{
    if (length == 0) {
        return 0;
    }

    lock();

    UARTSerialWrite write_op((uint8_t*)buffer, length);
    _tx_list.enqueue(&write_op);
    write_prep_buf();
    SerialBase::attach(callback(this, &UARTSerial::tx_irq), TxIrq);

    unlock();

    write_op.wait();
    return write_op.result;
}

void UARTSerial::write_prep_buf()
{
    if (_tx_cur == NULL) {
        _tx_cur = static_cast<UARTSerialWrite*>(_tx_list.dequeue());
    }
    if (_tx_cur == NULL) {
        return;
    }

    while ((_tx_cur->tx_size > 0) && !_txbuf.full()) {
        _txbuf.push(_tx_cur->tx_buf[0]);
        _tx_cur->tx_size -= 1;
        _tx_cur->tx_buf += 1;
        _tx_cur->result += 1;
    }

    if (_tx_cur->tx_size == 0) {
        _tx_cur->complete();
        _tx_cur = NULL;
    } else if (!_blocking) {
        write_abort_all();
    }
}

void UARTSerial::write_abort_all()
{
    if (_tx_cur == NULL) {
        _tx_cur = static_cast<UARTSerialWrite*>(_tx_list.dequeue());
    }
    while (_tx_cur != NULL) {
        if (_tx_cur->result == 0) {
            _tx_cur->result = (ssize_t)-EAGAIN;
        }
        _tx_cur->complete();
        _tx_cur = static_cast<UARTSerialWrite*>(_tx_list.dequeue());
    }
}

ssize_t UARTSerial::read(void* buffer, size_t length)
{
    if (length == 0) {
        return 0;
    }

    lock();

    UARTSerialRead read_op((uint8_t*)buffer, length);
    _rx_list.enqueue(&read_op);
    read_prep_buf();
    SerialBase::attach(callback(this, &UARTSerial::rx_irq), RxIrq);

    unlock();

    read_op.wait();
    return read_op.result;
}
void UARTSerial::read_prep_buf()
{
    if (_rx_cur == NULL) {
        _rx_cur = static_cast<UARTSerialRead*>(_rx_list.dequeue());
    }
    if (_rx_cur == NULL) {
        return;
    }

    while ((_rx_cur->rx_size > 0) && !_rxbuf.empty()) {
        char data;
        _rxbuf.pop(data);
        *_rx_cur->rx_buf = data;
        _rx_cur->rx_size -= 1;
        _rx_cur->rx_buf += 1;
        _rx_cur->result += 1;
    }

    if (_rx_cur->result > 0) {
        _rx_cur->complete();
        _rx_cur = NULL;
    } else if (!_blocking) {
        read_abort_all();
    }
}

void UARTSerial::read_abort_all()
{
    if (_rx_cur == NULL) {
        _rx_cur = static_cast<UARTSerialRead*>(_rx_list.dequeue());
    }
    while (_rx_cur != NULL) {
        if (_rx_cur->result == 0) {
            _rx_cur->result = (ssize_t)-EAGAIN;
        }
        _rx_cur->complete();
        _rx_cur = static_cast<UARTSerialRead*>(_rx_list.dequeue());
    }
}

bool UARTSerial::hup() const
{
    return _dcd_irq && _dcd_irq->read() != 0;
}

void UARTSerial::wake()
{
    if (_sigio_cb) {
        _sigio_cb();
    }
}

short UARTSerial::poll(short events) const {

    short revents = 0;
    /* Check the Circular Buffer if space available for writing out */


    if (!_rxbuf.empty()) {
        revents |= POLLIN;
    }

    /* POLLHUP and POLLOUT are mutually exclusive */
    if (hup()) {
        revents |= POLLHUP;
    } else if (!_txbuf.full()) {
        revents |= POLLOUT;
    }

    /*TODO Handle other event types */

    return revents;
}

void UARTSerial::lock()
{
    core_util_critical_section_enter();
}

void UARTSerial::unlock()
{
    core_util_critical_section_exit();
}

void UARTSerial::rx_irq(void)
{
    bool was_empty = _rxbuf.empty();

    /* Fill in the receive buffer if the peripheral is readable
     * and receive buffer is not full. */
    while (!_rxbuf.full() && SerialBase::readable()) {
        char data = SerialBase::_base_getc();
        _rxbuf.push(data);
        read_prep_buf();
    }

    if (_rxbuf.full()) {
        SerialBase::attach(NULL, RxIrq);
    }

    /* Report the File handler that data is ready to be read from the buffer. */
    if (was_empty && !_rxbuf.empty()) {
        wake();
    }
}

// Also called from write to start transfer
void UARTSerial::tx_irq(void)
{
    bool was_full = _txbuf.full();

    write_prep_buf();

    /* Write to the peripheral if there is something to write
     * and if the peripheral is available to write. */
    while (!_txbuf.empty() && SerialBase::writeable()) {
        char data;
        _txbuf.pop(data);
        SerialBase::_base_putc(data);
    }

    if (_txbuf.empty()) {
        SerialBase::attach(NULL, TxIrq);
    }

    /* Report the File handler that data can be written to peripheral. */
    if (was_full && !_txbuf.full() && !hup()) {
        wake();
    }
}

void UARTSerial::wait_ms(uint32_t millisec)
{
    /* wait_ms implementation for RTOS spins until exact microseconds - we
     * want to just sleep until next tick.
     */
#if MBED_CONF_RTOS_PRESENT
    rtos::Thread::wait(millisec);
#else
    ::wait_ms(millisec);
#endif
}
} //namespace mbed

#endif //(DEVICE_SERIAL && DEVICE_INTERRUPTIN)
