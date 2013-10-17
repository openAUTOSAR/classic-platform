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

/*
 *  General requirements
 */
/** @req DCM030 */


#include <string.h>
#include "Dcm.h"
#include "Dcm_Internal.h"
#include "MemMap.h"
#if defined(USE_COMM)
#include "ComM_Dcm.h"
#endif
#include "PduR_Dcm.h"
#include "PduR_Types.h" // OBD: for preemption cancel PDUR tx
#include "PduR.h"       // OBD: for cancel
#include "ComStack_Types.h"
#include "Cpu.h"
//#define USE_DEBUG_PRINTF
#include "debug.h"

#define DECREMENT(timer) { if (timer > 0){timer--;} }
#define DCM_CONVERT_MS_TO_MAIN_CYCLES(x)  ((x)/DCM_MAIN_FUNCTION_PERIOD_TIME_MS)


#if (DCM_PAGEDBUFFER_ENABLED)
#error "DCM_PAGEDBUFFER_ENABLED is set to STD_ON, this is not supported by the code."
#endif

/*
 * Type definitions.
 */
// #define MAX_PARALLEL_PROTOCOLS_ALLOWED		1

typedef struct {
	boolean initRun;
	//boolean diagnosticRequestPending; // This is a "semaphore" because DSD and DCM can handle multiple/parallel request at the moment.
	const Dcm_DslProtocolRowType *activeProtocol; // Points to the currently active protocol.
	const Dcm_DslProtocolRowType *preemptingProtocol;
//	Dcm_DslRunTimeProtocolParametersType protocolList[MAX_PARALLEL_PROTOCOLS_ALLOWED];	// Reserved for future use
} DcmDsl_RunTimeDataType;

static DcmDsl_RunTimeDataType DcmDslRunTimeData = {
		.initRun = FALSE,
		.activeProtocol = NULL,
		.preemptingProtocol = NULL
//		.protocolList = {}
};


// OBD: define the status flag of processing done when preemption happens.
static boolean PreemptionNotProcessingDone = FALSE;
static boolean BusyRepeatSent = FALSE;

// ################# HELPER FUNCTIONS START #################

//
// This function reset/stars the session (S3) timer. See requirement
// DCM141 when that action should be taken.
//
static inline void startS3SessionTimer(Dcm_DslRunTimeProtocolParametersType *runtime, const Dcm_DslProtocolRowType *protocolRow) {
	const Dcm_DslProtocolTimingRowType *timeParams;
	timeParams = protocolRow->DslProtocolTimeLimit;
	runtime->S3ServerTimeoutCount = DCM_CONVERT_MS_TO_MAIN_CYCLES(timeParams->TimStrS3Server);
	runtime->S3ServerStarted = TRUE;
}

// - - - - - - - - - - -

//
// This function reset/stars the session (S3) timer. See requirement
// DCM141 when that action should be taken.
//
static inline void stopS3SessionTimer(Dcm_DslRunTimeProtocolParametersType *runtime) {
	runtime->S3ServerStarted = FALSE;
}

// - - - - - - - - - - -
// OBD: This function reset/stars the preempt timer.
//
static inline void startPreemptTimer(Dcm_DslRunTimeProtocolParametersType *runtime, const Dcm_DslProtocolRowType *protocolRow) {
	runtime->preemptTimeoutCount= DCM_CONVERT_MS_TO_MAIN_CYCLES(protocolRow->DslProtocolPreemptTimeout);
}

//
//	This function implements the requirement DCM139 when
// 	transition from one session to another.
//
static void changeDiagnosticSession(Dcm_DslRunTimeProtocolParametersType *runtime, Dcm_SesCtrlType newSession) {

	/** @req DCM139 */

	switch (runtime->sessionControl) {
	case DCM_DEFAULT_SESSION: // "default".
		/* to set the dsp buffer to default*/
		DspInit();
		break;

	case DCM_PROGRAMMING_SESSION:
	case DCM_EXTENDED_DIAGNOSTIC_SESSION:
	case DCM_SAFTEY_SYSTEM_DIAGNOSTIC_SESSION:
	case DCM_ALL_SESSION_LEVEL:
		runtime->securityLevel = DCM_SEC_LEV_LOCKED; // "0x00".
		break;

	default:
		DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_CHANGE_DIAGNOSTIC_SESSION_ID, DCM_E_PARAM);
		DEBUG(DEBUG_MEDIUM, "Old session invalid");
		break;
	}

	switch (newSession) {
	case DCM_DEFAULT_SESSION: // "default".
	case DCM_PROGRAMMING_SESSION:
	case DCM_EXTENDED_DIAGNOSTIC_SESSION:
	case DCM_SAFTEY_SYSTEM_DIAGNOSTIC_SESSION:
	case DCM_ALL_SESSION_LEVEL:
		runtime->sessionControl = newSession;
		break;

	default:
		DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_CHANGE_DIAGNOSTIC_SESSION_ID, DCM_E_PARAM);
		DEBUG(DEBUG_MEDIUM, "New session invalid");
		break;
	}
}

// - - - - - - - - - - -

void DslResetSessionTimeoutTimer(void) {
	const Dcm_DslProtocolRowType *activeProtocol;
	Dcm_DslRunTimeProtocolParametersType *runtime;

	activeProtocol = DcmDslRunTimeData.activeProtocol;
	if(activeProtocol != NULL) // OBD
	{
		runtime = activeProtocol->DslRunTimeProtocolParameters;
		startS3SessionTimer(runtime, activeProtocol); /** @req DCM141 */
	}
}

// - - - - - - - - - - -

void DslGetCurrentServiceTable(const Dcm_DsdServiceTableType **currentServiceTable) { /** @req DCM195 */
	const Dcm_DslProtocolRowType *activeProtocol;

	activeProtocol = DcmDslRunTimeData.activeProtocol;
	if (activeProtocol != NULL) {
		*currentServiceTable = activeProtocol->DslProtocolSIDTable;
	}
}

// - - - - - - - - - - -

Std_ReturnType DslGetActiveProtocol(Dcm_ProtocolType *protocolId) { /** @req DCM340 */
	Std_ReturnType ret = E_NOT_OK;
	const Dcm_DslProtocolRowType *activeProtocol;

	activeProtocol = DcmDslRunTimeData.activeProtocol;
	if (activeProtocol != NULL) {
		*protocolId = activeProtocol->DslProtocolID;
		ret = E_OK;
	}
	return ret;
}

// - - - - - - - - - - -

void DslSetSecurityLevel(Dcm_SecLevelType secLevel) { /** @req DCM020 */
	const Dcm_DslProtocolRowType *activeProtocol;
	Dcm_DslRunTimeProtocolParametersType *runtime;

	activeProtocol = DcmDslRunTimeData.activeProtocol;
	runtime = activeProtocol->DslRunTimeProtocolParameters;
	runtime->securityLevel = secLevel;
}

// - - - - - - - - - - -

Std_ReturnType DslGetSecurityLevel(Dcm_SecLevelType *secLevel) {  /** @req DCM020 *//** @req DCM338 */
	Std_ReturnType ret = E_NOT_OK;
	const Dcm_DslProtocolRowType *activeProtocol;
	Dcm_DslRunTimeProtocolParametersType *runtime = NULL;

	activeProtocol = DcmDslRunTimeData.activeProtocol;
	if (activeProtocol != NULL) {
		runtime = activeProtocol->DslRunTimeProtocolParameters;
		*secLevel = runtime->securityLevel;
		ret = E_OK;
	}
	return ret;
}

// - - - - - - - - - - -

void DslSetSesCtrlType(Dcm_SesCtrlType sesCtrl) {  /** @req DCM022 */
	const Dcm_DslProtocolRowType *activeProtocol;
	Dcm_DslRunTimeProtocolParametersType *runtime = NULL;

	activeProtocol = DcmDslRunTimeData.activeProtocol;
	if (activeProtocol != NULL) {
		runtime = activeProtocol->DslRunTimeProtocolParameters;
		if (runtime->sessionControl != sesCtrl) {
			changeDiagnosticSession(runtime, sesCtrl);
			DslResetSessionTimeoutTimer();
		}
	}
}

// - - - - - - - - - - -

Std_ReturnType DslGetSesCtrlType(Dcm_SesCtrlType *sesCtrlType) { /** @req DCM022 *//** @req DCM339 */
	Std_ReturnType ret = E_NOT_OK;
	const Dcm_DslProtocolRowType *activeProtocol;
	Dcm_DslRunTimeProtocolParametersType *runtime = NULL;

	activeProtocol = DcmDslRunTimeData.activeProtocol;
	if (activeProtocol != NULL) {
		runtime = activeProtocol->DslRunTimeProtocolParameters;
		*sesCtrlType = runtime->sessionControl;
		ret = E_OK;
	}
	return ret;
}

const Dcm_DspSessionRowType *getActiveSessionRow(Dcm_DslRunTimeProtocolParametersType *runtime) {
	 const Dcm_DspSessionRowType *sessionRow = DCM_Config.Dsp->DspSession->DspSessionRow;

	while ((sessionRow->DspSessionLevel != runtime->sessionControl) && (!sessionRow->Arc_EOL) ) {
		sessionRow++;
	}
	if( TRUE == sessionRow->Arc_EOL ) {
		/* Since we are in a session with no configuration - take any session configuration and report error */
		DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_CHANGE_DIAGNOSTIC_SESSION_ID, DCM_E_CONFIG_INVALID);
		sessionRow = DCM_Config.Dsp->DspSession->DspSessionRow;
	}

	return sessionRow;
}

// - - - - - - - - - - -

static boolean findRxPduIdParentConfigurationLeafs(PduIdType dcmRxPduId,
		const Dcm_DslProtocolRxType **protocolRx,
		const Dcm_DslMainConnectionType **mainConnection,
		const Dcm_DslConnectionType **connection,
		const Dcm_DslProtocolRowType **protocolRow,
		Dcm_DslRunTimeProtocolParametersType **runtime) {

	boolean ret = FALSE;
	if (dcmRxPduId < DCM_DSL_RX_PDU_ID_LIST_LENGTH) {
		*protocolRx = &DCM_Config.Dsl->DslProtocol->DslProtocolRxGlobalList[dcmRxPduId];
		*mainConnection = (*protocolRx)->DslMainConnectionParent;
		*connection = (*mainConnection)->DslConnectionParent;
		*protocolRow = (*connection)->DslProtocolRow;
		*runtime = (*protocolRow)->DslRunTimeProtocolParameters;
		ret = TRUE;
	}
	return ret;
}

// - - - - - - - - - - -

static boolean findTxPduIdParentConfigurationLeafs(PduIdType dcmTxPduId,
		const Dcm_DslProtocolTxType **protocolTx,
		const Dcm_DslMainConnectionType **mainConnection,
		const Dcm_DslConnectionType **connection,
		const Dcm_DslProtocolRowType **protocolRow,
		Dcm_DslRunTimeProtocolParametersType **runtime) {

	boolean found = FALSE;
	Dcm_ProtocolType activeProtocol;
	uint16 i = 0;
	if( E_OK == Dcm_GetActiveProtocol(&activeProtocol)) {
		/* Check if this txPdu belongs to the active protocol */
		while(!DCM_Config.Dsl->DslProtocol->DslProtocolTxGlobalList[i].Arc_EOL && !found) {
			*protocolTx = &DCM_Config.Dsl->DslProtocol->DslProtocolTxGlobalList[i];
			*mainConnection = (*protocolTx)->DslMainConnectionParent;
			*connection = (*mainConnection)->DslConnectionParent;
			*protocolRow = (*connection)->DslProtocolRow;
			*runtime = (*protocolRow)->DslRunTimeProtocolParameters;
			if( (dcmTxPduId == (*protocolTx)->DcmDslProtocolDcmTxPduId) && (activeProtocol == (*protocolRow)->DslProtocolID) ) {
				found = TRUE;
			}
			i++;
		}
		if(!found && (NULL != DcmDslRunTimeData.preemptingProtocol)) {
			/* Check if this txPdu belongs to the preempting protocol */
			i = 0;
			while(!DCM_Config.Dsl->DslProtocol->DslProtocolTxGlobalList[i].Arc_EOL && !found) {
				*protocolTx = &DCM_Config.Dsl->DslProtocol->DslProtocolTxGlobalList[i];
				*mainConnection = (*protocolTx)->DslMainConnectionParent;
				*connection = (*mainConnection)->DslConnectionParent;
				*protocolRow = (*connection)->DslProtocolRow;
				*runtime = (*protocolRow)->DslRunTimeProtocolParameters;
				if( (dcmTxPduId == (*protocolTx)->DcmDslProtocolDcmTxPduId) && (DcmDslRunTimeData.preemptingProtocol->DslProtocolID == (*protocolRow)->DslProtocolID) ) {
					found = TRUE;
				}
				i++;
			}
		}
	}
	return found;
}

// - - - - - - - - - - -

static inline void releaseExternalRxTxBuffers(const Dcm_DslProtocolRowType *protocolRow,
		Dcm_DslRunTimeProtocolParametersType *runtime) {

	protocolRow->DslProtocolTxBufferID->externalBufferRuntimeData->status = BUFFER_AVAILABLE;
	protocolRow->DslProtocolRxBufferID->externalBufferRuntimeData->status = BUFFER_AVAILABLE;
	runtime->externalTxBufferStatus = NOT_IN_USE; // We are waiting for DSD to return the buffer. qqq.
	runtime->externalRxBufferStatus = NOT_IN_USE; // We are waiting for DSD to return the buffer. qqq.
}

// - - - - - - - - - - -


static inline void releaseExternalRxTxBuffersHelper(PduIdType rxPduIdRef) {
	const Dcm_DslProtocolRxType *protocolRx = NULL;
	const Dcm_DslMainConnectionType *mainConnection = NULL;
	const Dcm_DslConnectionType *connection = NULL;
	const Dcm_DslProtocolRowType *protocolRow = NULL;
	Dcm_DslRunTimeProtocolParametersType *runtime = NULL;

	if (findRxPduIdParentConfigurationLeafs(rxPduIdRef, &protocolRx, &mainConnection, &connection, &protocolRow, &runtime)) {
		releaseExternalRxTxBuffers(protocolRow, runtime);
	}
}

// - - - - - - - - - - -

/*
 *  This function is called from the DSD module to the DSL when
 *  a response to a diagnostic request has been copied into the
 *  given TX-buffer and is ready for transmission.
 */
void DslDsdProcessingDone(PduIdType rxPduIdRef, DsdProcessingDoneResultType responseResult) {
	const Dcm_DslProtocolRxType *protocolRx = NULL;
	const Dcm_DslMainConnectionType *mainConnection = NULL;
	const Dcm_DslConnectionType *connection = NULL;
	const Dcm_DslProtocolRowType *protocolRow = NULL;
	Dcm_DslRunTimeProtocolParametersType *runtime = NULL;

	DEBUG( DEBUG_MEDIUM, "DslDsdProcessingDone rxPduIdRef=%d\n", rxPduIdRef);

	if (findRxPduIdParentConfigurationLeafs(rxPduIdRef, &protocolRx, &mainConnection, &connection, &protocolRow, &runtime)) {
	    imask_t state;
	    Irq_Save(state);
		switch (responseResult) {
		case DSD_TX_RESPONSE_READY:
			runtime->externalTxBufferStatus = DSD_PENDING_RESPONSE_SIGNALED; /** @req DCM114 */
			break;
		case DSD_TX_RESPONSE_SUPPRESSED: /** @req DCM238 */
			DEBUG( DEBUG_MEDIUM, "DslDsdProcessingDone called with DSD_TX_RESPONSE_SUPPRESSED.\n");
			releaseExternalRxTxBuffersHelper(rxPduIdRef);
			break;
		default:
			DEBUG( DEBUG_MEDIUM, "Unknown response result from DslDsdProcessingDone!\n");
			break;
		}
	    Irq_Restore(state);
	}
}

// - - - - - - - - - - -

/*
 *	This function preparing transmission of response
 *	pending message to tester.
 */
static void sendResponse(const Dcm_DslProtocolRowType *protocol,
		Dcm_NegativeResponseCodeType responseCode) {
	//Dcm_DslRunTimeProtocolParametersType *runtime = NULL;
	const Dcm_DslProtocolRxType *protocolRx = NULL;
	const Dcm_DslMainConnectionType *mainConnection = NULL;
	const Dcm_DslConnectionType *connection = NULL;
	const Dcm_DslProtocolRowType *protocolRow = NULL;
	Dcm_DslRunTimeProtocolParametersType *runtime = NULL;
	Std_ReturnType transmitResult;
    imask_t state;

    Irq_Save(state);
	/** @req DCM119 */
	if (findRxPduIdParentConfigurationLeafs(protocol->DslRunTimeProtocolParameters->diagReqestRxPduId, &protocolRx,	&mainConnection, &connection, &protocolRow, &runtime)) {
		if (runtime->localTxBuffer.status == NOT_IN_USE) {
			runtime->localTxBuffer.status = PROVIDED_TO_DSD;
			runtime->localTxBuffer.buffer[0] = SID_NEGATIVE_RESPONSE;
			runtime->localTxBuffer.buffer[1] = protocol->DslProtocolRxBufferID->pduInfo.SduDataPtr[0];
			runtime->localTxBuffer.buffer[2] = responseCode;
			runtime->localTxBuffer.PduInfo.SduDataPtr = runtime->localTxBuffer.buffer;
			runtime->localTxBuffer.PduInfo.SduLength = 3;
			runtime->localTxBuffer.status = DCM_TRANSMIT_SIGNALED; // In the DslProvideTxBuffer 'callback' this state signals it is the local buffer we are interested in sending.
			transmitResult = PduR_DcmTransmit(mainConnection->DslProtocolTx->DcmDslProtocolTxPduId, &(runtime->localTxBuffer.PduInfo));/** @req DCM115.Partially */ /* The P2ServerMin has not been implemented. */
			if (transmitResult != E_OK) {
				// TODO: What to do here?
				releaseExternalRxTxBuffers(protocolRow, runtime);
			}
		}
	}
    Irq_Restore(state);
}

// - - - - - - - - - - -

static Std_ReturnType StartProtocolHelper(Dcm_ProtocolType protocolId) {
	Std_ReturnType ret = E_NOT_OK;
	uint16 i;

	for (i = 0; !DCM_Config.Dsl->DslCallbackDCMRequestService[i].Arc_EOL; i++) {
		if (DCM_Config.Dsl->DslCallbackDCMRequestService[i].StartProtocol != NULL) {
			ret = DCM_Config.Dsl->DslCallbackDCMRequestService[i]. StartProtocol(protocolId);
			if (ret != E_OK) {
				break;
			}
		}
	}
	return ret;
}

// OBD: add stop protocol for stack preempting
static Std_ReturnType StopProtocolHelper(Dcm_ProtocolType protocolId) {
	Std_ReturnType ret = E_NOT_OK;
	uint16 i;

	for (i = 0; !DCM_Config.Dsl->DslCallbackDCMRequestService[i].Arc_EOL; i++) {
		if (DCM_Config.Dsl->DslCallbackDCMRequestService[i].StopProtocol != NULL) {
			ret = DCM_Config.Dsl->DslCallbackDCMRequestService[i]. StopProtocol(protocolId);
			if (ret != E_OK) {
				break;
			}
		}
	}
	return ret;
}
// - - - - - - - - - - -

static boolean isTesterPresentCommand(const PduInfoType *rxPdu) {
	boolean ret = FALSE;
	if ((rxPdu->SduDataPtr[0] == SID_TESTER_PRESENT) && (rxPdu->SduDataPtr[1] & SUPPRESS_POS_RESP_BIT)) {
		ret = TRUE;
	}
	return ret;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Implements 'void Dcm_Init(void)' for DSL.
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DslInit(void) {
	const Dcm_DslProtocolRowType *listEntry;
	Dcm_DslRunTimeProtocolParametersType *runtime = NULL;

	listEntry = DCM_Config.Dsl->DslProtocol->DslProtocolRowList;
	while (listEntry->Arc_EOL == FALSE) {
		runtime = listEntry->DslRunTimeProtocolParameters;
		runtime->externalRxBufferStatus = DCM_IDLE;
		runtime->externalTxBufferStatus = DCM_IDLE;
		runtime->localRxBuffer.status = DCM_IDLE;
		runtime->localTxBuffer.status = DCM_IDLE;
		runtime->securityLevel = DCM_SEC_LEV_LOCKED; /** @req DCM033 */
		runtime->sessionControl = DCM_DEFAULT_SESSION; /** @req DCM034 */
		runtime->protocolStarted = FALSE;// OBD: close the protocol
		listEntry->DslProtocolRxBufferID->externalBufferRuntimeData->status = BUFFER_AVAILABLE;
		listEntry->DslProtocolRxBufferID->externalBufferRuntimeData->status = BUFFER_AVAILABLE;
		listEntry++;
	};
	//DcmDslRunTimeData.diagnosticRequestPending = FALSE;
	DcmDslRunTimeData.initRun = TRUE;
	DcmDslRunTimeData.activeProtocol = NULL;// OBD: close the current active protocol
	DcmDslRunTimeData.preemptingProtocol = NULL;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Implements 'void DslInternal_ResponseOnOneDataByPeriodicId(uint8 PericodID)' for simulator a periodic did data.
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Std_ReturnType DslInternal_ResponseOnOneDataByPeriodicId(uint8 PericodID)
{
	Std_ReturnType ret = E_NOT_OK;
	const Dcm_DslProtocolRowType *protocolRowEntry;
	Dcm_DslRunTimeProtocolParametersType *runtime = NULL;
	PduInfoType  *pPeriodData;
	protocolRowEntry = DCM_Config.Dsl->DslProtocol->DslProtocolRowList;

	if( NULL != DcmDslRunTimeData.activeProtocol ) {
		runtime =  DcmDslRunTimeData.activeProtocol->DslRunTimeProtocolParameters;
		if(runtime != NULL)	// find the runtime
		{
			if( BUFREQ_OK == DslProvideRxBufferToPdur(runtime->diagReqestRxPduId, 3, (const PduInfoType **)&pPeriodData)){
				pPeriodData->SduDataPtr[0] = SID_READ_DATA_BY_PERIODIC_IDENTIFIER;
				pPeriodData->SduDataPtr[1] = DCM_PERIODICTRANSMIT_DEFAULT_MODE;
				pPeriodData->SduDataPtr[2] = PericodID;
				pPeriodData->SduLength = 3;
				DslRxIndicationFromPduR(runtime->diagReqestRxPduId, NTFRSLT_OK);
				ret = E_OK;
			}
		}
	}

	return ret;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Implements 'void Dcm_MainFunction(void)' for DSL.
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void DslMain(void) {
	const Dcm_DslProtocolRowType *protocolRowEntry;
	const Dcm_DspSessionRowType *sessionRow = NULL;
	Dcm_DslRunTimeProtocolParametersType *runtime = NULL;

	protocolRowEntry = DCM_Config.Dsl->DslProtocol->DslProtocolRowList;
	while (protocolRowEntry->Arc_EOL == FALSE) {
		runtime = protocolRowEntry->DslRunTimeProtocolParameters;
		if (runtime != NULL) {
			// #### HANDLE THE TESTER PRESENT PRESENCE ####
			if (runtime->sessionControl != DCM_DEFAULT_SESSION) { // Timeout if tester present is lost.
				if( TRUE == runtime->S3ServerStarted ) {
					DECREMENT(runtime->S3ServerTimeoutCount);
					if (runtime->S3ServerTimeoutCount == 0) {
						changeDiagnosticSession(runtime, DCM_DEFAULT_SESSION); /** @req DCM140 */
						runtime->protocolStarted = FALSE;
						if( (NULL != DcmDslRunTimeData.activeProtocol) &&
								(protocolRowEntry->DslProtocolID == DcmDslRunTimeData.activeProtocol->DslProtocolID) ) {
							DcmDslRunTimeData.activeProtocol = NULL;
						}
						if( (NULL != DcmDslRunTimeData.preemptingProtocol)
								&& (protocolRowEntry->DslProtocolID == DcmDslRunTimeData.preemptingProtocol->DslProtocolID) ) {
							DcmDslRunTimeData.preemptingProtocol = NULL;
						}
					}
				}
			}
			switch (runtime->externalTxBufferStatus) { // #### TX buffer state. ####
			case NOT_IN_USE:
				DEBUG( DEBUG_MEDIUM, "state NOT_IN_USE!\n");
				break;
			case PROVIDED_TO_DSD: {
				DECREMENT(runtime->stateTimeoutCount);
				if (runtime->stateTimeoutCount == 0) {
					sessionRow = getActiveSessionRow(runtime);
					runtime->stateTimeoutCount = DCM_CONVERT_MS_TO_MAIN_CYCLES(sessionRow->DspSessionP2StarServerMax); /* Reinitiate timer, see 9.2.2. */
					if (DCM_Config.Dsl->DslDiagResp != NULL) {
						if (DCM_Config.Dsl->DslDiagResp->DslDiagRespForceRespPendEn == TRUE) {
							if (runtime->responsePendingCount != 0) {
								sendResponse(protocolRowEntry, DCM_E_RESPONSEPENDING);  /** @req DCM024 */
								DECREMENT( runtime->responsePendingCount );
							} else {
								DspCancelPendingRequests();
								sendResponse(protocolRowEntry, DCM_E_GENERALREJECT); /** @req DCM120 */
								releaseExternalRxTxBuffers(protocolRowEntry, runtime);
							}
						} else {
							DEBUG( DEBUG_MEDIUM, "Not configured to send response pending, now sending general reject!\n");
							DspCancelPendingRequests();
							sendResponse(protocolRowEntry, DCM_E_GENERALREJECT);
							releaseExternalRxTxBuffers(protocolRowEntry, runtime);
						}
					}
				}
				break;
			}
			case DSD_PENDING_RESPONSE_SIGNALED:
				// The DSD has signaled to DSL that the diagnostic response is available in the Tx buffer.
				// Make sure that response pending or general reject have not been issued,
				// if so we can not transmit to PduR because we would not know from where
				// the Tx confirmation resides later.
				DEBUG( DEBUG_MEDIUM, "state DSD_PENDING_RESPONSE_SIGNALED!\n");
				if (runtime->localTxBuffer.status == NOT_IN_USE) { // Make sure that no TxConfirm could be sent by the local buffer and mixed up with this transmission.
					const Dcm_DslProtocolRxType *protocolRx = NULL;
					const Dcm_DslMainConnectionType *mainConnection = NULL;
					const Dcm_DslConnectionType *connection = NULL;
					const Dcm_DslProtocolRowType *protocolRow = NULL;
					Std_ReturnType transmitResult;

					if (findRxPduIdParentConfigurationLeafs(runtime->diagReqestRxPduId, &protocolRx, &mainConnection, &connection, &protocolRow, &runtime)) {
						const PduIdType txPduId = mainConnection->DslProtocolTx->DcmDslProtocolTxPduId;
						DEBUG( DEBUG_MEDIUM, "runtime->externalTxBufferStatus enter state DCM_TRANSMIT_SIGNALED.\n" );
						runtime->externalTxBufferStatus = DCM_TRANSMIT_SIGNALED;
						transmitResult = PduR_DcmTransmit(txPduId, &runtime->diagnosticResponseFromDsd); /** @req DCM237 *//* Will trigger PduR (CanTP) to call DslProvideTxBuffer(). */
						if (transmitResult != E_OK) {
							// TODO: What to do here?
							releaseExternalRxTxBuffers(protocolRow, runtime);
						}
					} else {
						DEBUG( DEBUG_MEDIUM, "***** WARNING, THIS IS UNEXPECTED !!! ********.\n" );
						const PduIdType txPduId = protocolRowEntry->DslConnection->DslMainConnection->DslProtocolTx->DcmDslProtocolTxPduId;
						DEBUG( DEBUG_MEDIUM, "runtime->externalTxBufferStatus enter state DSD_PENDING_RESPONSE_SIGNALED.\n", txPduId);
						runtime->externalTxBufferStatus = DCM_TRANSMIT_SIGNALED;
						DEBUG( DEBUG_MEDIUM, "Calling PduR_DcmTransmit with txPduId = %d from DslMain\n", txPduId);
						transmitResult = PduR_DcmTransmit(txPduId, &runtime->diagnosticResponseFromDsd); /** @req DCM237 *//* Will trigger PduR (CanTP) to call DslProvideTxBuffer(). */
						if (transmitResult != E_OK) {
							// TODO: What to do here?
							releaseExternalRxTxBuffers(protocolRow, runtime);
						}
					}
				}
				break;
			case DCM_TRANSMIT_SIGNALED:
				DEBUG( DEBUG_MEDIUM, "state DSD_PENDING_RESPONSE_SIGNALED!\n");
				break;
			case PROVIDED_TO_PDUR: // The valid data is being transmitted by TP-layer.
				DEBUG( DEBUG_MEDIUM, "state DSD_PENDING_RESPONSE_SIGNALED!\n");
				break;
			case PREEMPT_TRANSMIT_NRC: /* preemption has happened,send NRC 0x21 to OBD tester */
//				DEBUG( DEBUG_MEDIUM, "state PREEMPT_TRANSMIT_NRC!\n");
				if (TRUE == PreemptionNotProcessingDone){
					if( !BusyRepeatSent ) {
						/*sent NRC 0x21 till timeout or processing done*/
						sendResponse(protocolRowEntry, DCM_E_BUSYREPEATREQUEST);
						BusyRepeatSent = TRUE;
					}

					/*decrease preempt timeout count*/					
					DECREMENT(runtime->preemptTimeoutCount);
//					DEBUG( DEBUG_MEDIUM, "runtime->preemptTimeoutCount = %x\n",runtime->preemptTimeoutCount);
					/*if processing done is finished,clear the flag*/
//					DEBUG( DEBUG_MEDIUM, "DcmDslRunTimeData.activeProtocol->DslRunTimeProtocolParameters->externalTxBufferStatus = %x\n",DcmDslRunTimeData.activeProtocol->DslRunTimeProtocolParameters->externalTxBufferStatus);
					if (DcmDslRunTimeData.activeProtocol->DslRunTimeProtocolParameters->externalTxBufferStatus == NOT_IN_USE){
						/*if processing done is finished,clear the flag*/
						PreemptionNotProcessingDone = FALSE;
						/*close the preempted protocol*/
						DcmDslRunTimeData.activeProtocol->DslRunTimeProtocolParameters->protocolStarted = FALSE;
						/*remove the active protocol and waiting for second OBD request*/
						DcmDslRunTimeData.activeProtocol = NULL;
						/*release current protocol buffer*/
						releaseExternalRxTxBuffers(protocolRowEntry, runtime);
						DcmDslRunTimeData.preemptingProtocol = NULL;
					}
					else if(runtime->preemptTimeoutCount == 0){
						/*if preempt timeout,clear the flag*/
						PreemptionNotProcessingDone = FALSE;
						/*close the preempted protocol*/
						DcmDslRunTimeData.activeProtocol->DslRunTimeProtocolParameters->protocolStarted = FALSE;
						/*release the extrnal Rx and Tx buffters of the preempted protocol*/
						releaseExternalRxTxBuffers(DcmDslRunTimeData.activeProtocol, DcmDslRunTimeData.activeProtocol->DslRunTimeProtocolParameters);
						/*remove the active protocol and waiting for second OBD request*/
						DcmDslRunTimeData.activeProtocol = NULL;
						/*release the extrnal Rx and Tx buffters of the preempting protocol*/
						releaseExternalRxTxBuffers(protocolRowEntry, runtime);
						DcmDslRunTimeData.preemptingProtocol = NULL;
						/*initialize DSP*/
						DspInit();
					}
					else
					{
					}																															
				}
				break;
			default:
				break;
			}
		}
		protocolRowEntry++;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Implements 'BufReq_ReturnType Dcm_ProvideRxBuffer(PduIdType dcmRxPduId,
//  PduLengthType tpSduLength, PduInfoType **pduInfoPtr)'.
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  This function is called called by the PduR typically when CanTp has
//  received a FF or a single frame and needs to obtain a buffer from the
//  receiver so that received data can be forwarded.

BufReq_ReturnType DslProvideRxBufferToPdur(PduIdType dcmRxPduId, PduLengthType tpSduLength, const PduInfoType **pduInfoPtr) {
	BufReq_ReturnType ret = BUFREQ_NOT_OK;
	const Dcm_DslProtocolRxType *protocolRx = NULL;
	const Dcm_DslMainConnectionType *mainConnection = NULL;
	const Dcm_DslConnectionType *connection = NULL;
	const Dcm_DslProtocolRowType *protocolRow = NULL;
	Dcm_DslRunTimeProtocolParametersType *runtime = NULL;
    imask_t state;
	DEBUG( DEBUG_MEDIUM, "DslProvideRxBufferToPdur(dcmRxPduId=%d) called!\n", dcmRxPduId);	
    Irq_Save(state);
	if (findRxPduIdParentConfigurationLeafs(dcmRxPduId, &protocolRx, &mainConnection, &connection, &protocolRow, &runtime)) {
		const Dcm_DslBufferType *externalRxBuffer = protocolRow->DslProtocolRxBufferID;
		if (externalRxBuffer->pduInfo.SduLength >= tpSduLength) { /** @req DCM443 */
			if ((runtime->externalRxBufferStatus == NOT_IN_USE) && (externalRxBuffer->externalBufferRuntimeData->status == BUFFER_AVAILABLE)) {
				DEBUG( DEBUG_MEDIUM, "External buffer available!\n");
				// ### EXTERNAL BUFFER IS AVAILABLE; GRAB IT AND REMEBER THAT WE OWN IT! ###
				externalRxBuffer->externalBufferRuntimeData->status = BUFFER_BUSY;
				runtime->diagnosticRequestFromTester.SduDataPtr = externalRxBuffer->pduInfo.SduDataPtr;
				runtime->diagnosticRequestFromTester.SduLength = tpSduLength;
				*pduInfoPtr = &(runtime->diagnosticRequestFromTester);
				runtime->externalRxBufferStatus = PROVIDED_TO_PDUR; /** @req DCM342 */
				ret = BUFREQ_OK;
			} else {
				if (runtime->externalRxBufferStatus == PROVIDED_TO_DSD) {
					// ### EXTERNAL BUFFER IS IN USE BY THE DSD, TRY TO USE LOCAL BUFFER! ###
					if (runtime->localRxBuffer.status == NOT_IN_USE) {
						if (tpSduLength < DCM_DSL_LOCAL_BUFFER_LENGTH) {
							runtime->localRxBuffer.status = PROVIDED_TO_PDUR;
							runtime->localRxBuffer.PduInfo.SduDataPtr = runtime->localRxBuffer.buffer;
							runtime->localRxBuffer.PduInfo.SduLength = tpSduLength;
							*pduInfoPtr = &(runtime->localRxBuffer.PduInfo);
							ret = BUFREQ_OK;
						} else {
							ret = BUFREQ_BUSY;
						}
					}
				} else {
					// The buffer is in use by the PduR, we can not help this because then
					// we would have two different Rx-indications with same PduId but we
					// will not know which buffer the indication should free.
					ret = BUFREQ_BUSY; /** @req DCM445 */
				}
			}
		} else {
			ret = BUFREQ_OVFL; /** @req DCM444 */
		}
		if (ret == BUFREQ_OK) {
			stopS3SessionTimer(runtime); /** @req DCM141 */
		}
	}
    Irq_Restore(state);
	return ret;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Implements 'void Dcm_RxIndication(PduIdType dcmRxPduId, NotifResultType result)'.
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// 	This function is called called by the PduR typically when CanTp has
//	received the diagnostic request, copied it to the provided buffer and need to indicate
//	this to the DCM (DSL) module via proprietary API.

void DslRxIndicationFromPduR(PduIdType dcmRxPduId, NotifResultType result) {
	const Dcm_DslProtocolRxType *protocolRx = NULL;
	const Dcm_DslMainConnectionType *mainConnection = NULL;
	const Dcm_DslConnectionType *connection = NULL;
	const Dcm_DslProtocolRowType *protocolRow = NULL;
	const Dcm_DspSessionRowType *sessionRow = NULL;
	Dcm_DslRunTimeProtocolParametersType *runtime = NULL;
	Std_ReturnType higherLayerResp;
	imask_t state;

	/** @req DCM345, this needs to be verified when connection to CanIf works. */

	if (findRxPduIdParentConfigurationLeafs(dcmRxPduId, &protocolRx, &mainConnection, &connection, &protocolRow, &runtime)) {
		// We need to find out in what buffer we can find our Rx data (it can
		// be either in the normal RX-buffer or the 'extra' buffer for implementing
		// the Concurrent "Test Present" functionality.
	    Irq_Save(state);
		if (runtime->externalRxBufferStatus == PROVIDED_TO_PDUR) {
			if ( result == NTFRSLT_OK ) { /** @req DCM111 */
				if (isTesterPresentCommand(&(protocolRow->DslProtocolRxBufferID->pduInfo))) {
					startS3SessionTimer(runtime, protocolRow); /** @req DCM141 *//** @req DCM112 *//** @req DCM113 */
					runtime->externalRxBufferStatus = NOT_IN_USE;
					protocolRow->DslProtocolRxBufferID->externalBufferRuntimeData->status = BUFFER_AVAILABLE;
				} else {
					if (runtime->protocolStarted == FALSE) {
						//if there is no active protocol,then start the current protocol
						if(DcmDslRunTimeData.activeProtocol == NULL){
							higherLayerResp = StartProtocolHelper(protocolRow->DslProtocolID); /** @req DCM036 */
							if (higherLayerResp == E_OK) {
								runtime->protocolStarted = TRUE;
								DcmDslRunTimeData.activeProtocol = protocolRow;
								//if it's OBD diagnostic,change session to DCM_OBD_SESSION
								if(DCM_OBD_ON_CAN == protocolRow->DslProtocolID){
									runtime->sessionControl = DCM_OBD_SESSION;
									//startS3SessionTimer(runtime, protocolRow); // init s3.
//									DEBUG( DEBUG_MEDIUM, "runtime->sessionControl = DCM_OBD_SESSION;");
								}								
							}	
						}
						else{
							/*if there is a active protocol and the priority of the current protocol is higher than the priority of the active protocol,
							then preemption will happen.*/
							if(protocolRow->DslProtocolPriority < DcmDslRunTimeData.activeProtocol->DslProtocolPriority){	
								/*@req OBD_DCM_REQ_31*/
								higherLayerResp = StopProtocolHelper(DcmDslRunTimeData.activeProtocol->DslProtocolID);
								if (higherLayerResp == E_OK) {
									if (DcmDslRunTimeData.activeProtocol->DslRunTimeProtocolParameters->externalTxBufferStatus == NOT_IN_USE){
										higherLayerResp = StartProtocolHelper(protocolRow->DslProtocolID); /** @req DCM036 */
										if (higherLayerResp == E_OK) {
											DcmDslRunTimeData.activeProtocol->DslRunTimeProtocolParameters->protocolStarted = FALSE;	/* Close the last protocol*/
											runtime->protocolStarted = TRUE;
											DcmDslRunTimeData.activeProtocol = protocolRow;
											/*if it's OBD diagnostic,change session to DCM_OBD_SESSION*/
											if(DCM_OBD_ON_CAN == protocolRow->DslProtocolID){
												runtime->sessionControl = DCM_OBD_SESSION;
											}								
										}	
									}
									else{

										/*set the flag,activate preemption mechanism*/
										PreemptionNotProcessingDone = TRUE;
										BusyRepeatSent = FALSE;
										/*set Tx buffer status PREEMPT_TRANSMIT_NRC*/
										runtime->externalTxBufferStatus = PREEMPT_TRANSMIT_NRC;
										runtime->responsePendingCount = DCM_Config.Dsl->DslDiagResp->DslDiagRespMaxNumRespPend;
										runtime->diagReqestRxPduId = dcmRxPduId;
										/* Store the preempting protocol */
										DcmDslRunTimeData.preemptingProtocol = protocolRow;
//										DEBUG(DEBUG_MEDIUM,"\n runtime->externalTxBufferStatus = %X\n",runtime->externalTxBufferStatus);
										/*start preemption timer*/
										startPreemptTimer(runtime, protocolRow);/* TODO: Is it correct to restart timer on each new request? */
										/*request PduR to cancel transmit of preempted protocol*/
										/*@req OBD_DCM_REQ_32*//*@req OBD_DCM_REQ_33*/
										higherLayerResp = PduR_CancelTransmitRequest(PDU_CNLOR, DsdDslGetCurrentTxPduId());										
									}
									
								}
								
							}
							else{
								/*if the priority of the current protocol is lower than the priority of the active protocol, we do nothing.*/
								runtime->externalRxBufferStatus = NOT_IN_USE;
								protocolRow->DslProtocolRxBufferID->externalBufferRuntimeData->status = BUFFER_AVAILABLE;

							}

						}
					}
					if (runtime->protocolStarted == TRUE) {
						if (runtime->diagnosticActiveComM == FALSE) {
#if defined(USE_COMM)
							ComM_DCM_ActiveDiagnostic(); /** @req DCM163 */
#endif
							runtime->diagnosticActiveComM = TRUE;
						}
						sessionRow = getActiveSessionRow(runtime);
						runtime->stateTimeoutCount = DCM_CONVERT_MS_TO_MAIN_CYCLES(sessionRow->DspSessionP2ServerMax); /* See 9.2.2. */
						runtime->externalRxBufferStatus = PROVIDED_TO_DSD; /** @req DCM241 */
						if (runtime->externalTxBufferStatus == NOT_IN_USE) {
							DEBUG( DEBUG_MEDIUM, "External Tx buffer available, we can pass it to DSD.\n");
						} else {
							DEBUG( DEBUG_MEDIUM, "External buffer not available, a response is being transmitted?\n");
						}
						runtime->externalTxBufferStatus = PROVIDED_TO_DSD; /** @req DCM241 */
						runtime->responsePendingCount = DCM_Config.Dsl->DslDiagResp->DslDiagRespMaxNumRespPend;
						runtime->diagnosticResponseFromDsd.SduDataPtr = protocolRow->DslProtocolTxBufferID->pduInfo.SduDataPtr;
						runtime->diagnosticResponseFromDsd.SduLength = protocolRow->DslProtocolTxBufferID->pduInfo.SduLength;
						DEBUG( DEBUG_MEDIUM, "DsdDslDataIndication(DcmDslProtocolTxPduId=%d, dcmRxPduId=%d)\n", mainConnection->DslProtocolTx->DcmDslProtocolTxPduId, dcmRxPduId);
						runtime->diagReqestRxPduId = dcmRxPduId;
						DEBUG(DEBUG_MEDIUM,"\n\n runtime->diagnosticRequestFromTester.SduDataPtr[2]  %x\n\n ",runtime->diagnosticRequestFromTester.SduDataPtr[2]);
						DsdDslDataIndication(  // qqq: We are inside a critical section.
								&(runtime->diagnosticRequestFromTester),
								protocolRow->DslProtocolSIDTable,	/** @req DCM035 */
								protocolRx->DslProtocolAddrType,
								mainConnection->DslProtocolTx->DcmDslProtocolTxPduId,
								&(runtime->diagnosticResponseFromDsd),
								dcmRxPduId);
					}
				}
			} else { /** @req DCM344 */
				// The indication was not equal to NTFRSLT_OK, release the resources and no forward to DSD.
				DslResetSessionTimeoutTimer(); /** @req DCM141 */
				runtime->externalRxBufferStatus = NOT_IN_USE;
				protocolRow->DslProtocolRxBufferID->externalBufferRuntimeData->status = BUFFER_AVAILABLE;
			}
		} else {
			// It is the local buffer that was provided to the PduR, that buffer
			// is only used for tester present reception in parallel to diagnostic
			// requests.
			if (runtime->localRxBuffer.status == PROVIDED_TO_PDUR) {
				if ( result == NTFRSLT_OK ) { // Make sure that the data in buffer is valid.
					if (isTesterPresentCommand(&(runtime->localRxBuffer.PduInfo))) {
						startS3SessionTimer(runtime, protocolRow); /** @req DCM141 *//** @req DCM112 *//** @req DCM113 */
					}
				}
				else {
					DslResetSessionTimeoutTimer(); /** @req DCM141 */
				}
				runtime->localRxBuffer.status = NOT_IN_USE;
			}
		}
	    Irq_Restore(state);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Implements 'BufReq_ReturnType Dcm_ProvideTxBuffer(PduIdType dcmTxPduId,
//  PduInfoType **pduInfoPtr, PduLengthType length)'.
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//  This TX-buffer request is likely triggered by the transport layer (i.e. CanTp)
//  after PduR_DcmTransmit() has been called (via PduR to CanTp) indicating that something
//  is to be sent. The PduR_DcmTransmit() call is done from the DSL main function when
//  it has detected that the pending request has been answered by DSD
//  (or any other module?).

BufReq_ReturnType DslProvideTxBuffer(PduIdType dcmTxPduId, const PduInfoType **pduInfoPtr, PduLengthType length) {
	BufReq_ReturnType ret = BUFREQ_NOT_OK;
	const Dcm_DslProtocolTxType *protocolTx = NULL;
	const Dcm_DslMainConnectionType *mainConnection = NULL;
	const Dcm_DslConnectionType *connection = NULL;
	const Dcm_DslProtocolRowType *protocolRow = NULL;
	Dcm_DslRunTimeProtocolParametersType *runtime = NULL;

	(void)length;		// Currently not used, this is only to remove compilation warnings
	DEBUG( DEBUG_MEDIUM, "DslProvideTxBuffer=%d\n", dcmTxPduId);
	if (findTxPduIdParentConfigurationLeafs(dcmTxPduId, &protocolTx, &mainConnection, &connection, &protocolRow, &runtime)) {
		switch (runtime->externalTxBufferStatus) { // ### EXTERNAL TX BUFFER ###
		case DCM_TRANSMIT_SIGNALED: {
			/** @req DCM346 */ /* Length verification is already done if this state is reached. */
			*pduInfoPtr = &(protocolRow->DslProtocolTxBufferID->pduInfo);
			runtime->externalTxBufferStatus = PROVIDED_TO_PDUR; /** @req DCM349 */
			ret = BUFREQ_OK;
			break;
		}
		default:
			DEBUG( DEBUG_MEDIUM, "DCM_TRANSMIT_SIGNALED was not signaled in the external buffer\n");
			ret = BUFREQ_NOT_OK;
			break;
		}
		if (ret == BUFREQ_NOT_OK) {
			switch (runtime->localTxBuffer.status) { // ### LOCAL TX BUFFER ###
			case DCM_TRANSMIT_SIGNALED: {
				runtime->localTxBuffer.PduInfo.SduDataPtr = runtime->localTxBuffer.buffer;
				runtime->localTxBuffer.PduInfo.SduLength = runtime->localTxBuffer.messageLenght;
				*pduInfoPtr = &runtime->localTxBuffer.PduInfo;
				runtime->localTxBuffer.status = PROVIDED_TO_PDUR; // Now the DSL should not touch this Tx-buffer anymore.
				ret = BUFREQ_OK;
				break;
			}
			default:
				DEBUG( DEBUG_MEDIUM, "DCM_TRANSMIT_SIGNALED was not signaled for the local buffer either\n");
				ret = BUFREQ_NOT_OK;
				break;
			}
		}
	}
	return ret;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//	Implements 'void Dcm_TxConfirmation(PduIdType dcmTxPduId, NotifResultType result))'.
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// 	This function is called by the PduR (which has been trigged by i.e. CanTp)
// 	when a transmission has been successfully finished, have had errors or
// 	is even stopped.

void DslTxConfirmation(PduIdType dcmTxPduId, NotifResultType result) {
	const Dcm_DslProtocolTxType *protocolTx = NULL;
	const Dcm_DslMainConnectionType *mainConnection = NULL;
	const Dcm_DslConnectionType *connection = NULL;
	const Dcm_DslProtocolRowType *protocolRow = NULL;
	Dcm_DslRunTimeProtocolParametersType *runtime = NULL;
	imask_t state;

	DEBUG( DEBUG_MEDIUM, "DslTxConfirmation=%d, result=%d\n", dcmTxPduId, result);
	if (findTxPduIdParentConfigurationLeafs(dcmTxPduId, &protocolTx, &mainConnection, &connection, &protocolRow, &runtime)) {
		boolean externalBufferReleased = FALSE;

		// Free the buffer and free the Pdu runtime data buffer.
	    Irq_Save(state);
		switch (runtime->externalTxBufferStatus) { // ### EXTERNAL TX BUFFER ###
		case PROVIDED_TO_PDUR: {
#if defined(USE_COMM)
			ComM_DCM_InactiveDiagnostic(); /** @req DCM164 */
#endif
			startS3SessionTimer(runtime, protocolRow); /** @req DCM141 */
			releaseExternalRxTxBuffers(protocolRow, runtime); /** @req DCM118 *//** @req DCM352 *//** @req DCM353 *//** @req DCM354 */
			externalBufferReleased = TRUE;
		//	DEBUG( DEBUG_MEDIUM, "Released external buffer OK!\n");
			DsdDataConfirmation(mainConnection->DslProtocolTx->DcmDslProtocolTxPduId, result); /** @req DCM117 *//** @req DCM235 */
			break;
		}
		default:
			break;
		}
		if (!externalBufferReleased) {
			switch (runtime->localTxBuffer.status) { // ### LOCAL TX BUFFER ###
			case PROVIDED_TO_PDUR:
				DEBUG( DEBUG_MEDIUM, "Released local buffer buffer OK!\n");
				runtime->localTxBuffer.status = DCM_IDLE;
				break;
			default:
				DEBUG( DEBUG_MEDIUM, "WARNING! DslTxConfirmation could not release external or local buffer!\n");
				break;
			}
		}
	    Irq_Restore(state);
	}
}
