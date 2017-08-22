/* mbed Microcontroller Library
 * Copyright (c) 2006-2012 ARM Limited
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "rtos/rtos_idle.h"
#include "platform/mbed_sleep.h"
#include "mbed_critical.h"
#include "hal/ticker_api.h"
#include "hal/lp_ticker_api.h"
#include "cmsis_os2.h"
#include "rtx_lib.h"
#include <limits.h>

static ticker_event_t idle_loop_event;


static uint32_t start_time;

/// Setup System Timer.
int32_t osRtxSysTimerSetup (void) {
    const ticker_data_t *lp_ticker_data = get_lp_ticker_data();

    start_time = lp_ticker_read();
    ticker_set_handler(lp_ticker_data, &idle_loop_handler);
    ticker_insert_event_us(lp_ticker_data, &idle_loop_event, start_time + 1000, (uint32_t)&idle_loop_event);
    return 0;                  // Return IRQ number of SysTick
}

/// Enable System Timer.
void osRtxSysTimerEnable (void) {
    // TODO - fire interrupt on next tick
}

/// Disable System Timer.
void osRtxSysTimerDisable (void) {
    ticker_remove_event(get_lp_ticker_data(), &idle_loop_event);
}

/// Acknowledge System Timer IRQ.
void osRtxSysTimerAckIRQ (void) {
    // No special handling needed
}

/// Get System Timer count.
uint32_t osRtxSysTimerGetCount (void) {
    //TODO
    return 0;
}

/// Get System Timer frequency.
uint32_t osRtxSysTimerGetFreq (void) {
  return osRtxInfo.kernel.sys_freq;
}



void idle_loop_handler(uint32_t id)
{
    (void)id;
    //TODO - send Systick to pending
}

static void default_idle_hook(void)
{
#if DEVICE_LOWPOWERTIMER
    timestamp_t time_in_sleep = 0UL;

    core_util_critical_section_enter();
    uint32_t tick_freq = svcRtxKernelGetTickFreq();
    uint32_t ticks_to_sleep = svcRtxKernelSuspend();
    if (ticks_to_sleep) {
        uint64_t us_to_sleep = 1000000 * ticks_to_sleep / tick_freq;
        // Calculate the maximum period we can sleep and stay within
        if (us_to_sleep >= INT_MAX) {
            us_to_sleep = INT_MAX;
        }

        const ticker_data_t *lp_ticker_data = get_lp_ticker_data();
        ticker_remove_event(lp_ticker_data, &idle_loop_event);
        ticker_set_handler(lp_ticker_data, &idle_loop_handler);
        timestamp_t start_time = lp_ticker_read();
        ticker_insert_event_us(lp_ticker_data, &idle_loop_event, us_to_sleep, (uint32_t)&idle_loop_event);

        sleep();
        // calculate how long we slept
        time_in_sleep = lp_ticker_read() - start_time;
    }
    svcRtxKernelResume((uint64_t)time_in_sleep * tick_freq / 1000000);
    core_util_critical_section_exit();
#else
    sleep();
#endif
}
static void (*idle_hook_fptr)(void) = &default_idle_hook;

void rtos_attach_idle_hook(void (*fptr)(void))
{
    //Attach the specified idle hook, or the default idle hook in case of a NULL pointer
    if (fptr != NULL) {
        idle_hook_fptr = fptr;
    } else {
        idle_hook_fptr = default_idle_hook;
    }
}

void rtos_idle_loop(void)
{
    //Continuously call the idle hook function pointer
    while (1) {
        idle_hook_fptr();
    }
}
