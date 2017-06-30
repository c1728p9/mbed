/* mbed Microcontroller Library
 * Copyright (c) 2017-2017 ARM Limited
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

/** \addtogroup hal */
/** @{*/

#ifndef MBED_SLEEP_API_TESTS_H
#define MBED_SLEEP_API_TESTS_H

#include "device.h"

#if DEVICE_SLEEP

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup hal_sleep_tests sleep hal requirements and tests
 * The sleep HAL tests ensure driver conformance to defined behavior.
 *
 * To run the sleep hal tests use the command:
 *
 *     mbed test -t <toolchain> -m <target> -n tests-mbed_hal-sleep*
 *
 * # Defined behavior
 * * The functions ::hal_sleep and ::hal_deepsleep are safe to call from a
 *   critical section - Verified by ::critical_section_test
 * * The function ::hal_sleep does not increase interrupt latency by
 *   more than 20% - Verified by ::sleep_latency_test
 * * The function ::hal_deepsleep disables high frequency clocks - Verified
 *   by ::high_freq_shutoff_test
 * * The function ::hal_deepsleep has a wakeup time less than 10ms - Verified
 *   by ::deepsleep_wakeup_test
 *
 * # Undefined behavior
 * * Calling ::hal_sleep or ::hal_deepsleep from an interrupt handler
 * * The power consumption in ::hal_sleep and ::hal_deepsleep
 *
 * @{
 */

/** Test that sleep functions can be called from a critical section
 *
 */
void critical_section_test(void);

/** Test that sleep increases interrupt latency no longer than 20%
 *
 */
void sleep_latency_test(void);

/** Test that sleep increases interrupt latency no longer than 20%
 *
 */
void sleep_latency_test(void);

/** Test that high freqeuncy clocks are stopped running ::hal_deepsleep
 *
 * This test verifies that the high frequency clocking has been shut off
 * by checking if the microsecond ticker has stopped counting
 *
 */
void high_freq_shutoff_test(void);

/** Test that deepsleep wakeup takes no longer than 10ms
 *
 */
void deepsleep_wakeup_test(void);

/**@}*/

#ifdef __cplusplus
}
#endif

#endif

#endif

