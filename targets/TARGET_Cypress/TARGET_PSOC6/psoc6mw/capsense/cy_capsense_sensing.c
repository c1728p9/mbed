/***************************************************************************//**
* \file cy_capsense_sensing.c
* \version 1.1
*
* \brief
* This file contains the source of functions common for different sensing
* methods.
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

#include <stdlib.h>
#include "cy_gpio.h"
#include "cy_sysclk.h"
#include "cy_csd.h"

#include "cy_capsense_common.h"
#include "cy_capsense_structure.h"
#include "cy_capsense_sensing.h"
#include "cy_capsense_csx.h"
#include "cy_capsense_csd.h"
#include "cy_capsense_lib.h"


/*******************************************************************************
* Constants
*******************************************************************************/
#define CY_CAPSENSE_AUTOTUNE_CALIBRATION_RESOLUTION         (12u)
#define CY_CAPSENSE_AUTOTUNE_CALIBRATION_FREQ_KHZ           (1500u)
#define CY_CAPSENSE_CSD_SNS_FREQ_KHZ_MAX                    (6000u)
#define CY_CAPSENSE_CSD_AUTOTUNE_CAL_UNITS                  (1000u)
#define CY_CAPSENSE_AUTOTUNE_CP_MAX                         (69000Lu)

#define CY_CAPSENSE_EXT_CAP_DISCHARGE_TIME                  (1u)

#define CY_CAPSENSE_VREF_GAIN_MAX       (32u)
#define CY_CAPSENSE_VREF_VDDA_MIN_DIFF  (600u)
#define CY_CAPSENSE_VREF_SRSS_MV        (800u)
#define CY_CAPSENSE_VREF_PASS_MV        (1200u)



const cy_stc_csd_config_t cy_capsense_csdCfg = CY_CAPSENSE_CSD_CONFIG_DEFAULT;


/*******************************************************************************
* Function Name: Cy_CapSense_IsBusy
****************************************************************************//**
*
* Returns the current status of the middleware (Scan is completed or Scan is in
* progress).
*
* This function returns a status of the CapSense middleware whether a scan is
* currently in progress or not. If the middleware is busy, no new scan or setup
* widgets is made. The critical section (i.e. disable global interrupt)
* is recommended for the application when the device transitions from
* the Active mode to Sleep or Deep Sleep modes.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
* \return
* Returns the current status of the middleware:
* - CY_CAPSENSE_NOT_BUSY - No scan is in progress and a next scan
* can be initiated.
* - CY_CAPSENSE_SW_STS_BUSY - The previous scanning is not completed
* and the hardware block is busy.
*
*******************************************************************************/
uint32_t Cy_CapSense_IsBusy(const cy_stc_capsense_context_t * context)
{
    return (context->ptrCommonContext->status & CY_CAPSENSE_SW_STS_BUSY);
}


/*******************************************************************************
* Function Name: Cy_CapSense_SetBusyFlags
****************************************************************************//**
*
* Sets BUSY flags of the cy_capsense_context.status register specified
* by the flags parameter.
*
* \details
* This is an internal function. Do not call this function directly from 
* the application layer.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
*******************************************************************************/
void Cy_CapSense_SetBusyFlags(cy_stc_capsense_context_t * context)
{
    context->ptrCommonContext->status |= CY_CAPSENSE_SW_STS_BUSY;
}


/*******************************************************************************
* Function Name: Cy_CapSense_ClrBusyFlags
****************************************************************************//**
*
* Clears BUSY flags of the cy_capsense_context.status register specified
* by the flags parameter.
*
* This is an internal function. Do not call this function directly from 
* the application layer.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
*******************************************************************************/
void Cy_CapSense_ClrBusyFlags(cy_stc_capsense_context_t * context)
{
    /* Clear busy flag */
    context->ptrCommonContext->status &= (uint32_t)(~((uint32_t)CY_CAPSENSE_SW_STS_BUSY));
    
    /* Reset scan flags */
    context->ptrActiveScanSns->scanScopeAll = CY_CAPSENSE_SCAN_SCOPE_SGL_WD;
    context->ptrActiveScanSns->scanScopeSns = CY_CAPSENSE_SCAN_SCOPE_UND;
    
    /* Mark completion of scan cycle */ 
    context->ptrCommonContext->scanCounter++;
    
     if(NULL != context->ptrCommonContext->ptrEOSCallback)
    {
        context->ptrCommonContext->ptrEOSCallback(context->ptrActiveScanSns);
    }
}


/*******************************************************************************
* Function Name: Cy_CapSense_SetupWidget
****************************************************************************//**
*
* Performs the initialization required to scan the specified widget.
*
* This function prepares the middleware to scan all the sensors in the specified
* widget by executing the following tasks:
* 1. Re-initialize the hardware if it is not configured to perform the
* sensing method used by the specified widget, this happens only if the
* CSD and CSX methods are used in the middleware.
* 2. Initialize the hardware with specific sensing configuration (e.g.
* sensor clock, scan resolution) used by the widget.
* 3. Disconnect all previously connected electrodes, if the electrodes
* connected by the Cy_CapSense_CSDSetupWidgetExt(),
* Cy_CapSense_CSXSetupWidgetExt() or Cy_CapSense_CSDConnectSns()
* functions and not disconnected.
*
* This function does not start sensor scanning. The Cy_CapSense_Scan()
* function must be called to start the scan sensors in the widget. If this
* function is called more than once, it does not break the middleware operation,
* but only the last initialized widget is in effect.
*
* \param widgetId
* Specifies the ID number of the widget to be initialized for scanning.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
* \return
* Returns the status of the widget setting up operation:
* - CY_RET_SUCCESS - The operation is successfully completed.
* - CY_RET_BAD_PARAM - The widget is invalid or if the specified widget is
* disabled.
* - CY_RET_INVALID_STATE - The previous scanning is not completed and the
* hardware block is busy.
* - CY_RET_UNKNOWN - An unknown sensing method is used by the widget or any
* other spurious error occurred.
*
**********************************************************************************/
cy_status Cy_CapSense_SetupWidget(
                uint32_t widgetId, 
                cy_stc_capsense_context_t * context)
{
    cy_status widgetStatus;

    if (CY_CAPSENSE_SW_STS_BUSY == Cy_CapSense_IsBusy(context))
    {
        /* Previous widget is being scanned. Return error. */
        widgetStatus = CY_RET_INVALID_STATE;
    }
    /*
     * Check whether widget id is valid, specified widget is enabled and widget did not
     * detect any fault conditions. If all conditions are met,
     * set widgetStatus as good, if not, set widgetStatus as bad.
     */
    else if ((context->ptrCommonConfig->numWd > widgetId) &&
        (0u == (context->ptrWdConfig[widgetId].ptrWdContext->status & CY_CAPSENSE_WD_DISABLE_MASK)))
    {
        /* Check widget sensing method and call corresponding setup function */
        switch(context->ptrWdConfig[widgetId].senseMethod)
        {
            case (uint8_t)CY_CAPSENSE_SENSE_METHOD_CSD_E:
                Cy_CapSense_CSDSetupWidget(widgetId, context);
                break;

            case (uint8_t)CY_CAPSENSE_SENSE_METHOD_CSX_E:
                Cy_CapSense_CSXSetupWidget(widgetId, context);
                break;

            default:
                widgetStatus = CY_RET_UNKNOWN;
                break;
        }
        widgetStatus = CY_RET_SUCCESS;
    }
    else
    {
        widgetStatus = CY_RET_BAD_PARAM;
    }

    return (widgetStatus);
}


/*******************************************************************************
* Function Name: Cy_CapSense_Scan
****************************************************************************//**
*
* Initiates scanning of all the sensors in the widget initialized by
* Cy_CapSense_SetupWidget(), if no scan is in progress.
*
* This function is called only after the Cy_CapSense_SetupWidget()
* function is called to start the scanning of the sensors in the widget. The
* status of a sensor scan must be checked using the Cy_CapSense_IsBusy()
* prior to starting a next scan or setting up another widget.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
* \return
* Returns the status of the scan initiation operation:
* - CY_RET_SUCCESS - Scanning is successfully started.
* - CY_RET_INVALID_STATE - The previous scanning is not completed and the
* hardware block is busy.
* - CY_RET_UNKNOWN - An unknown sensing method is used by the widget.
*
********************************************************************************/
cy_status Cy_CapSense_Scan(cy_stc_capsense_context_t * context)
{
    cy_status scanStatus = CY_RET_SUCCESS;

    if (CY_CAPSENSE_SW_STS_BUSY == Cy_CapSense_IsBusy(context))
    {
        /* Previous widget is being scanned. Return error. */
        scanStatus = CY_RET_INVALID_STATE;
    }
    else
    {
        /* Check widget sensing method and call appropriate functions */
        switch(context->ptrActiveScanSns->ptrWdConfig->senseMethod)
        {
            case (uint8_t)CY_CAPSENSE_SENSE_METHOD_CSD_E:
                Cy_CapSense_CSDScan(context);
                break;

            case (uint8_t)CY_CAPSENSE_SENSE_METHOD_CSX_E:
                Cy_CapSense_CSXScan(context);
                break;

            default:
                scanStatus = CY_RET_UNKNOWN;
                break;
        }
    }
    return (scanStatus);
}


/*******************************************************************************
* Function Name: Cy_CapSense_SetupWidgetExt
****************************************************************************//**
*
* Performs extended initialization for the specified widget and also performs
* initialization required for a specific sensor in the widget. 
* 
* This function requires using the Cy_CapSense_ScanExt() function to 
* initiate a scan.
*
* This function does the same as Cy_CapSense_SetupWidget() and also
* does the following tasks:
* 1. Connects the specified sensor of the widget.
* 2. Configures HW to perform scan of the specified sensor.
*
* Once this function is called to initialize a widget and a sensor, the
* Cy_CapSense_ScanExt() function is called to scan the sensor.
*
* This function is called when no scanning is in progress. I.e.
* Cy_CapSense_IsBusy() returns a non-busy status.
*
* Calling this function directly from the application layer is not
* recommended. This function is used to implement only the user's specific
* use cases (for faster execution time or pipeline scanning for example).
*
* \param widgetId
* Specifies the ID number of a widget.
*
* \param sensorId
* Specifies the ID number of the sensor within the widget to perform hardware
* and firmware initialization required for scanning a specific sensor in a
* specific widget.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
**********************************************************************************/
cy_status Cy_CapSense_SetupWidgetExt(
                uint32_t widgetId, 
                uint32_t sensorId, 
                cy_stc_capsense_context_t * context)
{
    cy_status widgetStatus = CY_RET_BAD_PARAM;

    if (CY_CAPSENSE_SW_STS_BUSY == Cy_CapSense_IsBusy(context))
    {
        /* Previous widget is being scanned. Return error. */
        widgetStatus = CY_RET_INVALID_STATE;
    }
    /*
     * Check whether widget id is valid, specified widget is enabled and widget did not
     * detect any fault conditions. If all conditions are met,
     * set widgetStatus as good, if not, set widgetStatus as bad.
     */
    else if (context->ptrCommonConfig->numWd > widgetId)
    {
        if (context->ptrWdConfig[widgetId].numSns > sensorId)
        {
            /* Check widget sensing method and call corresponding setup function */
            switch(context->ptrWdConfig[widgetId].senseMethod)
            {
                case (uint8_t)CY_CAPSENSE_SENSE_METHOD_CSD_E:
                    Cy_CapSense_CSDSetupWidgetExt(widgetId, sensorId, context);
                    break;

                case (uint8_t)CY_CAPSENSE_SENSE_METHOD_CSX_E:
                    Cy_CapSense_CSXSetupWidgetExt(widgetId, sensorId, context);
                    break;

                default:
                    widgetStatus = CY_RET_UNKNOWN;
                    break;
            }
            widgetStatus = CY_RET_SUCCESS;
        }
        else
        {
            widgetStatus = CY_RET_BAD_PARAM;
        }
    }
    else
    {
        widgetStatus = CY_RET_BAD_PARAM;
    }

    return (widgetStatus);
}


/*******************************************************************************
* Function Name: Cy_CapSense_ScanExt
****************************************************************************//**
*
* Starts a conversion on the pre-configured sensor. This function requires
* using the Cy_CapSense_SetupWidgetExt() function to set up the a
* widget.
*
* This function performs single scanning of one sensor in the widget configured
* by the Cy_CapSense_SetupWidgetExt() function. 
*
* Calling this function directly from the application layer is not
* recommended. This function is used to implement only the user's specific
* use cases (for faster execution time or pipeline scanning for example).
* This function is called when no scanning is in progress. I.e.
* Cy_CapSense_IsBusy() returns a non-busy status.
*
* The sensor must be pre-configured by using the
* Cy_CapSense_SetupWidgetExt() prior to calling this function.
* The sensor remains ready for a next scan if a previous scan was triggered
* by using the Cy_CapSense_ScanExt() function. In this case, calling
* Cy_CapSense_SetupWidgetExt() is not required every time before the
* Cy_CapSense_ScanExt() function. If a previous scan was triggered in
* any other way - Cy_CapSense_Scan(), Cy_CapSense_ScanAllWidgets() or
* Cy_CapSense_RunTuner() - (see the Cy_CapSense_RunTuner() function
* description for more details), the sensor must be pre-configured again by
* using the Cy_CapSense_SetupWidgetExt() prior to calling the
* Cy_CapSense_ScanExt() function.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
*******************************************************************************/
cy_status Cy_CapSense_ScanExt(cy_stc_capsense_context_t * context)
{
    cy_status scanStatus = CY_RET_SUCCESS;

    if (CY_CAPSENSE_SW_STS_BUSY == Cy_CapSense_IsBusy(context))
    {
        /* Previous widget is being scanned. Return error. */
        scanStatus = CY_RET_INVALID_STATE;
    }
    else
    {
        /* Check widget sensing method and call appropriate functions */
        switch(context->ptrActiveScanSns->ptrWdConfig->senseMethod)
        {
            case (uint8_t)CY_CAPSENSE_SENSE_METHOD_CSD_E:
                Cy_CapSense_CSDScan(context);
                break;

            case (uint8_t)CY_CAPSENSE_SENSE_METHOD_CSX_E:
                Cy_CapSense_CSXScan(context);
                break;

            default:
                scanStatus = CY_RET_UNKNOWN;
                break;
        }
    }
    return (scanStatus);
}


/*******************************************************************************
* Function Name: Cy_CapSense_ScanAllWidgets
****************************************************************************//**
*
* Initializes the first enabled widget and scanning of all the sensors in the
* widget, then the same process is repeated for all the widgets in the 
* middleware; i.e. scanning of all the widgets in the middleware.
*
* The tasks of the Cy_CapSense_SetupWidget() and Cy_CapSense_Scan() functions 
* are executed by these functions. The status of a sensor scan must be checked
* using the Cy_CapSense_IsBusy() prior to starting a next scan
* or setting up another widget.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
* \return
* Returns the status of the operation:
* - CY_RET_SUCCESS - Scanning is successfully started.
* - CY_RET_BAD_PARAM - All the widgets are disabled.
* - CY_RET_INVALID_STATE - The previous scanning is not completed and the 
* - HW block is busy.
* - CY_RET_UNKNOWN - There are unknown errors.
*
*******************************************************************************/
cy_status Cy_CapSense_ScanAllWidgets(cy_stc_capsense_context_t * context)
{
    uint32_t wdgtIndex;
    cy_stc_active_scan_sns_t * ptrActive = context->ptrActiveScanSns;
    cy_status scanStatus = CY_RET_UNKNOWN;

    if (CY_CAPSENSE_SW_STS_BUSY == Cy_CapSense_IsBusy(context))
    {
        /* Previous widget is being scanned. Return error. */
        scanStatus = CY_RET_INVALID_STATE;
    }
    else
    {
        /*
        * Set up first widget. If widget returned error, 
        * set up next, continue same until widget does not return error.
        */
        for (wdgtIndex = 0u; wdgtIndex < context->ptrCommonConfig->numWd; wdgtIndex++)
        {
            scanStatus = Cy_CapSense_SetupWidget(wdgtIndex, context);
            if (CY_RET_SUCCESS == scanStatus)
            {
                ptrActive->scanScopeAll = CY_CAPSENSE_SCAN_SCOPE_ALL_WD;
                
                /* Initiate scan and quit loop */
                scanStatus = Cy_CapSense_Scan(context);

                break;
            }
        }
    }

    return (scanStatus);
}


/*******************************************************************************
* Function Name: Cy_CapSense_InternalPreCalculation
****************************************************************************//**
*
* Calculate different internal parameters, register values in advance to 
* speed up execution time.
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
cy_status Cy_CapSense_InternalPreCalculation(cy_stc_capsense_context_t * context)
{
    cy_status initStatus = CY_RET_SUCCESS;
    uint32_t wdgtIndex;
    uint32_t temp;
    
    if (CY_CAPSENSE_ENABLE == context->ptrCommonConfig->csdEn)
    {
        if ((context->ptrCommonConfig->portCmodPadNum == context->ptrCommonConfig->portCmodNum) &&
            (context->ptrCommonConfig->pinCmodPad == context->ptrCommonConfig->pinCmod))
        {
            context->ptrInternalContext->csdCmodConnection = (uint8_t)CY_CAPSENSE_CMODPAD_E;
        }
        else if ((context->ptrCommonConfig->portCshPadNum == context->ptrCommonConfig->portCmodNum) &&
                 (context->ptrCommonConfig->pinCshPad == context->ptrCommonConfig->pinCmod))
        {
            context->ptrInternalContext->csdCmodConnection = (uint8_t)CY_CAPSENSE_CTANKPAD_E;
        }
        else if ((context->ptrCommonConfig->portShieldPadNum == context->ptrCommonConfig->portCmodNum) &&
                 (context->ptrCommonConfig->pinShieldPad == context->ptrCommonConfig->pinCmod))
        {
            context->ptrInternalContext->csdCmodConnection = (uint8_t)CY_CAPSENSE_CSHIELDPAD_E;
        }
        else if ((context->ptrCommonConfig->portVrefExtPadNum == context->ptrCommonConfig->portCmodNum) &&
                 (context->ptrCommonConfig->pinVrefExtPad == context->ptrCommonConfig->pinCmod))
        {
            context->ptrInternalContext->csdCmodConnection = (uint8_t)CY_CAPSENSE_VREFEXTPAD_E;
        }
        else
        {
            context->ptrInternalContext->csdCmodConnection = (uint8_t)CY_CAPSENSE_CMODPAD_E;
            initStatus = CY_RET_BAD_PARAM;
        }

        if(CY_CAPSENSE_ENABLE == context->ptrCommonConfig->csdCTankShieldEn)
        {
            /* Csh Capacitor Placement */
            if ((context->ptrCommonConfig->portCmodPadNum == context->ptrCommonConfig->portCshNum) &&
                (context->ptrCommonConfig->pinCmodPad == context->ptrCommonConfig->pinCsh))
            {
                context->ptrInternalContext->csdCshConnection = (uint8_t)CY_CAPSENSE_CMODPAD_E;
            }
            else if ((context->ptrCommonConfig->portCshPadNum == context->ptrCommonConfig->portCshNum) &&
                     (context->ptrCommonConfig->pinCshPad == context->ptrCommonConfig->pinCsh))
            {
                context->ptrInternalContext->csdCshConnection = (uint8_t)CY_CAPSENSE_CTANKPAD_E;
            }
            else if ((context->ptrCommonConfig->portShieldPadNum == context->ptrCommonConfig->portCshNum) &&
                     (context->ptrCommonConfig->pinShieldPad == context->ptrCommonConfig->pinCsh))
            {
                context->ptrInternalContext->csdCshConnection = (uint8_t)CY_CAPSENSE_CSHIELDPAD_E;
            }
            else if ((context->ptrCommonConfig->portVrefExtPadNum == context->ptrCommonConfig->portCshNum) &&
                     (context->ptrCommonConfig->pinVrefExtPad == context->ptrCommonConfig->pinCsh))
            {
                context->ptrInternalContext->csdCshConnection = (uint8_t)CY_CAPSENSE_VREFEXTPAD_E;
            }
            else
            {
                context->ptrInternalContext->csdCshConnection = (uint8_t)CY_CAPSENSE_CTANKPAD_E;
                initStatus = CY_RET_BAD_PARAM;
            }
        }
        
        /* Switch CSD Comparator Selection */
        switch (context->ptrInternalContext->csdCmodConnection)
        {
            case (uint8_t)CY_CAPSENSE_CMODPAD_E:
                context->ptrInternalContext->csdRegSwCmpPSel = CY_CAPSENSE_CSD_SW_CMP_P_SEL_SW_SFPM_STATIC_CLOSE;
                break;
            case (uint8_t)CY_CAPSENSE_CSHIELDPAD_E:
                context->ptrInternalContext->csdRegSwCmpPSel = CY_CAPSENSE_CSD_SW_CMP_P_SEL_SW_SFPS_STATIC_CLOSE;
                break;
            default:
                context->ptrInternalContext->csdRegSwCmpPSel = CY_CAPSENSE_CSD_SW_CMP_P_SEL_SW_SFPT_STATIC_CLOSE;
                break;
        }

        /* Defines the drive mode of pins depending on the Inactive sensor connection setting */
        switch (context->ptrCommonConfig->csdInactiveSnsConnection)
        {
            case CY_CAPSENSE_SNS_CONNECTION_HIGHZ:
                context->ptrInternalContext->csdInactiveSnsDm = CY_GPIO_DM_ANALOG;
                context->ptrInternalContext->csdInactiveSnsHsiom = CY_CAPSENSE_HSIOM_SEL_GPIO;
                break;
            case CY_CAPSENSE_SNS_CONNECTION_SHIELD:
                context->ptrInternalContext->csdInactiveSnsDm = CY_GPIO_DM_STRONG_IN_OFF;
                context->ptrInternalContext->csdInactiveSnsHsiom = CY_CAPSENSE_HSIOM_SEL_CSD_SHIELD;
                break;
            case CY_CAPSENSE_SNS_CONNECTION_GROUND:
            default:
                context->ptrInternalContext->csdInactiveSnsDm = CY_GPIO_DM_STRONG;
                context->ptrInternalContext->csdInactiveSnsHsiom = CY_CAPSENSE_HSIOM_SEL_GPIO;
                break;
        }
        
        /* Prepares CONFIG register value */
        context->ptrInternalContext->csdRegConfig = 0u;
        /* Iref Source */
        if (CY_CAPSENSE_IREF_PASS == context->ptrCommonConfig->ssIrefSource)
        {
            context->ptrInternalContext->csdRegConfig |= CSD_CONFIG_IREF_SEL_Msk;
        }
        /* Filter Delay */
        temp = context->ptrCommonConfig->periClkHz / context->ptrCommonContext->modCsdClk;
        if (temp > CY_CAPSENSE_MOD_CSD_CLK_24000000_HZ)
        {
            context->ptrInternalContext->csdRegConfig |= CY_CAPSENSE_CSD_CONFIG_FILTER_DELAY_48MHZ;
        }
        else if (temp > CY_CAPSENSE_MOD_CSD_CLK_12000000_HZ)
        {
            context->ptrInternalContext->csdRegConfig |= CY_CAPSENSE_CSD_CONFIG_FILTER_DELAY_24MHZ;
        }
        else
        {
            context->ptrInternalContext->csdRegConfig |= CY_CAPSENSE_CSD_CONFIG_FILTER_DELAY_12MHZ;
        }
        /* Shield Delay */
        context->ptrInternalContext->csdRegConfig |= (((uint32_t)context->ptrCommonConfig->csdShieldDelay) << CY_CAPSENSE_CSD_CONFIG_SHIELD_DELAY_POS);
        /* Sense Enable */
        context->ptrInternalContext->csdRegConfig |= CSD_CONFIG_SENSE_EN_Msk;
        /* Power On */
        context->ptrInternalContext->csdRegConfig |= CSD_CONFIG_ENABLE_Msk;
        
        /* High Speed comparator positive input selection */
        context->ptrInternalContext->csdRegSwHsPSelScan = 0u;
        context->ptrInternalContext->csdRegSwHsPSelInit = 0u;
        /* Switch Comparator Selection */
        switch (context->ptrInternalContext->csdCmodConnection)
        {
            case (uint8_t)CY_CAPSENSE_CMODPAD_E:
                context->ptrInternalContext->csdRegSwHsPSelInit = CY_CAPSENSE_CSD_SW_HS_P_SEL_SW_HMPM_STATIC_CLOSE;
                break;
            case (uint8_t)CY_CAPSENSE_CSHIELDPAD_E:
                context->ptrInternalContext->csdRegSwHsPSelInit = CY_CAPSENSE_CSD_SW_HS_P_SEL_SW_HMPS_STATIC_CLOSE;
                break;
            default:
                context->ptrInternalContext->csdRegSwHsPSelInit = CY_CAPSENSE_CSD_SW_HS_P_SEL_SW_HMPT_STATIC_CLOSE;
                break;
        }
        if (0u != context->ptrCommonConfig->csdShieldEn)
        {
            context->ptrInternalContext->csdRegSwHsPSelScan = CY_CAPSENSE_CSD_SW_HS_P_SEL_SW_HMMB_STATIC_CLOSE;
        }
        
        /* BYP switch selection */
        context->ptrInternalContext->csdRegSwBypSel = CY_CAPSENSE_CSD_SW_BYP_SEL_SW_BYA_MSK;
        if (0u != context->ptrCommonConfig->csdShieldEn)
        {
            context->ptrInternalContext->csdRegSwBypSel = CY_CAPSENSE_CSD_SW_BYP_SEL_SW_BYA_MSK | CY_CAPSENSE_CSD_SW_BYP_SEL_SW_BYB_MSK;
        }
        
        /* Switch resistance selection */
        context->ptrInternalContext->csdRegSwResScan = ((uint32_t)context->ptrCommonConfig->csdShieldSwRes << CSD_SW_RES_RES_HCBV_Pos) |
                                                     ((uint32_t)context->ptrCommonConfig->csdShieldSwRes << CSD_SW_RES_RES_HCBG_Pos);
        /* Switch resistance selection */
        context->ptrInternalContext->csdRegSwResInit = ((uint32_t)context->ptrCommonConfig->csdInitSwRes << CSD_SW_RES_RES_HCAV_Pos);
        
        if (CY_CAPSENSE_ENABLE == context->ptrCommonConfig->csdShieldEn)
        {
            if (CY_CAPSENSE_2000_MV > context->ptrCommonConfig->vdda)
            {
             
                context->ptrInternalContext->csdRegAmuxbufInit = CY_CAPSENSE_CSD_AMBUF_PWR_MODE_NORM;
            }
            else
            {
                context->ptrInternalContext->csdRegAmuxbufInit = CY_CAPSENSE_CSD_AMBUF_PWR_MODE_HI;
            }
        }
        else
        {
            context->ptrInternalContext->csdRegAmuxbufInit = CY_CAPSENSE_CSD_AMBUF_PWR_MODE_OFF;
        }

        /* Switch AMUXBUF selection */
        context->ptrInternalContext->csdRegSwAmuxbufSel = 0u;
        if (0u != context->ptrCommonConfig->csdShieldEn)
        {
            if (((CY_CAPSENSE_2000_MV > context->ptrCommonConfig->vdda) &&
                 (CY_CAPSENSE_IDAC_SINKING == context->ptrCommonConfig->csdChargeTransfer)) ||
                ((CY_CAPSENSE_2000_MV <= context->ptrCommonConfig->vdda) &&
                 (CY_CAPSENSE_IDAC_SOURCING == context->ptrCommonConfig->csdChargeTransfer)))
            {
                context->ptrInternalContext->csdRegSwAmuxbufSel = CY_CAPSENSE_CSD_SW_AMUXBUF_SEL_SW_IRH_STATIC_CLOSE | CY_CAPSENSE_CSD_SW_AMUXBUF_SEL_SW_ICB_PHI2;
            }
        }
        /* Switch Shield selection */
        context->ptrInternalContext->csdRegSwShieldSelInit = CY_CAPSENSE_CSD_SW_SHIELD_SEL_SW_HCAV_HSCMP;
        context->ptrInternalContext->csdRegSwShieldSelScan = 0u;
        if (0u != context->ptrCommonConfig->csdShieldEn)
        {
            if (0u != context->ptrCommonConfig->csdCTankShieldEn)
            {
                if (CY_CAPSENSE_IDAC_SINKING == context->ptrCommonConfig->csdChargeTransfer)
                {
                    context->ptrInternalContext->csdRegSwShieldSelScan = CY_CAPSENSE_CSD_SW_SHIELD_SEL_SW_HCBG_HSCMP;
                }
                else
                {
                    context->ptrInternalContext->csdRegSwShieldSelScan = CY_CAPSENSE_CSD_SW_SHIELD_SEL_SW_HCBV_HSCMP;
                }
            }
            else
            {
                if (CY_CAPSENSE_IDAC_SINKING == context->ptrCommonConfig->csdChargeTransfer)
                {
                    context->ptrInternalContext->csdRegSwShieldSelScan = CY_CAPSENSE_CSD_SW_SHIELD_SEL_SW_HCBV_PHI1 |
                                                                         CY_CAPSENSE_CSD_SW_SHIELD_SEL_SW_HCBG_PHI2_HSCMP;
                }
                else
                {
                    context->ptrInternalContext->csdRegSwShieldSelScan = CY_CAPSENSE_CSD_SW_SHIELD_SEL_SW_HCBG_PHI1 |
                                                                         CY_CAPSENSE_CSD_SW_SHIELD_SEL_SW_HCBV_PHI2_HSCMP;
                }
            }
        }

        /* High-Speed Comparator config */
        context->ptrInternalContext->csdRegHscmpInit = CY_CAPSENSE_CSD_HSCMP_HSCMP_EN_MSK;

        if (CY_CAPSENSE_IDAC_SINKING == context->ptrCommonConfig->csdChargeTransfer)
        {
            context->ptrInternalContext->csdRegHscmpScan = CY_CAPSENSE_CSD_HSCMP_HSCMP_EN_MSK | 
                                                           CY_CAPSENSE_CSD_HSCMP_HSCMP_INVERT_MSK;
        }
        else
        {
            context->ptrInternalContext->csdRegHscmpScan = CY_CAPSENSE_CSD_HSCMP_HSCMP_EN_MSK;
        }


        /* IDACs Config */
        context->ptrInternalContext->csdIdacAConfig = 0u;
        context->ptrInternalContext->csdIdacBConfig = 0u;

        /* IDACs Ranges */
        temp = context->ptrCommonConfig->csdIdacGainInit;
        /* Initialize idacGain parameters only for CSD widgets */
        for (wdgtIndex = context->ptrCommonConfig->numWd; wdgtIndex-- > 0u;)
        {
            if ((uint8_t)CY_CAPSENSE_SENSE_METHOD_CSD_E == context->ptrWdConfig[wdgtIndex].senseMethod)
            {
                context->ptrWdContext[wdgtIndex].idacGainIndex = (uint8_t)temp;
            }
        }
        context->ptrInternalContext->csdIdacAConfig |= context->ptrCommonConfig->idacGainTable[temp].gainReg;
        context->ptrInternalContext->csdIdacBConfig |= context->ptrCommonConfig->idacGainTable[temp].gainReg;

        /* IDACs Polarities */
        if (CY_CAPSENSE_IDAC_SINKING == context->ptrCommonConfig->csdChargeTransfer)
        {
            context->ptrInternalContext->csdIdacAConfig |= (CY_CAPSENSE_CSD_IDACA_POLARITY_VDDA_SNK << CY_CAPSENSE_CSD_IDACA_POLARITY_POS);
            context->ptrInternalContext->csdIdacBConfig |= (CY_CAPSENSE_CSD_IDACB_POLARITY_VDDA_SNK << CY_CAPSENSE_CSD_IDACB_POLARITY_POS);
        }
        else
        {
            context->ptrInternalContext->csdIdacAConfig |= (CY_CAPSENSE_CSD_IDACA_POLARITY_VSSA_SRC << CY_CAPSENSE_CSD_IDACA_POLARITY_POS);
            context->ptrInternalContext->csdIdacBConfig |= (CY_CAPSENSE_CSD_IDACB_POLARITY_VSSA_SRC << CY_CAPSENSE_CSD_IDACB_POLARITY_POS);
        }
        /* IDACs Balancing Mode */
        context->ptrInternalContext->csdIdacAConfig |= (CY_CAPSENSE_CSD_IDACA_BALL_MODE_FULL << CY_CAPSENSE_CSD_IDACA_BALL_MODE_POS);
        context->ptrInternalContext->csdIdacBConfig |= (CY_CAPSENSE_CSD_IDACB_BALL_MODE_FULL << CY_CAPSENSE_CSD_IDACB_BALL_MODE_POS);
        /* Legs Config */
        context->ptrInternalContext->csdIdacAConfig |= (CY_CAPSENSE_CSD_IDACA_LEG1_MODE_CSD << CY_CAPSENSE_CSD_IDACA_LEG1_MODE_POS);
        context->ptrInternalContext->csdIdacAConfig |= (CY_CAPSENSE_CSD_IDACA_LEG2_MODE_CSD << CY_CAPSENSE_CSD_IDACA_LEG2_MODE_POS);
        context->ptrInternalContext->csdIdacAConfig |= (CY_CAPSENSE_CSD_IDACA_LEG1_EN_MSK);
        
        context->ptrInternalContext->csdIdacBConfig |= (CY_CAPSENSE_CSD_IDACB_LEG1_MODE_CSD_STATIC << CY_CAPSENSE_CSD_IDACB_LEG1_MODE_POS);
        context->ptrInternalContext->csdIdacBConfig |= (CY_CAPSENSE_CSD_IDACB_LEG2_MODE_CSD_STATIC << CY_CAPSENSE_CSD_IDACB_LEG2_MODE_POS);
        if (0u != context->ptrCommonConfig->csdIdacCompEn)
        {
            context->ptrInternalContext->csdIdacBConfig |= (CY_CAPSENSE_CSD_IDACB_LEG1_EN_MSK);
        }

        /* Switch Csh tank selection */
        context->ptrInternalContext->csdRegSwFwTankSelScan = 0u;
        context->ptrInternalContext->csdRegSwFwTankSelInit = 0u;
        if ((0u != context->ptrCommonConfig->csdShieldEn) &&
            (0u != context->ptrCommonConfig->csdCTankShieldEn) &&
            ((uint8_t)CY_CAPSENSE_CTANKPAD_E == context->ptrInternalContext->csdCshConnection))
        {
            context->ptrInternalContext->csdRegSwFwTankSelScan = (CY_CAPSENSE_CSD_SW_FW_TANK_SEL_SW_F2PT_STATIC_CLOSE |
                                                                  CY_CAPSENSE_CSD_SW_FW_TANK_SEL_SW_F2CB_STATIC_CLOSE);
        }

        if (((uint8_t)CY_CAPSENSE_CTANKPAD_E == context->ptrInternalContext->csdCmodConnection) || 
            ((uint8_t)CY_CAPSENSE_CTANKPAD_E == context->ptrInternalContext->csdCshConnection))
        {
            context->ptrInternalContext->csdRegSwFwTankSelInit = (CY_CAPSENSE_CSD_SW_FW_TANK_SEL_SW_F2PT_STATIC_CLOSE | 
                                                                  CY_CAPSENSE_CSD_SW_FW_TANK_SEL_SW_F2MA_STATIC_CLOSE |
                                                                  CY_CAPSENSE_CSD_SW_FW_TANK_SEL_SW_F2CA_STATIC_CLOSE);
        }

        /* IO Control Selection */
        if (CY_CAPSENSE_IDAC_SINKING == context->ptrCommonConfig->csdChargeTransfer)
        {
            if (0u != context->ptrCommonConfig->csdShieldEn)
            {
                context->ptrInternalContext->csdRegIoSel = (CY_CAPSENSE_CSD_TX_N_OUT_EN_PHI1 | 
                                                            CY_CAPSENSE_CSD_TX_N_AMUXA_EN_PHI2 |
                                                            CY_CAPSENSE_CSD_TX_OUT_EN_PHI1_DELAY | 
                                                            CY_CAPSENSE_CSD_TX_AMUXB_EN_PHI2_DELAY |
                                                            CY_CAPSENSE_CSD_TX_OUT_MSK | 
                                                            CY_CAPSENSE_CSD_TX_N_OUT_STATIC_CLOSE);
            }
            else
            {
                context->ptrInternalContext->csdRegIoSel = (CY_CAPSENSE_CSD_TX_N_OUT_EN_PHI1 | 
                                                            CY_CAPSENSE_CSD_TX_N_AMUXA_EN_PHI2 |
                                                            CY_CAPSENSE_CSD_TX_N_OUT_STATIC_CLOSE);
            }
        }
        else
        {
            if (0u != context->ptrCommonConfig->csdShieldEn)
            {
                context->ptrInternalContext->csdRegIoSel = (CY_CAPSENSE_CSD_TX_N_OUT_EN_PHI1 | 
                                                            CY_CAPSENSE_CSD_TX_N_AMUXA_EN_PHI2 |
                                                            CY_CAPSENSE_CSD_TX_OUT_EN_PHI1_DELAY | 
                                                            CY_CAPSENSE_CSD_TX_AMUXB_EN_PHI2_DELAY);
            }
            else
            {
                context->ptrInternalContext->csdRegIoSel = (CY_CAPSENSE_CSD_TX_N_OUT_EN_PHI1 | 
                                                            CY_CAPSENSE_CSD_TX_N_AMUXA_EN_PHI2);
            }
        }

        if(context->ptrCommonConfig->csdVref == 0u)
        {
            /*
            * Get the recommended Vref voltage value if the Custom Vref option is not used.
            */
            temp = Cy_CapSense_GetVrefAutoMv((uint32_t)context->ptrCommonConfig->vdda);
        }
        else
        {
            /*
            * Initialize the Temp variable with the user defined Vref voltage if the
            * Custom Vref option is used.
            */
            temp = context->ptrCommonConfig->csdVref;
        }

        if(CY_CAPSENSE_VREF_SRSS == context->ptrCommonConfig->ssVrefSource)
        {
            /*
            * Calculate the VrefHigh Gain for the desired Vref voltage when SRSS is used as the Vref source. 
            */
            temp = (uint8_t)Cy_CapSense_GetVrefHighGain((uint32_t)context->ptrCommonConfig->vdda,
                                                                                CY_CAPSENSE_VREF_SRSS_MV, temp);
            /*
            * Calculates the VrefHigh voltage, when SRSS is used as the Vref source.
            */
            context->ptrInternalContext->csdVrefVoltageMv = (uint16_t)Cy_CapSense_GetVrefHighMv(CY_CAPSENSE_VREF_SRSS_MV, temp);
        }
        else if(CY_CAPSENSE_VREF_PASS == context->ptrCommonConfig->ssVrefSource)
        {
            /*
            * Calculate the VrefHigh Gain for the desired Vref voltage when PASS is used as the Vref source. 
            */
            temp = (uint8_t)Cy_CapSense_GetVrefHighGain((uint32_t)context->ptrCommonConfig->vdda,
                                                                                CY_CAPSENSE_VREF_PASS_MV, temp);
            /*
            * Calculates the VrefHigh voltage, when SRSS is used as the Vref source.
            */
            context->ptrInternalContext->csdVrefVoltageMv = (uint16_t)Cy_CapSense_GetVrefHighMv(CY_CAPSENSE_VREF_PASS_MV, temp);
        }
        else
        {
            /*
            * Pass the Vref source voltage to the VrefHigh output in case if the Vref source in unknown. 
            */
            temp = (uint8_t)(CY_CAPSENSE_VREF_GAIN_MAX - 1u);
        }

        context->ptrInternalContext->csdVrefGain = (uint8_t)temp;
        context->ptrInternalContext->csdRegRefgen  = (CSD_REFGEN_REFGEN_EN_Msk |
                                                      CSD_REFGEN_RES_EN_Msk |
                                                      (temp << CSD_REFGEN_GAIN_Pos));
    }    

    /* Common initialization */
    
    /* Initialize RefGen Gain and Vref switches for sensing */
    if(CY_CAPSENSE_VREF_SRSS != context->ptrCommonConfig->ssVrefSource)
    {
        context->ptrInternalContext->csxRegRefgen = CY_CAPSENSE_DEFAULT_CSD_REFGEN_CFG | 0x00001F00uL;
        context->ptrInternalContext->regSwRefGenSel = 0x00100000uL;
    }
    else
    {
        context->ptrInternalContext->csxRegRefgen  = CY_CAPSENSE_DEFAULT_CSD_REFGEN_CFG | 0x00001500uL;
        context->ptrInternalContext->regSwRefGenSel = 0x10000000uL;
    }

    /* Initialize maxRawCount for all CSX widgets */
    for (wdgtIndex = context->ptrCommonConfig->numWd; wdgtIndex-- > 0u;)
    {
        if ((uint8_t)CY_CAPSENSE_SENSE_METHOD_CSD_E == context->ptrWdConfig[wdgtIndex].senseMethod)
        {
            context->ptrWdContext[wdgtIndex].maxRawCount = 
                            (uint16_t)(((uint32_t)0x01u << context->ptrWdContext[wdgtIndex].resolution) - 1u);
        }
        if ((uint8_t)CY_CAPSENSE_SENSE_METHOD_CSX_E == context->ptrWdConfig[wdgtIndex].senseMethod)
        {
            context->ptrWdContext[wdgtIndex].maxRawCount = (uint16_t)(context->ptrWdContext[wdgtIndex].resolution * 
                            (context->ptrWdContext[wdgtIndex].snsClk - CY_CAPSENSE_CSX_DEADBAND_CYCLES_NUMBER));
        }
    }

    if (CY_CAPSENSE_ENABLE == context->ptrCommonConfig->csxEn)
    {
        
        /* 
        * CSX Initialization 
        */
        /* Initialize CONFIG register values */
        context->ptrInternalContext->csxRegConfigInit = CY_CAPSENSE_DEFAULT_CSD_CONFIG_CFG;
        context->ptrInternalContext->csxRegConfigScan = CY_CAPSENSE_PRESCAN_CSD_CONFIG_CFG;
        
        if(CY_CAPSENSE_IREF_SRSS != context->ptrCommonConfig->ssIrefSource)
        {
            context->ptrInternalContext->csxRegConfigInit |= 0x00000001uL;
            context->ptrInternalContext->csxRegConfigScan |= 0x00000001uL;
        }

        /* Initialize FilterDelay for CSX sensing */    
        if (CY_CAPSENSE_MOD_CSD_CLK_12000000_HZ < context->ptrCommonConfig->periClkHz)
        {
            /* Calculate CSX modClk in Hz */
            temp = context->ptrCommonConfig->periClkHz / (uint32_t) context->ptrCommonContext->modCsxClk;
            if(temp <= CY_CAPSENSE_MOD_CSD_CLK_12000000_HZ)
            {
                context->ptrInternalContext->csxRegConfigScan |= CY_CAPSENSE_CSD_CONFIG_FILTER_DELAY_12MHZ;
            }
            else if(temp <= CY_CAPSENSE_MOD_CSD_CLK_24000000_HZ)
            {
                context->ptrInternalContext->csxRegConfigScan |= CY_CAPSENSE_CSD_CONFIG_FILTER_DELAY_24MHZ;
            }
            else
            {
                context->ptrInternalContext->csxRegConfigScan |= CY_CAPSENSE_CSD_CONFIG_FILTER_DELAY_48MHZ;
            }
        }
        else
        {
            context->ptrInternalContext->csxRegConfigScan |= CY_CAPSENSE_CSD_CONFIG_FILTER_DELAY_12MHZ;
        }

        /* Default switch resistance selection */
        context->ptrInternalContext->csxRegSwResInit = 
        (((uint32_t)context->ptrCommonConfig->csxInitShieldSwRes << CSD_SW_RES_RES_HCAV_Pos) |
         ((uint32_t)context->ptrCommonConfig->csxInitShieldSwRes << CSD_SW_RES_RES_HCAG_Pos) |
         ((uint32_t)context->ptrCommonConfig->csxInitShieldSwRes << CSD_SW_RES_RES_HCBV_Pos) |
         ((uint32_t)context->ptrCommonConfig->csxInitShieldSwRes << CSD_SW_RES_RES_HCBG_Pos) |
         ((uint32_t)context->ptrCommonConfig->csxInitSwRes << CSD_SW_RES_RES_F1PM_Pos) |
         ((uint32_t)context->ptrCommonConfig->csxInitSwRes << CSD_SW_RES_RES_F2PT_Pos));
        /* Switch resistance selection for scan */
        context->ptrInternalContext->csxRegSwResScan = 
        (((uint32_t)context->ptrCommonConfig->csxScanShieldSwRes << CSD_SW_RES_RES_HCAV_Pos) |
         ((uint32_t)context->ptrCommonConfig->csxScanShieldSwRes << CSD_SW_RES_RES_HCAG_Pos) |
         ((uint32_t)context->ptrCommonConfig->csxScanShieldSwRes << CSD_SW_RES_RES_HCBV_Pos) |
         ((uint32_t)context->ptrCommonConfig->csxScanShieldSwRes << CSD_SW_RES_RES_HCBG_Pos) |
         ((uint32_t)context->ptrCommonConfig->csxScanSwRes << CSD_SW_RES_RES_F1PM_Pos) |
         ((uint32_t)context->ptrCommonConfig->csxScanSwRes << CSD_SW_RES_RES_F2PT_Pos));
        /* Switch resistance selection for precharge */
        context->ptrInternalContext->csxRegSwResPrech = context->ptrInternalContext->csxRegSwResInit;
    }
    
    return (initStatus);
}


/*******************************************************************************
* Function Name: Cy_CapSense_SsInitialize
****************************************************************************//**
*
* Performs hardware and firmware initialization required for proper operation
* of the CapSense middleware. This function is called from
* the Cy_CapSense_Init() prior to calling any other function of the middleware.
*
* This function 
* 1. Depending on the configuration, the function initializes the CSD block
* for the corresponding sensing mode.
* 2. Sets all IOs into the default state.
* 3. Performs pre-calculations of register values depending on configuration.
*
* Calling the Cy_CapSense_Init() is the recommended method to initialize
* the CapSense middleware at power-up. The Cy_CapSense_SsInitialize()
* should not be used for initialization, resume, or wake-up operations.
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
cy_status Cy_CapSense_SsInitialize(cy_stc_capsense_context_t * context)
{
    cy_status initStatus;
    
    initStatus = Cy_CapSense_InternalPreCalculation(context);
    
    Cy_CapSense_InitializeSourceSenseClk(context);

    /* Set all IO states to default state */
    Cy_CapSense_SetIOsInDefaultState(context);
    Cy_CapSense_SetSpecificIOsInDefaultState(context);

    /*
     * CSD HW block is initialized in the Setup Widget functions based
     * on widget sensing method. 
     */
    Cy_CapSense_SwitchSensingMode((uint8_t)CY_CAPSENSE_UNDEFINED_E, context);

    return (initStatus);
}


/*******************************************************************************
* Function Name: Cy_CapSense_SetPinState
****************************************************************************//**
*
* Sets the state (drive mode and HSIOM state) of the electrode used by a sensor.
* The possible states are GND, Shield, High-Z, Tx, Rx, and Sensor. If the 
* electrode specified in the input parameter is a ganged one, then the state 
* of all pins associated with the ganged electrode is updated.
*
* This function sets a specified state for a specified sensor electrode. For the
* CSD of button or slider type widgets, the sensor electrode is a sensor ID.
* For the CSD matrix button or touchpad type widgets, the sensor electrode is row or
* column ID. For the CSX widgets, it is either an Rx or Tx electrode ID. 
* If the specified electrode is a ganged one, then the specified state is set 
* to which all the pins belong.
* This function must not be called while the middleware is in the busy state.
*
* This function accepts the CY_CAPSENSE_SHIELD and
* CY_CAPSENSE_SENSOR states as an input only if there is at least
* one CSD widget configured. Similarly, this function accepts the 
* CY_CAPSENSE_TX_PIN and CY_CAPSENSE_RX_PIN states as an input only 
* if there is at least one CSX widget in the project.
*
* Calling this function directly from the application layer is not
* recommended. This function is used to implement only the custom-specific
* use cases. Functions that perform a setup and scan of a sensor/widget
* automatically set the required pin states. They ignore changes
* in the design made by the Cy_CapSense_SetPinState() function.
* This function checks widgetId and sensorElement for correctness.
*
* \param widgetId
* Specifies the ID of the widget to change the pin state of the specified
* sensor.
*
* \param sensorElement
* Specifies the ID of the sensor element within the widget to change
* its pin state.
* For the CSD widgets, sensorElement is the sensor ID.
* For the CSX widgets, sensorElement is defined either as Rx ID or Tx ID.
* The first Rx in a widget corresponds to sensorElement = 0; the second
* Rx in a widget corresponds to sensorElement = 1, and so on.
* The last Tx in a widget corresponds to sensorElement = (RxNum + TxNum - 1).
*
* \param state
* Specifies the state of the sensor to be set:
* 1. CY_CAPSENSE_GROUND - The pin is connected to the ground.
* 2. CY_CAPSENSE_HIGHZ - The drive mode of the pin is set to High-Z
* Analog.
* 3. CY_CAPSENSE_SHIELD - The shield signal is routed to the pin
* (available only if CSD sensing method with shield electrode is enabled).
* 4. CY_CAPSENSE_SENSOR - The pin is connected to the scanning bus
* (available only if CSD sensing method is enabled).
* 5. CY_CAPSENSE_TX_PIN - The Tx signal is routed to the sensor
* (available only if CSX sensing method is enabled).
* 6. CY_CAPSENSE_RX_PIN - The pin is connected to the scanning bus
* (available only if CSX sensing method is enabled).
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
* \return status
* Returns the operation status:
* - CY_RET_SUCCESS - Indicates the successful electrode setting.
* - CY_RET_BAD_PARAM - 1) widgetID or sensorElement are not valid;
* 2) the CSD sensing method is disabled for desired 
* CY_CAPSENSE_SHIELD or CY_CAPSENSE_SENSOR states;
* 3) the CSX sensing method is disabled for desired 
* CY_CAPSENSE_TX_PIN or CY_CAPSENSE_RX_PIN states.
*
*******************************************************************************/
cy_status Cy_CapSense_SetPinState(
                uint32_t widgetId, 
                uint32_t sensorElement, 
                uint32_t state, 
                cy_stc_capsense_context_t * context)
{
    cy_status connState = CY_RET_BAD_PARAM;
    uint32_t eltdNum;
    uint32_t eltdIndex;
    uint32_t interruptState;
    cy_stc_capsense_pin_config_t const * ioPtr;
    cy_stc_capsense_widget_config_t const * ptrWdCfg = &context->ptrWdConfig[widgetId];
    
    if (context->ptrCommonConfig->numWd > widgetId)
    {
        /* Get number of electrodes within the widget */
        eltdNum = (uint32_t)ptrWdCfg->numCols + ptrWdCfg->numRows;
        if (((uint8_t)CY_CAPSENSE_CSD_SENSING_METHOD == (uint32_t)ptrWdCfg->senseMethod) &&
           (((uint8_t)CY_CAPSENSE_WD_BUTTON_E == ptrWdCfg->wdType) || 
            ((uint8_t)CY_CAPSENSE_WD_LINEAR_SLIDER_E == ptrWdCfg->wdType) ||
            ((uint8_t)CY_CAPSENSE_WD_RADIAL_SLIDER_E == ptrWdCfg->wdType)))
        {
            eltdNum = context->ptrWdConfig->numSns;
        }
        
        if (eltdNum > sensorElement)
        {
            /* Get number of electrode's pins and a pointer to the electrode's pin structure */
            eltdNum = ptrWdCfg->ptrEltdConfig->numPins;
            ioPtr = ptrWdCfg->ptrEltdConfig->ptrPin;
            /* Loop through all pins of the specified sensor electrode */
            for (eltdIndex = 0u; eltdIndex < eltdNum; eltdIndex++)
            {
                /* Reset HSIOM and PC registers */
                interruptState = Cy_SysLib_EnterCriticalSection();
                Cy_GPIO_SetHSIOM(ioPtr->pcPtr, (uint32_t)ioPtr->pinNumber, CY_CAPSENSE_HSIOM_SEL_GPIO);
                Cy_SysLib_ExitCriticalSection(interruptState);

                switch (state)
                {
                    case CY_CAPSENSE_GROUND:
                        interruptState = Cy_SysLib_EnterCriticalSection();
                        Cy_GPIO_SetDrivemode(ioPtr->pcPtr, (uint32_t)ioPtr->pinNumber, CY_GPIO_DM_STRONG_IN_OFF);
                        Cy_GPIO_Clr(ioPtr->pcPtr, (uint32_t)ioPtr->pinNumber);
                        Cy_SysLib_ExitCriticalSection(interruptState);
                        
                        connState = CY_RET_SUCCESS;
                        break;

                    case CY_CAPSENSE_HIGHZ:
                        interruptState = Cy_SysLib_EnterCriticalSection();
                        Cy_GPIO_SetDrivemode(ioPtr->pcPtr, (uint32_t)ioPtr->pinNumber, CY_GPIO_DM_ANALOG);
                        Cy_GPIO_Clr(ioPtr->pcPtr, (uint32_t)ioPtr->pinNumber);
                        Cy_SysLib_ExitCriticalSection(interruptState);
                        
                        connState = CY_RET_SUCCESS;
                        break;

                    case CY_CAPSENSE_SENSOR:
                        if (CY_CAPSENSE_ENABLE == context->ptrCommonConfig->csdEn)
                        {
                            /* Enable sensor */
                            Cy_CapSense_CSDConnectSns(ioPtr, context);
                            
                            connState = CY_RET_SUCCESS;
                        }
                        break;

                    case CY_CAPSENSE_SHIELD:
                        if (CY_CAPSENSE_ENABLE == context->ptrCommonConfig->csdShieldEn)
                        {
                            Cy_CapSense_SsConfigPinRegisters(ioPtr->pcPtr, (uint32_t)ioPtr->pinNumber, 
                                                             CY_GPIO_DM_STRONG_IN_OFF, CY_CAPSENSE_HSIOM_SEL_CSD_SHIELD);
                            connState = CY_RET_SUCCESS;
                        }
                        break;

                    case CY_CAPSENSE_TX_PIN:
                            
                        if (CY_CAPSENSE_ENABLE == context->ptrCommonConfig->csxEn)
                        {
                            Cy_CapSense_SsConfigPinRegisters(ioPtr->pcPtr, (uint32_t)ioPtr->pinNumber, 
                                                             CY_GPIO_DM_STRONG_IN_OFF, CY_CAPSENSE_HSIOM_SEL_CSD_SHIELD);
                            connState = CY_RET_SUCCESS;
                        }
                        break;

                    case CY_CAPSENSE_RX_PIN:
                        if (CY_CAPSENSE_ENABLE == context->ptrCommonConfig->csxEn)
                        {
                            Cy_CapSense_SsConfigPinRegisters(ioPtr->pcPtr, (uint32_t)ioPtr->pinNumber, 
                                                             CY_GPIO_DM_ANALOG, CY_CAPSENSE_HSIOM_SEL_AMUXA);
                            connState = CY_RET_SUCCESS;
                        }
                        break;

                    default:
                        break;
                }

                ioPtr++;
            }
        }
    }
    return (connState);
}


/*******************************************************************************
* Function Name: Cy_CapSense_SwitchSensingMode
****************************************************************************//**
*
* This function changes the mode for case when both CSD and CSX widgets are
* scanned.
*
* Once the function is called it performs the following tasks:
* 1. Releasing HW block by existing sensing mode.
* 2. Configuring corresponding external capacitors and sensor pins into
*    default states.
* 3. Setting new mode.
* 4. Configuring HW to the new sensing mode.
*
* \param mode
* Specifies the scan mode:
* - CY_CAPSENSE_SENSE_METHOD_CSD_E
* - CY_CAPSENSE_SENSE_METHOD_CSX_E
* - CY_CAPSENSE_UNDEFINED_E
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
*******************************************************************************/
void Cy_CapSense_SwitchSensingMode(uint8_t mode, cy_stc_capsense_context_t * context)
{
    if (context->ptrActiveScanSns->currentSenseMethod != mode)
    {
        /* The requested mode differs from the current one. Disable the current mode. */
        switch(context->ptrActiveScanSns->currentSenseMethod)
        {
            case (uint8_t)CY_CAPSENSE_SENSE_METHOD_CSD_E:
                Cy_CapSense_CSDDisableMode(context);
                break;
            case (uint8_t)CY_CAPSENSE_SENSE_METHOD_CSX_E:
                Cy_CapSense_CSXDisableMode(context);
                break;
            default:
                break;
        }

        context->ptrActiveScanSns->currentSenseMethod = mode;
        
        /* Enable the specified mode */
        switch(context->ptrActiveScanSns->currentSenseMethod)
        {
            case (uint8_t)CY_CAPSENSE_SENSE_METHOD_CSD_E:
                Cy_CapSense_CSDInitialize(context);
                break;
            case (uint8_t)CY_CAPSENSE_SENSE_METHOD_CSX_E:
                Cy_CapSense_CSXInitialize(context);
                break;
            default:
                break;
        }
    }
}


/*******************************************************************************
* Function Name: Cy_CapSense_SetIOsInDefaultState
****************************************************************************//**
*
* Sets all CapSense pins into a default state.
*
* \details
* Sets all the CSD/CSX IOs into a default state:
* - HSIOM   - Disconnected, the GPIO mode.
* - DM      - Strong drive.
* - State   - Zero.
*
* Do not call this function directly from the application
* layer.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
*******************************************************************************/
void Cy_CapSense_SetIOsInDefaultState(const cy_stc_capsense_context_t * context)
{
    uint32_t loopIndex;
    uint32_t interruptState;
    const cy_stc_capsense_pin_config_t * ptrPinCfg = context->ptrPinConfig;

    /* Loop through all electrodes */
    for (loopIndex = 0u; loopIndex < context->ptrCommonConfig->numPin; loopIndex++)
    {
        interruptState = Cy_SysLib_EnterCriticalSection();
        Cy_GPIO_Pin_FastInit(ptrPinCfg->pcPtr, (uint32_t)ptrPinCfg->pinNumber, 
                CY_GPIO_DM_STRONG, 0u, CY_CAPSENSE_HSIOM_SEL_GPIO);
        Cy_SysLib_ExitCriticalSection(interruptState);

        /* Get next electrode */
        ptrPinCfg++;
    }
}


/*******************************************************************************
* Function Name: Cy_CapSense_SetSpecificIOsInDefaultState
****************************************************************************//**
*
* Sets all CapSense pins into a default state.
*
* \details
* Sets all external capacitors and shield electrodes into a default state:
* - HSIOM   - Disconnected, the GPIO mode.
* - DM      - High-Z Analog.
*
* Do not call this function directly from the application
* layer.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
*******************************************************************************/
void Cy_CapSense_SetSpecificIOsInDefaultState(const cy_stc_capsense_context_t * context)
{
    if (CY_CAPSENSE_ENABLE == context->ptrCommonConfig->csdEn)
    {
        /* Cmod pin to default state */
        Cy_CapSense_SsConfigPinRegisters(context->ptrCommonConfig->portCmod, (uint32_t)context->ptrCommonConfig->pinCmod, CY_GPIO_DM_ANALOG, CY_CAPSENSE_HSIOM_SEL_GPIO);

        if (CY_CAPSENSE_ENABLE == context->ptrCommonConfig->csdShieldEn)
        {
            /* Disconnect shields */
            Cy_CapSense_CSDDisableShieldElectrodes(context);
            
            if (CY_CAPSENSE_ENABLE == context->ptrCommonConfig->csdCTankShieldEn)
            {
                /* Csh pin to default state */
                Cy_CapSense_SsConfigPinRegisters(context->ptrCommonConfig->portCsh, (uint32_t)context->ptrCommonConfig->pinCsh, CY_GPIO_DM_ANALOG, CY_CAPSENSE_HSIOM_SEL_GPIO);
            }
        }
    }

    if (CY_CAPSENSE_ENABLE == context->ptrCommonConfig->csxEn)
    {
        /* CintA pin to default state */
        Cy_CapSense_SsConfigPinRegisters(context->ptrCommonConfig->portCintA, (uint32_t)context->ptrCommonConfig->pinCintA, CY_GPIO_DM_ANALOG, CY_CAPSENSE_HSIOM_SEL_GPIO);

        /* CintB pin to default state */
        Cy_CapSense_SsConfigPinRegisters(context->ptrCommonConfig->portCintB, (uint32_t)context->ptrCommonConfig->pinCintB, CY_GPIO_DM_ANALOG, CY_CAPSENSE_HSIOM_SEL_GPIO);
    }
}


/*******************************************************************************
* Function Name: Cy_CapSense_SsPostAllWidgetsScan
****************************************************************************//**
*
* Configures and triggers the scan of the next widget if queue is not empty.
* This function should not be used by the application layer.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
*******************************************************************************/
void Cy_CapSense_SsPostAllWidgetsScan(cy_stc_capsense_context_t * context)
{
    /*
    * 1. Increment widget index
    * 2. Check whether all the widgets are scanned
    * 3. If all the widgets are not scanned, set up and scan next widget
    */
    cy_status postScanStatus;
    cy_stc_active_scan_sns_t * ptrActive = context->ptrActiveScanSns;
    uint32_t widgetId = (uint32_t)ptrActive->widgetIndex + 1u;
    
    do
    {
        if (context->ptrCommonConfig->numWd > widgetId)
        {
            /* Set up the next widget */
            context->ptrCommonContext->status = 0u;
            postScanStatus = Cy_CapSense_SetupWidget(widgetId, context);
            if (CY_RET_SUCCESS == postScanStatus)
            {
                (void)Cy_CapSense_Scan(context);
            }
            else
            {
                /* The next widget is not available for scanning. Skip it. */
                widgetId++;
            }
        }
        else
        {
            /* All the widgets are scanned. Reset BUSY flag. */
            Cy_CapSense_ClrBusyFlags(context);

            /* Set postScanStatus to exit the while loop */
            postScanStatus = CY_RET_SUCCESS;
        }
    } while (CY_RET_SUCCESS != postScanStatus);
}


/*******************************************************************************
* Function Name: Cy_CapSense_InterruptHandler
****************************************************************************//**
*
* This function is intended to handle CSD IRQs and it is recommended to call it 
* from a user's defined CSD ISR.
*
* The CapSense MW supposes using the CSD interrupt that triggers after the end
* of each sensor scan. After scanning is complete, the Cy_CapSense_InterruptHandler()
* routine copies the measured sensor raw data to the CapSense Structures. If the
* scanning queue is not empty, the Cy_CapSense_InterruptHandler() starts the next
* sensor scanning.
* Calling this function from other routines is possible, however a user must 
* be responsible for calling the function once after each scan.
* The CapSense component does not alter or affect the priority of other 
* interrupts in the system.
* 
* \param base
* The pointer to the base register address of the CSD HW block.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
* \note
* The calls of the Start Sample and End Of Scan callbacks
* (see the \ref group_capsense_callbacks section for details) are the part of the
* Cy_CapSense_InterruptHandler() routine and they lengthen its execution. These
* callbacks will lengthen the CSD ISR execution in case of a direct call of the
* Cy_CapSense_InterruptHandler() function from a CSD ISR.
*
*******************************************************************************/
void Cy_CapSense_InterruptHandler(const CSD_Type * base, cy_stc_capsense_context_t * context)
{
    (void)base;
    context->ptrActiveScanSns->ptrISRCallback((void *)context);
}


/*******************************************************************************
* Function Name: Cy_CapSense_CalibrateWidget
****************************************************************************//**
*
* Calibrates the IDACs for all the sensors in the specified widget to the 
* default target.
*
* This function detects the sensing method used by the widget prior 
* to calibration.
* This function performs exactly the same tasks as
* Cy_CapSense_CalibrateAllWidgets(), but only for a specified widget.
*
* \param widgetId
* Specifies the ID number of the widget to calibrate its IDACs.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
* \return
* Returns the status of the specified widget calibration:
* - CY_RET_SUCCESS - The operation is successfully completed.
* - CY_RET_BAD_PARAM - The input parameter is invalid.
* - CY_RET_BAD_DATA - The calibration failed and the middleware may not
* operate as expected.
*
*******************************************************************************/
cy_status Cy_CapSense_CalibrateWidget(uint32_t widgetId, cy_stc_capsense_context_t * context)
{
    cy_status calibrateStatus = CY_RET_BAD_PARAM;

    if(context->ptrCommonConfig->numWd > widgetId)
    {
        switch(context->ptrWdConfig[widgetId].senseMethod)
        {
            case (uint8_t)CY_CAPSENSE_SENSE_METHOD_CSD_E:
                calibrateStatus = Cy_CapSense_CSDCalibrateWidget(widgetId, (uint32_t)context->ptrCommonConfig->csdRawTarget, context);
                break;

            case (uint8_t)CY_CAPSENSE_SENSE_METHOD_CSX_E:
                calibrateStatus = Cy_CapSense_CSXCalibrateWidget(widgetId, (uint32_t)context->ptrCommonConfig->csxRawTarget, context);
                break;

            default:
                break;
        }
    }

    return calibrateStatus;
}


/*******************************************************************************
* Function Name: Cy_CapSense_CalibrateAllWidgets
****************************************************************************//**
*
* Calibrates the IDACs for all the widgets in the middleware to the default
* target.
*
* This function detects the sensing method used by the widgets
* prior to calibration.
* This function is available when the CSD and/or CSX Enable IDAC
* auto-calibration parameter is enabled.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
* \return
* Returns the status of the calibration process:
* - CY_RET_SUCCESS - The operation is successfully completed.
* - CY_RET_BAD_DATA - The calibration failed and the middleware may not
* operate as expected.
*
*******************************************************************************/
cy_status Cy_CapSense_CalibrateAllWidgets(cy_stc_capsense_context_t * context)
{
    cy_status calibrateStatus = CY_RET_SUCCESS;
    uint32_t wdgtIndex;

    for (wdgtIndex = 0u; wdgtIndex < context->ptrCommonConfig->numWd; wdgtIndex++)
    {
        calibrateStatus |= Cy_CapSense_CalibrateWidget(wdgtIndex, context);
    }

    return calibrateStatus;
}


/*******************************************************************************
* Function Name: Cy_CapSense_CalibrateAllCsdWidgets
****************************************************************************//**
*
* Performs the IDAC calibration of all the CSD widgets.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
* \return
* Returns the status of the operation:
* - Zero     - All the widgets are calibrated successfully.
* - Non-zero - Calibration failed for at least one widget.
*
*******************************************************************************/
cy_status Cy_CapSense_CalibrateAllCsdWidgets(cy_stc_capsense_context_t * context)
{
    uint32_t widgetId;
    cy_status calibrateStatus = CY_RET_SUCCESS;

    for(widgetId = 0uL; widgetId < context->ptrCommonConfig->numWd; widgetId++)
    {
        if((uint8_t)CY_CAPSENSE_SENSE_METHOD_CSD_E == context->ptrWdConfig[widgetId].senseMethod)
        {
            calibrateStatus |= Cy_CapSense_CSDCalibrateWidget(widgetId, (uint32_t)context->ptrCommonConfig->csdRawTarget, context);
        }
    }

    return(calibrateStatus);
}


/*******************************************************************************
* Function Name: Cy_CapSense_CalibrateAllCsxWidgets
****************************************************************************//**
*
* Performs the IDAC calibration of all the CSX widgets.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
* \return
* Returns the status of the operation:
* - Zero     - All the widgets are calibrated successfully.
* - Non-zero - Calibration failed for at least one widget.
*
*******************************************************************************/
cy_status Cy_CapSense_CalibrateAllCsxWidgets(cy_stc_capsense_context_t * context)
{
    uint32_t widgetId;
    cy_status calibrateStatus = CY_RET_SUCCESS;

    for(widgetId = 0uL; widgetId < context->ptrCommonConfig->numWd; widgetId++)
    {
        if((uint8_t)CY_CAPSENSE_SENSE_METHOD_CSX_E == context->ptrWdConfig[widgetId].senseMethod)
        {
            calibrateStatus |= Cy_CapSense_CSXCalibrateWidget(widgetId, (uint32_t)context->ptrCommonConfig->csxRawTarget, context);
        }
    }

    return(calibrateStatus);

}



/*******************************************************************************
* Function Name: Cy_CapSense_SsAutoTune
****************************************************************************//**
*
* Performs the parameters auto-tuning for the optimal CapSense operation when
* SmartSense is enabled.
*
* This function performs the following tasks:
* - Calibrate Modulator and Compensation IDACs.
* - Tune the Sense Clock optimal value to get a Sense Clock period greater 
*   than 2*5*R*Cp.
* - Calculate the resolution for the optimal finger capacitance.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
* \return
* Returns the status of the operation:
* - Zero     - All the widgets are auto-tuned successfully.
* - Non-zero - Auto-tuning failed for any widget.
*
*******************************************************************************/
cy_status Cy_CapSense_SsAutoTune(cy_stc_capsense_context_t * context)
{
    cy_status autoTuneStatus = CY_RET_SUCCESS;
    uint32_t wdIndex;

    for (wdIndex = 0u; wdIndex < context->ptrCommonConfig->numWd; wdIndex++)
    {
        if ((uint8_t)CY_CAPSENSE_SENSE_METHOD_CSD_E == context->ptrWdConfig[wdIndex].senseMethod)
        {
            autoTuneStatus |= Cy_CapSense_SsAutoTuneWidget(wdIndex, context);
        }
    }

    return autoTuneStatus;
}


/*******************************************************************************
* Function Name: Cy_CapSense_SsAutoTuneWidget
****************************************************************************//**
*
* Performs the parameters auto-tuning for the optimal CapSense operation when
* SmartSense is enabled.
*
* This function performs the following tasks:
* - Calibrate Modulator and Compensation IDACs.
* - Tune the Sense Clock optimal value to get a Sense Clock period greater 
*   than 2*5*R*Cp.
* - Calculate the resolution for the optimal finger capacitance.
*
* \param widgetId
* Specifies the ID number of the widget.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
* \return
* Returns the status of the operation:
* - Zero     - All the widgets are auto-tuned successfully.
* - Non-zero - Auto-tuning failed.
*
*******************************************************************************/
cy_status Cy_CapSense_SsAutoTuneWidget(
                uint32_t widgetId, 
                cy_stc_capsense_context_t * context)
{
    cy_status autoTuneStatus = CY_RET_SUCCESS;
    
    uint32_t snsIndex;
    uint32_t inputSnsClk;
    uint32_t minSnsClkDiv;
    uint32_t snsClkSourceAuto;

    cy_stc_capsense_auto_tune_config_t autoTuneConfig;

    uint32_t maxRaw = 0u;
    uint32_t maxCp = 0u;
    uint32_t maxIdacComp = 0u;

    uint32_t maxCpSnsId = 0u;
    uint32_t maxCpRowSnsId = 0u;

    const cy_stc_capsense_widget_config_t * ptrWdCfg = &context->ptrWdConfig[widgetId];
    cy_stc_capsense_widget_context_t * ptrWdCxt = ptrWdCfg->ptrWdContext;
    cy_stc_capsense_sensor_context_t * ptrSnsCtx;
    
    /* Store sense clock source to be restored at the end of function */
    snsClkSourceAuto = ptrWdCxt->snsClkSource;
    
    /* Find input of sensor clock frequency */
    inputSnsClk = context->ptrCommonConfig->periClkHz / 
                    context->ptrCommonContext->modCsdClk /
                    CY_CAPSENSE_CSD_AUTOTUNE_CAL_UNITS;
    
    minSnsClkDiv = context->ptrCommonConfig->periClkHz / 
                    context->ptrCommonContext->modCsdClk /
                    CY_CAPSENSE_CSD_SNS_FREQ_KHZ_MAX;
    if (CY_CAPSENSE_MIN_SNS_CLK_DIVIDER > minSnsClkDiv)
    {
        minSnsClkDiv = CY_CAPSENSE_MIN_SNS_CLK_DIVIDER;
    }
    minSnsClkDiv = (minSnsClkDiv + (CY_CAPSENSE_CSD_AUTOTUNE_CAL_UNITS - 1u)) / CY_CAPSENSE_CSD_AUTOTUNE_CAL_UNITS;
    /* Make divider even */
    if (0u != (minSnsClkDiv & 0x01u))
    {
        minSnsClkDiv++;
    }

    /* Initialize auto-tuning configuration structure */
    autoTuneConfig.snsClkInputClock = (uint16_t)inputSnsClk;
    autoTuneConfig.snsClkConstantR = context->ptrCommonConfig->csdRConst;
    autoTuneConfig.vRef = context->ptrInternalContext->csdVrefVoltageMv;
    autoTuneConfig.fingerCap = ptrWdCxt->fingerCap;
    autoTuneConfig.sigPFC = &(ptrWdCxt->sigPFC);

    /*
    * Autotune phase #1:
    * - performing the first calibration at fixed settings
    * - getting maximum sensor Cp
    * - getting sense clock frequency based on Cp
    */

    ptrWdCxt->resolution = CY_CAPSENSE_AUTOTUNE_CALIBRATION_RESOLUTION;
    ptrWdCxt->maxRawCount = (uint16_t)(0x01uL << CY_CAPSENSE_AUTOTUNE_CALIBRATION_RESOLUTION) - 1u;
    ptrWdCxt->snsClkSource = CY_CAPSENSE_CLK_SOURCE_DIRECT;
    ptrWdCxt->snsClk = (uint16_t)(inputSnsClk / CY_CAPSENSE_AUTOTUNE_CALIBRATION_FREQ_KHZ);
    ptrWdCxt->rowSnsClk = ptrWdCxt->snsClk;

    /* Calibrate CSD widget to the default calibration target */
    (void)Cy_CapSense_CSDCalibrateWidget(widgetId, (uint32_t)context->ptrCommonConfig->csdRawTarget, context);

    /* Find raw count and IDAC of a sensor with maximum Cp */
    ptrSnsCtx = ptrWdCfg->ptrSnsContext;
    if (CY_CAPSENSE_ENABLE == context->ptrCommonConfig->csdIdacCompEn)
    {
        for (snsIndex = 0u; snsIndex < ptrWdCfg->numCols; snsIndex++)
        {
            if (maxIdacComp < ptrSnsCtx->idacComp)
            {
                maxCpSnsId = snsIndex;
                maxIdacComp = ptrSnsCtx->idacComp;
                maxRaw = ptrSnsCtx->raw;
            }
            ptrSnsCtx++;
        }
    }
    else
    {
        for (snsIndex = 0u; snsIndex < ptrWdCfg->numCols; snsIndex++)
        {
            if (maxRaw < ptrSnsCtx->raw)
            {
                maxCpSnsId = snsIndex;
                maxRaw = ptrSnsCtx->raw;
            }
            ptrSnsCtx++;
        }
    }

    /* Update auto-tuning configuration structure */
    autoTuneConfig.iDacMod = ptrWdCxt->idacMod[CY_CAPSENSE_MFS_CH0_INDEX];
    autoTuneConfig.iDacComp = (uint8_t)maxIdacComp;
    autoTuneConfig.iDacGain = context->ptrCommonConfig->idacGainTable[ptrWdCxt->idacGainIndex].gainValue;
    autoTuneConfig.ptrSenseClk = &ptrWdCxt->snsClk;
    autoTuneConfig.calTarget = (uint16_t)((maxRaw * CY_CAPSENSE_CSD_AUTOTUNE_CAL_UNITS) /
            ((uint32_t)(0x01uL << CY_CAPSENSE_AUTOTUNE_CALIBRATION_RESOLUTION) - 1u));

    /* Find sensor Cp and optimum sense clock value */
    maxCp = Cy_CapSense_TunePrescalers_Lib(&autoTuneConfig);
    autoTuneConfig.sensorCap = maxCp;

    /* Increase sensor clock divider to valid value */
    if (((uint32_t)ptrWdCxt->snsClk) < minSnsClkDiv)
    {
        ptrWdCxt->snsClk = (uint16_t)minSnsClkDiv;
    }

    if (((uint8_t)CY_CAPSENSE_WD_TOUCHPAD_E == ptrWdCfg->wdType) ||
        ((uint8_t)CY_CAPSENSE_WD_MATRIX_BUTTON_E == ptrWdCfg->wdType))
    {
        /* Find a sensor with maximum Cp */
        maxRaw = 0u;
        maxIdacComp = 0u;
        if (CY_CAPSENSE_ENABLE == context->ptrCommonConfig->csdIdacCompEn)
        {
            for (snsIndex = 0u; snsIndex < ptrWdCfg->numCols; snsIndex++)
            {
                if (maxIdacComp < ptrSnsCtx->idacComp)
                {
                    maxCpRowSnsId = snsIndex;
                    maxIdacComp = ptrSnsCtx->idacComp;
                    maxRaw = ptrSnsCtx->raw;
                }
                ptrSnsCtx++;
            }
        }
        else
        {
            for (snsIndex = 0u; snsIndex < ptrWdCfg->numCols; snsIndex++)
            {
                if (maxRaw < ptrSnsCtx->raw)
                {
                    maxCpRowSnsId = snsIndex;
                    maxRaw = ptrSnsCtx->raw;
                }
                ptrSnsCtx++;
            }
        }

        /* Configure auto-tuning configuration structure */
        autoTuneConfig.iDacMod = ptrWdCxt->rowIdacMod[CY_CAPSENSE_MFS_CH0_INDEX];
        autoTuneConfig.iDacComp = (uint8_t)maxIdacComp;
        autoTuneConfig.ptrSenseClk = &ptrWdCxt->rowSnsClk;
        autoTuneConfig.calTarget = (uint16_t)((maxRaw * CY_CAPSENSE_CSD_AUTOTUNE_CAL_UNITS) /
                ((uint32_t)(0x01uL << CY_CAPSENSE_AUTOTUNE_CALIBRATION_RESOLUTION) - 1u));

        /* Find correct sense clock value */
        maxCp = Cy_CapSense_TunePrescalers_Lib(&autoTuneConfig);
        /* Save maximum sensor Cp and corresponding sensor Id */
        if (autoTuneConfig.sensorCap < maxCp)
        {
            autoTuneConfig.sensorCap = maxCp;
            maxCpSnsId = maxCpRowSnsId;
        }

        /* Increase sensor clock divider to valid value */
        if (((uint32_t)ptrWdCxt->rowSnsClk) < minSnsClkDiv)
        {
            ptrWdCxt->rowSnsClk = (uint16_t)minSnsClkDiv;
        }
    }
    
    /* Check tuning results */
    if (CY_CAPSENSE_AUTOTUNE_CP_MAX < maxCp)
    {
        autoTuneStatus = CYRET_BAD_DATA;
    }

    /*
    * Autotune phase #2:
    * - repeating calibration with new sense clock frequency
    * - getting resolution
    */

    /* Calibrate CSD widget to the default calibration target with newly defined sense frequency */
    (void)Cy_CapSense_CSDCalibrateWidget(widgetId, (uint32_t)context->ptrCommonConfig->csdRawTarget, context);

    autoTuneConfig.iDacGain = context->ptrCommonConfig->idacGainTable[ptrWdCxt->idacGainIndex].gainValue;
    autoTuneConfig.iDacMod = ptrWdCxt->idacMod[CY_CAPSENSE_MFS_CH0_INDEX];
    autoTuneConfig.ptrSenseClk = &ptrWdCxt->snsClk;

    if (((uint8_t)CY_CAPSENSE_WD_TOUCHPAD_E == ptrWdCfg->wdType) ||
        ((uint8_t)CY_CAPSENSE_WD_MATRIX_BUTTON_E == ptrWdCfg->wdType))
    {
        if (maxCpSnsId >= ptrWdCfg->numCols)
        {
            autoTuneConfig.iDacMod = ptrWdCxt->rowIdacMod[CY_CAPSENSE_MFS_CH0_INDEX];
            autoTuneConfig.ptrSenseClk = &ptrWdCxt->rowSnsClk;
        }
    }

    /* Find resolution */
    ptrWdCxt->resolution = Cy_CapSense_TuneSensitivity_Lib(&autoTuneConfig);
    ptrWdCxt->maxRawCount = (uint16_t)(0x01uL << ptrWdCxt->resolution) - 1u;


    /*
    * Autotune phase #3:
    * - selecting a widget clock source if AUTO
    * - repeating calibration with found clock frequency, resolution and clock source
    * - updating sensitivity
    */

    /* Update clock source */
    ptrWdCxt->snsClkSource = (uint8_t)snsClkSourceAuto;
    Cy_CapSense_CSDSetWidgetSenseClkSrc(ptrWdCfg);

    /* Calibrate CSD widget to the default calibration target with newly defined resolution */
    autoTuneStatus |= Cy_CapSense_CSDCalibrateWidget(widgetId, (uint32_t)context->ptrCommonConfig->csdRawTarget, context);

    return autoTuneStatus;
}


/*******************************************************************************
* Function Name: Cy_CapSense_InitializeSourceSenseClk
****************************************************************************//**
*
* Sets a source for Sense/Tx clock for all widgets.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
*******************************************************************************/
void Cy_CapSense_InitializeSourceSenseClk(const cy_stc_capsense_context_t * context)
{
    uint32_t wdIndex;
    const cy_stc_capsense_widget_config_t * ptrWdCfg = context->ptrWdConfig;
    
    for (wdIndex = 0u; wdIndex < context->ptrCommonConfig->numWd; wdIndex++)
    {
        switch(ptrWdCfg->senseMethod)
        {
            case (uint8_t)CY_CAPSENSE_SENSE_METHOD_CSD_E:
                Cy_CapSense_CSDSetWidgetSenseClkSrc(ptrWdCfg);
                break;

            case (uint8_t)CY_CAPSENSE_SENSE_METHOD_CSX_E:
                Cy_CapSense_CSXSetWidgetTxClkSrc(ptrWdCfg);
                break;

            default:
                break;
        }
        /* Switch to the next widget */
        ptrWdCfg++;
    }
}


/*******************************************************************************
* Function Name: Cy_CapSense_SsCalcLfsrSize
****************************************************************************//**
*
* Finds a SSC polynomial size when clock source is configured to Auto mode.
*
* The SSC polynomial size in the auto mode is found based on the following
* requirements:
* - an LFSR value should be selected so that the max clock dither 
*   is limited with +/-10%.
* - at least one full spread spectrum polynomial should pass during 
*   the scan time.
* - the value of the number of conversions should be an integer multiple 
*   of the repeat period of the programmed LFSR_SIZE.
*
* \param snsClkDivider
* The divider value for the sense/Tx clock.
*
* \param conversionsNum
* The widget conversion number.
*
* \return 
* Returns the size of LFSR value for the SENSE_PERIOD register.
*
*******************************************************************************/
uint32_t Cy_CapSense_SsCalcLfsrSize(uint32_t clkDivider, uint32_t conversionsNum)
{
    uint32_t lfsrSize = 0u;

    if(CY_CAPSENSE_SNSCLK_LFSR_THRESHOLD <= clkDivider)
    {
        /* Find LFSR value */
        if((CY_CAPSENSE_SNSCLK_SSC4_PERIOD <= conversionsNum) &&
           (0uL == (conversionsNum % CY_CAPSENSE_SNSCLK_SSC4_PERIOD)))
        {
            lfsrSize = CY_CAPSENSE_CLK_SOURCE_SSC10;
        }
        else if((CY_CAPSENSE_SNSCLK_SSC3_PERIOD <= conversionsNum) &&
                (0uL == (conversionsNum % CY_CAPSENSE_SNSCLK_SSC3_PERIOD)))
        {
            lfsrSize = CY_CAPSENSE_CLK_SOURCE_SSC9;
        }
        else if((CY_CAPSENSE_SNSCLK_SSC2_PERIOD <= conversionsNum) &&
                (0uL == (conversionsNum % CY_CAPSENSE_SNSCLK_SSC2_PERIOD)))
        {
            lfsrSize = CY_CAPSENSE_CLK_SOURCE_SSC7;
        }
        else if((CY_CAPSENSE_SNSCLK_SSC1_PERIOD <= conversionsNum) &&
                (0uL == (conversionsNum % CY_CAPSENSE_SNSCLK_SSC1_PERIOD)))
        {
            lfsrSize = CY_CAPSENSE_CLK_SOURCE_SSC6;
        }
        else
        {
            lfsrSize = CY_CAPSENSE_CLK_SOURCE_DIRECT;
        }
    }
    
    return lfsrSize;
}


/*******************************************************************************
* Function Name: Cy_CapSense_DischargeExtCapacitors
****************************************************************************//**
*
* Discharges available external capacitors.
*
* The function discharges available external capacitors by disconnecting them
* from an analog bus and configuring strong GPIO drive mode with 0.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
*******************************************************************************/
void Cy_CapSense_DischargeExtCapacitors(cy_stc_capsense_context_t * context)
{
    uint32_t cshPcReg = 0uL;
    uint32_t cmodPcReg = 0uL;
    uint32_t cintAPcReg = 0uL;
    uint32_t cintBPcReg = 0uL;
        
    en_hsiom_sel_t cshHsiomReg = CY_CAPSENSE_HSIOM_SEL_GPIO;
    en_hsiom_sel_t cmodHsiomReg = CY_CAPSENSE_HSIOM_SEL_GPIO;
    en_hsiom_sel_t cintAHsiomReg = CY_CAPSENSE_HSIOM_SEL_GPIO;
    en_hsiom_sel_t cintBHsiomReg = CY_CAPSENSE_HSIOM_SEL_GPIO;

    if (CY_CAPSENSE_ENABLE == context->ptrCommonConfig->csdEn)
    {
        Cy_CapSense_SsReadPinRegisters(context->ptrCommonConfig->portCmod,
            (uint32_t)context->ptrCommonConfig->pinCmod, &cmodPcReg, &cmodHsiomReg);
        Cy_CapSense_SsConfigPinRegisters(context->ptrCommonConfig->portCmod,
            (uint32_t)context->ptrCommonConfig->pinCmod, CY_GPIO_DM_STRONG_IN_OFF, CY_CAPSENSE_HSIOM_SEL_GPIO);
        Cy_GPIO_Clr(context->ptrCommonConfig->portCmod, (uint32_t)context->ptrCommonConfig->pinCmod);
            
        if((CY_CAPSENSE_ENABLE == context->ptrCommonConfig->csdShieldEn) && 
            (CY_CAPSENSE_ENABLE == context->ptrCommonConfig->csdCTankShieldEn))
        {
            Cy_CapSense_SsReadPinRegisters(context->ptrCommonConfig->portCsh,
                (uint32_t)context->ptrCommonConfig->pinCsh, &cshPcReg, &cshHsiomReg);
            Cy_CapSense_SsConfigPinRegisters(context->ptrCommonConfig->portCsh,
                (uint32_t)context->ptrCommonConfig->pinCsh, CY_GPIO_DM_STRONG_IN_OFF, CY_CAPSENSE_HSIOM_SEL_GPIO);   
            Cy_GPIO_Clr(context->ptrCommonConfig->portCsh, (uint32_t)context->ptrCommonConfig->pinCsh);
        }
    }

    if (CY_CAPSENSE_ENABLE == context->ptrCommonConfig->csxEn)
    {
        Cy_CapSense_SsReadPinRegisters(context->ptrCommonConfig->portCintA,
            (uint32_t)context->ptrCommonConfig->pinCintA, &cintAPcReg, &cintAHsiomReg);
        Cy_CapSense_SsConfigPinRegisters(context->ptrCommonConfig->portCintA,
            (uint32_t)context->ptrCommonConfig->pinCintA, CY_GPIO_DM_STRONG_IN_OFF, CY_CAPSENSE_HSIOM_SEL_GPIO);
        Cy_GPIO_Clr(context->ptrCommonConfig->portCintA, (uint32_t)context->ptrCommonConfig->pinCintA);
        
        Cy_CapSense_SsReadPinRegisters(context->ptrCommonConfig->portCintB,
            (uint32_t)context->ptrCommonConfig->pinCintB, &cintBPcReg, &cintBHsiomReg);
        Cy_CapSense_SsConfigPinRegisters(context->ptrCommonConfig->portCintB,
            (uint32_t)context->ptrCommonConfig->pinCintB, CY_GPIO_DM_STRONG_IN_OFF, CY_CAPSENSE_HSIOM_SEL_GPIO);
        Cy_GPIO_Clr(context->ptrCommonConfig->portCintB, (uint32_t)context->ptrCommonConfig->pinCintB);
    }

    Cy_SysLib_DelayUs(CY_CAPSENSE_EXT_CAP_DISCHARGE_TIME);

    if (CY_CAPSENSE_ENABLE == context->ptrCommonConfig->csdEn)
    {
        Cy_CapSense_SsConfigPinRegisters(context->ptrCommonConfig->portCmod,
            (uint32_t)context->ptrCommonConfig->pinCmod, cmodPcReg, cmodHsiomReg);
            
        if((CY_CAPSENSE_ENABLE == context->ptrCommonConfig->csdShieldEn) && 
            (CY_CAPSENSE_ENABLE == context->ptrCommonConfig->csdCTankShieldEn))
        {
            Cy_CapSense_SsConfigPinRegisters(context->ptrCommonConfig->portCsh,
                (uint32_t)context->ptrCommonConfig->pinCsh, cshPcReg, cshHsiomReg);
        }
    }

    if (CY_CAPSENSE_ENABLE == context->ptrCommonConfig->csxEn)
    {
        Cy_CapSense_SsConfigPinRegisters(context->ptrCommonConfig->portCintA,
            (uint32_t)context->ptrCommonConfig->pinCintA, cintAPcReg, cintAHsiomReg);
        
        Cy_CapSense_SsConfigPinRegisters(context->ptrCommonConfig->portCintB,
            (uint32_t)context->ptrCommonConfig->pinCintB, cintBPcReg, cintBHsiomReg);
    }
}


/*******************************************************************************
* Function Name: Cy_CapSense_InitActivePtr
****************************************************************************//**
*
* Initializes active scan sensor structure with all available
* pointers for further faster access to widget/sensor parameters.
*
* \param widgetId
* Specifies the ID number of the widget to perform hardware and firmware
* initialization required for scanning sensors in the specific widget.
*
* \param sensorId
* Specifies the ID number of the sensor to perform hardware and firmware
* initialization required for scanning sensors in the specific widget.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
*******************************************************************************/
void Cy_CapSense_InitActivePtr(
                uint32_t widgetId, 
                uint32_t sensorId, 
                cy_stc_capsense_context_t * context)
{
    Cy_CapSense_InitActivePtrWd(widgetId, context);
    Cy_CapSense_InitActivePtrSns(sensorId, context);
}


/*******************************************************************************
* Function Name: Cy_CapSense_InitActivePtrSns
****************************************************************************//**
*
* Initializes active scan sensor structure with pointers to sensor 
* for further faster access to widget/sensor parameters.
*
* This function supposes that the Cy_CapSense_InitActivePtrWd() function
* is called before.
*
* \param sensorId
* Specifies the ID number of the sensor to perform hardware and firmware
* initialization required for scanning sensors in the specific widget.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
*******************************************************************************/
void  Cy_CapSense_InitActivePtrSns(
                uint32_t sensorId, 
                cy_stc_capsense_context_t * context)
{
    cy_stc_active_scan_sns_t * ptrActive = context->ptrActiveScanSns;
    uint32_t numberRows;
    uint32_t numberCols;

    ptrActive->mfsChannelIndex = 0u;
    switch(context->ptrActiveScanSns->currentSenseMethod)
    {
        case (uint8_t)CY_CAPSENSE_SENSE_METHOD_CSD_E:
            ptrActive->sensorIndex = (uint16_t)sensorId;
            ptrActive->ptrEltdConfig = &ptrActive->ptrWdConfig->ptrEltdConfig[sensorId];
            ptrActive->ptrSnsContext = &ptrActive->ptrWdConfig->ptrSnsContext[sensorId];
            ptrActive->connectedSnsState = CY_CAPSENSE_SNS_DISCONNECTED;
            break;
        case (uint8_t)CY_CAPSENSE_SENSE_METHOD_CSX_E:
            ptrActive->sensorIndex = (uint16_t)sensorId;
            ptrActive->ptrSnsContext = &ptrActive->ptrWdConfig->ptrSnsContext[sensorId];
            ptrActive->connectedSnsState = CY_CAPSENSE_SNS_DISCONNECTED;
            
            numberRows = ptrActive->ptrWdConfig->numRows;
            numberCols = ptrActive->ptrWdConfig->numCols;
            ptrActive->rxIndex = (uint8_t)numberCols;
            ptrActive->txIndex = (uint8_t)numberRows;
            ptrActive->ptrRxConfig = &ptrActive->ptrWdConfig->ptrEltdConfig[sensorId / numberRows];
            ptrActive->ptrTxConfig = &ptrActive->ptrWdConfig->ptrEltdConfig[numberCols + (sensorId % numberRows)];
            
            break;
        default:
            break;
    }
}

/*******************************************************************************
* Function Name: Cy_CapSense_InitActivePtrWd
****************************************************************************//**
*
* Initializes active scan sensor structure with pointers to current widget
* for further faster access to widget/sensor parameters.
*
* This function does not update pointers to current sensor and the 
* Cy_CapSense_InitActivePtrSns() function should be called after current one.
*
* \param widgetId
* Specifies the ID number of the widget to perform hardware and firmware
* initialization required for scanning sensors in the specific widget.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
*******************************************************************************/
void Cy_CapSense_InitActivePtrWd(
                uint32_t widgetId, 
                cy_stc_capsense_context_t * context)
{
    cy_stc_active_scan_sns_t * ptrActive = context->ptrActiveScanSns;

    ptrActive->widgetIndex = (uint8_t)widgetId;
    ptrActive->ptrWdConfig = &context->ptrWdConfig[widgetId];
    ptrActive->ptrWdContext = ptrActive->ptrWdConfig->ptrWdContext;
}


/*******************************************************************************
* Function Name: Cy_CapSense_SsConfigPinRegisters
****************************************************************************//**
*
* Configures drive mode and HSIOM state of a desired pin. The action
* is performed inside the critical section.
*
* \param base
* The pointer to the pin's port register base address.
*
* \param pinNum
* Position of the pin bit-field within the port register.
*
* \param dm
* Specifies drive mode of the pin.
*
* \param hsiom
* Specifies HSIOM state of the pin.
*
*******************************************************************************/
void Cy_CapSense_SsConfigPinRegisters(
                GPIO_PRT_Type * base, 
                uint32_t pinNum, 
                uint32_t dm, 
                en_hsiom_sel_t hsiom)
{
    uint32_t interruptState = Cy_SysLib_EnterCriticalSection();
    Cy_GPIO_SetDrivemode(base, pinNum, dm);
    Cy_GPIO_SetHSIOM(base, pinNum, hsiom);
    Cy_SysLib_ExitCriticalSection(interruptState);
}


/*******************************************************************************
* Function Name: Cy_CapSense_SsReadPinRegisters
****************************************************************************//**
*
* Reads drive mode and HSIOM registers of the specified pin.
*
* \param base
* The pointer to the pin's port register base address.
*
* \param pinNum
* Position of the pin bit-field within the port register.
*
* \param dm
* The pointer to the drive mode variable.
*
* \param hsiom
* The pointer to the HSIOM variable.
*
*
*******************************************************************************/
void Cy_CapSense_SsReadPinRegisters(GPIO_PRT_Type * base, uint32_t pinNum, uint32_t * dm, en_hsiom_sel_t * hsiom)
{
    uint32_t interruptState = Cy_SysLib_EnterCriticalSection();
    *dm = Cy_GPIO_GetDrivemode(base, pinNum);
    *hsiom = Cy_GPIO_GetHSIOM(base, pinNum);
    Cy_SysLib_ExitCriticalSection(interruptState);
}


/*******************************************************************************
* Function Name: Cy_CapSense_SetClkDivider
****************************************************************************//**
*
* Sets the modulator clock and then starts it.
*
* Do not call this function directly from the application layer.
*
* \param dividerValue
* The divider value for the modulator clock.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
*******************************************************************************/
void Cy_CapSense_SetClkDivider(
                uint32_t dividerValue, 
                const cy_stc_capsense_context_t * context)
{
    uint32_t dividerIndex = (uint32_t)context->ptrCommonConfig->periDividerIndex;
    cy_en_divider_types_t dividerType = (cy_en_divider_types_t)context->ptrCommonConfig->periDividerType;

    (void)Cy_SysClk_PeriphDisableDivider(dividerType, dividerIndex);
    if ((CY_SYSCLK_DIV_8_BIT == dividerType) || (CY_SYSCLK_DIV_16_BIT == dividerType))
    {
        (void)Cy_SysClk_PeriphSetDivider(dividerType, dividerIndex, dividerValue);
    }
    else
    {
        (void)Cy_SysClk_PeriphSetFracDivider(dividerType, dividerIndex, dividerValue, 0u);
    }
    (void)Cy_SysClk_PeriphEnableDivider(dividerType, dividerIndex);
}


/*******************************************************************************
* Function Name: Cy_CapSense_WatchdogCyclesNum
****************************************************************************//**
*
* Converts the specified time into number of CPU cycles.
*
* \param desiredTimeUs
* The time (delay) specified in us.
*
* \param cpuFreqMHz
* The CPU frequency in MHz.
*
* \param cyclesPerLoop
* The number of cycles per a loop.
*
*******************************************************************************/
uint32_t Cy_CapSense_WatchdogCyclesNum(
                uint32_t desiredTimeUs, 
                uint32_t cpuFreqMHz, 
                uint32_t cyclesPerLoop)
{
    uint32_t retVal;

    if(0uL != cyclesPerLoop)
    {
        retVal = (desiredTimeUs * cpuFreqMHz) / cyclesPerLoop;
    }
    else
    {
        retVal = 0xFFFFFFFFuL;
    }

    return(retVal);
}


/*******************************************************************************
* Function Name: Cy_CapSense_CalibrateCheck
****************************************************************************//**
*
* Verifies that the calibrated widget meets the configured conditions.
*
* This function checks whether the raw count of each sensor of the specified widget
* is within the raw count range defined by raw count target and +/- calibration
* error.
*
* \param widgetId
* Specifies the ID number of the widget.
*
* \param target
* The raw count target in percentage.
*
* \param senseMethod
* The widget sensing method.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
*******************************************************************************/
cy_status Cy_CapSense_CalibrateCheck(
                uint32_t widgetId, 
                uint32_t target, 
                uint32_t senseMethod,
                const cy_stc_capsense_context_t * context)
{
    cy_status calibrateStatus = CY_RET_SUCCESS;
    uint32_t rawcount;
    uint32_t snsIndex;
    uint32_t upperLimit;
    uint32_t lowerLimit;
    uint8_t freqChIndex;
    uint8_t freqChNumber;
    uint8_t calibrationError;
    const cy_stc_capsense_widget_config_t * ptrWdCfg = &context->ptrWdConfig[widgetId];
    cy_stc_capsense_sensor_context_t * ptrChCxt = ptrWdCfg->ptrSnsContext;
    cy_stc_capsense_sensor_context_t * ptrSnsCxt;

    /* Calculate acceptable raw count range based on the resolution, target and error */
    if((uint8_t)CY_CAPSENSE_SENSE_METHOD_CSD_E == senseMethod)
    {
        rawcount = (1uL << context->ptrWdContext[widgetId].resolution) - 1u;
        calibrationError = context->ptrCommonConfig->csdCalibrationError;
    }
    else
    {
        rawcount = context->ptrWdContext[widgetId].maxRawCount;
        calibrationError = context->ptrCommonConfig->csxCalibrationError;
    }
    
    lowerLimit = 0u;

    if (target > calibrationError)
    {
        lowerLimit = target - calibrationError;
    }
    upperLimit = target + calibrationError;
    if (upperLimit > CY_CAPSENSE_PERCENTAGE_100)
    {
        upperLimit = CY_CAPSENSE_PERCENTAGE_100;
    }
    lowerLimit = (rawcount * lowerLimit) / CY_CAPSENSE_PERCENTAGE_100;
    upperLimit = (rawcount * upperLimit) / CY_CAPSENSE_PERCENTAGE_100;

    freqChNumber = (CY_CAPSENSE_ENABLE == context->ptrCommonConfig->mfsEn) ? 3u : 1u;
    
    /* Check whether rawcount is in the defined range */
    for (freqChIndex = 0u; freqChIndex < freqChNumber; freqChIndex++)
    {
        ptrSnsCxt = ptrChCxt;
        for(snsIndex = 0u; snsIndex < ptrWdCfg->numSns; snsIndex++)
        {
            rawcount = ptrSnsCxt->raw;
            if ((rawcount < lowerLimit) || (rawcount > upperLimit))
            {
                calibrateStatus = CYRET_BAD_DATA;
                break;
            }
            ptrSnsCxt++;
        }
        ptrChCxt += context->ptrCommonConfig->numSns;
    }

    return (calibrateStatus);
}


/*******************************************************************************
* Function Name: Cy_CapSense_GetVrefAutoMv
****************************************************************************//**
*
* Finds the optimal Vref value based on the VDDA voltage.
*
* \param vddaMv
* Device VDDA voltage.
*
*******************************************************************************/
uint32_t Cy_CapSense_GetVrefAutoMv(uint32_t vddaMv)
{
    uint32_t vrefHighVoltageMv;

    if(vddaMv >= 2750u)
    {
        /*
        * The recommended VrefHigh Voltage for the VDDA >= 2.75V.
        */
        vrefHighVoltageMv = 2133u;
    }
    else if(vddaMv >= 2200u)
    {
        /*
        * The recommended VrefHigh Voltage for the 2.2V - 2.75V VDDA range.
        */
        vrefHighVoltageMv = 1600u;
    }
    else if(vddaMv >= 1800u)
    {
        /*
        * The recommended VrefHigh Voltage for the 1.8V - 2.20V VDDA range.
        */
        vrefHighVoltageMv = 1164u;
    }
    else
    {
        /*
        * The recommended VrefHigh Voltage for the VDDA < 1.80V.
        */
        vrefHighVoltageMv = 800u;
    }
    
    return(vrefHighVoltageMv);
}


/*******************************************************************************
* Function Name: Cy_CapSense_GetVrefHighGain
****************************************************************************//**
*
* Calculates the VrefHigh Gain, based on the values of the device VDDA voltage,
* the Vref source voltage, and the desired VrefHigh voltage. 
*
* \param vddaMv
* Device VDDA voltage.
*
* \param vrefInputMv
* Vref source voltage.
*
* \param vrefDesiredMv
* Desired VrefHigh voltage.
*
* \return 
* Returns the lowest Gain code required to produce the VrefHigh voltage,
* when the VrefHigh is close to the desired voltage (passed through the
* vrefDesiredMv parameter) but not greater than the desired voltage.
*
*******************************************************************************/
uint32_t Cy_CapSense_GetVrefHighGain(uint32_t vddaMv, uint32_t vrefInputMv, uint32_t vrefDesiredMv)
{
    uint32_t vferhiGain;

    if(vrefDesiredMv > (vddaMv - CY_CAPSENSE_VREF_VDDA_MIN_DIFF))
    {
        vrefDesiredMv = (vddaMv - CY_CAPSENSE_VREF_VDDA_MIN_DIFF);
    }

    vferhiGain = ((CY_CAPSENSE_VREF_GAIN_MAX * vrefInputMv) - 1u) / (vrefDesiredMv + 1u);

    if (vferhiGain > (uint32_t)(CY_CAPSENSE_VREF_GAIN_MAX - 1u))
    {
        vferhiGain = (uint32_t)CY_CAPSENSE_VREF_GAIN_MAX - 1u;
    }

    return (vferhiGain);
}


/*******************************************************************************
* Function Name: Cy_CapSense_GetVrefhiAutoMv
****************************************************************************//**
*
* Calculates the VrefHigh voltage, based on the Gain code and the Vref source
* voltage.
*
* \param vrefInputMv
* Vref source voltage.
*
* \param vrefGain
* The Gain code.
*
* \return 
* Returns the VrefHigh voltage in mV, to be produced for the specified vref Gain
* code and Vref source voltage.
*
*******************************************************************************/
uint32_t Cy_CapSense_GetVrefHighMv(uint32_t vrefInputMv, uint32_t vrefGain)
{
    uint32_t vrefHighMv;

    vrefHighMv =  (CY_CAPSENSE_VREF_GAIN_MAX * vrefInputMv) + ((vrefGain + 1u) >> 1u);
    vrefHighMv /= (vrefGain + 1u);

    return (vrefHighMv);
}

/* [] END OF FILE */
