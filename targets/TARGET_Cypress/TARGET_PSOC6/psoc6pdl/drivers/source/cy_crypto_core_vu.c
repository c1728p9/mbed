/***************************************************************************//**
* \file cy_crypto_core_vu.c
* \version 2.20
*
* \brief
*  This file provides the source code to the API for the Vector Unit helpers
*  in the Crypto driver.
*
********************************************************************************
* Copyright 2016-2018 Cypress Semiconductor Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/


#include "cy_crypto_core_vu.h"

#if defined(CY_IP_MXCRYPTO)

#if (CPUSS_CRYPTO_VU == 1)

#include "cy_crypto_core_hw.h"
#include "cy_crypto_core_mem.h"
#include "cy_syslib.h"

#if !defined(CY_CRYPTO_SERVICE_LIBRARY_LEVEL)
    #define CY_CRYPTO_SERVICE_LIBRARY_LEVEL CY_CRYPTO_FULL_LIBRARY
#endif

void Cy_Crypto_Core_Vu_SetMemValue(CRYPTO_Type *base, uint32_t dstReg, uint8_t const *src, uint32_t size)
{
    uint32_t reg0_data = 0uL;
    uint32_t reg1_data = 0uL;

    Cy_Crypto_Core_Vu_WaitForComplete(base);

    if (cy_device->cryptoVersion == 1u)
    {
        CY_CRYPTO_VU_SAVE_REG(base, CY_CRYPTO_VU_HW_REG0, &reg0_data);
        CY_CRYPTO_VU_SAVE_REG(base, CY_CRYPTO_VU_HW_REG1, &reg1_data);
    }

    /* Copy value to Crypto SRAM */
    uint16_t byteSize = (uint16_t)((size / 8u) & 0xFFFFu);
    if ((size % 8u) != 0u)
    {
        ++byteSize;
    }

    CY_ASSERT_L1(size <= Cy_Crypto_Core_Vu_RegBitSizeRead(base, dstReg));

    CY_ASSERT_L1(((uint32_t)Cy_Crypto_Core_Vu_RegMemPointer(base, dstReg) + byteSize - 1) < ((uint32_t)REG_CRYPTO_MEM_BUFF(base) + CY_CRYPTO_MEM_BUFF_SIZE) );
    Cy_Crypto_Core_MemCpy(base, (void*)Cy_Crypto_Core_Vu_RegMemPointer(base, dstReg), (const void*)src, byteSize);

    if (cy_device->cryptoVersion == 1u)
    {
        CY_CRYPTO_VU_RESTORE_REG(base, CY_CRYPTO_VU_HW_REG0, reg0_data);
        CY_CRYPTO_VU_RESTORE_REG(base, CY_CRYPTO_VU_HW_REG1, reg1_data);
    }
}

void Cy_Crypto_Core_Vu_GetMemValue(CRYPTO_Type *base, uint8_t *dst, uint32_t srcReg, uint32_t size)
{
    uint32_t reg0_data = 0uL;
    uint32_t reg1_data = 0uL;

    Cy_Crypto_Core_Vu_WaitForComplete(base);

    if (cy_device->cryptoVersion == 1u)
    {
        CY_CRYPTO_VU_SAVE_REG(base, CY_CRYPTO_VU_HW_REG0, &reg0_data);
        CY_CRYPTO_VU_SAVE_REG(base, CY_CRYPTO_VU_HW_REG1, &reg1_data);
    }

    /* Copy value from Crypto SRAM */
    uint16_t byteSize = (uint16_t)((size / 8u) & 0xFFFFu);
    if ((size % 8u) != 0u)
    {
        ++byteSize;
    }

    CY_ASSERT_L1(size <= Cy_Crypto_Core_Vu_RegBitSizeRead(base, srcReg));
    CY_ASSERT_L1(((uint32_t)Cy_Crypto_Core_Vu_RegMemPointer(base, srcReg) + byteSize - 1) < ((uint32_t)REG_CRYPTO_MEM_BUFF(base) + CY_CRYPTO_MEM_BUFF_SIZE) );

    Cy_Crypto_Core_MemCpy(base, (void*)dst, (void*)Cy_Crypto_Core_Vu_RegMemPointer(base, srcReg), byteSize);

    if (cy_device->cryptoVersion == 1u)
    {
        CY_CRYPTO_VU_RESTORE_REG(base, CY_CRYPTO_VU_HW_REG0, reg0_data);
        CY_CRYPTO_VU_RESTORE_REG(base, CY_CRYPTO_VU_HW_REG1, reg1_data);
    }
}


#endif /* #if (CPUSS_CRYPTO_VU == 1) */

#endif /* CY_IP_MXCRYPTO */


/* [] END OF FILE */

