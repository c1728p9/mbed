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
#include "hal/mbed_lp_ticker_wrapper.h"

#if DEVICE_LPTICKER && (LPTICKER_DELAY_TICKS > 0)

#include "Timeout.h"
#include "mbed_critical.h"

static const timestamp_t min_delta = LPTICKER_DELAY_TICKS;

static bool init = false;

static bool pending = false;
static bool timeout_pending = false;
static timestamp_t last_set_interrupt = 0;
static timestamp_t last_request = 0;
static timestamp_t next = 0;

class LowPowerTickerWrapper {
public:
    LowPowerTickerWrapper(ticker_data_t *data);

    void irq_handler();

private:
    Timeout timeout;
    bool expecting_isr;
    bool pending_timeout;
    bool pending_set_interrupt_isr;

    timestamp_t mask;
    timestamp_t reschedule_us;
};






static ticker_irq_handler_type irq_handler = ticker_irq_handler;

static const ticker_interface_t lp_interface = {
    .init = lp_ticker_wrapper_init,
    .read = lp_ticker_wrapper_read,
    .disable_interrupt = lp_ticker_wrapper_disable_interrupt,
    .clear_interrupt = lp_ticker_wrapper_clear_interrupt,
    .set_interrupt = lp_ticker_wrapper_set_interrupt,
    .fire_interrupt = lp_ticker_wrapper_fire_interrupt,
    .get_info = lp_ticker_wrapper_get_info,
};

static const ticker_data_t lp_data = {
    .interface = &lp_interface,
    .queue = NULL,
};




// Do not use SingletonPtr since this must be initialized in a critical section
static LowPowerTickerWrapper *ticker_wrapper;
static uint64_t ticker_wrapper_data[(sizeof(LowPowerTickerWrapper) + 7) / 8];

void lp_ticker_wrapper_irq_handler()
{
    ticker_wrapper->irq_handler();
}

ticker_data_t *get_lp_ticker_wrapper_data(ticker_data_t *data)
{
    if (!init) {
        lp_data.queue = data->queue;
        ticker_wrapper = new (LowPowerTickerWrapper) LowPowerTickerWrapper();
    }
    return &lp_data;
}














void lp_ticker_wrapper_init(void)
{
    ticker_wrapper->init();
}

void lp_ticker_wrapper_free();
{
    ticker_wrapper->free();
}

uint32_t lp_ticker_wrapper_read()
{
    return ticker_wrapper->read();
}

void lp_ticker_wrapper_set_interrupt(timestamp_t timestamp)
{
    ticker_wrapper->set_interrupt(timestamp);
}

void lp_ticker_wrapper_disable_interrupt()
{
    ticker_wrapper->disable_interrupt();
}

void lp_ticker_wrapper_clear_interrupt()
{
    ticker_wrapper->clear_interrupt();
}

void lp_ticker_wrapper_fire_interrupt()
{
    ticker_wrapper->fire_interrupt();
}

const ticker_info_t *lp_ticker_wrapper_get_info()
{
    return ticker_wrapper->get_info();
}













/**
 * Initialize variables
 */
static void init_local()
{
    MBED_ASSERT(core_util_in_critical_section());

    const ticker_info_t *info = lp_ticker_get_info();
    if (info->bits >= 32) {
        mask = 0xffffffff;
    } else {
        mask = ((uint64_t)1 << info->bits) - 1;
    }

    // Round us_per_tick up
    timestamp_t us_per_tick = (1000000 + info->frequency - 1) / info->frequency;

    // Add 1 tick to the min delta for the case where the clock transitions after you read it
    // Add 4 microseconds to round up the micro second ticker time (which has a frequency of at least 250KHz - 4us period)
    reschedule_us = (min_delta + 1) * us_per_tick + 4;

    timeout = new (timeout_data) mbed::Timeout();
}

/**
 * Call lp_ticker_set_interrupt with a value that is guaranteed to fire
 *
 * Assumptions
 * -Only one low power clock tick can pass from the last read (last_read)
 * -The closest an interrupt can fire is max_delta + 1
 *
 * @param last_read The last value read from lp_ticker_read
 * @param timestamp The timestamp to trigger the interrupt at
 */
static void set_interrupt_safe(timestamp_t last_read, timestamp_t timestamp)
{
    MBED_ASSERT(core_util_in_critical_section());
    uint32_t delta = (timestamp - last_read) & mask;
    if (delta < min_delta + 2) {
        timestamp = (last_read + min_delta + 2) & mask;
    }
    lp_ticker_set_interrupt(timestamp);
}

/**
 * Set the low power ticker match time when hardware is ready
 *
 * This event is scheduled to set the lp timer after the previous write
 * has taken effect and it is safe to write a new value without blocking.
 * If the time has already passed then this function fires and interrupt
 * immediately.
 */
static void set_interrupt_later()
{
    core_util_critical_section_enter();

    timestamp_t current = lp_ticker_read();
    if (_ticker_match_interval_passed(last_request, current, next)) {
        lp_ticker_fire_interrupt();
    } else {
        set_interrupt_safe(current, next);
        last_set_interrupt = lp_ticker_read();
    }
    timeout_pending = false;

    core_util_critical_section_exit();
}

void lp_ticker_set_interrupt_wrapper(timestamp_t timestamp)
{
    core_util_critical_section_enter();

    if (!init) {
        init_local();
        init = true;
    }

    timestamp_t current = lp_ticker_read();
    if (pending) {
        // Check if pending should be cleared
        if (((current - last_set_interrupt) & mask) >= min_delta) {
            pending = false;
        }
    }

    if (pending || timeout_pending) {
        next = timestamp;
        last_request = current;
        if (!timeout_pending) {
            timeout->attach_us(set_interrupt_later, reschedule_us);
            timeout_pending = true;
        }
    } else {
        // Schedule immediately if nothing is pending
        set_interrupt_safe(current, timestamp);
        last_set_interrupt = lp_ticker_read();
        pending = true;
    }

    core_util_critical_section_exit();
}

bool lp_ticker_get_timeout_pending()
{
    core_util_critical_section_enter();

    bool pending = timeout_pending;

    core_util_critical_section_exit();

    return pending;
}

#endif
