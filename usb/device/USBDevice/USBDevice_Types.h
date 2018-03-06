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

#ifndef USBDEVICE_TYPES_H
#define USBDEVICE_TYPES_H

#include <stdint.h>

/* Standard requests */
#define GET_STATUS        (0)
#define CLEAR_FEATURE     (1)
#define SET_FEATURE       (3)
#define SET_ADDRESS       (5)
#define GET_DESCRIPTOR    (6)
#define SET_DESCRIPTOR    (7)
#define GET_CONFIGURATION (8)
#define SET_CONFIGURATION (9)
#define GET_INTERFACE     (10)
#define SET_INTERFACE     (11)

/* bmRequestType.dataTransferDirection */
#define HOST_TO_DEVICE (0)
#define DEVICE_TO_HOST (1)

/* bmRequestType.Type*/
#define STANDARD_TYPE  (0)
#define CLASS_TYPE     (1)
#define VENDOR_TYPE    (2)
#define RESERVED_TYPE  (3)

/* bmRequestType.Recipient */
#define DEVICE_RECIPIENT    (0)
#define INTERFACE_RECIPIENT (1)
#define ENDPOINT_RECIPIENT  (2)
#define OTHER_RECIPIENT     (3)

/* Descriptors */
#define DESCRIPTOR_TYPE(wValue)  (wValue >> 8)
#define DESCRIPTOR_INDEX(wValue) (wValue & 0xff)

typedef uint8_t usb_ep_t;

typedef enum {
    USB_EP_TYPE_CTRL = 0,
    USB_EP_TYPE_ISO = 1,
    USB_EP_TYPE_BULK = 2,
    USB_EP_TYPE_INT = 3
} usb_ep_type_t;

enum  {
    USB_EP_ATTR_ALLOW_CTRL = 1 << USB_EP_TYPE_CTRL,
    USB_EP_ATTR_ALLOW_BULK = 1 << USB_EP_TYPE_BULK,
    USB_EP_ATTR_ALLOW_INT = 1 << USB_EP_TYPE_INT,
    USB_EP_ATTR_ALLOW_ISO = 1 << USB_EP_TYPE_ISO,
    USB_EP_ATTR_ALLOW_ALL = USB_EP_ATTR_ALLOW_CTRL | USB_EP_ATTR_ALLOW_BULK |
                             USB_EP_ATTR_ALLOW_INT | USB_EP_ATTR_ALLOW_ISO,

    USB_EP_ATTR_DIR_IN = 0 << 4,
    USB_EP_ATTR_DIR_OUT = 1 << 4,
    USB_EP_ATTR_DIR_IN_OR_OUT = 2 << 4,
    USB_EP_ATTR_DIR_IN_AND_OUT = 3 << 4,
    USB_EP_ATTR_DIR_MASK = 3 << 4
};
typedef uint8_t usb_ep_attr_t;

struct usb_ep_entry_t {
    usb_ep_attr_t attributes;
    uint8_t byte_cost;
    uint16_t base_cost;
};

struct usb_ep_table_t {
    uint32_t resources;
    usb_ep_entry_t table[16];
};

#endif
