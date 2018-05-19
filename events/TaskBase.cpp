/* events
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

#include "events/TaskBase.h"
#include "events/TaskQueue.h"
#include "events/mbed_events.h"
#include "rtos/Semaphore.h"
#include "mbed.h"

static void stub(void *buffer)
{
    (void)buffer;
}

TaskBase::TaskBase(uint8_t *data, uint32_t size, copy_callback_t copy, run_callback_t run)
    : size(size), _buffer(data), _copy(copy), _callback(cb),
      _started(false), _queue(NULL), _flush_sem(NULL)
{
    _copy = copy;
    if (!_callback) {
        _callback = stub;
    }
}

TaskBase::~TaskBase()
{
    cancel();
    wait_finished();
}

TaskBase &TaskBase::operator=(callback_t callback)
{
    // Event must be idle
    MBED_ASSERT(ready());

    if (callback) {
        _callback = callback;
    } else {
        _callback = stub;
    }

    MBED_ASSERT(_callback);
    return *this;
}

void TaskBase::cancel()
{
    core_util_critical_section_enter();

    if (_queue) {
        _queue->cancel(this);
        _queue = NULL;
    }

    core_util_critical_section_exit();
}

void TaskBase::wait_finished()
{
    // Fast path check for finished
    core_util_critical_section_enter();
    if (finished()) {
        core_util_critical_section_exit();
        return;
    }
    core_util_critical_section_exit();

    rtos::Semaphore sem;

    // If the event is in-flight then wait for it to complete
    core_util_critical_section_enter();
    if (finished()) {
        // This element has been flushed from the queue
        core_util_critical_section_exit();
        return;
    }
    _flush_sem = &sem;
    core_util_critical_section_exit();

    sem.wait();
}

bool TaskBase::ready()
{
    core_util_critical_section_enter();

    bool is_ready = _queue == NULL;

    core_util_critical_section_exit();
    return is_ready;
}

bool TaskBase::finished()
{
    core_util_critical_section_enter();

    bool is_finished = ready() && !_started;

    core_util_critical_section_exit();
    return is_finished;
}

void TaskBase::_post(TaskQueue *queue)
{
    _queue = queue;
}

TaskBase::callback_t TaskBase::_start(uint8_t *buffer, uint32_t size)
{
    // Each call to _start must result in a call to _finish
    // before _start can be called again
    MBED_ASSERT(!_started);
    _started = true;
    _queue = NULL;


    if (size < this->size) {
        // There isn't enough space to copy the event data
        MBED_ASSERT(0);
        return stub;
    }

    if (_copy) {
        _copy((void*)buffer, this->_buffer);
    } else {
        memcpy(buffer, this->_buffer, this->size);
    }
    return _callback;
}

void TaskBase::_finish()
{
    // Each call to _finish must be preceded by a call to _start
    MBED_ASSERT(_started);
    _started = false;
    _wake_check();
}

void TaskBase::_cancel()
{
    _queue = NULL;
    _wake_check();
}

void TaskBase::_wake_check()
{
    if (!finished()) {
        return;
    }
    if (_flush_sem) {
        _flush_sem->release();
        _flush_sem = NULL;
    }
}
