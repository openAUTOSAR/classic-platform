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

// 904 PC-Lint MISRA 14.7: OK. Allow VALIDATE, VALIDATE_RV and VALIDATE_NO_RV to return value.
//lint -emacro(904,VALIDATE_RV,VALIDATE_NO_RV,VALIDATE)
// 522 PC-Lint exception for empty functions
//lint -esym(522,storeFreezeFrameDataEvtMem)
//lint -esym(522,deleteFreezeFrameDataPriMem)
//lint -esym(522,storeFreezeFrameDataPreInit)
//lint -esym(522,storeFreezeFrameDataPriMem)
//lint -esym(522,updateFreezeFrameOccurrencePreInit)


/*
 *  General requirements
 */
/** @req DEM126 */
/** @req DEM151.partially */
/** @req DEM152 */
/** @req DEM013.14229-1 */
/** @req DEM277 */
/** @req DEM363 */
/** @req DEM113 */ /** @req DEM174 */
/** @req DEM286 */
/** @req DEM267 */
/** @req DEM364 */
/** @req DEM114 */
/** @req DEM124 */
/** @req DEM370 */



#include <string.h>
#include "Dem.h"
//#include "Fim.h"
//#include "Nvm.h"
//#include "SchM_Dem.h"
#include "MemMap.h"
#include "McuExtensions.h"

/*
 * Local defines
 */
#define DEBOUNCE_FDC_TEST_FAILED  127
#define DEBOUNCE_FDC_TEST_PASSED -128

#if  ( DEM_DEV_ERROR_DETECT == STD_ON )
#include "Det.h"
/** @req DEM117 */
#define VALIDATE_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_DEM, 0, _api, _err); \
          return _rv; \
        }

#define VALIDATE_NO_RV(_exp,_api,_err ) \
  if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_DEM, 0, _api, _err); \
          return; \
        }
#define DET_REPORTERROR(_x,_y,_z,_q) Det_ReportError(_x, _y, _z, _q)

#else
#define VALIDATE_RV(_exp,_api,_err,_rv )
#define VALIDATE_NO_RV(_exp,_api,_err )
#define DET_REPORTERROR(_x,_y,_z,_q)
#endif

#if (DEM_OBD_SUPPORT == STD_ON)
#error "DEM_OBD_SUPPORT is set to STD_ON, this is not supported by the code."
#endif

#if (DEM_PTO_SUPPORT == STD_ON)
#error "DEM_PTO_SUPPORT is set to STD_ON, this is not supported by the code."
#endif

#if (DEM_TYPE_OF_DTC_SUPPORTED != 0x01)
#error "DEM_TYPE_OF_DTC_SUPPORTED is not set to 1 (ISO14229-1), only ISO14229-1 is currently supported by the code."
#endif


/*
 * Local types
 */

typedef uint16 ChecksumType;

// DtcFilterType
typedef struct {
	Dem_EventStatusExtendedType dtcStatusMask;
	Dem_DTCKindType				dtcKind;
	Dem_DTCOriginType			dtcOrigin;
	Dem_FilterWithSeverityType	filterWithSeverity;
	Dem_DTCSeverityType			dtcSeverityMask;
	Dem_FilterForFDCType		filterForFaultDetectionCounter;
	uint16						faultIndex;
} DtcFilterType;

// DisableDtcStorageType
typedef struct {
	boolean						storageDisabled;
	Dem_DTCGroupType			dtcGroup;
	Dem_DTCKindType				dtcKind;
} DisableDtcStorageType;

// For keeping track of the events status
typedef struct {
	Dem_EventIdType				eventId;
	const Dem_EventParameterType *eventParamRef;
	sint8						faultDetectionCounter;
	uint16						occurrence;				/** @req DEM011 */
	Dem_EventStatusExtendedType	eventStatusExtended;	/** @req DEM006 */
	boolean						errorStatusChanged;
} EventStatusRecType;


// Types for storing different event data on event memory
typedef struct {
	Dem_EventIdType		eventId;
	uint16				occurrence;
	ChecksumType		checksum;
} EventRecType;

typedef struct {
	Dem_EventIdType		eventId;
	uint16				occurrence;
	uint16				dataSize;
	sint8				data[DEM_MAX_SIZE_FF_DATA];
	ChecksumType		checksum;
} FreezeFrameRecType;

typedef struct {
	Dem_EventIdType		eventId;
	uint16				dataSize;
	uint8				data[DEM_MAX_SIZE_EXT_DATA];
	ChecksumType		checksum;
} ExtDataRecType;


// State variable
typedef enum
{
  DEM_UNINITIALIZED = 0,
  DEM_PREINITIALIZED,
  DEM_INITIALIZED
} Dem_StateType; /** @req DEM169 */


static Dem_StateType demState = DEM_UNINITIALIZED;

// Help pointer to configuration set
static const Dem_ConfigSetType *configSet;

/*
 * Allocation of DTC filter parameters
 */
static DtcFilterType dtcFilter;

/*
 * Allocation of Disable/Enable DTC storage parameters
 */
static DisableDtcStorageType disableDtcStorage;

/*
 * Allocation of operation cycle state list
 */
static Dem_OperationCycleStateType operationCycleStateList[DEM_OPERATION_CYCLE_ID_ENDMARK];

/*
 * Allocation of local event status buffer
 */
static EventStatusRecType	eventStatusBuffer[DEM_MAX_NUMBER_EVENT];

/*
 * Allocation of pre-init event memory (used between pre-init and init)
 */
/** @req DEM207 */
static FreezeFrameRecType	preInitFreezeFrameBuffer[DEM_MAX_NUMBER_FF_DATA_PRE_INIT];
static ExtDataRecType		preInitExtDataBuffer[DEM_MAX_NUMBER_EXT_DATA_PRE_INIT];

/*
 * Allocation of primary event memory ramlog (after init) in uninitialized memory
 */
/** @req DEM162 */
static EventRecType 		priMemEventBuffer[DEM_MAX_NUMBER_EVENT_PRI_MEM] __attribute__ ((section (".dem_eventmemory_pri")));
static FreezeFrameRecType	priMemFreezeFrameBuffer[DEM_MAX_NUMBER_FF_DATA_PRI_MEM] __attribute__ ((section (".dem_eventmemory_pri")));
static ExtDataRecType		priMemExtDataBuffer[DEM_MAX_NUMBER_EXT_DATA_PRI_MEM] __attribute__ ((section (".dem_eventmemory_pri")));


/*
 * Procedure:	zeroPriMemBuffers
 * Description:	Fill the primary buffers with zeroes
 */
//lint -e957	PC-Lint exception - Used only by DemTest
void demZeroPriMemBuffers(void)
{
	memset(priMemEventBuffer, 0, sizeof(priMemEventBuffer));
	memset(priMemFreezeFrameBuffer, 0, sizeof(priMemFreezeFrameBuffer));
	memset(priMemExtDataBuffer, 0, sizeof(priMemExtDataBuffer));
}


/*
 * Procedure:	calcChecksum
 * Description:	Calculate checksum over *data to *(data+nrOfBytes-1) area
 */
static ChecksumType calcChecksum(void *data, uint16 nrOfBytes)
{
	uint16 i;
	uint8 *byte = (uint8*)data;
	ChecksumType sum = 0;

	for (i = 0; i < nrOfBytes; i++) {
		sum += byte[i];
	}
	sum ^= 0xaaaau;
	return sum;
}


/*
 * Procedure:	checkDtcKind
 * Description:	Return TRUE if "dtcKind" match the events DTCKind or "dtcKind"
 * 				is "DEM_DTC_KIND_ALL_DTCS" otherwise FALSE.
 */
static boolean checkDtcKind(Dem_DTCKindType dtcKind, const Dem_EventParameterType *eventParam)
{
	boolean result = FALSE;

	if (dtcKind == DEM_DTC_KIND_ALL_DTCS) {
		result = TRUE;
	}
	else {
		if (eventParam->DTCClassRef != NULL) {
			if (eventParam->DTCClassRef->DTCKind == dtcKind) {
				result = TRUE;
			}
		}
	}
	return result;
}


/*
 * Procedure:	checkDtcGroup
 * Description:	Return TRUE if "dtc" match the events DTC or "dtc" is
 * 				"DEM_DTC_GROUP_ALL_DTCS" otherwise FALSE.
 */
static boolean checkDtcGroup(uint32 dtc, const Dem_EventParameterType *eventParam)
{
	boolean result = FALSE;

	if (dtc == DEM_DTC_GROUP_ALL_DTCS) {
		result = TRUE;
	}
	else {
		if (eventParam->DTCClassRef != NULL) {
			if (eventParam->DTCClassRef->DTC == dtc) {
				result = TRUE;
			}
		}
	}
	return result;
}


/*
 * Procedure:	checkDtcOrigin
 * Description:	Return TRUE if "dtcOrigin" match any of the events DTCOrigin otherwise FALSE.
 */
static boolean checkDtcOrigin(Dem_DTCOriginType dtcOrigin, const Dem_EventParameterType *eventParam)
{
	boolean result = FALSE;
	boolean dtcOriginFound = FALSE;
	uint16 i;

	for (i = 0;(i < DEM_MAX_NR_OF_EVENT_DESTINATION) && (!dtcOriginFound); i++){
		dtcOriginFound = (eventParam->EventClass->EventDestination[i] == dtcOrigin);
	}

	if (dtcOriginFound) {
		result = TRUE;
	}

	return result;
}

/*
 * Procedure:	checkDtcSeverityMask
 * Description:	Return TRUE if "dtcSeverityMask" match any of the events DTC severity otherwise FALSE.
 */
// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715}		Symbol not referenced
static boolean checkDtcSeverityMask(Dem_DTCSeverityType dtcSeverityMask, const Dem_EventParameterType *eventParam)
{
	boolean result = TRUE;

	// TODO: This function is optional, may be implemented here.

	return result;
}


/*
 * Procedure:	checkDtcFaultDetectionCounterMask
 * Description:	TBD.
 */
// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715}		Symbol not referenced
static boolean checkDtcFaultDetectionCounter(const Dem_EventParameterType *eventParam)
{
	boolean result = TRUE;

	// TODO: Not implemented yet.

	return result;
}


/*
 * Procedure:	lookupEventStatusRec
 * Description:	Returns the pointer to event id parameters of "eventId" in "*eventStatusBuffer",
 * 				if not found NULL is returned.
 */
static void lookupEventStatusRec(Dem_EventIdType eventId, EventStatusRecType **const eventStatusRec)
{
	uint8 i;
	boolean eventIdFound = FALSE;

	for (i = 0; (i < DEM_MAX_NUMBER_EVENT) && (!eventIdFound); i++) {
		eventIdFound = (eventStatusBuffer[i].eventId == eventId);
	}

	if (eventIdFound) {
		*eventStatusRec = &eventStatusBuffer[i-1];
	} else {
		*eventStatusRec = NULL;
	}
}


/*
 * Procedure:	lookupEventIdParameter
 * Description:	Returns the pointer to event id parameters of "eventId" in "*eventIdParam",
 * 				if not found NULL is returned.
 */
static void lookupEventIdParameter(Dem_EventIdType eventId, const Dem_EventParameterType **const eventIdParam)
{
	const Dem_EventParameterType *EventIdParamList = configSet->EventParameter;

	// Lookup the correct event id parameters
	uint16 i=0;
	while ((EventIdParamList[i].EventID != eventId) && (!EventIdParamList[i].Arc_EOL)) {
		i++;
	}

	if (!EventIdParamList[i].Arc_EOL) {
		*eventIdParam = &EventIdParamList[i];
	} else {
		*eventIdParam = NULL;
	}
}


/*
 * Procedure:	preDebounceNone
 * Description:	Returns the result of the debouncing.
 */
static Dem_EventStatusType preDebounceNone(const Dem_EventStatusType reportedStatus, const EventStatusRecType* statusRecord) {
	Dem_EventStatusType returnCode;
	(void)statusRecord;		// Just to get rid of PC-Lint warnings

	switch (reportedStatus) {
	case DEM_EVENT_STATUS_FAILED: /** @req DEM091.NoneFailed */
	case DEM_EVENT_STATUS_PASSED: /** @req DEM091.NonePassed */
		// Already debounced, do nothing.
		break;

	default:
		// TODO: What to do with PREFAIL and PREPASSED on no debouncing?
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_PREDEBOUNCE_NONE_ID, DEM_E_PARAM_DATA);
		break;
	}

	returnCode = reportedStatus;
	return returnCode;
}


/*
 * Procedure:	preDebounceCounterBased
 * Description:	Returns the result of the debouncing.
 */
static Dem_EventStatusType preDebounceCounterBased(Dem_EventStatusType reportedStatus, EventStatusRecType* statusRecord) {
	Dem_EventStatusType returnCode;
	const Dem_PreDebounceCounterBasedType* pdVars = statusRecord->eventParamRef->EventClass->PreDebounceAlgorithmClass->PreDebounceAlgorithm.PreDebounceCounterBased;

	switch (reportedStatus) {
	case DEM_EVENT_STATUS_PREFAILED:
		if (statusRecord->faultDetectionCounter < DEBOUNCE_FDC_TEST_FAILED) {
			if ((pdVars->JumpUp) && (statusRecord->faultDetectionCounter < 0)) {
				statusRecord->faultDetectionCounter = 0;
			} else {
				if (((sint16)statusRecord->faultDetectionCounter + (sint8)pdVars->CountInStepSize) < DEBOUNCE_FDC_TEST_FAILED) {
					statusRecord->faultDetectionCounter += (sint8)pdVars->CountInStepSize;
				} else {
					statusRecord->faultDetectionCounter = DEBOUNCE_FDC_TEST_FAILED;
				}
			}
		}
		break;

	case DEM_EVENT_STATUS_PREPASSED:
		if (statusRecord->faultDetectionCounter > DEBOUNCE_FDC_TEST_PASSED) {
			if ((pdVars->JumpDown) && (statusRecord->faultDetectionCounter > 0)) {
				statusRecord->faultDetectionCounter = 0;
			} else {
				if (((sint16)statusRecord->faultDetectionCounter - (sint8)pdVars->CountOutStepSize) > DEBOUNCE_FDC_TEST_PASSED) {
					statusRecord->faultDetectionCounter -= (sint8)pdVars->CountOutStepSize;
				} else {
					statusRecord->faultDetectionCounter = DEBOUNCE_FDC_TEST_PASSED;
				}
			}
		}
		break;

	case DEM_EVENT_STATUS_FAILED:
		statusRecord->faultDetectionCounter = DEBOUNCE_FDC_TEST_FAILED; /** @req DEM091.CounterFailed */
		break;

	case DEM_EVENT_STATUS_PASSED:
		statusRecord->faultDetectionCounter = DEBOUNCE_FDC_TEST_PASSED; /** @req DEM091.CounterPassed */
		break;

	default:
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_PREDEBOUNCE_COUNTER_BASED_ID, DEM_E_PARAM_DATA);
		break;

	}

	switch (statusRecord->faultDetectionCounter) {
	case DEBOUNCE_FDC_TEST_FAILED:
		returnCode = DEM_EVENT_STATUS_FAILED;
		break;

	case DEBOUNCE_FDC_TEST_PASSED:
		returnCode = DEM_EVENT_STATUS_PASSED;
		break;

	default:
		returnCode = reportedStatus;
		break;
	}

	return returnCode;
}


/*
 * Procedure:	updateEventStatusRec
 * Description:	Update the status of "eventId", if not exist and "createIfNotExist" is
 * 				true a new record is created
 */
static void updateEventStatusRec(const Dem_EventParameterType *eventParam, Dem_EventStatusType eventStatus, boolean createIfNotExist, EventStatusRecType *eventStatusRec)
{
	EventStatusRecType *eventStatusRecPtr;
	imask_t state = McuE_EnterCriticalSection();

	// Lookup event ID
	lookupEventStatusRec(eventParam->EventID, &eventStatusRecPtr);

	if ((eventStatusRecPtr == NULL) && (createIfNotExist)) {
		// Search for free position
		lookupEventStatusRec(DEM_EVENT_ID_NULL, &eventStatusRecPtr);

		if (eventStatusRecPtr != NULL) {
			// Create new event record
			eventStatusRecPtr->eventId = eventParam->EventID;
			eventStatusRecPtr->eventParamRef = eventParam;
			eventStatusRecPtr->faultDetectionCounter = 0;
			eventStatusRecPtr->occurrence = 0;
			eventStatusRecPtr->eventStatusExtended = DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR | DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE;
			eventStatusRecPtr->errorStatusChanged = FALSE;
		}
		else {
			// Error: Event status buffer full
			DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_UPDATE_EVENT_STATUS_ID, DEM_E_EVENT_STATUS_BUFF_FULL);
		}
	}


	if (eventStatusRecPtr != NULL) {
		// Handle debouncing
		if (eventParam->EventClass->PreDebounceAlgorithmClass != NULL) {
			switch (eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceName) { /** @req DEM004 */ /** @req DEM342 */
			case DEM_NO_PRE_DEBOUNCE:
				eventStatus = preDebounceNone(eventStatus, eventStatusRecPtr);
				break;

			case DEM_PRE_DEBOUNCE_COUNTER_BASED:
				eventStatus = preDebounceCounterBased(eventStatus, eventStatusRecPtr);
				break;

			default:
				// Don't know how to handle this.
				DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_UPDATE_EVENT_STATUS_ID, DEM_E_NOT_IMPLEMENTED_YET);
				break;
			}
		}

		eventStatusRecPtr->errorStatusChanged = FALSE;

		// Check test failed
		if (eventStatus == DEM_EVENT_STATUS_FAILED) {
			if (!(eventStatusRecPtr->eventStatusExtended & DEM_TEST_FAILED)) {
				eventStatusRecPtr->occurrence++;
				eventStatusRecPtr->errorStatusChanged = TRUE;
			}
			/** @req DEM036 */ /** @req DEM379.PendingSet */
			eventStatusRecPtr->eventStatusExtended |= (DEM_TEST_FAILED | DEM_TEST_FAILED_THIS_OPERATION_CYCLE | DEM_TEST_FAILED_SINCE_LAST_CLEAR | DEM_PENDING_DTC);
			eventStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)~(DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR | DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE);
		}

		// Check test passed
		if (eventStatus == DEM_EVENT_STATUS_PASSED) {
			if (eventStatusRecPtr->eventStatusExtended & DEM_TEST_FAILED) {
				eventStatusRecPtr->errorStatusChanged = TRUE;
			}
			/** @req DEM036 */
			eventStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)~DEM_TEST_FAILED;
			eventStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)~(DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR | DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE);
		}

		// Copy the record
		memcpy(eventStatusRec, eventStatusRecPtr, sizeof(EventStatusRecType));
	}
	else {
		// Copy an empty record to return data
		eventStatusRec->eventId = DEM_EVENT_ID_NULL;
		eventStatusRec->faultDetectionCounter = 0;
		eventStatusRec->occurrence = 0;
		eventStatusRec->eventStatusExtended = DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE | DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR;
		eventStatusRec->errorStatusChanged = FALSE;
	}

	McuE_ExitCriticalSection(state);
}


/*
 * Procedure:	mergeEventStatusRec
 * Description:	Update the occurrence counter of status, if not exist a new record is created
 */
static void mergeEventStatusRec(const EventRecType *eventRec)
{
	EventStatusRecType *eventStatusRecPtr;
	imask_t state = McuE_EnterCriticalSection();

	// Lookup event ID
	lookupEventStatusRec(eventRec->eventId, &eventStatusRecPtr);

	if (eventStatusRecPtr != NULL) {
		// Update occurrence counter, rest of pre init state is kept.
		eventStatusRecPtr->occurrence += eventRec->occurrence;

	}
	else {
		// Search for free position
		lookupEventStatusRec(DEM_EVENT_ID_NULL, &eventStatusRecPtr);

		if (eventStatusRecPtr != NULL) {
			// Create new event, from stored event
			eventStatusRecPtr->eventId = eventRec->eventId;
			lookupEventIdParameter(eventRec->eventId, &eventStatusRecPtr->eventParamRef);
			eventStatusRecPtr->faultDetectionCounter = 0;
			eventStatusRecPtr->occurrence = eventRec->occurrence;
			eventStatusRecPtr->eventStatusExtended = DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE | DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR;
			eventStatusRecPtr->errorStatusChanged = FALSE;
		}
		else {
			// Error: Event status buffer full
			DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_MERGE_EVENT_STATUS_ID, DEM_E_EVENT_STATUS_BUFF_FULL);
		}
	}

	McuE_ExitCriticalSection(state);
}


/*
 * Procedure:	deleteEventStatusRec
 * Description:	Delete the status record of "eventParam->eventId" from "eventStatusBuffer".
 */
static void deleteEventStatusRec(const Dem_EventParameterType *eventParam)
{
	EventStatusRecType *eventStatusRecPtr;
	imask_t state = McuE_EnterCriticalSection();

	// Lookup event ID
	lookupEventStatusRec(eventParam->EventID, &eventStatusRecPtr);

	if (eventStatusRecPtr != NULL) {
		// Delete event record
		memset(eventStatusRecPtr, 0, sizeof(EventStatusRecType));
	}

	McuE_ExitCriticalSection(state);
}


/*
 * Procedure:	getEventStatusRec
 * Description:	Returns the status record of "eventId" in "eventStatusRec"
 */
static void getEventStatusRec(Dem_EventIdType eventId, EventStatusRecType *eventStatusRec)
{
	EventStatusRecType *eventStatusRecPtr;

	// Lookup event ID
	lookupEventStatusRec(eventId, &eventStatusRecPtr);

	if (eventStatusRecPtr != NULL) {
		// Copy the record
		memcpy(eventStatusRec, eventStatusRecPtr, sizeof(EventStatusRecType));
	}
	else {
		eventStatusRec->eventId = DEM_EVENT_ID_NULL;
	}
}


/*
 * Procedure:	lookupDtcEvent
 * Description:	Returns TRUE if the DTC was found and "eventStatusRec" points
 * 				to the event record found.
 */
static boolean lookupDtcEvent(uint32 dtc, EventStatusRecType **eventStatusRec)
{
	boolean dtcFound = FALSE;
	uint16 i;

	*eventStatusRec = NULL;

	for (i = 0; (i < DEM_MAX_NUMBER_EVENT) && (!dtcFound); i++) {
		if (eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL) {
			if (eventStatusBuffer[i].eventParamRef->DTCClassRef != NULL) {

				// Check DTC
				if (eventStatusBuffer[i].eventParamRef->DTCClassRef->DTC == dtc) {
					*eventStatusRec = &eventStatusBuffer[i];
					dtcFound = TRUE;
				}
			}
		}
	}

	return dtcFound;
}


/*
 * Procedure:	matchEventWithDtcFilter
 * Description:	Returns TRUE if the event pointed by "event" fulfill
 * 				the "dtcFilter" global filter settings.
 */
static boolean matchEventWithDtcFilter(const EventStatusRecType *eventRec)
{
	boolean dtcMatch = FALSE;

	// Check status
	if ((dtcFilter.dtcStatusMask == DEM_DTC_STATUS_MASK_ALL) || (eventRec->eventStatusExtended & dtcFilter.dtcStatusMask)) {
		if (eventRec->eventParamRef != NULL) {

			// Check dtcKind
			if (checkDtcKind(dtcFilter.dtcKind, eventRec->eventParamRef)) {

				// Check dtcOrigin
				if (checkDtcOrigin(dtcFilter.dtcOrigin, eventRec->eventParamRef)) {

					// Check severity
					if ((dtcFilter.filterWithSeverity == DEM_FILTER_WITH_SEVERITY_NO)
						|| ((dtcFilter.filterWithSeverity == DEM_FILTER_WITH_SEVERITY_YES) && (checkDtcSeverityMask(dtcFilter.dtcSeverityMask, eventRec->eventParamRef)))) {

						// Check fault detection counter
						if ((dtcFilter.filterForFaultDetectionCounter == DEM_FILTER_FOR_FDC_NO)
							|| ((dtcFilter.filterWithSeverity == DEM_FILTER_FOR_FDC_YES) && (checkDtcFaultDetectionCounter(eventRec->eventParamRef)))) {
							dtcMatch = TRUE;
						}
					}
				}
			}
		}
	}

	return dtcMatch;
}


// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715}		Symbol not referenced
static void getFreezeFrameData(const Dem_EventParameterType *eventParam, FreezeFrameRecType *freezeFrame)
{
	// TODO: Fill out
	freezeFrame->eventId = DEM_EVENT_ID_NULL;	// Not supported yet
}


// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715}		Symbol not referenced
static void storeFreezeFrameDataPreInit(const Dem_EventParameterType *eventParam, const FreezeFrameRecType *freezeFrame)
{
	// TODO: Fill out
}


// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715}		Symbol not referenced
static void updateFreezeFrameOccurrencePreInit(const EventRecType *EventBuffer)
{
	// TODO: Fill out
}


/*
 * Procedure:	getExtendedData
 * Description:	Collects the extended data according to "eventParam" and return it in "extData",
 * 				if not found eventId is set to DEM_EVENT_ID_NULL.
 */
static void getExtendedData(const Dem_EventParameterType *eventParam, ExtDataRecType *extData)
{
	Std_ReturnType callbackReturnCode;
	uint16 i;
	uint16 storeIndex = 0;
	uint16 recordSize;

	// Clear ext data record
	memset(extData, 0, sizeof(ExtDataRecType));

	// Check if any pointer to extended data class
	if (eventParam->ExtendedDataClassRef != NULL) {
		// Request extended data and copy it to the buffer
		for (i = 0; (i < DEM_MAX_NR_OF_RECORDS_IN_EXTENDED_DATA) && (eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i] != NULL); i++) {
			recordSize = eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i]->DataSize;
			if ((storeIndex + recordSize) <= DEM_MAX_SIZE_EXT_DATA) {
				callbackReturnCode = eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i]->CallbackGetExtDataRecord(&extData->data[storeIndex]); /** @req DEM282 */
				if (callbackReturnCode != E_OK) {
					// Callback data currently not available, clear space.
					memset(&extData->data[storeIndex], 0xFF, recordSize);
				}
				storeIndex += recordSize;
			}
			else {
				// Error: Size of extended data record is bigger than reserved space.
				DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GET_EXTENDED_DATA_ID, DEM_E_EXT_DATA_TOO_BIG);
				break;	// Break the loop
			}
		}
	}

	// Check if any data has been stored
	if (storeIndex != 0) {
		extData->eventId = eventParam->EventID;
		extData->dataSize = storeIndex;
		extData->checksum = calcChecksum(extData, sizeof(ExtDataRecType)-sizeof(ChecksumType));
	}
	else {
		extData->eventId = DEM_EVENT_ID_NULL;
		extData->dataSize = storeIndex;
		extData->checksum = 0;
	}
}


/*
 * Procedure:	storeExtendedDataPreInit
 * Description:	Store the extended data pointed by "extendedData" to the "preInitExtDataBuffer",
 * 				if non existent a new entry is created.
 */
static void storeExtendedDataPreInit(const Dem_EventParameterType *eventParam, const ExtDataRecType *extendedData)
{
	boolean eventIdFound = FALSE;
	boolean eventIdFreePositionFound=FALSE;
	uint16 i;
	imask_t state = McuE_EnterCriticalSection();

	// Check if already stored
	for (i = 0; (i<DEM_MAX_NUMBER_EXT_DATA_PRE_INIT) && (!eventIdFound); i++){
		eventIdFound = (preInitExtDataBuffer[i].eventId == eventParam->EventID);
	}

	if(eventIdFound){
		// Yes, overwrite existing
		memcpy(&preInitExtDataBuffer[i-1], extendedData, sizeof(ExtDataRecType));
	}
	else{
		// No, lookup first free position
		for (i = 0; (i<DEM_MAX_NUMBER_EXT_DATA_PRE_INIT) && (!eventIdFreePositionFound); i++){
			if(preInitExtDataBuffer[i].eventId ==0){
				eventIdFreePositionFound=TRUE;
			}
		}

		if (eventIdFreePositionFound) {
			memcpy(&preInitExtDataBuffer[i-1], extendedData, sizeof(ExtDataRecType));
		}
		else {
			// Error: Pre init extended data buffer full
			DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_STORE_EXT_DATA_PRE_INIT_ID, DEM_E_PRE_INIT_EXT_DATA_BUFF_FULL);
		}
	}

	McuE_ExitCriticalSection(state);
}

/*
 * Procedure:	storeEventPriMem
 * Description:	Store the event data of "eventStatus->eventId" in "priMemEventBuffer",
 * 				if non existent a new entry is created.
 */
static void storeEventPriMem(const Dem_EventParameterType *eventParam, const EventStatusRecType *eventStatus)
{
	boolean eventIdFound = FALSE;
	boolean eventIdFreePositionFound=FALSE;
	uint16 i;
	imask_t state = McuE_EnterCriticalSection();

	(void)*eventParam;	// Currently not used, do this to avoid warning

	// Lookup event ID
	for (i = 0; (i < DEM_MAX_NUMBER_EVENT_ENTRY_PRI) && (!eventIdFound); i++){
		eventIdFound = (priMemEventBuffer[i].eventId == eventStatus->eventId);
	}

	if (eventIdFound) {
		// Update event found
		priMemEventBuffer[i-1].occurrence = eventStatus->occurrence;
		priMemEventBuffer[i-1].checksum = calcChecksum(&priMemEventBuffer[i-1], sizeof(EventRecType)-sizeof(ChecksumType));
	}
	else {
		// Search for free position
		for (i=0; (i < DEM_MAX_NUMBER_EVENT_ENTRY_PRI) && (!eventIdFreePositionFound); i++){
			eventIdFreePositionFound = (priMemEventBuffer[i].eventId == DEM_EVENT_ID_NULL);
		}


		if (eventIdFreePositionFound) {
			priMemEventBuffer[i-1].eventId = eventStatus->eventId;
			priMemEventBuffer[i-1].occurrence = eventStatus->occurrence;
			priMemEventBuffer[i-1].checksum = calcChecksum(&priMemEventBuffer[i-1], sizeof(EventRecType)-sizeof(ChecksumType));
		}
		else {
			// Error: Pri mem event buffer full
			DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_STORE_EVENT_PRI_MEM_ID, DEM_E_PRI_MEM_EVENT_BUFF_FULL);
		}
	}

	McuE_ExitCriticalSection(state);
}

/*
 * Procedure:	deleteEventPriMem
 * Description:	Delete the event data of "eventParam->eventId" from "priMemEventBuffer".
 */
static void deleteEventPriMem(const Dem_EventParameterType *eventParam)
{
	boolean eventIdFound = FALSE;
	uint16 i;
	imask_t state = McuE_EnterCriticalSection();


	// Lookup event ID
	for (i = 0; (i < DEM_MAX_NUMBER_EVENT_ENTRY_PRI) && (!eventIdFound); i++){
		eventIdFound = (priMemEventBuffer[i].eventId == eventParam->EventID);
	}

	if (eventIdFound) {
		// Delete event found
		memset(&priMemEventBuffer[i-1], 0, sizeof(EventRecType));
	}

	McuE_ExitCriticalSection(state);
}

/*
 * Procedure:	storeEventEvtMem
 * Description:	Store the event data of "eventStatus->eventId" in event memory according to
 * 				"eventParam" destination option.
 */
static void storeEventEvtMem(const Dem_EventParameterType *eventParam, const EventStatusRecType *eventStatus)
{
	uint16 i;

	for (i = 0; (i < DEM_MAX_NR_OF_EVENT_DESTINATION)
				 && (eventParam->EventClass->EventDestination[i] != DEM_EVENT_DESTINATION_END_OF_LIST); i++) {
		switch (eventParam->EventClass->EventDestination[i])
		{
		case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
			storeEventPriMem(eventParam, eventStatus);	/** @req DEM010 */
			break;

		case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
		case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
		case DEM_DTC_ORIGIN_MIRROR_MEMORY:
			// Not yet supported
			DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GLOBAL_ID, DEM_E_NOT_IMPLEMENTED_YET);
			break;
		default:
			break;
		}
	}
}


/*
 * Procedure:	storeExtendedDataPriMem
 * Description:	Store the extended data pointed by "extendedData" to the "priMemExtDataBuffer",
 * 				if non existent a new entry is created.
 */
static void storeExtendedDataPriMem(const Dem_EventParameterType *eventParam, const ExtDataRecType *extendedData) /** @req DEM041 */
{
	boolean eventIdFound = FALSE;
	boolean eventIdFreePositionFound=FALSE;
	uint16 i;
	imask_t state = McuE_EnterCriticalSection();

	// Check if already stored
	for (i = 0; (i<DEM_MAX_NUMBER_EXT_DATA_PRI_MEM) && (!eventIdFound); i++){
		eventIdFound = (priMemExtDataBuffer[i].eventId == eventParam->EventID);
	}

	if (eventIdFound) {
		// Yes, overwrite existing
		memcpy(&priMemExtDataBuffer[i-1], extendedData, sizeof(ExtDataRecType));
	}
	else {
		// No, lookup first free position
		for (i = 0; (i < DEM_MAX_NUMBER_EXT_DATA_PRI_MEM) && (!eventIdFreePositionFound); i++){
			eventIdFreePositionFound =  (priMemExtDataBuffer[i].eventId == DEM_EVENT_ID_NULL);
		}
		if (eventIdFreePositionFound) {
			memcpy(&priMemExtDataBuffer[i-1], extendedData, sizeof(ExtDataRecType));
		}
		else {
			// Error: Pri mem extended data buffer full
			DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_STORE_EXT_DATA_PRI_MEM_ID, DEM_E_PRI_MEM_EXT_DATA_BUFF_FULL);
		}
	}

	McuE_ExitCriticalSection(state);
}

/*
 * Procedure:	deleteExtendedDataPriMem
 * Description:	Delete the extended data of "eventParam->eventId" from "priMemExtDataBuffer".
 */
static void deleteExtendedDataPriMem(const Dem_EventParameterType *eventParam)
{
	boolean eventIdFound = FALSE;
	uint16 i;
	imask_t state = McuE_EnterCriticalSection();

	// Check if already stored
	for (i = 0;(i<DEM_MAX_NUMBER_EXT_DATA_PRI_MEM) && (!eventIdFound); i++){
		eventIdFound = (priMemExtDataBuffer[i].eventId == eventParam->EventID);
	}

	if (eventIdFound) {
		// Yes, clear record
		memset(&priMemExtDataBuffer[i-1], 0, sizeof(ExtDataRecType));
	}

	McuE_ExitCriticalSection(state);
}

/*
 * Procedure:	storeExtendedDataEvtMem
 * Description:	Store the extended data in event memory according to
 * 				"eventParam" destination option
 */
static void storeExtendedDataEvtMem(const Dem_EventParameterType *eventParam, const ExtDataRecType *extendedData)
{
	uint16 i;

	for (i = 0; (i < DEM_MAX_NR_OF_EVENT_DESTINATION) && (eventParam->EventClass->EventDestination[i] != DEM_EVENT_DESTINATION_END_OF_LIST); i++) {
		switch (eventParam->EventClass->EventDestination[i])
		{
		case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
			storeExtendedDataPriMem(eventParam, extendedData);
			break;

		case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
		case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
		case DEM_DTC_ORIGIN_MIRROR_MEMORY:
			// Not yet supported
			DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GLOBAL_ID, DEM_E_NOT_IMPLEMENTED_YET);
			break;

		default:
			break;
		}
	}
}


/*
 * Procedure:	lookupExtendedDataRecNumParam
 * Description:	Returns TRUE if the requested extended data number was found among the configured records for the event.
 * 				"extDataRecClassPtr" returns a pointer to the record class, "posInExtData" returns the position in stored extended data.
 */
static boolean lookupExtendedDataRecNumParam(uint8 extendedDataNumber, const Dem_EventParameterType *eventParam, Dem_ExtendedDataRecordClassType const **extDataRecClassPtr, uint16 *posInExtData)
{
	boolean recNumFound = FALSE;

	if (eventParam->ExtendedDataClassRef != NULL) {
		uint16	byteCnt = 0;
		uint16 i;

		// Request extended data and copy it to the buffer
		for (i = 0; (i < DEM_MAX_NR_OF_RECORDS_IN_EXTENDED_DATA) && (eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i] != NULL) && (!recNumFound); i++) {
			if (eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i]->RecordNumber == extendedDataNumber) {
				*extDataRecClassPtr =  eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i];
				*posInExtData = byteCnt;
				recNumFound = TRUE;
			}
			byteCnt += eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i]->DataSize;
		}
	}

	return recNumFound;
}


/*
 * Procedure:	lookupExtendedDataPriMem
 * Description: Returns TRUE if the requested event id is found, "extData" points to the found data.
 */
static boolean lookupExtendedDataPriMem(Dem_EventIdType eventId, ExtDataRecType **extData)
{
	boolean eventIdFound = FALSE;
	sint16 i;

	// Lookup corresponding extended data
	for (i = 0; (i < DEM_MAX_NUMBER_EXT_DATA_PRI_MEM) && (!eventIdFound); i++) {
		eventIdFound = (priMemExtDataBuffer[i].eventId == eventId);
	}

	if (eventIdFound) {
		// Yes, return pointer
		*extData = &priMemExtDataBuffer[i-1];
	}

	return eventIdFound;
}

// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715}		Symbol not referenced
static void storeFreezeFrameDataPriMem(const Dem_EventParameterType *eventParam, const FreezeFrameRecType *freezeFrame)
{
	// TODO: Fill out
}


// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715}		Symbol not referenced
static void deleteFreezeFrameDataPriMem(const Dem_EventParameterType *eventParam)
{
	// TODO: Fill out
}


/*
 * Procedure:	storeFreezeFrameDataEvtMem
 * Description:	Store the freeze frame data in event memory according to
 * 				"eventParam" destination option
 */
static void storeFreezeFrameDataEvtMem(const Dem_EventParameterType *eventParam, const FreezeFrameRecType *freezeFrame)
{
	uint16 i;

	for (i = 0; (i < DEM_MAX_NR_OF_EVENT_DESTINATION) && (eventParam->EventClass->EventDestination[i] != DEM_EVENT_DESTINATION_END_OF_LIST); i++) {
		switch (eventParam->EventClass->EventDestination[i])
		{
		case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
			storeFreezeFrameDataPriMem(eventParam, freezeFrame);
			break;

		case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
		case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
		case DEM_DTC_ORIGIN_MIRROR_MEMORY:
			// Not yet supported
			DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GLOBAL_ID, DEM_E_NOT_IMPLEMENTED_YET);
			break;

		default:
			break;
		}
	}
}


/*
 * Procedure:	handlePreInitEvent
 * Description:	Handle the updating of event status and storing of
 * 				event related data in preInit buffers.
 */
static void handlePreInitEvent(Dem_EventIdType eventId, Dem_EventStatusType eventStatus)
{
	const Dem_EventParameterType *eventParam;
	EventStatusRecType eventStatusLocal;
	FreezeFrameRecType freezeFrameLocal;
	ExtDataRecType extendedDataLocal;

	// Find configuration for this event
	lookupEventIdParameter(eventId, &eventParam);
	if (eventParam != NULL) {
		if (eventParam->EventClass->OperationCycleRef < DEM_OPERATION_CYCLE_ID_ENDMARK) {
			if (operationCycleStateList[eventParam->EventClass->OperationCycleRef] == DEM_CYCLE_STATE_START) {
				if (eventStatus == DEM_EVENT_STATUS_FAILED) {
					updateEventStatusRec(eventParam, eventStatus, TRUE, &eventStatusLocal);
				}
				else {
					updateEventStatusRec(eventParam, eventStatus, FALSE, &eventStatusLocal);
				}

				if (eventStatusLocal.errorStatusChanged) {

					if (eventStatusLocal.eventStatusExtended & DEM_TEST_FAILED) {
						// Collect freeze frame data
						getFreezeFrameData(eventParam, &freezeFrameLocal);
						if (freezeFrameLocal.eventId != DEM_EVENT_ID_NULL) {
							storeFreezeFrameDataPreInit(eventParam, &freezeFrameLocal);
						}

						// Collect extended data
						getExtendedData(eventParam, &extendedDataLocal);
						if (extendedDataLocal.eventId != DEM_EVENT_ID_NULL) {
							storeExtendedDataPreInit(eventParam, &extendedDataLocal);
						}
					}
				}
			}
			else {
				// Operation cycle not started
				// TODO: Report error?
			}
		}
		else {
			// Operation cycle not set
			// TODO: Report error?
		}
	}
	else {
		// Event ID not configured
		// TODO: Report error?
	}
}


/*
 * Procedure:	handleEvent
 * Description:	Handle the updating of event status and storing of
 * 				event related data in event memory.
 */
static Std_ReturnType handleEvent(Dem_EventIdType eventId, Dem_EventStatusType eventStatus)
{
	Std_ReturnType returnCode = E_OK;
	const Dem_EventParameterType *eventParam;
	EventStatusRecType eventStatusLocal;
	FreezeFrameRecType freezeFrameLocal;
	ExtDataRecType extendedDataLocal;

	// Find configuration for this event
	lookupEventIdParameter(eventId, &eventParam);
	if (eventParam != NULL) {
		if (eventParam->EventClass->OperationCycleRef < DEM_OPERATION_CYCLE_ID_ENDMARK) {
			if (operationCycleStateList[eventParam->EventClass->OperationCycleRef] == DEM_CYCLE_STATE_START) {
				if ((!((disableDtcStorage.storageDisabled) && (checkDtcGroup(disableDtcStorage.dtcGroup, eventParam)) && (checkDtcKind(disableDtcStorage.dtcKind, eventParam)))))  {
					updateEventStatusRec(eventParam, eventStatus, TRUE, &eventStatusLocal);
					if (eventStatusLocal.errorStatusChanged) {
						if (eventStatusLocal.eventStatusExtended & DEM_TEST_FAILED) {
							storeEventEvtMem(eventParam, &eventStatusLocal); /** @req DEM184 */
							// Collect freeze frame data
							getFreezeFrameData(eventParam, &freezeFrameLocal);
							if (freezeFrameLocal.eventId != DEM_EVENT_ID_NULL) {
								storeFreezeFrameDataEvtMem(eventParam, &freezeFrameLocal); /** @req DEM190 */
							}

							// Collect extended data
							getExtendedData(eventParam, &extendedDataLocal);
							if (extendedDataLocal.eventId != DEM_EVENT_ID_NULL)
							{
								storeExtendedDataEvtMem(eventParam, &extendedDataLocal);
							}
						}
					}
				}
			}
			else {
				// Operation cycle not started
				returnCode = E_NOT_OK;
			}
		}
		else {
			// Operation cycle not set
			returnCode = E_NOT_OK;
		}
	}
	else {
		// Event ID not configured
		returnCode = E_NOT_OK;
	}

	return returnCode;
}


/*
 * Procedure:	resetEventStatus
 * Description:	Resets the events status of eventId.
 */
static void resetEventStatus(Dem_EventIdType eventId)
{
	imask_t state = McuE_EnterCriticalSection();
	EventStatusRecType *eventStatusRecPtr;

	lookupEventStatusRec(eventId, &eventStatusRecPtr);
	if (eventStatusRecPtr != NULL) {
		eventStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)~DEM_TEST_FAILED; /** @req DEM187 */
	}

	McuE_ExitCriticalSection(state);
}


/*
 * Procedure:	getEventStatus
 * Description:	Returns the extended event status bitmask of eventId in "eventStatusExtended".
 */
static void getEventStatus(Dem_EventIdType eventId, Dem_EventStatusExtendedType *eventStatusExtended)
{
	EventStatusRecType eventStatusLocal;

	// Get recorded status
	getEventStatusRec(eventId, &eventStatusLocal);
	if (eventStatusLocal.eventId == eventId) {
		*eventStatusExtended = eventStatusLocal.eventStatusExtended; /** @req DEM051 */
	}
	else {
		// Event Id not found, no report received.
		*eventStatusExtended = DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE | DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR;
	}
}


/*
 * Procedure:	getEventFailed
 * Description:	Returns the TRUE or FALSE of "eventId" in "eventFailed" depending on current status.
 */
static void getEventFailed(Dem_EventIdType eventId, boolean *eventFailed)
{
	EventStatusRecType eventStatusLocal;

	// Get recorded status
	getEventStatusRec(eventId, &eventStatusLocal);
	if (eventStatusLocal.eventId == eventId) {
		if (eventStatusLocal.eventStatusExtended & DEM_TEST_FAILED) { /** @req DEM052 */
			*eventFailed = TRUE;
		}
		else {
			*eventFailed = FALSE;
		}
	}
	else {
		// Event Id not found, assume ok.
		*eventFailed = FALSE;
	}
}


/*
 * Procedure:	getEventTested
 * Description:	Returns the TRUE or FALSE of "eventId" in "eventTested" depending on
 * 				current status the "test not completed this operation cycle" bit.
 */
static void getEventTested(Dem_EventIdType eventId, boolean *eventTested)
{
	EventStatusRecType eventStatusLocal;

	// Get recorded status
	getEventStatusRec(eventId, &eventStatusLocal);
	if (eventStatusLocal.eventId == eventId) {
		if ( !(eventStatusLocal.eventStatusExtended & DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE)) { /** @req DEM053 */
			*eventTested = TRUE;
		}
		else {
			*eventTested = FALSE;
		}
	}
	else {
		// Event Id not found, not tested.
		*eventTested = FALSE;
	}
}


/*
 * Procedure:	getFaultDetectionCounter
 * Description:	Returns pre debounce counter of "eventId" in "counter" and return value E_OK if
 * 				the counter was available else E_NOT_OK.
 */
static Std_ReturnType getFaultDetectionCounter(Dem_EventIdType eventId, sint8 *counter)
{
	Std_ReturnType returnCode = E_NOT_OK;
	const Dem_EventParameterType *eventParam;

	lookupEventIdParameter(eventId, &eventParam);
	if (eventParam != NULL) {
		if (eventParam->EventClass->PreDebounceAlgorithmClass != NULL) {
			switch (eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceName)
			{
			case DEM_NO_PRE_DEBOUNCE:
				if (eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceAlgorithm.PreDebounceMonitorInternal != NULL) {
					if (eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceAlgorithm.PreDebounceMonitorInternal->CallbackGetFDCntFnc != NULL) {
						returnCode = eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceAlgorithm.PreDebounceMonitorInternal->CallbackGetFDCntFnc(counter); /** @req DEM204.None */ /** @req DEM264 */ /** @req DEM265 */
					}
				}
				break;

			case DEM_PRE_DEBOUNCE_COUNTER_BASED:
				{
					EventStatusRecType *eventStatusRec;

					lookupEventStatusRec(eventId, &eventStatusRec);
					if (eventStatusRec != NULL) {
						*counter = eventStatusRec->faultDetectionCounter; /** @req DEM204.Counter */
					} else {
						*counter = 0;
					}
					returnCode = E_OK;
				}
				break;

			case DEM_PRE_DEBOUNCE_FREQUENCY_BASED:
			case DEM_PRE_DEBOUNCE_TIME_BASED:
				DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETFAULTDETECTIONCOUNTER_ID, DEM_E_NOT_IMPLEMENTED_YET);
				break;

			default:
				DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETFAULTDETECTIONCOUNTER_ID, DEM_E_PARAM_DATA);
				break;
			}
		}
	}

	return returnCode;
}


/*
 * Procedure:	setOperationCycleState
 * Description:	Change the operation state of "operationCycleId" to "cycleState" and updates stored
 * 				event connected to this cycle id.
 * 				Returns E_OK if operation was successful else E_NOT_OK.
 */
static Std_ReturnType setOperationCycleState(Dem_OperationCycleIdType operationCycleId, Dem_OperationCycleStateType cycleState) /** @req DEM338 */
{
	uint16 i;
	Std_ReturnType returnCode = E_OK;

	if (operationCycleId < DEM_OPERATION_CYCLE_ID_ENDMARK) {
		switch (cycleState)
		{
		case DEM_CYCLE_STATE_START:
			operationCycleStateList[operationCycleId] = cycleState;
			// Lookup event ID
			for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
				if ((eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL) && (eventStatusBuffer[i].eventParamRef->EventClass->OperationCycleRef == operationCycleId)) {
					eventStatusBuffer[i].eventStatusExtended &= (Dem_EventStatusExtendedType)~DEM_TEST_FAILED_THIS_OPERATION_CYCLE;
					eventStatusBuffer[i].eventStatusExtended |= DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE;
				}
			}
			break;

		case DEM_CYCLE_STATE_END:
			operationCycleStateList[operationCycleId] = cycleState;
			// Lookup event ID
			for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
				if ((eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL) && (eventStatusBuffer[i].eventParamRef->EventClass->OperationCycleRef == operationCycleId)) {
					if ((!(eventStatusBuffer[i].eventStatusExtended & DEM_TEST_FAILED_THIS_OPERATION_CYCLE)) && (!(eventStatusBuffer[i].eventStatusExtended & DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE))) {
						eventStatusBuffer[i].eventStatusExtended &= (Dem_EventStatusExtendedType)~DEM_PENDING_DTC;		// Clear pendingDTC bit /** @req DEM379.PendingClear
					}
				}
			}
			break;
		default:
			DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_SETOPERATIONCYCLESTATE_ID, DEM_E_PARAM_DATA);
			returnCode = E_NOT_OK;
			break;
		}
	}
	else {
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_SETOPERATIONCYCLESTATE_ID, DEM_E_PARAM_DATA);
		returnCode = E_NOT_OK;
		}

	return returnCode;
}


//==============================================================================//
//																				//
//					  E X T E R N A L   F U N C T I O N S						//
//																				//
//==============================================================================//

/*********************************************
 * Interface for upper layer modules (8.3.1) *
 *********************************************/

/*
 * Procedure:	Dem_GetVersionInfo
 * Reentrant:	Yes
 */
// Defined in Dem.h


/***********************************************
 * Interface ECU State Manager <-> DEM (8.3.2) *
 ***********************************************/

/*
 * Procedure:	Dem_PreInit
 * Reentrant:	No
 */
void Dem_PreInit(void)
{
	/** @req DEM180 */
	uint16 i, j;

	VALIDATE_NO_RV(DEM_Config.ConfigSet != NULL, DEM_PREINIT_ID, DEM_E_CONFIG_PTR_INVALID);

	configSet = DEM_Config.ConfigSet;

	// Initializion of operation cycle states.
	for (i = 0; i < DEM_OPERATION_CYCLE_ID_ENDMARK; i++) {
		operationCycleStateList[i] = DEM_CYCLE_STATE_END;
	}

	// Initialize the event status buffer
	for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
		eventStatusBuffer[i].eventId = DEM_EVENT_ID_NULL;
		eventStatusBuffer[i].eventParamRef = NULL;
		eventStatusBuffer[i].faultDetectionCounter = 0;
		eventStatusBuffer[i].occurrence = 0;
		eventStatusBuffer[i].eventStatusExtended = DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE | DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR;
		eventStatusBuffer[i].errorStatusChanged = FALSE;
	}

	// Initialize the pre init buffers
	for (i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRE_INIT; i++) {
		preInitFreezeFrameBuffer[i].checksum = 0;
		preInitFreezeFrameBuffer[i].eventId = DEM_EVENT_ID_NULL;
		preInitFreezeFrameBuffer[i].occurrence = 0;
		preInitFreezeFrameBuffer[i].dataSize = 0;
		for (j = 0; j < DEM_MAX_SIZE_FF_DATA;j++){
			preInitFreezeFrameBuffer[i].data[j] = 0;
		}
	}

	for (i = 0; i < DEM_MAX_NUMBER_EXT_DATA_PRE_INIT; i++) {
		preInitExtDataBuffer[i].checksum = 0;
		preInitExtDataBuffer[i].eventId = DEM_EVENT_ID_NULL;
		preInitExtDataBuffer[i].dataSize = 0;
		for (j = 0; j < DEM_MAX_SIZE_EXT_DATA;j++){
			preInitExtDataBuffer[i].data[j] = 0;
		}
	}

	disableDtcStorage.storageDisabled = FALSE;

	(void)setOperationCycleState(DEM_ACTIVE, DEM_CYCLE_STATE_START); /** @req DEM047 */

	demState = DEM_PREINITIALIZED;
}


/*
 * Procedure:	Dem_Init
 * Reentrant:	No
 */
void Dem_Init(void)
{
	uint16 i;
	ChecksumType cSum;
	const Dem_EventParameterType *eventParam;

	/*
	 *  Validate and read out saved error log from non volatile memory
	 */

	// Validate event records stored in primary memory
	for (i = 0; i < DEM_MAX_NUMBER_EVENT_PRI_MEM; i++) {
		cSum = calcChecksum(&priMemEventBuffer[i], sizeof(EventRecType)-sizeof(ChecksumType));
		if ((cSum != priMemEventBuffer[i].checksum) || (priMemEventBuffer[i].eventId == DEM_EVENT_ID_NULL)) {
			// Unlegal record, clear the record
			memset(&priMemEventBuffer[i], 0, sizeof(EventRecType));
		}
		else {
			// Valid, update current status
			mergeEventStatusRec(&priMemEventBuffer[i]);

			// Update occurrence counter on pre init stored freeze frames
			updateFreezeFrameOccurrencePreInit(&priMemEventBuffer[i]);
		}
	}

	// Validate extended data records stored in primary memory
	for (i = 0; i < DEM_MAX_NUMBER_EXT_DATA_PRI_MEM; i++) {
		cSum = calcChecksum(&priMemExtDataBuffer[i], sizeof(ExtDataRecType)-sizeof(ChecksumType));
		if ((cSum != priMemExtDataBuffer[i].checksum) || (priMemExtDataBuffer[i].eventId == DEM_EVENT_ID_NULL)) {
			// Unlegal record, clear the record
			memset(&priMemExtDataBuffer[i], 0, sizeof(ExtDataRecType));
		}
	}

	// Validate freeze frame records stored in primary memory
	for (i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM; i++) {
		cSum = calcChecksum(&priMemFreezeFrameBuffer[i], sizeof(FreezeFrameRecType)-sizeof(ChecksumType));
		if ((cSum != priMemFreezeFrameBuffer[i].checksum) || (priMemFreezeFrameBuffer[i].eventId == DEM_EVENT_ID_NULL)) {
			// Unlegal record, clear the record
			memset(&priMemFreezeFrameBuffer[i], 0, sizeof(FreezeFrameRecType));
		}
	}

	/*
	 *  Handle errors stored in temporary buffer (if any)
	 */

	// Transfer updated event data to event memory
	for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
		if (eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL) {
			// Update the event memory
			lookupEventIdParameter(eventStatusBuffer[i].eventId, &eventParam);
			storeEventEvtMem(eventParam, &eventStatusBuffer[i]);
		}
	}

	// Transfer extended data to event memory if necessary
	for (i = 0; i < DEM_MAX_NUMBER_EXT_DATA_PRE_INIT; i++) {
		if (preInitExtDataBuffer[i].eventId !=  DEM_EVENT_ID_NULL) {
			lookupEventIdParameter(preInitExtDataBuffer[i].eventId, &eventParam);
			storeExtendedDataEvtMem(eventParam, &preInitExtDataBuffer[i]);
		}
	}

	// Transfer freeze frames to event memory
	for (i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRE_INIT; i++) {
		if (preInitFreezeFrameBuffer[i].eventId != DEM_EVENT_ID_NULL) {
			lookupEventIdParameter(preInitFreezeFrameBuffer[i].eventId, &eventParam);
			storeFreezeFrameDataEvtMem(eventParam, &preInitFreezeFrameBuffer[i]);
		}
	}

	// Init the dtc filter
	dtcFilter.dtcStatusMask = DEM_DTC_STATUS_MASK_ALL;					// All allowed
	dtcFilter.dtcKind = DEM_DTC_KIND_ALL_DTCS;							// All kinds of DTCs
	dtcFilter.dtcOrigin = DEM_DTC_ORIGIN_PRIMARY_MEMORY;				// Primary memory
	dtcFilter.filterWithSeverity = DEM_FILTER_WITH_SEVERITY_NO;			// No Severity filtering
	dtcFilter.dtcSeverityMask = DEM_SEVERITY_NO_SEVERITY;				// Not used when filterWithSeverity is FALSE
	dtcFilter.filterForFaultDetectionCounter = DEM_FILTER_FOR_FDC_NO;	// No fault detection counter filtering

	dtcFilter.faultIndex = DEM_MAX_NUMBER_EVENT;

	disableDtcStorage.storageDisabled = FALSE;

	demState = DEM_INITIALIZED;
}


/*
 * Procedure:	Dem_shutdown
 * Reentrant:	No
 */
void Dem_Shutdown(void)
{
	(void)setOperationCycleState(DEM_ACTIVE, DEM_CYCLE_STATE_END); /** @req DEM047 */

	demState = DEM_UNINITIALIZED; /** @req DEM368 */
}


/*
 * Interface for basic software scheduler
 */
void Dem_MainFunction(void)
{
	/** @req DEM125 */

}


/***************************************************
 * Interface SW-Components via RTE <-> DEM (8.3.3) *
 ***************************************************/

/*
 * Procedure:	Dem_SetEventStatus
 * Reentrant:	Yes
 */
Std_ReturnType Dem_SetEventStatus(Dem_EventIdType eventId, Dem_EventStatusType eventStatus) /** @req DEM330 */
{
	Std_ReturnType returnCode = E_OK;

	if (demState == DEM_INITIALIZED) // No action is taken if the module is not started
	{
		returnCode = handleEvent(eventId, eventStatus);
	}
	else
	{
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_SETEVENTSTATUS_ID, DEM_E_UNINIT);
		returnCode = E_NOT_OK;
	}

	return returnCode;
}


/*
 * Procedure:	Dem_ResetEventStatus
 * Reentrant:	Yes
 */
Std_ReturnType Dem_ResetEventStatus(Dem_EventIdType eventId) /** @req DEM331 */
{
	Std_ReturnType returnCode = E_OK;

	if (demState == DEM_INITIALIZED) // No action is taken if the module is not started
	{
		resetEventStatus(eventId); /** @req DEM186 */
	}
	else
	{
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_RESETEVENTSTATUS_ID, DEM_E_UNINIT);
		returnCode = E_NOT_OK;
	}

	return returnCode;
}


/*
 * Procedure:	Dem_GetEventStatus
 * Reentrant:	Yes
 */
Std_ReturnType Dem_GetEventStatus(Dem_EventIdType eventId, Dem_EventStatusExtendedType *eventStatusExtended) /** @req DEM332 */
{
	Std_ReturnType returnCode = E_OK;

	if (demState == DEM_INITIALIZED) // No action is taken if the module is not started
	{
		getEventStatus(eventId, eventStatusExtended);
	}
	else
	{
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETEVENTSTATUS_ID, DEM_E_UNINIT);
		returnCode = E_NOT_OK;
	}

	return returnCode;
}


/*
 * Procedure:	Dem_GetEventFailed
 * Reentrant:	Yes
 */
Std_ReturnType Dem_GetEventFailed(Dem_EventIdType eventId, boolean *eventFailed) /** @req DEM333 */
{
	Std_ReturnType returnCode = E_OK;

	if (demState == DEM_INITIALIZED) // No action is taken if the module is not started
	{
		getEventFailed(eventId, eventFailed);
	}
	else
	{
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETEVENTFAILED_ID, DEM_E_UNINIT);
		returnCode = E_NOT_OK;
	}

	return returnCode;
}


/*
 * Procedure:	Dem_GetEventTested
 * Reentrant:	Yes
 */
Std_ReturnType Dem_GetEventTested(Dem_EventIdType eventId, boolean *eventTested)
{
	Std_ReturnType returnCode = E_OK;

	if (demState == DEM_INITIALIZED) // No action is taken if the module is not started
	{
		getEventTested(eventId, eventTested);
	}
	else
	{
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETEVENTTESTED_ID, DEM_E_UNINIT);
		returnCode = E_NOT_OK;
	}

	return returnCode;
}


/*
 * Procedure:	Dem_GetFaultDetectionCounter
 * Reentrant:	No
 */
Std_ReturnType Dem_GetFaultDetectionCounter(Dem_EventIdType eventId, sint8 *counter)
{
	Std_ReturnType returnCode = E_OK;

	if (demState == DEM_INITIALIZED) // No action is taken if the module is not started
	{
		returnCode = getFaultDetectionCounter(eventId, counter);
	}
	else
	{
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETFAULTDETECTIONCOUNTER_ID, DEM_E_UNINIT);
		returnCode = E_NOT_OK;
	}

	return returnCode;
}


/*
 * Procedure:	Dem_SetOperationCycleState
 * Reentrant:	No
 */
Std_ReturnType Dem_SetOperationCycleState(Dem_OperationCycleIdType operationCycleId, Dem_OperationCycleStateType cycleState)
{
	Std_ReturnType returnCode = E_OK;

	if (demState == DEM_INITIALIZED) // No action is taken if the module is not started
	{
		returnCode = setOperationCycleState(operationCycleId, cycleState);

	}
	else
	{
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_SETOPERATIONCYCLESTATE_ID, DEM_E_UNINIT);
		returnCode = E_NOT_OK;
	}

	return returnCode;
}


/*
 * Procedure:	Dem_GetDTCOfEvent
 * Reentrant:	Yes
 */
Std_ReturnType Dem_GetDTCOfEvent(Dem_EventIdType eventId, Dem_DTCKindType dtcKind, uint32* dtcOfEvent)
{
	Std_ReturnType returnCode = E_NO_DTC_AVAILABLE;
	const Dem_EventParameterType *eventParam;

	if (demState == DEM_INITIALIZED) // No action is taken if the module is not started
	{
		lookupEventIdParameter(eventId, &eventParam);

		if (eventParam != NULL) {
			if (checkDtcKind(dtcKind, eventParam)) {
				if (eventParam->DTCClassRef != NULL) {
					*dtcOfEvent = eventParam->DTCClassRef->DTC; /** @req DEM269 */
					returnCode = E_OK;
				}
			}
		}
		else {
			// Event Id not found
			returnCode = E_NOT_OK;
		}
	}
	else
	{
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETDTCOFEVENT_ID, DEM_UNINITIALIZED);
		returnCode = E_NOT_OK;
	}

	return returnCode;
}


/********************************************
 * Interface BSW-Components <-> DEM (8.3.4) *
 ********************************************/

/*
 * Procedure:	Dem_ReportErrorStatus
 * Reentrant:	Yes
 */
void Dem_ReportErrorStatus( Dem_EventIdType eventId, Dem_EventStatusType eventStatus ) /** @req DEM107 */
{

	switch (demState) {
		case DEM_PREINITIALIZED:
			// Update status and check if is to be stored
			if ((eventStatus == DEM_EVENT_STATUS_PASSED) || (eventStatus == DEM_EVENT_STATUS_FAILED)) {
				handlePreInitEvent(eventId, eventStatus); /** @req DEM168 */
			}
			break;

		case DEM_INITIALIZED:
			(void)handleEvent(eventId, eventStatus);	/** @req DEM167 */
			break;

		case DEM_UNINITIALIZED:
		default:
			// Uninitialized can not do anything
			DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_REPORTERRORSTATUS_ID, DEM_E_UNINIT);

			break;

	} // switch (demState)
}

/*********************************
 * Interface DCM <-> DEM (8.3.5) *
 *********************************/
/*
 * Procedure:	Dem_GetDTCStatusAvailabilityMask
 * Reentrant:	No
 */
Std_ReturnType Dem_GetDTCStatusAvailabilityMask(uint8 *dtcStatusMask) /** @req DEM014 */
{
	*dtcStatusMask = 	DEM_DTC_STATUS_AVAILABILITY_MASK;		// User configuration mask
	*dtcStatusMask &= 	DEM_TEST_FAILED							// Mask with supported bits /** @req DEM060 */
						| DEM_TEST_FAILED_THIS_OPERATION_CYCLE
						| DEM_PENDING_DTC
//						| DEM_CONFIRMED_DTC					TODO: Add support for this bit
						| DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR
						| DEM_TEST_FAILED_SINCE_LAST_CLEAR
						| DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE
//						| DEM_WARNING_INDICATOR_REQUESTED	TODO: Add support for this bit
						;

	return E_OK;
}


/*
 * Procedure:	Dem_SetDTCFilter
 * Reentrant:	No
 */
Dem_ReturnSetDTCFilterType Dem_SetDTCFilter(uint8 dtcStatusMask,
		Dem_DTCKindType dtcKind,
		Dem_DTCOriginType dtcOrigin,
		Dem_FilterWithSeverityType filterWithSeverity,
		Dem_DTCSeverityType dtcSeverityMask,
		Dem_FilterForFDCType filterForFaultDetectionCounter)
{
	Dem_ReturnSetDTCFilterType returnCode = DEM_FILTER_ACCEPTED;

	if (demState == DEM_INITIALIZED) {
		// Check dtcKind parameter
		VALIDATE_RV((dtcKind == DEM_DTC_KIND_ALL_DTCS) || (dtcKind == DEM_DTC_KIND_EMISSION_REL_DTCS), DEM_SETDTCFILTER_ID, DEM_E_PARAM_DATA, DEM_WRONG_FILTER);

		// Check dtcOrigin parameter
		VALIDATE_RV((dtcOrigin == DEM_DTC_ORIGIN_SECONDARY_MEMORY) || (dtcOrigin == DEM_DTC_ORIGIN_PRIMARY_MEMORY)
					|| (dtcOrigin == DEM_DTC_ORIGIN_PERMANENT_MEMORY) || (dtcOrigin == DEM_DTC_ORIGIN_MIRROR_MEMORY), DEM_SETDTCFILTER_ID, DEM_E_PARAM_DATA, DEM_WRONG_FILTER);

		// Check filterWithSeverity and dtcSeverityMask parameter
		VALIDATE_RV(((filterWithSeverity == DEM_FILTER_WITH_SEVERITY_NO)
					|| ((filterWithSeverity == DEM_FILTER_WITH_SEVERITY_YES)
						&& (!(dtcSeverityMask & (Dem_DTCSeverityType)~(DEM_SEVERITY_MAINTENANCE_ONLY | DEM_SEVERITY_CHECK_AT_NEXT_FALT | DEM_SEVERITY_CHECK_IMMEDIATELY))))), DEM_SETDTCFILTER_ID, DEM_E_PARAM_DATA, DEM_WRONG_FILTER);

		// Check filterForFaultDetectionCounter parameter
		VALIDATE_RV((filterForFaultDetectionCounter == DEM_FILTER_FOR_FDC_YES) || (filterForFaultDetectionCounter ==  DEM_FILTER_FOR_FDC_NO), DEM_SETDTCFILTER_ID, DEM_E_PARAM_DATA, DEM_WRONG_FILTER);

		// Yes all parameters correct, set the new filters.  /** @req DEM057 */
		dtcFilter.dtcStatusMask = dtcStatusMask;
		dtcFilter.dtcKind = dtcKind;
		dtcFilter.dtcOrigin = dtcOrigin;
		dtcFilter.filterWithSeverity = filterWithSeverity;
		dtcFilter.dtcSeverityMask = dtcSeverityMask;
		dtcFilter.filterForFaultDetectionCounter = filterForFaultDetectionCounter;
		dtcFilter.faultIndex = DEM_MAX_NUMBER_EVENT;
	} else {
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_SETDTCFILTER_ID, DEM_E_UNINIT);
		returnCode = DEM_WRONG_FILTER;
	}

	return returnCode;
}


/*
 * Procedure:	Dem_GetStatusOfDTC
 * Reentrant:	No
 */
Dem_ReturnGetStatusOfDTCType Dem_GetStatusOfDTC(uint32 dtc, Dem_DTCKindType dtcKind, Dem_DTCOriginType dtcOrigin, Dem_EventStatusExtendedType* status) {
	Dem_ReturnGetStatusOfDTCType returnCode = DEM_STATUS_FAILED;
	EventStatusRecType *eventRec;

	if (demState == DEM_INITIALIZED) {
		if (lookupDtcEvent(dtc, &eventRec)) {
			if (checkDtcKind(dtcKind, eventRec->eventParamRef)) {
				if (checkDtcOrigin(dtcOrigin,eventRec->eventParamRef)) {
					*status = eventRec->eventStatusExtended; /** @req DEM059 */
					returnCode = DEM_STATUS_OK;
				}
				else {
					returnCode = DEM_STATUS_WRONG_DTCORIGIN; /** @req DEM171 */
				}
			}
			else {
				returnCode = DEM_STATUS_WRONG_DTCKIND;
			}
		}
		else {
			returnCode = DEM_STATUS_WRONG_DTC; /** @req DEM172 */
		}
	} else {
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETSTATUSOFDTC_ID, DEM_E_UNINIT);
		returnCode = DEM_STATUS_FAILED;
	}

	return returnCode;
}


/*
 * Procedure:	Dem_GetNumberOfFilteredDtc
 * Reentrant:	No
 */
Dem_ReturnGetNumberOfFilteredDTCType Dem_GetNumberOfFilteredDtc(uint16 *numberOfFilteredDTC) {
	uint16 i;
	uint16 numberOfFaults = 0;
	Dem_ReturnGetNumberOfFilteredDTCType returnCode = DEM_NUMBER_OK;

	if (demState == DEM_INITIALIZED) {
		//Dem_DisableEventStatusUpdate();

		for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
			if (eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL) {
				if (matchEventWithDtcFilter(&eventStatusBuffer[i])) {
					if (eventStatusBuffer[i].eventParamRef->DTCClassRef != NULL) {
						numberOfFaults++;
					}
				}
			}
		}

		//Dem_EnableEventStatusUpdate();

		*numberOfFilteredDTC = numberOfFaults; /** @req DEM061 */
	} else {
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETNUMBEROFFILTEREDDTC_ID, DEM_E_UNINIT);
		returnCode = DEM_NUMBER_FAILED;
	}

	return returnCode;
}


/*
 * Procedure:	Dem_GetNextFilteredDTC
 * Reentrant:	No
 */
Dem_ReturnGetNextFilteredDTCType Dem_GetNextFilteredDTC(uint32 *dtc, Dem_EventStatusExtendedType *dtcStatus)
{
	Dem_ReturnGetNextFilteredDTCType returnCode = DEM_FILTERED_OK;
	boolean dtcFound = FALSE;

	if (demState == DEM_INITIALIZED) {
		// TODO: This job should be done in an more advanced way according to Dem217
		while ((!dtcFound) && (dtcFilter.faultIndex != 0)) {
			dtcFilter.faultIndex--;
			if (eventStatusBuffer[dtcFilter.faultIndex].eventId != DEM_EVENT_ID_NULL) {
				if (matchEventWithDtcFilter(&eventStatusBuffer[dtcFilter.faultIndex])) {
					if (eventStatusBuffer[dtcFilter.faultIndex].eventParamRef->DTCClassRef != NULL) {
						*dtc = eventStatusBuffer[dtcFilter.faultIndex].eventParamRef->DTCClassRef->DTC; /** @req DEM216 */
						*dtcStatus = eventStatusBuffer[dtcFilter.faultIndex].eventStatusExtended;
						dtcFound = TRUE;
					}
				}
			}
		}

		if (!dtcFound) {
			dtcFilter.faultIndex = DEM_MAX_NUMBER_EVENT;
			returnCode = DEM_FILTERED_NO_MATCHING_DTC;
		}
	} else {
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETNEXTFILTEREDDTC_ID, DEM_E_UNINIT);
		returnCode = DEM_FILTERED_NO_MATCHING_DTC;
	}

	return returnCode;
}


/*
 * Procedure:	Dem_GetTranslationType
 * Reentrant:	No
 */
Dem_ReturnTypeOfDtcSupportedType Dem_GetTranslationType(void)
{
	return DEM_TYPE_OF_DTC_SUPPORTED; /** @req DEM231 */
}


/*
 * Procedure:	Dem_ClearDTC
 * Reentrant:	No
 */
Dem_ReturnClearDTCType Dem_ClearDTC(uint32 dtc, Dem_DTCKindType dtcKind, Dem_DTCOriginType dtcOrigin) /** @req DEM009 */
{
	Dem_ReturnClearDTCType returnCode = DEM_CLEAR_OK;
	Dem_EventIdType eventId;
	const Dem_EventParameterType *eventParam;
	uint16 i, j;

	if (demState == DEM_INITIALIZED) {
		// Loop through the event buffer
		for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
			eventId = eventStatusBuffer[i].eventId;
			if (eventId != DEM_EVENT_ID_NULL) {
				eventParam = eventStatusBuffer[i].eventParamRef;
				if (eventParam != NULL) {
					//lint --e(506)	PC-Lint exception Misra 13.7, 14.1 Allow configuration variables in boolean expression
					//lint --e(774)	PC-Lint exception	Related to MISRA 13.7
					if ((DEM_CLEAR_ALL_EVENTS == STD_ON) || (eventParam->DTCClassRef != NULL)) {
						if (checkDtcKind(dtcKind, eventParam)) {
							if (checkDtcGroup(dtc, eventParam)) {
								boolean dtcOriginFound = FALSE;
								for (j = 0; (j < DEM_MAX_NR_OF_EVENT_DESTINATION) && (!dtcOriginFound) ; j++){
									dtcOriginFound =(eventParam->EventClass->EventDestination[j] == dtcOrigin);
								}
								//if (j-1 < DEM_MAX_NR_OF_EVENT_DESTINATION) {
								if (dtcOriginFound) {
									// Yes! All conditions met.
									switch (dtcOrigin)
									{
									case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
										/** @req DEM077 */
										deleteEventPriMem(eventParam);
										deleteFreezeFrameDataPriMem(eventParam);
										deleteExtendedDataPriMem(eventParam);
										deleteEventStatusRec(eventParam);		// TODO: Shall this be done or just resetting the status?
										break;

									case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
									case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
									case DEM_DTC_ORIGIN_MIRROR_MEMORY:
										// Not yet supported
										returnCode = DEM_CLEAR_WRONG_DTCORIGIN;
										DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_CLEARDTC_ID, DEM_E_NOT_IMPLEMENTED_YET);
										break;
									default:
										returnCode = DEM_CLEAR_WRONG_DTCORIGIN;
										break;
									}
								}
							}
						}
					}
				}
				else {
					// Fatal error, no event parameters found for the stored event!
					DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_CLEARDTC_ID, DEM_E_UNEXPECTED_EXECUTION);
				}
			}
		}
	} else {
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_CLEARDTC_ID, DEM_E_UNINIT);
		returnCode = DEM_CLEAR_FAILED;
	}

	return returnCode;
}


/*
 * Procedure:	Dem_DisableDTCStorage
 * Reentrant:	No
 */
Dem_ReturnControlDTCStorageType Dem_DisableDTCStorage(Dem_DTCGroupType dtcGroup, Dem_DTCKindType dtcKind) /** @req DEM035 */
{
	Dem_ReturnControlDTCStorageType returnCode = DEM_CONTROL_DTC_STORAGE_OK;

	if (demState == DEM_INITIALIZED) {
		// Check dtcGroup parameter
		if (dtcGroup == DEM_DTC_GROUP_ALL_DTCS) {
			// Check dtcKind parameter
			if ((dtcKind == DEM_DTC_KIND_ALL_DTCS) || (dtcKind ==  DEM_DTC_KIND_EMISSION_REL_DTCS)) {
				/** @req DEM079 */
				disableDtcStorage.dtcGroup = dtcGroup;
				disableDtcStorage.dtcKind = dtcKind;
				disableDtcStorage.storageDisabled = TRUE;
			} else {
				returnCode = DEM_CONTROL_DTC_STORAGE_N_OK;
			}
		} else {
			returnCode = DEM_CONTROL_DTC_WRONG_DTCGROUP;
		}
	} else {
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_DISABLEDTCSTORAGE_ID, DEM_E_UNINIT);
		returnCode = DEM_CONTROL_DTC_STORAGE_N_OK;
	}

	return returnCode;
}


/*
 * Procedure:	Dem_EnableDTCStorage
 * Reentrant:	No
 */
Dem_ReturnControlDTCStorageType Dem_EnableDTCStorage(Dem_DTCGroupType dtcGroup, Dem_DTCKindType dtcKind)
{
	Dem_ReturnControlDTCStorageType returnCode = DEM_CONTROL_DTC_STORAGE_OK;

	if (demState == DEM_INITIALIZED) {
		// TODO: Behavior is not defined if group or kind do not match active settings, therefore the filter is just switched off.
		(void)dtcGroup; (void)dtcKind;	// Just to make get rid of PC-Lint warnings
		disableDtcStorage.storageDisabled = FALSE; /** @req DEM080 */
	} else {
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_ENABLEDTCSTORAGE_ID, DEM_E_UNINIT);
		returnCode = DEM_CONTROL_DTC_STORAGE_N_OK;
	}

	return returnCode;
}

/*
 * Procedure:	Dem_GetExtendedDataRecordByDTC
 * Reentrant:	No
 */
Dem_ReturnGetExtendedDataRecordByDTCType Dem_GetExtendedDataRecordByDTC(uint32 dtc, Dem_DTCKindType dtcKind, Dem_DTCOriginType dtcOrigin, uint8 extendedDataNumber, uint8 *destBuffer, uint16 *bufSize)
{
	Dem_ReturnGetExtendedDataRecordByDTCType returnCode = DEM_RECORD_WRONG_DTC;
	EventStatusRecType *eventRec;
	Dem_ExtendedDataRecordClassType const *extendedDataRecordClass = NULL;
	ExtDataRecType *extData;
	uint16 posInExtData = 0;

	if (demState == DEM_INITIALIZED) {
		if (lookupDtcEvent(dtc, &eventRec)) {
			if (checkDtcKind(dtcKind, eventRec->eventParamRef)) {
				if (checkDtcOrigin(dtcOrigin, eventRec->eventParamRef)) {
					if (lookupExtendedDataRecNumParam(extendedDataNumber, eventRec->eventParamRef, &extendedDataRecordClass, &posInExtData)) {
						if (*bufSize >= extendedDataRecordClass->DataSize) {
							switch (dtcOrigin)
							{
							case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
								if (lookupExtendedDataPriMem(eventRec->eventId, &extData)) {
									// Yes all conditions met, copy the extended data record to destination buffer.
									memcpy(destBuffer, &extData->data[posInExtData], extendedDataRecordClass->DataSize); /** @req DEM075 */
									*bufSize = extendedDataRecordClass->DataSize;
									returnCode = DEM_RECORD_OK;
								}
								else {
									// The record number is legal but no record was found for the DTC
									*bufSize = 0;
									returnCode = DEM_RECORD_OK;
								}
								break;

							case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
							case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
							case DEM_DTC_ORIGIN_MIRROR_MEMORY:
								// Not yet supported
								returnCode = DEM_RECORD_WRONG_DTCORIGIN;
								DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETEXTENDEDDATARECORDBYDTC_ID, DEM_E_NOT_IMPLEMENTED_YET);
								break;
							default:
								returnCode = DEM_RECORD_WRONG_DTCORIGIN;
								break;
							}
						}
						else {
							returnCode = DEM_RECORD_BUFFERSIZE;
						}
					}
					else {
						returnCode = DEM_RECORD_NUMBER;
					}
				}
				else {
					returnCode = DEM_RECORD_WRONG_DTCORIGIN;
				}
			}
			else {
				returnCode = DEM_RECORD_DTCKIND;
			}
		}
	} else {
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETEXTENDEDDATARECORDBYDTC_ID, DEM_E_UNINIT);
		returnCode = DEM_RECORD_WRONG_DTC;
	}

	return returnCode;
}


/*
 * Procedure:	Dem_GetSizeOfExtendedDataRecordByDTC
 * Reentrant:	No
 */
Dem_ReturnGetSizeOfExtendedDataRecordByDTCType Dem_GetSizeOfExtendedDataRecordByDTC(uint32 dtc, Dem_DTCKindType dtcKind, Dem_DTCOriginType dtcOrigin, uint8 extendedDataNumber, uint16 *sizeOfExtendedDataRecord)
{
	Dem_ReturnGetExtendedDataRecordByDTCType returnCode = DEM_GET_SIZEOFEDRBYDTC_W_DTC;
	EventStatusRecType *eventRec;
	Dem_ExtendedDataRecordClassType const *extendedDataRecordClass = NULL;
	uint16 posInExtData;

	if (demState == DEM_INITIALIZED) {
		if (lookupDtcEvent(dtc, &eventRec)) {
			if (checkDtcKind(dtcKind, eventRec->eventParamRef)) {
				if (checkDtcOrigin(dtcOrigin, eventRec->eventParamRef)) {
					if (lookupExtendedDataRecNumParam(extendedDataNumber, eventRec->eventParamRef, &extendedDataRecordClass, &posInExtData)) {
						*sizeOfExtendedDataRecord = extendedDataRecordClass->DataSize; /** @req DEM076 */
						returnCode = DEM_GET_SIZEOFEDRBYDTC_OK;
					}
					else {
						returnCode = DEM_GET_SIZEOFEDRBYDTC_W_RNUM;
					}
				}
				else {
					returnCode = DEM_GET_SIZEOFEDRBYDTC_W_DTCOR;
				}
			}
			else {
				returnCode = DEM_GET_SIZEOFEDRBYDTC_W_DTCKI;
			}
		}
	} else {
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETSIZEOFEXTENDEDDATARECORDBYDTC_ID, DEM_E_UNINIT);
		returnCode = DEM_GET_SIZEOFEDRBYDTC_W_DTC;
	}

	return returnCode;
}

/***********************************
 * OBD-specific Interfaces (8.3.6) *
 ***********************************/



