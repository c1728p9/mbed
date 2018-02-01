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
    virtual void configure(void);
    virtual void unconfigure(void);
    virtual void set_address(uint8_t address);
    virtual void remote_wakeup(void);
    virtual const endpoint_table_t* endpoint_table();

    virtual void ep0_setup_read_result(uint8_t *buffer);
    virtual void ep0_read(void);
    virtual uint32_t ep0_read_result(uint8_t *buffer);
    virtual void ep0_write(uint8_t *buffer, uint32_t size);
    virtual void ep0_write_result(void);
    virtual void ep0_stall(void);

    virtual bool endpoint_add(uint8_t endpoint, uint32_t maxPacket, uint8_t type);
    virtual bool endpoint_remove(uint8_t endpoint);
    virtual void endpoint_stall(uint8_t endpoint);
    virtual void endpoint_unstall(uint8_t endpoint);

    virtual EP_STATUS endpoint_read(uint8_t endpoint, uint32_t maximumSize);
    virtual EP_STATUS endpoint_read_result(uint8_t endpoint, uint8_t *data, uint32_t *bytesRead);
    virtual EP_STATUS endpoint_write(uint8_t endpoint, uint8_t *data, uint32_t size);
    virtual EP_STATUS endpoint_write_result(uint8_t endpoint);
    virtual EP_STATUS endpoint_write_abort(uint8_t endpoint);

    virtual void process();

private:
    USBPhyEvents *events;

    static void _usbisr(void);
};

#endif
