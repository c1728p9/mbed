/*
 * Copyright (c) 2017-2017 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef BM_TYPES_H_
#define BM_TYPES_H_

#include <stdint.h>
#include <stdbool.h>

#include "cmsis_os2.h"

typedef struct {
    uint32_t count;
    const char *name;
    bool user_mem;
} os_mutex_t;

typedef struct {
    const char *name;
    uint16_t  tokens;
    uint16_t  max_tokens;
    uint8_t  state;
    bool user_mem;
} os_semaphore_t;

typedef struct {
    const char *name;
    uint8_t state;
    uint32_t event_flags;
    bool user_mem;
} os_event_flags_t;
 
typedef void* os_thread_t;
typedef void* os_memory_pool_t;
typedef void* os_message_queue_t;
typedef void* os_message_t;
typedef void* os_timer_t;

#endif
