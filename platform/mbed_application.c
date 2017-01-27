/* mbed Microcontroller Library
 * Copyright (c) 2017-2017 ARM Limited
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
#include <stdlib.h>
#include <stdarg.h>
#include "device.h"
#include "platform/mbed_application.h"
#include "platform/mbed_assert.h"
#include "platform/toolchain.h"

//TODOS
//-Modify linker file for various sizes
//x-modify add ram region to linker file
//-get application to start from bootloader app
//-Hook into system init or something just as early
//-Add a test for the new attributes

//-Do this for IAR
//-Do this for ARMCC


#define VALID_KEY   0x91385532

typedef struct {
    uint32_t key;
    uint32_t size;
    uint32_t image_start;
    uint32_t checksum;
} ram_options_t;

MBED_SECTION("no_init") volatile ram_options_t ram_options;

static void start_new_application(void *sp, void *pc);
static uint32_t compute_checksum(volatile ram_options_t *options);
void mbed_application_boot_check();

void mbed_application_start(uint32_t address)
{
    MBED_ASSERT(address % 4 == 0);
    ram_options.size = sizeof(ram_options_t);
    ram_options.image_start = address;
    ram_options.key = VALID_KEY;
    ram_options.checksum = compute_checksum(&ram_options);
    NVIC_SystemReset();
}

void mbed_application_boot_check()
{
    void *sp;
    void *pc;
    if (ram_options.key != VALID_KEY) {
        // TODO - fill with valid data?
        return;
    }

    if (ram_options.checksum != compute_checksum(&ram_options)) {
        return;
    }

    sp = *((void**)ram_options.image_start + 0);
    pc = *((void**)ram_options.image_start + 1);
    SCB->VTOR = ram_options.image_start;
    ram_options.key = 0;
    //TODO - enable interrupts?
    start_new_application(sp, pc);
}

static uint32_t compute_checksum(volatile ram_options_t *options)
{
    //TODO
    return 0;
}


#if defined (__CC_ARM)

__asm static void start_new_application(void *sp, void *pc)
{
    MOVS SP, R0
    B R1
}

#elif defined (__GNUC__)


__attribute__((naked)) void start_new_application(void *sp, void *pc)
{
    __asm (
        "mov sp,r0\n"
        "b    r1\n"
    );
}

#elif defined (__ICCARM__)

//TODO

#endif
