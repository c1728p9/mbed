/* mbed Microcontroller Library
 * Copyright (c) 2016 ARM Limited
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

#if DEVICE_LOWPOWERTIMER

#include "lp_ticker_api.h"
#include "fsl_rtc.h"
#include "fsl_lptmr.h"
#include "cmsis.h"
#include "rtc_api.h"

#define OSC32K_CLK_HZ (32768)
#define TIME_BITS       15
#define TIME_MASK       ((1LLU << TIME_BITS) - 1)

static bool lp_ticker_inited = false;

static void lptmr_isr(void)
{
    LPTMR_ClearStatusFlags(LPTMR0, kLPTMR_TimerCompareFlag);
    LPTMR_StopTimer(LPTMR0);

    lp_ticker_read();
    lp_ticker_irq_handler();
}

/** Initialize the low power ticker
 *
 */
void lp_ticker_init(void)
{
    lptmr_config_t lptmrConfig;

    if (lp_ticker_inited) {
        return;
    }
    lp_ticker_inited = true;

    /* Setup low resolution clock - RTC */
    if (!rtc_isenabled()) {
        rtc_init();
        RTC_DisableInterrupts(RTC, kRTC_AlarmInterruptEnable | kRTC_SecondsInterruptEnable);
        RTC_StartTimer(RTC);
    }

    /* Setup high resolution clock - LPTMR */
    LPTMR_GetDefaultConfig(&lptmrConfig);
    /* Use 32kHz drive */
    CLOCK_SetXtal32Freq(OSC32K_CLK_HZ);
    lptmrConfig.prescalerClockSource = kLPTMR_PrescalerClock_2;
    LPTMR_Init(LPTMR0, &lptmrConfig);
    LPTMR_EnableInterrupts(LPTMR0, kLPTMR_TimerInterruptEnable);
    NVIC_ClearPendingIRQ(LPTMR0_IRQn);
    NVIC_SetVector(LPTMR0_IRQn, (uint32_t)lptmr_isr);
    EnableIRQ(LPTMR0_IRQn);
}

/** Read the current counter
 *
 * @return The current timer's counter value in microseconds
 */
uint32_t lp_ticker_read(void)
{
    if (!lp_ticker_inited) {
        lp_ticker_init();
    }

    uint32_t val = RTC->TPR;
    uint32_t last_val = RTC->TPR;
    while (val != last_val) {
        last_val = val;
        val = RTC->TPR;
    }
    return val & TIME_MASK;
}

/** Set interrupt for specified timestamp
 *
 * @param timestamp The time in microseconds to be set
 */
void lp_ticker_set_interrupt(timestamp_t timestamp)
{
    if (!lp_ticker_inited) {
        lp_ticker_init();
    }

    LPTMR_StopTimer(LPTMR0);
    LPTMR_SetTimerPeriod(LPTMR0, (timestamp - lp_ticker_read()) & TIME_MASK);
    LPTMR_EnableInterrupts(LPTMR0, kLPTMR_TimerInterruptEnable);
    LPTMR_StartTimer(LPTMR0);
}

void lp_ticker_fire_interrupt(void)
{
    NVIC_SetPendingIRQ(LPTMR0_IRQn);
}

/** Disable low power ticker interrupt
 *
 */
void lp_ticker_disable_interrupt(void)
{
    LPTMR_DisableInterrupts(LPTMR0, kLPTMR_TimerInterruptEnable);
    RTC_DisableInterrupts(RTC, kRTC_AlarmInterruptEnable);
}

/** Clear the low power ticker interrupt
 *
 */
void lp_ticker_clear_interrupt(void)
{
    LPTMR_ClearStatusFlags(LPTMR0, kLPTMR_TimerCompareFlag);
}

const ticker_info_t* lp_ticker_get_info()
{
    static ticker_info_t info;
    info.frequency = CLOCK_GetFreq(kCLOCK_Er32kClk);
    info.bits = TIME_BITS;
    return &info;
}

#endif /* DEVICE_LOWPOWERTIMER */
