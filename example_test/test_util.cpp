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

#include "mbed.h"
#include "test_util.h"

void test_util_countdown(int count)
{
    while (count > 0) {
        printf("%i\r\n", count);
        count--;
        wait(1);
    }
}

void test_util_wait_line()
{
    int val;
    do {
        val = getc(stdin);
        MBED_ASSERT(val >= 0);
    } while (val != '\r' && val != '\n');
}

void test_util_read_line(char *buf, uint32_t size)
{
    int val;
    do {
        val = getc(stdin);
        MBED_ASSERT(val >= 0);
    } while (val == '\r' || val == '\n' || val == ' ');

    while (val != '\r' && val != '\n') {
        *buf = val;
        buf++;
        size--;
        if (size == 1) {
            *buf = 0;
            break;
        }
        val = getc(stdin);
    }
    if (size) {
        *buf = 0;
    }
}

bool test_util_get_response()
{
    int val = getc(stdin);
    while (val != 'y' && val != 'n') {
        printf("Enter y or n for Yes or No\r\n");
        val = getc(stdin);
    }
    return val == 'y' ? true : false;
}
