/***************************************************************************//**
* \file cy_capsense_lib.h
* \version 1.1
*
* \brief
* The file contains application public interface to the CapSense library.
*
********************************************************************************
* \copyright
* Copyright 2018-2019 Cypress Semiconductor Corporation
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
*******************************************************************************/

#if !defined(CY_CAPSENSE_LIB_H)
#define CY_CAPSENSE_LIB_H

#include "cy_syslib.h"

#if defined(__cplusplus)
extern "C" {
#endif


/*******************************************************************************
* Public definitions
*******************************************************************************/

/******************************************************************************/
/** \addtogroup group_capsense_macros_touch *//** \{ */
/******************************************************************************/
/** No touch detected */
#define CY_CAPSENSE_ADVANCED_CENTROID_NO_TOUCHES                        (0x00u)
/** Multiple touches detected */
#define CY_CAPSENSE_ADVANCED_CENTROID_POSITION_ERROR                    (0xFFu)
/** \} */


/*******************************************************************************
* Structures
*******************************************************************************/

/******************************************************************************/
/** \addtogroup group_capsense_structures *//** \{ */
/******************************************************************************/

/** Declares Adaptive Filter configuration parameters */
typedef struct
{
    uint8_t maxK;                                   /**< Maximum filter coefficient */
    uint8_t minK;                                   /**< Minimum filter coefficient */
    uint8_t noMovTh;                                /**< No-movement threshold */
    uint8_t littleMovTh;                            /**< Little movement threshold */
    uint8_t largeMovTh;                             /**< Large movement threshold */
    uint8_t divVal;                                 /**< Divisor value */
    uint8_t reserved0;                              /**< Reserved field */
    uint8_t reserved1;                              /**< Reserved field */
} cy_stc_capsense_adaptive_filter_config_t;

/** Declares Advanced Centroid configuration parameters */
typedef struct
{
    uint16_t fingerTh;                              /**< Finger threshold of widget */
    uint16_t penultimateTh;                         /**< Penultimate threshold */
    uint16_t virtualSnsTh;                          /**< Virtual sensor threshold */
    uint16_t resolutionX;                           /**< X axis maximum position */
    uint16_t resolutionY;                           /**< Y axis maximum position */
    uint8_t crossCouplingTh;                        /**< Cross-coupling threshold */
    uint8_t snsCountX;                              /**< Number of segments on X axis */
    uint8_t snsCountY;                              /**< Number of segments on Y axis */
    uint8_t edgeCorrectionEn;                       /**< Edge correction enabled */
    uint8_t twoFingersEn;                           /**< Two-finger detection enabled */
} cy_stc_capsense_advanced_centroid_config_t;

/** Declares Advanced Centroid position structure */
typedef struct
{
    uint16_t x;                                     /**< X position */
    uint16_t y;                                     /**< Y position */
    uint16_t z;                                     /**< Z value of X axis */
    uint16_t id;                                    /**< Z value of Y axis */
} cy_stc_capsense_position_t;

/** Declares Advanced Centroid touch structure */
typedef struct
{
    cy_stc_capsense_position_t * ptrPosition;       /**< Array of position structure */
    uint8_t numPosition;                            /**< Number of touches */
} cy_stc_capsense_touch_t;

/** Declares HW SmartSense data structure for CSD widgets */
typedef struct
{
    uint32_t sensorCap;                             /**< Sensor parasitic capacitance in fF 10^-15 */
    uint32_t iDacGain;                              /**< IDAC gain in pA */
    uint16_t * ptrSenseClk;                         /**< Pointer to SnsClk divider */
    uint16_t * sigPFC;                              /**< Pointer to sigPFC value (Signal Per Finger Capacitance) */
    uint16_t snsClkConstantR;                       /**< Resistance in series to a sensor */
    uint16_t vRef;                                  /**< Vref in mVolts  */
    uint16_t fingerCap;                             /**< Finger capacitance in fF 10^-15 (Set in Basic tab in pF 10^-12) */
    uint16_t snsClkInputClock;                      /**< Frequency for sense clock divider in kHz */
    uint16_t calTarget;                             /**< Calibration target in percentage */
    uint8_t iDacMod;                                /**< Modulation idac code */
    uint8_t iDacComp;                               /**< Compensation idac code */
} cy_stc_capsense_auto_tune_config_t;

/** Declares Noise envelope data structure for CSD widgets when SmartSense is enabled */
typedef struct
{
    uint16_t param0;                                /**< Parameter 0 configuration */
    uint16_t param1;                                /**< Parameter 1 configuration */
    uint16_t param2;                                /**< Parameter 2 configuration */
    uint16_t param3;                                /**< Parameter 3 configuration */
    uint16_t param4;                                /**< Parameter 4 configuration */
    uint8_t  param5;                                /**< Parameter 5 configuration */
    uint8_t  param6;                                /**< Parameter 6 configuration */
} cy_stc_capsense_smartsense_csd_noise_envelope_t;

/** Declares Update Thresholds structure */
typedef struct
{
    uint16_t fingerTh;                              /**< Widget finger threshold */
    uint8_t  noiseTh;                               /**< Widget noise threshold */
    uint8_t  nNoiseTh;                              /**< Widget negative noise threshold */
    uint8_t  hysteresis;                            /**< Widget hysteresis for the signal crossing finger threshold */
} cy_stc_capsense_smartsense_update_thresholds_t;

/** Declares Ballistics Multiplier Configuration data structure */
typedef struct
{
    uint8_t  accelCoeff;                            /**< Acceleration Coefficient */
    uint8_t  speedCoeff;                            /**< Speed Coefficient */
    uint8_t  divisorValue;                          /**< Divisor Value */
    uint8_t  speedThresholdX;                       /**< Speed Threshold X */
    uint8_t  speedThresholdY;                       /**< Speed Threshold Y */
    uint8_t  reserved0;                             /**< Reserved field */
    uint8_t  reserved1;                             /**< Reserved field */
    uint8_t  reserved2;                             /**< Reserved field */
} cy_stc_capsense_ballistic_config_t;

/** Declares Ballistics Multiplier Configuration data structure */
typedef struct
{
    uint32_t currentTimestamp;                      /**< Current timestamp */
    uint32_t oldTimestamp;                          /**< Previous timestamp */
    int32_t deltaXfrac;                             /**< Fraction of X-axis displacement */
    int32_t deltaYfrac;                             /**< Fraction of Y-axis displacement */
    uint16_t x;                                     /**< X-axis position */
    uint16_t y;                                     /**< Y-axis position */
    uint8_t touchNumber;                            /**< Current number of touches */
    uint8_t oldTouchNumber;                         /**< Previous number of touches */
    uint8_t reserved0;                              /**< Reserved field */
    uint8_t reserved1;                              /**< Reserved field */
} cy_stc_capsense_ballistic_context_t;

/** Declares Ballistic Displacement structure */
typedef struct
{
    int16_t deltaX;                                 /**< X-axis displacement */
    int16_t deltaY;                                 /**< Y-axis displacement */
} cy_stc_capsense_ballistic_delta_t;

/** \} */

/** Declares ALP filter data structure */
typedef struct
{
    uint32_t dataParam0;                            /**< Parameter 0 context */
    uint16_t dataParam1;                            /**< Parameter 1 context */
    uint16_t dataParam2;                            /**< Parameter 2 context */
    uint16_t dataParam3;                            /**< Parameter 3 context */
    uint16_t dataParam4;                            /**< Parameter 4 context */
    uint16_t dataParam5;                            /**< Parameter 5 context */
    uint16_t dataParam6;                            /**< Parameter 6 context */
    uint8_t dataParam7;                             /**< Parameter 7 context */
} cy_stc_capsense_alp_fltr_channel_t;

/** Declares ALP filter configuration structure */
typedef struct
{
    uint16_t configParam0;                          /**< Parameter 0 configuration */
    uint16_t configParam1;                          /**< Parameter 1 configuration */
    uint16_t configParam2;                          /**< Parameter 2 configuration */
    uint8_t configParam3;                           /**< Parameter 3 configuration */
    uint8_t configParam4;                           /**< Parameter 4 configuration */
    uint8_t configParam5;                           /**< Parameter 5 configuration */
} cy_stc_capsense_alp_fltr_config_t;


/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void Cy_CapSense_AdaptiveFilterInitialize_Lib(
                const cy_stc_capsense_adaptive_filter_config_t * config, 
                cy_stc_capsense_position_t * context);
void Cy_CapSense_AdaptiveFilterRun_Lib(
                const cy_stc_capsense_adaptive_filter_config_t * config,
                cy_stc_capsense_position_t * context, 
                uint32_t * currentX, 
                uint32_t * currentY);
void Cy_CapSense_AdvancedCentroidGetTouchCoordinates_Lib(
                const cy_stc_capsense_advanced_centroid_config_t * config,
                const uint16_t * ptrSns,
                cy_stc_capsense_touch_t * touch);
void Cy_CapSense_BallisticMultiplier_Lib(
                const cy_stc_capsense_ballistic_config_t * config, 
                cy_stc_capsense_touch_t * touch,
                cy_stc_capsense_ballistic_delta_t * delta,
                uint32_t timestamp,
                cy_stc_capsense_ballistic_context_t * context);
void Cy_CapSense_AlpRun_Lib(
                cy_stc_capsense_alp_fltr_channel_t * ptrFilterObj, 
                const cy_stc_capsense_alp_fltr_config_t * ptrFilterConfig,
                uint16_t * rawCount, 
                const uint16_t * baseline);
void Cy_CapSense_AlpInitialize_Lib(
                cy_stc_capsense_alp_fltr_channel_t * ptrFilterObj, 
                const uint16_t * rawCount);
void Cy_CapSense_AlpResetState_Lib(
                cy_stc_capsense_alp_fltr_channel_t * ptrFilterObj);
uint32_t Cy_CapSense_AlpGetAverage_Lib(
                const cy_stc_capsense_alp_fltr_channel_t * ptrFilterObj);
uint32_t Cy_CapSense_TunePrescalers_Lib(
                cy_stc_capsense_auto_tune_config_t * config);
uint8_t Cy_CapSense_TuneSensitivity_Lib(
                cy_stc_capsense_auto_tune_config_t * config);
void Cy_CapSense_UpdateThresholds_Lib(
                const cy_stc_capsense_smartsense_csd_noise_envelope_t * ptrNoiseEnvelope,
                cy_stc_capsense_smartsense_update_thresholds_t * ptrThresholds,
                uint16_t sigPFC,
                uint32_t startFlag);
void Cy_CapSense_InitializeNoiseEnvelope_Lib(
                uint16_t rawCount,
                uint16_t sigPFC,
                cy_stc_capsense_smartsense_csd_noise_envelope_t * ptrNoiseEnvelope);
void Cy_CapSense_RunNoiseEnvelope_Lib(
                uint16_t rawCount,
                uint16_t sigPFC,
                cy_stc_capsense_smartsense_csd_noise_envelope_t * ptrNoiseEnvelope);


#if defined(__cplusplus)
}
#endif

#endif /* CY_CAPSENSE_LIB_H */


/* [] END OF FILE */
