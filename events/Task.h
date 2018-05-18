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
#ifndef EVENT_H
#define EVENT_H

#include "events/TaskBase.h"
#include "platform/mbed_assert.h"

namespace events {
/** \addtogroup events */

/** Task
 *
 *  Representation of a postable task
 * @ingroup events
 */

//template <typename F>
//class Task;


/** Task
 *
 *  Representation of a postable task
 * @ingroup events
 */
template <typename F>
class Task: public TaskBase {
public:
    /** Create a task
     *
     *  Constructs a task. The specified callback acts as the target
     *  for the task and is executed in the context of the task queue's
     *  dispatch loop once posted.
     *
     *  @param q                Event queue to dispatch on
     *  @param f                Function to execute when the event is dispatched
     */
    Task(F f): TaskBase(reinterpret_cast<uint8_t *>(&_f), sizeof(F), task_thunk),  _f(f) {
        // Nothing to do
    }

private:

    F _f;

    static void task_thunk(void *data) {
        F *callback = (F*)data;
        (*callback)();
    }
};

}

/** @}*/

#endif
