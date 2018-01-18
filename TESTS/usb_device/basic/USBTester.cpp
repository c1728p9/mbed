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

void USBTester::USBCallback_busReset(void) {
};

bool USBTester::USBCallback_request(void) {
    /* Called in ISR context */

    bool success = false;
    CONTROL_TRANSFER * transfer = getTransferPtr();

    /* Process vendor-specific requests */

    if (transfer->setup.bmRequestType.Type == VENDOR_TYPE) {
        switch (transfer->setup.bRequest) {
            case VENDOR_TEST_CTRL_IN:
                transfer->remaining = transfer->setup.wValue < sizeof(ctrl_buf) ? transfer->setup.wValue  : sizeof(ctrl_buf);
                transfer->ptr = ctrl_buf;
                transfer->direction = DEVICE_TO_HOST;
                success = true;
                break;
            case VENDOR_TEST_CTRL_OUT:
                transfer->remaining = transfer->setup.wValue < 8 ? transfer->setup.wValue  : 8;
                transfer->notify = true;
                success = true;
                break;
            case VENDOR_TEST_CTRL_NONE:
                success = true;
                break;
            default:
                break;
        }
    }

    return success;
}

void USBTester::USBCallback_requestCompleted(uint8_t *buf, uint32_t length) {
    // Request of setting line coding has 7 bytes
    if (length != 7) {
        return;
    }

    CONTROL_TRANSFER * transfer = getTransferPtr();
}

// Called in ISR context
// Set configuration. Return false if the
// configuration is not supported.
bool USBTester::USBCallback_setConfiguration(uint8_t configuration) {
    if (configuration != DEFAULT_CONFIGURATION) {
        return false;
    }

    // Configure endpoints > 0
    addEndpoint(EPINT_IN, MAX_PACKET_SIZE_EPINT);
    addEndpoint(EPINT_OUT, MAX_PACKET_SIZE_EPINT);
    addEndpoint(EPBULK_IN, MAX_PACKET_SIZE_EPBULK);
    addEndpoint(EPBULK_OUT, MAX_PACKET_SIZE_EPBULK);

    readStart(EPBULK_OUT, MAX_PACKET_SIZE_EPBULK);
    readStart(EPINT_OUT, MAX_PACKET_SIZE_EPINT);

    return true;
}

bool USBTester::USBCallback_setInterface(uint16_t interface, uint8_t alternate) {
    if (interface == 0 && alternate == 0) {
        removeEndpoint(bulk_in);
        removeEndpoint(bulk_out);
        removeEndpoint(int_in);
        removeEndpoint(int_out);
        addEndpoint(EPINT_IN, MAX_PACKET_SIZE_EPINT);
        addEndpoint(EPINT_OUT, MAX_PACKET_SIZE_EPINT);
        addEndpoint(EPBULK_IN, MAX_PACKET_SIZE_EPBULK);
        addEndpoint(EPBULK_OUT, MAX_PACKET_SIZE_EPBULK);

        readStart(EPBULK_OUT, MAX_PACKET_SIZE_EPBULK);
        readStart(EPINT_OUT, MAX_PACKET_SIZE_EPINT);
        return true;
    }
    if (interface == 0 && alternate == 1) {
        removeEndpoint(bulk_in);
        removeEndpoint(bulk_out);
        removeEndpoint(int_in);
        removeEndpoint(int_out);
        addEndpoint(EPINT_IN, 8);
        addEndpoint(EPINT_OUT, 8);
        addEndpoint(EPBULK_IN, 8);
        addEndpoint(EPBULK_OUT, 8);

        readStart(EPBULK_OUT, 8);
        readStart(EPINT_OUT, 8);
        return true;
    }
    return false;
}

const uint8_t * USBTester::deviceDesc() {
    uint8_t deviceDescriptorTemp[] = {
        18,                   // bLength
        1,                    // bDescriptorType
        0x10, 0x01,           // bcdUSB
        0,                    // bDeviceClass
        0,                    // bDeviceSubClass
        0,                    // bDeviceProtocol
        MAX_PACKET_SIZE_EP0,  // bMaxPacketSize0
        (uint8_t)(LSB(VENDOR_ID)), (uint8_t)(MSB(VENDOR_ID)),  // idVendor
        (uint8_t)(LSB(PRODUCT_ID)), (uint8_t)(MSB(PRODUCT_ID)),// idProduct
        0x00, 0x01,           // bcdDevice
        1,                    // iManufacturer
        2,                    // iProduct
        3,                    // iSerialNumber
        1                     // bNumConfigurations
    };
    MBED_ASSERT(sizeof(deviceDescriptorTemp) == sizeof(deviceDescriptor));
    memcpy(deviceDescriptor, deviceDescriptorTemp, sizeof(deviceDescriptor));
    return deviceDescriptor;
}

const uint8_t * USBTester::stringIinterfaceDesc() {
    static const uint8_t stringIinterfaceDescriptor[] = {
        0x08,
        STRING_DESCRIPTOR,
        'C',0,'D',0,'C',0,
    };
    return stringIinterfaceDescriptor;
}

const uint8_t * USBTester::stringIproductDesc() {
    static const uint8_t stringIproductDescriptor[] = {
        0x16,
        STRING_DESCRIPTOR,
        'C',0,'D',0,'C',0,' ',0,'D',0,'E',0,'V',0,'I',0,'C',0,'E',0
    };
    return stringIproductDescriptor;
}


#define CONFIG1_DESC_SIZE (9+9+7+7+7+7 + 9+7+7+7+7)

const uint8_t * USBTester::configurationDesc() {
    static const uint8_t configDescriptor[] = {
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
    return configDescriptor;
}


bool USBTester::EPINT_OUT_callback() {
    uint8_t buffer[65];
    uint32_t size = 0;

    if (!readEP_NB(EPINT_OUT, buffer, &size, 64))
        return false;
    if (!readStart(EPINT_OUT, 64))
        return false;

    return true;
}
bool USBTester::EPBULK_OUT_callback() {
    uint8_t buffer[65];
    uint32_t size = 0;

    if (!readEP_NB(EPBULK_OUT, buffer, &size, 64))
        return false;
    if (!readStart(EPBULK_OUT, 64))
        return false;

    return true;
}

