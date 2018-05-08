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

#include "EventQueueContext.h"
#include "Runnable.h"
#include "mbed_critical.h"
#include "mbed_assert.h"
#include "mbed_shared_queues.h"

struct runnable_context_t {
    int token;
    int id;
    Runnable *runnable;
};
MBED_STATIC_ASSERT(sizeof(runnable_context_t) <= sizeof(Runnable::_context_data),
                   "runnable_context_t does not fit");

ExecutionContext *EventQueueContext::get_shared()
{
    static rtos::Mutex mutex;
    static EventQueueContext context(mbed::mbed_event_queue(), &mutex);
    return &context;
}

EventQueueContext::EventQueueContext(EventQueue *queue, rtos::Mutex *mutex):
    _queue(queue), _mutex(mutex), _lock_count(0)
{

}

EventQueueContext::~EventQueueContext()
{

}

void EventQueueContext::lock()
{
    _mutex->lock();
    _lock_count++;
}

void EventQueueContext::unlock()
{
    _lock_count--;
    _mutex->unlock();
}

static void run(Runnable *runnable, EventQueueContext *context, int token)
{
    context->lock();

    core_util_critical_section_enter();

    runnable_context_t *info = (runnable_context_t*)runnable->_context_data;
    mbed::Callback<void()> cb;
    if (token == info->token) {
        cb = runnable->_callback;
        runnable->_context = NULL;
    }

    core_util_critical_section_exit();

    if (cb) {
        cb();
    }

    context->unlock();
}

void EventQueueContext::post(Runnable *runnable)
{
    core_util_critical_section_enter();

    runnable->_context = this;
    runnable_context_t *info = (runnable_context_t*)runnable->_context_data;
    info->token++;
    info->id = _queue->call(run, runnable, this, info->token);
    MBED_ASSERT(info->id);

    core_util_critical_section_exit();
}

void EventQueueContext::assert_context()
{
    lock();

    MBED_ASSERT(_lock_count > 1);

    unlock();
}

void EventQueueContext::cancel(Runnable *runnable)
{
    MBED_ASSERT(core_util_in_critical_section());

    runnable_context_t *info = (runnable_context_t*)runnable->_context_data;
    _queue->cancel(info->id);
    info->id = 0;
    info->token++;
    runnable->_context = NULL;
}
