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
#include "Dem.h"
#include "Det.h"
#include "MemMap.h"
//#include "SchM_Dcm.h"
#include "ComM_Dcm.h"
#include "PduR_Dcm.h"
#include "ComStack_Types.h"

/*
 * Local types
 */
// SID table
#define SID_DIAGNOSTIC_SESSION_CONTROL			0x10
#define SID_ECU_RESET							0x11
#define SID_CLEAR_DIAGNOSTIC_INFORMATION		0x14
#define SID_READ_DTC_INFORMATION				0x19
#define SID_READ_DATA_BY_IDENTIFIER				0x22
#define SID_READ_SCALING_DATA_BY_IDENTIFIER		0x24
#define SID_SECURITY_ACCESS						0x27
#define SID_READ_DATA_BY_PERIODIC_IDENTIFIER	0x2A
#define SID_DYNAMICLLY_DEFINE_DATA_IDENTIFIER	0x2C
#define SID_WRIRE_DATA_BY_IDENTIFIER			0x2E
#define SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER	0x2F
#define SID_ROUTINE_CONTROL						0x31
#define SID_TESTER_PRESENT						0x3E
#define SID_NEGATIVE_RESPONSE					0x7F
#define SID_CONTROL_DTC_SETTING					0x85

// Global sub function
#define ZERO_SUB_FUNCTION			0x00

// TODO: Move following definitions to PduR
#define DCM_PDU_ID_NONE				0
#define DCM_PDU_ID_UDS_FUNC_RX		1
#define DCM_PDU_ID_UDS_PHYS_RX		2
#define DCM_PDU_ID_UDS_TX			3

#define DCM_PDU_ID_OBD_FUNC_RX		4
#define DCM_PDU_ID_OBD_TX			5

// Misc definitions
#define SUPPRESS_POS_RESP_BIT		0x80
#define SID_RESPONSE_BIT			0x40

typedef struct {
	PduIdType 	activePduId;
	boolean		suppressPosRspMsg;
	PduInfoType *pduRxData;
	PduInfoType *pduTxData;
} GlobalVarsType;

static GlobalVarsType commonVars;

// State variable
typedef enum
{
  DCM_UNINITIALIZED = 0,
  DCM_INITIALIZED
} Dcm_StateType;

static Dcm_StateType dcmState = DCM_UNINITIALIZED;

#if (DCM_VERSION_INFO_API == STD_ON)
static Std_VersionInfoType _Dcm_VersionInfo =
{
  .vendorID   = (uint16)1,
  .moduleID   = (uint16)1,
  .instanceID = (uint8)1,
  .sw_major_version = (uint8)DCM_SW_MAJOR_VERSION,
  .sw_minor_version = (uint8)DCM_SW_MINOR_VERSION,
  .sw_patch_version = (uint8)DCM_SW_PATCH_VERSION,
  .ar_major_version = (uint8)DCM_AR_MAJOR_VERSION,
  .ar_minor_version = (uint8)DCM_AR_MINOR_VERSION,
  .ar_patch_version = (uint8)DCM_AR_PATCH_VERSION,
};
#endif /* DCM_VERSION_INFO_API */

// In "queue" to DSD
static boolean	dsdDslDataIndication = FALSE;

// In "queue" to DSL
static boolean	dslDsdPduTransmit = FALSE;

// Global service table, set by DSL used by DSD
static Dcm_DsdServiceTableType *currentServiceTable = NULL;
/*
 * Allocation of ...
 */


/*
 * Function prototypes
 */

void DslInit(void);
void DsdInit(void);
void DspInit(void);

void DsdHandleRequest(void);

void DslHandleResponseTransmission(void);
void DslResponseSuppressed(void);

void DspDiagnosticSessionControl(void);
void DspTesterPresent(void);
void DsdDspProcessingDone(Dcm_NegativeResponseCodeType responseCode);
void DspDcmConfirmation(void);
void DsdDataConfirmation(void);


/*
 * Procedure:	...
 * Description:	...
 */
//==============================================================================//
//																				//
//					  E X T E R N A L   F U N C T I O N S						//
//																				//
//==============================================================================//

/*********************************************
 * Interface for upper layer modules (8.3.1) *
 *********************************************/

/*
 * Procedure:	Dcm_GetVersionInfo
 * Reentrant:	Yes
 */
#if (DCM_VERSION_INFO_API == STD_ON)
void Dcm_GetVersionInfo(Std_VersionInfoType *versionInfo) {
	memcpy(versionInfo, &_Dcm_VersionInfo, sizeof(Std_VersionInfoType));
}
#endif /* DCM_VERSION_INFO_API */



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
		commonVars.activePduId = DCM_PDU_ID_NONE;
		commonVars.pduRxData = NULL;
		commonVars.pduTxData = NULL;

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
	if (dsdDslDataIndication) {
		dsdDslDataIndication = FALSE;
		DsdHandleRequest();
	}

	if (dslDsdPduTransmit) {
		dslDsdPduTransmit = FALSE;
		DslHandleResponseTransmission();
	}


}

/*******
 * DSL *
 *******/
// Setup buffers
#define UDS_FUNC_RX_BUFFER_SIZE		8
#define OBD_FUNC_RX_BUFFER_SIZE		8
#define PHYS_BUFFER_SIZE			255

static uint8 udsFuncRxBuffer[8];
//static uint8 obdFuncRxBuffer[8];
static uint8 physBuffer[PHYS_BUFFER_SIZE];

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

void DslInit(void)
{
	const Dcm_DslProtocolType *dslProtocol = DCM_Config.Dsl->DslProtocol;
	uint16 i;

	activeProtocol = NULL;
	securityLevel = DCM_SEC_LEV_LOCKED;		/** @req DCM033 **/
	sessionControlType =  DCM_DEFAULT_SESSION;		/** @req DCM034 **/

	// Create shortcut pointers to different protocols.
	// Lookup DCM_UDS_ON_CAN
	for (i = 0; (dslProtocol->DslProtocolRow[i].DslProtocolID != DCM_UDS_ON_CAN) && !dslProtocol->DslProtocolRow[i].Arc_EOL; i++);

	if (dslProtocol->DslProtocolRow[i].DslProtocolID == DCM_UDS_ON_CAN) {
		protocolUdsOnCan = (Dcm_DslProtocolRowType*)&(dslProtocol->DslProtocolRow[i]);
	}
	else {
		protocolUdsOnCan = NULL;
	}

	//
	// TODO: Lookup the rest of the protocols...


}


void DslSendNack(PduIdType dcmRxPduId, BufReq_ReturnType nackId)
{
	// TODO: Fill out
}

void ResetSessionTimeoutTimer(void)
{
	// TODO: Fill out
}

boolean CheckIfOkToStartProtocol(Dcm_ProtocolType protocolId)
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

BufReq_ReturnType Dcm_ProvideRxBuffer(PduIdType dcmRxPduId, PduLengthType tpSduLength, PduInfoType **pduInfoPtr)
{
	BufReq_ReturnType returnCode = BUFREQ_OK;

	*pduInfoPtr = NULL;

	if (dcmRxPduId != commonVars.activePduId) {
		switch (dcmRxPduId)
		{
		case DCM_PDU_ID_UDS_FUNC_RX:
			if (tpSduLength <= UDS_FUNC_RX_BUFFER_SIZE) {
				// Everything ok, provide the buffer
				*pduInfoPtr = &udsFuncRxPduInfo;
			} else{
				// Requested buffer size to large
				DslSendNack(dcmRxPduId, DCM_E_GENERALREJECT);
				returnCode = BUFREQ_OVFL;
			}
			break;

		case DCM_PDU_ID_UDS_PHYS_RX:
			if (tpSduLength <= PHYS_BUFFER_SIZE) {
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


void Dcm_RxIndication(PduIdType dcmRxPduId, NotifResultType result)
{

	if (result == NTFRSLT_OK) {	/** @req DCM111 **/
		switch (dcmRxPduId)
		{
		case DCM_PDU_ID_UDS_FUNC_RX:
			// TODO: Add check if SID_TESTER_PRESENT is in the available SID table
			// Check if "TesterPresent" without response
			if (udsFuncRxPduInfo.SduDataPtr[0] == SID_TESTER_PRESENT && (udsFuncRxPduInfo.SduDataPtr[1] & SUPPRESS_POS_RESP_BIT)) {
				ResetSessionTimeoutTimer();	/** @req DCM112 **/ /** @req DCM113 **/
			}
			else {
				if (commonVars.activePduId == DCM_PDU_ID_NONE) {	/** @req DCM241 **/
					if (activeProtocol != protocolUdsOnCan) {
						if (CheckIfOkToStartProtocol(DCM_UDS_ON_CAN)) {		/** @req DCM036 **/
							// TODO: Set default timing parameters (Dcm144)
							activeProtocol = protocolUdsOnCan;
							activeProtocolId = DCM_UDS_ON_CAN;
							securityLevel = DCM_SEC_LEV_LOCKED;		/** @req DCM146 **/
							sessionControlType = DCM_DEFAULT_SESSION;	/** @req147 **/
							currentServiceTable = (Dcm_DsdServiceTableType*)activeProtocol->DslProtocolSIDTable;	/** @req DCM195 **/ /** @req DCM035 **/ /** @req DCM145 **/
						}
						else {
							// Protocol was not allowed to start
							DslSendNack(dcmRxPduId, DCM_E_GENERALREJECT);
							break;
						}
					}
					commonVars.activePduId = dcmRxPduId;
					// TODO: Start the response pending timer
					// Forward message to DSD
					commonVars.pduRxData = &udsFuncRxPduInfo;
					commonVars.pduTxData = &physPduInfo;
					dsdDslDataIndication = TRUE;
				}
			}
			break;

		case DCM_PDU_ID_UDS_PHYS_RX:
			if (commonVars.activePduId == DCM_PDU_ID_NONE) {	/** @req DCM241 **/
				if (activeProtocol != protocolUdsOnCan) {
					if (CheckIfOkToStartProtocol(DCM_UDS_ON_CAN)) {		/** @req DCM036 **/
						activeProtocol = protocolUdsOnCan;
						activeProtocolId = DCM_UDS_ON_CAN;
						securityLevel = DCM_SEC_LEV_LOCKED;		/** @req DCM146 **/
						sessionControlType = DCM_DEFAULT_SESSION;	/** @req147 **/
						currentServiceTable = (Dcm_DsdServiceTableType*)activeProtocol->DslProtocolSIDTable;	/** @req DCM195 **/ /** @req DCM035 **/ /** @req DCM145 **/
					}
					else {
						// Protocol was not allowed to start
						DslSendNack(dcmRxPduId, DCM_E_GENERALREJECT);
						break;
					}
				}
				commonVars.activePduId = dcmRxPduId;
				// TODO: Start the response pending timer
				// Forward message to DSD
				commonVars.pduRxData = &physPduInfo;
				commonVars.pduTxData = &physPduInfo;
				dsdDslDataIndication = TRUE;
			}
			break;

		case DCM_PDU_ID_OBD_FUNC_RX:
			// TODO: Not implemented yet!
			DslSendNack(dcmRxPduId, DCM_E_GENERALREJECT);
			break;
		}
	}
}

Std_ReturnType Dcm_GetActiveProtocol(Dcm_ProtocolType *protocolId)
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

Std_ReturnType Dcm_GetSecurityLevel(Dcm_SecLevelType *secLevel)
{
	Std_ReturnType returnCode = E_OK;

	*secLevel = securityLevel;

	return returnCode;
}

void DslSetSesCtrlType(Dcm_SesCtrlType sesCtrlType)
{
	// TODO: Handle changing of session timing
	ResetSessionTimeoutTimer();
	sessionControlType = sesCtrlType;
	// TODO: Inform others about the change
}

Std_ReturnType Dcm_GetSesCtrlType(Dcm_SesCtrlType *sesCtrlType)
{
	Std_ReturnType returnCode = E_OK;

	*sesCtrlType = sessionControlType;

	return returnCode;
}

void DslHandleResponseTransmission(void)
{
	switch (commonVars.activePduId)
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

	if (pduTxId != DCM_PDU_ID_NONE && commonVars.pduTxData != NULL) {
		txBufferPointer = commonVars.pduTxData;		// Save this for the Dcm_ProvideTxBuffer call
		Pdur_DcmTransmit(pduTxId, txBufferPointer);	/** @req DCM237 **/
	}
	else {
#if (DCM_DEV_ERROR_DETECT == STD_ON)
		Det_ReportError(MODULE_ID_DCM, 0, DCM_HANDLE_RESPONSE_TRANSMISSION, DCM_E_UNEXPECTED_PARAM);
#endif
	}
}

BufReq_ReturnType Dcm_ProvideTxBuffer(PduIdType dcmTxPduId, PduInfoType **pduInfoPtr, PduLengthType length)
{
	BufReq_ReturnType returnCode = BUFREQ_OK;

	if ((length <= PHYS_BUFFER_SIZE) && (dcmTxPduId == pduTxId) && (txBufferPointer != NULL)) {
		*pduInfoPtr = txBufferPointer;
	}
	else {
		returnCode = BUFREQ_NOT_OK;
	}

	return returnCode;
}

void Dcm_TxConfirmation(PduIdType dcmTxPduId, NotifResultType result)
{
	if (result == NTFRSLT_OK) {
		// Lets clean up
		commonVars.activePduId = DCM_PDU_ID_NONE;
		commonVars.pduRxData = NULL;
		commonVars.pduTxData = NULL;

		pduTxId = DCM_PDU_ID_NONE;
		txBufferPointer = NULL;

		// Inform DSD about the transmission completion
		DsdDataConfirmation();	/** @req DCM117 **/ /** @req DCM235 **/
	}
	else {
		// TODO: What to do?
	}

}

void DslResponseSuppressed(void)
{
	// Lets clean up
	commonVars.activePduId = DCM_PDU_ID_NONE;
	commonVars.pduRxData = NULL;
	commonVars.pduTxData = NULL;

	pduTxId = DCM_PDU_ID_NONE;
	txBufferPointer = NULL;
}


/*******
 * DSD *
 *******/
static uint8 currentSid;

void DsdInit(void)
{

}

boolean LookupSid(uint8 sid, Dcm_DsdServiceType **sidPtr)
{
	boolean returnStatus = TRUE;
	uint16 i;

	for (i = 0; currentServiceTable->DsdService[i].DsdSidTabServiceId != sid && !currentServiceTable->DsdService[i].Arc_EOL; i++);

	if (currentServiceTable->DsdService[i].DsdSidTabServiceId == sid) {
		*sidPtr = (Dcm_DsdServiceType*)&currentServiceTable->DsdService[i];
	}
	else {
		returnStatus = FALSE;
		*sidPtr = NULL;
	}

	return returnStatus;
}

boolean DsdCheckIfSessionConfigured(Dcm_DsdServiceType *sidPtr)
{
	boolean returnStatus = TRUE;
	Dcm_SesCtrlType currentSession;
	uint16 i;

	Dcm_GetSesCtrlType(&currentSession);
	for (i = 0; (sidPtr->DsdSidTabSessionLevelRef[i]->DspSessionLevel != currentSession) && !sidPtr->DsdSidTabSessionLevelRef[i]->Arc_EOL; i++);
	if (sidPtr->DsdSidTabSessionLevelRef[i]->DspSessionLevel != currentSession) {
		returnStatus = FALSE;
	}

	return returnStatus;
}

boolean DsdCheckIfSecurityAllowed(Dcm_DsdServiceType *sidPtr)
{
	boolean returnStatus = TRUE;

	// TODO: Perform check
	return returnStatus;
}

boolean DsdAskApplicationForServicePermission(uint8 *requestData, uint16 dataSize)
{
	Std_ReturnType returnCode = E_OK;
	Std_ReturnType result;
	uint16 i;

	for (i = 0; !DCM_Config.Dsl->DslServiceRequestIndication[i].Arc_EOL && returnCode != E_REQUEST_NOT_ACCEPTED; i++) {
		if (DCM_Config.Dsl->DslServiceRequestIndication[i].Indication != NULL) {
			result = DCM_Config.Dsl->DslServiceRequestIndication[i].Indication(requestData, dataSize);
			if (result != E_OK)
				returnCode = result;
		}
	}

	return returnCode;
}

void DsdSelectServiceFunction(uint8 sid)
{
	switch (sid)	 /** @req DCM221 **/
	{
	case SID_DIAGNOSTIC_SESSION_CONTROL:
		DspDiagnosticSessionControl();
		break;

	case SID_ECU_RESET:
		break;

	case SID_CLEAR_DIAGNOSTIC_INFORMATION:
		break;

	case SID_READ_DTC_INFORMATION:
		break;

	case SID_READ_DATA_BY_IDENTIFIER:
		break;

	case SID_READ_SCALING_DATA_BY_IDENTIFIER:
		break;

	case SID_SECURITY_ACCESS:
		break;

	case SID_READ_DATA_BY_PERIODIC_IDENTIFIER:
		break;

	case SID_DYNAMICLLY_DEFINE_DATA_IDENTIFIER:
		break;

	case SID_WRIRE_DATA_BY_IDENTIFIER:
		break;

	case SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER:
		break;

	case SID_ROUTINE_CONTROL:
		break;

	case SID_TESTER_PRESENT:
		DspTesterPresent();
		break;

	case SID_CONTROL_DTC_SETTING:
		break;

	default:
		break;
	}
}

void DsdCreateAndSendNcr(Dcm_NegativeResponseCodeType responseCode)
{
	if (!(((commonVars.activePduId == DCM_PDU_ID_UDS_FUNC_RX) || (commonVars.activePduId == DCM_PDU_ID_OBD_FUNC_RX)) &&
			((responseCode == DCM_E_SERVICENOTSUPPORTED) || (responseCode == DCM_E_SUBFUNCTIONNOTSUPPORTED) || (responseCode == DCM_E_REQUESTOUTOFRANGE)))) {   /** @req DCM001 **/
		commonVars.pduTxData->SduDataPtr[0] = SID_NEGATIVE_RESPONSE;
		commonVars.pduTxData->SduDataPtr[1] = currentSid;
		commonVars.pduTxData->SduDataPtr[2] = responseCode;
		commonVars.pduTxData->SduLength = 3;
		dslDsdPduTransmit = TRUE;	/** @req DCM114 **/ /** @req DCM232.1 **/
	}
	else {
		DslResponseSuppressed();
	}
}

void DsdHandleRequest(void)
{
	Std_ReturnType result;
	Dcm_DsdServiceType *sidConfPtr = NULL;

	/** @req DCM178 **/
	if (DCM_RESPOND_ALL_REQUEST || ((commonVars.pduRxData->SduDataPtr[0] & 0x7F) < 0x40)) {		/** @req DCM084 **/
		if (LookupSid(commonVars.pduRxData->SduDataPtr[0], &sidConfPtr)) {		/** @req DCM192 **/ /** @req DCM193 **/ /** @req DCM196 **/
			// SID found!

			if (DsdCheckIfSessionConfigured(sidConfPtr) || (sidConfPtr->DsdSidTabServiceId == SID_DIAGNOSTIC_SESSION_CONTROL)) {		 /** @req DCM211 **/
				if (DsdCheckIfSecurityAllowed(sidConfPtr)) {	 /** @req DCM217 **/
					if (DCM_REQUEST_INDICATION_ENABLED) {	 /** @req DCM218 **/
						 result = DsdAskApplicationForServicePermission(commonVars.pduRxData->SduDataPtr, commonVars.pduRxData->SduLength);
					}
					if (!DCM_REQUEST_INDICATION_ENABLED || result == E_OK) {
						// Yes! All conditions met!
						currentSid = commonVars.pduRxData->SduDataPtr[0];	/** @req DCM198 **/

						// Check if response shall be suppressed
						if (sidConfPtr->DsdSidTabSubfuncAvail && (commonVars.pduRxData->SduDataPtr[1] & SUPPRESS_POS_RESP_BIT)) {	/** @req DCM204 **/
							commonVars.suppressPosRspMsg = TRUE;	/** @req DCM202 **/
							commonVars.pduRxData->SduDataPtr[1] &= ~SUPPRESS_POS_RESP_BIT;	/** @req DCM201 **/
						}
						else
						{
							commonVars.suppressPosRspMsg = FALSE;	/** @req DCM202 **/
						}
						DsdSelectServiceFunction(currentSid);
					}
					else {
						if (result == E_REQUEST_ENV_NOK) {
							DsdCreateAndSendNcr(DCM_E_CONDITIONSNOTCORRECT);	/** @req DCM463 **/
						}
						else {
							// Do not any response		/** @req DCM462 **/
						}
					}
				}
				else {
					DsdCreateAndSendNcr(DCM_E_SECUTITYACCESSDENIED);	/** @req DCM217 **/
				}
			}
			else {
				DsdCreateAndSendNcr(DCM_E_SERVICENOTSUPPORTEDINACTIVESESSION);	/** @req DCM211 **/
			}
		}
		else {
			DsdCreateAndSendNcr(DCM_E_SERVICENOTSUPPORTED);	/** @req DCM197 **/
		}
	}
	else {
		// TODO: Inform DSL that message has been discard
	}
}

void DsdDspProcessingDone(Dcm_NegativeResponseCodeType responseCode)
{
	if (responseCode == DCM_E_POSITIVERESPONSE) {
		if (!commonVars.suppressPosRspMsg) {	/** @req DCM200 **/ /** @req DCM231 **/
			/** @req DCM222 **/
			commonVars.pduTxData->SduDataPtr[0] = currentSid | SID_RESPONSE_BIT;	/** @req DCM223 **/ /** @req DCM224 **/
			dslDsdPduTransmit = TRUE;	/** @req DCM114 **/ /** @req DCM225 **/ /** @req DCM232.2 **/
		}
		else {
			DspDcmConfirmation();	/** @req DCM236 **/ /** @req DCM240 **/
			DslResponseSuppressed();
		}
	}
	else {
		DsdCreateAndSendNcr(responseCode);	/** @req DCM228 **/
	}

}

void DsdDataConfirmation(void)
{
	DspDcmConfirmation();	/** @req DCM236 **/
}



/*******
 * DSP *
 *******/
void DspInit(void)
{

}

Std_ReturnType AskApplicationForSessionPermission(Dcm_SesCtrlType newSessionLevel)
{
	Std_ReturnType returnCode = E_OK;
	Dcm_SesCtrlType currentSessionLevel;
	Std_ReturnType result;
	uint16 i;

	for (i = 0; !DCM_Config.Dsl->DslSessionControl[i].Arc_EOL && (returnCode != E_SESSION_NOT_ALLOWED); i++)
	{
		if (DCM_Config.Dsl->DslSessionControl[i].GetSesChgPermission != NULL) {
			Dcm_GetSesCtrlType(&currentSessionLevel);
			result = DCM_Config.Dsl->DslSessionControl[i].GetSesChgPermission(currentSessionLevel ,newSessionLevel);
			if (result != E_OK) {
				returnCode = result;
			}
		}
	}

	return returnCode;
}

void DspDiagnosticSessionControl(void)
{
	// @req DCM250 **/
	Dcm_SesCtrlType reqSessionType;
	Std_ReturnType result;
	uint16	i;

	if (commonVars.pduRxData->SduLength == 2) {
		reqSessionType = commonVars.pduRxData->SduDataPtr[1];
		// Check if type exist in session table
		for (i = 0; (DCM_Config.Dsp->DspSession->DspSessionRow[i].DspSessionLevel != reqSessionType) && !DCM_Config.Dsp->DspSession->DspSessionRow[i].Arc_EOL;i++);

		if (DCM_Config.Dsp->DspSession->DspSessionRow[i].DspSessionLevel == reqSessionType) {
			result = AskApplicationForSessionPermission(reqSessionType);
			if (result == E_OK) {
				DslSetSesCtrlType(reqSessionType);		/** @req DCM311 **/
				// Create positive response
				/** @req DCM039.2 **/
				commonVars.pduTxData->SduDataPtr[1] = reqSessionType;
				commonVars.pduTxData->SduLength = 2;
				DsdDspProcessingDone(DCM_E_POSITIVERESPONSE); /** @req DCM269.2 **/
			}
			else {
				// TODO: Add handling of special case of E_FORCE_RCRRP (Dcm138)
				DsdDspProcessingDone(DCM_E_CONDITIONSNOTCORRECT);	/** @req DCM308 **/
			}
		}
		else {
			DsdDspProcessingDone(DCM_E_SUBFUNCTIONNOTSUPPORTED);	/** @req DCM307 **/
		}
	}
	else {
		// Wrong length
		DsdDspProcessingDone(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);	/** @req DCM272.1 **/
	}
}


void DspTesterPresent(void)
{
	if (commonVars.pduRxData->SduLength == 2) {
		switch (commonVars.pduRxData->SduDataPtr[1])
		{
		case ZERO_SUB_FUNCTION:
			ResetSessionTimeoutTimer();
			// Create positive response
			/** @req DCM039.1 **/
			commonVars.pduTxData->SduDataPtr[1] = ZERO_SUB_FUNCTION;
			commonVars.pduTxData->SduLength = 2;
			DsdDspProcessingDone(DCM_E_POSITIVERESPONSE); /** @req DCM269.1 **/
			break;

		default:
			DsdDspProcessingDone(DCM_E_SUBFUNCTIONNOTSUPPORTED);	/** @req DCM273.1 **/
			break;
		}
	}
	else {
		// Wrong length
		DsdDspProcessingDone(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);	/** @req DCM272.1 **/
	}
}


void DspDcmConfirmation(void)
{
	// TODO: What to do here?
}



