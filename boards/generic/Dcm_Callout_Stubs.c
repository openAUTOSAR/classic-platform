/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/

/* Ecum Callout Stubs - generic version */

#include "Dcm.h"
#include "Mcu.h"
Dcm_ReturnWriteMemoryType Dcm_WriteMemory(Dcm_OpStatusType OpStatus,
											   uint8 MemoryIdentifier,
											   uint32 MemoryAddress,
											   uint32 MemorySize,
											   uint8* MemoryData)
{

	return DCM_WRITE_FAILED;
}

/*@req Dcm495*/
Dcm_ReturnReadMemoryType Dcm_ReadMemory(Dcm_OpStatusType OpStatus,
											   uint8 MemoryIdentifier,
											   uint32 MemoryAddress,
											   uint32 MemorySize,
											   uint8* MemoryData)
{


	return DCM_READ_FAILED;
}

void Dcm_DiagnosticSessionControl(Dcm_SesCtrlType session)
{

}

Std_ReturnType DcmE_EcuReset(Dcm_EcuResetType resetType)
{
	return E_OK;
}

void DcmE_EcuPerformReset(Dcm_EcuResetType resetType)
{
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
