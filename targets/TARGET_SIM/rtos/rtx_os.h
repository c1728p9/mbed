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

#ifndef RTX_OS_H_
#define RTX_OS_H_

#include "cmsis_os2.h"

//  ==== Kernel definitions ====
 
/// Kernel State definitions
#define osRtxKernelInactive             ((uint8_t)osKernelInactive)
#define osRtxKernelReady                ((uint8_t)osKernelReady)
#define osRtxKernelRunning              ((uint8_t)osKernelRunning)
#define osRtxKernelLocked               ((uint8_t)osKernelLocked)
#define osRtxKernelSuspended            ((uint8_t)osKernelSuspended)
 
 
//  ==== Thread definitions ====
 
/// Thread State definitions (extending osThreadState)
#define osRtxThreadStateMask            0x0FU
 
#define osRtxThreadInactive             ((uint8_t)osThreadInactive)
#define osRtxThreadReady                ((uint8_t)osThreadReady)
#define osRtxThreadRunning              ((uint8_t)osThreadRunning)
#define osRtxThreadBlocked              ((uint8_t)osThreadBlocked)
#define osRtxThreadTerminated           ((uint8_t)osThreadTerminated)
 
#define osRtxThreadWaitingDelay         (osRtxThreadBlocked | 0x10U)
#define osRtxThreadWaitingJoin          (osRtxThreadBlocked | 0x20U)
#define osRtxThreadWaitingThreadFlags   (osRtxThreadBlocked | 0x30U) 
#define osRtxThreadWaitingEventFlags    (osRtxThreadBlocked | 0x40U) 
#define osRtxThreadWaitingMutex         (osRtxThreadBlocked | 0x50U)
#define osRtxThreadWaitingSemaphore     (osRtxThreadBlocked | 0x60U)
#define osRtxThreadWaitingMemoryPool    (osRtxThreadBlocked | 0x70U)
#define osRtxThreadWaitingMessageGet    (osRtxThreadBlocked | 0x80U)
#define osRtxThreadWaitingMessagePut    (osRtxThreadBlocked | 0x90U)


//  ==== OS External Functions ====

/// OS Error Codes
#define osRtxErrorStackUnderflow        1U
#define osRtxErrorISRQueueOverflow      2U
#define osRtxErrorTimerQueueOverflow    3U
#define osRtxErrorClibSpace             4U
#define osRtxErrorClibMutex             5U

#endif
