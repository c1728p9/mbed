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
#include "critical.h"

int32_t flash_init(flash_t *obj)
{
    status_t ret;

    ret = FLASH_Init(&obj->flash);
    if (ret != kStatus_FLASH_Success) {
        return -1;
    }
    ret = FLASH_PrepareExecuteInRamFunctions(&obj->flash);
    if (ret != kStatus_FLASH_Success) {
        return -1;
    }

    return 0;
}

int32_t flash_free(flash_t *obj)
{
    return 0;
}

int32_t flash_erase_sector(flash_t *obj, uint32_t address)
{
    status_t ret;

    // erase sector boundary
    if ((address % obj->flash.PFlashSectorSize) != 0) {
        return -1;
    }

    core_util_critical_section_enter();
    ret = FLASH_Erase(&obj->flash, address, obj->flash.PFlashSectorSize, 0);//kFLASH_apiEraseKey);
    core_util_critical_section_exit();
    if (ret != kStatus_FLASH_Success) {
        return -1;
    }

    return 0;
}


int32_t flash_program_page(flash_t *obj, uint32_t address, const uint8_t *data, uint32_t size)
{ 
    status_t ret;

    // write size boundary
    if ((address % obj->flash.PFlashSectorSize) != 0) {
        return -1;
    }

    //TODO - allow more flexibility here
    if (size != obj->flash.PFlashSectorSize) {
        return -1;
    }

    core_util_critical_section_enter();
    ret = FLASH_Program(&obj->flash, address, (uint32_t*)data, size);
    core_util_critical_section_exit();
    if (ret != kStatus_FLASH_Success) {
        return -1;
    }

    return 0;
}


uint32_t flash_get_sector_size(flash_t *obj, uint32_t address)
{
    return obj->flash.PFlashSectorSize;
}

uint32_t flash_get_page_size(flash_t *obj)
{
    return obj->flash.PFlashSectorSize;
}

