/* mbed Microcontroller Library
 * Copyright (c) 2018-2018 ARM Limited
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

#include "stdint.h"
#include "USBCDC.h"
#include "EndpointResolver.h"
#include "AsyncOp.h"

static const uint8_t cdc_line_coding_default[7]= {0x80, 0x25, 0x00, 0x00, 0x00, 0x00, 0x08};

#define DEFAULT_CONFIGURATION (1)

#define CDC_SET_LINE_CODING        0x20
#define CDC_GET_LINE_CODING        0x21
#define CDC_SET_CONTROL_LINE_STATE 0x22

// Control Line State bits
#define CLS_DTR   (1 << 0)
#define CLS_RTS   (1 << 1)

#define CDC_MAX_PACKET_SIZE    64

class AsyncWrite: public AsyncOp {
public:
    AsyncWrite(uint8_t *buf, uint32_t size): AsyncOp(NULL), tx_buf(buf), tx_size(size), result(false)
    {

    }
    uint8_t *tx_buf;
    uint32_t tx_size;
    bool result;
};

class AsyncRead: public AsyncOp {
public:
    AsyncRead(uint8_t *buf, uint32_t size, uint32_t* size_read, bool read_all)
        :   AsyncOp(NULL), rx_buf(buf), rx_size(size), rx_actual(size_read), all(read_all), result(false)
    {

    }
    uint8_t *rx_buf;
    uint32_t rx_size;
    uint32_t *rx_actual;
    bool all;
    bool result;
};

USBCDC::USBCDC(uint16_t vendor_id, uint16_t product_id, uint16_t product_release, bool connect_blocking)
        : USBDevice(vendor_id, product_id, product_release)

{

    memcpy(cdc_line_coding, cdc_line_coding_default, sizeof(cdc_line_coding));

    EndpointResolver resolver(endpoint_table());
    resolver.endpoint_ctrl(CDC_MAX_PACKET_SIZE);
    bulk_in = resolver.endpoint_in(USB_EP_TYPE_BULK, CDC_MAX_PACKET_SIZE);
    bulk_out = resolver.endpoint_out(USB_EP_TYPE_BULK, CDC_MAX_PACKET_SIZE);
    int_in = resolver.endpoint_in(USB_EP_TYPE_INT, CDC_MAX_PACKET_SIZE);
    MBED_ASSERT(resolver.valid());

    terminal_connected = false;

    _tx_cur = NULL;
    _tx_in_progress = false;

    _rx_cur = NULL;
    _rx_count = 0;
    _rx_buf = _rx_buffer;
    _rx_size = 0;

    init();
    USBDevice::connect(connect_blocking);
}

void USBCDC::callback_reset(void) {
    assert_locked();
    /* Called in ISR context */

    _tx_in_progress = false;
    change_terminal_connected(false);
};

void USBCDC::callback_state_change(DeviceState new_state)
{
    assert_locked();
    /* Called in ISR context */

    if (new_state != Configured) {
        change_terminal_connected(false);
    }
}

void USBCDC::callback_request(const setup_packet_t *setup) {
    assert_locked();
    /* Called in ISR context */

    RequestResult result = PassThrough;
    uint8_t *data = NULL;
    uint32_t size = 0;

    /* Only process class-specific requests */
    if (setup->bmRequestType.Type == CLASS_TYPE) {
        switch (setup->bRequest) {
            case CDC_GET_LINE_CODING:
                result = Send;
                data = cdc_line_coding;
                size = 7;
                break;
            case CDC_SET_LINE_CODING:
                result = Receive;
                data = cdc_new_line_coding;
                size = 7;
                break;
            case CDC_SET_CONTROL_LINE_STATE:
                if (setup->wValue & CLS_DTR) {
                    change_terminal_connected(true);
                } else {
                    change_terminal_connected(false);
                }
                result = Success;
                break;
            default:
                result = Failure;
                break;
        }
    }
    complete_request(result, data, size);
}


void USBCDC::callback_request_xfer_done(const setup_packet_t *setup, bool aborted) {
    assert_locked();
    /* Called in ISR context */

    if (aborted) {
        complete_request_xfer_done(false);
        return;
    }

    bool success = false;

    /* Process class-specific requests */
    if (setup->bmRequestType.Type == CLASS_TYPE) {
        if ((setup->bRequest == CDC_SET_LINE_CODING) && (setup->wLength == 7)) {
            if (memcmp(cdc_line_coding, cdc_new_line_coding, 7)) {
                memcpy(cdc_line_coding, cdc_new_line_coding, 7);

                const uint8_t *buf = cdc_line_coding;
                int baud = buf[0] + (buf[1] << 8)
                         + (buf[2] << 16) + (buf[3] << 24);
                int stop = buf[4];
                int bits = buf[6];
                int parity = buf[5];

                line_coding_changed(baud, bits, parity, stop);
            }
            success = true;
        }
        if (setup->bRequest == CDC_GET_LINE_CODING) {
            success = true;
        }
    }

    complete_request_xfer_done(success);
}

void USBCDC::callback_set_configuration(uint8_t configuration) {
    assert_locked();
    /* Called in ISR context */

    bool ret = false;
    if (configuration == DEFAULT_CONFIGURATION) {
        // Configure endpoints > 0
        endpoint_add(int_in, CDC_MAX_PACKET_SIZE, USB_EP_TYPE_INT);
        endpoint_add(bulk_in, CDC_MAX_PACKET_SIZE, USB_EP_TYPE_BULK, &USBCDC::send_isr);
        endpoint_add(bulk_out, CDC_MAX_PACKET_SIZE, USB_EP_TYPE_BULK, &USBCDC::receive_isr);

        // We activate the endpoint to be able to receive data
        read_start(bulk_out);
        ret = true;
    }

    complete_set_configuration(ret);
}

void USBCDC::callback_set_interface(uint16_t interface, uint8_t alternate) {
    assert_locked();
    complete_set_interface(true);
}

void USBCDC::change_terminal_connected(bool connected)
{
    assert_locked();

    if (!connected) {
        send_abort_all();
        receive_abort_all();
    }
    terminal_connected = connected;
}

bool USBCDC::send(uint8_t * buffer, uint32_t size) {
    lock();

    if (!terminal_connected) {
        unlock();
        return false;
    }
    AsyncWrite write_op(buffer, size);
    _tx_list.enqueue(&write_op);
    send_next();

    unlock();

    write_op.wait();
    return write_op.result;
}

void USBCDC::send_next()
{
    assert_locked();
    // Set current TX operation or return if there are none left
    if (_tx_cur == NULL) {
        _tx_cur = static_cast<AsyncWrite*>(_tx_list.dequeue());
    }
    if (_tx_cur == NULL) {
        return;
    }

    if (_tx_in_progress) {
        return;
    }

    uint32_t size_to_send = _tx_cur->tx_size > CDC_MAX_PACKET_SIZE ?  CDC_MAX_PACKET_SIZE : _tx_cur->tx_size;
    if (USBDevice::write(bulk_in, _tx_cur->tx_buf, size_to_send)) {
        _tx_cur->tx_size -= size_to_send;
        _tx_cur->tx_buf += size_to_send;
        _tx_in_progress = true;
        if (_tx_cur->tx_size == 0) {
            _tx_cur->result = true;
            _tx_cur->complete();
            _tx_cur = NULL;
        }
    } else {
        _tx_cur->result = false;
        _tx_cur->complete();
        _tx_cur = NULL;
    }
}

void USBCDC::send_abort_all()
{
    assert_locked();

    if (_tx_cur == NULL) {
        _tx_cur = static_cast<AsyncWrite*>(_tx_list.dequeue());
    }
    while (_tx_cur != NULL) {
        _tx_cur->result = false;
        _tx_cur->complete();
        _tx_cur = static_cast<AsyncWrite*>(_tx_list.dequeue());
    }
}

/*
* Called by when CDC data is sent
* Warning: Called in ISR
*/
void USBCDC::send_isr(usb_ep_t endpoint)
{
    assert_locked();

    _tx_in_progress = false;
    send_next();
}

bool USBCDC::receive(uint8_t * buffer, uint32_t size,  uint32_t *size_read) {
    lock();

    if (!terminal_connected) {
        unlock();
        return false;
    }
    bool read_all = size_read == NULL;
    uint32_t size_read_dummy;
    uint32_t *size_read_ptr = read_all ? &size_read_dummy : size_read;
    *size_read_ptr = 0;
    AsyncRead read_op(buffer, size, size_read_ptr, read_all);
    _rx_list.enqueue(&read_op);
    receive_next();

    unlock();

    read_op.wait();
    return read_op.result;
}

void USBCDC::receive_next()
{
    assert_locked();

    while (true) {
        // Set current RX operation or return if there are none left
        if (_rx_cur == NULL) {
            _rx_cur = static_cast<AsyncRead*>(_rx_list.dequeue());
        }
        if (_rx_cur == NULL) {
            return;
        }

        receive_buf_fill();
        if (_rx_size == 0) {
            return;
        }

        // Copy data over
        uint32_t copy_size = _rx_size > _rx_cur->rx_size ? _rx_cur->rx_size : _rx_size;
        memcpy(_rx_cur->rx_buf, _rx_buf, copy_size);
        _rx_cur->rx_buf += copy_size;
        *_rx_cur->rx_actual += copy_size;
        _rx_cur->rx_size -= copy_size;
        _rx_buf += copy_size;
        _rx_size -= copy_size;

        // Wake thread if request is done
        if (!_rx_cur->all || (_rx_cur->rx_size == 0)) {
            _rx_cur->result = true;
            _rx_cur->complete();
            _rx_cur = NULL;
        }
    }
}

void USBCDC::receive_buf_fill()
{
    assert_locked();

    // Fill receive buffer if it is empty and start another read
    while ((_rx_size == 0) && _rx_count) {
        _rx_buf = _rx_buffer;
        if (read_finish(bulk_out, _rx_buf, sizeof(_rx_buffer), &_rx_size)) {
            read_start(bulk_out);
            _rx_count--;
        } else {
            _rx_size = 0;
            break;
        }
    }
}

void USBCDC::receive_abort_all()
{
    assert_locked();

    if (_rx_cur == NULL) {
        _rx_cur = static_cast<AsyncRead*>(_rx_list.dequeue());
    }
    while (_rx_cur != NULL) {
        _rx_cur->result = false;
        _rx_cur->complete();
        _rx_cur = static_cast<AsyncRead*>(_rx_list.dequeue());
    }
}

/*
* Called by when CDC data is received
* Warning: Called in ISR
*/
void USBCDC::receive_isr(usb_ep_t endpoint)
{
    assert_locked();

    _rx_count++;
    receive_next();

}

const uint8_t * USBCDC::device_desc() {
    uint8_t ep0_size = endpoint_max_packet_size(0x00);
    uint8_t device_descriptor_temp[] = {
        18,                   // bLength
        1,                    // bDescriptorType
        0x10, 0x01,           // bcdUSB
        2,                    // bDeviceClass
        0,                    // bDeviceSubClass
        0,                    // bDeviceProtocol
        ep0_size,             // bMaxPacketSize0
        (uint8_t)(LSB(vendor_id)), (uint8_t)(MSB(vendor_id)),  // idVendor
        (uint8_t)(LSB(product_id)), (uint8_t)(MSB(product_id)),// idProduct
        0x00, 0x01,           // bcdDevice
        1,                    // iManufacturer
        2,                    // iProduct
        3,                    // iSerialNumber
        1                     // bNumConfigurations
    };
    MBED_ASSERT(sizeof(device_descriptor_temp) == sizeof(device_descriptor));
    memcpy(device_descriptor, device_descriptor_temp, sizeof(device_descriptor));
    return device_descriptor;
}

const uint8_t * USBCDC::string_iinterface_desc() {
    static const uint8_t stringIinterfaceDescriptor[] = {
        0x08,
        STRING_DESCRIPTOR,
        'C',0,'D',0,'C',0,
    };
    return stringIinterfaceDescriptor;
}

const uint8_t * USBCDC::string_iproduct_desc() {
    static const uint8_t stringIproductDescriptor[] = {
        0x16,
        STRING_DESCRIPTOR,
        'C',0,'D',0,'C',0,' ',0,'D',0,'E',0,'V',0,'I',0,'C',0,'E',0
    };
    return stringIproductDescriptor;
}


#define CONFIG1_DESC_SIZE (9+8+9+5+5+4+5+7+9+7+7)

const uint8_t * USBCDC::configuration_desc() {
    uint8_t config_descriptor_temp[] = {
        // configuration descriptor
        9,                      // bLength
        2,                      // bDescriptorType
        LSB(CONFIG1_DESC_SIZE), // wTotalLength
        MSB(CONFIG1_DESC_SIZE),
        2,                      // bNumInterfaces
        1,                      // bConfigurationValue
        0,                      // iConfiguration
        0x80,                   // bmAttributes
        50,                     // bMaxPower

        // IAD to associate the two CDC interfaces
        0x08,                   // bLength
        0x0b,                   // bDescriptorType
        0x00,                   // bFirstInterface
        0x02,                   // bInterfaceCount
        0x02,                   // bFunctionClass
        0x02,                   // bFunctionSubClass
        0,                      // bFunctionProtocol
        0,                      // iFunction

        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                      // bLength
        4,                      // bDescriptorType
        0,                      // bInterfaceNumber
        0,                      // bAlternateSetting
        1,                      // bNumEndpoints
        0x02,                   // bInterfaceClass
        0x02,                   // bInterfaceSubClass
        0x01,                   // bInterfaceProtocol
        0,                      // iInterface

        // CDC Header Functional Descriptor, CDC Spec 5.2.3.1, Table 26
        5,                      // bFunctionLength
        0x24,                   // bDescriptorType
        0x00,                   // bDescriptorSubtype
        0x10, 0x01,             // bcdCDC

        // Call Management Functional Descriptor, CDC Spec 5.2.3.2, Table 27
        5,                      // bFunctionLength
        0x24,                   // bDescriptorType
        0x01,                   // bDescriptorSubtype
        0x03,                   // bmCapabilities
        1,                      // bDataInterface

        // Abstract Control Management Functional Descriptor, CDC Spec 5.2.3.3, Table 28
        4,                      // bFunctionLength
        0x24,                   // bDescriptorType
        0x02,                   // bDescriptorSubtype
        0x06,                   // bmCapabilities

        // Union Functional Descriptor, CDC Spec 5.2.3.8, Table 33
        5,                      // bFunctionLength
        0x24,                   // bDescriptorType
        0x06,                   // bDescriptorSubtype
        0,                      // bMasterInterface
        1,                      // bSlaveInterface0

        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        ENDPOINT_DESCRIPTOR_LENGTH,     // bLength
        ENDPOINT_DESCRIPTOR,            // bDescriptorType
        int_in,                         // bEndpointAddress
        E_INTERRUPT,                    // bmAttributes (0x03=intr)
        LSB(CDC_MAX_PACKET_SIZE),       // wMaxPacketSize (LSB)
        MSB(CDC_MAX_PACKET_SIZE),       // wMaxPacketSize (MSB)
        16,                             // bInterval

        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                          // bLength
        4,                          // bDescriptorType
        1,                          // bInterfaceNumber
        0,                          // bAlternateSetting
        2,                          // bNumEndpoints
        0x0A,                       // bInterfaceClass
        0x00,                       // bInterfaceSubClass
        0x00,                       // bInterfaceProtocol
        0,                          // iInterface

        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        ENDPOINT_DESCRIPTOR_LENGTH, // bLength
        ENDPOINT_DESCRIPTOR,        // bDescriptorType
        bulk_in,                    // bEndpointAddress
        E_BULK,                     // bmAttributes (0x02=bulk)
        LSB(CDC_MAX_PACKET_SIZE),   // wMaxPacketSize (LSB)
        MSB(CDC_MAX_PACKET_SIZE),   // wMaxPacketSize (MSB)
        0,                          // bInterval

        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        ENDPOINT_DESCRIPTOR_LENGTH, // bLength
        ENDPOINT_DESCRIPTOR,        // bDescriptorType
        bulk_out,                   // bEndpointAddress
        E_BULK,                     // bmAttributes (0x02=bulk)
        LSB(CDC_MAX_PACKET_SIZE),   // wMaxPacketSize (LSB)
        MSB(CDC_MAX_PACKET_SIZE),   // wMaxPacketSize (MSB)
        0                           // bInterval
    };

    MBED_ASSERT(sizeof(config_descriptor_temp) == sizeof(config_descriptor));
    memcpy(config_descriptor, config_descriptor_temp, sizeof(config_descriptor));
    return config_descriptor;
}
