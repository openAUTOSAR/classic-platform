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
#include "Dcm.h"
#include "Dcm_Internal.h"
#include "Det.h"
#include "MemMap.h"

// In "queue" to DSD
static boolean	dsdDslDataIndication = FALSE;


static uint8 currentSid;

void DsdInit(void)
{

}


void DsdMain(void)
{
	if (dsdDslDataIndication) {
		dsdDslDataIndication = FALSE;
		DsdHandleRequest();
	}
}


boolean DsdLookupSid(uint8 sid, Dcm_DsdServiceType **sidPtr)
{
	boolean returnStatus = TRUE;
	Dcm_DsdServiceTableType *currentServiceTable;
	uint16 i;

	DslGetCurrentServiceTable(&currentServiceTable);
	for (i = 0; (currentServiceTable->DsdService[i].DsdSidTabServiceId != sid) && !currentServiceTable->DsdService[i].Arc_EOL; i++);

	if (!currentServiceTable->DsdService[i].Arc_EOL) {
		*sidPtr = (Dcm_DsdServiceType*)&currentServiceTable->DsdService[i];
	}
	else {
		returnStatus = FALSE;
		*sidPtr = NULL;
	}

	return returnStatus;
}


boolean DsdCheckSessionLevel(const Dcm_DspSessionRowType **sessionLevelRefTable)
{
	boolean returnStatus = TRUE;
	Dcm_SesCtrlType currentSession;
	uint16 i;

	DslGetSesCtrlType(&currentSession);
	for (i = 0; (sessionLevelRefTable[i]->DspSessionLevel != currentSession) && !sessionLevelRefTable[i]->Arc_EOL; i++);
	if (sessionLevelRefTable[i]->Arc_EOL) {
		returnStatus = FALSE;
	}

	return returnStatus;
}


boolean DsdCheckSecurityLevel(const Dcm_DspSecurityRowType	**securityLevelRefTable)
{
	boolean returnStatus = TRUE;
	Dcm_SecLevelType currentSecurityLevel;
	uint16 i;

	DslGetSecurityLevel(&currentSecurityLevel);
	for (i = 0; (securityLevelRefTable[i]->DspSecurityLevel != currentSecurityLevel) && !securityLevelRefTable[i]->Arc_EOL; i++);
	if (securityLevelRefTable[i]->Arc_EOL) {
		returnStatus = FALSE;
	}

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
		DspUdsDiagnosticSessionControl();
		break;

	case SID_ECU_RESET:
		DspUdsEcuReset();
		break;

	case SID_CLEAR_DIAGNOSTIC_INFORMATION:
		DspUdsClearDiagnosticInformation();
		break;

	case SID_READ_DTC_INFORMATION:
		DspUdsReadDtcInformation();
		break;

	case SID_READ_DATA_BY_IDENTIFIER:
		break;

	case SID_READ_SCALING_DATA_BY_IDENTIFIER:
		break;

	case SID_SECURITY_ACCESS:
		DspUdsSecurityAccess();
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
		DspUdsTesterPresent();
		break;

	case SID_CONTROL_DTC_SETTING:
		DspUdsControlDtcSetting();
		break;

	default:
		break;
	}
}


void DsdCreateAndSendNcr(Dcm_NegativeResponseCodeType responseCode)
{
	if (!(((dslMsgData.activePduId == DCM_PDU_ID_UDS_FUNC_RX) || (dslMsgData.activePduId == DCM_PDU_ID_OBD_FUNC_RX))
		  && ((responseCode == DCM_E_SERVICENOTSUPPORTED) || (responseCode == DCM_E_SUBFUNCTIONNOTSUPPORTED) || (responseCode == DCM_E_REQUESTOUTOFRANGE)))) {   /** @req DCM001 **/
		dslMsgData.pduTxData->SduDataPtr[0] = SID_NEGATIVE_RESPONSE;
		dslMsgData.pduTxData->SduDataPtr[1] = currentSid;
		dslMsgData.pduTxData->SduDataPtr[2] = responseCode;
		dslMsgData.pduTxData->SduLength = 3;
		DslDsdPduTransmit();	/** @req DCM114 **/ /** @req DCM232.1 **/
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
	if (DCM_RESPOND_ALL_REQUEST || ((dslMsgData.pduRxData->SduDataPtr[0] & 0x7F) < 0x40)) {		/** @req DCM084 **/
		if (DsdLookupSid(dslMsgData.pduRxData->SduDataPtr[0], &sidConfPtr)) {		/** @req DCM192 **/ /** @req DCM193 **/ /** @req DCM196 **/
			// SID found!
			if (DsdCheckSessionLevel(sidConfPtr->DsdSidTabSessionLevelRef) || (sidConfPtr->DsdSidTabServiceId == SID_DIAGNOSTIC_SESSION_CONTROL)) {		 /** @req DCM211 **/
				if (DsdCheckSecurityLevel(sidConfPtr->DsdSidTabSecurityLevelRef)) {	 /** @req DCM217 **/
					if (DCM_REQUEST_INDICATION_ENABLED) {	 /** @req DCM218 **/
						 result = DsdAskApplicationForServicePermission(dslMsgData.pduRxData->SduDataPtr, dslMsgData.pduRxData->SduLength);
					}
					if (!DCM_REQUEST_INDICATION_ENABLED || result == E_OK) {
						// Yes! All conditions met!
						currentSid = dslMsgData.pduRxData->SduDataPtr[0];	/** @req DCM198 **/

						// Check if response shall be suppressed
						if (sidConfPtr->DsdSidTabSubfuncAvail && (dslMsgData.pduRxData->SduDataPtr[1] & SUPPRESS_POS_RESP_BIT)) {	/** @req DCM204 **/
							dslMsgData.suppressPosRspMsg = TRUE;	/** @req DCM202 **/
							dslMsgData.pduRxData->SduDataPtr[1] &= ~SUPPRESS_POS_RESP_BIT;	/** @req DCM201 **/
						}
						else
						{
							dslMsgData.suppressPosRspMsg = FALSE;	/** @req DCM202 **/
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
		if (!dslMsgData.suppressPosRspMsg) {	/** @req DCM200 **/ /** @req DCM231 **/
			/** @req DCM222 **/
			dslMsgData.pduTxData->SduDataPtr[0] = currentSid | SID_RESPONSE_BIT;	/** @req DCM223 **/ /** @req DCM224 **/
			DslDsdPduTransmit();	/** @req DCM114 **/ /** @req DCM225 **/ /** @req DCM232.2 **/
		}
		else {
			DspDcmConfirmation(dslMsgData.activePduId);	/** @req DCM236 **/ /** @req DCM240 **/
			DslResponseSuppressed();
		}
	}
	else {
		DsdCreateAndSendNcr(responseCode);	/** @req DCM228 **/
	}

}


void DsdDataConfirmation(PduIdType confirmPduId)
{
	DspDcmConfirmation(confirmPduId);	/** @req DCM236 **/
}


void DsdDslDataIndication(void)
{
	dsdDslDataIndication = TRUE;
}

