/***************************************************************************//**
* \file cy_crypto_core_ecc_ecdsa.c
* \version 2.20
*
* \brief
*  This file provides constant and parameters for the API for the ECC ECDSA
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


#include "cy_crypto_core_ecc.h"
#include "cy_crypto_core_ecc_nist_p.h"
#include "cy_crypto_core_mem.h"
#include "cy_crypto_core_vu.h"

#include <stdbool.h>

#define ECC_ECDSA_DEBUG 0

#if ECC_ECDSA_DEBUG
#include "cy_crypto_core_my_support.h"
#ifndef Tb_PrintStr
#include <stdio.h>
#define Tb_PrintStr(s)      printf("%s\n", s)
#endif
#endif

//////////////////////////////////////////////////////////////////////////////
// returns TRUE if rsrc0 contains 0. FALSE otherwise
bool test_zero(CRYPTO_Type *base, int rsrc0)
{
    bool result;
    uint16_t status;

    CY_CRYPTO_VU_TST(base, rsrc0);
    Cy_Crypto_Core_WaitForReady(base);

    status = Cy_Crypto_Core_Vu_StatusRead(base);
    if (status & (1 << CY_CRYPTO_VU_STATUS_ZERO))
        result = true;
    else
        result = false;

    return result;
}

// returns TRUE if rsrc0 contains the same value as rsrc1. FALSE otherwise
bool test_equal(CRYPTO_Type *base, int rsrc0, int rsrc1)
{
    bool result;
    uint16_t status;

    CY_CRYPTO_VU_CMP_SUB (base, rsrc1, rsrc0);                    // C = (a >= b)
    Cy_Crypto_Core_WaitForReady(base);
    status = Cy_Crypto_Core_Vu_StatusRead(base);

    if (status &  (1 << CY_CRYPTO_VU_STATUS_ZERO))
        result = true;
    else
        result = false;

    return result;
}

// returns TRUE if rsrc0 contains the value less than value of rsrc1. FALSE otherwise
bool test_less_than(CRYPTO_Type *base, int rsrc0, int rsrc1)
{
    bool result;
    uint16_t status;

    CY_CRYPTO_VU_CMP_SUB (base, rsrc1, rsrc0);                    // C = (a >= b)
    Cy_Crypto_Core_WaitForReady(base);

    status = Cy_Crypto_Core_Vu_StatusRead(base);
    if (status &  (1 << CY_CRYPTO_VU_STATUS_CARRY))
        result = true;
    else
        result = false;

    return result;
}

//////////////////////////////////////////////////////////////////////////////
/**
  Sign a message digest
  @param hash      The message digest to sign
  @param hashlen   The length of the digest in bytes
  @param sig       [out] The destination for the signature, 'r' followed by 's'
  @param key       Key used for signature generation
*/
cy_en_crypto_status_t Cy_Crypto_Core_ECC_SignHash(CRYPTO_Type *base, const uint8_t *hash, uint32_t hashlen, uint8_t *sig,
        cy_stc_crypto_ecc_key *key, uint8_t *messageKey)
{
    cy_stc_crypto_ecc_key ephKey;
    uint8_t myKGX[CY_CRYPTO_ECC_MAX_BYTE_SIZE];
    uint8_t myKGY[CY_CRYPTO_ECC_MAX_BYTE_SIZE];

    const cy_stc_crypto_ecc_dp_type *eccDp = Cy_Crypto_Core_ECC_GetCurveParams(key->curveID);

    if (hash == NULL || sig == NULL || key == NULL || messageKey == NULL || eccDp == NULL)
    {
        /* NULL parameter detected in ecc_sign_hash()!!! */
        return CY_CRYPTO_NOT_SUPPORTED;
    }

#if ECC_ECDSA_DEBUG
    Tb_PrintStr("\necc_sign_hash() for ");
    Tb_PrintStr(eccDp->name);
    Tb_PrintStr("\n");
#endif  // ECC_ECDSA_DEBUG

    uint16_t status;
    uint32_t bitsize = eccDp->size;

    /* make ephemeral key pair */
    ephKey.pubkey.x = myKGX;
    ephKey.pubkey.y = myKGY;

    if (Cy_Crypto_Core_ECC_MakePublicKey(base, key->curveID, messageKey, &ephKey) != CY_CRYPTO_SUCCESS)
    {
        return CY_CRYPTO_NOT_SUPPORTED;
    }

#if ECC_ECDSA_DEBUG
    Tb_PrintStr("%10s: \t", "hash");
    Crypto_PrintNumber(hash, CY_CRYPTO_BYTE_SIZE_OF_BITS(bitsize));

    Tb_PrintStr("%10s: \t", "k");
    Crypto_PrintNumber(messageKey, CY_CRYPTO_BYTE_SIZE_OF_BITS(bitsize));

    Tb_PrintStr("%10s: \t", "x1");
    Crypto_PrintNumber((uint8_t *)ephKey.pubkey.x, CY_CRYPTO_BYTE_SIZE_OF_BITS(bitsize));
#endif // ECC_ECDSA_DEBUG

    //-----------------------------------------------------------------------------
    // load values needed for reduction modulo order of the base point
    CY_CRYPTO_VU_ALLOC_MEM (base, VR_P, bitsize);
    Cy_Crypto_Core_Vu_SetMemValue (base, VR_P, (uint8_t *)eccDp->order, bitsize);

    CY_CRYPTO_VU_ALLOC_MEM (base, VR_BARRETT, bitsize + 1);
    Cy_Crypto_Core_Vu_SetMemValue (base, VR_BARRETT, (uint8_t *)eccDp->barrett_o, bitsize);

    //-----------------------------------------------------------------------------
    // check if x1 is zero
    int p_temp = 8;    // temporal values
    int p_r = 9;       // x1 / r
    int p_s = 10;      // s
    int p_d = 11;      //

    CY_CRYPTO_VU_ALLOC_MEM (base, p_r, bitsize);
    CY_CRYPTO_VU_ALLOC_MEM (base, p_s, bitsize);
    CY_CRYPTO_VU_ALLOC_MEM (base, p_d, bitsize);
    Cy_Crypto_Core_Vu_SetMemValue (base, p_r, (uint8_t *)ephKey.pubkey.x, bitsize);

    if (test_zero(base, p_r))
    {
        /* r is zero!!! */
        return CY_CRYPTO_HW_ERROR;
    }

    // check that x1 is smaller than the order of the base point
    CY_CRYPTO_VU_CMP_SUB (base, p_r, VR_P);                    // C = (a >= b)
    Cy_Crypto_Core_WaitForReady(base);

    status = Cy_Crypto_Core_Vu_StatusRead(base);
    if (status & CY_CRYPTO_VU_STATUS_CARRY_BIT)
    {
        // x1 >= order, needs reduction
        CY_CRYPTO_VU_ALLOC_MEM (base, p_temp, bitsize);
        CY_CRYPTO_VU_MOV(base, p_temp, p_r);

        // z = x % mod
        Cy_Crypto_Core_EC_Bar_MulRed(base, p_r, p_temp, bitsize);
        CY_CRYPTO_VU_FREE_MEM(base, CY_CRYPTO_VU_REG_BIT(p_temp));

        // r = x1 mod n
        Cy_Crypto_Core_Vu_GetMemValue (base, sig, p_r, bitsize);

#if ECC_ECDSA_DEBUG
        Tb_PrintStr("x1 after reduction modulo order: ");
        Crypto_RegMemNumberPrint(p_r);
#endif // ECC_ECDSA_DEBUG

        if (test_zero(base, p_r))
        {
            /* R is zero!!! */
            return CY_CRYPTO_HW_ERROR;
        }
    }
    else
    {
        // carry is clear, i. e. x1 < order
        // r = x1
        Cy_Crypto_Core_MemCpy(base, sig, ephKey.pubkey.x, (uint16_t)CY_CRYPTO_BYTE_SIZE_OF_BITS(bitsize));
    }

    //-----------------------------------------------------------------------------
    // find s = (e + d*r)/k

    // load signing private key
    Cy_Crypto_Core_Vu_SetMemValue (base, p_d, (uint8_t *)key->k, bitsize);

    // use barrett reduction algorithm for operations modulo n (order of the base point)
    Cy_Crypto_Core_EC_NistP_SetRedAlg(eccDp->algo);

    // d*r mod n
    Cy_Crypto_Core_EC_MulMod(base, p_s, p_d, p_r, bitsize);    // z = a * b % mod

    // load message hash, truncate it if needed
    CY_CRYPTO_VU_SET_TO_ZERO(base, p_d);

    if ((hashlen * 8u) > bitsize)
    {
        Cy_Crypto_Core_Vu_SetMemValue (base, p_d, (uint8_t *)(&hash[hashlen - CY_CRYPTO_BYTE_SIZE_OF_BITS(bitsize)]), bitsize);
    }
    else
    {
        Cy_Crypto_Core_Vu_SetMemValue (base, p_d, (uint8_t *)hash, hashlen * 8);
    }

    Cy_Crypto_Core_Vu_SetMemValue (base, p_r, messageKey, bitsize);

    // e + d*r mod n
    Cy_Crypto_Core_EC_AddMod (base, p_s, p_d, p_s);      // z = a + b % mod

    // (e + d*r)/k mod n
    int dividend = 0;   // for whatever reason Crypto_EC_DivMod only works if dividend is in register 0
    CY_CRYPTO_VU_ALLOC_MEM (base, dividend, bitsize);
    CY_CRYPTO_VU_MOV(base, dividend, p_s);

    Cy_Crypto_Core_EC_DivMod(base, p_s, dividend, p_r, bitsize);    // z = a / b % mod

    if (test_zero(base, p_s))
    {
        /* S is zero!!! */
        return CY_CRYPTO_HW_ERROR;
    }

    Cy_Crypto_Core_Vu_GetMemValue (base, &sig[CY_CRYPTO_BYTE_SIZE_OF_BITS(bitsize)], p_s, bitsize);

#if ECC_ECDSA_DEBUG
    Crypto_PrintRegister(p_r, "k");
    Crypto_PrintRegister(p_s, "(e+d*r)/k");
#endif // ECC_ECDSA_DEBUG

    //-----------------------------------------------------------------------------
    CY_CRYPTO_VU_FREE_MEM(base, CY_CRYPTO_VU_REG_BIT(p_r) | CY_CRYPTO_VU_REG_BIT(p_d) | CY_CRYPTO_VU_REG_BIT(p_s) |
                                CY_CRYPTO_VU_REG_BIT(VR_P) | CY_CRYPTO_VU_REG_BIT(VR_BARRETT) |
                                CY_CRYPTO_VU_REG_BIT(dividend));

    return CY_CRYPTO_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
/**
  Verify an ECC signature
   @param sig         The signature to verify, 'R' followed by 'S'
   @param hash        The hash (message digest) that was signed
   @param hashlen     The length of the hash (octets)
   @param stat        Result of signature, 1==valid, 0==invalid
   @param key         The corresponding public ECC key
*/
cy_en_crypto_status_t Cy_Crypto_Core_ECC_VerifyHash(CRYPTO_Type *base,
                            const uint8_t *sig, const uint8_t *hash, uint32_t hashlen,
                            uint8_t *stat, cy_stc_crypto_ecc_key *key)
{
    const cy_stc_crypto_ecc_dp_type *eccDp = Cy_Crypto_Core_ECC_GetCurveParams(key->curveID);

    if (sig == NULL || hash == NULL || stat == NULL || key == NULL || eccDp == NULL)
    {
        /* NULL parameter detected in ecc_sign_hash()!!! */
        return CY_CRYPTO_NOT_SUPPORTED;
    }

#if ECC_ECDSA_DEBUG
    Tb_PrintStr("\necc_verify_hash() for ");
    Tb_PrintStr((char *)eccDp->name);
    Tb_PrintStr("\n");
#endif  // ECC_ECDSA_DEBUG

    uint32_t bitsize = eccDp->size;
    *stat = 0;

    //-----------------------------------------------------------------------------
    int dividend = 0;   // for whatever reason Crypto_EC_DivMod only works if dividend is in register 0
    int p_r  = 4;
    int p_s  = 5;
    int p_u1 = 6;
    int p_u2 = 7;
    int p_o  = 8;
    int p_gx = 9;
    int p_gy = 10;
    int p_qx = 11;
    int p_qy = 12;

    CY_CRYPTO_VU_ALLOC_MEM (base, dividend, bitsize);
    CY_CRYPTO_VU_ALLOC_MEM (base, p_r,  bitsize);
    CY_CRYPTO_VU_ALLOC_MEM (base, p_s,  bitsize);
    CY_CRYPTO_VU_ALLOC_MEM (base, p_u1, bitsize);
    CY_CRYPTO_VU_ALLOC_MEM (base, p_u2, bitsize);
    CY_CRYPTO_VU_ALLOC_MEM (base, p_o,  bitsize);
    CY_CRYPTO_VU_ALLOC_MEM (base, p_gx, bitsize);
    CY_CRYPTO_VU_ALLOC_MEM (base, p_gy, bitsize);
    CY_CRYPTO_VU_ALLOC_MEM (base, p_qx, bitsize);
    CY_CRYPTO_VU_ALLOC_MEM (base, p_qy, bitsize);

    //-----------------------------------------------------------------------------
    // load values needed for reduction modulo order of the base point
    CY_CRYPTO_VU_ALLOC_MEM (base, VR_P, bitsize);
    Cy_Crypto_Core_Vu_SetMemValue (base, VR_P, (uint8_t *)eccDp->order, bitsize);

    CY_CRYPTO_VU_ALLOC_MEM (base, VR_BARRETT, bitsize + 1);
    Cy_Crypto_Core_Vu_SetMemValue (base, VR_BARRETT, (uint8_t *)eccDp->barrett_o, bitsize + 1);

    // use barrett reduction algorithm for operations modulo n (order of the base point)
     Cy_Crypto_Core_EC_NistP_SetRedAlg(eccDp->algo);
     Cy_Crypto_Core_EC_NistP_SetMode(bitsize);

    //-----------------------------------------------------------------------------
    // check that R and S are within the valid range, i.e. 0 < R < n and 0 < S < n
    Cy_Crypto_Core_Vu_SetMemValue (base, p_r, (uint8_t *)sig, bitsize);
    Cy_Crypto_Core_Vu_SetMemValue (base, p_s, (uint8_t *)&sig[CY_CRYPTO_BYTE_SIZE_OF_BITS(bitsize)], bitsize);

    if (test_zero(base, p_r))
    {
        /* R is zero!!! */
        return CY_CRYPTO_NOT_SUPPORTED;
    }
    if (!test_less_than(base, p_r, VR_P))
    {
        /* R is not less than n!!! */
        return CY_CRYPTO_NOT_SUPPORTED;
    }
    if (test_zero(base, p_s))
    {
        /* S is zero!!! */
        return CY_CRYPTO_NOT_SUPPORTED;
    }
    if (!test_less_than(base, p_s, VR_P))
    {
        /* S is not less than n!!! */
        return CY_CRYPTO_NOT_SUPPORTED;
    }

    // load message hash, truncate it if needed
    CY_CRYPTO_VU_SET_TO_ZERO(base, p_u1);
    if ((hashlen * 8u) > bitsize)
    {
        Cy_Crypto_Core_Vu_SetMemValue (base, p_u1, (uint8_t *)(&hash[hashlen - CY_CRYPTO_BYTE_SIZE_OF_BITS(bitsize)]), bitsize);
    }
    else
    {
        Cy_Crypto_Core_Vu_SetMemValue (base, p_u1, (uint8_t *)hash, hashlen * 8);
    }

#if ECC_ECDSA_DEBUG
    Crypto_PrintRegister(p_u1, "hash");
    Crypto_PrintRegister(p_r,  "R");
    Crypto_PrintRegister(p_s,  "S");
    Tb_PrintStr("\n");
#endif // ECC_ECDSA_DEBUG

    // w = s^-1 mod n
    CY_CRYPTO_VU_SET_TO_ONE(base, dividend);
    Cy_Crypto_Core_EC_DivMod(base, p_s, dividend, p_s, bitsize);

#if ECC_ECDSA_DEBUG
     Crypto_PrintRegister(p_s, "w");
#endif // ECC_ECDSA_DEBUG

    // u1 = e*w mod n
    Cy_Crypto_Core_EC_MulMod(base, p_u1, p_u1, p_s, bitsize);
    // u2 = r*w mod n
    Cy_Crypto_Core_EC_MulMod(base, p_u2, p_r, p_s, bitsize);

#if ECC_ECDSA_DEBUG
    Crypto_PrintRegister(p_u1, "u1");
    Crypto_PrintRegister(p_u2, "u2");
#endif // ECC_ECDSA_DEBUG

    //-----------------------------------------------------------------------------
    // Initialize point multiplication
    //
    // load prime, order and barrett coefficient
    Cy_Crypto_Core_Vu_SetMemValue (base, VR_P,       (uint8_t *)eccDp->prime, bitsize);
    Cy_Crypto_Core_Vu_SetMemValue (base, p_o,        (uint8_t *)eccDp->order, bitsize);
    Cy_Crypto_Core_Vu_SetMemValue (base, VR_BARRETT, (uint8_t *)eccDp->barrett_p, bitsize + 1);

    // load base Point G
    Cy_Crypto_Core_Vu_SetMemValue (base, p_gx, (uint8_t *)eccDp->Gx, bitsize);
    Cy_Crypto_Core_Vu_SetMemValue (base, p_gy, (uint8_t *)eccDp->Gy, bitsize);

    // load public key Qa
    Cy_Crypto_Core_Vu_SetMemValue (base, p_qx, (uint8_t *)key->pubkey.x, bitsize);
    Cy_Crypto_Core_Vu_SetMemValue (base, p_qy, (uint8_t *)key->pubkey.y, bitsize);

    // u1 * G
    Cy_Crypto_Core_EC_NistP_PointMul(base, p_gx, p_gy, p_u1, p_o, bitsize);

    // reload order since p_o is modified by Crypto_EC_JacobianEcScalarMul_coZ (!!!!)
    Cy_Crypto_Core_Vu_SetMemValue (base, p_o, (uint8_t *)eccDp->order, bitsize);

    // u2 * Qa
    Cy_Crypto_Core_EC_NistP_PointMul(base, p_qx, p_qy, p_u2, p_o, bitsize);

#if ECC_ECDSA_DEBUG
    Tb_PrintStr("u1 * G: \n");
    Crypto_PrintRegister(p_gx, "  u1*G(x)");
    Crypto_PrintRegister(p_gy, "  u1*G(y)");
    Tb_PrintStr("u2 * Q: \n");
    Crypto_PrintRegister(p_qx, "  u2*Q(x)");
    Crypto_PrintRegister(p_qy, "  u2*Q(x)");
#endif // ECC_ECDSA_DEBUG

    //-----------------------------------------------------------------------------
    // P = u1 * G + u2 * Qa. Only Px is needed
    Cy_Crypto_Core_EC_SubMod(base, dividend, p_qy, p_gy);           // (y2-y1)
    Cy_Crypto_Core_EC_SubMod(base, p_s, p_qx, p_gx);                // (x2-x1)
    Cy_Crypto_Core_EC_DivMod(base, p_s, dividend, p_s, bitsize);    // s = (y2-y1)/(x2-x1)

#if ECC_ECDSA_DEBUG
    Tb_PrintStr("Point addition: \n");
    Crypto_PrintRegister(p_s, "s");
#endif // ECC_ECDSA_DEBUG

    Cy_Crypto_Core_EC_SquareMod(base, p_s, p_s, bitsize);     // s^2
    Cy_Crypto_Core_EC_SubMod(base, p_s, p_s, p_gx);           // s^2 - x1
    Cy_Crypto_Core_EC_SubMod(base, p_s, p_s, p_qx);           // s^2 - x1 - x2 which is Px mod n

#if ECC_ECDSA_DEBUG
    Crypto_PrintRegister(p_s, "px");
#endif // ECC_ECDSA_DEBUG

    if (test_equal(base, p_s, p_r))
    {
        *stat = 1;
    }
    else
    {
        *stat = 0;
    }

    //-----------------------------------------------------------------------------
    CY_CRYPTO_VU_FREE_MEM(base,
            CY_CRYPTO_VU_REG_BIT(dividend) | CY_CRYPTO_VU_REG_BIT(p_r) | CY_CRYPTO_VU_REG_BIT(p_s) |
            CY_CRYPTO_VU_REG_BIT(p_u1) | CY_CRYPTO_VU_REG_BIT(p_u2)    | CY_CRYPTO_VU_REG_BIT(p_o) |
            CY_CRYPTO_VU_REG_BIT(p_gx) | CY_CRYPTO_VU_REG_BIT(p_gy) |
            CY_CRYPTO_VU_REG_BIT(p_qx) | CY_CRYPTO_VU_REG_BIT(p_qy) |
            CY_CRYPTO_VU_REG_BIT(VR_P) | CY_CRYPTO_VU_REG_BIT(VR_BARRETT));

    return CY_CRYPTO_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
