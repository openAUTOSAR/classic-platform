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

//lint -esym(754, SID)	//Structure member SID not used in udsReadDtcInfoSub_0x01_0x07_0x11_0x12() and udsReadDtcInfoSub_0x02_0x0A_0x0F_0x13_0x15()


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
#if defined(USE_MCU)
#include "Mcu.h"
#endif

/*
 * Macros
 */
#define ZERO_SUB_FUNCTION				0x00
#define DCM_FORMAT_LOW_MASK			0x0F
#define DCM_FORMAT_HIGH_MASK			0xF0
#define DCM_MEMORY_ADDRESS_MASK		0xFFFFFF
#define DCM_DID_HIGH_MASK 				0xFF00			
#define DCM_DID_LOW_MASK				0xFF
#define DCM_PERODICDID_HIHG_MASK		0xF200
#define SID_AND_ALFID_LEN2   0x2
#define SID_AND_ALFID_LEN4   0x4
#define SID_AND_ALFID_LEN5	0x5
#define SID_AND_ALFID_LEN6   0x6
#define SID_AND_ALFID_LEN7   0x7

#define BYTES_TO_DTC(hb, mb, lb)	(((uint32)(hb) << 16) | ((uint32)(mb) << 8) | (uint32)(lb))
#define DTC_HIGH_BYTE(dtc)			(((uint32)(dtc) >> 16) & 0xFFu)
#define DTC_MID_BYTE(dtc)			(((uint32)(dtc) >> 8) & 0xFFu)
#define DTC_LOW_BYTE(dtc)			((uint32)(dtc) & 0xFFu)


typedef struct {
	boolean resetPending;
	PduIdType resetPduId;
} DspUdsEcuResetDataType;

static DspUdsEcuResetDataType dspUdsEcuResetData;
static boolean dspWritePending;

typedef struct {
	boolean 						reqInProgress;
	Dcm_SecLevelType				reqSecLevel;
	const Dcm_DspSecurityRowType	*reqSecLevelRef;
} DspUdsSecurityAccessDataType;

static DspUdsSecurityAccessDataType dspUdsSecurityAccesData;

typedef enum{
	DCM_MEMORY_UNUSED,
	DCM_MEMORY_READ,
	DCM_MEMORY_WRITE,
	DCM_MEMORY_FAILED	
}Dcm_DspMemoryStateType;
Dcm_DspMemoryStateType dspMemoryState;

typedef enum{
	DCM_DDD_SOURCE_DEFAULT,
	DCM_DDD_SOURCE_DID,
	DCM_DDD_SOURCE_ADDRESS
}Dcm_DspDDDTpyeID;

typedef struct{
	uint32 PDidTxCounter;
	uint32 PDidTxCounterNumber;
	uint8  PeriodicDid;
}Dcm_pDidType;/* a type to save  the periodic DID and cycle */

typedef struct{
	Dcm_pDidType dspPDid[DCM_LIMITNUMBER_PERIODDATA];	/*a buffer to save the periodic DID and cycle   */
	uint8 PDidNr;										/* note the number of periodic DID is used */
}Dsp_pDidRefType;

Dsp_pDidRefType dspPDidRef; 

typedef struct{
	uint8   formatOrPosition;						/*note the formate of address and size*/
	uint32 SourceAddressOrDid;								/*note the memory address */
	uint16 Size;										/*note the memory size */
	Dcm_DspDDDTpyeID DDDTpyeID;
}Dcm_DspDDDSourceType;

typedef struct{
	uint16 DynamicallyDid;
	Dcm_DspDDDSourceType DDDSource[DCM_MAX_DDDSOURCE_NUMBER];
}
Dcm_DspDDDType;

Dcm_DspDDDType dspDDD[DCM_MAX_DDD_NUMBER];


/*
 * * static Function
 */

static boolean lookupReadMemory(uint32 memoryAddress,
								uint8  memoryAddressFormat,
								uint32 memorySize,
								const Dcm_DspMemoryIdInfo **MemoryPtr);

static boolean LookupDDD(uint16 didNr, const Dcm_DspDDDType **DDid);

static boolean checkWriteMemoryByAddress(boolean useId,
		                                 uint32 memoryAddress,
										 uint8 memoryAddressFormat,
										 uint32 memorySize,
										 const Dcm_DspMemoryIdInfo *dspMemory);


/*
*   end  
*/

void DspInit(void)
{
	dspUdsSecurityAccesData.reqInProgress = FALSE;
	dspUdsEcuResetData.resetPending = FALSE;

	dspWritePending = FALSE;
	dspMemoryState=DCM_MEMORY_UNUSED;
	/* clear periodic send buffer */
	memset(&dspPDidRef,0,sizeof(dspPDidRef));
	/* clear dynamically Did buffer */
	memset(&dspDDD[0],0,sizeof(dspDDD)); 
}

void DspMemoryMainFunction(void)
{
	Dcm_ReturnWriteMemoryType WriteRet;
	Dcm_ReturnReadMemoryType ReadRet;
	switch(dspMemoryState)
	{
		case DCM_MEMORY_UNUSED:
			break;
		case DCM_MEMORY_READ:
			ReadRet = Dcm_ReadMemory(DCM_PENDING,0,0,0,0);
			if(ReadRet == DCM_READ_OK)/*asynchronous writing is ok*/
			{
				DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
				dspMemoryState = DCM_MEMORY_UNUSED;
			}
			if(ReadRet == DCM_READ_FAILED)
			{
				dspMemoryState = DCM_MEMORY_FAILED;
			}
			break;
		case DCM_MEMORY_WRITE:
			WriteRet = Dcm_WriteMemory(DCM_PENDING,0,0,0,0);
			if(WriteRet == DCM_WRITE_OK)/*asynchronous writing is ok*/
			{
				DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
				dspMemoryState = DCM_MEMORY_UNUSED;
			}
			if(WriteRet == DCM_WRITE_FAILED)
			{
				dspMemoryState = DCM_MEMORY_FAILED;
			}
			break;
		case DCM_MEMORY_FAILED:
			DsdDspProcessingDone(DCM_E_GENERALPROGRAMMINGFAILURE);
			dspMemoryState = DCM_MEMORY_UNUSED;
			break;
			
	}
}
void DspPeriodicDIDMainFunction()
{
	uint8 i;
	boolean sentResponseThisLoop = FALSE;

	for(i = 0;i < dspPDidRef.PDidNr; i++)
	{
		if(dspPDidRef.dspPDid[i].PDidTxCounterNumber > dspPDidRef.dspPDid[i].PDidTxCounter)
		{
			dspPDidRef.dspPDid[i].PDidTxCounter++;
		}
		else
		{
			if( sentResponseThisLoop  == FALSE ) {
				if (E_OK == DslInternal_ResponseOnOneDataByPeriodicId(dspPDidRef.dspPDid[i].PeriodicDid)){
					dspPDidRef.dspPDid[i].PDidTxCounter = 0;
					/*AutoSar  DCM  8.10.5 */
					sentResponseThisLoop = TRUE;
				}
			}
			else {
				/* Don't do anything - PDid will be sent next loop */
			}
		}	
	}
}
void DspMain(void)
{
	DspMemoryMainFunction();
	DspPeriodicDIDMainFunction();
}


boolean DspCheckSessionLevel(Dcm_DspSessionRowType const* const* sessionLevelRefTable)
{
	Std_ReturnType returnStatus;
	boolean levelFound = FALSE;
	Dcm_SesCtrlType currentSession;

	returnStatus = DslGetSesCtrlType(&currentSession);
	if (returnStatus == E_OK) {
		while ( ((*sessionLevelRefTable)->DspSessionLevel != currentSession) && (!(*sessionLevelRefTable)->Arc_EOL) ) {
			sessionLevelRefTable++;
		}

		if (!(*sessionLevelRefTable)->Arc_EOL) {
			levelFound = TRUE;
		}
	}

	return levelFound;
}


boolean DspCheckSecurityLevel(Dcm_DspSecurityRowType const* const* securityLevelRefTable)
{
	Std_ReturnType returnStatus;
	boolean levelFound = FALSE;
	Dcm_SecLevelType currentSecurityLevel;

	returnStatus = DslGetSecurityLevel(&currentSecurityLevel);
	if (returnStatus == E_OK) {
		while ( ((*securityLevelRefTable)->DspSecurityLevel != currentSecurityLevel) && (!(*securityLevelRefTable)->Arc_EOL) ) {
			securityLevelRefTable++;
		}
		if (!(*securityLevelRefTable)->Arc_EOL) {
			levelFound = TRUE;
		}
	}

	return levelFound;
}


static Std_ReturnType askApplicationForSessionPermission(Dcm_SesCtrlType newSessionLevel)
{
	Std_ReturnType returnCode = E_OK;
	const Dcm_DslSessionControlType *sesControl = DCM_Config.Dsl->DslSessionControl;
	Dcm_SesCtrlType currentSessionLevel;
	Std_ReturnType result;

	while ( (!sesControl->Arc_EOL) && (returnCode != E_SESSION_NOT_ALLOWED)) {
		if (sesControl->GetSesChgPermission != NULL) {
			result = Dcm_GetSesCtrlType(&currentSessionLevel);
			if (result == E_OK) {
				result = sesControl->GetSesChgPermission(currentSessionLevel ,newSessionLevel);
				if (result != E_OK) {
					returnCode = result;
				}
			} else {
				returnCode = E_NOT_OK;
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
		while ((sessionRow->DspSessionLevel != reqSessionType) && (!sessionRow->Arc_EOL) ) {
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
		Std_ReturnType result;
		Dem_ReturnGetNumberOfFilteredDTCType getNumerResult;
		uint16 numberOfFilteredDtc;
		uint8 dtcStatusMask;
		//lint --e(826)	PC-Lint exception - Suspicious pointer conversion
		//lint --e(927)	PC-Lint exception - Pointer to pointer cast
		TxDataType *txData = (TxDataType*)pduTxData->SduDataPtr;

		/** @req DCM376 */
		getNumerResult = Dem_GetNumberOfFilteredDtc(&numberOfFilteredDtc);
		if (getNumerResult == DEM_NUMBER_OK) {
			result = Dem_GetDTCStatusAvailabilityMask(&dtcStatusMask);
			if (result != E_OK) {
				dtcStatusMask = 0;
			}

			// Create positive response (ISO 14229-1 table 251)
			txData->reportType = pduRxData->SduDataPtr[1];						// reportType
			txData->dtcStatusAvailabilityMask = dtcStatusMask;					// DTCStatusAvailabilityMask
			txData->dtcFormatIdentifier = Dem_GetTranslationType();				// DTCFormatIdentifier
			txData->dtcCountHighByte = (numberOfFilteredDtc >> 8);				// DTCCount high byte
			txData->dtcCountLowByte = (numberOfFilteredDtc & 0xFFu);			// DTCCount low byte
			pduTxData->SduLength = 6;
		} else {
			// TODO: What to do?
			responseCode = DCM_E_GENERALREJECT;
		}
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
		//lint --e(826)	PC-Lint exception - Suspicious pointer conversion
		//lint --e(927)	PC-Lint exception - Pointer to pointer cast
		TxDataType *txData = (TxDataType*)pduTxData->SduDataPtr;
		Dem_ReturnGetNextFilteredDTCType getNextFilteredDtcResult;
		uint32 dtc;
		Dem_EventStatusExtendedType dtcStatus;
		uint16 nrOfDtcs = 0;
		Std_ReturnType result;

		/** @req DCM377 */
		result = Dem_GetDTCStatusAvailabilityMask(&dtcStatusMask);
		if (result != E_OK) {
			dtcStatusMask = 0;
		}

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
		pduTxData->SduLength = (PduLengthType)(3 + (nrOfDtcs * sizeof(dtcAndStatusRecordType)));
	}
	else {
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}

	return responseCode;
}

// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715, 838, 818}		Symbol not referenced, responseCode not used, txData should be const
static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x08(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	// TODO: Not supported yet, (DEM module does not currently support severity).
	responseCode = DCM_E_REQUESTOUTOFRANGE;

	return responseCode;
}


// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715, 838, 818}		Symbol not referenced, responseCode not used, txData should be const
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
	Dem_DTCOriginType dtcOrigin;
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
		dtcOrigin = 0;
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
			uint8 recNum;
			uint16 recLength;
			uint16 txIndex = 6;

			/** @req DCM297 */ /** @req DCM474 */ /** @req DCM386 */
			pduTxData->SduDataPtr[1] = pduRxData->SduDataPtr[1];			// Sub function
			pduTxData->SduDataPtr[2] = DTC_HIGH_BYTE(dtc);					// DTC high byte
			pduTxData->SduDataPtr[3] = DTC_MID_BYTE(dtc);					// DTC mid byte
			pduTxData->SduDataPtr[4] = DTC_LOW_BYTE(dtc);					// DTC low byte
			pduTxData->SduDataPtr[5] = statusOfDtc;							// DTC status
			for (recNum = startRecNum; recNum <= endRecNum; recNum++) {
				recLength = pduTxData->SduLength - (txIndex + 1);	// Calculate what's left in buffer
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


// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715, 838, 818}		Symbol not referenced, responseCode not used, txData should be const
static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x03(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	// TODO: Not supported yet
	responseCode = DCM_E_REQUESTOUTOFRANGE;

	return responseCode;
}


// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715, 838, 818}		Symbol not referenced, responseCode not used, txData should be const
static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x04(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	// 1. Only consider Negative Response 0x10

	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	Dem_DTCKindType	DtcType = 0;
	Dem_DTCOriginType  DtcOrigin = 0;
	uint32 DtcNumber = 0;
	uint8 RecordNumber = 0;
	uint8 SizeOfTxBuf = pduTxData->SduLength;
	uint8 AvailableBufSize = 0;
	uint8 RecNumOffset = 0;
	uint16 index = 0;
	uint16 EventIndex =0;
	uint16 FFIdNumber = 0;
	Dem_ReturnGetFreezeFrameDataByDTCType GetFFbyDtcReturnCode = DEM_GET_FFDATABYDTC_OK;
	Dem_ReturnGetStatusOfDTCType GetStatusOfDtc = DEM_STATUS_OK;
	Dem_EventStatusExtendedType DtcStatus = 0;
	Dem_EventParameterType *pEventParaTemp = NULL;

	// Now let's assume DTC has 3 bytes.
	DtcNumber = (((uint32)pduRxData->SduDataPtr[2])<<16) +
				(((uint32)pduRxData->SduDataPtr[3])<<8) +
				((uint32)pduRxData->SduDataPtr[4]);

	RecordNumber = pduRxData->SduDataPtr[5];

	for (EventIndex = 0; DEM_Config.ConfigSet->EventParameter[EventIndex].Arc_EOL != TRUE; EventIndex++){
		// search each event linked to this DTC
		if (DEM_Config.ConfigSet->EventParameter[EventIndex].DTCClassRef->DTC == DtcNumber){
			pEventParaTemp = (Dem_EventParameterType *)(&DEM_Config.ConfigSet->EventParameter[EventIndex]);
		}
		else {
			pEventParaTemp = NULL;
		}

		if (pEventParaTemp != NULL) {
			DtcType = pEventParaTemp->DTCClassRef->DTCKind;
			//DtcOrigin = pEventParaTemp->EventClass->EventDestination[?];
			// now use DEM_DTC_ORIGIN_PRIMARY_MEMORY as default.
			DtcOrigin = DEM_DTC_ORIGIN_PRIMARY_MEMORY;
			pduTxData->SduDataPtr[6 + RecNumOffset] = RecordNumber;

			// get Dids' number
			for (index = 0; pEventParaTemp->FreezeFrameClassRef[index] != NULL; index++){
				if (pEventParaTemp->FreezeFrameClassRef[index]->FFRecordNumber == RecordNumber) {
					// Calculate the Number of Dids in FF
					for (FFIdNumber = 0; pEventParaTemp->FreezeFrameClassRef[index]->FFIdClassRef[FFIdNumber].Arc_EOL != FALSE; FFIdNumber++) {
						;
					}
				}
			}
			pduTxData->SduDataPtr[7 + RecNumOffset] = FFIdNumber;

			// get FF data
			AvailableBufSize = SizeOfTxBuf - 7 - RecNumOffset;
			GetFFbyDtcReturnCode = Dem_GetFreezeFrameDataByDTC(DtcNumber, DtcType, DtcOrigin,
					RecordNumber, &pduTxData->SduDataPtr[8 + RecNumOffset], &AvailableBufSize);
			if (GetFFbyDtcReturnCode != DEM_GET_FFDATABYDTC_OK){
				break;
			}
			RecNumOffset = RecNumOffset + AvailableBufSize;
			pduTxData->SduLength = 8 + RecNumOffset;
		}
	}

	// Negative response
	switch (GetFFbyDtcReturnCode) {
		case DEM_GET_FFDATABYDTC_OK:
			break;
		default:
			responseCode = DCM_E_GENERALREJECT;
			return responseCode;
	}

	GetStatusOfDtc = Dem_GetStatusOfDTC(DtcNumber, DtcType, DtcOrigin, &DtcStatus); /** @req DEM212 */
	switch (GetStatusOfDtc) {
		case DEM_STATUS_OK:
			break;
		default:
			responseCode = DCM_E_GENERALREJECT;
			return responseCode;
	}


	// Positive response
	// See ISO 14229(2006) Table 254
	pduTxData->SduDataPtr[0] = 0x59;	// positive response
	pduTxData->SduDataPtr[1] = 0x04;	// subid
	pduTxData->SduDataPtr[2] = pduRxData->SduDataPtr[2];	// DTC
	pduTxData->SduDataPtr[3] = pduRxData->SduDataPtr[3];
	pduTxData->SduDataPtr[4] = pduRxData->SduDataPtr[4];
	pduTxData->SduDataPtr[5] = (uint8)DtcStatus;	//status
	return responseCode;
}


// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715, 838, 818}		Symbol not referenced, responseCode not used, txData should be const
static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x05(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	// TODO: Not supported yet
	responseCode = DCM_E_REQUESTOUTOFRANGE;

	return responseCode;
}


// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715, 838, 818}		Symbol not referenced, responseCode not used, txData should be const
static Dcm_NegativeResponseCodeType udsReadDtcInfoSub_0x0B_0x0C_0x0D_0x0E(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	// TODO: Not supported yet
	responseCode = DCM_E_REQUESTOUTOFRANGE;

	return responseCode;
}


// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715, 838, 818}		Symbol not referenced, responseCode not used, txData should be const
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
/**
**		This Function for check the pointer of Dynamically Did Sourced by Did buffer using a didNr
**/
static boolean LookupDDD(uint16 didNr,  const Dcm_DspDDDType **DDid )	
{
	uint8 i;
	boolean ret = FALSE;
	const Dcm_DspDDDType* DDidptr = &dspDDD[0];
	
	for(i = 0;((i < DCM_MAX_DDD_NUMBER) && (ret == FALSE)); i++)
	{
		if(DDidptr->DynamicallyDid == didNr)
		{
			ret = TRUE;
		
		}
		else
		{
			DDidptr++;
		}
	}
	if(ret == TRUE)
	{
		*DDid = DDidptr;
	}

	return ret;
}

static boolean lookupDid(uint16 didNr, const Dcm_DspDidType **didPtr)
{
	const Dcm_DspDidType *dspDid = DCM_Config.Dsp->DspDid;
	boolean didFound = FALSE;

	while ((dspDid->DspDidIdentifier != didNr) &&  (!dspDid->Arc_EOL)) {
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
					uint16 didLen = 0;
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
							pduTxData->SduDataPtr[*txPos] = (didPtr->DspDidIdentifier >> 8) & 0xFFu;
							(*txPos)++;
							pduTxData->SduDataPtr[*txPos] = didPtr->DspDidIdentifier & 0xFFu;
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

/**
**		This Function for read Dynamically Did data buffer Sourced by Memory address using a didNr
**/
static Dcm_NegativeResponseCodeType readDDDData( Dcm_DspDDDType *PDidPtr, uint8 *Data,uint16 *Length)
{
	uint8 i;
	uint8 dataCount;
	uint8 AddressFormat;
	uint16 SourceDataLength = 0;
	const Dcm_DspMemoryIdInfo *SourceMemoryInfoptr = NULL;
	const Dcm_DspDidType *SourceDidPtr = NULL;
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	*Length = 0;

	for(i = 0;(i < DCM_MAX_DDDSOURCE_NUMBER) && (PDidPtr->DDDSource[i].formatOrPosition != 0)
		&&(responseCode == DCM_E_POSITIVERESPONSE);i++)
	{
		if(PDidPtr->DDDSource[i].DDDTpyeID == DCM_DDD_SOURCE_ADDRESS)
		{

			AddressFormat = PDidPtr->DDDSource[i].formatOrPosition&DCM_FORMAT_HIGH_MASK >> 4;
			if(TRUE == lookupReadMemory(PDidPtr->DDDSource[i].SourceAddressOrDid,AddressFormat,
				PDidPtr->DDDSource[i].Size, &SourceMemoryInfoptr))
			{
				if(DspCheckSecurityLevel(SourceMemoryInfoptr->pReadMemoryInfo->pSecurityLevel) != TRUE)
				{
					responseCode = DCM_E_SECUTITYACCESSDENIED;
				}
			}
			else
			{
				responseCode = DCM_E_REQUESTOUTOFRANGE;
			}
			if(responseCode == DCM_E_POSITIVERESPONSE)
			{
				Dcm_ReadMemory(DCM_INITIAL,SourceMemoryInfoptr->MemoryIdValue,
										PDidPtr->DDDSource[i].SourceAddressOrDid,
										PDidPtr->DDDSource[i].Size,
										(Data + *Length));
				*Length = *Length + PDidPtr->DDDSource[i].Size;
			}
		}
		else if(PDidPtr->DDDSource[i].DDDTpyeID == DCM_DDD_SOURCE_DID)
		{
			
			if(lookupDid(PDidPtr->DDDSource[i].SourceAddressOrDid,&SourceDidPtr) == TRUE)
			{
				if(DspCheckSecurityLevel(SourceDidPtr->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSecurityLevelRef) != TRUE)
				{
					responseCode = DCM_E_SECUTITYACCESSDENIED;
				}
				if(SourceDidPtr->DspDidInfoRef->DspDidFixedLength == TRUE)
				{
					SourceDataLength = SourceDidPtr->DspDidSize;
				}
				else
				{
					if(SourceDidPtr->DspDidReadDataLengthFnc != NULL)
					{
						SourceDidPtr->DspDidReadDataLengthFnc(&SourceDataLength);
					}
				}
				if((SourceDidPtr->DspDidReadDataFnc != NULL) && (SourceDataLength != 0) && (DCM_E_POSITIVERESPONSE == responseCode))
				{
				
					SourceDidPtr->DspDidReadDataFnc((Data + *Length));
					for(dataCount = 0;dataCount < SourceDataLength;dataCount++)
					{
						if(dataCount < PDidPtr->DDDSource[i].Size)
						{
							*(Data + *Length + dataCount) = *(Data + *Length + dataCount + PDidPtr->DDDSource[i].formatOrPosition - 1);
						}
						else
						{
							*(Data + *Length + dataCount) = 0;	
						}
					}
					*Length = *Length + PDidPtr->DDDSource[i].Size;
				}
				else
				{
					responseCode = DCM_E_REQUESTOUTOFRANGE;
				}
			}
			else
			{
				responseCode = DCM_E_REQUESTOUTOFRANGE;
			}
		}
		else
		{
			
			responseCode = DCM_E_REQUESTOUTOFRANGE;	
		}
	}
	return responseCode;
}

void DspUdsReadDataByIdentifier(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	/** @req DCM253 */
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	uint16 nrOfDids;
	uint16 didNr;
	const Dcm_DspDidType *didPtr = NULL;
	Dcm_DspDDDType *DDidPtr=NULL;
	uint16 txPos = 1;
	uint16 i;
	uint16 Length;

	if ( ((pduRxData->SduLength - 1) % 2) == 0 ) {
		nrOfDids = (pduRxData->SduLength - 1) / 2;

		for (i = 0; (i < nrOfDids) && (responseCode == DCM_E_POSITIVERESPONSE); i++) 
			{
			didNr = (uint16)((uint16)pduRxData->SduDataPtr[1 + (i * 2)] << 8) + pduRxData->SduDataPtr[2 + (i * 2)];
			if (lookupDid(didNr, &didPtr)) {	/** @req DCM438 */
				responseCode = readDidData(didPtr, pduTxData, &txPos);
			}

			else if(LookupDDD(didNr,(const Dcm_DspDDDType **)&DDidPtr) == TRUE)
			{
				/*DCM 651,DCM 652*/
				pduTxData->SduDataPtr[txPos] = (DDidPtr->DynamicallyDid>>8) & 0xFF;
				txPos++;
				pduTxData->SduDataPtr[txPos] = (uint8)(DDidPtr->DynamicallyDid & 0xFF);
				txPos++;
				responseCode = readDDDData(DDidPtr,&(pduTxData->SduDataPtr[txPos]), &Length);
				txPos = txPos + Length;
			}

			else
			{ // DID not found
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


static Dcm_NegativeResponseCodeType readDidScalingData(const Dcm_DspDidType *didPtr, const PduInfoType *pduTxData, uint16 *txPos)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	if (didPtr->DspDidGetScalingInfoFnc != NULL) {
		uint16 scalingInfoLen;

		scalingInfoLen = didPtr->DspDidInfoRef->DspDidScalingInfoSize;
		if ((*txPos + scalingInfoLen + 2) <= pduTxData->SduLength) {
			Std_ReturnType result;
			Dcm_NegativeResponseCodeType errorCode;

			pduTxData->SduDataPtr[*txPos] = (didPtr->DspDidIdentifier >> 8) & 0xFFu;
			(*txPos)++;
			pduTxData->SduDataPtr[*txPos] = didPtr->DspDidIdentifier & 0xFFu;
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
		didNr = (uint16)((uint16)pduRxData->SduDataPtr[1] << 8) + pduRxData->SduDataPtr[2];
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
					uint16 didLen = 0;
					result = E_NOT_OK;
					if (didPtr->DspDidInfoRef->DspDidFixedLength) {	/** @req DCM472 */
						didLen = didPtr->DspDidSize;
						result = E_OK;
					}
					else {
						if (didPtr->DspDidReadDataLengthFnc != NULL) {
							result = didPtr->DspDidReadDataLengthFnc(&didLen);
						}					}

					if (result == E_OK) {
						if (didLen == writeDidLen) {	/** @req DCM473 */
							result = didPtr->DspDidWriteDataFnc(&pduRxData->SduDataPtr[3], (uint8)didLen, &responseCode);	/** @req DCM395 */
							if( result != E_OK && responseCode == DCM_E_POSITIVERESPONSE ) {
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
	didNr = (uint16)((uint16)pduRxData->SduDataPtr[1] << 8) + pduRxData->SduDataPtr[2];
	if (lookupDid(didNr, &didPtr)) {	/** @req DCM467 */
		responseCode = writeDidData(didPtr, pduRxData, didDataLength);
	}
	else { // DID not found
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}

	if (responseCode == DCM_E_POSITIVERESPONSE) {
		pduTxData->SduLength = 3;
		pduTxData->SduDataPtr[1] = (didNr >> 8) & 0xFFu;
		pduTxData->SduDataPtr[2] = didNr & 0xFFu;
	}

	DsdDspProcessingDone(responseCode);
}


void DspUdsSecurityAccess(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	/** @req DCM252 */
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	// Check sub function range (0x01 to 0x42)
	if ((pduRxData->SduDataPtr[1] >= 0x01) && (pduRxData->SduDataPtr[1] <= 0x42)) {
		boolean isRequestSeed = pduRxData->SduDataPtr[1] & 0x01u;
		Dcm_SecLevelType requestedSecurityLevel = (pduRxData->SduDataPtr[1]-1)/2;
		Dcm_NegativeResponseCodeType getSeedErrorCode;

		if (isRequestSeed) {
			// requestSeed message
			// Check if type exist in security table
			const Dcm_DspSecurityRowType *securityRow = &DCM_Config.Dsp->DspSecurity->DspSecurityRow[0];
			while ((securityRow->DspSecurityLevel != requestedSecurityLevel) && (!securityRow->Arc_EOL)) {
				securityRow++;
			}
			if (!securityRow->Arc_EOL) {
				// Check length
				if (pduRxData->SduLength == (2 + securityRow->DspSecurityADRSize)) {	/** @req DCM321.RequestSeed */
					Dcm_SecLevelType activeSecLevel;
					Std_ReturnType result;
					result = Dcm_GetSecurityLevel(&activeSecLevel);
					if (result == E_OK) {
						if (requestedSecurityLevel == activeSecLevel) {		/** @req DCM323 */
							pduTxData->SduDataPtr[1] = pduRxData->SduDataPtr[1];
							// If same level set the seed to zeroes
							memset(&pduTxData->SduDataPtr[2], 0, securityRow->DspSecuritySeedSize);
							pduTxData->SduLength = 2 + securityRow->DspSecuritySeedSize;
						} else {
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
					} else {
						// TODO: What to do?
						responseCode = DCM_E_GENERALREJECT;
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


static boolean lookupRoutine(uint16 routineId, const Dcm_DspRoutineType **routinePtr)
{
	const Dcm_DspRoutineType *dspRoutine = DCM_Config.Dsp->DspRoutine;
	boolean routineFound = FALSE;

	while ((dspRoutine->DspRoutineIdentifier != routineId) &&  (!dspRoutine->Arc_EOL)) {
		dspRoutine++;
	}

	if (!dspRoutine->Arc_EOL) {
		routineFound = TRUE;
		*routinePtr = dspRoutine;
	}

	return routineFound;
}


static Dcm_NegativeResponseCodeType startRoutine(const Dcm_DspRoutineType *routinePtr, const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	Std_ReturnType routineResult;

	// startRoutine
	if ((routinePtr->DspStartRoutineFnc != NULL) && (routinePtr->DspRoutineInfoRef->DspStartRoutine != NULL)) {
		if (((routinePtr->DspRoutineInfoRef->DspStartRoutine->DspStartRoutineCtrlOptRecSize + 4) == pduRxData->SduLength)
			&& ((routinePtr->DspRoutineInfoRef->DspStartRoutine->DspStartRoutineStsOptRecSize + 4) <= pduTxData->SduLength)) {
			pduTxData->SduLength = routinePtr->DspRoutineInfoRef->DspStartRoutine->DspStartRoutineStsOptRecSize + 4;
			routineResult = routinePtr->DspStartRoutineFnc(&pduRxData->SduDataPtr[4], &pduTxData->SduDataPtr[4], &responseCode);	/** @req DCM400 */ /** @req DCM401 */
			if (routineResult != E_OK) {
				responseCode = DCM_E_CONDITIONSNOTCORRECT;
			}
		}
		else {
			responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		}
	}
	else {
		responseCode = DCM_E_CONDITIONSNOTCORRECT;
	}

	return responseCode;
}


static Dcm_NegativeResponseCodeType stopRoutine(const Dcm_DspRoutineType *routinePtr, const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	Std_ReturnType routineResult;

	// stopRoutine
	if ((routinePtr->DspStopRoutineFnc != NULL) && (routinePtr->DspRoutineInfoRef->DspRoutineStop != NULL)) {
		if (((routinePtr->DspRoutineInfoRef->DspRoutineStop->DspStopRoutineCtrlOptRecSize + 4) == pduRxData->SduLength)
			&& ((routinePtr->DspRoutineInfoRef->DspRoutineStop->DspStopRoutineStsOptRecSize + 4) <= pduTxData->SduLength)) {
			pduTxData->SduLength = routinePtr->DspRoutineInfoRef->DspRoutineStop->DspStopRoutineStsOptRecSize + 4;
			routineResult = routinePtr->DspStopRoutineFnc(&pduRxData->SduDataPtr[4], &pduTxData->SduDataPtr[4], &responseCode);	/** @req DCM402 */ /** @req DCM403 */
			if (routineResult != E_OK) {
				responseCode = DCM_E_CONDITIONSNOTCORRECT;
			}
		}
		else {
			responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		}
	}
	else {
		responseCode = DCM_E_CONDITIONSNOTCORRECT;
	}

	return responseCode;
}


static Dcm_NegativeResponseCodeType requestRoutineResults(const Dcm_DspRoutineType *routinePtr, PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	Std_ReturnType routineResult;

	// requestRoutineResults
	if ((routinePtr->DspRequestResultRoutineFnc != NULL) && (routinePtr->DspRoutineInfoRef->DspRoutineRequestRes != NULL)) {
		if ((routinePtr->DspRoutineInfoRef->DspRoutineRequestRes->DspReqResRtnCtrlOptRecSize + 4) <= pduTxData->SduLength) {
			pduTxData->SduLength = routinePtr->DspRoutineInfoRef->DspRoutineRequestRes->DspReqResRtnCtrlOptRecSize + 4;
			routineResult = routinePtr->DspRequestResultRoutineFnc(&pduTxData->SduDataPtr[4], &responseCode);	/** @req DCM404 */ /** @req DCM405 */
			if (routineResult != E_OK) {
				responseCode = DCM_E_CONDITIONSNOTCORRECT;
			}
		}
		else {
			responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		}
	}
	else {
		responseCode = DCM_E_CONDITIONSNOTCORRECT;
	}

	return responseCode;
}


void DspUdsRoutineControl(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	/** @req DCM257 */
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	uint8 subFunctionNumber = 0;
	uint16 routineId = 0;
	const Dcm_DspRoutineType *routinePtr = NULL;

	if (pduRxData->SduLength >= 4) {
		subFunctionNumber = pduRxData->SduDataPtr[1];
		if ((subFunctionNumber > 0) && (subFunctionNumber < 4)) {
			routineId = (uint16)((uint16)pduRxData->SduDataPtr[2] << 8) + pduRxData->SduDataPtr[3];
			if (lookupRoutine(routineId, &routinePtr)) {
				if (DspCheckSessionLevel(routinePtr->DspRoutineInfoRef->DspRoutineAuthorization.DspRoutineSessionRef)) {
					if (DspCheckSecurityLevel(routinePtr->DspRoutineInfoRef->DspRoutineAuthorization.DspRoutineSecurityLevelRef)) {
						switch (subFunctionNumber) {
						case 0x01:	// startRoutine
							responseCode = startRoutine(routinePtr, pduRxData, pduTxData);
							break;

						case 0x02:	// stopRoutine
							responseCode = stopRoutine(routinePtr, pduRxData, pduTxData);
							break;

						case 0x03:	// requestRoutineResults
							responseCode =  requestRoutineResults(routinePtr, pduTxData);
							break;

						default:	// This shall never happen
							responseCode = DCM_E_SUBFUNCTIONNOTSUPPORTED;
							break;
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
			else {	// Unknown routine identifier
				responseCode = DCM_E_REQUESTOUTOFRANGE;
			}
		}
		else {	// Sub function not supported
			responseCode = DCM_E_SUBFUNCTIONNOTSUPPORTED;
		}
	}
	else {
		// Wrong length
		responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}

	if (responseCode == DCM_E_POSITIVERESPONSE) {
		// Add header to the positive response message
		pduTxData->SduDataPtr[1] = subFunctionNumber;
		pduTxData->SduDataPtr[2] = (routineId >> 8) & 0xFFu;
		pduTxData->SduDataPtr[3] = routineId & 0xFFu;
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
#if defined(USE_MCU) && ( MCU_PERFORM_RESET_API == STD_ON )
			Mcu_PerformReset();
#else
			DET_REPORTERROR(MODULE_ID_DCM, 0, DCM_UDS_RESET_ID, DCM_E_NOT_SUPPORTED);
#endif
		}
	}
}


static boolean CheckReadMemoryByAddress( boolean useId,uint32 memoryAddress,
										uint8 memoryAddressFormat,
										uint32 memorySize,
										const Dcm_DspMemoryIdInfo  *dspMemory)
{
	boolean ret = FALSE;
	uint8 MemoryId;

	if(useId == FALSE)
	{
		/*@req DCM493*/
		if((memoryAddress >= dspMemory->pReadMemoryInfo->MemoryAddressLow)
			&& (memoryAddress <= dspMemory->pReadMemoryInfo->MemoryAddressHigh)
			&& (memoryAddress + memorySize - 1 <= dspMemory->pReadMemoryInfo->MemoryAddressHigh))
		{
			ret = TRUE;
		}
	}
	else
	{
		MemoryId = (uint8)(memoryAddress >> ((memoryAddressFormat - 1)*8));
		memoryAddress = (uint32)(memoryAddress & DCM_MEMORY_ADDRESS_MASK);

		if((MemoryId == dspMemory->MemoryIdValue)&&
			(memoryAddress >= dspMemory->pReadMemoryInfo->MemoryAddressLow)
			&& (memoryAddress <= dspMemory->pReadMemoryInfo->MemoryAddressHigh)
			&& (memoryAddress + memorySize - 1 <= dspMemory->pReadMemoryInfo->MemoryAddressHigh))
		{
			ret = TRUE;
		}
	}
	
	return ret;
}

static boolean lookupReadMemory(uint32 memoryAddress,
								uint8  memoryAddressFormat,
								uint32 memorySize,
								const Dcm_DspMemoryIdInfo  **MemoryInfoPtr)
{
	uint8 i;
	boolean memoryFound = FALSE;
	const Dcm_DspMemoryIdInfo *dspMemoryInfo = DCM_Config.Dsp->DspMemory->DspMemoryIdInfo;

	for(i = 0; (dspMemoryInfo->Arc_EOL == FALSE) && (memoryFound == FALSE); i++)
	{
		if(TRUE == CheckReadMemoryByAddress(DCM_Config.Dsp->DspMemory->DcmDspUseMemoryId,memoryAddress,memoryAddressFormat,memorySize,dspMemoryInfo))
		{
			memoryFound = TRUE;
		}
		else
		{
			dspMemoryInfo++;
		}
	}
	if (memoryFound == TRUE)
	{
		*MemoryInfoPtr = dspMemoryInfo;
	}
	
	return memoryFound;
}

static Dcm_NegativeResponseCodeType readMemoryData( Dcm_OpStatusType *OpStatus,
													const Dcm_DspMemoryIdInfo *MemoryPtr,
													uint32 MemoryAddress,
													uint32 MemorySize,
													PduInfoType *pduTxData)
{
	Dcm_ReturnReadMemoryType ReadRet;
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	ReadRet = Dcm_ReadMemory(*OpStatus,MemoryPtr->MemoryIdValue,
									MemoryAddress,
									MemorySize,
									&pduTxData->SduDataPtr[1]);
	if(DCM_READ_FAILED == ReadRet)
	{
		responseCode = DCM_E_GENERALPROGRAMMINGFAILURE;  /*@req Dcm644*/
	}
	if (DCM_READ_PENDING == ReadRet)
	{
		*OpStatus = DCM_READ_PENDING;
	}	
	return responseCode;
}

/*@req Dcm442,DCM492*/
void DspUdsReadMemoryByAddress(const PduInfoType *pduRxData, PduInfoType *pduTxData)
{
	typedef struct{
		uint32 MemoryAddressStart;					/*  low  address of a  memory block to read or write*/
		uint32 MemoryIdValue;						/*  memory ID  to read or write uint used by parameter */
		const Dcm_DspMemoryIdInfo *MemoryIdConfigPtr;		
		Dcm_NegativeResponseCodeType ResponseCode; 
		uint8 MemorySizeFormat;
		uint8 MemoryAddressFormat;
		uint32 MemorySize;
		uint16 MessageLength;
	}DspUdsReadMemoryByAddressType;	/*the typed used for  SID read memory by address*/
	uint8 i;
	DspUdsReadMemoryByAddressType dspReadMemoryByAddress;
	Dcm_OpStatusType OpStatus = 0;
	dspReadMemoryByAddress.MemoryAddressStart = 0;
	dspReadMemoryByAddress.MemorySize = 0;
	dspReadMemoryByAddress.MemoryIdConfigPtr = NULL;
	dspReadMemoryByAddress.ResponseCode = DCM_E_POSITIVERESPONSE;
	dspReadMemoryByAddress.MemorySizeFormat = ((uint8)(pduRxData->SduDataPtr[1] & DCM_FORMAT_HIGH_MASK)) >> 4;	/*@req UDS_REQ_0x23_1 & UDS_REQ_0x23_5*/
	dspReadMemoryByAddress.MemoryAddressFormat = ((uint8)(pduRxData->SduDataPtr[1])) & 0x0Fu;   /*@req UDS_REQ_0x23_1 & UDS_REQ_0x23_5*/

	if((dspReadMemoryByAddress.MemoryAddressFormat == 0)||(dspReadMemoryByAddress.MemorySizeFormat == 0))
	{
		dspReadMemoryByAddress.ResponseCode = DCM_E_REQUESTOUTOFRANGE;  /*UDS_REQ_0x23_10*/
	}
	else
	{
		dspReadMemoryByAddress.MessageLength = (uint16)(dspReadMemoryByAddress.MemoryAddressFormat + dspReadMemoryByAddress.MemorySizeFormat + SID_AND_ALFID_LEN2);
		if(dspReadMemoryByAddress.MessageLength == (uint16)(pduRxData->SduLength))
		{
			/*take start address out */
			for(i = 0; i < dspReadMemoryByAddress.MemoryAddressFormat; i++)
			{
				dspReadMemoryByAddress.MemoryAddressStart <<= 8;
				dspReadMemoryByAddress.MemoryAddressStart += (uint32)(pduRxData->SduDataPtr[SID_AND_ALFID_LEN2 + i]);
			}

			/*take value of MemorySize out */
			for(i = 0; i < dspReadMemoryByAddress.MemorySizeFormat; i++)
			{
				dspReadMemoryByAddress.MemorySize <<= 8;
				dspReadMemoryByAddress.MemorySize += (uint32)(pduRxData->SduDataPtr[2 + i + dspReadMemoryByAddress.MemoryAddressFormat]);
			}
			if(dspReadMemoryByAddress.MemorySize < DCM_PROTOCAL_TP_MAX_LENGTH)
			{
				if(TRUE == lookupReadMemory(dspReadMemoryByAddress.MemoryAddressStart,
											dspReadMemoryByAddress.MemoryAddressFormat,
											dspReadMemoryByAddress.MemorySize,
											&dspReadMemoryByAddress.MemoryIdConfigPtr))
				{
					if (DspCheckSecurityLevel(dspReadMemoryByAddress.MemoryIdConfigPtr->pReadMemoryInfo->pSecurityLevel) == TRUE)
					{
						dspReadMemoryByAddress.ResponseCode = readMemoryData(&OpStatus,dspReadMemoryByAddress.MemoryIdConfigPtr,
																			dspReadMemoryByAddress.MemoryAddressStart,
																			dspReadMemoryByAddress.MemorySize,pduTxData);/*@req UDS_REQ_0x23_9*/
					
					}
					else
					{
						dspReadMemoryByAddress.ResponseCode = DCM_E_SECUTITYACCESSDENIED;/*@req UDS_REQ_0x23_11,@req DCM494*/
					}
				}
				else
				{
					dspReadMemoryByAddress.ResponseCode = DCM_E_REQUESTOUTOFRANGE;      /*@req UDS_REQ_0x23_7,UDS_REQ_0x23_8*/
				}
			}
			else
			{
				dspReadMemoryByAddress.ResponseCode = DCM_E_REQUESTOUTOFRANGE;
			}
		}
		else
		{
			dspReadMemoryByAddress.ResponseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT; /*@req UDS_REQ_0x23_6*/
		}
	}
	if(DCM_E_POSITIVERESPONSE == dspReadMemoryByAddress.ResponseCode)
	{
		pduTxData->SduLength = 1 + dspReadMemoryByAddress.MemorySize;
		if(OpStatus == DCM_READ_PENDING)
		{
			dspMemoryState = DCM_MEMORY_READ;
		}
		else
		{
			DsdDspProcessingDone(DCM_E_POSITIVERESPONSE);
		}
	}
	else
	{
		DsdDspProcessingDone(dspReadMemoryByAddress.ResponseCode);
	}
}

static boolean checkWriteMemoryByAddress(boolean useId,uint32 memoryAddress,
										uint8 memoryAddressFormat,
										uint32 memorySize,
										const Dcm_DspMemoryIdInfo *dspMemory)
{
	boolean ret = FALSE;
	uint8 MemoryId;
	
	if(useId == FALSE)
	{
		if((memoryAddress >= dspMemory->pWriteMemoryInfo->MemoryAddressLow)
			&& (memoryAddress <= dspMemory->pWriteMemoryInfo->MemoryAddressHigh)
			&& (memoryAddress + memorySize - 1 <= dspMemory->pWriteMemoryInfo->MemoryAddressHigh))
		{
			ret = TRUE;
		}
	}
	else
	{
		MemoryId = (uint8)(memoryAddress >> ((memoryAddressFormat - 1)*8));
		memoryAddress = memoryAddress & DCM_MEMORY_ADDRESS_MASK;

		if((MemoryId == dspMemory->MemoryIdValue) &&
			(memoryAddress >= dspMemory->pWriteMemoryInfo->MemoryAddressLow)
			&& (memoryAddress <= dspMemory->pWriteMemoryInfo->MemoryAddressHigh)
			&& (memoryAddress + memorySize -1 <= dspMemory->pWriteMemoryInfo->MemoryAddressHigh))
		{
			ret = TRUE;
		}

	}
	
	return ret;
}

static boolean lookupWriteMemory(uint32 memoryAddress,
								uint8 memoryAddressFormat,
								uint32 memorySize,
								const Dcm_DspMemoryIdInfo **MemoryIdConfigPtr)
{
	uint8 i;
	const Dcm_DspMemoryIdInfo *dspMemoryInfo = DCM_Config.Dsp->DspMemory->DspMemoryIdInfo;
	boolean memoryFound = FALSE;

	for(i = 0;(dspMemoryInfo->Arc_EOL == FALSE) && (memoryFound == FALSE);i++)
	{
		if(TRUE == checkWriteMemoryByAddress(DCM_Config.Dsp->DspMemory->DcmDspUseMemoryId, memoryAddress,memoryAddressFormat,memorySize,dspMemoryInfo))
		{
			memoryFound = TRUE;
		}
		else
		{
			dspMemoryInfo++;
		}
	}
	if (memoryFound == TRUE)
	{
		*MemoryIdConfigPtr = dspMemoryInfo;
	}
	
	return memoryFound;
}

static Dcm_NegativeResponseCodeType writeMemoryData(Dcm_OpStatusType* OpStatus,
												const Dcm_DspMemoryIdInfo *MemoryIdConfigPtr,
												uint32 MemoryAddress,
												uint32 MemorySize,
												uint8 *SourceData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	Dcm_ReturnWriteMemoryType writeRet;
	writeRet = Dcm_WriteMemory(*OpStatus,
								MemoryIdConfigPtr->MemoryIdValue,
								MemoryAddress,
								MemorySize,
								SourceData);
	if(DCM_READ_FAILED == writeRet)
	{
		responseCode = DCM_E_GENERALPROGRAMMINGFAILURE;   /*@req UDS_REQ_0X3D_16,DCM643*/
	}
	else if(DCM_WRITE_PENDING == writeRet)
	{
		*OpStatus = DCM_PENDING;
	}
	else
	{
		responseCode = DCM_E_POSITIVERESPONSE;
	}
	
	return responseCode;
}

void DspUdsWriteMemoryByAddress(const PduInfoType* pduRxData, PduInfoType* pduTxData)
{
	  typedef struct{
	  	uint32 MemoryAddressStart;
	  	uint32 MemoryIdValue;
	  	const Dcm_DspMemoryIdInfo *MemoryIdConfigPtr;
	  	Dcm_NegativeResponseCodeType ResponseCode;
	  	uint8 MemorySizeFormat;
	  	uint8 MemoryAddressFormat;
	  	uint32 MemorySize ;
	  	uint16 MessageLength;
	  }DspUdsWriteMemoryType;
	uint8 i;
	DspUdsWriteMemoryType dspUdsWriteMemory;
	Dcm_OpStatusType OpStatus = DCM_INITIAL;
	dspUdsWriteMemory.MemoryAddressStart = 0;
	dspUdsWriteMemory.MemoryIdConfigPtr = NULL;
	dspUdsWriteMemory.ResponseCode = DCM_E_POSITIVERESPONSE;
	dspUdsWriteMemory.MemorySizeFormat = ((uint8)(pduRxData->SduDataPtr[1] & DCM_FORMAT_HIGH_MASK)) >> 4;	/*@req UDS_REQ_0x3D_3 & UDS_REQ_0x3D_5*/
	dspUdsWriteMemory.MemoryAddressFormat = ((uint8)pduRxData->SduDataPtr[1]) & DCM_FORMAT_LOW_MASK;	        /*@req UDS_REQ_0x3D_3& UDS_REQ_0x3D_4*/
	dspUdsWriteMemory.MemorySize = 0;

	if((dspUdsWriteMemory.MemoryAddressFormat == 0) || (dspUdsWriteMemory.MemorySizeFormat == 0))  /*@req UDS_REQ_0x3D_14*/
	{
		dspUdsWriteMemory.ResponseCode = DCM_E_REQUESTOUTOFRANGE;
	}
	else
	{
		/*take value of MemorySize out */
		for(i = 0; i < dspUdsWriteMemory.MemorySizeFormat; i++)
		{
			dspUdsWriteMemory.MemorySize <<= 8;
			dspUdsWriteMemory.MemorySize += (uint32)(pduRxData->SduDataPtr[2 + i +dspUdsWriteMemory.MemoryAddressFormat]);
		}
		dspUdsWriteMemory.MessageLength = (uint16)(2 + dspUdsWriteMemory.MemoryAddressFormat + dspUdsWriteMemory.MemorySizeFormat + dspUdsWriteMemory.MemorySize);
		if(dspUdsWriteMemory.MessageLength == pduRxData->SduLength)
		{
			/*take Start Address out */
			for(i = 0; i < dspUdsWriteMemory.MemoryAddressFormat; i++)
			{
				dspUdsWriteMemory.MemoryAddressStart <<= 8;
				dspUdsWriteMemory.MemoryAddressStart += (uint32)(pduRxData->SduDataPtr[2 + i]);
			}
			if(TRUE == lookupWriteMemory(dspUdsWriteMemory.MemoryAddressStart,
										dspUdsWriteMemory.MemoryAddressFormat,
										dspUdsWriteMemory.MemorySize,
										&dspUdsWriteMemory.MemoryIdConfigPtr))
			{
				if (DspCheckSecurityLevel(dspUdsWriteMemory.MemoryIdConfigPtr->pReadMemoryInfo->pSecurityLevel) == TRUE)
				{
					dspUdsWriteMemory.ResponseCode = writeMemoryData(&OpStatus,
																	dspUdsWriteMemory.MemoryIdConfigPtr, 
																	dspUdsWriteMemory.MemoryAddressStart,
																	dspUdsWriteMemory.MemorySize,
																	&pduRxData->SduDataPtr[2 + dspUdsWriteMemory.MemoryAddressFormat + dspUdsWriteMemory.MemorySizeFormat]);
				}
				else
				{
					dspUdsWriteMemory.ResponseCode = DCM_E_SECUTITYACCESSDENIED;	/*@req UDS_REQ_0X3D_15,Dcm490*/
				}
			}
			else
			{
				dspUdsWriteMemory.ResponseCode = DCM_E_REQUESTOUTOFRANGE;		 /*@req UDS_REQ_0x3D_11,UDS_REQ_0x3D_12,Dcm489*/
			}
		}
		else
		{
			dspUdsWriteMemory.ResponseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;/*@req UDS_REQ_0x3D_9*/
		}	
	}
	if(DCM_E_POSITIVERESPONSE == dspUdsWriteMemory.ResponseCode)
	{
		pduTxData->SduLength = 2 + dspUdsWriteMemory.MemorySizeFormat + dspUdsWriteMemory.MemoryAddressFormat;
		pduTxData->SduDataPtr[1] = pduRxData->SduDataPtr[1];
		for(i = 0; i < dspUdsWriteMemory.MemorySizeFormat + dspUdsWriteMemory.MemoryAddressFormat; i++)
		{
			pduTxData->SduDataPtr[2 + i] = pduRxData->SduDataPtr[2 + i];
			if(OpStatus != DCM_PENDING)
			{
				DsdDspProcessingDone(dspUdsWriteMemory.ResponseCode);
			}
			else
			{
        		dspMemoryState=DCM_MEMORY_WRITE;
			}
		}
	}
	else
	{
		DsdDspProcessingDone(dspUdsWriteMemory.ResponseCode);
	}
}

static boolean checkPeriodicIdentifierBuffer(uint8 PeriodicDid,uint8 Length,uint8 *postion)
{
	uint8 i;
	boolean ret = FALSE;
	for(i = 0;(i < Length) && (ret == FALSE);i++)
	{
		if(PeriodicDid == dspPDidRef.dspPDid[i].PeriodicDid)
		{
			ret = TRUE;
			*postion = i;
		}
	}
	
	return ret;
}

static void ClearPeriodicIdentifierBuffer(uint8 BufferEnd,uint8 postion)
{
	dspPDidRef.dspPDid[postion].PeriodicDid = dspPDidRef.dspPDid[BufferEnd ].PeriodicDid;
	dspPDidRef.dspPDid[postion].PDidTxCounter = dspPDidRef.dspPDid[BufferEnd].PDidTxCounter;
	dspPDidRef.dspPDid[postion].PDidTxCounterNumber = dspPDidRef.dspPDid[BufferEnd].PDidTxCounterNumber;
	dspPDidRef.dspPDid[BufferEnd].PeriodicDid = 0;
	dspPDidRef.dspPDid[BufferEnd].PDidTxCounter = 0;
	dspPDidRef.dspPDid[BufferEnd ].PDidTxCounterNumber = 0;
}

static Dcm_NegativeResponseCodeType readPeriodDidData(const Dcm_DspDidType *PDidPtr, uint8 *Data,uint16 *Length)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	if ((PDidPtr->DspDidInfoRef->DspDidAccess.DspDidRead != NULL) 
		&& (PDidPtr->DspDidConditionCheckReadFnc != NULL) 
		&& (PDidPtr->DspDidReadDataFnc != NULL) ) 
	{	
		if (DspCheckSessionLevel(PDidPtr->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSessionRef)) 
		{ 
			if (DspCheckSecurityLevel(PDidPtr->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSecurityLevelRef)) 
			{
				Std_ReturnType result = E_NOT_OK;
				Dcm_NegativeResponseCodeType errorCode = DCM_E_POSITIVERESPONSE;
				result = PDidPtr->DspDidConditionCheckReadFnc(&errorCode);
				if ((result == E_OK) && (errorCode == DCM_E_POSITIVERESPONSE))
				{
					result = E_NOT_OK;
					if (PDidPtr->DspDidInfoRef->DspDidFixedLength)
					{	
						*Length= PDidPtr->DspDidSize;
						result = E_OK;
					}
					else
					{
						if(PDidPtr->DspDidReadDataLengthFnc!=NULL)
						{
							result = PDidPtr->DspDidReadDataLengthFnc(Length);
						}
						else
						{
							responseCode = DCM_E_GENERALREJECT;
						}
					}
					if (result == E_OK) 
					{
						result = PDidPtr->DspDidReadDataFnc(Data);
						if (result != E_OK)
						{
							responseCode = DCM_E_REQUESTOUTOFRANGE;
						}
					}
					else
					{
						responseCode = DCM_E_REQUESTOUTOFRANGE;
					}
				}
				else
				{
					responseCode = DCM_E_REQUESTOUTOFRANGE;
				}
			}
			else
			{
				responseCode = DCM_E_SECUTITYACCESSDENIED;
			}
		}
		else
		{
			responseCode = DCM_E_REQUESTOUTOFRANGE;
		}
	}
	else
	{
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}
	return responseCode;
}

static Dcm_NegativeResponseCodeType DspSavePeriodicData(uint16 didNr, uint32 periodicTransmitCounter,uint8 PdidBufferNr)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	const Dcm_DspDidType *SourceDid = NULL;
	Dcm_DspDDDType *DDidPtr = NULL;

	if (TRUE == lookupDid(didNr, &SourceDid))
	{
		if(DspCheckSessionLevel(SourceDid->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSessionRef) == TRUE)
		{
			if(DspCheckSecurityLevel(SourceDid->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSecurityLevelRef) == TRUE)
			{
				Std_ReturnType result = E_NOT_OK;
				Dcm_NegativeResponseCodeType errorCode = DCM_E_POSITIVERESPONSE;
				
				if(SourceDid->DspDidConditionCheckReadFnc != NULL)
				{
					result = SourceDid->DspDidConditionCheckReadFnc(&errorCode);
				}
				if ((result != E_OK) || (errorCode != DCM_E_POSITIVERESPONSE))
				{
					responseCode = DCM_E_REQUESTOUTOFRANGE;
				}
				if((SourceDid->DspDidInfoRef->DspDidFixedLength!=TRUE) && (SourceDid->DspDidReadDataLengthFnc == NULL))
				{
					responseCode = DCM_E_GENERALREJECT;
				}
			}
			else
			{
				responseCode = DCM_E_SECUTITYACCESSDENIED;
			}
		}
		else
		{
			responseCode = DCM_E_REQUESTOUTOFRANGE;
		}
	}
	else if(LookupDDD(didNr, (const Dcm_DspDDDType **)&DDidPtr) == TRUE)
	{
		responseCode = DCM_E_POSITIVERESPONSE;
	}
	else
	{
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}
	if(responseCode == DCM_E_POSITIVERESPONSE)
	{
		dspPDidRef.dspPDid[PdidBufferNr].PeriodicDid = (uint8)didNr & DCM_DID_LOW_MASK;
		dspPDidRef.dspPDid[PdidBufferNr].PDidTxCounter = 0;
		dspPDidRef.dspPDid[PdidBufferNr].PDidTxCounterNumber = periodicTransmitCounter;
	}
	return responseCode;
}
static void ClearPeriodicIdentifier(const PduInfoType *pduRxData,PduInfoType *pduTxData )
{
	uint16 PdidNumber;
	uint8 PDidLowByte;
	uint8 PdidPostion;
	uint8 i;
	if( pduRxData->SduDataPtr[1] == DCM_PERIODICTRANSMIT_STOPSENDING_MODE )
	{
		PdidNumber = pduRxData->SduLength - 2;
		for(i = 0;i < PdidNumber;i++)
		{
			PDidLowByte = pduRxData->SduDataPtr[2];
			if(checkPeriodicIdentifierBuffer(PDidLowByte,dspPDidRef.PDidNr,&PdidPostion) == TRUE)
			{
				dspPDidRef.PDidNr--;
				ClearPeriodicIdentifierBuffer(dspPDidRef.PDidNr, PdidPostion);
				
			}		
		}
		pduTxData->SduLength = 1;
	}
}
/*
	DESCRIPTION:
		 UDS Service 0x2a - Read Data By Periodic Identifier
*/
void DspReadDataByPeriodicIdentifier(const PduInfoType *pduRxData,PduInfoType *pduTxData)
{
	/** @req DCM254 */
	uint8 PDidLowByte;
	uint16 PdidNumber;
	uint8 PdidPostion;
	uint16 i;
	uint8 PdidBufferNr;
	uint32 periodicTransmitCounter;
	uint16 DataLength;
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	const Dcm_DspDidType *PDidPtr = NULL;
	Dcm_DspDDDType *DDidPtr = NULL;
	PdidBufferNr = dspPDidRef.PDidNr;
	if(pduRxData->SduLength > 2)
	{
		
		switch(pduRxData->SduDataPtr[1])
		{
			case DCM_PERIODICTRANSMIT_DEFAULT_MODE:
				periodicTransmitCounter = 0;
				break;
			case DCM_PERIODICTRANSMIT_SLOWRATE_MODE:
				periodicTransmitCounter = DCM_PERIODICTRANSMIT_SLOW;
				break;
				case DCM_PERIODICTRANSMIT_MEDIUM_MODE:
					periodicTransmitCounter = DCM_PERIODICTRANSMIT_MEDIUM;
				break;
			case DCM_PERIODICTRANSMIT_FAST_MODE:
				periodicTransmitCounter = DCM_PERIODICTRANSMIT_FAST;
				break;
			case DCM_PERIODICTRANSMIT_STOPSENDING_MODE:
				ClearPeriodicIdentifier(pduRxData,pduTxData);
				break;
			default:
				responseCode = DCM_E_REQUESTOUTOFRANGE;
				break;
		}
		if((pduRxData->SduDataPtr[1] != DCM_PERIODICTRANSMIT_STOPSENDING_MODE) && responseCode == DCM_E_POSITIVERESPONSE)
		{
			PdidNumber = pduRxData->SduLength - 2;
			if(1 == PdidNumber)
			{
				PDidLowByte = pduRxData->SduDataPtr[2];			
				if(checkPeriodicIdentifierBuffer(PDidLowByte,dspPDidRef.PDidNr,&PdidPostion) == TRUE)
				{
					if(0 == periodicTransmitCounter)
					{
						if (TRUE == lookupDid(((uint16)PDidLowByte + 0xF200), &PDidPtr))   /*UDS_REQ_0x2A_1*/
						{
							pduTxData->SduDataPtr[1] = PDidLowByte;	
							responseCode = readPeriodDidData(PDidPtr,&pduTxData->SduDataPtr[2],&DataLength);
							pduTxData->SduLength = DataLength + 2;
						}
						else if(TRUE == LookupDDD((0xF200 + (uint16)PDidLowByte), (const Dcm_DspDDDType **)&DDidPtr))
						{
							pduTxData->SduDataPtr[1] = PDidLowByte;
							responseCode = readDDDData(DDidPtr,&pduTxData->SduDataPtr[2],&DataLength);
							pduTxData->SduLength = DataLength + 2;
						}
						else
						{
							responseCode = DCM_E_REQUESTOUTOFRANGE;
						}
						if(responseCode != DCM_E_POSITIVERESPONSE)
						{
							dspPDidRef.PDidNr--;
							ClearPeriodicIdentifierBuffer(dspPDidRef.PDidNr,PdidPostion);
						}
					}
					else
					{
						dspPDidRef.dspPDid[PdidPostion].PDidTxCounterNumber = periodicTransmitCounter;
	  					pduTxData->SduLength = 1;
					}
				}
				else
				{	
					responseCode = DspSavePeriodicData((DCM_PERODICDID_HIHG_MASK + (uint16)PDidLowByte),periodicTransmitCounter,PdidBufferNr);
					PdidBufferNr++;
					pduTxData->SduLength = 1;
				}
			}
			else if(((PdidNumber + PdidBufferNr) <= DCM_LIMITNUMBER_PERIODDATA) && (responseCode == DCM_E_POSITIVERESPONSE))	/*UDS_REQ_0x2A_6*/
			{	
				for(i = 0;(i < PdidNumber)&&(responseCode == DCM_E_POSITIVERESPONSE);i++)
				{
					PDidLowByte = pduRxData->SduDataPtr[2 + i];
					if(checkPeriodicIdentifierBuffer(PDidLowByte,PdidBufferNr,&PdidPostion) == TRUE)
					{
						if(dspPDidRef.dspPDid[PdidPostion].PDidTxCounterNumber != periodicTransmitCounter)
						{
							dspPDidRef.dspPDid[PdidPostion].PDidTxCounterNumber = periodicTransmitCounter;
						}
					}
					else
					{
						responseCode = DspSavePeriodicData((0xF200 + (uint16)PDidLowByte),periodicTransmitCounter,PdidBufferNr);
						PdidBufferNr++;
					}
						pduTxData->SduLength = 1;
				}
			}
			else
			{
				responseCode = DCM_E_REQUESTOUTOFRANGE;
			}
			if(responseCode == DCM_E_POSITIVERESPONSE)
			{					
				dspPDidRef.PDidNr = PdidBufferNr;				
			}
		}							
	}
	else if((pduRxData->SduLength == 2)&&(pduRxData->SduDataPtr[1] == DCM_PERIODICTRANSMIT_STOPSENDING_MODE))
	{
		memset(&dspPDidRef,0,sizeof(dspPDidRef));
		pduTxData->SduLength = 1;
	}
	else
	{
		responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}
	DsdDspProcessingDone(responseCode);
}

static Dcm_NegativeResponseCodeType dynamicallyDefineDataIdentifierbyDid(uint16 DDIdentifier,const PduInfoType *pduRxData,PduInfoType *pduTxData)
{
	uint8 i;
	uint16 SourceDidNr;
	const Dcm_DspDidType *SourceDid = NULL;
	Dcm_DspDDDType *DDid = NULL;
	uint16 SourceLength = 0;
	uint16 DidLength = 0;
	uint16 Length = 0;
	uint8 Num = 0;
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;

	if(FALSE == LookupDDD(DDIdentifier, (const Dcm_DspDDDType **)&DDid))
	{
		while((Num < DCM_MAX_DDD_NUMBER) && (dspDDD[Num].DynamicallyDid != 0 ))
		{
			Num++;
		}
		if(Num >= DCM_MAX_DDD_NUMBER)
		{
			responseCode = DCM_E_REQUESTOUTOFRANGE;
		}
		else
		{
			DDid = &dspDDD[Num];
		}
	}
	else
	{
		while((SourceLength < DCM_MAX_DDDSOURCE_NUMBER) && (DDid->DDDSource[SourceLength].formatOrPosition != 0 ))
		{
			SourceLength++;
		}
		if(SourceLength > DCM_MAX_DDDSOURCE_NUMBER)
		{
			responseCode = DCM_E_REQUESTOUTOFRANGE;
		}
	}
	if(responseCode == DCM_E_POSITIVERESPONSE)
	{
		Length = (pduRxData->SduLength - SID_AND_ALFID_LEN4) /SID_AND_ALFID_LEN4;
		if(((Length*SID_AND_ALFID_LEN4) == (pduRxData->SduLength - SID_AND_ALFID_LEN4)) && (Length != 0))
		{
			if((Length + SourceLength) <= DCM_MAX_DDDSOURCE_NUMBER)
			{
				for(i = 0;(i < Length) && (responseCode == DCM_E_POSITIVERESPONSE);i++)
				{
					SourceDidNr = (((uint16)pduRxData->SduDataPtr[SID_AND_ALFID_LEN4 + i*SID_AND_ALFID_LEN4] << 8) & DCM_DID_HIGH_MASK) + (((uint16)pduRxData->SduDataPtr[(5 + i*SID_AND_ALFID_LEN4)]) & DCM_DID_LOW_MASK);
					if(TRUE == lookupDid(SourceDidNr, &SourceDid))/*UDS_REQ_0x2C_4*/
					{	
						if(DspCheckSessionLevel(SourceDid->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSessionRef))
						{
							if(DspCheckSecurityLevel(SourceDid->DspDidInfoRef->DspDidAccess.DspDidRead->DspDidReadSecurityLevelRef))
							{
								if(SourceDid->DspDidInfoRef->DspDidFixedLength == TRUE)
								{
									DidLength = SourceDid->DspDidSize;
								}
								else
								{
									if(	SourceDid->DspDidReadDataLengthFnc != NULL)
									{
										SourceDid->DspDidReadDataLengthFnc(&DidLength);
									}
								}
								if(DidLength != 0)
								{
									if((pduRxData->SduDataPtr[SID_AND_ALFID_LEN6 + i*SID_AND_ALFID_LEN4] != 0) &&
										(pduRxData->SduDataPtr[SID_AND_ALFID_LEN7 + i*SID_AND_ALFID_LEN4] != 0) &&
										(((uint16)pduRxData->SduDataPtr[SID_AND_ALFID_LEN6 + i*SID_AND_ALFID_LEN4] + (uint16)pduRxData->SduDataPtr[SID_AND_ALFID_LEN7 + i*SID_AND_ALFID_LEN4] - 1) <= DidLength))
									{
										DDid->DDDSource[i + SourceLength].formatOrPosition = pduRxData->SduDataPtr[SID_AND_ALFID_LEN6 + i*SID_AND_ALFID_LEN4];
										DDid->DDDSource[i + SourceLength].Size = pduRxData->SduDataPtr[SID_AND_ALFID_LEN7 + i*SID_AND_ALFID_LEN4];
										DDid->DDDSource[i + SourceLength].SourceAddressOrDid = SourceDid->DspDidIdentifier;
										DDid->DDDSource[i + SourceLength].DDDTpyeID = DCM_DDD_SOURCE_DID;
									}
									else
									{
										/*UDS_REQ_0x2C_6*/
										responseCode = DCM_E_REQUESTOUTOFRANGE;
									}
									
								}
								else
								{
									/*UDS_REQ_0x2C_14*/
									responseCode = DCM_E_REQUESTOUTOFRANGE;
								}
							}
							else
							{
								responseCode = DCM_E_SECUTITYACCESSDENIED;
							}
						}
						else
						{
							/*UDS_REQ_0x2C_19,DCM726*/
							responseCode = DCM_E_REQUESTOUTOFRANGE;
						}
					}
					else
					{
						/*DCM725*/
						responseCode = DCM_E_REQUESTOUTOFRANGE;
					}
				}
			}
			else
			{
				/*UDS_REQ_0x2C_13*/
				responseCode = DCM_E_REQUESTOUTOFRANGE;
			}
		}
		else
		{
			/*UDS_REQ_0x2C_11*/
			responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
		}
		if(responseCode == DCM_E_POSITIVERESPONSE)
		{
			DDid->DynamicallyDid = DDIdentifier;
			pduTxData->SduDataPtr[1] = DCM_DDD_SUBFUNCTION_DEFINEBYDID;
		}
	}
	if(responseCode == DCM_E_POSITIVERESPONSE)
	{
		pduTxData->SduDataPtr[1] = DCM_DDD_SUBFUNCTION_DEFINEBYDID;
	}
	
	return responseCode;
}

static Dcm_NegativeResponseCodeType dynamicallyDefineDataIdentifierbyAddress(uint16 DDIdentifier,const PduInfoType *pduRxData,PduInfoType *pduTxData)
{
	uint8 LengthCount;
	uint8 SourceCount;
	uint16 Length;
	uint8 AddressFormat;
	uint8 MemorySizeFormat;
	uint32 MemoryAddress = 0;
	uint16 MemorySize = 0;
	uint16 SourceLength = 0;
	Dcm_DspDDDType *DDid = NULL;
	const Dcm_DspMemoryIdInfo *SourceMemoryInfo = NULL;
	uint8 Num = 0;
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	
	if(FALSE == LookupDDD(DDIdentifier, (const Dcm_DspDDDType **)&DDid))
	{
		while((Num < DCM_MAX_DDD_NUMBER) && (dspDDD[Num].DynamicallyDid != 0 ))
		{
			Num++;
		}
		if(Num >= DCM_MAX_DDD_NUMBER)
		{
			responseCode = DCM_E_REQUESTOUTOFRANGE;
		}
		else
		{
			DDid = &dspDDD[Num];
		}
	}
	else
	{
		while((SourceLength < DCM_MAX_DDDSOURCE_NUMBER) && (DDid->DDDSource[SourceLength].formatOrPosition != 0 ))
		{
			SourceLength++;
		}
		if(SourceLength >= DCM_MAX_DDDSOURCE_NUMBER)
		{
			responseCode = DCM_E_REQUESTOUTOFRANGE;
		}
	}
	if(responseCode == DCM_E_POSITIVERESPONSE)
	{
		AddressFormat = (uint8)pduRxData->SduDataPtr[4] & DCM_FORMAT_LOW_MASK;
		MemorySizeFormat = (uint8)(pduRxData->SduDataPtr[4] >> 4) & DCM_FORMAT_LOW_MASK;
		if((AddressFormat+MemorySizeFormat) != 0)
		{
			Length = (pduRxData->SduLength - SID_AND_ALFID_LEN5) / (AddressFormat + MemorySizeFormat);
		}
		if((AddressFormat != 0) && (MemorySizeFormat != 0) && ((SourceLength+Length) <= DCM_MAX_DDDSOURCE_NUMBER))
		{
			if((Length != 0)&&( Length * (AddressFormat + MemorySizeFormat) == (pduRxData->SduLength - 5) ))
			{
				for(LengthCount = 0; (LengthCount < Length) && (responseCode == DCM_E_POSITIVERESPONSE); LengthCount++)
				{
					MemoryAddress = 0;
					for(SourceCount = 0; SourceCount < AddressFormat; SourceCount++)
					{
						MemoryAddress = MemoryAddress << 8;
						MemoryAddress += (uint32)(pduRxData->SduDataPtr[5 + SourceCount + LengthCount * (AddressFormat + MemorySizeFormat)]);
					}

					/*take value of MemorySize out */
					MemorySize = 0;
					for(SourceCount = 0; SourceCount < MemorySizeFormat; SourceCount++)
					{
						MemorySize = MemorySize << 8;
						MemorySize += (uint32)(pduRxData->SduDataPtr[5 + SourceCount + AddressFormat + LengthCount * (AddressFormat + MemorySizeFormat)]);
					}
					if(TRUE == lookupReadMemory(MemoryAddress, AddressFormat,MemorySize,&SourceMemoryInfo))
					{
						if(DspCheckSecurityLevel(SourceMemoryInfo->pReadMemoryInfo->pSecurityLevel) == TRUE)
						{
							DDid->DDDSource[LengthCount + SourceLength].formatOrPosition = pduRxData->SduDataPtr[4];
							DDid->DDDSource[LengthCount + SourceLength].SourceAddressOrDid = MemoryAddress;
							DDid->DDDSource[LengthCount + SourceLength].Size = MemorySize;
							DDid->DDDSource[LengthCount + SourceLength].DDDTpyeID = DCM_DDD_SOURCE_ADDRESS;
							/*UDS_REQ_0x2C_6*/
						}
						else
						{
							/*UDS_REQ_0x2C_19,DCM726*/
							responseCode = DCM_E_SECUTITYACCESSDENIED;
						}
					}
					else
					{
						/*UDS_REQ_0x2C_15,UDS_REQ_0x2C_16*/
						responseCode = DCM_E_REQUESTOUTOFRANGE;
					}
				}
				if(responseCode == DCM_E_POSITIVERESPONSE)
				{
					DDid->DynamicallyDid = DDIdentifier;
				}
			}
			else
			{
				/*UDS_REQ_0x2C_11*/
				responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
			}
		}
		else
		{
			/*UDS_REQ_0x2C_17, UDS_REQ_0x2C_18*/
			responseCode = DCM_E_REQUESTOUTOFRANGE;
		}

	}
	if(responseCode == DCM_E_POSITIVERESPONSE)
	{
		pduTxData->SduDataPtr[1] = DCM_DDD_SUBFUNCTION_DEFINEBYADDRESS;
	}
	
	return responseCode;
}


/*
	DESCRIPTION:
		 UDS Service 0x2c - Clear dynamically Did
*/
static Dcm_NegativeResponseCodeType CleardynamicallyDid(uint16 DDIdentifier,const PduInfoType *pduRxData, PduInfoType * pduTxData)
{
	/*UDS_REQ_0x2C_5*/
	sint8 i, j;
	uint8 ClearCount;
	uint8 position;
	uint8 ClearNum = 0;
	Dcm_DspDDDType *DDid = NULL;
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	
	if(pduRxData->SduLength == 4)
	{
		if(TRUE == LookupDDD(DDIdentifier, (const Dcm_DspDDDType **)&DDid))
		{
			
			if((checkPeriodicIdentifierBuffer(pduRxData->SduDataPtr[3], dspPDidRef.PDidNr, &position) == TRUE)&&(pduRxData->SduDataPtr[2] == 0xF2))
			{
				/*UDS_REQ_0x2C_9*/
				responseCode = DCM_E_REQUESTOUTOFRANGE;
			}
			else
			{
				memset(DDid, 0, sizeof(Dcm_DspDDDType));
				for(i = DCM_MAX_DDD_NUMBER - 1;i >= 0 ;i--) {	/* find the first DDDid from bottom */
					if (0 != dspDDD[i].DynamicallyDid) {
						for (j = 0; j <DCM_MAX_DDD_NUMBER; j++) { /* find the first empty slot from top */
							if (j >= i) {
								/* Rearrange finished */
								pduTxData->SduDataPtr[1] = DCM_DDD_SUBFUNCTION_CLEAR;
								pduTxData->SduLength = 2;
								return responseCode;
							}
							else if (0 == dspDDD[j].DynamicallyDid) {	/* find, exchange */
								memcpy(&dspDDD[j], &dspDDD[i], sizeof(Dcm_DspDDDType));
								memset(&dspDDD[i], 0, sizeof(Dcm_DspDDDType));
							}
						}
					}
				}
			}
		}
		else{
			responseCode = DCM_E_REQUESTOUTOFRANGE;	/* DDDid not found */
		}
	}
	else if (pduRxData->SduDataPtr[1] == 0x03 && pduRxData->SduLength == 2){
		/* clear all */
		memset(dspDDD, 0, (sizeof(Dcm_DspDDDType) * DCM_MAX_DDD_NUMBER));
	}
	else
	{
		responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}
	if(responseCode == DCM_E_POSITIVERESPONSE)
	{
		pduTxData->SduDataPtr[1] = DCM_DDD_SUBFUNCTION_CLEAR;
		pduTxData->SduLength = 2;
	}
	
	return responseCode;
}

void DspDynamicallyDefineDataIdentifier(const PduInfoType *pduRxData,PduInfoType *pduTxData)
{
	/*UDS_REQ_0x2C_1,DCM 259*/
	uint16 i;
	uint8 Position;
	boolean PeriodicdUse = FALSE;
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	uint16 DDIdentifier = ((((uint16)pduRxData->SduDataPtr[2]) << 8) & DCM_DID_HIGH_MASK) + (pduRxData->SduDataPtr[3] & DCM_DID_LOW_MASK);
	if(pduRxData->SduLength > 2)
	{
		/* Check if DDID equals 0xF2 or 0xF3 */
		if((pduRxData->SduDataPtr[2] & 0xF2) == 0xF2)
		{
			switch(pduRxData->SduDataPtr[1])	/*UDS_REQ_0x2C_2,DCM 646*/
			{
				case DCM_DDD_SUBFUNCTION_DEFINEBYDID:
					responseCode  = dynamicallyDefineDataIdentifierbyDid(DDIdentifier,pduRxData,pduTxData);
					break;
				case DCM_DDD_SUBFUNCTION_DEFINEBYADDRESS:
					responseCode = dynamicallyDefineDataIdentifierbyAddress(DDIdentifier,pduRxData,pduTxData);
					break;
				case DCM_DDD_SUBFUNCTION_CLEAR:
					responseCode = CleardynamicallyDid(DDIdentifier,pduRxData,pduTxData);
					break;
				default:
					responseCode = DCM_E_SUBFUNCTIONNOTSUPPORTED;
					/*UDS_REQ_0x2C_10*/
					break;		
			}
		}
		else
		{
			responseCode = DCM_E_REQUESTOUTOFRANGE;
		}
		if(responseCode == DCM_E_POSITIVERESPONSE)
		{
			pduTxData->SduDataPtr[2] = pduRxData->SduDataPtr[2];
			pduTxData->SduDataPtr[3] = pduRxData->SduDataPtr[3];
			pduTxData->SduLength = 4;
		}
	}
	else if((pduRxData->SduLength == 2)&&(pduRxData->SduDataPtr[1] == DCM_DDD_SUBFUNCTION_CLEAR))
	{
		/*UDS_REQ_0x2C_7*/
		for(i = 0;i < DCM_MAX_DDD_NUMBER;i++)
		{
			if(checkPeriodicIdentifierBuffer((uint8)(dspDDD[i].DynamicallyDid & DCM_DID_LOW_MASK),dspPDidRef.PDidNr,&Position) == TRUE)
			{
				PeriodicdUse = TRUE;
			}
		}
		if(PeriodicdUse == FALSE)
		{
			memset(dspDDD,0,sizeof(dspDDD));
			pduTxData->SduDataPtr[1] = DCM_DDD_SUBFUNCTION_CLEAR;
			pduTxData->SduLength = 2;
		}
		else
		{
			responseCode = DCM_E_REQUESTOUTOFRANGE;
		}
	}
	else
	{
		/*UDS_REQ_0x2C_11*/
		responseCode =  DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}
	DsdDspProcessingDone(responseCode);
}

static Dcm_NegativeResponseCodeType DspIOControlReturnControlToECU(const Dcm_DspDidType *DidPtr,const PduInfoType *pduRxData,PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	if(DidPtr->DspDidInfoRef->DspDidAccess.DspDidControl->DspDidReturnControlToEcu == TRUE)
	{
		if(pduRxData->SduLength > 4)
		{
			if(DidPtr->DspDidInfoRef->DspDidAccess.DspDidControl != NULL)
			{
				if(((DidPtr->DspDidControlRecordSize->DspDidControlRecordSize + 7) >> 3) == (pduRxData->SduLength - 4))
				{
					if(DidPtr->DspDidReturnControlToEcuFnc != NULL)
					{
						DidPtr->DspDidReturnControlToEcuFnc(NULL,&pduRxData->SduDataPtr[4],&pduTxData->SduDataPtr[4],&responseCode);
						
					}
					else
					{
						responseCode = DCM_E_REQUESTOUTOFRANGE;
					}
				}
				else
				{
					responseCode = DCM_E_REQUESTOUTOFRANGE;
				}
			}
			else
			{
				responseCode = DCM_E_REQUESTOUTOFRANGE;
			}
		}
		else
		{
			if(DidPtr->DspDidReturnControlToEcuFnc != NULL)
			{

				if(DidPtr->DspDidControlRecordSize != NULL)
				{
					DidPtr->DspDidReturnControlToEcuFnc(NULL,NULL,&pduTxData->SduDataPtr[4],&responseCode);
					pduTxData->SduLength = DidPtr->DspDidControlRecordSize->DspDidControlStatusRecordSize + 4;
				}
				else
				{
					responseCode = DCM_E_REQUESTOUTOFRANGE;
				}
			}
			else
			{
				responseCode = DCM_E_REQUESTOUTOFRANGE;
			}
		}
	}
	else
	{
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}
	if(responseCode == DCM_E_POSITIVERESPONSE)
	{
		pduTxData->SduLength = DidPtr->DspDidControlRecordSize->DspDidControlStatusRecordSize + 4;
		pduTxData->SduDataPtr[3] = DCM_RETURN_CONTROL_TO_ECU;
	}
	
	return responseCode;
}

static Dcm_NegativeResponseCodeType DspIOControlResetToDefault(const Dcm_DspDidType *DidPtr,const PduInfoType *pduRxData,PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	if(DidPtr->DspDidInfoRef->DspDidAccess.DspDidControl->DspDidResetToDefault == TRUE)
	{
		if(pduRxData->SduLength > 4)
		{
			if(DidPtr->DspDidInfoRef->DspDidAccess.DspDidControl != NULL)
			{
				if(((DidPtr->DspDidControlRecordSize->DspDidControlRecordSize + 7) >> 3) == (pduRxData->SduLength - 4))
				{
					if(DidPtr->DspDidReturnControlToEcuFnc != NULL)
					{
						DidPtr->DspDidResetToDeaultFnc(NULL,&pduRxData->SduDataPtr[4],&pduTxData->SduDataPtr[4],&responseCode);
					}
					else
					{
						responseCode = DCM_E_REQUESTOUTOFRANGE;
					}
				}
				else
				{
					responseCode = DCM_E_REQUESTOUTOFRANGE;
				}
			}
			else
			{
				responseCode = DCM_E_REQUESTOUTOFRANGE;
			}
		}
		else
		{
			if(DidPtr->DspDidResetToDeaultFnc != NULL)
			{

				if(DidPtr->DspDidControlRecordSize != NULL)
				{
					DidPtr->DspDidResetToDeaultFnc(NULL,NULL,&pduTxData->SduDataPtr[4],&responseCode);
					pduTxData->SduLength = DidPtr->DspDidControlRecordSize->DspDidControlStatusRecordSize + 4;
				}
				else
				{
					responseCode = DCM_E_REQUESTOUTOFRANGE;
				}
			}
			else
			{
				responseCode = DCM_E_REQUESTOUTOFRANGE;
			}
		}
	}
	else
	{
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}
	if(responseCode == DCM_E_POSITIVERESPONSE)
	{
		pduTxData->SduLength = DidPtr->DspDidControlRecordSize->DspDidControlStatusRecordSize+4;
		pduTxData->SduDataPtr[3] = DCM_RESET_TO_DEFAULT;
	}
	return responseCode;
}
/*
	DESCRIPTION:
		 UDS Service 0x2F -  IOControl Freeze Current State
*/
static Dcm_NegativeResponseCodeType DspIOControlFreezeCurrentState(const Dcm_DspDidType *DidPtr,const PduInfoType *pduRxData,PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	if(DidPtr->DspDidInfoRef->DspDidAccess.DspDidControl->DspDidFreezeCurrentState == TRUE)
	{
		if(pduRxData->SduLength > 4)
		{
			if(DidPtr->DspDidInfoRef->DspDidAccess.DspDidControl != NULL)
			{
				if(((DidPtr->DspDidControlRecordSize->DspDidControlRecordSize + 7) >> 3) == (pduRxData->SduLength - 4))
				{
					if(DidPtr->DspDidFreezeCurrentStateFnc != NULL)
					{
						DidPtr->DspDidFreezeCurrentStateFnc(NULL,&pduRxData->SduDataPtr[4],&pduTxData->SduDataPtr[4],&responseCode);
					}
					else
					{
						responseCode = DCM_E_REQUESTOUTOFRANGE;
					}
				}
				else
				{
					responseCode = DCM_E_REQUESTOUTOFRANGE;
				}
			}
			else
			{
				responseCode = DCM_E_REQUESTOUTOFRANGE;
			}
		}
		else
		{
			if(DidPtr->DspDidFreezeCurrentStateFnc != NULL)
			{

				if(DidPtr->DspDidControlRecordSize != 0)
				{
					DidPtr->DspDidFreezeCurrentStateFnc(NULL,NULL,&pduTxData->SduDataPtr[4],&responseCode);
					pduTxData->SduLength = DidPtr->DspDidControlRecordSize->DspDidControlStatusRecordSize + 4;
				}
				else
				{
					responseCode = DCM_E_REQUESTOUTOFRANGE;
				}
			}
			else
			{
				responseCode = DCM_E_REQUESTOUTOFRANGE;
			}
		}
	}
	else
	{
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}
	if(responseCode == DCM_E_POSITIVERESPONSE)
	{
		pduTxData->SduLength = DidPtr->DspDidControlRecordSize->DspDidControlStatusRecordSize + 4;
		pduTxData->SduDataPtr[3] = DCM_FREEZE_CURRENT_STATE;
	}
	
	return responseCode;
}

static Dcm_NegativeResponseCodeType DspIOControlShortTeamAdjustment(const Dcm_DspDidType *DidPtr,const PduInfoType *pduRxData,PduInfoType *pduTxData)
{
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	uint8 didControlOptionRecordSize = DidPtr->DspDidControlRecordSize->DspDidControlOptionRecordSize;
	if(DidPtr->DspDidInfoRef->DspDidAccess.DspDidControl->DspDidShortTermAdjustment == TRUE)
	{
		if(pduRxData->SduLength > 4)
		{
			if(DidPtr->DspDidInfoRef->DspDidAccess.DspDidControl != NULL)
			{
				if(((((DidPtr->DspDidControlRecordSize->DspDidControlRecordSize + 7)) >> 3) + (didControlOptionRecordSize)) == (pduRxData->SduLength - 4))
				{
					if(DidPtr->DspDidShortTermAdjustmentFnc != NULL)
					{
						DidPtr->DspDidShortTermAdjustmentFnc(&pduRxData->SduDataPtr[4],&pduRxData->SduDataPtr[4 + didControlOptionRecordSize],&pduTxData->SduDataPtr[4],&responseCode);
					}
					else
					{
						responseCode = DCM_E_REQUESTOUTOFRANGE;
					}
				}
				else if((didControlOptionRecordSize) == (pduRxData->SduLength - 4))
				{
					if(DidPtr->DspDidShortTermAdjustmentFnc != NULL)
					{
						DidPtr->DspDidShortTermAdjustmentFnc(&pduRxData->SduDataPtr[4],NULL,&pduTxData->SduDataPtr[4],&responseCode);
					}
					else
					{
						responseCode = DCM_E_REQUESTOUTOFRANGE;
					}
				}
				else
				{
					responseCode = DCM_E_REQUESTOUTOFRANGE;
				}
			}
			else 
			{
				responseCode = DCM_E_REQUESTOUTOFRANGE;
			}
		}
		else
		{
			responseCode = DCM_E_REQUESTOUTOFRANGE;
		}
	}
	else
	{
		responseCode = DCM_E_REQUESTOUTOFRANGE;
	}
	if(responseCode == DCM_E_POSITIVERESPONSE)
	{
		pduTxData->SduLength = DidPtr->DspDidControlRecordSize->DspDidControlStatusRecordSize + 4;
		pduTxData->SduDataPtr[3] = DCM_SHORT_TERM_ADJUSTMENT;
	}
	
	return responseCode;
}

void DspIOControlByDataIdentifier(const PduInfoType *pduRxData,PduInfoType *pduTxData)
{
	uint16 didNr;
	const Dcm_DspDidType *DidPtr = NULL;
	Dcm_NegativeResponseCodeType responseCode = DCM_E_POSITIVERESPONSE;
	didNr = (pduRxData->SduDataPtr[1] << 8 & DCM_DID_HIGH_MASK) + (pduRxData->SduDataPtr[2] & DCM_DID_LOW_MASK);
	if(pduRxData->SduLength > 3)
	{
		if(TRUE == lookupDid(didNr, &DidPtr))
		{
			if(FALSE == DidPtr->DspDidUsePort)
			{
				if(NULL != DidPtr->DspDidInfoRef->DspDidAccess.DspDidControl)
				{
					if(TRUE == DspCheckSessionLevel(DidPtr->DspDidInfoRef->DspDidAccess.DspDidControl->DspDidControlSessionRef))
					{
						if(TRUE == DspCheckSecurityLevel(DidPtr->DspDidInfoRef->DspDidAccess.DspDidControl->DspDidControlSecurityLevelRef))
						{
							switch(pduRxData->SduDataPtr[3])
							{
								case DCM_RETURN_CONTROL_TO_ECU:
									responseCode = DspIOControlReturnControlToECU(DidPtr,pduRxData,pduTxData);
									break;
								case DCM_RESET_TO_DEFAULT:
									responseCode = DspIOControlResetToDefault(DidPtr,pduRxData,pduTxData);								
									break;
								case DCM_FREEZE_CURRENT_STATE:
									responseCode = DspIOControlFreezeCurrentState(DidPtr,pduRxData,pduTxData);
									break;
								case DCM_SHORT_TERM_ADJUSTMENT:
									responseCode = DspIOControlShortTeamAdjustment(DidPtr,pduRxData,pduTxData);
									break;
								default:
									responseCode = DCM_E_REQUESTOUTOFRANGE;
									break;
								
							}
						}
						else
						{
							responseCode = DCM_E_SECUTITYACCESSDENIED;
						}
					}
					else
					{
						responseCode = DCM_E_REQUESTOUTOFRANGE;
					}
				}
				else
				{
					responseCode = DCM_E_REQUESTOUTOFRANGE;
				}
			}
			else
			{
				/* if UsePort == True, NRC 0x10 */
				responseCode = DCM_E_GENERALREJECT;
			}
		}
		else
		{
			responseCode = DCM_E_REQUESTOUTOFRANGE;
		}
	}
	else
	{
		responseCode = DCM_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}
	if(responseCode == DCM_E_POSITIVERESPONSE)
	{
		pduTxData->SduDataPtr[1] = pduRxData->SduDataPtr[1];
		pduTxData->SduDataPtr[2] = pduRxData->SduDataPtr[2];
	}
	DsdDspProcessingDone(responseCode);
}
