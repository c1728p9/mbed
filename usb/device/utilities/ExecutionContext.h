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

#ifndef MBED_EXECUTION_CONTEXT_H
#define MBED_EXECUTION_CONTEXT_H

#include "Callback.h"

class Runnable;

class ExecutionContext {
public:

    /**
     * Construct a new ExecutionContext object
     */
    ExecutionContext() {};

    /**
     * Cleanup resources used by this ExecutionContext
     */
    virtual ~ExecutionContext() {};

    /**
     * Enter this execution context
     */
    virtual void lock() = 0;

    /**
     * Exit this execution context
     */
    virtual void unlock() = 0;

    /**
     * Run code on this execution context in the future
     *
     * When the runnable is executed the context will be locked.
     *
     * @param runnable Code to run on this execution context
     * @note This function is safe to call from any context including interrupt
     */
    virtual void post(Runnable *runnable) = 0;

    /**
     * Assert that the code is running on this execution context.
     *
     * Code can run on an execution context synchronously by calling
     * ExecutionContext::lock and ExecutionContext::unlock or
     * asynchronously by calling ExecutionContext::post.
     */
    virtual void assert_context() = 0;

protected:

    friend class Runnable;

    /**
     * Cancel the runnable
     *
     * The runnable is guaranteed not to run after this call has returned.
     *
     * @param runnable Runnable to cancel
     * @note This function is only called by Runnable::cancel. If you want to
     *  cancel a runnable then use Runnable::cancel instead.
     * @note This function is always called from a critical section
     */
    virtual void cancel(Runnable *runnable) = 0;

};

#endif
