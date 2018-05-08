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

#ifndef MBED_RUNNABLE_H
#define MBED_RUNNABLE_H

#include "Callback.h"
#include "ExecutionContext.h"
#include "mbed_critical.h"
#include <string.h>

class Runnable {
public:

    /**
     * Construct a new Runnable object
     *
     * @param callback Completion callback
     */
    Runnable(): _context(NULL), _callback(stub)
    {
        memset(_context_data, 0, sizeof(_context_data));
    };

    /**
     * Set the callback for this runnable
     *
     * @param callback Callback to invoke when posted
     */
    Runnable &operator=(mbed::Callback<void()> callback)
    {
        _callback = callback;
        MBED_ASSERT(_callback);
        return *this;
    }

    /**
     * Cancel the execution of this runnable
     *
     * @note cancel must only be called from the context the event was posted to
     */
    void cancel()
    {
        core_util_critical_section_enter();

        ExecutionContext *context = _context;
        if (_context) {
            _context->cancel(this);
            _context = NULL;
        }

        core_util_critical_section_exit();

        if (context) {
            context->assert_context();
        }
    }

    /**
     * Return true if this runnable is finished
     *
     * @return true if this runnable is finished
     */
    bool finished()
    {
        return _context == NULL;
    }

    /* For use by implementations of ExecutionContext only */

    ExecutionContext *_context;
    uint32_t _context_data[4];
    mbed::Callback<void()> _callback;

private:

    static void stub() {};
};

#endif
