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
 * -----------------------------------------------------------------------------
 *
 * Project:     CMSIS-RTOS RTX
 * Title:       RTX Library definitions
 *
 * -----------------------------------------------------------------------------
 */

#ifndef RTX_LIB_H_
#define RTX_LIB_H_

#include <pthread.h>
#include <stdint.h>

#include "cmsis_os2.h"

typedef struct {
  const char *name;
  uint8_t alloc;
  pthread_mutex_t pt_mutex;
  pthread_t pt_owner;
} os_mutex_t;

typedef void* os_semaphore_t;

typedef struct {
  const char *name;
  uint8_t alloc;
  osThreadFunc_t func;
  void *argument;
  pthread_t pt_thread;
} os_thread_t;

typedef void* os_memory_pool_t;
typedef void* os_message_queue_t;
typedef void* os_event_flags_t;
typedef void* os_message_t;
typedef void* os_timer_t;

#endif
