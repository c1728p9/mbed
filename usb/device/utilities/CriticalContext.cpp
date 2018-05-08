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

#include "CriticalContext.h"
#include "Runnable.h"
#include "mbed_critical.h"
#include "mbed_assert.h"

struct runnable_context_t {
    LinkEntry link;
    Runnable *me;
};
MBED_STATIC_ASSERT(sizeof(runnable_context_t) <= sizeof(Runnable::_context_data),
                   "runnable_context_t does not fit");

static Runnable* link_to_runnable(LinkEntry *link)
{
    runnable_context_t *info = (runnable_context_t *)link;
    MBED_ASSERT(info->me);
    return info->me;
}

static LinkEntry* runnable_to_link(Runnable *runnable)
{
    runnable_context_t *info = (runnable_context_t *)runnable->_context_data;
    if (info->me == NULL) {
        new (&info->link) LinkEntry();
        info->me = runnable;
    }
    return &info->link;
}

// Force data to be 4 byte aligned
static uint32_t context_data[(sizeof(CriticalContext) + sizeof(uint32_t) - 1) / sizeof(uint32_t)];

ExecutionContext *CriticalContext::get()
{
    static bool init = false;
    static ExecutionContext *context;

    if (init) {
        return context;
    }

    core_util_critical_section_enter();

    if (!init) {
        context = new (context_data) CriticalContext();
        init = true;
    }

    core_util_critical_section_exit();

    return context;
}

void CriticalContext::lock()
{
    core_util_critical_section_enter();
    _lock_count++;
}

void CriticalContext::unlock()
{
    if (_lock_count == 1) {
        LinkEntry *link = _run_list.dequeue();
        while (link) {
            Runnable *runnable = link_to_runnable(link);
            mbed::Callback<void()> cb = runnable->_callback;
            runnable->_context = NULL;
            cb();
            link = _run_list.dequeue();
        }
    }
    _lock_count--;
    core_util_critical_section_exit();
}

void CriticalContext::post(Runnable *runnable)
{
    lock();

    runnable->_context = this;
    _run_list.enqueue(runnable_to_link(runnable));

    unlock();
}

CriticalContext::CriticalContext(): _lock_count(0)
{

}

CriticalContext::~CriticalContext()
{

}

void CriticalContext::cancel(Runnable *runnable)
{
    MBED_ASSERT(core_util_in_critical_section());

    _run_list.remove(runnable_to_link(runnable));
}

void CriticalContext::assert_context()
{
    MBED_ASSERT(_lock_count > 0);
}

