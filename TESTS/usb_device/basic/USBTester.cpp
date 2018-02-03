/*
 * Copyright (c) 2018-2018, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "stdint.h"
#include "USBTester.h"

#define DEFAULT_CONFIGURATION (1)

#define CDC_SET_LINE_CODING        0x20
#define CDC_GET_LINE_CODING        0x21
#define CDC_SET_CONTROL_LINE_STATE 0x22

// Control Line State bits
#define CLS_DTR   (1 << 0)
#define CLS_RTS   (1 << 1)

#define VENDOR_TEST_CTRL_IN         1
#define VENDOR_TEST_CTRL_OUT        2
#define VENDOR_TEST_CTRL_NONE       3

#define MAX_CDC_REPORT_SIZE MAX_PACKET_SIZE_EPBULK

USBTester::USBTester(uint16_t vendor_id, uint16_t product_id, uint16_t product_release, bool connect_blocking): USBDevice(vendor_id, product_id, product_release) {
    bulk_in = EPBULK_IN;
    bulk_out = EPBULK_OUT;
    int_in = EPINT_IN;
    int_out = EPINT_OUT;

    USBDevice::connect(connect_blocking);

}

void USBTester::callback_reset(void) {
    complete_reset();
};

bool USBTester::callback_request(void) {
    /* Called in ISR context */

    bool success = false;
    control_transfer_t *transfer = get_transfer_ptr();

    /* Process vendor-specific requests */

    if (transfer->setup.bmRequestType.Type == VENDOR_TYPE) {
        switch (transfer->setup.bRequest) {
            case VENDOR_TEST_CTRL_IN:
                transfer->remaining = transfer->setup.wValue < sizeof(ctrl_buf) ? transfer->setup.wValue  : sizeof(ctrl_buf);
                transfer->ptr = ctrl_buf;
                transfer->direction = DEVICE_TO_HOST;
                complete_request(true);
                success = true;
                break;
            case VENDOR_TEST_CTRL_OUT:
                transfer->remaining = transfer->setup.wValue < 8 ? transfer->setup.wValue  : 8;
                transfer->ptr = ctrl_buf;
                transfer->direction = HOST_TO_DEVICE;
                transfer->notify = true;
                complete_request(true);
                success = true;
                break;
            case VENDOR_TEST_CTRL_NONE:
                complete_request(true);
                success = true;
                break;
            default:
                break;
        }
    }

    return success;
}

void USBTester::callback_request_xfer_done() {
//    // Request of setting line coding has 7 bytes
//    if (length != 7) {
//        complete_request_data(false);
//        return;
//    }
//
//    control_transfer_t *transfer = get_transfer_ptr();
    complete_request_xfer_done(true);
}

// Called in ISR context
// Set configuration. Return false if the
// configuration is not supported.
void USBTester::callback_set_configuration(uint8_t configuration) {
    if (configuration != DEFAULT_CONFIGURATION) {
        complete_set_configuration(false);
        return;
    }

    // Configure endpoints > 0
    endpoint_add(EPINT_IN, MAX_PACKET_SIZE_EPINT);
    endpoint_add(EPINT_OUT, MAX_PACKET_SIZE_EPINT, 0, &USBTester::epint_out_callback);
    endpoint_add(EPBULK_IN, MAX_PACKET_SIZE_EPBULK);
    endpoint_add(EPBULK_OUT, MAX_PACKET_SIZE_EPBULK, 0, &USBTester::epbulk_out_callback);

    readStart(EPBULK_OUT, MAX_PACKET_SIZE_EPBULK);
    readStart(EPINT_OUT, MAX_PACKET_SIZE_EPINT);

    complete_set_configuration(true);
}

void USBTester::callback_set_interface(uint16_t interface, uint8_t alternate) {
    if (interface == 0 && alternate == 0) {
        endpoint_remove(bulk_in);
        endpoint_remove(bulk_out);
        endpoint_remove(int_in);
        endpoint_remove(int_out);
        endpoint_add(EPINT_IN, MAX_PACKET_SIZE_EPINT);
        endpoint_add(EPINT_OUT, MAX_PACKET_SIZE_EPINT, 0, &USBTester::epint_out_callback);
        endpoint_add(EPBULK_IN, MAX_PACKET_SIZE_EPBULK);
        endpoint_add(EPBULK_OUT, MAX_PACKET_SIZE_EPBULK, 0, &USBTester::epbulk_out_callback);

        readStart(EPBULK_OUT, MAX_PACKET_SIZE_EPBULK);
        readStart(EPINT_OUT, MAX_PACKET_SIZE_EPINT);
        complete_set_interface(true);
        return;
    }
    if (interface == 0 && alternate == 1) {
        endpoint_remove(bulk_in);
        endpoint_remove(bulk_out);
        endpoint_remove(int_in);
        endpoint_remove(int_out);
        endpoint_add(EPINT_IN, 8);
        endpoint_add(EPINT_OUT, 8, 0, &USBTester::epint_out_callback);
        endpoint_add(EPBULK_IN, 8);
        endpoint_add(EPBULK_OUT, 8, 0, &USBTester::epbulk_out_callback);

        readStart(EPBULK_OUT, 8);
        readStart(EPINT_OUT, 8);
        complete_set_interface(true);
        return;
    }
    complete_set_interface(false);
}

const uint8_t * USBTester::device_desc() {
    uint8_t device_descriptor_temp[] = {
        18,                   // bLength
        1,                    // bDescriptorType
        0x10, 0x01,           // bcdUSB
        0,                    // bDeviceClass
        0,                    // bDeviceSubClass
        0,                    // bDeviceProtocol
        MAX_PACKET_SIZE_EP0,  // bMaxPacketSize0
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

const uint8_t * USBTester::string_iinterface_desc() {
    static const uint8_t string_iinterface_descriptor[] = {
        0x08,
        STRING_DESCRIPTOR,
        'C',0,'D',0,'C',0,
    };
    return string_iinterface_descriptor;
}

const uint8_t * USBTester::string_iproduct_desc() {
    static const uint8_t string_iproduct_descriptor[] = {
        0x16,
        STRING_DESCRIPTOR,
        'C',0,'D',0,'C',0,' ',0,'D',0,'E',0,'V',0,'I',0,'C',0,'E',0
    };
    return string_iproduct_descriptor;
}


#define CONFIG1_DESC_SIZE (9+9+7+7+7+7 + 9+7+7+7+7)

const uint8_t * USBTester::configuration_desc() {
    static const uint8_t config_descriptor[] = {
        // configuration descriptor
        9,                      // bLength
        2,                      // bDescriptorType
        LSB(CONFIG1_DESC_SIZE), // wTotalLength
        MSB(CONFIG1_DESC_SIZE),
        1,                      // bNumInterfaces
        1,                      // bConfigurationValue
        0,                      // iConfiguration
        0x80,                   // bmAttributes
        50,                     // bMaxPower

        // Interface 0 setting 0

        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                          // bLength
        4,                          // bDescriptorType
        0,                          // bInterfaceNumber
        0,                          // bAlternateSetting
        4,                          // bNumEndpoints
        0xFF,                       // bInterfaceClass
        0xFF,                       // bInterfaceSubClass
        0xFF,                       // bInterfaceProtocol
        0,                          // iInterface

        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        ENDPOINT_DESCRIPTOR_LENGTH, // bLength
        ENDPOINT_DESCRIPTOR,        // bDescriptorType
        PHY_TO_DESC(EPBULK_IN),     // bEndpointAddress
        E_BULK,                     // bmAttributes (0x02=bulk)
        LSB(MAX_PACKET_SIZE_EPBULK),// wMaxPacketSize (LSB)
        MSB(MAX_PACKET_SIZE_EPBULK),// wMaxPacketSize (MSB)
        0,                          // bInterval

        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        ENDPOINT_DESCRIPTOR_LENGTH, // bLength
        ENDPOINT_DESCRIPTOR,        // bDescriptorType
        PHY_TO_DESC(EPBULK_OUT),    // bEndpointAddress
        E_BULK,                     // bmAttributes (0x02=bulk)
        LSB(MAX_PACKET_SIZE_EPBULK),// wMaxPacketSize (LSB)
        MSB(MAX_PACKET_SIZE_EPBULK),// wMaxPacketSize (MSB)
        0,                          // bInterval



        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        ENDPOINT_DESCRIPTOR_LENGTH, // bLength
        ENDPOINT_DESCRIPTOR,        // bDescriptorType
        PHY_TO_DESC(EPINT_IN),      // bEndpointAddress
        E_INTERRUPT,                // bmAttributes (0x03=interrupt)
        LSB(MAX_PACKET_SIZE_EPINT), // wMaxPacketSize (LSB)
        MSB(MAX_PACKET_SIZE_EPINT), // wMaxPacketSize (MSB)
        1,                          // bInterval

        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        ENDPOINT_DESCRIPTOR_LENGTH, // bLength
        ENDPOINT_DESCRIPTOR,        // bDescriptorType
        PHY_TO_DESC(EPINT_OUT),     // bEndpointAddress
        E_INTERRUPT,                // bmAttributes (0x03=interrupt)
        LSB(MAX_PACKET_SIZE_EPINT), // wMaxPacketSize (LSB)
        MSB(MAX_PACKET_SIZE_EPINT), // wMaxPacketSize (MSB)
        1,                          // bInterval

        // Interface 0 setting 1

        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                          // bLength
        4,                          // bDescriptorType
        0,                          // bInterfaceNumber
        1,                          // bAlternateSetting
        4,                          // bNumEndpoints
        0xFF,                       // bInterfaceClass
        0xFF,                       // bInterfaceSubClass
        0xFF,                       // bInterfaceProtocol
        0,                          // iInterface

        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        ENDPOINT_DESCRIPTOR_LENGTH, // bLength
        ENDPOINT_DESCRIPTOR,        // bDescriptorType
        PHY_TO_DESC(EPBULK_IN),     // bEndpointAddress
        E_BULK,                     // bmAttributes (0x02=bulk)
        LSB(8),                     // wMaxPacketSize (LSB)
        MSB(8),                     // wMaxPacketSize (MSB)
        0,                          // bInterval

        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        ENDPOINT_DESCRIPTOR_LENGTH, // bLength
        ENDPOINT_DESCRIPTOR,        // bDescriptorType
        PHY_TO_DESC(EPBULK_OUT),    // bEndpointAddress
        E_BULK,                     // bmAttributes (0x02=bulk)
        LSB(8),                     // wMaxPacketSize (LSB)
        MSB(8),                     // wMaxPacketSize (MSB)
        0,                          // bInterval



        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        ENDPOINT_DESCRIPTOR_LENGTH, // bLength
        ENDPOINT_DESCRIPTOR,        // bDescriptorType
        PHY_TO_DESC(EPINT_IN),      // bEndpointAddress
        E_INTERRUPT,                // bmAttributes (0x03=interrupt)
        LSB(8),                     // wMaxPacketSize (LSB)
        MSB(8),                     // wMaxPacketSize (MSB)
        1,                          // bInterval

        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        ENDPOINT_DESCRIPTOR_LENGTH, // bLength
        ENDPOINT_DESCRIPTOR,        // bDescriptorType
        PHY_TO_DESC(EPINT_OUT),     // bEndpointAddress
        E_INTERRUPT,                // bmAttributes (0x03=interrupt)
        LSB(8),                     // wMaxPacketSize (LSB)
        MSB(8),                     // wMaxPacketSize (MSB)
        1                           // bInterval

    };
    return config_descriptor;
}


void USBTester::epint_out_callback() {
    uint8_t buffer[65];
    uint32_t size = 0;

    if (!readEP_NB(EPINT_OUT, buffer, &size, 64))
        return;
    if (!readStart(EPINT_OUT, 64))
        return;
}
void USBTester::epbulk_out_callback() {
    uint8_t buffer[65];
    uint32_t size = 0;

    if (!readEP_NB(EPBULK_OUT, buffer, &size, 64))
        return;
    if (!readStart(EPBULK_OUT, 64))
        return;
}

