/***************************************************************************//**
* \file cy_capsense_structure.c
* \version 1.1
*
* \brief
* This file defines the data structure global variables and provides the
* implementation for the functions of the Data Structure module.
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

#include <stddef.h>
#include <string.h>
#include "cy_syslib.h"
#include "cy_capsense_common.h"
#include "cy_capsense_structure.h"
#include "cy_capsense_lib.h"
#include "cy_csd.h"


/*******************************************************************************
* Function Name: Cy_CapSense_IsAnyWidgetActive
****************************************************************************//**
*
* Reports whether any widget has detected a touch.
*
* This function reports whether any widget has detected a touch by extracting
* information from the widget status registers. This function does 
* not process a widget but extracts processed results 
* from the \ref group_capsense_structures.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
* \return
* Returns the touch detection status of all the widgets:
* - Zero - No touch is detected in any of the widgets or sensors.
* - Non-zero - At least one widget or sensor has detected a touch.
*
*******************************************************************************/
uint32_t Cy_CapSense_IsAnyWidgetActive(const cy_stc_capsense_context_t * context)
{
    uint32_t result = 0u;
    uint32_t wdIndex;

    for (wdIndex = context->ptrCommonConfig->numWd; wdIndex-- > 0u;)
    {
        result |= (uint32_t)context->ptrWdContext[wdIndex].status & CY_CAPSENSE_WD_ACTIVE_MASK;
    }

    return result;
}


/*******************************************************************************
* Function Name: Cy_CapSense_IsWidgetActive
****************************************************************************//**
*
* Reports whether the specified widget detects a touch on any of its sensors.
*
* This function reports whether the specified widget has detected a touch by
* extracting information from the widget status register.
* This function does not process the widget but extracts processed 
* results from the \ref group_capsense_structures.
*
* \param widgetId
* Specifies the ID number of the widget to get its status.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
* \return
* Returns the touch detection status of the specified widgets:
* - Zero - No touch is detected in the specified widget or a wrong widgetId
* is specified.
* - Non-zero if at least one sensor of the specified widget is active, i.e.
* a touch is detected.
*
*******************************************************************************/
uint32_t Cy_CapSense_IsWidgetActive(
                uint32_t widgetId, 
                const cy_stc_capsense_context_t * context)
{
    uint32_t result = 0uL;

    if (widgetId < context->ptrCommonConfig->numWd)
    {
        result = (uint32_t)context->ptrWdContext[widgetId].status & CY_CAPSENSE_WD_ACTIVE_MASK;
    }
    return result;
}


/*******************************************************************************
* Function Name: Cy_CapSense_IsSensorActive
****************************************************************************//**
*
* Reports whether the specified sensor in the widget detects a touch.
*
* This function reports whether the specified sensor in the widget has detected a
* touch by extracting information from the widget status register.
* This function does not process the widget or sensor but extracts processed 
* results from the \ref group_capsense_structures.
*
* For proximity sensors, this function returns the proximity detection status. 
* To get the finger touch status of proximity sensors, use the
* Cy_CapSense_IsProximitySensorActive() function.
*
* \param widgetId
* Specifies the ID number of the widget.

* \param sensorId
* Specifies the ID number of the sensor within the widget to get its touch
* detection status.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
* \return
* Returns the touch detection status of the specified sensor/widget:
* - Zero if no touch is detected in the specified sensor/widget or a wrong
* widget ID/sensor ID is specified.
* - Non-zero if the specified sensor is active, i.e. touch is detected. If the
* specific sensor belongs to a proximity widget, the proximity detection
* status is returned.
*
*******************************************************************************/
uint32_t Cy_CapSense_IsSensorActive(
                uint32_t widgetId, 
                uint32_t sensorId, 
                const cy_stc_capsense_context_t * context)
{
    uint32_t result = 0uL;

    if (widgetId < context->ptrCommonConfig->numWd)
    {
        if (sensorId < context->ptrWdConfig[widgetId].numSns)
        {
            result = context->ptrWdConfig[widgetId].ptrSnsContext[sensorId].status;
        }
    }
    return result;
}


/*******************************************************************************
* Function Name: Cy_CapSense_IsProximitySensorActive
****************************************************************************//**
*
* Reports the finger detection status of the specified proximity widget/sensor.
*
* This function reports whether the specified proximity sensor has detected a touch
* by extracting information from the widget status register.
* This function is used only with proximity sensor widgets. 
* This function does not process the widget but extracts processed results 
* from the \ref group_capsense_structures.
*
* \param widgetId
* Specifies the ID number of the proximity widget.
*
* \param sensorId
* Specifies the ID number of the proximity sensor within the proximity widget to
* get its touch detection status.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
* \return
* Returns the status of the specified sensor of the proximity widget. Zero
* indicates that no touch is detected in the specified sensor/widget or a
* wrong widgetId/proxId is specified.
* - Bits [31..2] are reserved.
* - Bit [1] indicates that a touch is detected.
* - Bit [0] indicates that a proximity is detected.
*
*******************************************************************************/
uint32_t Cy_CapSense_IsProximitySensorActive(
                uint32_t widgetId, 
                uint32_t sensorId, 
                const cy_stc_capsense_context_t * context)
{
    uint32_t result = 0uL;

    if (widgetId < context->ptrCommonConfig->numWd)
    {
        if ((uint8_t)CY_CAPSENSE_WD_PROXIMITY_E == context->ptrWdConfig[widgetId].wdType)
        {
            if (sensorId < context->ptrWdConfig[widgetId].numSns)
            {
                result |= context->ptrWdConfig[widgetId].ptrSnsContext[sensorId].status;
            }
        }
    }
    
    return result;
}


/*******************************************************************************
* Function Name: Cy_CapSense_GetTouchInfo
****************************************************************************//**
*
* Reports the touch position detected for the specified touchpad, matrix
* button, and slider widgets.
*
* This function does not process the widget but extracts processed 
* results from the \ref group_capsense_structures.
*
* \param widgetId
* Specifies the ID number of a touchpad widget to get the X/Y position of a
* detected touch.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
* \return
* Returns the pointer to touch widget touch cy_stc_capsense_touch_t that 
* contains number of positions and data about each position.
*
*******************************************************************************/
cy_stc_capsense_touch_t * Cy_CapSense_GetTouchInfo(
                uint32_t widgetId, 
                const cy_stc_capsense_context_t * context)
{
    cy_stc_capsense_touch_t * ptrTouch = NULL;
    
    const cy_stc_capsense_widget_config_t * ptrWdCfg;

    if (widgetId < context->ptrCommonConfig->numWd)
    {
        ptrWdCfg = &context->ptrWdConfig[widgetId];
        switch (ptrWdCfg->wdType)
        {
            case (uint8_t)CY_CAPSENSE_WD_TOUCHPAD_E:
            case (uint8_t)CY_CAPSENSE_WD_MATRIX_BUTTON_E:
            case (uint8_t)CY_CAPSENSE_WD_LINEAR_SLIDER_E:
            case (uint8_t)CY_CAPSENSE_WD_RADIAL_SLIDER_E:
                ptrTouch = &ptrWdCfg->ptrWdContext->wdTouch;
                break;
            default:
                break;
        }
    }
    
    return ptrTouch;
}


/*******************************************************************************
* Function Name: Cy_CapSense_CheckConfigIntegrity
****************************************************************************//**
*
* Performs verification of CapSense data structure initialization.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
* \return status
* Returns status of operation:
* - Zero        - Indicates successful initialization.
* - Non-zero    - One or more errors occurred in the initialization process.
*
*******************************************************************************/
cy_status Cy_CapSense_CheckConfigIntegrity(const cy_stc_capsense_context_t * context)
{
    cy_status result = CY_RET_SUCCESS;
    
    const cy_stc_capsense_common_config_t * ptrCommonCfg = context->ptrCommonConfig;
    const cy_stc_capsense_common_context_t * ptrCommonCxt = context->ptrCommonContext;
    const cy_stc_capsense_internal_context_t * ptrInternalCxt = context->ptrInternalContext;
    const cy_stc_capsense_widget_config_t * ptrWdCfg = context->ptrWdConfig;
    const cy_stc_capsense_widget_context_t * ptrWdCxt = context->ptrWdContext;
    const cy_stc_capsense_pin_config_t * ptrPinCfg = context->ptrPinConfig;
    const cy_stc_capsense_pin_config_t * ptrShieldPinCfg = context->ptrShieldPinConfig;
    const cy_stc_active_scan_sns_t * ptrActScanSns = context->ptrActiveScanSns;
    
    if (ptrCommonCfg == NULL)       {result |= CY_RET_BAD_DATA;}
    if (ptrCommonCxt == NULL)       {result |= CY_RET_BAD_DATA;}
    if (ptrInternalCxt == NULL)     {result |= CY_RET_BAD_DATA;}
    if (ptrWdCfg == NULL)           {result |= CY_RET_BAD_DATA;}
    if (ptrWdCxt == NULL)           {result |= CY_RET_BAD_DATA;}
    if (ptrPinCfg == NULL)          {result |= CY_RET_BAD_DATA;}
    if (ptrCommonCfg->csdShieldEn != 0u)
    {
        if((ptrCommonCfg->csdShieldNumPin > 0u) && (ptrShieldPinCfg == NULL))
        {
            result |= CY_RET_BAD_DATA;
        }
    }
    if (ptrActScanSns == NULL)      {result |= CY_RET_BAD_DATA;}
    
    
    return (result);
}


/* [] END OF FILE */
