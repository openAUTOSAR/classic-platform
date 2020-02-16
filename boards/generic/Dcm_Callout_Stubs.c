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
#if defined(USE_MCU) && ( MCU_PERFORM_RESET_API == STD_ON )
#include "Mcu.h"
#endif
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
    (void)MemoryData;
	return DCM_WRITE_FAILED;
}

/*@req Dcm495*/
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
    (void)MemoryData;
	return DCM_READ_FAILED;
}

void Dcm_DiagnosticSessionControl(Dcm_SesCtrlType session)
{
    (void)session;
}

Std_ReturnType DcmE_EcuReset(Dcm_EcuResetType resetType)
{
    (void)resetType;
	return E_OK;
}

void DcmE_EcuPerformReset(Dcm_EcuResetType resetType)
{
    (void)resetType;
#if defined(USE_MCU) && ( MCU_PERFORM_RESET_API == STD_ON )
	Mcu_PerformReset();
#endif
}

void Dcm_E_CommunicationControl(uint8 subFunction, uint8 communicationType, Dcm_NegativeResponseCodeType *responseCode )
{
    (void)subFunction;
    (void)communicationType;
    (void)responseCode;
}
