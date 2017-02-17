/* mbed Microcontroller Library
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

#include "flash_api.h"
#include "flash_data.h"
#include "critical.h"

#define MBED_FLASH_ALGO_ERASE   1UL
#define MBED_FLASH_ALGO_PROGRAM 2UL

extern uint32_t SystemCoreClock;

/*
    This binary blob (thumb code) sets r9 (static base) as the code we are jumping to
    is PIC (position independent code).

    These are the instructions (r0 is a pointer to arg_t):
        push {r5, lr, r4}
        mov r5, r9
        push {r5}
        ldr r5, [r0, #20]
        ldr r3, [r0, #16]
        mov r9, r3
        ldr r3, [r0, #12]
        ldr r2, [r0, #8]
        ldr r1, [r0, #4]
        ldr r0, [r0, #0]
        blx r5
        pop {r5}
        mov r9, r5
        pop {r4-r5, pc}
        bx r14
*/
static uint32_t jump_to_flash_algo[] = {
    0x464DB530,
    0x6945B420,
    0x46996903,
    0x688268C3,
    0x68006841,
    0xBC2047A8,
    0xBD3046A9
};

static uint32_t get_sector_index(flash_t *obj, uint32_t address)
{
    // check where address belongs to
    size_t sector_index = 0;
    size_t sectors_count = sizeof(obj->target_config->sectors) / sizeof(sector_info_t);
    for (; sector_index < sectors_count; sector_index++) {
        if ((address > obj->target_config->sectors[sector_index].start) && 
            (address < (obj->target_config->sectors[sector_index].start +obj->target_config->sectors[sector_index].size))) {
            break;
        }
    }
    return sector_index;
}

// should be called within critical section
static int32_t flash_algo_init(flash_t *obj, uint32_t address, uint32_t function)
{
    args_t arguments = {
        .r0 = address,
        .r1 = SystemCoreClock,
        .r2 = function,
        .r3 = 0,
        .r9 = (uint32_t)obj->flash_algo->algo_blob + obj->flash_algo->static_base,
        .pc = (uint32_t)obj->flash_algo->algo_blob + obj->flash_algo->init
    };
    return ((flash_algo_jump_t)(((uint32_t)&jump_to_flash_algo) | 1))(&arguments);
}

// should be called within critical section
static int32_t flash_algo_uninit(flash_t *obj, uint32_t address, uint32_t function)
{
    args_t arguments = {
        .r0 = address,
        .r1 = SystemCoreClock,
        .r2 = function,
        .r3 = 0,
        .r9 = (uint32_t)obj->flash_algo->algo_blob + obj->flash_algo->static_base,
        .pc = (uint32_t)obj->flash_algo->algo_blob + obj->flash_algo->uninit
    };
    return ((flash_algo_jump_t)(((uint32_t)&jump_to_flash_algo) | 1))(&arguments);
}


int32_t flash_init(flash_t *obj)
{
    flash_set_target_config(obj);
    return 0;
}

int32_t flash_free(flash_t *obj)
{
    return 0;
}

int32_t flash_erase_sector(flash_t *obj, uint32_t address)
{
    size_t sector_index = get_sector_index(obj, address);

    // erase sector boundary
    if ((address % obj->target_config->sectors[sector_index].size) != 0) {
        return -1;
    }

    core_util_critical_section_enter();
    flash_algo_init(obj, address, MBED_FLASH_ALGO_ERASE);

    args_t arguments = {
        .r0 = address,
        .r1 = 0,
        .r2 = 0,
        .r3 = 0,
        .r9 = (uint32_t)obj->flash_algo->algo_blob + obj->flash_algo->static_base,
        .pc = (uint32_t)obj->flash_algo->algo_blob + obj->flash_algo->erase_sector
    };
    int32_t ret = ((flash_algo_jump_t)(((uint32_t)&jump_to_flash_algo) | 1))(&arguments);

    flash_algo_uninit(obj, address, MBED_FLASH_ALGO_ERASE);
    core_util_critical_section_exit();
    return ret ? -1 : 0;
}


int32_t flash_program_page(flash_t *obj, uint32_t address, const uint8_t *data, uint32_t size)
{ 
    // write size boundary
    if ((address % obj->target_config->page_size) != 0) {
        return -1; // TODO return types
    }
    //  size multiple of sectors
    if ((size < obj->target_config->page_size) || (size % obj->target_config->page_size != 0)) {
        return -1;
    }

    uint32_t sector_index = get_sector_index(obj, address);
    // should not cross sector boundary
    if (((address % obj->target_config->sectors[sector_index].size) + size) > obj->target_config->sectors[sector_index].size) {
        return -1;
    }

    core_util_critical_section_enter();
    flash_algo_init(obj, address, MBED_FLASH_ALGO_PROGRAM);

    args_t arguments = {
        .r0 = address,
        .r1 = size,
        .r2 = (uint32_t)data,
        .r3 = 0,
        .r9 = (uint32_t)obj->flash_algo->algo_blob + obj->flash_algo->static_base,
        .pc = (uint32_t)obj->flash_algo->algo_blob + obj->flash_algo->program_page
    };
    int32_t ret = ((flash_algo_jump_t)(((uint32_t)&jump_to_flash_algo) | 1))(&arguments);

    flash_algo_uninit(obj, address, MBED_FLASH_ALGO_PROGRAM);
    core_util_critical_section_exit();
    return ret ? -1 : 0;
}


uint32_t flash_get_sector_size(flash_t *obj, uint32_t address)
{
    uint32_t sector_index = get_sector_index(obj, address);
    return obj->target_config->sectors[sector_index].size;
}

uint32_t flash_get_page_size(flash_t *obj)
{
    return obj->target_config->page_size;
}

