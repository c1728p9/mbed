/* events
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
#ifndef TASK_BASE_H
#define TASK_BASE_H

#include "platform/Callback.h"
#include "platform/mbed_assert.h"
#include "LinkEntry.h"

namespace rtos {
class Semaphore;
}

namespace events {
/** \addtogroup events */


class TaskQueue;

/** TaskBase
 *
 *  Representation of a caller allocated event
 * @ingroup events
 */
class TaskBase : public LinkEntry {
public:

    typedef void (*callback_t)(void *data);
    const uint32_t size;

    /**
     * Construct a new TaskBase object
     *
     * @param callback Completion callback
     */
    TaskBase(uint8_t *data, uint32_t size, callback_t cb=NULL);

    ~TaskBase();

    /**
     * Set the callback for this event
     *
     * @param callback Callback to invoke when posted
     */
    TaskBase &operator=(callback_t callback);

    /**
     * Cancel the execution of this event
     *
     * The callback may still run after the cancel the the memory
     * of this static event will no longer be used. To ensure
     * the callback is finished see wait_finished()
     *
     * @note This function is interrupt safe
     */
    void cancel();

    /**
     * Return true if this event is ready to be posted
     *
     * Check if this event is on a queue waiting to be run.
     *
     * @return true if it is safe to call post
     */

    bool ready();


    void wait_finished();

    /**
     * Check if the callback has run to completion or been fully canceled
     *
     * When an event is finished the queue is completely done with it and the
     * callback is either fully complete or has been canceled and will not run.
     *
     * @return true if this event has been flushed from the queue, false otherwise
     */
    bool finished();

private:

    uint8_t * const _buffer;
    callback_t _callback;

    bool _started;
    TaskQueue *_queue;
    rtos::Semaphore *_flush_sem;

    friend class TaskQueue;

    /*
     * Must be called in a critical section
     */
    void _post(TaskQueue *queue);

    /*
     * Must be called in a critical section
     */
    callback_t _start(uint8_t *buffer, uint32_t size);

    /*
     * Must be called in a critical section
     */
    void _finish();

    /*
     * Must be called in a critical section
     */
    void _cancel();

    void _wake_check();
};

}

#endif

/** @}*/
