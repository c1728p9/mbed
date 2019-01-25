/***************************************************************************//**
* \file cy_crypto_core_ecc_nist_p.h
* \version 2.20
*
* \brief
*  This file provides constant and parameters for the API for the ECC
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


#if !defined(CY_CRYPTO_CORE_NIST_P_H)
#define CY_CRYPTO_CORE_NIST_P_H

#include "cy_crypto_core_ecc.h"
#include "cy_syslib.h"

#if defined(CY_IP_MXCRYPTO)

#if (CPUSS_CRYPTO_VU == 1)

void Cy_Crypto_Core_EC_NistP_SetMode(int bitsize);

/*
 * Select which reduction algorithm has to be used
 * @param: alg one of {CURVE_SPECIFIC_RED_ALG, SHIFT_MUL_RED_ALG, BARRETT_RED_ALG}
 */
void Cy_Crypto_Core_EC_NistP_SetRedAlg(cy_en_crypto_ecc_red_mul_algs_t alg);

/**
 * @brief Elliptic curve point multiplication in GF(p)
 * @param[in] p_x        Register index for affine X coordinate of base point.
 * @param[in] p_y        Register index for affine Y coordinate of base point.
 * @param[in] p_d        Register index for multiplication value.
 * @param[in] p_order    Register index for order value.
 * @param[in] bitsize    Bit size of the used curve.
 */
void Cy_Crypto_Core_EC_NistP_PointMul(CRYPTO_Type *base, int p_x, int p_y, int p_d, int p_order, int bitsize);

cy_en_crypto_status_t Cy_Crypto_Core_EC_NistP_PointMultiplication(CRYPTO_Type *base,
    cy_en_crypto_ecc_curve_id_t curveID,
    uint8_t* ecpGX,
    uint8_t* ecpGY,
    uint8_t* ecpD,
    uint8_t* ecpQX,
    uint8_t* ecpQY);

/// @brief Modular multiplication in GF(VR_P).
///
/// z = a * b % mod
///
/// Leaf function.
///
/// @param[in] z Register index for product value.
/// @param[in] a Register index for multiplicand value.
/// @param[in] b Register index for multiplier value.
/// @param[in] VR_BARRETT Register index for Barrett reduction value.
/// @param[in] mod Register index for modulo value.
/// @param[in] size Bit size.
void Cy_Crypto_Core_EC_MulMod( CRYPTO_Type *base,
    int z,
    int a,
    int b,
    int size);

/// @brief Modular division in GF(VR_P).
///
/// z = a / b % mod
///
/// This algorithm works when "dividend" and "divisor" are relatively prime,
///
/// Reference: "From Euclid's GCD to Montgomery Multiplication to the Great Divide", S.C. Schantz
///
/// @param[in] z Register index for quotient value.
/// @param[in] a Register index for dividend value.
/// @param[in] b Register index for divisor value.
/// @param[in] mod Register index for modulo value.
/// @param[in] size Bit size.
void Cy_Crypto_Core_EC_DivMod( CRYPTO_Type *base,
    int z,
    int a,
    int b,
    int size);

/// @brief Modular squaring in GF(VR_P).
///
/// z = a * a % mod
///
/// @param[in] z Register index for product value.
/// @param[in] a Register index for multiplicand and multiplier value.
/// @param[in] VR_BARRETT Register index for Barrett reduction value.
/// @param[in] mod Register index for modulo value.
/// @param[in] size Bit size.
void Cy_Crypto_Core_EC_SquareMod( CRYPTO_Type *base,
    int z,
    int a,
    int size);

/// @brief Barrett reduction in GF(VR_P).
///
/// z = a_double % mod
///
/// Leaf function.
///
/// @param[in] z Register index for Barrett reduced value.
/// @param[in] a_double Register index for non reduced value.
/// @param[in] VR_BARRETT Register index for Barrett reduction value.
/// @param[in] mod Register index for modulo value.
/// @param[in] size Bit size.

// t[b-1:0] = z_double >> size
// t        = t * VR_BARRETT
// t        = t + ((z_double >> size) << size)         // for leading '1' Barrett bit.
// t        = t >> size
// t        = t * mod                           //r2 (not reduced)
// u        = z_double - t                      //r = r1 - r2 (not reduced)

// u        = IF (u >= mod) u = u - mod         //reduce r using mod
// u        = IF (u >= mod) u = u - mod
void Cy_Crypto_Core_EC_Bar_MulRed(CRYPTO_Type *base,
    int z,
    int x,
    int size
);

/// @brief Modular addition in GF(VR_P).
///
/// z = a + b % mod
///
/// Leaf function.
///
/// @param[in] z Register index for sum value.
/// @param[in] a Register index for augend a value.
/// @param[in] b Register index for addend b value.
/// @param[in] mod Register index for modulo value.
void Cy_Crypto_Core_EC_AddMod( CRYPTO_Type *base, int z, int a, int b);

/// @brief Modular subtraction in GF(VR_P).
///
/// z = a - b % mod
///
/// Leaf function.
///
/// @param[in] z Register index for difference value.
/// @param[in] a Register index for minuend a value.
/// @param[in] b Register index for subtrahend b value.
/// @param[in] mod Register index for modulo value.
void Cy_Crypto_Core_EC_SubMod( CRYPTO_Type *base, int z, int a, int b);

/// @brief Modular halving in GF(VR_P).
///
/// z = a / 2 % mod
///
/// Leaf function.
///
/// @param[in] z Register index for result value.
/// @param[in] a Register index for value to be halved.
/// @param[in] mod Register index for modulo value.
void Cy_Crypto_Core_EC_HalfMod( CRYPTO_Type *base, int z, int a);

#endif /* #if (CPUSS_CRYPTO_VU == 1) */

#endif /* #if defined(CY_IP_MXCRYPTO) */

#endif /* #if !defined(CY_CRYPTO_CORE_NIST_P_H) */


/* [] END OF FILE */

