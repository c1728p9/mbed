/***************************************************************************//**
* \file cy_crypto_core_hw.h
* \version 2.20
*
* \brief
*  This file provides the headers to the API for the utils
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


#if !defined(CY_CRYPTO_CORE_VU_H)
#define CY_CRYPTO_CORE_VU_H

#include "cy_crypto_common.h"

#if defined(CY_IP_MXCRYPTO)

#if (CPUSS_CRYPTO_VU == 1)

#include "cy_crypto_core_hw.h"
#include "cy_crypto_core_hw_vu.h"
#include <string.h>

#define CY_CRYPTO_VU_SIZE_FLD_MASK    (0x00001fffuL)
#define CY_CRYPTO_VU_DATA_FLD_MASK    (0x00003fffuL)
#define CY_CRYPTO_VU_DATA_FLD_POS     (16u)

/*******************************************************************************
* Function Name: Cy_Crypto_Core_Vu_SetMemValue
****************************************************************************//**
*
* Sets the value in the crypto memory allocated by destination VU register.
*
* This function available for Server side only.
*
* This function is internal and should not to be called directly by user software
*
* \param base
* The pointer to the CRYPTO instance address.
*
* \param dstReg
* The destination vector unit register.
*
* \param src
* The pointer to the source value.
*
* \param size
* The size of the operated value.
*
*******************************************************************************/
void Cy_Crypto_Core_Vu_SetMemValue(CRYPTO_Type *base, uint32_t dstReg, uint8_t const *src, uint32_t size);

/*******************************************************************************
* Function Name: Cy_Crypto_Core_Vu_GetMemValue
****************************************************************************//**
*
* Gets the value located in the crypto memory and pointed by source VU register.
*
* This function available for Server side only.
*
* This function is internal and should not to be called directly by user software
*
* \param base
* The pointer to the CRYPTO instance address.
*
* \param dst
* The pointer to the destination value.
*
* \param srcReg
* The source vector unit register.
*
* \param size
* The size of the operated value.
*
*******************************************************************************/
void Cy_Crypto_Core_Vu_GetMemValue(CRYPTO_Type *base, uint8_t *dst, uint32_t srcReg, uint32_t size);


/*******************************************************************************
* Function Name: Cy_Crypto_Core_Vu_RegRead
****************************************************************************//**
*
* Returns the data pointed in given register
*
* This function available for Server side only.
*
* This function is internal and should not to be called directly by user software
*
* \param base
* The pointer to the CRYPTO instance address.
*
* \param srcReg
* The source vector unit register.
*
*******************************************************************************/
__STATIC_INLINE uint32_t Cy_Crypto_Core_Vu_RegRead(CRYPTO_Type *base, uint32_t srcReg)
{
    return ((uint32_t)_FLD2VAL(CRYPTO_RF_DATA_DATA32, REG_CRYPTO_VU_RF_DATA(base, srcReg)));
}

/*******************************************************************************
* Function Name: Cy_Crypto_Core_Vu_RegSizeRead
****************************************************************************//**
*
* Returns size of the data pointed in given register,
* it is lower 12Bit of the 32Bit word
*
* This function available for Server side only.
*
* This function is internal and should not to be called directly by user software
*
* \param base
* The pointer to the CRYPTO instance address.
*
* \param srcReg
* The source vector unit register.
*
*******************************************************************************/
__STATIC_INLINE uint16_t Cy_Crypto_Core_Vu_RegSizeRead(CRYPTO_Type *base, uint32_t srcReg)
{
    return ((uint16_t)(_FLD2VAL(CRYPTO_RF_DATA_DATA32, REG_CRYPTO_VU_RF_DATA(base, srcReg)) & CY_CRYPTO_VU_SIZE_FLD_MASK));
}

/*******************************************************************************
* Function Name: Cy_Crypto_Core_Vu_RegBitSizeRead
****************************************************************************//**
*
* Returns size of the data pointed in given register, in Bits.
* It is lower 12Bit of the 32Bit word
*
* This function available for Server side only.
*
* This function is internal and should not to be called directly by user software
*
* \param base
* The pointer to the CRYPTO instance address.
*
* \param srcReg
* The source vector unit register.
*
*******************************************************************************/
__STATIC_INLINE uint16_t Cy_Crypto_Core_Vu_RegBitSizeRead(CRYPTO_Type *base, uint32_t srcReg)
{
    return ((uint16_t)(_FLD2VAL(CRYPTO_RF_DATA_DATA32, REG_CRYPTO_VU_RF_DATA(base, srcReg)) & CY_CRYPTO_VU_SIZE_FLD_MASK) + 1u);
}

/*******************************************************************************
* Function Name: Cy_Crypto_Core_Vu_RegByteSizeRead
****************************************************************************//**
*
* Returns size of the data pointed in given register, in Bytes.
* It is lower 12Bit of the 32Bit word
*
* This function available for Server side only.
*
* This function is internal and should not to be called directly by user software
*
* \param base
* The pointer to the CRYPTO instance address.
*
* \param srcReg
* The source vector unit register.
*
*******************************************************************************/
__STATIC_INLINE uint16_t Cy_Crypto_Core_Vu_RegByteSizeRead(CRYPTO_Type *base, uint32_t srcReg)
{
    return ((uint16_t)((_FLD2VAL(CRYPTO_RF_DATA_DATA32, REG_CRYPTO_VU_RF_DATA(base, srcReg)) & CY_CRYPTO_VU_SIZE_FLD_MASK) + 1u) >> 3u);
}

/*******************************************************************************
* Function Name: Cy_Crypto_Core_Vu_RegWordSizeRead
****************************************************************************//**
*
* Returns size of the data pointed in given register, in words (uint32_t).
* It is lower 12Bit of the 32Bit word
*
* This function available for Server side only.
*
* This function is internal and should not to be called directly by user software
*
* \param base
* The pointer to the CRYPTO instance address.
*
* \param srcReg
* The source vector unit register.
*
*******************************************************************************/
__STATIC_INLINE uint16_t Cy_Crypto_Core_Vu_RegWordSizeRead(CRYPTO_Type *base, uint32_t srcReg)
{
    return ((uint16_t)((_FLD2VAL(CRYPTO_RF_DATA_DATA32, REG_CRYPTO_VU_RF_DATA(base, srcReg)) & CY_CRYPTO_VU_SIZE_FLD_MASK) + 1u) >> 5u);
}

/*******************************************************************************
* Function Name: Cy_Crypto_Core_Vu_RegDataPtrRead
****************************************************************************//**
*
* Returns the data pointer, in 14-bit format of the data pointed in given register,
* it is upper 16Bit of the 32Bir word. Pointer is in words (uint32_t).
*
* This function available for Server side only.
*
* This function is internal and should not to be called directly by user software
*
* \param base
* The pointer to the CRYPTO instance address.
*
* \param srcReg
* The source vector unit register.
*
*******************************************************************************/
__STATIC_INLINE uint16_t Cy_Crypto_Core_Vu_RegDataPtrRead(CRYPTO_Type *base, uint32_t srcReg)
{
    return ((uint16_t)(_FLD2VAL(CRYPTO_RF_DATA_DATA32, REG_CRYPTO_VU_RF_DATA(base, srcReg)) >> CY_CRYPTO_VU_DATA_FLD_POS)
                                & CY_CRYPTO_VU_DATA_FLD_MASK);
}

/*******************************************************************************
* Function Name: Cy_Crypto_Core_Vu_RegMemPointer
****************************************************************************//**
*
* Returns the memory address of the data pointed in given register.
*
* This function available for Server side only.
*
* This function is internal and should not to be called directly by user software
*
* \param base
* The pointer to the CRYPTO instance address.
*
* \param srcReg
* The source vector unit register.
*
*******************************************************************************/
__STATIC_INLINE uint32_t * Cy_Crypto_Core_Vu_RegMemPointer(CRYPTO_Type *base, uint32_t srcReg)
{
    return (uint32_t *)((uint32_t)REG_CRYPTO_MEM_BUFF(base) + 4u * (uint32_t)Cy_Crypto_Core_Vu_RegDataPtrRead(base, srcReg));
}

/*******************************************************************************
* Function Name: Cy_Crypto_Core_Vu_StatusRead
****************************************************************************//**
*
* Returns value of the VU status register
*
* This function available for Server side only.
*
* This function is internal and should not to be called directly by user software
*
* \param base
* The pointer to the CRYPTO instance address.
*
*******************************************************************************/
__STATIC_INLINE uint16_t Cy_Crypto_Core_Vu_StatusRead(CRYPTO_Type *base)
{
    Cy_Crypto_Core_WaitForReady(base);

    return((uint16_t)REG_CRYPTO_VU_STATUS(base));
}

/*******************************************************************************
* Function Name: Cy_Crypto_Wait_Vu_ForComplete
****************************************************************************//**
*
* Waits until VU instruction will be completed
*
* This function available for Server side only.
*
* This function is internal and should not to be called directly by user software
*
* \param base
* The pointer to the CRYPTO instance address.
*
*******************************************************************************/
__STATIC_INLINE void Cy_Crypto_Core_Vu_WaitForComplete(CRYPTO_Type *base)
{
    /* Wait until the VU instruction is complete */
    if (cy_device->cryptoVersion == 1u)
    {
        while (0uL != _FLD2VAL(CRYPTO_STATUS_VU_BUSY, REG_CRYPTO_STATUS(base)))
        {
        }
    }
    else
    {
        while (0uL != _FLD2VAL(CRYPTO_V2_STATUS_BUSY, REG_CRYPTO_STATUS(base)))
        {
        }
    }
}


#endif /* #if (CPUSS_CRYPTO_VU == 1) */

#endif /* CY_IP_MXCRYPTO */

#endif /* #if !defined(CY_CRYPTO_CORE_VU_H) */

/* [] END OF FILE */

