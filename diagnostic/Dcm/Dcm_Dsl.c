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
#include "Dcm_Cfg.h"
#include "Dcm_Cbk.h"
#include "Dcm.h"
#include "Dcm_Internal.h"
#include "Det.h"
#include "MemMap.h"
#include "ComM_Dcm.h"
#include "PduR_Dcm.h"
#include "ComStack_Types.h"



/*
 * Local types
 */
#define MAX_NR_OF_PDUID		10		// ??
typedef struct {
	const Dcm_DslProtocolRowType	*DslProtocolRow;	// (1) For accessing DslProtocolID, DslProtocolSIDTable, DslProtocolTimeLimit etc
	const Dcm_DslProtocolRxType		*DslProtocolRx;		// (1) For accessing DslProtocolAddrType (func/phys)
	const Dcm_DslProtocolTxType		*DslProtocolTx;		// (1) For accessing TxPduId
} PduIdProtocolCrossRefListType;

PduIdProtocolCrossRefListType PduIdProtocolCrossRefList[MAX_NR_OF_PDUID];

PduIdType 	activePduId;
PduInfoType *pduRxData;
PduInfoType *pduTxData;

// Setup buffers
static uint8 udsFuncRxBuffer[DCM_UDS_FUNC_RX_BUFFER_SIZE];
//static uint8 obdFuncRxBuffer[DCM_OBD_FUNC_RX_BUFFER_SIZE];
static uint8 physBuffer[DCM_PHYS_BUFFER_SIZE];

static PduInfoType udsFuncRxPduInfo = {
		udsFuncRxBuffer,
		0
};

//static PduInfoType obdFuncRxPduInfo = {
//		obdFuncRxBuffer,
//		0
//};

static PduInfoType physPduInfo = {
		physBuffer,
		0
};


// Shortcut pointers to protocol
static Dcm_DslProtocolRowType *protocolUdsOnCan = NULL;


// Misc variables
static Dcm_DslProtocolRowType *activeProtocol = NULL;
static Dcm_ProtocolType activeProtocolId;
static Dcm_SecLevelType securityLevel = DCM_SEC_LEV_LOCKED;
static Dcm_SesCtrlType sessionControlType =  DCM_DEFAULT_SESSION;
static PduIdType pduTxId = DCM_PDU_ID_NONE;
static PduInfoType *txBufferPointer = NULL;


// In "queue" to DSL
static boolean	dslDsdPduTransmit = FALSE;

// Global service table, set by DSL used by DSD
//Dcm_DsdServiceTableType *DslCurrentServiceTable = NULL;


void DslInit(void)
{
	const Dcm_DslProtocolType *dslProtocol = DCM_Config.Dsl->DslProtocol;
	uint16 i;

	activePduId = DCM_PDU_ID_NONE;
	pduRxData = NULL;
	pduTxData = NULL;

	activeProtocol = NULL;
	securityLevel = DCM_SEC_LEV_LOCKED;		/** @req DCM033 **/
	sessionControlType =  DCM_DEFAULT_SESSION;		/** @req DCM034 **/

	// Create shortcut pointers to different protocols.
	// Lookup DCM_UDS_ON_CAN
	for (i = 0; (dslProtocol->DslProtocolRow[i].DslProtocolID != DCM_UDS_ON_CAN) && !dslProtocol->DslProtocolRow[i].Arc_EOL; i++);

	if (!dslProtocol->DslProtocolRow[i].Arc_EOL) {
		protocolUdsOnCan = (Dcm_DslProtocolRowType*)&(dslProtocol->DslProtocolRow[i]);
	}
	else {
		protocolUdsOnCan = NULL;
	}

	//
	// TODO: Lookup the rest of the protocols...


}

void DslMain(void)
{
	if (dslDsdPduTransmit) {
		dslDsdPduTransmit = FALSE;
		DslHandleResponseTransmission();
	}
}


void DslSendNack(PduIdType dcmRxPduId, BufReq_ReturnType nackId)
{
	// TODO: Fill out
}


void DslResetSessionTimeoutTimer(void)
{
	// TODO: Fill out
}


boolean DslCheckIfOkToStartProtocol(Dcm_ProtocolType protocolId)
{
	boolean returnCode = TRUE;
	uint16 i;

	for (i = 0; !DCM_Config.Dsl->DslCallbackDCMRequestService[i].Arc_EOL && returnCode; i++) {
		if (DCM_Config.Dsl->DslCallbackDCMRequestService[i].StartProtocol != NULL) {
			if (DCM_Config.Dsl->DslCallbackDCMRequestService[i].StartProtocol(protocolId) != E_OK) {
				returnCode = FALSE;
			}
		}
	}

	return returnCode;
}


BufReq_ReturnType DslProvideRxBuffer(PduIdType dcmRxPduId, PduLengthType tpSduLength, PduInfoType **pduInfoPtr)
{
	BufReq_ReturnType returnCode = BUFREQ_OK;

	*pduInfoPtr = NULL;

	if (dcmRxPduId != activePduId) {
		switch (dcmRxPduId)
		{
		case DCM_PDU_ID_UDS_FUNC_RX:
			if (tpSduLength <= DCM_UDS_FUNC_RX_BUFFER_SIZE) {
				// Everything ok, provide the buffer
				*pduInfoPtr = &udsFuncRxPduInfo;
			} else{
				// Requested buffer size to large
				DslSendNack(dcmRxPduId, DCM_E_GENERALREJECT);
				returnCode = BUFREQ_OVFL;
			}
			break;

		case DCM_PDU_ID_UDS_PHYS_RX:
			if (tpSduLength <= DCM_PHYS_BUFFER_SIZE) {
				// Everything ok, provide the buffer
				*pduInfoPtr = &physPduInfo;
			} else{
				// Requested buffer size to large
				DslSendNack(dcmRxPduId, DCM_E_GENERALREJECT);
				returnCode = BUFREQ_OVFL;
			}
			break;

		case DCM_PDU_ID_OBD_FUNC_RX:
			// TODO: Not implemented yet!
			DslSendNack(dcmRxPduId, DCM_E_GENERALREJECT);
			returnCode = BUFREQ_NOT_OK;
			break;
		}
	}
	else {
		DslSendNack(dcmRxPduId, DCM_E_BUSYREPEATREQUEST);
		returnCode = BUFREQ_NOT_OK;
	}

	return returnCode;
}


void DslRxIndication(PduIdType dcmRxPduId, NotifResultType result)
{
	if (result == NTFRSLT_OK) {	/** @req DCM111 **/
		switch (dcmRxPduId)
		{
		case DCM_PDU_ID_UDS_FUNC_RX:
			// TODO: Add check if SID_TESTER_PRESENT is in the available SID table
			// Check if "TesterPresent" without response
			if (udsFuncRxPduInfo.SduDataPtr[0] == SID_TESTER_PRESENT && (udsFuncRxPduInfo.SduDataPtr[1] & SUPPRESS_POS_RESP_BIT)) {
				DslResetSessionTimeoutTimer();	/** @req DCM112 **/ /** @req DCM113 **/
			}
			else {
				if (activePduId == DCM_PDU_ID_NONE) {	/** @req DCM241 **/
					if (activeProtocol != protocolUdsOnCan) {
						if (DslCheckIfOkToStartProtocol(DCM_UDS_ON_CAN)) {		/** @req DCM036 **/
							// TODO: Set default timing parameters (Dcm144)
							activeProtocol = protocolUdsOnCan;
							activeProtocolId = DCM_UDS_ON_CAN;
							securityLevel = DCM_SEC_LEV_LOCKED;		/** @req DCM146 **/
							sessionControlType = DCM_DEFAULT_SESSION;	/** @req147 **/
//							DslCurrentServiceTable = (Dcm_DsdServiceTableType*)activeProtocol->DslProtocolSIDTable;	/** @req DCM195 **/ /** @req DCM035 **/ /** @req DCM145 **/
						}
						else {
							// Protocol was not allowed to start
							DslSendNack(dcmRxPduId, DCM_E_GENERALREJECT);
							break;
						}
					}
					activePduId = dcmRxPduId;
					// TODO: Start the response pending timer
					// Forward message to DSD
					pduRxData = &udsFuncRxPduInfo;
					pduTxData = &physPduInfo;
//					DsdDslDataIndication();
					DsdDslDataIndication(&udsFuncRxPduInfo, activeProtocol->DslProtocolSIDTable, DCM_PROTOCOL_FUNCTIONAL_ADDR_TYPE, DCM_PDU_ID_UDS_FUNC_RX, &physPduInfo);
				}
			}
			break;

		case DCM_PDU_ID_UDS_PHYS_RX:
			if (activePduId == DCM_PDU_ID_NONE) {	/** @req DCM241 **/
				if (activeProtocol != protocolUdsOnCan) {
					if (DslCheckIfOkToStartProtocol(DCM_UDS_ON_CAN)) {		/** @req DCM036 **/
						activeProtocol = protocolUdsOnCan;
						activeProtocolId = DCM_UDS_ON_CAN;
						securityLevel = DCM_SEC_LEV_LOCKED;		/** @req DCM146 **/
						sessionControlType = DCM_DEFAULT_SESSION;	/** @req147 **/
//						DslCurrentServiceTable = (Dcm_DsdServiceTableType*)activeProtocol->DslProtocolSIDTable;	/** @req DCM195 **/ /** @req DCM035 **/ /** @req DCM145 **/
					}
					else {
						// Protocol was not allowed to start
						DslSendNack(dcmRxPduId, DCM_E_GENERALREJECT);
						break;
					}
				}
				activePduId = dcmRxPduId;
				// TODO: Start the response pending timer
				// Forward message to DSD
				pduRxData = &physPduInfo;
				pduTxData = &physPduInfo;
//				DsdDslDataIndication();
				DsdDslDataIndication(&physPduInfo, activeProtocol->DslProtocolSIDTable, DCM_PROTOCOL_PHYSICAL_ADDR_TYPE, DCM_PDU_ID_UDS_PHYS_RX, &physPduInfo);
			}
			break;

		case DCM_PDU_ID_OBD_FUNC_RX:
			// TODO: Not implemented yet!
			DslSendNack(dcmRxPduId, DCM_E_GENERALREJECT);
			break;
		}
	}
}


Std_ReturnType DslGetActiveProtocol(Dcm_ProtocolType *protocolId)
{
	Std_ReturnType returnCode = E_OK;

	if (activeProtocol != NULL) {
		*protocolId = activeProtocolId;
	}
	else {
		returnCode = E_NOT_OK;
	}

	return returnCode;
}


void DslSetSecurityLevel(Dcm_SecLevelType secLevel)
{
	securityLevel = secLevel;
}


Std_ReturnType DslGetSecurityLevel(Dcm_SecLevelType *secLevel)
{
	Std_ReturnType returnCode = E_OK;

	*secLevel = securityLevel;

	return returnCode;
}


void DslSetSesCtrlType(Dcm_SesCtrlType sesCtrlType)
{
	// TODO: Handle changing of session timing
	DslResetSessionTimeoutTimer();
	sessionControlType = sesCtrlType;
	// TODO: Inform others about the change
}


Std_ReturnType DslGetSesCtrlType(Dcm_SesCtrlType *sesCtrlType)
{
	Std_ReturnType returnCode = E_OK;

	*sesCtrlType = sessionControlType;

	return returnCode;
}


void DslHandleResponseTransmission(void)
{
	switch (activePduId)
	{
	case DCM_PDU_ID_UDS_FUNC_RX:
	case DCM_PDU_ID_UDS_PHYS_RX:
		pduTxId = DCM_PDU_ID_UDS_TX;
		break;

	case DCM_PDU_ID_OBD_FUNC_RX:
		pduTxId = DCM_PDU_ID_OBD_TX;
		break;

	default:
		pduTxId = DCM_PDU_ID_NONE;
		break;
	}

	if (pduTxId != DCM_PDU_ID_NONE && pduTxData != NULL) {
		txBufferPointer = pduTxData;		// Save this for the Dcm_ProvideTxBuffer call
		PduR_DcmTransmit(pduTxId, txBufferPointer);	/** @req DCM237 **/
	}
	else {
#if (DCM_DEV_ERROR_DETECT == STD_ON)
		Det_ReportError(MODULE_ID_DCM, 0, DCM_HANDLE_RESPONSE_TRANSMISSION, DCM_E_UNEXPECTED_PARAM);
#endif
	}
}


BufReq_ReturnType DslProvideTxBuffer(PduIdType dcmTxPduId, PduInfoType **pduInfoPtr, PduLengthType length)
{
	BufReq_ReturnType returnCode = BUFREQ_OK;

	if ((length <= DCM_PHYS_BUFFER_SIZE) && (dcmTxPduId == pduTxId) && (txBufferPointer != NULL)) {
		*pduInfoPtr = txBufferPointer;
	}
	else {
		returnCode = BUFREQ_NOT_OK;
	}

	return returnCode;
}


void DslTxConfirmation(PduIdType dcmTxPduId, NotifResultType result)
{
	if (result == NTFRSLT_OK) {
		// Lets clean up
		activePduId = DCM_PDU_ID_NONE;
		pduRxData = NULL;
		pduTxData = NULL;

		pduTxId = DCM_PDU_ID_NONE;
		txBufferPointer = NULL;

		// Inform DSD about the transmission completion
		DsdDataConfirmation(dcmTxPduId);	/** @req DCM117 **/ /** @req DCM235 **/
	}
	else {
		// TODO: What to do?
	}
}


void DslResponseSuppressed(void)
{
	// Lets clean up
	activePduId = DCM_PDU_ID_NONE;
	pduRxData = NULL;
	pduTxData = NULL;

	pduTxId = DCM_PDU_ID_NONE;
	txBufferPointer = NULL;
}


void DslDsdPduTransmit(void)
{
	dslDsdPduTransmit = TRUE;
}
