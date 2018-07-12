
/** \addtogroup hal */
/** @{*/
/* mbed Microcontroller Library
 * Copyright (c) 2018 ARM Limited
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
#ifndef MBED_LP_TICKER_WRAPPER_H
#define MBED_LP_TICKER_WRAPPER_H

#include "device.h"

#if DEVICE_LPTICKER && (LPTICKER_DELAY_TICKS > 0)

#include "hal/ticker_api.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Wrapper around lp_ticker_set_interrupt to prevent blocking
 *
 * Problems this function is solving:
 * 1. Interrupt may not fire if set earlier than LPTICKER_DELAY_TICKS low power clock cycles
 * 2. Setting the interrupt back-to-back will block
 *
 * This wrapper function prevents lp_ticker_set_interrupt from being called
 * back-to-back and blocking while the first write is in progress. This function
 * avoids that problem by scheduling a timeout event if the lp ticker is in the
 * middle of a write operation.
 *
 * @param timestamp Time to call ticker irq
 * @note this is a utility function and it's not required part of HAL implementation
 */
void lp_ticker_set_interrupt_wrapper(timestamp_t timestamp);

/**
 * Check if the Timer object used by the wrapper is active
 *
 * @return true if the Timer object is scheduled, false otherwise
 */
bool lp_ticker_get_timeout_pending();

/**@}*/

#ifdef __cplusplus
}
#endif

#endif

#endif

/** @}*/
