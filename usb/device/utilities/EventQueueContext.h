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

#ifndef MBED_EVENT_QUEUE_CONTEXT_H
#define MBED_EVENT_QUEUE_CONTEXT_H

#include "ExecutionContext.h"
#include "LinkedList.h"
#include "Mutex.h"
#include "mbed_events.h"


class EventQueueContext: public ExecutionContext {
public:

    static ExecutionContext *get_shared();

    EventQueueContext(EventQueue *queue, rtos::Mutex *mutex);

    virtual ~EventQueueContext();

    virtual void lock();

    virtual void unlock();

    virtual void post(Runnable *runnable);

    virtual void assert_context();

private:

    virtual void cancel(Runnable *runnable);

    EventQueue *_queue;
    rtos::Mutex *_mutex;
    uint32_t _lock_count;

    LinkedList<Runnable> _run_list;

};

#endif
