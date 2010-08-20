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
/** @req DCM273 */ /** @req DCM272 */
/** @req DCM039 */ /** @req DCM038 */ /** @req DCM269 */
/** @req DCM271 */ /** @req DCM274 */ /** @req DCM275 */ /** @req DCM424 */
/** @req DCM007 */

#include <string.h>
#include "Dcm.h"
#include "Dcm_Internal.h"
#include "Dem.h"
#include "MemMap.h"
#include "Mcu.h"

#define ZERO_SUB_FUNCTION			0x00

/*
 * Macros
 */
#define BYTES_TO_DTC(hb, mb, lb)	(((uint32)(hb) << 16) | ((uint32)(mb) << 8) | (uint32)(lb))
#define DTC_HIGH_BYTE(dtc)			(((uint32)(dtc) >> 16) & 0xFF)
#define DTC_MID_BYTE(dtc)			(((uint32)(dtc) >> 8) & 0xFF)
#define DTC_LOW_BYTE(dtc)			((uint32)(dtc) & 0xFF)


typedef struct {
	boolean resetPending;
	PduIdType resetPduId;
} DspUdsEcuResetDataType;

static DspUdsEcuResetDataType dspUdsEcuResetData;


typedef struct {
	boolean 						reqInProgress;
	Dcm_SecLevelType				reqSecLevel;
	const Dcm_DspSecurityRowType	*reqSecLevelRef;
} DspUdsSecurityAccessDataType;

static DspUdsSecurityAccessDataType dspUdsSecurityAccesData;


void DspInit(void)
{
	dspUdsSecurityAccesData.reqInProgress = FALSE;
	dspUdsEcuResetData.resetPending = FALSE;
}


void DspMain(void)
{

}


boolean DspCheckSessionLevel(const Dcm_DspSessionRowType **sessionLevelRefTable)
{
	boolean returnStatus = TRUE;
	Dcm_SesCtrlType currentSession;

	DslGetSesCtrlType(&currentSession);
	while (((*sessionLevelRefTable)->DspSessionLevel != currentSession) && !(*sessionLevelRefTable)->Arc_EOL) {
		sessionLevelRefTable++;
	}

	if ((*sessionLevelRefTable)->Arc_EOL) {
		returnStatus = FALSE;
	}

	return returnStatus;
}


boolean DspCheckSecurityLevel(const Dcm_DspSecurityRowType	**securityLevelRefTable)
{
	boolean returnStatus = TRUE;
	Dcm_SecLevelType currentSecurityLevel;

	DslGetSecurityLevel(&currentSecurityLevel);
	while (((*securityLevelRefTable)->DspSecurityLevel != currentSecurityLevel) && !(*securityLevelRefTable)->Arc_EOL) {
		securityLevelRefTable++;
	}
	if ((*securityLevelRefTable)->Arc_EOL) {
		returnStatus = FALSE;
	}

	return returnStatus;
}


static Std_ReturnType askApplicationForSessionPermission(Dcm_SesCtrlType newSessionLevel)
{
	Std_ReturnType returnCode = E_OK;
	const Dcm_DslSessionControlType *sesControl = DCM_Config.Dsl->DslSessionControl;
	Dcm_SesCtrlType currentSessionLevel;
	Std_ReturnType result;

	while (!sesControl->Arc_EOL && (returnCode != E_SESSION_NOT_ALLOWED)) {
		if (sesControl->GetSesChgPermission != NULL) {
			Dcm_GetSesCtrlType(&currentSessionLevel);
			result = sesControl->GetSesChgPermission(currentSessionLevel ,newSessionLevel);
			if (result != E_OK) {
				returnCode = result;
			}
		}
		sesControl++;
	}

	return returnCode;
}


void DspUdsDiagnosticSessionControl(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	/** @req DCM250 */
	const Dcm_DspSessionRowType *sessionRow = DCM_Config.Dsp->DspSession->DspSessionRow;
	Dcm_SesCtrlType reqSessionType;
	Std_ReturnType result;

	if (pduRxData->SduLength == 2) {
		reqSessionType = pduRxData->SduDataPtr[1];
		// Check if type exist in session table
		while ((sessionRow->DspSessionLevel != reqSessionType) && !sessionRow->Arc_EOL) {
			sessionRow++;
		}

		if (!sessionRow->Arc_EOL) {
			result = askApplicationForSessionPermission(reqSessionType);
			if (result == E_OK) {
				DslSetSesCtrlType(reqSessionType);		/** @req DCM311 */
				// Create positive response
				pduTxData->SduDataPtr[1] = reqSessionType;
				pduTxData->SduLength = 2;
				DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
			}
			else {
				// TODO: Add handling of special case of E_FORCE_RCRRP (Dcm138)
				DsdDspProcessingDone(DCM_E_CONDITIONSNOTCORRECT);	/** @req DCM308 */
			}
		}
		else {
			DsdDspProcessingDone(DCM_E_SUBFUNCTIONNOTSUPPORTED);	/** @req DCM307 */
		}
	}
	else {
		// Wrong length
		DsdDspProcessingDone(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
	}
}


void DspUdsEcuReset(const PduInfoType *pduRxData, PduIdType txPduId, PduInfoType *pduTxData)
{
	/** @req DCM260 */
	uint8 reqResetType;

	if (pduRxData->SduLength == 2) {
		reqResetType = pduRxData->SduDataPtr[1];

		switch (reqResetType)
		{
		case 0x01:	// Hard reset
			// TODO: Ask application for permission (Dcm373) (Dcm375) (Dcm377)

			// Schedule the reset
			dspUdsEcuResetData.resetPending = TRUE;
			dspUdsEcuResetData.resetPduId = txPduId;

			// Create positive response
			pduTxData->SduDataPtr[1] = reqResetType;
			pduTxData->SduLength = 2;
			DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
			break;

		default:
			DsdDspProcessingDone(DCM_E_SUBFUNCTIONNOTSUPPORTED);
			break;
		}
	}
	else {
		// Wrong length
		DsdDspProcessingDone(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
	}
}


void DspUdsClearDiagnosticInformation(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	/** @req DCM247 */
	uint32 dtc;
	Dem_ReturnClearDTCType result;

	if (pduRxData->SduLength == 4) {
		dtc = BYTES_TO_DTC(pduRxData->SduDataPtr[1], pduRxData->SduDataPtr[2], pduRxData->SduDataPtr[3]);

		result = Dem_ClearDTC(dtc, DEM_DTC_KIND_ALL_DTCS, DEM_DTC_ORIGIN_PRIMARY_MEMORY); /** @req DCM005 */

		switch (result)
		{
		case DEM_CLEAR_OK:
			// Create positive response
			pduTxData->SduLength = 1;
			DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
			break;

		default:
			DsdDspProcessingDone(DCM_E_REQUESTOUTOFRANGE);
			break;
		}
	}
	else {
		// Wrong length
		DsdDspProcessingDone(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
	}
}


static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x01_0x07_0x11_0x12(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	typedef struct {
		uint8		SID;
		uint8		reportType;
		uint8 		dtcStatusAvailabilityMask;
		uint8		dtcFormatIdentifier;
		uint8		dtcCountHighByte;
		uint8		dtcCountLowByte;
	} TxDataType;

	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	Dem_ReturnSetDTCFilterType setDtcFilterResult;

	// Setup the DTC filter
	switch (pduRxData->SduDataPtr[1]) 	/** @req DCM293 */
	{
	case 0x01:	// reportNumberOfDTCByStatusMask
		setDtcFilterResult = Dem_SetDTCFilter(pduRxData->SduDataPtr[2], DEM_DTC_KIND_ALL_DTCS, DEM_DTC_ORIGIN_PRIMARY_MEMORY, DEM_FILTER_WITH_SEVERITY_NO, VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);
		break;

	case 0x07:	// reportNumberOfDTCBySeverityMaskRecord
		setDtcFilterResult = Dem_SetDTCFilter(pduRxData->SduDataPtr[3], DEM_DTC_KIND_ALL_DTCS, DEM_DTC_ORIGIN_PRIMARY_MEMORY, DEM_FILTER_WITH_SEVERITY_YES, pduRxData->SduDataPtr[2], DEM_FILTER_FOR_FDC_NO);
		break;

	case 0x11:	// reportNumberOfMirrorMemoryDTCByStatusMask
		setDtcFilterResult = Dem_SetDTCFilter(pduRxData->SduDataPtr[2], DEM_DTC_KIND_ALL_DTCS, DEM_DTC_ORIGIN_MIRROR_MEMORY, DEM_FILTER_WITH_SEVERITY_NO, VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);
		break;

	case 0x12:	// reportNumberOfEmissionRelatedOBDDTCByStatusMask
		setDtcFilterResult = Dem_SetDTCFilter(pduRxData->SduDataPtr[2], DEM_DTC_KIND_EMISSION_REL_DTCS, DEM_DTC_ORIGIN_PRIMARY_MEMORY, DEM_FILTER_WITH_SEVERITY_NO, VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);
		break;

	default:
		setDtcFilterResult = DEM_WRONG_FILTER;
		break;
	}

	if (setDtcFilterResult == DEM_FILTER_ACCEPTED) {
		uint16 numberOfFilteredDtc;
		uint8 dtcStatusMask;
		TxDataType *txData = (TxDataType*)pduTxData->SduDataPtr;

		/** @req DCM376 */
		Dem_GetNumberOfFilteredDtc(&numberOfFilteredDtc);
		Dem_GetDTCStatusAvailabilityMask(&dtcStatusMask);

		// Create positive response (ISO 14229-1 table 251)
		txData->reportType = pduRxData->SduDataPtr[1];						// reportType
		txData->dtcStatusAvailabilityMask = dtcStatusMask;					// DTCStatusAvailabilityMask
		txData->dtcFormatIdentifier = Dem_GetTranslationType();				// DTCFormatIdentifier
		txData->dtcCountHighByte = (numberOfFilteredDtc >> 8);				// DTCCount high byte
		txData->dtcCountLowByte = (numberOfFilteredDtc & 0xFF);				// DTCCount low byte
		pduTxData->SduLength = 6;
	}
	else {
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}

	return responseCode;
}


static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x02_0x0A_0x0F_0x13_0x15(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	Dem_ReturnSetDTCFilterType setDtcFilterResult;

	typedef struct {
		uint8		dtcHighByte;
		uint8		dtcMiddleByte;
		uint8		dtcLowByte;
		uint8		statusOfDtc;
	} dtcAndStatusRecordType;

	typedef struct {
		uint8					SID;
		uint8					reportType;
		uint8 					dtcStatusAvailabilityMask;
		dtcAndStatusRecordType	dtcAndStatusRecord[];
	} TxDataType;

	// Setup the DTC filter
	switch (pduRxData->SduDataPtr[1]) 	/** @req DCM378 */
	{
	case 0x02:	// reportDTCByStatusMask
		setDtcFilterResult = Dem_SetDTCFilter(pduRxData->SduDataPtr[2], DEM_DTC_KIND_ALL_DTCS, DEM_DTC_ORIGIN_PRIMARY_MEMORY, DEM_FILTER_WITH_SEVERITY_NO, VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);
		break;

	case 0x0A:	// reportSupportedDTC
		setDtcFilterResult = Dem_SetDTCFilter(DEM_DTC_STATUS_MASK_ALL, DEM_DTC_KIND_ALL_DTCS, DEM_DTC_ORIGIN_PRIMARY_MEMORY, DEM_FILTER_WITH_SEVERITY_NO, VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);
		break;

	case 0x0F:	// reportMirrorMemoryDTCByStatusMask
		setDtcFilterResult = Dem_SetDTCFilter(pduRxData->SduDataPtr[2], DEM_DTC_KIND_ALL_DTCS, DEM_DTC_ORIGIN_MIRROR_MEMORY, DEM_FILTER_WITH_SEVERITY_NO, VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);
		break;

	case 0x13:	// reportEmissionRelatedOBDDTCByStatusMask
		setDtcFilterResult = Dem_SetDTCFilter(pduRxData->SduDataPtr[2], DEM_DTC_KIND_EMISSION_REL_DTCS, DEM_DTC_ORIGIN_PRIMARY_MEMORY, DEM_FILTER_WITH_SEVERITY_NO, VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);
		break;

	case 0x15:	// reportDTCWithPermanentStatus
		setDtcFilterResult = Dem_SetDTCFilter(DEM_DTC_STATUS_MASK_ALL, DEM_DTC_KIND_ALL_DTCS, DEM_DTC_ORIGIN_PERMANENT_MEMORY, DEM_FILTER_WITH_SEVERITY_NO, VALUE_IS_NOT_USED, DEM_FILTER_FOR_FDC_NO);
		break;

	default:
		setDtcFilterResult = DEM_WRONG_FILTER;
		break;
	}

	if (setDtcFilterResult == DEM_FILTER_ACCEPTED) {
		uint8 dtcStatusMask;
		TxDataType *txData = (TxDataType*)pduTxData->SduDataPtr;
		Dem_ReturnGetNextFilteredDTCType getNextFilteredDtcResult;
		uint32 dtc;
		Dem_EventStatusExtendedType dtcStatus;
		uint16 nrOfDtcs = 0;

		/** @req DCM377 */
		Dem_GetDTCStatusAvailabilityMask(&dtcStatusMask);

		// Create positive response (ISO 14229-1 table 252)
		txData->reportType = pduRxData->SduDataPtr[1];
		txData->dtcStatusAvailabilityMask = dtcStatusMask;

		if (dtcStatusMask != 0x00) {	/** @req DCM008 */
			getNextFilteredDtcResult = Dem_GetNextFilteredDTC(&dtc, &dtcStatus);
			while (getNextFilteredDtcResult == DEM_FILTERED_OK) {
				txData->dtcAndStatusRecord[nrOfDtcs].dtcHighByte = DTC_HIGH_BYTE(dtc);
				txData->dtcAndStatusRecord[nrOfDtcs].dtcMiddleByte = DTC_MID_BYTE(dtc);
				txData->dtcAndStatusRecord[nrOfDtcs].dtcLowByte = DTC_LOW_BYTE(dtc);
				txData->dtcAndStatusRecord[nrOfDtcs].statusOfDtc = dtcStatus;
				nrOfDtcs++;
				getNextFilteredDtcResult = Dem_GetNextFilteredDTC(&dtc, &dtcStatus);
			}

			if (getNextFilteredDtcResult != DEM_FILTERED_NO_MATCHING_DTC) {
				responseCode = DCM_E_REQUESTOUTOFRANGE;
			}
		}
		pduTxData->SduLength = 3 + nrOfDtcs * sizeof(dtcAndStatusRecordType);
	}
	else {
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}

	return responseCode;
}


static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x08(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	// TODO: Not supported yet, (DEM module does not currently support severity).
	responseCode = DCM_E_REQUESTOUTOFRANGE;

	return responseCode;
}


static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x09(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	// TODO: Not supported yet, (DEM module does not currently support severity).
	responseCode = DCM_E_REQUESTOUTOFRANGE;

	return responseCode;
}


static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x06_0x10(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	Dem_DTCOriginType dtcOrigin = NULL;
	uint8 startRecNum;
	uint8 endRecNum;

	// Switch on sub function
	switch (pduRxData->SduDataPtr[1]) 	/** @req DCM378 */
	{
	case 0x06:	// reportDTCExtendedDataRecordByDTCNumber
		dtcOrigin = DEM_DTC_ORIGIN_PRIMARY_MEMORY;
		break;

	case 0x10:	// reportMirrorMemoryDTCExtendedDataRecordByDTCNumber
		dtcOrigin = DEM_DTC_ORIGIN_MIRROR_MEMORY;
		break;

	default:
		responseCode = DCM_E_SUBFUNCTIONNOTSUPPORTED;
		break;
	}

	// Switch on record number
	switch (pduRxData->SduDataPtr[5])
	{
	case 0xFF:	// Report all Extended Data Records for a particular DTC
		startRecNum = 0x00;
		endRecNum = 0xEF;
		break;

	case 0xFE:	// Report all OBD Extended Data Records for a particular DTC
		startRecNum = 0x90;
		endRecNum = 0xEF;
		break;

	default:	// Report one specific Extended Data Records for a particular DTC
		startRecNum = pduRxData->SduDataPtr[5];
		endRecNum = startRecNum;
		break;
	}

	if (responseCode == DCM_E_POSITIVERESPONSE) {
		Dem_ReturnGetStatusOfDTCType getStatusOfDtcResult;
		uint32 dtc;
		Dem_EventStatusExtendedType statusOfDtc;

		dtc = BYTES_TO_DTC(pduRxData->SduDataPtr[2], pduRxData->SduDataPtr[3], pduRxData->SduDataPtr[4]);
		getStatusOfDtcResult = Dem_GetStatusOfDTC(dtc, DEM_DTC_KIND_ALL_DTCS, dtcOrigin, &statusOfDtc); /** @req DCM295 */ /** @req DCM475 */
		if (getStatusOfDtcResult == DEM_STATUS_OK) {
			Dem_ReturnGetExtendedDataRecordByDTCType getExtendedDataRecordByDtcResult;
			uint16 recNum;
			uint8 recLength;
			uint16 txIndex = 6;

			/** @req DCM297 */ /** @req DCM474 */ /** @req DCM386 */
			pduTxData->SduDataPtr[1] = pduRxData->SduDataPtr[1];			// Sub function
			pduTxData->SduDataPtr[2] = DTC_HIGH_BYTE(dtc);					// DTC high byte
			pduTxData->SduDataPtr[3] = DTC_MID_BYTE(dtc);					// DTC mid byte
			pduTxData->SduDataPtr[4] = DTC_LOW_BYTE(dtc);					// DTC low byte
			pduTxData->SduDataPtr[5] = statusOfDtc;							// DTC status
			for (recNum = startRecNum; recNum <= endRecNum; recNum++) {
				recLength = pduTxData->SduLength - txIndex -1;	// Calculate what's left in buffer
				/** @req DCM296 */ /** @req DCM476 */ /** @req DCM382 */
				getExtendedDataRecordByDtcResult = Dem_GetExtendedDataRecordByDTC(dtc, DEM_DTC_KIND_ALL_DTCS, dtcOrigin, recNum, &pduTxData->SduDataPtr[txIndex+1], &recLength);
				if (getExtendedDataRecordByDtcResult == DEM_RECORD_OK) {
					pduTxData->SduDataPtr[txIndex++] = recNum;
					/* Instead of calling Dem_GetSizeOfExtendedDataRecordByDTC() the result from Dem_GetExtendedDataRecordByDTC() is used */
					/** @req DCM478 */ /** @req DCM479 */ /** @req DCM480 */
					txIndex += recLength;
				}
				else {
					// TODO: What to do here?
				}
			}
			pduTxData->SduLength = txIndex;
		}
		else {
			responseCode = DCM_E_REQUESTOUTOFRANGE;
		}
	}

	return responseCode;
}


static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x03(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	// TODO: Not supported yet
	responseCode = DCM_E_REQUESTOUTOFRANGE;

	return responseCode;
}


static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x04(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	// TODO: Not supported yet
	responseCode = DCM_E_REQUESTOUTOFRANGE;

	return responseCode;
}


static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x05(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	// TODO: Not supported yet
	responseCode = DCM_E_REQUESTOUTOFRANGE;

	return responseCode;
}


static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x0B_0x0C_0x0D_0x0E(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	// TODO: Not supported yet
	responseCode = DCM_E_REQUESTOUTOFRANGE;

	return responseCode;
}


static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x14(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	// TODO: Not supported yet
	responseCode = DCM_E_REQUESTOUTOFRANGE;

	return responseCode;
}


void DspUdsReadDtcInformation(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	/** @req DCM248 */
	// Sub function number         0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 10 11 12 13 14 15
	const uint8 sduLength[0x16] = {0, 3, 3, 6, 6, 3, 6, 4, 4, 5, 2, 2, 2, 2, 2, 3, 6, 3, 3, 3, 2, 2};

	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	uint8 subFunctionNumber = pduRxData->SduDataPtr[1];

	// Check length
	if (subFunctionNumber <= 0x15) {
		if (pduRxData->SduLength == sduLength[subFunctionNumber]) {
			switch (subFunctionNumber)
			{
			case 0x01:	// reportNumberOfDTCByStatusMask
			case 0x07:	// reportNumberOfDTCBySeverityMaskRecord
			case 0x11:	// reportNumberOfMirrorMemoryDTCByStatusMask
			case 0x12:	// reportNumberOfEmissionRelatedOBDDTCByStatusMask
				responseCode = udsReadDtcInfoSub_0x01_0x07_0x11_0x12(pduRxData, pduTxData);
				break;

			case 0x02:	// reportDTCByStatusMask
			case 0x0A:	// reportSupportedDTC
			case 0x0F:	// reportMirrorMemoryDTCByStatusMask
			case 0x13:	// reportEmissionRelatedOBDDTCByStatusMask
			case 0x15:	// reportDTCWithPermanentStatus
				responseCode = udsReadDtcInfoSub_0x02_0x0A_0x0F_0x13_0x15(pduRxData, pduTxData);
				break;

			case 0x08:	// reportDTCBySeverityMaskRecord
				responseCode = udsReadDtcInfoSub_0x08(pduRxData, pduTxData);
				break;

			case 0x09:	// reportSeverityInformationOfDTC
				responseCode = udsReadDtcInfoSub_0x09(pduRxData, pduTxData);
				break;

			case 0x06:	// reportDTCExtendedDataRecordByDTCNumber
			case 0x10:	// reportMirrorMemoryDTCExtendedDataRecordByDTCNumber
				responseCode = udsReadDtcInfoSub_0x06_0x10(pduRxData, pduTxData);
				break;

			case 0x03:	// reportDTCSnapshotIdentidication
				responseCode = udsReadDtcInfoSub_0x03(pduRxData, pduTxData);
				break;

			case 0x04:	// reportDTCSnapshotByDtcNumber
				responseCode = udsReadDtcInfoSub_0x04(pduRxData, pduTxData);
				break;

			case 0x05:	// reportDTCSnapshotRecordNumber
				responseCode = udsReadDtcInfoSub_0x05(pduRxData, pduTxData);
				break;

			case 0x0B:	// reportFirstTestFailedDTC
			case 0x0C:	// reportFirstConfirmedDTC
			case 0x0D:	// reportMostRecentTestFailedDTC
			case 0x0E:	// reportMostRecentConfirmedDTC
				responseCode = udsReadDtcInfoSub_0x0B_0x0C_0x0D_0x0E(pduRxData, pduTxData);
				break;

			case 0x14:	// reportDTCFaultDetectionCounter
				responseCode = udsReadDtcInfoSub_0x14(pduRxData, pduTxData);
				break;

			default:
				// Unknown sub function
				responseCode = DCM_E_REQUESTOUTOFRANGE;
				break;
			}
		}
		else {
			// Wrong length
			responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		}
	}
	else {
		// Sub function out of range
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}

	DsdDspProcessingDone(responseCode);
}


static boolean lookupDid(uint16 didNr, const Dcm_DspDidType **didPtr)
{
	const Dcm_DspDidType *dspDid = DCM_Config.Dsp->DspDid;
	boolean didFound = FALSE;

	while ((dspDid->DspDidIdentifier != didNr) &&  !dspDid->Arc_EOL) {
		dspDid++;
	}

	if (!dspDid->Arc_EOL) {
		didFound = TRUE;
		*didPtr = dspDid;
	}

	return didFound;
}


static Dcm_NegativeResponseCodeType readDidData(const Dcm_DspDidType *didPtr, PduInfoType *pduTxData, uint16 *txPos)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	if ((didPtr->DspDidInfoRef->DspDidAccess.DspDidRead != NULL) && (didPtr->DspDidConditionCheckReadFnc != NULL) && (didPtr->DspDidReadDataFnc != NULL)) {	/** @req DCM433 */
		if (DspCheckSessionLevel(didPtr->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSessionRef)) { /** @req DCM434 */
			if (DspCheckSecurityLevel(didPtr->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSecurityLevelRef)) { /** @req DCM435 */
				Std_ReturnType result;
				Dcm_NegativeResponseCodeType errorCode;
				result = didPtr->DspDidConditionCheckReadFnc(&errorCode);
				if ((result == E_OK) && (errorCode == DCM_E_POSITIVERESPONSE)) {	/** @req DCM439 */
					uint16 didLen;
					result = E_NOT_OK;
					if (didPtr->DspDidInfoRef->DspDidFixedLength) {	/** @req DCM436 */
						didLen = didPtr->DspDidSize;
						result = E_OK;
					}
					else {
						if (didPtr->DspDidReadDataLengthFnc != NULL) {
							result = didPtr->DspDidReadDataLengthFnc(&didLen);
						}
					}

					if (result == E_OK) {
						// Now ready for reading the data!
						if ((*txPos + didLen + 2) <= pduTxData->SduLength) {
							pduTxData->SduDataPtr[*txPos] = (didPtr->DspDidIdentifier >> 8) & 0xFF;
							(*txPos)++;
							pduTxData->SduDataPtr[*txPos] = (didPtr->DspDidIdentifier >> 0) & 0xFF;
							(*txPos)++;
							result = didPtr->DspDidReadDataFnc(&pduTxData->SduDataPtr[*txPos]);	/** @req DCM437 */
							*txPos += didLen;

							if (result != E_OK) {
								responseCode = DCM_E_CONDITIONSNOTCORRECT;
							}
						}
						else { // tx buffer full
							responseCode = DCM_E_REQUESTOUTOFRANGE;
						}
					}
					else {	// Not possible to obtain did length
						responseCode = DCM_E_CONDITIONSNOTCORRECT;
					}
				}
				else {	// CheckRead failed
					responseCode = DCM_E_CONDITIONSNOTCORRECT;
				}
			}
			else {	// Not allowed in current security level
				responseCode = DCM_E_SECUTITYACCESSDENIED;
			}
		}
		else {	// Not allowed in current session
			responseCode = DCM_E_SERVICENOTSUPPORTEDINACTIVESESSION;
		}
	}
	else {	// Read access not configured
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}

	if (responseCode == DCM_E_POSITIVERESPONSE) {
		// Recurse trough the rest of the dids. 	/** @req DCM440 */
		uint16 i;
		for (i=0; (!didPtr->DspDidRef[i]->Arc_EOL) && (responseCode == DCM_E_POSITIVERESPONSE); i++) {
			responseCode = readDidData(didPtr->DspDidRef[i], pduTxData, txPos);
		}
	}

	return responseCode;
}

void DspUdsReadDataByIdentifier(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	/** @req DCM253 */
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	uint8 nrOfDids;
	uint16 didNr;
	const Dcm_DspDidType *didPtr = NULL;

	uint16 txPos = 1;
	uint16 i;

	if ((pduRxData->SduLength - 1) % 2 == 0 ) {
		nrOfDids = (pduRxData->SduLength - 1) / 2;

		for (i = 0; (i < nrOfDids) && (responseCode == DCM_E_POSITIVERESPONSE); i++) {
			didNr = (pduRxData->SduDataPtr[1+i*2] << 8) + pduRxData->SduDataPtr[2+i*2];
			if (lookupDid(didNr, &didPtr)) {	/** @req DCM438 */
				responseCode = readDidData(didPtr, pduTxData, &txPos);
			}
			else { // DID not found
				responseCode = DCM_E_REQUESTOUTOFRANGE;
			}
		}
	}
	else {
		responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}

	if (responseCode == DCM_E_POSITIVERESPONSE) {
		pduTxData->SduLength = txPos;
	}

	DsdDspProcessingDone(responseCode);
}


static Dcm_NegativeResponseCodeType readDidScalingData(const Dcm_DspDidType *didPtr, PduInfoType *pduTxData, uint16 *txPos)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	if (didPtr->DspDidGetScalingInfoFnc != NULL) {
		uint16 scalingInfoLen;

		scalingInfoLen = didPtr->DspDidInfoRef->DspDidScalingInfoSize;
		if ((*txPos + scalingInfoLen + 2) <= pduTxData->SduLength) {
			Std_ReturnType result;
			Dcm_NegativeResponseCodeType errorCode;

			pduTxData->SduDataPtr[*txPos] = (didPtr->DspDidIdentifier >> 8) & 0xFF;
			(*txPos)++;
			pduTxData->SduDataPtr[*txPos] = (didPtr->DspDidIdentifier >> 0) & 0xFF;
			(*txPos)++;
			result = didPtr->DspDidGetScalingInfoFnc(&pduTxData->SduDataPtr[*txPos], &errorCode);	/** @req DCM394 */
			*txPos += scalingInfoLen;

			if ((result != E_OK) || (errorCode != DCM_E_POSITIVERESPONSE)) {
				responseCode = DCM_E_CONDITIONSNOTCORRECT;
			}
		}
		else { // tx buffer full
			responseCode = DCM_E_REQUESTOUTOFRANGE;
		}
	}
	else {	// DspDidGetScalingInfoFnc null pointer
		responseCode = DCM_E_CONDITIONSNOTCORRECT;
	}

	return responseCode;
}

void DspUdsReadScalingDataByIdentifier(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	/** @req DCM258 */
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	uint16 didNr;
	const Dcm_DspDidType *didPtr = NULL;

	uint16 txPos = 1;

	if (pduRxData->SduLength == 3) {
		didNr = (pduRxData->SduDataPtr[1] << 8) + pduRxData->SduDataPtr[2];
		if (lookupDid(didNr, &didPtr)) {
			responseCode = readDidScalingData(didPtr, pduTxData, &txPos);
		}
		else { // DID not found
			responseCode = DCM_E_REQUESTOUTOFRANGE;
		}

		if (responseCode == DCM_E_POSITIVERESPONSE) {
			pduTxData->SduLength = txPos;
		}
	}
	else {
		// Length not ok
		responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}

	DsdDspProcessingDone(responseCode);
}


static Dcm_NegativeResponseCodeType writeDidData(const Dcm_DspDidType *didPtr, const PduInfoType *pduRxData, uint16 writeDidLen)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	if ((didPtr->DspDidInfoRef->DspDidAccess.DspDidWrite != NULL) && (didPtr->DspDidConditionCheckWriteFnc != NULL) && (didPtr->DspDidWriteDataFnc != NULL)) {	/** @req DCM468 */
		if (DspCheckSessionLevel(didPtr->DspDidInfoRef->DspDidAccess.DspDidWrite->DspDidWriteSessionRef)) { /** @req DCM469 */
			if (DspCheckSecurityLevel(didPtr->DspDidInfoRef->DspDidAccess.DspDidWrite->DspDidWriteSecurityLevelRef)) { /** @req DCM470 */
				Std_ReturnType result;
				Dcm_NegativeResponseCodeType errorCode;
				result = didPtr->DspDidConditionCheckWriteFnc(&errorCode);	/** @req DCM471 */
				if ((result == E_OK) && (errorCode == DCM_E_POSITIVERESPONSE)) {
					uint16 didLen;
					result = E_NOT_OK;
					if (didPtr->DspDidInfoRef->DspDidFixedLength) {	/** @req DCM472 */
						didLen = didPtr->DspDidSize;
						result = E_OK;
					}
					else {
						if (didPtr->DspDidReadDataLengthFnc != NULL) {
							result = didPtr->DspDidReadDataLengthFnc(&didLen);
						}
					}

					if (result == E_OK) {
						if (didLen == writeDidLen) {	/** @req DCM473 */
							result = didPtr->DspDidWriteDataFnc(&pduRxData->SduDataPtr[3], didLen, &errorCode);	/** @req DCM395 */
							if ((result != E_OK) || (errorCode != DCM_E_POSITIVERESPONSE)) {
								responseCode = DCM_E_CONDITIONSNOTCORRECT;
							}
						}
						else {
							responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
						}
					}
					else {	// Not possible to obtain did length
						responseCode = DCM_E_CONDITIONSNOTCORRECT;
					}
				}
				else {	// CheckRead failed
					responseCode = DCM_E_CONDITIONSNOTCORRECT;
				}
			}
			else {	// Not allowed in current security level
				responseCode = DCM_E_SECUTITYACCESSDENIED;
			}
		}
		else {	// Not allowed in current session
			responseCode = DCM_E_SERVICENOTSUPPORTEDINACTIVESESSION;
		}
	}
	else {	// Read access not configured
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}

	return responseCode;
}

void DspUdsWriteDataByIdentifier(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	/** @req DCM255 */
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	uint16 didNr;
	const Dcm_DspDidType *didPtr = NULL;

	uint16 didDataLength;

	didDataLength = pduRxData->SduLength - 3;
	didNr = (pduRxData->SduDataPtr[1] << 8) + pduRxData->SduDataPtr[2];
	if (lookupDid(didNr, &didPtr)) {	/** @req DCM467 */
		responseCode = writeDidData(didPtr, pduRxData, didDataLength);
	}
	else { // DID not found
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}

	if (responseCode == DCM_E_POSITIVERESPONSE) {
		pduTxData->SduLength = 3;
		pduTxData->SduDataPtr[1] = (didNr >> 8) & 0xFF;
		pduTxData->SduDataPtr[2] = (didNr >> 0) & 0xFF;
	}

	DsdDspProcessingDone(responseCode);
}


void DspUdsSecurityAccess(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	/** @req DCM252 */
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	// Check sub function range (0x01 to 0x42)
	if ((pduRxData->SduDataPtr[1] >= 0x01) && (pduRxData->SduDataPtr[1] <= 0x42)) {
		boolean isRequestSeed = pduRxData->SduDataPtr[1] & 0x01;
		Dcm_SecLevelType requestedSecurityLevel = (pduRxData->SduDataPtr[1]-1)/2;
		Dcm_NegativeResponseCodeType getSeedErrorCode;

		if (isRequestSeed) {
			// requestSeed message
			// Check if type exist in security table
			const Dcm_DspSecurityRowType *securityRow = &DCM_Config.Dsp->DspSecurity->DspSecurityRow[0];
			while ((securityRow->DspSecurityLevel != requestedSecurityLevel) && !securityRow->Arc_EOL) {
				securityRow++;
			}
			if (!securityRow->Arc_EOL) {
				// Check length
				if (pduRxData->SduLength == (2 + securityRow->DspSecurityADRSize)) {	/** @req DCM321.RequestSeed */
					Dcm_SecLevelType activeSecLevel;
					Dcm_GetSecurityLevel(&activeSecLevel);
					if (requestedSecurityLevel == activeSecLevel) {		/** @req DCM323 */
						pduTxData->SduDataPtr[1] = pduRxData->SduDataPtr[1];
						// If same level set the seed to zeroes
						memset(&pduTxData->SduDataPtr[2], 0, securityRow->DspSecuritySeedSize);
						pduTxData->SduLength = 2 + securityRow->DspSecuritySeedSize;
					}
					else {
						// New security level ask for seed
						if (securityRow->GetSeed != NULL) {
							Std_ReturnType getSeedResult;
							getSeedResult = securityRow->GetSeed(&pduRxData->SduDataPtr[2], &pduTxData->SduDataPtr[2], &getSeedErrorCode); /** @req DCM324.RequestSeed */
							if ((getSeedResult == E_OK) && (getSeedErrorCode == E_OK)) {
								// Everything ok add sub function to tx message and send it.
								pduTxData->SduDataPtr[1] = pduRxData->SduDataPtr[1];
								pduTxData->SduLength = 2 + securityRow->DspSecuritySeedSize;

								dspUdsSecurityAccesData.reqSecLevel = requestedSecurityLevel;
								dspUdsSecurityAccesData.reqSecLevelRef = securityRow;
								dspUdsSecurityAccesData.reqInProgress = TRUE;
							}
							else {
								// GetSeed returned not ok
								responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
							}
						} else {
							responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
						}
					}
				}
				else {
					// Length not ok
					responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
				}
			}
			else {
				// Requested security level not configured
				responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
			}
		}
		else {
			// sendKey message
			if (dspUdsSecurityAccesData.reqInProgress) {
				if (pduRxData->SduLength == (2 + dspUdsSecurityAccesData.reqSecLevelRef->DspSecurityKeySize)) {	/** @req DCM321.SendKey */
					if (requestedSecurityLevel == dspUdsSecurityAccesData.reqSecLevel) {
						if (dspUdsSecurityAccesData.reqSecLevelRef->CompareKey != NULL) {
							Std_ReturnType compareKeyResult;
							compareKeyResult = dspUdsSecurityAccesData.reqSecLevelRef->CompareKey(&pduRxData->SduDataPtr[2]); /** @req DCM324.SendKey */
							if (compareKeyResult == E_OK) {
								// Request accepted
								// Kill timer
								DslSetSecurityLevel(dspUdsSecurityAccesData.reqSecLevelRef->DspSecurityLevel); /** @req DCM325 */
								dspUdsSecurityAccesData.reqInProgress = FALSE;
								pduTxData->SduDataPtr[1] = pduRxData->SduDataPtr[1];
								pduTxData->SduLength = 2;
							}
							else {
								responseCode = DCM_E_CONDITIONSNOTCORRECT;
							}
						} else {
							responseCode = DCM_E_CONDITIONSNOTCORRECT;
						}
					}
					else {
						responseCode = DCM_E_CONDITIONSNOTCORRECT;
					}
				}
				else {
					// Length not ok
					responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
				}
			}
			else {
				// sendKey request without a preceding requestSeed
				responseCode = DCM_E_REQUESTSEQUENCEERROR;
			}
		}
	}
	else {
		responseCode = DCM_E_SUBFUNCTIONNOTSUPPORTED;
	}

	DsdDspProcessingDone(responseCode);
}


void DspUdsTesterPresent(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	/** @req DCM251 */
	if (pduRxData->SduLength == 2) {
		switch (pduRxData->SduDataPtr[1])
		{
		case ZERO_SUB_FUNCTION:
			DslResetSessionTimeoutTimer();
			// Create positive response
			pduTxData->SduDataPtr[1] = ZERO_SUB_FUNCTION;
			pduTxData->SduLength = 2;
			DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
			break;

		default:
			DsdDspProcessingDone(DCM_E_SUBFUNCTIONNOTSUPPORTED);
			break;
		}
	}
	else {
		// Wrong length
		DsdDspProcessingDone(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
	}
}


void DspUdsControlDtcSetting(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	/** @req DCM249 */
	Dem_ReturnControlDTCStorageType resultCode;

	if (pduRxData->SduLength == 2) {
		switch (pduRxData->SduDataPtr[1])
		{
		case 0x01:	// ON
			resultCode = Dem_EnableDTCStorage(DEM_DTC_GROUP_ALL_DTCS, DEM_DTC_KIND_ALL_DTCS);		/** @req DCM304 */
			if (resultCode == DEM_CONTROL_DTC_STORAGE_OK) {
				pduTxData->SduDataPtr[1] = 0x01;
				pduTxData->SduLength = 2;
				DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
			}
			else {
				DsdDspProcessingDone(DCM_E_REQUESTOUTOFRANGE);
			}
			break;

		case 0x02:	// OFF
			resultCode = Dem_DisableDTCStorage(DEM_DTC_GROUP_ALL_DTCS, DEM_DTC_KIND_ALL_DTCS);		/** @req DCM406 */
			if (resultCode == DEM_CONTROL_DTC_STORAGE_OK) {
				pduTxData->SduDataPtr[1] = 0x02;
				pduTxData->SduLength = 2;
				DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
			}
			else {
				DsdDspProcessingDone(DCM_E_REQUESTOUTOFRANGE);
			}
			break;

		default:
			DsdDspProcessingDone(DCM_E_SUBFUNCTIONNOTSUPPORTED);
			break;
		}
	}
	else {
		// Wrong length
		DsdDspProcessingDone(DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
	}
}


void DspDcmConfirmation(PduIdType confirmPduId)
{
	if (dspUdsEcuResetData.resetPending) {
		if (confirmPduId == dspUdsEcuResetData.resetPduId) {
			dspUdsEcuResetData.resetPending = FALSE;
#if ( MCU_PERFORM_RESET_API == STD_ON )
			Mcu_PerformReset();
#else
			DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_UDS_RESET_ID, DCM_E_NOT_SUPPORTED);
#endif
		}
	}
}



