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
#include <stdio.h>
#include <string.h>
#include "mbed.h"
#include "greentea-client/test_env.h"
#include "unity/unity.h"
#include "utest/utest.h"

#include "USBTester.h"

using namespace utest::v1;

// Echo server (echo payload to host)
void test_case_basic() {
    char _key[11] = {};
    char _value[128] = {};

    {
    USBTester serial(0x1f00, 0x2012, 0x0001, true);

//    uint8_t buf_bulk[MAX_PACKET_SIZE_EPBULK];
//    uint8_t buf_int[MAX_PACKET_SIZE_EPINT];
//    uint32_t bulk_size;
//    uint32_t int_size;

//    serial.readStart(EPBULK_OUT, MAX_PACKET_SIZE_EPBULK);
//    serial.readStart(EPINT_OUT, MAX_PACKET_SIZE_EPINT);
//    serial.USBDevice::readEP_NB(EPBULK_OUT, buf_bulk, &bulk_size, MAX_PACKET_SIZE_EPBULK);
//    serial.USBDevice::readEP_NB(EPINT_OUT, buf_int, &int_size, MAX_PACKET_SIZE_EPINT);

    greentea_send_kv("usb_enumeration_done", "0123456789");
    // Wait for host before terminating
    greentea_parse_kv(_key, _value, sizeof(_key), sizeof(_value));
    }
}

Case cases[] = {
    Case("pyusb basic test", test_case_basic),
};

utest::v1::status_t greentea_test_setup(const size_t number_of_cases) {
    GREENTEA_SETUP(120, "pyusb_basic");
    return greentea_test_setup_handler(number_of_cases);
}

Specification specification(greentea_test_setup, cases, greentea_test_teardown_handler);

int main() {
    Harness::run(specification);
}


