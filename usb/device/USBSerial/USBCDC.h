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

#ifndef USBCDC_H
#define USBCDC_H

/* These headers are included for child class. */
#include "USBDescriptor.h"
#include "USBDevice_Types.h"

#include "USBDevice.h"
#include "LinkedList.h"

class AsyncWrite;
class AsyncRead;
class USBCDC: public USBDevice {
public:

    /*
    * Constructor
    *
    * @param vendor_id Your vendor_id
    * @param product_id Your product_id
    * @param product_release Your preoduct_release
    * @param connect_blocking define if the connection must be blocked if USB not plugged in
    */
    USBCDC(uint16_t vendor_id, uint16_t product_id, uint16_t product_release, bool connect_blocking);

    /*
    * Send a buffer
    *
    * @param endpoint endpoint which will be sent the buffer
    * @param buffer buffer to be sent
    * @param size length of the buffer
    * @returns true if successful
    */
    bool send(uint8_t * buffer, uint32_t size);

    /*
    * Read a buffer from a certain endpoint. Warning: blocking
    *
    * @param endpoint endpoint to read
    * @param buffer buffer where will be stored bytes
    * @param size the number of bytes read will be stored in *size
    * @param maxSize the maximum length that can be read
    * @returns true if successful
    */
    bool receive(uint8_t * buffer, uint32_t size, uint32_t *size_read);//readEP

protected:
    /*
    * Get device descriptor. Warning: this method has to store the length of the report descriptor in reportLength.
    *
    * @returns pointer to the device descriptor
    */
    virtual const uint8_t * device_desc();

    /*
    * Get string product descriptor
    *
    * @returns pointer to the string product descriptor
    */
    virtual const uint8_t * string_iproduct_desc();

    /*
    * Get string interface descriptor
    *
    * @returns pointer to the string interface descriptor
    */
    virtual const uint8_t * string_iinterface_desc();

    /*
    * Get configuration descriptor
    *
    * @returns pointer to the configuration descriptor
    */
    virtual const uint8_t * configuration_desc();

    /*
    * Read a buffer from a certain endpoint. Warning: non blocking
    *
    * @param endpoint endpoint to read
    * @param buffer buffer where will be stored bytes
    * @param size the number of bytes read will be stored in *size
    * @param maxSize the maximum length that can be read
    * @returns true if successful
    */
//    bool readEP_NB(uint8_t * buffer, uint32_t * size);

    /*
    * Called by USBCallback_requestCompleted when CDC line coding is changed
    * Warning: Called in ISR
    *
    * @param baud The baud rate
    * @param bits The number of bits in a word (5-8)
    * @param parity The parity
    * @param stop The number of stop bits (1 or 2)
    */
    virtual void line_coding_changed(int baud, int bits, int parity, int stop) {};

//    /*
//    * Called by when CDC data is received
//    * Warning: Called in ISR
//    */
//    virtual void data_rx(usb_ep_t endpoint);
//
//    /*
//    * Called by when CDC data is sent
//    * Warning: Called in ISR
//    */
//    virtual void data_tx(usb_ep_t endpoint);

protected:

    virtual void callback_reset(void);
    virtual void callback_state_change(DeviceState new_state);
    virtual void callback_request(const setup_packet_t *setup);
    virtual void callback_request_xfer_done(const setup_packet_t *setup, bool aborted);
    virtual void callback_set_configuration(uint8_t configuration);
    virtual void callback_set_interface(uint16_t interface, uint8_t alternate);

    void change_terminal_connected(bool connected);

    void send_next();
    void send_abort_all();
    void send_isr(usb_ep_t endpoint);

    void receive_next();
    void receive_buf_fill();
    void receive_abort_all();
    void receive_isr(usb_ep_t endpoint);

    usb_ep_t bulk_in;
    usb_ep_t bulk_out;
    usb_ep_t int_in;

    uint8_t cdc_line_coding[7];
    uint8_t cdc_new_line_coding[7];
    uint8_t config_descriptor[75];

    bool terminal_connected;

    AsyncWrite *_tx_cur;
    LinkedList _tx_list;
    bool _tx_in_progress;

    AsyncRead *_rx_cur;
    LinkedList _rx_list;
    uint32_t _rx_count;
    uint8_t _rx_buffer[64];
    uint8_t *_rx_buf;
    uint32_t _rx_size;
};

#endif
