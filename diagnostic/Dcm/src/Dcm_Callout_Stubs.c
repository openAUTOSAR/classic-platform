/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/

/* Ecum Callout Stubs - generic version */

#include "Dcm.h"
#if defined(USE_MCU)
#include "Mcu.h"
#endif
/* @req DCM540 */
Dcm_ReturnWriteMemoryType Dcm_WriteMemory(Dcm_OpStatusType OpStatus,
                                               uint8 MemoryIdentifier,
                                               uint32 MemoryAddress,
                                               uint32 MemorySize,
                                               uint8* MemoryData)
{
    (void)OpStatus;
    (void)MemoryIdentifier;
    (void)MemoryAddress;
    (void)MemorySize;
    (void)*MemoryData;
	return DCM_WRITE_FAILED;
}

/* @req DCM539 */
Dcm_ReturnReadMemoryType Dcm_ReadMemory(Dcm_OpStatusType OpStatus,
                                               uint8 MemoryIdentifier,
                                               uint32 MemoryAddress,
                                               uint32 MemorySize,
                                               uint8* MemoryData)
{

    (void)OpStatus;
    (void)MemoryIdentifier;
    (void)MemoryAddress;
    (void)MemorySize;
    (void)*MemoryData;
	return DCM_READ_FAILED;
}

#ifdef DCM_NOT_SERVICE_COMPONENT
Std_ReturnType Rte_Switch_DcmDiagnosticSessionControl_DcmDiagnosticSessionControl(Dcm_SesCtrlType session)
{
    (void)session;
    return E_OK;
}

Std_ReturnType Rte_Switch_DcmEcuReset_DcmEcuReset(uint8 resetMode)
{

    switch(resetMode) {
        case RTE_MODE_DcmEcuReset_NONE:
        case RTE_MODE_DcmEcuReset_HARD:
        case RTE_MODE_DcmEcuReset_KEYONOFF:
        case RTE_MODE_DcmEcuReset_SOFT:
        case RTE_MODE_DcmEcuReset_JUMPTOBOOTLOADER:
        case RTE_MODE_DcmEcuReset_JUMPTOSYSSUPPLIERBOOTLOADER:
            break;
        case RTE_MODE_DcmEcuReset_EXECUTE:
#if defined(USE_MCU) && ( MCU_PERFORM_RESET_API == STD_ON )
            Mcu_PerformReset();
#endif
            break;
        default:
            break;

    }
    return E_OK;
}

Std_ReturnType Rte_Switch_DcmControlDTCSetting_DcmControlDTCSetting(uint8 mode)
{
    (void)mode;
    return E_OK;
}
#endif

/* @req DCM754 */
Std_ReturnType Dcm_ProcessRequestDownload(Dcm_OpStatusType OpStatus, uint8 DataFormatIdentifier, uint32 MemoryAddress, uint32 MemorySize, uint32 *BlockLength, Dcm_NegativeResponseCodeType* ErrorCode)
{
    (void)OpStatus;
    (void)DataFormatIdentifier;
    (void)MemoryAddress;
    (void)MemorySize;
    (void)*BlockLength;
    *ErrorCode = DCM_E_GENERALPROGRAMMINGFAILURE;
    return E_NOT_OK;
}

/* @req DCM756 */
Std_ReturnType Dcm_ProcessRequestUpload(Dcm_OpStatusType OpStatus, uint8 DataFormatIdentifier, uint32 MemoryAddress, uint32 MemorySize, Dcm_NegativeResponseCodeType* ErrorCode)
{
    (void)OpStatus;
    (void)DataFormatIdentifier;
    (void)MemoryAddress;
    (void)MemorySize;
    *ErrorCode = DCM_E_GENERALPROGRAMMINGFAILURE;
    return E_NOT_OK;
}

/* @req DCM755 */
Std_ReturnType Dcm_ProcessRequestTransferExit(Dcm_OpStatusType OpStatus, uint8 *ParameterRecord, uint32 ParameterRecordSize, Dcm_NegativeResponseCodeType* ErrorCode)
{
    (void)OpStatus;
    (void)*ParameterRecord;
    (void)ParameterRecordSize;
    *ErrorCode = DCM_E_GENERALPROGRAMMINGFAILURE;
    return E_NOT_OK;
}

/**
 *
 * @param Data
 * @param ParameterRecordLength: In: The available buffer size, Out: Number of bytes written to Data
 */
void Dcm_Arc_GetDownloadResponseParameterRecord(uint8 *Data, uint16 *ParameterRecordLength) {
    (void)*Data;
    *ParameterRecordLength = 0;
}

/**
 *
 * @param Data
 * @param ParameterRecordLength: In: The available buffer size, Out: Number of bytes written to Data
 */
void Dcm_Arc_GetTransferExitResponseParameterRecord(uint8 *Data, uint16 *ParameterRecordLength) {
    (void)*Data;
    *ParameterRecordLength = 0;
}

/* @req DCM543 */
Std_ReturnType Dcm_SetProgConditions(Dcm_ProgConditionsType *ProgConditions)
{
    (void)*ProgConditions;
    return E_OK;
}

/* @req DCM544 */
Dcm_EcuStartModeType Dcm_GetProgConditions(Dcm_ProgConditionsType *ProgConditions)
{
    (void)*ProgConditions;
    return DCM_COLD_START;
}

/* @req DCM547 */
void Dcm_Confirmation(Dcm_IdContextType idContext,PduIdType dcmRxPduId,Dcm_ConfirmationStatusType status)
{
    (void)idContext;
    (void)dcmRxPduId;
    (void)status;
}

#ifdef DCM_USE_SERVICE_RESPONSEONEVENT
/**
 *
 * @param ROEPreConfigPtr
 * @return E_OK: PreConfig available, E_NOT_OK: PreConfig not available
 */
Std_ReturnType Dcm_Arc_GetROEPreConfig(const Dcm_ArcROEDidPreconfigType **ROEPreConfigPtr)
{
    (void)ROEPreConfigPtr; //lint !e920 cast from pointer to void
    return E_NOT_OK;
}
#endif
