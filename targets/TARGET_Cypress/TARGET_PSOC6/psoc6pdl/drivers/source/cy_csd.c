/***************************************************************************//**
* \file cy_csd.c
* \version 1.0
*
* \brief
* The source file of the CSD driver.
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

#include <stdint.h>
#include "cy_device_headers.h"
#include "cy_syslib.h"
#include "cy_csd.h"

#if defined(__cplusplus)
extern "C" {
#endif


/**
* \addtogroup group_csd_functions
* \{
*/

/*******************************************************************************
* Function Name: Cy_CSD_Init
****************************************************************************//**
*
* \brief
* Configures HW CSD block and locks it to be used by specified middleware.
*
* \details
* This function configures the HW CSD block and locks it. If the block is already
* in use by other middleware or by the application layer, then the function returns
* the corresponding status and does not configure the block.
* 
* It is not possible to trigger any kind of conversion using this function. 
* To do this, use the Cy_CSD_WriteReg() function.
* 
* \param base
* The pointer to a HW CSD instance.
*
* \param config
* The pointer to a configuration structure that contains the initial configuration.
*
* \param key
* An ID of middleware or user level function that is going to work with
* the specified HW CSD block.
*
* \param context
* The pointer to the context structure allocated by a user or middleware.
*
* \return
* Returns an operation result status (CSD status code).
* See \ref cy_en_csd_status_t.
*
*******************************************************************************/
cy_en_csd_status_t Cy_CSD_Init(CSD_Type * base, cy_stc_csd_config_t const * config, cy_en_csd_key_t key, cy_stc_csd_context_t * context)
{
    cy_en_csd_status_t csdStatus = CY_CSD_LOCKED;

    if ((NULL == base) || (CY_CSD_NONE_KEY == key) || (NULL == config) || (NULL == context))
    {
        csdStatus = CY_CSD_BAD_PARAM;
    }
    else
    {
        if(CY_CSD_NONE_KEY == context->lockKey)
        {
            context->lockKey = key;
            csdStatus = Cy_CSD_Configure(base, config, key, context);
        }
    }

    return(csdStatus);
}


/*******************************************************************************
* Function Name: Cy_CSD_DeInit
****************************************************************************//**
*
* \brief
* Releases the HW CSD block previously captured by middleware.
*
* \details
* Releases the HW CSD block previously captured by middleware. If the block is in 
* use by another instance or if the block is performing any kind of conversion, the 
* function returns the corresponding status and does not perform deinitialization.
*
* \param base
* The pointer to a CSD HW instance.
*
* \param key
* An ID of middleware or user level function that is going to work with
* a specified CSD HW block.
*
* \param context
* The pointer to the context structure allocated by user or middleware.
*
* \return
* Returns an operation result status (CSD status code).
* See \ref cy_en_csd_status_t.
*
*******************************************************************************/
cy_en_csd_status_t Cy_CSD_DeInit(const CSD_Type * base, cy_en_csd_key_t key,  cy_stc_csd_context_t * context)
{
    cy_en_csd_status_t csdStatus = CY_CSD_LOCKED;

    if(key == context->lockKey)
    {
        if(CY_CSD_SUCCESS == Cy_CSD_GetConversionStatus(base, context))
        {
            context->lockKey = CY_CSD_NONE_KEY;
            csdStatus = CY_CSD_SUCCESS;
        }
        else
        {
            csdStatus = CY_CSD_BUSY;
        }
    }

    return(csdStatus);
}


/*******************************************************************************
* Function Name: Cy_CSD_Configure
****************************************************************************//**
*
* \brief
* Sets configuration of all CSD HW block registers at once.
* 
* \details
* Sets configuration of all CSD HW block registers at once. This includes only
* write-enabled registers. It excludes the SEQ_START register. Therefore,
* triggering any kind of conversion using this function is not possible. 
* To do this, use the Cy_CSD_WriteReg() function.
*
* \param base
* The pointer to a CSD HW instance.
*
* \param config
* The pointer to a configuration structure that contains initial configuration.
*
* \param key
* An ID of middleware or user level function that is going to work with the
* specified CSD HW block.
*
* \param context
* The pointer to the context structure, allocated by a user or middleware.
*
* \return
* Returns an operation result status (CSD status code).
* See \ref cy_en_csd_status_t.
*
*******************************************************************************/
cy_en_csd_status_t Cy_CSD_Configure(CSD_Type * base, const cy_stc_csd_config_t * config, cy_en_csd_key_t key, const cy_stc_csd_context_t * context)
{
    cy_en_csd_status_t csdStatus = CY_CSD_LOCKED;

    if (key == CY_CSD_NONE_KEY)
    {
        csdStatus = CY_CSD_BAD_PARAM;
    }
    else
    {
        if(key == context->lockKey)
        {
            csdStatus = CY_CSD_SUCCESS;

            base->CONFIG         = config->config;
            base->SPARE          = config->spare;
            base->INTR           = config->intr;
            base->INTR_SET       = config->intrSet;
            base->INTR_MASK      = config->intrMask;
            base->HSCMP          = config->hscmp;
            base->AMBUF          = config->ambuf;
            base->REFGEN         = config->refgen;
            base->CSDCMP         = config->csdCmp;
            base->SW_RES         = config->swRes;
            base->SENSE_PERIOD   = config->sensePeriod;
            base->SENSE_DUTY     = config->senseDuty;
            base->SW_HS_P_SEL    = config->swHsPosSel;
            base->SW_HS_N_SEL    = config->swHsNegSel;
            base->SW_SHIELD_SEL  = config->swShieldSel;
            base->SW_AMUXBUF_SEL = config->swAmuxbufSel;
            base->SW_BYP_SEL     = config->swBypSel;
            base->SW_CMP_P_SEL   = config->swCmpPosSel;
            base->SW_CMP_N_SEL   = config->swCmpNegSel;
            base->SW_REFGEN_SEL  = config->swRefgenSel;
            base->SW_FW_MOD_SEL  = config->swFwModSel;
            base->SW_FW_TANK_SEL = config->swFwTankSel;
            base->SW_DSI_SEL     = config->swDsiSel;
            base->IO_SEL         = config->ioSel;
            base->SEQ_TIME       = config->seqTime;
            base->SEQ_INIT_CNT   = config->seqInitCnt;
            base->SEQ_NORM_CNT   = config->seqNormCnt;
            base->ADC_CTL        = config->adcCtl;
            base->IDACA          = config->idacA;
            base->IDACB          = config->idacB;

            (void)config->intr;
        }
    }

    return(csdStatus);
}


/*******************************************************************************
* Function Name: Cy_CSD_GetVrefTrim
****************************************************************************//**
*
* Trims reference voltage per SFALSH Vref trim registers.
*
* \param referenceVoltage
* The reference voltage to trim.
*
* \return
* Returns a trimmed reference voltage.
*
*******************************************************************************/
uint32_t Cy_CSD_GetVrefTrim(uint32_t referenceVoltage)
{
    uint32_t vRef;
    uint32_t nominalVoltage;
    uint32_t registerVoltage;
    uint32_t vRefTrimDelta;

    /* Choose a Vref trim register */
    if (referenceVoltage < CY_CSD_ADC_VREF_1P2)
    {
        nominalVoltage = CY_CSD_ADC_VREF_0P8;
        registerVoltage = (uint32_t)(SFLASH_CSD0_ADC_VREF1_TRIM & SFLASH_CSDV2_CSD0_ADC_VREF1_VREF_HI_LEVELS_0P8_Msk) >>
                                                                    SFLASH_CSDV2_CSD0_ADC_VREF1_VREF_HI_LEVELS_0P8_Pos;
    }
    else if (referenceVoltage < CY_CSD_ADC_VREF_1P6)
    {
        nominalVoltage = CY_CSD_ADC_VREF_1P2;
        registerVoltage = (uint32_t)(SFLASH_CSD0_ADC_VREF0_TRIM & SFLASH_CSDV2_CSD0_ADC_VREF0_VREF_HI_LEVELS_1P2_Msk) >>
                                                                    SFLASH_CSDV2_CSD0_ADC_VREF0_VREF_HI_LEVELS_1P2_Pos;
    }
    else if (referenceVoltage < CY_CSD_ADC_VREF_2P1)
    {
        nominalVoltage = CY_CSD_ADC_VREF_1P6;
        registerVoltage = (uint32_t)(SFLASH_CSD0_ADC_VREF0_TRIM & SFLASH_CSDV2_CSD0_ADC_VREF0_VREF_HI_LEVELS_1P6_Msk) >>
                                                                    SFLASH_CSDV2_CSD0_ADC_VREF0_VREF_HI_LEVELS_1P6_Pos;
    }
    else if (referenceVoltage < CY_CSD_ADC_VREF_2P6)
    {
        nominalVoltage = CY_CSD_ADC_VREF_2P1;
        registerVoltage = (uint32_t)(SFLASH_CSD0_ADC_VREF1_TRIM & SFLASH_CSDV2_CSD0_ADC_VREF1_VREF_HI_LEVELS_2P1_Msk) >>
                                                                    SFLASH_CSDV2_CSD0_ADC_VREF1_VREF_HI_LEVELS_2P1_Pos;
    }
    else
    {
        nominalVoltage = CY_CSD_ADC_VREF_2P6;
        registerVoltage = (uint32_t)(SFLASH_CSD0_ADC_VREF2_TRIM & SFLASH_CSDV2_CSD0_ADC_VREF2_VREF_HI_LEVELS_2P6_Msk) >>
                                                                    SFLASH_CSDV2_CSD0_ADC_VREF2_VREF_HI_LEVELS_2P6_Pos;
    }

    vRef = (referenceVoltage * registerVoltage) / nominalVoltage;

    /* Calculate deviation of trim register */
    if (vRef > referenceVoltage)
    {
        vRefTrimDelta = vRef - referenceVoltage;
    }
    else
    {
        vRefTrimDelta = referenceVoltage - vRef;
    }
    vRefTrimDelta = (vRefTrimDelta * CY_CSDADC_PERCENTAGE_100) / referenceVoltage;

    /* Return input Vref if trim-value is not within the allowed range */
    if (CY_CSDADC_VREF_TRIM_MAX_DEVIATION < vRefTrimDelta)
    {
        vRef = referenceVoltage;
    }

    return vRef;
}


/** \} group_csd_functions */

#if defined(__cplusplus)
}
#endif


/* [] END OF FILE */
