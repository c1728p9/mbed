/***************************************************************************//**
* \file cy_capsense_filter.h
* \version 1.1
*
* \brief
* This file contains the definitions for all the filters implementation.
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

#if !defined(CY_CAPSENSE_FILTER_H)
#define CY_CAPSENSE_FILTER_H

#include "cy_syslib.h"

#include "cy_capsense_structure.h"
#include "cy_capsense_structure.h"
#include "cy_capsense_common.h"

#if defined(__cplusplus)
extern "C" {
#endif

/***************************************
* Function Prototypes
***************************************/

/*******************************************************************************
* LOW LEVEL FUNCTIONS
*******************************************************************************/

/******************************************************************************/
/** \addtogroup group_capsense_low_level *//** \{ */
/******************************************************************************/

cy_status Cy_CapSense_UpdateAllBaselines(const cy_stc_capsense_context_t * context);
cy_status Cy_CapSense_UpdateWidgetBaseline(uint32_t widgetId, const cy_stc_capsense_context_t * context);
cy_status Cy_CapSense_UpdateSensorBaseline(uint32_t widgetId, uint32_t sensorId, const cy_stc_capsense_context_t * context);

void Cy_CapSense_InitializeAllBaselines(cy_stc_capsense_context_t * context);
void Cy_CapSense_InitializeWidgetBaseline(uint32_t widgetId, cy_stc_capsense_context_t * context);
void Cy_CapSense_InitializeSensorBaseline(uint32_t widgetId, uint32_t sensorId, cy_stc_capsense_context_t * context);

void Cy_CapSense_InitializeAllFilters(const cy_stc_capsense_context_t * context);
void Cy_CapSense_InitializeWidgetFilter(uint32_t widgetId, const cy_stc_capsense_context_t * context);

/** \} */


/*******************************************************************************
* Function Prototypes - Internal functions
*******************************************************************************/

/******************************************************************************/
/** \cond SECTION_CAPSENSE_INTERNAL */
/** \addtogroup group_capsense_internal *//** \{ */
/******************************************************************************/

void Cy_CapSense_InitializeIIR(
                uint32_t widgetId, 
                uint32_t sensorId, 
                const cy_stc_capsense_context_t * context);
void Cy_CapSense_RunIIR(
                uint32_t widgetId, 
                uint32_t sensorId, 
                const cy_stc_capsense_context_t * context);

void Cy_CapSense_InitializeMedian(
                uint32_t widgetId, 
                uint32_t sensorId, 
                const cy_stc_capsense_context_t * context);
void Cy_CapSense_RunMedian(
                uint32_t widgetId, 
                uint32_t sensorId, 
                const cy_stc_capsense_context_t * context);

void Cy_CapSense_InitializeAverage(
                uint32_t widgetId, 
                uint32_t sensorId, 
                const cy_stc_capsense_context_t * context);
void Cy_CapSense_RunAverage(
                uint32_t widgetId, 
                uint32_t sensorId, 
                const cy_stc_capsense_context_t * context);

void Cy_CapSense_FtInitializeBaseline(
                cy_stc_capsense_sensor_context_t * ptrSnsContext);
uint32_t Cy_CapSense_FtUpdateBaseline(
                const cy_stc_capsense_widget_context_t * ptrWdContext,
                cy_stc_capsense_sensor_context_t * ptrSnsContext,
                const cy_stc_capsense_context_t * context);

uint32_t Cy_CapSense_FtMedian(uint32_t x1, uint32_t x2, uint32_t x3);
uint32_t Cy_CapSense_FtIIR1stOrder(uint32_t input, uint32_t prevOutput, uint32_t n);
uint32_t Cy_CapSense_FtJitter(uint32_t input, uint32_t prevOutput);

void Cy_CapSense_FtRunEnabledFiltersInternal(
                const cy_stc_capsense_widget_config_t * ptrWdConfig,
                cy_stc_capsense_sensor_context_t * ptrSnsContext,
                uint16_t * ptrSnsRawHistory,
                uint8_t * ptrSnsRawHistoryLow);
void Cy_CapSense_InitializeIIRInternal(
                const cy_stc_capsense_widget_config_t * ptrWdConfig,
                const cy_stc_capsense_sensor_context_t * ptrSnsContext,
                uint16_t * ptrSnsRawHistory,
                uint8_t * ptrSnsRawHistoryLow);
void Cy_CapSense_RunIIRInternal(
                const cy_stc_capsense_widget_config_t * ptrWdConfig,
                cy_stc_capsense_sensor_context_t * ptrSnsContext,
                uint16_t * ptrSnsRawHistory,
                uint8_t * ptrSnsRawHistoryLow);
void Cy_CapSense_InitializeMedianInternal(
                const cy_stc_capsense_widget_config_t * ptrWdConfig,
                const cy_stc_capsense_sensor_context_t * ptrSnsContext,
                uint16_t * ptrSnsRawHistory);
void Cy_CapSense_RunMedianInternal(
                const cy_stc_capsense_widget_config_t * ptrWdConfig,
                cy_stc_capsense_sensor_context_t * ptrSnsContext,
                uint16_t * ptrSnsRawHistory);
void Cy_CapSense_InitializeAverageInternal(
                const cy_stc_capsense_widget_config_t * ptrWdConfig,
                const cy_stc_capsense_sensor_context_t * ptrSnsContext,
                uint16_t * ptrSnsRawHistory);
void Cy_CapSense_RunAverageInternal(
                const cy_stc_capsense_widget_config_t * ptrWdConfig,
                cy_stc_capsense_sensor_context_t * ptrSnsContext,
                uint16_t * ptrSnsRawHistory);

/** \} \endcond */

#if defined(__cplusplus)
}
#endif

#endif /* CY_CAPSENSE_FILTER_H */


/* [] END OF FILE */
