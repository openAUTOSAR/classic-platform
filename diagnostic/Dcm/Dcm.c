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


// 904 PC-Lint: OK. Allow VALIDATE, VALIDATE_RV and VALIDATE_NO_RV to return value.
//lint -emacro(904,VALIDATE_RV,VALIDATE_NO_RV,VALIDATE)


/*
 *  General requirements
 */
/** @req DCM054.Partially */ /** @req DCM055.Partially */ /** @req DCM110 */ /** @req DCM107 */ /** @req DCM332 */
/** @req DCM012 */ /** @req DCM044 */ /** @req DCM364 */ /** @req DCM041 */
/** @req DCM042 */ /** @req DCM049 */
/** @req DCM033 */
/** @req DCM171 */
/** @req DCM333 */ /** @req DCM334 */
/** @req DCM018 */
/** @req DCM048 */
 /** @req DCM040.Uninit */
 /** @req DCM040.Param */
 /** @req DCM043 */
 /** @req DCM048 */

#include <string.h>
#include "Dcm.h"
#include "Dcm_Internal.h"
#include "Dem.h"
#include "MemMap.h"
//#include "SchM_Dcm.h"
//#include "ComM_Dcm.h"
#include "PduR_Dcm.h"
#include "ComStack_Types.h"
#include "McuExtensions.h"

// State variable
typedef enum
{
  DCM_UNINITIALIZED = 0,
  DCM_INITIALIZED
} Dcm_StateType;

//lint -esym(551,dcmState)	PC-Lint - Turn of warning of dcmState not accessed when having DCM_DEV_ERROR_DETECT to STD_OFF
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
void Dcm_Init(void) /** @req DCM037 */
{
	VALIDATE_NO_RV((DCM_Config.Dsl != NULL) && (DCM_Config.Dsd != NULL) && (DCM_Config.Dsp != NULL), DCM_INIT_ID, DCM_E_CONFIG_INVALID);

	DslInit();
	DsdInit();
	DspInit();

	dcmState = DCM_INITIALIZED;

	return;
}


/*
 * Interface for basic software scheduler
 */
void Dcm_MainFunction(void) /** @req DCM362 */
{
	VALIDATE_NO_RV(dcmState == DCM_INITIALIZED, DCM_MAIN_ID, DCM_E_UNINIT);

	DsdMain();
	DspMain();
	DslMain();
}


/***********************************************
 * Interface for BSW modules and SW-Cs (8.3.2) *
 ***********************************************/
BufReq_ReturnType Dcm_ProvideRxBuffer(PduIdType dcmRxPduId, PduLengthType tpSduLength, PduInfoType **pduInfoPtr)
{
	BufReq_ReturnType returnCode;

	VALIDATE_RV(dcmState == DCM_INITIALIZED, DCM_PROVIDE_RX_BUFFER_ID, DCM_E_UNINIT, BUFREQ_NOT_OK);
	VALIDATE_RV(dcmRxPduId < DCM_DSL_RX_PDU_ID_LIST_LENGTH, DCM_PROVIDE_RX_BUFFER_ID, DCM_E_PARAM, BUFREQ_NOT_OK);

	//lint --e(929)		// PC-Lint exception, MISRA 11.4 Ok by atosar
	//lint --e(960)		// PC-Lint exception, MISRA 11.5 Ok by atosar
	returnCode = DslProvideRxBufferToPdur(dcmRxPduId, tpSduLength, (const PduInfoType**)pduInfoPtr);

	return returnCode;
}


void Dcm_RxIndication(PduIdType dcmRxPduId, NotifResultType result)
{
	VALIDATE_NO_RV(dcmState == DCM_INITIALIZED, DCM_RX_INDICATION_ID, DCM_E_UNINIT);
	VALIDATE_NO_RV(dcmRxPduId < DCM_DSL_RX_PDU_ID_LIST_LENGTH, DCM_RX_INDICATION_ID, DCM_E_PARAM);

	DslRxIndicationFromPduR(dcmRxPduId, result);
}


Std_ReturnType Dcm_GetActiveProtocol(Dcm_ProtocolType *activeProtocol)
{
	Std_ReturnType returnCode;

	VALIDATE_RV(dcmState == DCM_INITIALIZED, DCM_GET_ACTIVE_PROTOCOL_ID, DCM_E_UNINIT, E_NOT_OK);

	returnCode = DslGetActiveProtocol(activeProtocol);

	return returnCode;
}


Std_ReturnType Dcm_GetSecurityLevel(Dcm_SecLevelType *secLevel)
{
	Std_ReturnType returnCode;

	VALIDATE_RV(dcmState == DCM_INITIALIZED, DCM_GET_SECURITY_LEVEL_ID, DCM_E_UNINIT, E_NOT_OK);

	returnCode = DslGetSecurityLevel(secLevel);

	return returnCode;
}


Std_ReturnType Dcm_GetSesCtrlType(Dcm_SesCtrlType *sesCtrlType)
{
	Std_ReturnType returnCode;

	VALIDATE_RV(dcmState == DCM_INITIALIZED, DCM_GET_SES_CTRL_TYPE_ID, DCM_E_UNINIT, E_NOT_OK);

	returnCode = DslGetSesCtrlType(sesCtrlType);

	return returnCode;
}

BufReq_ReturnType Dcm_ProvideTxBuffer(PduIdType dcmTxPduId, PduInfoType **pduInfoPtr, PduLengthType length)
{
	BufReq_ReturnType returnCode;

	VALIDATE_RV(dcmState == DCM_INITIALIZED, DCM_PROVIDE_TX_BUFFER_ID, DCM_E_UNINIT, BUFREQ_NOT_OK);
	VALIDATE_RV(dcmTxPduId < DCM_DSL_TX_PDU_ID_LIST_LENGTH, DCM_PROVIDE_TX_BUFFER_ID, DCM_E_PARAM, BUFREQ_NOT_OK);

	//lint --e(929)		// PC-Lint exception, MISRA 11.4 Ok by atosar
	//lint --e(960)		// PC-Lint exception, MISRA 11.5 Ok by atosar
	returnCode = DslProvideTxBuffer(dcmTxPduId, (const PduInfoType**)pduInfoPtr, length);

	return returnCode;
}

void Dcm_TxConfirmation(PduIdType dcmTxPduId, NotifResultType result)
{
	VALIDATE_NO_RV(dcmState == DCM_INITIALIZED, DCM_TX_CONFIRMATION_ID, DCM_E_UNINIT);
	VALIDATE_NO_RV(dcmTxPduId < DCM_DSL_TX_PDU_ID_LIST_LENGTH, DCM_TX_CONFIRMATION_ID, DCM_E_PARAM);

	DslTxConfirmation(dcmTxPduId, result);
}

