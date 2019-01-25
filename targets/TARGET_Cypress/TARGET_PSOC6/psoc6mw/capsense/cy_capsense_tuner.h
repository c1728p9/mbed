/***************************************************************************//**
* \file cy_capsense_tuner.h
* \version 1.1
*
* \brief
* This file provides Tuner FW module function prototypes.
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

#if !defined(CY_CAPSENSE_TUNER_H)
#define CY_CAPSENSE_TUNER_H

#include "cy_syslib.h"
#include "cy_capsense_structure.h"

#if defined(__cplusplus)
extern "C" {
#endif


/*******************************************************************************
* Macros Definitions
*******************************************************************************/
#define CY_CAPSENSE_COMMAND_PACKET_SIZE             (16u)
#define CY_CAPSENSE_COMMAND_CRC_DATA_SIZE           (11u)

#define CY_CAPSENSE_COMMAND_HEAD_0_IDX              (0u)
#define CY_CAPSENSE_COMMAND_HEAD_1_IDX              (1u)
#define CY_CAPSENSE_COMMAND_CODE_0_IDX              (2u)
#define CY_CAPSENSE_COMMAND_CNTR_0_IDX              (3u)
#define CY_CAPSENSE_COMMAND_SIZE_0_IDX              (4u)
#define CY_CAPSENSE_COMMAND_OFFS_0_IDX              (5u)
#define CY_CAPSENSE_COMMAND_OFFS_1_IDX              (6u)
#define CY_CAPSENSE_COMMAND_DATA_0_IDX              (7u)
#define CY_CAPSENSE_COMMAND_DATA_1_IDX              (8u)
#define CY_CAPSENSE_COMMAND_DATA_2_IDX              (9u)
#define CY_CAPSENSE_COMMAND_DATA_3_IDX              (10u)
#define CY_CAPSENSE_COMMAND_CRC_0_IDX               (11u)
#define CY_CAPSENSE_COMMAND_CRC_1_IDX               (12u)
#define CY_CAPSENSE_COMMAND_TAIL_0_IDX              (13u)
#define CY_CAPSENSE_COMMAND_TAIL_1_IDX              (14u)
#define CY_CAPSENSE_COMMAND_TAIL_2_IDX              (15u)

#define CY_CAPSENSE_COMMAND_HEAD_0                  (0x0Du)
#define CY_CAPSENSE_COMMAND_HEAD_1                  (0x0Au)
#define CY_CAPSENSE_COMMAND_TAIL_0                  (0x00u)
#define CY_CAPSENSE_COMMAND_TAIL_1                  (0xFFu)
#define CY_CAPSENSE_COMMAND_TAIL_2                  (0xFFu)

#define CY_CAPSENSE_COMMAND_OK                      (0u)
#define CY_CAPSENSE_WRONG_HEADER                    (1u)
#define CY_CAPSENSE_WRONG_CRC                       (2u)
#define CY_CAPSENSE_WRONG_TAIL                      (3u)
#define CY_CAPSENSE_WRONG_CODE                      (4u)

#define CY_CAPSENSE_MSB_SHIFT                       (8u)

/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/******************************************************************************/
/** \addtogroup group_capsense_high_level *//** \{ */
/******************************************************************************/

uint32_t Cy_CapSense_RunTuner(cy_stc_capsense_context_t * context);
uint32_t Cy_CapSense_CheckCommandIntegrity(const uint8_t * commandPacket);

/** \} */

/******************************************************************************/
/** \cond SECTION_CAPSENSE_INTERNAL */
/** \addtogroup group_capsense_internal *//** \{ */
/******************************************************************************/

void Cy_CapSense_TuInitialize(cy_stc_capsense_context_t * context);
uint16 Cy_CapSense_CalculateCrc16(const uint8_t *ptrData, uint32_t len);

/** \} \endcond */

#if defined(__cplusplus)
}
#endif

#endif /* CY_CAPSENSE_TUNER_H */


/* [] END OF FILE */
