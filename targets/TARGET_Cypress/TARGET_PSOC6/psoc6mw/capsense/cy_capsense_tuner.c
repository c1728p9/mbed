/***************************************************************************//**
* \file cy_capsense_tuner.c
* \version 1.1
*
* \brief
* This file provides the source code for the Tuner module functions.
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

#include "cy_syslib.h"
#include "cy_capsense_structure.h"
#include "cy_capsense_tuner.h"
#include "cy_capsense_control.h"
#include "cy_capsense_common.h"


/*******************************************************************************
* Function Name: Cy_CapSense_TuInitialize
****************************************************************************//**
*
* Initializes the communication interface with the Tuner tool.
*
*******************************************************************************/
void Cy_CapSense_TuInitialize(cy_stc_capsense_context_t * context)
{
    volatile cy_stc_capsense_common_context_t * ptrCommonCxt = context->ptrCommonContext;
    ptrCommonCxt->tunerCmd = (uint16_t)CY_CAPSENSE_TU_CMD_NONE_E;
    ptrCommonCxt->tunerSt = (uint8_t)CY_CAPSENSE_TU_FSM_RUNNING;
}

/*******************************************************************************
* Function Name: Cy_CapSense_RunTuner
****************************************************************************//**
*
* Establishes synchronized communication with the CapSense Tuner tool.
*
* This function is used to establish synchronized communication between the
* CapSense FW and Tuner tool (or other Host controllers).
* This function is called periodically in the application program loop
* to serve the CapSense Tuner tool (or Host controller) requests and commands.
* In most cases, the best place to call this function is after processing and
* before next scanning.
*
* If this function is absent from the application program, then communication is
* asynchronous and the following disadvantages are applicable:
* - The raw counts displayed in the tuner may be filtered and/or non-filtered.
* As a result, noise and SNR measurements will not be accurate.
* - The Tuner tool may read the sensor data such as raw counts from a scan
* multiple times. As a result, noise and SNR measurement will not be
* accurate.
* - The Tuner tool and Host controller should not change the parameters
* via the tuner interface. Changing the parameters via the tuner
* interface in the async mode will result in abnormal behavior.
* - Displaying detected gestures may be missed.
*
* Note that calling this function is not mandatory for the application, but
* required only to synchronize the communication with the Host controller or
* the CapSense Tuner tool.
*
* \param context
* The pointer to the CapSense context structure \ref cy_stc_capsense_context_t.
*
* \return
* In some cases, the application program may need to know whether the CapSense was
* re-initialized. The return indicates whether a restart command was executed:
* - CY_CAPSENSE_STATUS_RESTART_DONE - Based on a received command, the
* CapSense was restarted.
* - CY_CAPSENSE_STATUS_RESTART_NONE - No restart was executed by this
* function.
*
*******************************************************************************/
uint32_t Cy_CapSense_RunTuner(cy_stc_capsense_context_t * context)
{
    uint32_t interruptState;
    uint32_t updateFlag = 0uL;
    uint32_t tunerStatus = CY_CAPSENSE_STATUS_RESTART_NONE;
    uint16_t tunerCommand;
    uint32_t cmdOffset;
    uint32_t cmdSize;
    uint8_t cmdCounter;

    uint8_t * tunerStructure = NULL;
    uint8_t * commandPacket = NULL;
    volatile cy_stc_capsense_common_context_t * ptrCommonCxt = context->ptrCommonContext;
    uint8_t tunerState = ptrCommonCxt->tunerSt;

    cy_capsense_tuner_send_callback_t sendCallback = context->ptrCommonContext->ptrTunerSendCallback;
    cy_capsense_tuner_receive_callback_t receiveCallback = context->ptrCommonContext->ptrTunerReceiveCallback;

    do
    {
        /* 
        * ONE_SCAN command could be interpreted as two commands:
        * RESUME till next call of this function and then
        * SUSPEND till next command receiving.
        * So, after one scan cycle tuner state is changed to suspend.
        */
        if ((uint8_t)CY_CAPSENSE_TU_FSM_ONE_SCAN == tunerState)
        {
            interruptState = Cy_SysLib_EnterCriticalSection();
            context->ptrCommonContext->tunerCmd = (uint16_t)CY_CAPSENSE_TU_CMD_SUSPEND_E;
            Cy_SysLib_ExitCriticalSection(interruptState);
        }

        /* Send Data to the Tuner tool */
        if(NULL != sendCallback)
        {
            sendCallback((void *)context);
        }

        /* Command can come from EzI2C by direct writing into data structure */
        tunerCommand = context->ptrCommonContext->tunerCmd;
        cmdCounter = context->ptrCommonContext->tunerCnt + 1u;

        /* Call user's callback function if it is registered */
        if (NULL != receiveCallback)
        {
            commandPacket = NULL;
            tunerStructure = NULL;
            receiveCallback(&commandPacket, &tunerStructure, context);

            /* If command exists and is correct then read command */
            if ((NULL != commandPacket) || (NULL != tunerStructure))
            {
                if (CY_CAPSENSE_COMMAND_OK == Cy_CapSense_CheckCommandIntegrity(commandPacket))
                {
                    tunerCommand = commandPacket[CY_CAPSENSE_COMMAND_CODE_0_IDX];
                    ptrCommonCxt->tunerCmd = tunerCommand;
                    cmdCounter = commandPacket[CY_CAPSENSE_COMMAND_CNTR_0_IDX];
                }
            }
        }

        /* Check command register */
        switch (tunerCommand)
        {
        case (uint16_t)CY_CAPSENSE_TU_CMD_SUSPEND_E:
            tunerState = (uint8_t)CY_CAPSENSE_TU_FSM_SUSPENDED;
            updateFlag = 1u;
            break;

        case (uint16_t)CY_CAPSENSE_TU_CMD_RESUME_E:
            tunerState = (uint8_t)CY_CAPSENSE_TU_FSM_RUNNING;
            updateFlag = 1u;
            break;

        case (uint16_t)CY_CAPSENSE_TU_CMD_RESTART_E:
            (void)Cy_CapSense_Enable(context);
            tunerStatus = CY_CAPSENSE_STATUS_RESTART_DONE;
            tunerState = (uint8_t)CY_CAPSENSE_TU_FSM_RUNNING;
            updateFlag = 1u;
            break;

        case (uint16_t)CY_CAPSENSE_TU_CMD_PING_E:
            tunerState = (uint8_t)CY_CAPSENSE_TU_FSM_RUNNING;
            updateFlag = 1u;
            break;

        case (uint16_t)CY_CAPSENSE_TU_CMD_ONE_SCAN_E:
            tunerState = (uint8_t)CY_CAPSENSE_TU_FSM_ONE_SCAN;
            updateFlag = 0u;
            break;

        case (uint16_t)CY_CAPSENSE_TU_CMD_WRITE_E:
            /* Tuner state is not changed */
            cmdOffset = (uint32_t)((uint32_t)commandPacket[CY_CAPSENSE_COMMAND_OFFS_0_IDX] << CY_CAPSENSE_MSB_SHIFT) |
                                   (uint32_t)commandPacket[CY_CAPSENSE_COMMAND_OFFS_1_IDX];
            cmdSize = commandPacket[CY_CAPSENSE_COMMAND_SIZE_0_IDX];

            if (1u == cmdSize)
            {
                tunerStructure[cmdOffset] = commandPacket[CY_CAPSENSE_COMMAND_DATA_0_IDX + 3u];
            }
            else if (2u == cmdSize)
            {
                tunerStructure[cmdOffset + 1u] = commandPacket[CY_CAPSENSE_COMMAND_DATA_0_IDX + 2u];
                tunerStructure[cmdOffset + 0u] = commandPacket[CY_CAPSENSE_COMMAND_DATA_0_IDX + 3u];
            }
            else
            {
                tunerStructure[cmdOffset + 3u] = commandPacket[CY_CAPSENSE_COMMAND_DATA_0_IDX + 0u];
                tunerStructure[cmdOffset + 2u] = commandPacket[CY_CAPSENSE_COMMAND_DATA_0_IDX + 1u];
                tunerStructure[cmdOffset + 1u] = commandPacket[CY_CAPSENSE_COMMAND_DATA_0_IDX + 2u];
                tunerStructure[cmdOffset + 0u] = commandPacket[CY_CAPSENSE_COMMAND_DATA_0_IDX + 3u];
            }

            updateFlag = 1u;
            break;

        default:
            break;
        }

        ptrCommonCxt->tunerSt = tunerState;

        /* Set Complete flag in command register if needed */
        if (0uL != updateFlag)
        {
            interruptState = Cy_SysLib_EnterCriticalSection();
            /* Check that command wasn't overwritten with new command */
            if (tunerCommand == ptrCommonCxt->tunerCmd)
            {
                ptrCommonCxt->tunerCmd |= CY_CAPSENSE_TU_CMD_COMPLETE_BIT;
                ptrCommonCxt->tunerCnt = cmdCounter;
            }
            Cy_SysLib_ExitCriticalSection(interruptState);
            updateFlag = 0uL;
        }
        tunerStatus = 0u;

    } while ((uint8_t)CY_CAPSENSE_TU_FSM_SUSPENDED == tunerState);
        
    return tunerStatus;
}


/*******************************************************************************
* Function Name: Cy_CapSense_CheckCommandIntegrity
****************************************************************************//**
*
* Checks command format, header, tail, CRC, etc.
*
* This function checks if the specified packet with the size
* CY_CAPSENSE_COMMAND_PACKET_SIZE could be represented as a
* command received from the Tuner tool.
* The verification includes the following items:
* * Header
* * Tail
* * CRC
* * Command code
*
* Command format is the following:
* * Byte  0: Header 0 = 0x0D
* * Byte  1: Header 1 = 0x0A
* * Byte  2: Command code MSB = cy_en_capsense_tuner_cmd_t
* * Byte  3: Command code LSB = cy_en_capsense_tuner_cmd_t
* * Byte  4: Command counter
* * Byte  5: Size = either 1, 2 or 4
* * Byte  6: Offset MSB
* * Byte  7: Offset LSB
* * Byte  8: Data MSB
* * Byte  9: Data
* * Byte 10: Data
* * Byte 11: Data LSB
* * Byte 12: 8-bit CRC
* * Byte 13: Tail 0 = 0x00
* * Byte 14: Tail 1 = 0xFF
* * Byte 15: Tail 2 = 0xFF
*
* \param commandPacket
* The pointer to the data packet that should be verified.
*
* \return
* Returns the result of the command verification:
* - CY_CAPSENSE_COMMAND_OK - Command is correct.
* - CY_CAPSENSE_WRONG_HEADER - Wrong header.
* - CY_CAPSENSE_WRONG_TAIL - Wrong tail.
* - CY_CAPSENSE_WRONG_CRC - Wrong CRC.
* - CY_CAPSENSE_WRONG_CODE - Wrong Command code.
*
*******************************************************************************/
uint32_t Cy_CapSense_CheckCommandIntegrity(const uint8_t * commandPacket)
{
    uint32_t cmdCheckStatus = CY_CAPSENSE_COMMAND_OK;
    uint16_t crcValue;

    if (CY_CAPSENSE_COMMAND_HEAD_0 != commandPacket[CY_CAPSENSE_COMMAND_HEAD_0_IDX])
    {
        cmdCheckStatus = CY_CAPSENSE_WRONG_HEADER;
    }
    else if (CY_CAPSENSE_COMMAND_HEAD_1 != commandPacket[CY_CAPSENSE_COMMAND_HEAD_1_IDX])
    {
        cmdCheckStatus = CY_CAPSENSE_WRONG_HEADER;
    }
    else if (CY_CAPSENSE_COMMAND_TAIL_0 != commandPacket[CY_CAPSENSE_COMMAND_TAIL_0_IDX])
    {
        cmdCheckStatus = CY_CAPSENSE_WRONG_TAIL;
    }
    else if (CY_CAPSENSE_COMMAND_TAIL_1 != commandPacket[CY_CAPSENSE_COMMAND_TAIL_1_IDX])
    {
        cmdCheckStatus = CY_CAPSENSE_WRONG_TAIL;
    }
    else if (CY_CAPSENSE_COMMAND_TAIL_2 != commandPacket[CY_CAPSENSE_COMMAND_TAIL_2_IDX])
    {
        cmdCheckStatus = CY_CAPSENSE_WRONG_TAIL;
    }
    else if (((uint8_t)CY_CAPSENSE_TU_CMD_WRITE_E) < commandPacket[CY_CAPSENSE_COMMAND_CODE_0_IDX])
    {
        cmdCheckStatus = CY_CAPSENSE_WRONG_CODE;
    }
    else
    {
        crcValue = (uint16_t)((uint16_t)commandPacket[CY_CAPSENSE_COMMAND_CRC_0_IDX] << CY_CAPSENSE_MSB_SHIFT);
        crcValue |= (uint16_t)commandPacket[CY_CAPSENSE_COMMAND_CRC_1_IDX];
        if (crcValue != Cy_CapSense_CalculateCrc16(&commandPacket[0u], CY_CAPSENSE_COMMAND_CRC_DATA_SIZE))
        {
            cmdCheckStatus = CY_CAPSENSE_WRONG_CRC;
        }
        else
        {
            cmdCheckStatus = CY_CAPSENSE_COMMAND_OK;
        }
    }

    return cmdCheckStatus;
}


/*******************************************************************************
* Function Name: Cy_CapSense_CalculateCrc16
****************************************************************************//**
*
* Calculates CRC for the specified buffer and length. CRC Poly: 0xAC9A
*
* This API is used for the CRC protection of a packet received from the Tuner
* tool. CRC polynomial is 0xAC9A. It has a Hamming distance 5 for data
* words up to 241 bits.
*
* Reference:  "P. Koopman, T. Chakravarthy,
* "Cyclic Redundancy Code (CRC) Polynomial Selection for Embedded Networks",
* The International Conference on Dependable Systems and Networks, DSN-2004"
*
* \param ptrData
* The pointer to the data.
*
* \param len
* The length of the data in bytes.
*
* \return
* Returns a calculated CRC-16 value.
*
*******************************************************************************/
uint16 Cy_CapSense_CalculateCrc16(const uint8_t *ptrData, uint32_t len)
{
    uint32_t idx;
    uint32_t actualCrc = 0u;
    const uint16_t crcTable[] =
    {
        0x0000u, 0xAC9Au, 0xF5AEu, 0x5934u, 0x47C6u, 0xEB5Cu, 0xB268u, 0x1EF2u,
        0x8F8Cu, 0x2316u, 0x7A22u, 0xD6B8u, 0xC84Au, 0x64D0u, 0x3DE4u, 0x917Eu
    };

    for (;len-- > 0u;)
    {
        /* Process HI Nibble */
        idx = ((actualCrc >> 12u) ^ (((uint32)*ptrData) >> 4u)) & 0xFLu;
        actualCrc = crcTable[idx] ^ (actualCrc << 4u);

        /* Process LO Nibble */
        idx = ((actualCrc >> 12u) ^ (uint32)*ptrData) & 0xFLu;
        actualCrc = crcTable[idx] ^ (actualCrc << 4u);

        ptrData++;
    }

    return (uint16_t)actualCrc;
}


/* [] END OF FILE */
