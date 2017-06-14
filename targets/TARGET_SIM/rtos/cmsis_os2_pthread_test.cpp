/*
 * Copyright (c) 2017-2017 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "mbed.h"
#include "SimulatorTest.h"
#include <time.h>
#include "cmsis_os2.h"

MBED_TEST(time_test, nanosecond_range)
{
    struct timespec ts;
    int largest = 0;
    for (int i = 0; i < 1000000; i++) {
        clock_gettime(CLOCK_REALTIME, &ts);
        if (ts.tv_nsec > largest) {
            largest = ts.tv_nsec;
        }
        TEST_ASSERT(ts.tv_nsec <1000000000);
    }
}

//MBED_TEST(time_test, nanosleep)
//{
//    int ret;
//    struct timespec sleep_time;
//    struct timespec remaining;
//    printf("Staring 2 second sleep\r\n");
//    sleep_time.tv_nsec = 999999999;
//    sleep_time.tv_sec = 1;
//    ret = nanosleep(&sleep_time, &remaining);
//    TEST_ASSERT(0 == ret);
//    printf("Finished 2 second sleep\r\n");
//}

MBED_TEST(time_test, nanosleep_invalid)
{
    int ret;
    struct timespec sleep_time;
    struct timespec remaining;

    sleep_time.tv_nsec = 1000000000;
    sleep_time.tv_sec = 1;
    ret = nanosleep(&sleep_time, &remaining);
    TEST_ASSERT_EQUAL_INT(-1, ret);
    TEST_ASSERT_EQUAL_INT(EINVAL, errno);
}

static void thread_increment(void *ptr)
{
    uint32_t *val = (uint32_t*)ptr;
    for (int i = 0; i < 5; i++) {
        *val += 1;
    }
    printf("test thread done, thread name %s\r\n", osThreadGetName(osThreadGetId()));
    fflush(stdout);
}

MBED_TEST(pthread_test, basic)
{
    osThreadId_t thread;
    osThreadAttr_t attr;
    static const char *thread_name = "Test thread";
    uint32_t val = 0;
    memset(&attr, 0, sizeof(attr));
    attr.attr_bits = osThreadJoinable;
    attr.name = thread_name;

    thread = osThreadNew(thread_increment, (void*)&val, &attr);

    TEST_ASSERT_EQUAL_STRING(thread_name, osThreadGetName(thread));

    osThreadJoin(thread);
    TEST_ASSERT_EQUAL_UINT32(5, val);
}
