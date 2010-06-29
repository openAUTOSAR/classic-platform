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





#include <string.h>
#include "Dcm.h"
#include "Dcm_Internal.h"
#include "Dem.h"
#include "Det.h"
#include "MemMap.h"
//#include "SchM_Dcm.h"
#include "ComM_Dcm.h"
#include "PduR_Dcm.h"
#include "ComStack_Types.h"
#include "Mcu.h"

// State variable
typedef enum
{
  DCM_UNINITIALIZED = 0,
  DCM_INITIALIZED
} Dcm_StateType;

static Dcm_StateType dcmState = DCM_UNINITIALIZED;


/*********************************************
 * Interface for upper layer modules (8.3.1) *
 *********************************************/

/*
 * Procedure:	Dcm_GetVersionInfo
 * Reentrant:	Yes
 */
// Defined in Dcm.h



/*
 * Procedure:	Dcm_Init
 * Reentrant:	No
 */
void Dcm_Init(void)
{
	if ((DCM_Config.Dsl == NULL) || (DCM_Config.Dsd == NULL) || (DCM_Config.Dsp == NULL)) {
#if (DCM_DEV_ERROR_DETECT == STD_ON)
		Det_ReportError(MODULE_ID_DCM, 0, DCM_INIT_ID, DCM_E_CONFIG_INVALID);
#endif

	}
	else {
		DslInit();
		DsdInit();
		DspInit();

		dcmState = DCM_INITIALIZED;
	}

	return;
}


/*
 * Interface for basic software scheduler
 */
void Dcm_MainFunction(void)
{
	DsdMain();
	DspMain();
	DslMain();
}

/***********************************************
 * Interface for BSW modules and SW-Cs (8.3.2) *
 ***********************************************/
BufReq_ReturnType Dcm_ProvideRxBuffer(PduIdType dcmRxPduId, PduLengthType tpSduLength, PduInfoType **pduInfoPtr)
{
	BufReq_ReturnType returnCode = BUFREQ_OK;

	returnCode = DslProvideRxBufferToPdur(dcmRxPduId, tpSduLength, (const PduInfoType**)pduInfoPtr);

	return returnCode;
}


void Dcm_RxIndication(PduIdType dcmRxPduId, NotifResultType result)
{
	DslRxIndicationFromPduR(dcmRxPduId, result);
}


Std_ReturnType Dcm_GetActiveProtocol(Dcm_ProtocolType *protocolId)
{
	Std_ReturnType returnCode = E_OK;

	returnCode = DslGetActiveProtocol(protocolId);

	return returnCode;
}


Std_ReturnType Dcm_GetSecurityLevel(Dcm_SecLevelType *secLevel)
{
	Std_ReturnType returnCode = E_OK;

	returnCode = DslGetSecurityLevel(secLevel);

	return returnCode;
}


Std_ReturnType Dcm_GetSesCtrlType(Dcm_SesCtrlType *sesCtrlType)
{
	Std_ReturnType returnCode = E_OK;

	returnCode = DslGetSesCtrlType(sesCtrlType);

	return returnCode;
}

BufReq_ReturnType Dcm_ProvideTxBuffer(PduIdType dcmTxPduId, PduInfoType **pduInfoPtr, PduLengthType length)
{
	BufReq_ReturnType returnCode = BUFREQ_OK;

	returnCode = DslProvideTxBuffer(dcmTxPduId, (const PduInfoType**)pduInfoPtr, length);

	return returnCode;
}

void Dcm_TxConfirmation(PduIdType dcmTxPduId, NotifResultType result)
{
	DslTxConfirmation(dcmTxPduId, result);
}

