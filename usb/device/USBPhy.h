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

#ifndef USBPHY_H
#define USBPHY_H

#include "mbed.h"
#include "USBEndpoints.h"

/**
 * \defgroup hal_usb_device USB Device HAL
 * Abstract interface to physical USB hardware
 *
 * # Defined behavior
 * * Any endpoint configurations which fit in the parameters of the table returned
 *      by USBPhy::endpointTable can be used.
 * * Device supports use of at least one control, bulk, interrupt and
 *      isochronous in each direction at the same time - at least 8 endpoints.
 * * Device supports all standard endpoint sizes (wMaxPacketSize)
 * * Device can handle an interrupt latency of at least 100ms if reset is not being performed and address is not being set
 *
 * # Undefined behavior
 * * Calling USBPhy::addEndpoint or USBPhy::removeEndpoint outside of the control requests SetInterface or SetConfiguration
 * * Devices behavior is undefined if latency is greater than 2ms when address is being set - see USB spec 9.2.6.3
 * * Devices behavior is undefined if latency is greater than 10ms when a reset occurs - see USB spec 7.1.7.5
 *
 * # Potential bugs
 * * Processing control packets in the wrong order when multiple packets are present
 * * Back to back setup packets handled incorrectly
 * * Signal corruption not handled correctly
 * * USB race conditions
 *
 */

enum  {
    EP_TYPE_CTRL = 1 << 0,
    EP_TYPE_BULK = 1 << 1,
    EP_TYPE_INT = 1 << 2,
    EP_TYPE_ISO = 1 << 3,

    EP_DIR_IN = 0 << 4,
    EP_DIR_OUT = 1 << 4,
    EP_DIR_IN_OR_OUT = 2 << 4,
    EP_DIR_IN_AND_OUT = 3 << 4,
    EP_DIR_MASK = 3 << 4
};
typedef uint8_t ep_attr_t;

struct endpoint_entry_t {
    ep_attr_t attributes;
    uint8_t byte_cost;
    uint16_t base_cost;
};

struct endpoint_table_t {
    uint32_t resources;
    endpoint_entry_t table[16];
};

class USBPhyEvents {
public:
    USBPhyEvents() {};
    virtual ~USBPhyEvents() {};
    virtual void busReset(void) = 0;
    virtual void EP0setup(void) = 0;
    virtual void EP0out(void) = 0;
    virtual void EP0in(void) = 0;
    virtual void connectStateChanged(unsigned int connected) = 0;
    virtual void suspendStateChanged(unsigned int suspended) = 0;
    virtual void SOF(int frameNumber) = 0;
    virtual bool OUT_callback(uint8_t endpoint) = 0;
    virtual bool IN_callback(uint8_t endpoint) = 0;

    virtual void startProcess() = 0;
};

class USBPhy {
public:
    USBPhy() {};
    virtual ~USBPhy() {};
    virtual void init(USBPhyEvents *events) = 0;
    virtual void deinit() = 0;
    virtual void connect(void) = 0;
    virtual void disconnect(void) = 0;
    virtual void configureDevice(void) = 0;
    virtual void unconfigureDevice(void) = 0;
    virtual void setAddress(uint8_t address) = 0;
    virtual void remoteWakeup(void) = 0;
    virtual const endpoint_table_t* endpointTable() = 0;

    virtual void EP0setupReadResult(uint8_t *buffer) = 0;
    virtual void EP0read(void) = 0;
    virtual uint32_t EP0ReadResult(uint8_t *buffer) = 0;
    virtual void EP0write(uint8_t *buffer, uint32_t size) = 0;
    virtual void EP0WriteResult(void) = 0;
    virtual void EP0stall(void) = 0;

    virtual EP_STATUS endpointRead(uint8_t endpoint, uint32_t maximumSize) = 0;
    virtual EP_STATUS endpointReadResult(uint8_t endpoint, uint8_t *data, uint32_t *bytesRead) = 0;
    virtual EP_STATUS endpointWrite(uint8_t endpoint, uint8_t *data, uint32_t size) = 0;
    virtual EP_STATUS endpointWriteResult(uint8_t endpoint) = 0;
    virtual EP_STATUS endpointWriteAbort(uint8_t endpoint) = 0;

    virtual void stallEndpoint(uint8_t endpoint) = 0;
    virtual void unstallEndpoint(uint8_t endpoint) = 0;
    virtual bool addEndpoint(uint8_t endpoint, uint32_t maxPacket, uint8_t type) = 0;
    virtual bool removeEndpoint(uint8_t endpoint) = 0;

    virtual void process() = 0;
};

#endif
