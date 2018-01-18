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

#ifndef USB_TESTER_H
#define USB_TESTER_H

/* These headers are included for child class. */
#include "USBEndpoints.h"
#include "USBDescriptor.h"
#include "USBDevice_Types.h"

#include "USBDevice.h"

class USBTester: public USBDevice {
public:

    /*
    * Constructor
    *
    * @param vendor_id Your vendor_id
    * @param product_id Your product_id
    * @param product_release Your preoduct_release
    * @param connect_blocking define if the connection must be blocked if USB not plugged in
    */
    USBTester(uint16_t vendor_id, uint16_t product_id, uint16_t product_release, bool connect_blocking);

protected:

    /*
    * Get device descriptor. Warning: this method has to store the length of the report descriptor in reportLength.
    *
    * @returns pointer to the device descriptor
    */
    virtual const uint8_t * deviceDesc();

    /*
    * Get string product descriptor
    *
    * @returns pointer to the string product descriptor
    */
    virtual const uint8_t * stringIproductDesc();

    /*
    * Get string interface descriptor
    *
    * @returns pointer to the string interface descriptor
    */
    virtual const uint8_t * stringIinterfaceDesc();

    /*
    * Get configuration descriptor
    *
    * @returns pointer to the configuration descriptor
    */
    virtual const uint8_t * configurationDesc();

protected:
    uint8_t bulk_in;
    uint8_t bulk_out;
    uint8_t int_in;
    uint8_t int_out;

    virtual bool USBCallback_request();
    virtual void USBCallback_requestCompleted(uint8_t *buf, uint32_t length);
    virtual bool USBCallback_setConfiguration(uint8_t configuration);
    virtual bool USBCallback_setInterface(uint16_t interface, uint8_t alternate);
    virtual void USBCallback_busReset(void);
    virtual bool EPBULK_OUT_callback();
    virtual bool EPINT_OUT_callback();
    uint8_t ctrl_buf[2048];

};

#endif
