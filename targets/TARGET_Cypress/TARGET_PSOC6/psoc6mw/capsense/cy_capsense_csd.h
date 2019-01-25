/***************************************************************************//**
* \file cy_capsense_csd.h
* \version 1.1
*
* \brief
* This file provides the function prototypes specific to the CSD sensing
* implementation.
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

#if !defined(CY_CAPSENSE_CSD_H)
#define CY_CAPSENSE_CSD_H

#include "cy_syslib.h"
#include "cy_capsense_common.h"
#include "cy_capsense_structure.h"

#if defined(__cplusplus)
extern "C" {
#endif

/****************************************************************************
* Register and mode mask definition
****************************************************************************/
#define CY_CAPSENSE_DEFAULT_CSD_SW_DSI_SEL                      (0x00000000uL)
#define CY_CAPSENSE_DEFAULT_CSD_INTR_SET                        (0x00000000uL)
#define CY_CAPSENSE_DEFAULT_SW_HS_N_SEL                         (0x00000000uL)
#define CY_CAPSENSE_DEFAULT_CSD_ADC_CTL                         (0x00000000uL)

/* SW_FW_MOD_SEL switches states */
#define CY_CAPSENSE_CSD_SW_FW_MOD_SEL_SCAN                      (0x00000000uL)
#define CY_CAPSENSE_CSD_SW_FW_MOD_SEL_INIT                      (0x00000000uL)

#define CY_CAPSENSE_EXT_CAP_DISCHARGE_TIME                      (1u)

/***************************************
* Function Prototypes
**************************************/

/******************************************************************************/
/** \addtogroup group_capsense_low_level *//** \{ */
/******************************************************************************/

void Cy_CapSense_CSDSetupWidget(uint32_t widgetId, cy_stc_capsense_context_t * context);
void Cy_CapSense_CSDSetupWidgetExt(uint32_t widgetId, uint32_t sensorId, cy_stc_capsense_context_t * context);
void Cy_CapSense_CSDScan(cy_stc_capsense_context_t * context);
void Cy_CapSense_CSDScanExt(cy_stc_capsense_context_t * context);
cy_status Cy_CapSense_CSDCalibrateWidget(uint32_t widgetId, uint32_t target, cy_stc_capsense_context_t * context);
void Cy_CapSense_CSDConnectSns(const cy_stc_capsense_pin_config_t * snsAddrPtr, cy_stc_capsense_context_t * context);
void Cy_CapSense_CSDDisconnectSns(const cy_stc_capsense_pin_config_t * snsAddrPtr, const cy_stc_capsense_context_t * context);

/** \} */

/******************************************************************************/
/** \cond SECTION_CAPSENSE_INTERNAL */
/** \addtogroup group_capsense_internal *//** \{ */
/******************************************************************************/

void Cy_CapSense_CSDDisableMode(cy_stc_capsense_context_t * context);
void Cy_CapSense_CSDInitialize(cy_stc_capsense_context_t * context);
void Cy_CapSense_CSDStartSample(cy_stc_capsense_context_t * context);
void Cy_CapSense_CSDDisableShieldElectrodes(const cy_stc_capsense_context_t * context);

uint32_t Cy_CapSense_CSDGetNumberOfConversions(uint32_t snsClkDivider, uint32_t resolution, uint32_t snsClkSrc);
void Cy_CapSense_CSDSetUpIdacs(cy_stc_capsense_context_t * context);
void Cy_CapSense_CSDSnsStateCheck(cy_stc_capsense_context_t * context);
void Cy_CapSense_CSDCalculateScanDuration(cy_stc_capsense_context_t * context);
void Cy_CapSense_CSDConnectSnsExt(cy_stc_capsense_context_t * context);
void Cy_CapSense_CSDDisconnectSnsExt(cy_stc_capsense_context_t * context);
void Cy_CapSense_CSDConfigClock(cy_stc_capsense_context_t * context);
void Cy_CapSense_CSDClearSensors(const cy_stc_capsense_context_t * context);
void Cy_CapSense_CSDSetWidgetSenseClkSrc(const cy_stc_capsense_widget_config_t * ptrWdConfig);
uint32_t Cy_CapSense_CSDCalcPrsSize(uint32_t snsClkDivider, uint32_t resolution);
void Cy_CapSense_CSDScanISR(void * capsenseContext);

/** \} \endcond */

#if defined(__cplusplus)
}
#endif

#endif /* CY_CAPSENSE_CSD_H */


/* [] END OF FILE */
