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

#ifndef USBPHYHW_H
#define USBPHYHW_H

#include "mbed.h"
#include "USBPhy.h"
#include "USBHAL.h"


class USBPhyHw : public USBPhy {
public:
    USBPhyHw();
    virtual ~USBPhyHw();
    virtual void init(USBPhyEvents *events);
    virtual void deinit();
    virtual void connect(void);
    virtual void disconnect(void);
    virtual void configureDevice(void);
    virtual void unconfigureDevice(void);
    virtual void setAddress(uint8_t address);
    virtual void remoteWakeup(void);
    virtual const endpoint_table_t* endpointTable();

    virtual void EP0setupReadResult(uint8_t *buffer);
    virtual void EP0read(void);
    virtual uint32_t EP0ReadResult(uint8_t *buffer);
    virtual void EP0write(uint8_t *buffer, uint32_t size);
    virtual void EP0WriteResult(void);
    virtual void EP0stall(void);

    virtual EP_STATUS endpointRead(uint8_t endpoint, uint32_t maximumSize);
    virtual EP_STATUS endpointReadResult(uint8_t endpoint, uint8_t *data, uint32_t *bytesRead);
    virtual EP_STATUS endpointWrite(uint8_t endpoint, uint8_t *data, uint32_t size);
    virtual EP_STATUS endpointWriteResult(uint8_t endpoint);
    virtual EP_STATUS endpointWriteAbort(uint8_t endpoint);

    virtual void stallEndpoint(uint8_t endpoint);
    virtual void unstallEndpoint(uint8_t endpoint);
    virtual bool addEndpoint(uint8_t endpoint, uint32_t maxPacket, uint8_t type);
    virtual bool removeEndpoint(uint8_t endpoint);

    virtual void process();

private:
    USBPhyEvents *events;
    virtual void busReset(void){if (events) {events->busReset();}};
    virtual void EP0setupCallback(void){if (events) {events->EP0setup();}};
    virtual void EP0out(void){if (events) {events->EP0out();}};
    virtual void EP0in(void){if (events) {events->EP0in();}};
    virtual void connectStateChanged(unsigned int connected){if (events) {events->connectStateChanged(connected);}};
    virtual void suspendStateChanged(unsigned int suspended){if (events) {events->suspendStateChanged(suspended);}};
    virtual void SOF(int frameNumber){if (events) {events->SOF(frameNumber);}};

    static void _usbisr(void);
};

#endif
