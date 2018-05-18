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

#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include "events/TaskBase.h"
#include "platform/Callback.h"
#include "mbed_critical.h"

namespace events {
/** \addtogroup events */



/** TaskQueue
 *
 *  Flexible task queue for dispatching tasks
 * @ingroup events
 */
class TaskQueue {
public:
    /** Create a TaskQueue
     *
     *  Create an event queue.
     */
    TaskQueue()
    {

    }

    /** Destroy a TaskQueue
     */
    virtual ~TaskQueue()
    {

    }

    /**
     * Add this event to the queue for execution
     *
     * If the event is already in the queue then it is canceled and
     * added to the end of the queue.
     *
     *  @param event    Pointer to the event
     */
    virtual void post(TaskBase *event) = 0;

    /** Cancel an in-flight event
     *
     *  Cancels the given event so the event's memory can be reused.
     *
     *  The cancel function is irq safe.
     *
     *  If called while the event queue's dispatch loop is active, the cancel
     *  function does not guarantee that the event will not execute after it
     *  returns, as the event may have already begun executing. It does
     *  guarantee that the event queue is no longer using event data so
     *  the event can be freed or reused.
     *
     *  @param event    Pointer to the event
     */
    virtual void cancel(TaskBase *event) = 0;

protected:

    /**
     * Set the queue this event was posted to
     *
     * This queue is used for calls to event->cancel().
     *
     * @param task The task to inform of the post
     * @param dest The callback to fill with the function to run
     *
     * @note This call must be made in a critical section
     */
    static void task_post(TaskBase *task, TaskQueue *queue)
    {
        task->_post(queue);
    }

    /**
     * Start processing this event by copying out its data
     *
     * Inform this event both that callback execution has started
     * and that the event is free to be posted again.
     *
     * @param task The task to start processing
     * @param dest The callback to fill with the function to run
     *
     * @note event_start must not be called on a canceled event as the
     *                   memory may have been freed already
     * @note Every call to event_start must be paired with event_finish
     * @note This call must be made in a critical section
     */
    static TaskBase::callback_t task_start(TaskBase *task, uint8_t *buffer, uint32_t size)
    {
        return task->_start(buffer, size);
    }

    /**
     * Finish processing this event
     *
     * Inform this event that the callback has run to completion.
     *
     * @param task The task to finish processing
     *
     * @note Every call to event_finish must be preceded by a call to event_start
     * @note This call must be made in a critical section
     */
    static void task_finish(TaskBase *task)
    {
        task->_finish();
    }

    /**
     * Cancel this event
     *
     * Inform this event that it has been canceled. It is the callers
     * responsibility to ensure that event_start is not called on
     * an event after it has been canceled.
     *
     * OK:
     * Thread 1: queue.post(event) -> event_post(event, queue)
     * Thread 2: event_start(event, cb)
     * Thread 1: queue.cancel(event) -> event_cancel(event)     <- Allowed since event is caller supplied
     * Thread 2: event_finish(event)
     *
     * OK:
     * Thread 1: queue.post(event) -> event_post(event, queue)
     * Thread 2: event_start(event)
     * Thread 2: event_finish(event, cb)
     * Thread 1: queue.cancel(event) -> event_cancel(event)     <- Allowed since event is caller supplied
     *
     * ERROR:
     * Thread 1: queue.post(event) -> event_post(event, queue)
     * Thread 1: queue.cancel(event) -> event_cancel(event)
     * Thread 2: event_start(event, cb)                         <- Not allowed, event may have been freed
     * Thread 2: event_finish(event)
     *
     * @param event The event to cancel
     *
     * @note This call must be made in a critical section
     */
    static void task_cancel(TaskBase *event)
    {
        event->_cancel();
    }
};

}
#endif

