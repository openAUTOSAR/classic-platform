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

// PC-Lint misunderstanding of MISRA 8.12, 18.1: Empty arrays are OK as long as size 0 is specified.
//lint -esym(85,preInitFreezeFrameBuffer)
//lint -esym(85,priMemFreezeFrameBuffer)


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
#include "NvM.h"
//#include "SchM_Dem.h"
#include "MemMap.h"
#include "Cpu.h"
#include "Dem_Types.h"
#include "Dem_Lcfg.h"

#define USE_DEBUG_PRINTF
#include "debug.h"
/*
 * Local defines
 */
#define DEBOUNCE_FDC_TEST_FAILED  127
#define DEBOUNCE_FDC_TEST_PASSED -128

#define DEM_PID_IDENTIFIER_SIZE_OF_BYTES		1 // OBD 

#define DEM_CONFIRMATION_CNTR_MAX (uint8)255

#if  ( DEM_DEV_ERROR_DETECT == STD_ON )
#if defined(USE_DET)
#include "Det.h"
#endif
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

#if !(DEM_TYPE_OF_DTC_SUPPORTED == DEM_ISO15031_6 || DEM_TYPE_OF_DTC_SUPPORTED == DEM_ISO14229_1)
#error "DEM_TYPE_OF_DTC_SUPPORTED is not set to ISO15031-6 or ISO14229-1. Only these are supported by the code."
#endif


/*
 * Local types
 */
#if !defined(USE_DCM)
typedef uint8 Dcm_NegativeResponseCodeType;
#define DCM_E_POSITIVERESPONSE ((Dcm_NegativeResponseCodeType)0x00)
#endif

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
	sint8						maxFaultDetectionCounter;
	uint16						occurrence;				/** @req DEM011 */
	Dem_EventStatusExtendedType	eventStatusExtended;	/** @req DEM006 */
	boolean						errorStatusChanged;
	uint8						confirmationCounter;
} EventStatusRecType;

// Types for storing different event data on event memory
typedef struct {
	Dem_EventIdType				eventId;
	uint16						occurrence;
	Dem_EventStatusExtendedType eventStatusExtended;
	uint8						confirmationCounter;
	ChecksumType				checksum;
} EventRecType;

typedef struct {
	Dem_EventIdType		eventId;
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
EventRecType 		        priMemEventBuffer[DEM_MAX_NUMBER_EVENT_PRI_MEM];
static FreezeFrameRecType	priMemFreezeFrameBuffer[DEM_MAX_NUMBER_FF_DATA_PRI_MEM];
//FreezeFrameRecType        FreezeFrameMirrorBuffer[DEM_MAX_NUMBER_FF_DATA_PRI_MEM];
extern FreezeFrameRecType*  FreezeFrameMirrorBuffer[];
ExtDataRecType		priMemExtDataBuffer[DEM_MAX_NUMBER_EXT_DATA_PRI_MEM];
HealingRecType         		priMemAgingBuffer[DEM_MAX_NUMBER_AGING_PRI_MEM];
extern HealingRecType   		HealingMirrorBuffer[DEM_MAX_NUMBER_AGING_PRI_MEM];

/* block in NVRam, use for freezeframe */
extern const NvM_BlockIdType FreezeFrameBlockId[DEM_MAX_NUMBER_FF_DATA_PRI_MEM];
/* block in NVRam, use for aging */
extern const NvM_BlockIdType HealingBlockId;


/*
*Allocation of freezeFrame storage timestamp,record the time order
*/
/**private variable for freezeframe */
static uint32 FF_TimeStamp = 0;

/*
 * TRUE:	priMemFreezeFrameBuffer changed,
 * FALSE:	priMemFreezeFrameBuffer not changed
 */
static boolean FFIsModified = FALSE;

/*
 * TRUE:	priMemAgingBuffer changed,
 * FALSE:	priMemAgingBuffer not changed
 */
static boolean AgingIsModified = FALSE;

/* Index for keeping track of ffRecordFilter */
static uint16 FFRecordFilterIndex;


static void getPidData(const Dem_PidOrDidType ** const *pidClassPtr, FreezeFrameRecType **freezeFrame, uint16 *storeIndexPtr);
static void getDidData(const Dem_PidOrDidType ** const *didClassPtr, FreezeFrameRecType **freezeFrame, uint16 *storeIndexPtr);
static void storeOBDFreezeFrameDataPreInit(const Dem_EventParameterType * eventParam, const FreezeFrameRecType * freezeFrame);
static void storeOBDFreezeFrameDataPriMem(const Dem_EventParameterType *eventParam, const FreezeFrameRecType *freezeFrame);

/*
 * Procedure:	zeroPriMemBuffers
 * Description:	Fill the primary buffers with zeroes
 */
//lint -save
//lint -e84 //PC-Lint exception, size 0 is OK.
//lint -e957	PC-Lint exception - Used only by DemTest
void demZeroPriMemBuffers(void)
{
	memset(priMemEventBuffer, 0, sizeof(priMemEventBuffer));
	memset(priMemFreezeFrameBuffer, 0, sizeof(priMemFreezeFrameBuffer));
	memset(priMemExtDataBuffer, 0, sizeof(priMemExtDataBuffer));
}
//lint -restore

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

	/* Lookup the correct event id parameters */
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
 * Procedure:	checkEntryValid
 * Description:	Returns whether event id "eventId" is a valid entry in primary memory
 */
static boolean checkEntryValid(Dem_EventIdType eventId){
	const Dem_EventParameterType *EventIdParamList = configSet->EventParameter;
	boolean isValid = FALSE;
	uint16 i=0;
	while ((EventIdParamList[i].EventID != eventId) && (!EventIdParamList[i].Arc_EOL)) {
		i++;
	}

	if (!EventIdParamList[i].Arc_EOL) {
		// Event was found
		uint16 index = 0;
		for (index = 0; (index < DEM_MAX_NR_OF_EVENT_DESTINATION)
					 && (EventIdParamList[i].EventClass->EventDestination[index] != DEM_EVENT_DESTINATION_END_OF_LIST); index++) {
			if( DEM_DTC_ORIGIN_PRIMARY_MEMORY == EventIdParamList[i].EventClass->EventDestination[index]){
				// Event should be stored in primary memory.
				isValid = TRUE;
			}
		}

	} else {
		// The event did not exist
	}
	return isValid;
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

	statusRecord->maxFaultDetectionCounter = MAX(statusRecord->maxFaultDetectionCounter, statusRecord->faultDetectionCounter);
	return returnCode;
}
static boolean faultConfirmationCriteriaFulfilled(const Dem_EventParameterType *eventParam, const EventStatusRecType *eventStatusRecPtr)
{
	if( eventStatusRecPtr->confirmationCounter >= eventParam->EventClass->ConfirmationCycleCounterThreshold ) {
		return TRUE;
	} else {
		return FALSE;
	}
}
static void handleFaultConfirmation(const Dem_EventParameterType *eventParam, EventStatusRecType *eventStatusRecPtr)
{
	if( eventStatusRecPtr->confirmationCounter < DEM_CONFIRMATION_CNTR_MAX ) {
		eventStatusRecPtr->confirmationCounter++;
	}
	if( faultConfirmationCriteriaFulfilled(eventParam, eventStatusRecPtr) ) {
		eventStatusRecPtr->eventStatusExtended |= DEM_CONFIRMED_DTC;
	}
}

/*
 * Procedure:	updateEventStatusRec
 * Description:	Update the status of "eventId", if not exist and "createIfNotExist" is
 * 				true a new record is created
 */
static void updateEventStatusRec(const Dem_EventParameterType *eventParam, Dem_EventStatusType eventStatus, boolean createIfNotExist, EventStatusRecType *eventStatusRec)
{
	EventStatusRecType *eventStatusRecPtr;
	sint8 faultCounterBeforeDebounce = 0;
	sint8 faultCounterAfterDebounce = 0;
	imask_t state;
    Irq_Save(state);

	lookupEventStatusRec(eventParam->EventID, &eventStatusRecPtr);

	if ((eventStatusRecPtr == NULL) && (createIfNotExist)) {
		lookupEventStatusRec(DEM_EVENT_ID_NULL, &eventStatusRecPtr);
		if (eventStatusRecPtr != NULL) {
			eventStatusRecPtr->eventId = eventParam->EventID;
			eventStatusRecPtr->eventParamRef = eventParam;
			eventStatusRecPtr->faultDetectionCounter = 0;
			eventStatusRecPtr->maxFaultDetectionCounter = 0;
			eventStatusRecPtr->occurrence = 0;
			eventStatusRecPtr->eventStatusExtended = DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR | DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE;
			eventStatusRecPtr->errorStatusChanged = FALSE;
			eventStatusRecPtr->confirmationCounter = 0;
		}
		else {
			DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_UPDATE_EVENT_STATUS_ID, DEM_E_EVENT_STATUS_BUFF_FULL);
		}
	}


	if (eventStatusRecPtr != NULL) {
		faultCounterBeforeDebounce = eventStatusRecPtr->faultDetectionCounter;

		if (eventParam->EventClass->PreDebounceAlgorithmClass != NULL) {
			switch (eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceName) { /** @req DEM004 */ /** @req DEM342 */
			case DEM_NO_PRE_DEBOUNCE:
				eventStatus = preDebounceNone(eventStatus, eventStatusRecPtr);
				break;

			case DEM_PRE_DEBOUNCE_COUNTER_BASED:
				eventStatus = preDebounceCounterBased(eventStatus, eventStatusRecPtr);
				break;

			default:
				DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_UPDATE_EVENT_STATUS_ID, DEM_E_NOT_IMPLEMENTED_YET);
				break;
			}
		}
		faultCounterAfterDebounce = eventStatusRecPtr->faultDetectionCounter;

		eventStatusRecPtr->errorStatusChanged = FALSE;

		if (eventStatus == DEM_EVENT_STATUS_FAILED) {
			if (!(eventStatusRecPtr->eventStatusExtended & DEM_TEST_FAILED)) {
				eventStatusRecPtr->occurrence++;
				eventStatusRecPtr->errorStatusChanged = TRUE;
				if( !(eventStatusRecPtr->eventStatusExtended & DEM_TEST_FAILED_THIS_OPERATION_CYCLE) ) {
					/* First fail this operation cycle */
					handleFaultConfirmation(eventParam, eventStatusRecPtr);
				}
			}
			/** @req DEM036 */ /** @req DEM379.PendingSet */
			eventStatusRecPtr->eventStatusExtended |= (DEM_TEST_FAILED | DEM_TEST_FAILED_THIS_OPERATION_CYCLE | DEM_TEST_FAILED_SINCE_LAST_CLEAR | DEM_PENDING_DTC);
			eventStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)~(DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR | DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE);
			if ((eventParam->DTCClassRef != NULL) && (eventParam->DTCClassRef->DTCKind == DEM_DTC_KIND_EMISSION_REL_DTCS))
					eventStatusRecPtr->eventStatusExtended |= DEM_WARNING_INDICATOR_REQUESTED;
		}

		if (eventStatus == DEM_EVENT_STATUS_PASSED) {
			if (eventStatusRecPtr->eventStatusExtended & (DEM_TEST_FAILED | DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE)) {
				eventStatusRecPtr->errorStatusChanged = TRUE;
			}
			/** @req DEM036 */
			eventStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)~DEM_TEST_FAILED;
			eventStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)~(DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR | DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE);
			if ((eventParam->DTCClassRef != NULL) && (eventParam->DTCClassRef->DTCKind == DEM_DTC_KIND_EMISSION_REL_DTCS))
				eventStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)~DEM_WARNING_INDICATOR_REQUESTED;
		}

		if ((eventStatus == DEM_EVENT_STATUS_PREFAILED)\
			&& (faultCounterBeforeDebounce <= 0) && (faultCounterAfterDebounce > 0)){
			eventStatusRecPtr->errorStatusChanged = TRUE;
		}

		eventStatusRec->maxFaultDetectionCounter = MAX(eventStatusRec->maxFaultDetectionCounter, eventStatusRec->faultDetectionCounter);
		memcpy(eventStatusRec, eventStatusRecPtr, sizeof(EventStatusRecType));
	}
	else {
		eventStatusRec->eventId = DEM_EVENT_ID_NULL;
		eventStatusRec->faultDetectionCounter = 0;
		eventStatusRec->maxFaultDetectionCounter = 0;
		eventStatusRec->occurrence = 0;
		eventStatusRec->eventStatusExtended = DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE | DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR;
		eventStatusRec->errorStatusChanged = FALSE;
		eventStatusRec->confirmationCounter = 0;
	}

    Irq_Restore(state);
}


/*
 * Procedure:	mergeEventStatusRec
 * Description:	Update the occurrence counter of status, if not exist a new record is created
 */
static void mergeEventStatusRec(const EventRecType *eventRec)
{
	EventStatusRecType *eventStatusRecPtr;
	const Dem_EventParameterType *eventParam;
	imask_t state;
    Irq_Save(state);

	// Lookup event ID
	lookupEventStatusRec(eventRec->eventId, &eventStatusRecPtr);
	lookupEventIdParameter(eventRec->eventId, &eventParam);
	if (eventStatusRecPtr != NULL) {
		// Update occurrence counter.
		eventStatusRecPtr->occurrence += eventRec->occurrence;
		// Merge event status extended with stored
		// TEST_FAILED_SINCE_LAST_CLEAR should be set if set if set in either
		eventStatusRecPtr->eventStatusExtended |= (Dem_EventStatusExtendedType)(eventRec->eventStatusExtended & DEM_TEST_FAILED_SINCE_LAST_CLEAR);
		// DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR should cleared if cleared in either
		eventStatusRecPtr->eventStatusExtended |= (Dem_EventStatusExtendedType)(eventRec->eventStatusExtended & eventStatusRecPtr->eventStatusExtended & DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR);
		// DEM_PENDING_DTC and DEM_CONFIRMED_DTC should be set if set in either
		eventStatusRecPtr->eventStatusExtended |= (Dem_EventStatusExtendedType)(eventRec->eventStatusExtended & (DEM_PENDING_DTC | DEM_CONFIRMED_DTC));
		// DEM_WARNING_INDICATOR_REQUESTED should be set if set in either
		eventStatusRecPtr->eventStatusExtended |= (Dem_EventStatusExtendedType)(eventRec->eventStatusExtended & DEM_WARNING_INDICATOR_REQUESTED);
        if( (DEM_CONFIRMATION_CNTR_MAX - eventRec->confirmationCounter) < eventStatusRecPtr->confirmationCounter) {
            /* Would overflow */
            eventStatusRecPtr->confirmationCounter = DEM_CONFIRMATION_CNTR_MAX;
        } else {
            eventStatusRecPtr->confirmationCounter += eventRec->confirmationCounter;
        }
        if( (NULL != eventParam) && faultConfirmationCriteriaFulfilled(eventParam, eventStatusRecPtr) ) {
        	eventStatusRecPtr->eventStatusExtended |= (Dem_EventStatusExtendedType)DEM_CONFIRMED_DTC;
        }
	}

    Irq_Restore(state);
}

/*
 * Procedure:	resetEventStatusRec
 * Description:	Reset the status record of "eventParam->eventId" from "eventStatusBuffer".
 */
static void resetEventStatusRec(const Dem_EventParameterType *eventParam)
{
	EventStatusRecType *eventStatusRecPtr;
	imask_t state;
	Irq_Save(state);

	// Lookup event ID
	lookupEventStatusRec(eventParam->EventID, &eventStatusRecPtr);

	if (eventStatusRecPtr != NULL) {
		// Reset event record
		eventStatusRecPtr->faultDetectionCounter = 0;
		eventStatusRecPtr->maxFaultDetectionCounter = 0;
		eventStatusRecPtr->eventStatusExtended = (DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE | DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR);
		eventStatusRecPtr->errorStatusChanged = FALSE;
		eventStatusRecPtr->occurrence = 0;
		eventStatusRecPtr->confirmationCounter = 0;
	}

	Irq_Restore(state);
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

static void getInternalElement( const Dem_EventParameterType *eventParameter, Dem_InternalDataElementType elementType, uint8_t* buf )
{
	EventStatusRecType eventStatusRec;

	getEventStatusRec(eventParameter->EventID, &eventStatusRec);

	if( DEM_EVENT_ID_NULL != eventStatusRec.eventId ) {
		if( DEM_OCCCTR == elementType ) {
			buf[0] = eventStatusRec.occurrence >> 8;
			buf[1] = eventStatusRec.occurrence;
		}
		else if( DEM_FAULTDETCTR == elementType ) {
			buf[0] = eventStatusRec.faultDetectionCounter;
		}
		else if( DEM_MAXFAULTDETCTR == elementType ) {
			buf[0] = eventStatusRec.maxFaultDetectionCounter;
		}
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
/*
 * Procedure:	bubbleSort
 * Description:	bubble sort
 */
static void bubbleSort(FreezeFrameRecType *freezeFrameBuf, uint16 length)
{
	FreezeFrameRecType temp;
	uint16 i = 0;
	uint16 j = 0;

	//Bubble sort:rearrange priMemFreezeFrameBuffer from little to big
	for(i=0;i<length;i++){
		for(j=length-1; j > i; j--){
			if(freezeFrameBuf[i].timeStamp > freezeFrameBuf[j].timeStamp){
				//exchange buffer data
				memcpy(&temp,&freezeFrameBuf[i],sizeof(FreezeFrameRecType));
				memcpy(&freezeFrameBuf[i],&freezeFrameBuf[j],sizeof(FreezeFrameRecType));
				memcpy(&freezeFrameBuf[j],&temp,sizeof(FreezeFrameRecType));
			}
		}
	}

}

/*
 * Procedure:	retrieveEventStatusBit
 * Description:	retrieve Event Status Bit
 */
static boolean retrieveEventStatusBit(FreezeFrameRecType *freezeFrameBuf,
											uint16 length ,
											Dem_EventStatusExtendedType nBit,
											FreezeFrameRecType **freezeFrame)
{
	boolean freezeFrameFound = FALSE;
	uint16 i = 0;
	uint16 j = 0;

	for(i=0;i<length;i++){
		if(freezeFrameBuf[i].kind != DEM_FREEZE_FRAME_OBD) {
			for(j=0; (j < DEM_MAX_NUMBER_EVENT) && (!freezeFrameFound); j++){
				freezeFrameFound = (eventStatusBuffer[j].eventId == freezeFrameBuf[i].eventId)\
										 && (!(eventStatusBuffer[j].eventStatusExtended & nBit));
				if(freezeFrameFound == TRUE){
					*freezeFrame = &freezeFrameBuf[i];
				}
			}
		}
	}

	return freezeFrameFound;

}

/*
 * Procedure:	lookupFreezeFrameForDisplacementPreInit
 * Description:	implement displacement strategy preinit:1.find out the oldest "not confirmed" DTC
 * 												   2.find out the oldest inactive DTC,inactive:testFailed is not set
 *											          3.find ou the oldest active DTC,active:testFailed is set
 */
static boolean lookupFreezeFrameForDisplacementPreInit(FreezeFrameRecType **freezeFrame)
{
	boolean freezeFrameFound = FALSE;

	/* Bubble sort:rearrange priMemFreezeFrameBuffer from little to big */
	bubbleSort(preInitFreezeFrameBuffer, DEM_MAX_NUMBER_FF_DATA_PRE_INIT);

	/* find out the oldest not confirmed dtc */
	freezeFrameFound = retrieveEventStatusBit(preInitFreezeFrameBuffer, DEM_MAX_NUMBER_FF_DATA_PRE_INIT, DEM_CONFIRMED_DTC, freezeFrame);

	/* if all confirmed,lookup the oldest inactive dtc */
	if(freezeFrameFound == FALSE){
		freezeFrameFound = retrieveEventStatusBit(preInitFreezeFrameBuffer, DEM_MAX_NUMBER_FF_DATA_PRE_INIT, DEM_TEST_FAILED, freezeFrame);
	}

	/* if all confirmed,lookup the oldest active dtc */
	if(freezeFrameFound == FALSE){
		for(uint16 i = 0; (i < DEM_MAX_NUMBER_FF_DATA_PRE_INIT) && (freezeFrameFound == FALSE); i++){
			//prevent the displacement of OBD FF
			if(preInitFreezeFrameBuffer[i].kind != DEM_FREEZE_FRAME_OBD) {
				*freezeFrame = &preInitFreezeFrameBuffer[i];
				freezeFrameFound = TRUE;
			}
		}	
	}

	return freezeFrameFound;
}

/*
 * Procedure:	lookupFreezeFrameForDisplacement
 * Description:	implement displacement strategy:1.find out the oldest "not confirmed" DTC
 * 											2.find out the oldest inactive DTC,inactive:testFailed is not set
 *											3.find ou the oldest active DTC,active:testFailed is set
 */
static boolean lookupFreezeFrameForDisplacement(FreezeFrameRecType **freezeFrame)
{
	boolean freezeFrameFound = FALSE;

	bubbleSort(priMemFreezeFrameBuffer, DEM_MAX_NUMBER_FF_DATA_PRI_MEM);

	/* Find out the oldest not confirmed dtc */
	freezeFrameFound = retrieveEventStatusBit(priMemFreezeFrameBuffer, DEM_MAX_NUMBER_FF_DATA_PRI_MEM, DEM_CONFIRMED_DTC, freezeFrame);

	/* If all confirmed, lookup the oldest inactive dtc */
	if(freezeFrameFound == FALSE){
		freezeFrameFound = retrieveEventStatusBit(priMemFreezeFrameBuffer, DEM_MAX_NUMBER_FF_DATA_PRI_MEM, DEM_TEST_FAILED, freezeFrame);
	}

	/* If all confirmed,lookup the oldest active dtc */
	if(freezeFrameFound == FALSE){
		for(uint16 i = 0; (i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM) && (freezeFrameFound == FALSE); i++){
			//prevent the displacement of OBD FF
			if(priMemFreezeFrameBuffer[i].kind != DEM_FREEZE_FRAME_OBD) {
				*freezeFrame = &priMemFreezeFrameBuffer[i];
				freezeFrameFound = TRUE;
			}
		}		
	}

	return freezeFrameFound;
}
/*
 * Procedure:	rearrangeFreezeFrameTimeStamp
 * Description:	rearrange FF timestamp when timestamp is beyond DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT			
 */
static void rearrangeFreezeFrameTimeStamp(uint32 *timeStamp)
{
	FreezeFrameRecType temp;
	uint32 i = 0;
	uint32 j = 0;
	uint32 k = 0;

	/* Bubble sort:rearrange priMemFreezeFrameBuffer from little to big */
	for(i=0;i<DEM_MAX_NUMBER_FF_DATA_PRI_MEM;i++){
		if(priMemFreezeFrameBuffer[i].eventId != DEM_EVENT_ID_NULL){
			for(j=DEM_MAX_NUMBER_FF_DATA_PRI_MEM-1; j > i; j--){
				if(priMemFreezeFrameBuffer[j].eventId != DEM_EVENT_ID_NULL){
					if(priMemFreezeFrameBuffer[i].timeStamp > priMemFreezeFrameBuffer[j].timeStamp){
						//exchange buffer data
						memcpy(&temp,&priMemFreezeFrameBuffer[i],sizeof(FreezeFrameRecType));
						memcpy(&priMemFreezeFrameBuffer[i],&priMemFreezeFrameBuffer[j],sizeof(FreezeFrameRecType));
						memcpy(&priMemFreezeFrameBuffer[j],&temp,sizeof(FreezeFrameRecType));
					}

				}

			}
			priMemFreezeFrameBuffer[i].timeStamp = k++;
		}

	}
	/* update the current timeStamp */
	*timeStamp = k;

}
/*
 * Procedure:	getFreezeFrameData
 * Description:	get FF data according configuration			
 */
static void getFreezeFrameData(const Dem_EventParameterType *eventParam,
                               FreezeFrameRecType *freezeFrame,
                               Dem_EventStatusType eventStatus,
                               EventStatusRecType *eventStatusRec)
{
	Dem_FreezeFrameStorageConditonType prefailedOrFailed;
	uint16 i = 0;
	uint16 storeIndex = 0;
	imask_t state;
	const Dem_FreezeFrameClassType *FreezeFrameLocal = NULL;

	/* clear FF data record */
	memset(freezeFrame, 0, sizeof(FreezeFrameRecType ));

	/* check if prefailed or failed */
	if(eventStatusRec->eventStatusExtended & DEM_TEST_FAILED){
		prefailedOrFailed = FAILED;//confirm the FF is stored for failed

	}
	else{
		 if(eventStatus == DEM_EVENT_STATUS_PREFAILED){
			prefailedOrFailed = PREFAILED;//confirm the FF is stored for prefailed
		 }
		 else{
			prefailedOrFailed = FF_STORAGE_CONDITION_WRONG;
			freezeFrame->eventId = DEM_EVENT_ID_NULL;
			return;
		 }
	}

	/* Find out the corresponding FF class */
	if( eventParam->FreezeFrameClassRef != NULL ) {
		for(i = 0;(i<DEM_MAX_NR_OF_CLASSES_IN_FREEZEFRAME_DATA) && (eventParam->FreezeFrameClassRef[i] != NULL);i++){
			if(eventParam->FreezeFrameClassRef[i]->FFStorageCondition == prefailedOrFailed){
				FreezeFrameLocal = eventParam->FreezeFrameClassRef[i];
				break;
			}
		}
	}

	/* get the dids */
	if(FreezeFrameLocal != NULL){
		if(FreezeFrameLocal->FFIdClassRef != NULL){
			if(DEM_FREEZE_FRAME_NON_OBD == FreezeFrameLocal->FFKind){
				getDidData(&FreezeFrameLocal->FFIdClassRef, &freezeFrame, &storeIndex);
			}
			else if(DEM_FREEZE_FRAME_OBD == FreezeFrameLocal->FFKind){
				getPidData(&FreezeFrameLocal->FFIdClassRef, &freezeFrame, &storeIndex);
			}
		}

	}
	else{
		/* create an empty FF */
		freezeFrame->eventId = DEM_EVENT_ID_NULL;
	}

	/* Check if any data has been stored */
	if (storeIndex != 0) {
		freezeFrame->eventId = eventParam->EventID;
		freezeFrame->occurrence = eventStatusRec->occurrence;
		freezeFrame->dataSize = storeIndex;
		freezeFrame->recordNumber = FreezeFrameLocal->FFRecordNumber;
		freezeFrame->kind = FreezeFrameLocal->FFKind;
		Irq_Save(state);

		if(FF_TimeStamp > DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT){
			rearrangeFreezeFrameTimeStamp(&FF_TimeStamp);
		}

		freezeFrame->timeStamp = FF_TimeStamp;

		FF_TimeStamp++;

		Irq_Restore(state);

		freezeFrame->checksum = calcChecksum(freezeFrame, sizeof(FreezeFrameRecType)-sizeof(ChecksumType));
	}
	else{
		freezeFrame->eventId = DEM_EVENT_ID_NULL;
		freezeFrame->dataSize = storeIndex;
		freezeFrame->checksum = 0;
	}
}


/*
 * Procedure:	storeFreezeFrameDataPreInit
 * Description:	store FF in before  preInitFreezeFrameBuffer DEM's full initialization				
 */
static void storeFreezeFrameDataPreInit(const Dem_EventParameterType *eventParam, const FreezeFrameRecType *freezeFrame)
{
	boolean eventIdFound = FALSE;
	boolean eventIdFreePositionFound=FALSE;
	FreezeFrameRecType *freezeFrameLocal = NULL;
	uint16 i;
	imask_t state;

	Irq_Save(state);

	/* Check if already stored */
	for (i = 0; (i<DEM_MAX_NUMBER_FF_DATA_PRE_INIT) && (!eventIdFound); i++){
		eventIdFound = ((preInitFreezeFrameBuffer[i].eventId == eventParam->EventID) && (preInitFreezeFrameBuffer[i].recordNumber== freezeFrame->recordNumber));
	}

	if(eventIdFound){
		/* overwrite existing */
		memcpy(&preInitFreezeFrameBuffer[i-1], freezeFrame, sizeof(FreezeFrameRecType));
	}
	else{
		/* lookup first free position */
		for (i = 0; (i<DEM_MAX_NUMBER_FF_DATA_PRE_INIT) && (!eventIdFreePositionFound); i++){
			if(preInitFreezeFrameBuffer[i].eventId == DEM_EVENT_ID_NULL){
				eventIdFreePositionFound=TRUE;
			}
		}

		if (eventIdFreePositionFound) {
			memcpy(&preInitFreezeFrameBuffer[i-1], freezeFrame, sizeof(FreezeFrameRecType));
		}
		else {
			/* do displacement */
			if(lookupFreezeFrameForDisplacementPreInit(&freezeFrameLocal)){
				memcpy(freezeFrameLocal, freezeFrame, sizeof(FreezeFrameRecType));
			}
			else{
				DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_STORE_FF_DATA_PRE_INIT_ID, DEM_E_PRE_INIT_FF_DATA_BUFF_FULL);
			}
		}
	}

	Irq_Restore(state);
}


/*
 * Procedure:	updateFreezeFrameOccurrencePreInit
 * Description:	update the occurrence in preInitFreezeFrameBuffer according to the occurrence stored in priMemEventBuffer				
 */
static void updateFreezeFrameOccurrencePreInit(const EventRecType *EventBuffer)
{
	uint16 i;

	for (i = 0; i<DEM_MAX_NUMBER_FF_DATA_PRE_INIT; i++){
		if(preInitFreezeFrameBuffer[i].eventId == EventBuffer->eventId){
			preInitFreezeFrameBuffer[i].occurrence += EventBuffer->occurrence;
		}
	}

}
/*
 * Procedure:	initCurrentFreezeFrameTimeStamp
 * Description:	initialize current timestamp and update the corresponding timestamp in preInitFreezeFrameBuffer		
 */
static void initCurrentFreezeFrameTimeStamp(uint32 *timeStampPtr)
{
	uint16 i = 0;
	uint32 temp = 0;
	imask_t state;

	Irq_Save(state);

	/* Find out the biggest timestamp in the last power on */
	for (i = 0; i<DEM_MAX_NUMBER_FF_DATA_PRI_MEM; i++){
		if((priMemFreezeFrameBuffer[i].eventId != DEM_EVENT_ID_NULL) &&
		  (priMemFreezeFrameBuffer[i].timeStamp > temp)){
			temp = priMemFreezeFrameBuffer[i].timeStamp;
		}
	}
	temp++;
	for (i = 0; i<DEM_MAX_NUMBER_FF_DATA_PRE_INIT; i++){
		if(preInitFreezeFrameBuffer[i].eventId != DEM_EVENT_ID_NULL){
			preInitFreezeFrameBuffer[i].timeStamp += temp;
		}
	}
	*timeStampPtr += temp;
	Irq_Restore(state);
}

/*
 * Procedure:	getExtendedData
 * Description:	Collects the extended data according to "eventParam" and return it in "extData",
 * 				if not found eventId is set to DEM_EVENT_ID_NULL.
 */
static void storeExtendedData(const Dem_EventParameterType *eventParam, boolean firstFail)
{
	Std_ReturnType callbackReturnCode;
	uint16 i;
	uint16 storeIndex = 0;
	uint16 recordSize;
	const Dem_ExtendedDataRecordClassType *extendedDataRecord;
	ExtDataRecType *extData;
	boolean eventIdFound = FALSE;
	boolean bStorePrimaryMem = FALSE;
	boolean bStoredData = FALSE;
	ExtDataRecType *extDataMem;

	if( DEM_PREINITIALIZED == demState ) {
		extDataMem = preInitExtDataBuffer;
	} else {
		extDataMem = priMemExtDataBuffer;
	}

	for (i = 0; (i < DEM_MAX_NR_OF_EVENT_DESTINATION) && (eventParam->EventClass->EventDestination[i] != DEM_EVENT_DESTINATION_END_OF_LIST); i++) {
		if (eventParam->EventClass->EventDestination[i] == DEM_DTC_ORIGIN_PRIMARY_MEMORY) {
			bStorePrimaryMem = TRUE;
		}
	}

	if( TRUE == bStorePrimaryMem ) {

		// Check if already stored
		for (i = 0; (i<DEM_MAX_NUMBER_EXT_DATA_PRI_MEM) && (!eventIdFound); i++){
			eventIdFound = (extDataMem[i].eventId == eventParam->EventID);
			extData = &extDataMem[i];
		}
		if( FALSE == eventIdFound ) {
			extData = NULL;
			for (i = 0; (i < DEM_MAX_NUMBER_EXT_DATA_PRI_MEM); i++){
				if( extDataMem[i].eventId == DEM_EVENT_ID_NULL ) {
					extData = &extDataMem[i];
					break;
				}
			}
			if( NULL == extData ) {
				// Error: Pri mem extended data buffer full
				DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_STORE_EXT_DATA_PRI_MEM_ID, DEM_E_PRI_MEM_EXT_DATA_BUFF_FULL);
				return;
			}
		}

		// Check if any pointer to extended data class
		if (eventParam->ExtendedDataClassRef != NULL) {
			// Request extended data and copy it to the buffer
			for (i = 0; (i < DEM_MAX_NR_OF_RECORDS_IN_EXTENDED_DATA) && (eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i] != NULL); i++) {
				extendedDataRecord = eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i];
				if( DEM_UPDATE_RECORD_VOLATILE != extendedDataRecord->UpdateRule )
				{
					recordSize = extendedDataRecord->DataSize;
					if ((storeIndex + recordSize) <= DEM_MAX_SIZE_EXT_DATA) {
						if( DEM_UPDATE_RECORD_YES == extendedDataRecord->UpdateRule ||
								(DEM_UPDATE_RECORD_NO == extendedDataRecord->UpdateRule && TRUE == firstFail) )
						{
							if( NULL != extendedDataRecord->CallbackGetExtDataRecord ) {

								callbackReturnCode = extendedDataRecord->CallbackGetExtDataRecord(&extData->data[storeIndex]); /** @req DEM282 */
								if (callbackReturnCode != E_OK) {
									// Callback data currently not available, clear space.
									memset(&extData->data[storeIndex], 0xFF, recordSize);
								}
								bStoredData = TRUE;
							} else if( DEM_NO_ELEMENT != extendedDataRecord->InternalDataElement ) {
								getInternalElement( eventParam, extendedDataRecord->InternalDataElement, &extData->data[storeIndex] );
								bStoredData = TRUE;
							}
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
		}

		// Check if any data has been stored
		if (TRUE == bStoredData) {
			extData->eventId = eventParam->EventID;
			extData->checksum = calcChecksum(extData, sizeof(ExtDataRecType)-sizeof(ChecksumType));
		}
	}
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
	imask_t state;
    Irq_Save(state);

	(void)*eventParam;	// Currently not used, do this to avoid warning

	// Lookup event ID
	for (i = 0; (i < DEM_MAX_NUMBER_EVENT_ENTRY_PRI) && (!eventIdFound); i++){
		eventIdFound = (priMemEventBuffer[i].eventId == eventStatus->eventId);
	}

	if (eventIdFound) {
		// Update event found
		priMemEventBuffer[i-1].occurrence = eventStatus->occurrence;
		priMemEventBuffer[i-1].eventStatusExtended = eventStatus->eventStatusExtended;
		priMemEventBuffer[i-1].confirmationCounter = eventStatus->confirmationCounter;
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
			priMemEventBuffer[i-1].eventStatusExtended = eventStatus->eventStatusExtended;
			priMemEventBuffer[i-1].confirmationCounter = eventStatus->confirmationCounter;
			priMemEventBuffer[i-1].checksum = calcChecksum(&priMemEventBuffer[i-1], sizeof(EventRecType)-sizeof(ChecksumType));
		}
		else {
			// Error: Pri mem event buffer full
			DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_STORE_EVENT_PRI_MEM_ID, DEM_E_PRI_MEM_EVENT_BUFF_FULL);
		}
	}

    Irq_Restore(state);
}

/*
 * Procedure:	deleteEventPriMem
 * Description:	Delete the event data of "eventParam->eventId" from "priMemEventBuffer".
 */
static void deleteEventPriMem(const Dem_EventParameterType *eventParam)
{
	boolean eventIdFound = FALSE;
	uint16 i;
	imask_t state;
    Irq_Save(state);

	for (i = 0; (i < DEM_MAX_NUMBER_EVENT_ENTRY_PRI) && (!eventIdFound); i++){
		eventIdFound = (priMemEventBuffer[i].eventId == eventParam->EventID);
	}

	if (eventIdFound) {
		memset(&priMemEventBuffer[i-1], 0, sizeof(EventRecType));
	}

    Irq_Restore(state);
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
 * Procedure:	getExtendedDataPriMem
 * Description:	Get record from buffer if it exists, or pick next free if it doesn't
 */
static void getExtendedDataPriMem(const Dem_EventParameterType *eventParam, ExtDataRecType ** const extendedData) /** @req DEM041 */
{
	boolean eventIdFound = FALSE;
	boolean eventIdFreePositionFound=FALSE;
	uint16 i;

	// Check if already stored
	for (i = 0; (i<DEM_MAX_NUMBER_EXT_DATA_PRI_MEM) && (!eventIdFound); i++){
		if( priMemExtDataBuffer[i].eventId == eventParam->EventID ) {
			*extendedData = &priMemExtDataBuffer[i];
			eventIdFound = TRUE;
		}
	}

	if (!eventIdFound) {
		// No, lookup first free position
		for (i = 0; (i < DEM_MAX_NUMBER_EXT_DATA_PRI_MEM) && (!eventIdFreePositionFound); i++){
			eventIdFreePositionFound =  (priMemExtDataBuffer[i].eventId == DEM_EVENT_ID_NULL);
		}
		if (eventIdFreePositionFound) {
			*extendedData = &priMemExtDataBuffer[i-1];
		}
		else {
			// Error: Pri mem extended data buffer full
			DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_STORE_EXT_DATA_PRI_MEM_ID, DEM_E_PRI_MEM_EXT_DATA_BUFF_FULL);
		}
	}
}

/*
 * Procedure:	deleteExtendedDataPriMem
 * Description:	Delete the extended data of "eventParam->eventId" from "priMemExtDataBuffer".
 */
static void deleteExtendedDataPriMem(const Dem_EventParameterType *eventParam)
{
	boolean eventIdFound = FALSE;
	uint16 i;
	imask_t state;
    Irq_Save(state);

	// Check if already stored
	for (i = 0;(i<DEM_MAX_NUMBER_EXT_DATA_PRI_MEM) && (!eventIdFound); i++){
		eventIdFound = (priMemExtDataBuffer[i].eventId == eventParam->EventID);
	}

	if (eventIdFound) {
		// Yes, clear record
		memset(&priMemExtDataBuffer[i-1], 0, sizeof(ExtDataRecType));
	}

    Irq_Restore(state);
}

/*
 * Procedure:	storeExtendedDataEvtMem
 * Description:	Store the extended data in event memory according to
 * 				"eventParam" destination option
 */
static void mergeExtendedDataEvtMem(const Dem_EventParameterType *eventParam, const ExtDataRecType *extendedData)
{
	uint16 i;
	imask_t state;
	EventRecType *priMemEventRec = NULL;
	const Dem_ExtendedDataRecordClassType *extendedDataRecordClass;
	ExtDataRecType *priMemExtDataRec = NULL;
	uint16 storeIndex = 0;
	boolean bCopiedData = FALSE;
	boolean bStorePrimarMem = FALSE;

	for (i = 0; (i < DEM_MAX_NR_OF_EVENT_DESTINATION) && (eventParam->EventClass->EventDestination[i] != DEM_EVENT_DESTINATION_END_OF_LIST); i++) {
		if (eventParam->EventClass->EventDestination[i] ==  DEM_DTC_ORIGIN_PRIMARY_MEMORY ) {
			bStorePrimarMem = TRUE;
		}
	}

	if( bStorePrimarMem ) {
		/* Management is only relevant for events stored in primary mem (i.e. nvram) */

		getExtendedDataPriMem(eventParam, &priMemExtDataRec);

		if( NULL != priMemExtDataRec ) {
			/* We found an old record or could allocate a new slot */

			for( i = 0; i < DEM_MAX_NUMBER_EVENT_PRI_MEM; i++ ) {
				if( priMemEventBuffer[i].eventId == eventParam->EventID ) {
					priMemEventRec = &priMemEventBuffer[i];
				}
			}

			/* Only copy extended data related to event set during pre-init */
			Irq_Save(state);
			for(i = 0; (i < DEM_MAX_NR_OF_RECORDS_IN_EXTENDED_DATA) && (eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i] != NULL); i++) {
				extendedDataRecordClass = eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i];
				if( DEM_UPDATE_RECORD_VOLATILE != extendedDataRecordClass->UpdateRule ) {
					if( DEM_UPDATE_RECORD_YES == extendedDataRecordClass->UpdateRule ) {
						/* Copy records that failed during pre init */
						memcpy(&priMemExtDataRec->data[storeIndex], &extendedData->data[storeIndex],extendedDataRecordClass->DataSize);
						bCopiedData = TRUE;
					}
					else if( DEM_UPDATE_RECORD_NO == extendedDataRecordClass->UpdateRule ) {
						if( (NULL == priMemEventRec) || !(priMemEventRec->eventStatusExtended && DEM_TEST_FAILED_SINCE_LAST_CLEAR) ) {
							/* Copy only records that was not already failed */
							memcpy(&priMemExtDataRec->data[storeIndex], &extendedData->data[storeIndex],extendedDataRecordClass->DataSize);
							bCopiedData = TRUE;
						}
					}
					else {
						/* DET FEL */
					}
					storeIndex += extendedDataRecordClass->DataSize;
				}
			}
			if( TRUE == bCopiedData ) {
				priMemExtDataRec->eventId = extendedData->eventId;
				priMemExtDataRec->checksum = extendedData->checksum;
			}
			Irq_Restore(state);
		}
		else {
			/* DET FEL */
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
			if(DEM_UPDATE_RECORD_VOLATILE != eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i]->UpdateRule) {
				byteCnt += eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i]->DataSize;
			}
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
/*
 * Procedure:	copyNvmMirror
 * Description: Copies Nvram to buffer
 */

Std_ReturnType copyNvmMirror(const NvM_BlockIdType BlockId, uint8 *dstPtr, const uint8 *srcPtr, uint8 len)
{

#if (DEM_USE_NVM == STD_ON  && DEM_UNIT_TEST == STD_OFF)
	Std_ReturnType blockReadStatus = E_NOT_OK;
	NvM_RequestResultType requestResult;

    if( BlockId != 0 ) {
		NvM_GetErrorStatus(BlockId, &requestResult);
		if(requestResult != NVM_REQ_PENDING ) {
			memcpy(dstPtr, srcPtr, len);
			blockReadStatus = E_OK;
		}
    }

    return blockReadStatus;
#else
    return E_OK;
#endif
}
/*
 * Procedure:	writeNvmMirror
 * Description: store data in NVRam
 */
Std_ReturnType writeNvmMirror(const NvM_BlockIdType BlockId, uint8 *dstPtr, const uint8 *srcPtr, uint8 len)
{
#if (DEM_USE_NVM == STD_ON && DEM_UNIT_TEST == STD_OFF)
	Std_ReturnType blockWriteStatus = E_NOT_OK;
	NvM_RequestResultType requestResult;

    if( BlockId != 0 ) {
    	NvM_GetErrorStatus(BlockId, &requestResult);
		if(requestResult != NVM_REQ_PENDING ) {
			memcpy(dstPtr, srcPtr, len);
			(void)NvM_WriteBlock(BlockId, (const uint8*)dstPtr);
			blockWriteStatus = E_OK;
		}
    }

    return blockWriteStatus;
#else
    return E_OK;
#endif
}

/*
 * Procedure:	storeAgingRecPerMem
 * Description: store aging records in NVRam
 */
static void storeAgingRecPerMem(const NvM_BlockIdType AgingBlockId)
{
	imask_t state;

	Irq_Save(state);

	if( E_OK == writeNvmMirror(AgingBlockId, (uint8 *)HealingMirrorBuffer, (const uint8 *)priMemAgingBuffer, sizeof(priMemAgingBuffer)) ){
		AgingIsModified = FALSE;
	} else {
		AgingIsModified = TRUE;
	}

	Irq_Restore(state);
}
/*
 * Procedure:	deleteAgingRecPriMem
 * Description: delete aging record in primary memory
 */
static void deleteAgingRecPriMem(const Dem_EventParameterType *eventParam)
{
	uint16 i;
	imask_t state;

	Irq_Save(state);

	for (i = 0; i<DEM_MAX_NUMBER_AGING_PRI_MEM; i++){
		if (priMemAgingBuffer[i].eventId == eventParam->EventID){
			memset(&priMemAgingBuffer[i], 0, sizeof(HealingRecType));
		}
	}

	Irq_Restore(state);
}

/*
 * Procedure:	storeFreezeFrameDataPriMem
 * Description: store FreezeFrame data record in primary memory
 */
static void storeFreezeFrameDataPriMem(const Dem_EventParameterType *eventParam, const FreezeFrameRecType *freezeFrame)
{
	boolean eventIdFound = FALSE;
	boolean eventIdFreePositionFound=FALSE;
	boolean displacementPositionFound=FALSE;
	FreezeFrameRecType *freezeFrameLocal;
	uint16 i;
	imask_t state;

	Irq_Save(state);

	/* Check if already stored */
	for (i = 0; (i<DEM_MAX_NUMBER_FF_DATA_PRI_MEM) && (!eventIdFound); i++){
		eventIdFound = ((priMemFreezeFrameBuffer[i].eventId == eventParam->EventID) && (priMemFreezeFrameBuffer[i].recordNumber == freezeFrame->recordNumber));
	}

	if (eventIdFound) {
		memcpy(&priMemFreezeFrameBuffer[i-1], freezeFrame, sizeof(FreezeFrameRecType));
	}
	else {
		for (i = 0; (i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM) && (!eventIdFreePositionFound); i++){
			eventIdFreePositionFound =  (priMemFreezeFrameBuffer[i].eventId == DEM_EVENT_ID_NULL);
		}
		if (eventIdFreePositionFound) {
			memcpy(&priMemFreezeFrameBuffer[i-1], freezeFrame, sizeof(FreezeFrameRecType));
		}
		else {
			displacementPositionFound = lookupFreezeFrameForDisplacement(&freezeFrameLocal);
			if(displacementPositionFound){
				memcpy(freezeFrameLocal, freezeFrame, sizeof(FreezeFrameRecType));
			}
			else{
				DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_STORE_FF_DATA_PRI_MEM_ID, DEM_E_PRI_MEM_FF_DATA_BUFF_FULL);
			}
		}
	}

	Irq_Restore(state);
}
/*
 * Procedure:	storeFreezeFrameDataPerMem
 * Description:	Store the freeze frame data in NVRam
 * 
 */
static void storeFreezeFrameDataPerMem()
{
	imask_t state;

	Irq_Save(state);

	for(uint16 i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM; i++){
		if(memcmp(&priMemFreezeFrameBuffer[i], FreezeFrameMirrorBuffer[i], sizeof(FreezeFrameRecType))){
			if( E_OK == writeNvmMirror(FreezeFrameBlockId[i], (uint8 *)FreezeFrameMirrorBuffer[i], (const uint8 *)&priMemFreezeFrameBuffer[i], sizeof(FreezeFrameRecType)) ) {
				FFIsModified = FALSE;
			}
			else
			{
				FFIsModified = TRUE;
			}
		}
	}

	Irq_Restore(state);
}

// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715}		Symbol not referenced
static void deleteFreezeFrameDataPriMem(const Dem_EventParameterType *eventParam)
{
	uint16 i;
	imask_t state;

	Irq_Save(state);

	for (i = 0; i<DEM_MAX_NUMBER_FF_DATA_PRI_MEM; i++){
		if (priMemFreezeFrameBuffer[i].eventId == eventParam->EventID){
			memset(&priMemFreezeFrameBuffer[i], 0, sizeof(FreezeFrameRecType));

		}
	}

	Irq_Restore(state);
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
			if(freezeFrame->kind == DEM_FREEZE_FRAME_OBD){
				storeOBDFreezeFrameDataPriMem(eventParam, freezeFrame);
			}
			else{
				storeFreezeFrameDataPriMem(eventParam, freezeFrame);
			}
			
			storeFreezeFrameDataPerMem();
			break;

		case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
		case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
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
 * Procedure:	lookupFreezeFrameDataRecNumParam
 * Description:	Returns TRUE if the requested freezeFrame data number was found among the configured records for the event.
 * 				"freezeFrameClassPtr" returns a pointer to the record class.
 */
static boolean lookupFreezeFrameDataRecNumParam(uint8 recordNumber, const Dem_EventParameterType *eventParam, Dem_FreezeFrameClassType const **freezeFrameClassPtr)
{
	boolean recNumFound = FALSE;
	uint16 i;

	if (eventParam->FreezeFrameClassRef != NULL) {
		for (i = 0; (i < DEM_MAX_NR_OF_CLASSES_IN_FREEZEFRAME_DATA) && (eventParam->FreezeFrameClassRef[i] != NULL) && (!recNumFound); i++) {
			if (eventParam->FreezeFrameClassRef[i]->FFRecordNumber == recordNumber) {
				*freezeFrameClassPtr =  eventParam->FreezeFrameClassRef[i];
				recNumFound = TRUE;
			}
		}
	}

	return recNumFound;
}

/*
 * Procedure:	lookupFreezeFrameDataSize
 * Description:	Returns TRUE if the requested freezeFrame data size was obtained successfully from the configuration.
 * 				"dataSize" returns a pointer to the data size.
 */
static boolean lookupFreezeFrameDataSize(uint8 recordNumber, Dem_FreezeFrameClassType const  **freezeFrameClassPtr, uint16 *dataSize)
{
	Std_ReturnType callbackReturnCode;
	boolean dataSizeFound = TRUE;
	uint16 dataSizeLocal = 0;
	uint16 i;

	if (*freezeFrameClassPtr != NULL) {
		for (i = 0; (i < DEM_MAX_NR_OF_RECORDS_IN_FREEZEFRAME_DATA) && ((*freezeFrameClassPtr)->FFIdClassRef[i]->Arc_EOL != TRUE); i++) {
			if((*freezeFrameClassPtr)->FFIdClassRef[i]->DidReadDataLengthFnc != NULL){
				callbackReturnCode = (*freezeFrameClassPtr)->FFIdClassRef[i]->DidReadDataLengthFnc(&dataSizeLocal);
				if(callbackReturnCode != E_OK){
					return (dataSizeFound = FALSE);
				}
			}
			else{
				dataSizeLocal = (*freezeFrameClassPtr)->FFIdClassRef[i]->PidOrDidSize;
			}

			*dataSize += dataSizeLocal + DEM_DID_IDENTIFIER_SIZE_OF_BYTES;
		}

	}

	return dataSizeFound;
}

/*
 * Procedure:	lookupFreezeFrameDataPriMem
 * Description: Returns TRUE if the requested event id is found, "freezeFrame" points to the found data.
 */
static boolean lookupFreezeFrameDataPriMem(Dem_EventIdType eventId,uint8 recordNumber, FreezeFrameRecType **freezeFrame)
{
	boolean eventIdFound = FALSE;
	uint16 i;

	for (i = 0; (i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM) && (!eventIdFound); i++) {
		eventIdFound = ((priMemFreezeFrameBuffer[i].eventId == eventId) && (priMemFreezeFrameBuffer[i].recordNumber == recordNumber));
	}

	if (eventIdFound) {
		*freezeFrame = &priMemFreezeFrameBuffer[i-1];
	}

	return eventIdFound;
}

/*
 * Procedure:	handlePreInitEvent
 * Description:	Handle the updating of event status and storing of
 * 				event related data in preInit buffers.
 */
static void handlePreInitEvent(Dem_EventIdType eventId, Dem_EventStatusType eventStatus)
{
	const Dem_EventParameterType *eventParam;
	EventStatusRecType *eventStatusBeforeUpdate;
	EventStatusRecType eventStatusLocal;
	FreezeFrameRecType freezeFrameLocal;
	boolean bFirstFail;

	lookupEventIdParameter(eventId, &eventParam);
	if (eventParam != NULL) {
		if (eventParam->EventClass->OperationCycleRef < DEM_OPERATION_CYCLE_ID_ENDMARK) {
			if (operationCycleStateList[eventParam->EventClass->OperationCycleRef] == DEM_CYCLE_STATE_START) {
				lookupEventStatusRec(eventParam->EventID, &eventStatusBeforeUpdate);
				bFirstFail = (NULL == eventStatusBeforeUpdate) || !(eventStatusBeforeUpdate->eventStatusExtended & DEM_TEST_FAILED_SINCE_LAST_CLEAR);
				if (eventStatus == DEM_EVENT_STATUS_FAILED) {
					updateEventStatusRec(eventParam, eventStatus, TRUE, &eventStatusLocal);
				}
				else {
					updateEventStatusRec(eventParam, eventStatus, FALSE, &eventStatusLocal);
				}

				if (eventStatusLocal.errorStatusChanged) {
					if (eventStatusLocal.eventStatusExtended & DEM_TEST_FAILED) {
						storeExtendedData(eventParam, bFirstFail);
						getFreezeFrameData(eventParam, &freezeFrameLocal,eventStatus,&eventStatusLocal);
						if (freezeFrameLocal.eventId != DEM_EVENT_ID_NULL) {
							if(freezeFrameLocal.kind == DEM_FREEZE_FRAME_OBD){
								storeOBDFreezeFrameDataPreInit(eventParam, &freezeFrameLocal);
							}
							else{
								storeFreezeFrameDataPreInit(eventParam, &freezeFrameLocal);
							}							
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
	EventStatusRecType *eventStatusBeforeUpdate;
	EventStatusRecType eventStatusLocal;
	FreezeFrameRecType freezeFrameLocal;
	Dem_EventStatusType eventStatusTemp = eventStatus;
	boolean bFirstFail;

	lookupEventIdParameter(eventId, &eventParam);
	if (eventParam != NULL) {
		if (eventParam->EventClass->OperationCycleRef < DEM_OPERATION_CYCLE_ID_ENDMARK) {
			if (operationCycleStateList[eventParam->EventClass->OperationCycleRef] == DEM_CYCLE_STATE_START) {
				if ((!((disableDtcStorage.storageDisabled) && (checkDtcGroup(disableDtcStorage.dtcGroup, eventParam)) && (checkDtcKind(disableDtcStorage.dtcKind, eventParam)))))  {
					lookupEventStatusRec(eventParam->EventID, &eventStatusBeforeUpdate);
					bFirstFail = (NULL == eventStatusBeforeUpdate) || !(eventStatusBeforeUpdate->eventStatusExtended & DEM_TEST_FAILED_SINCE_LAST_CLEAR);
					updateEventStatusRec(eventParam, eventStatus, TRUE, &eventStatusLocal);
					if (eventStatusLocal.errorStatusChanged) {
						storeEventEvtMem(eventParam, &eventStatusLocal); /** @req DEM184 */
						if (eventStatusLocal.eventStatusExtended & DEM_TEST_FAILED) {
							storeExtendedData(eventParam, bFirstFail);
						}

						if ((eventStatusTemp == DEM_EVENT_STATUS_PREFAILED)
								|| (eventStatusLocal.eventStatusExtended & DEM_TEST_FAILED)){
							getFreezeFrameData(eventParam, &freezeFrameLocal,eventStatus,&eventStatusLocal);
							if (freezeFrameLocal.eventId != DEM_EVENT_ID_NULL) {
								storeFreezeFrameDataEvtMem(eventParam, &freezeFrameLocal); /** @req DEM190 */
							}
						}
						else{
							// do nothing
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
	EventStatusRecType *eventStatusRecPtr;
	imask_t state;
    Irq_Save(state);

	lookupEventStatusRec(eventId, &eventStatusRecPtr);
	if (eventStatusRecPtr != NULL) {
		eventStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)~DEM_TEST_FAILED; /** @req DEM187 */
	}

    Irq_Restore(state);
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
 * Procedure:	deleteEventMemory
 * Description:	If aging of an event occurs, the Dem module shall delete the event from 
 *				the event memory including its event related data
 */
static void deleteEventMemory(const Dem_EventParameterType *eventParam)
{
	uint16 i;

	for (i = 0; (i < DEM_MAX_NR_OF_EVENT_DESTINATION) && (eventParam->EventClass->EventDestination[i] != DEM_EVENT_DESTINATION_END_OF_LIST); i++) {
		switch (eventParam->EventClass->EventDestination[i])
		{
		case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
			deleteEventPriMem(eventParam);
			deleteFreezeFrameDataPriMem(eventParam);
			deleteExtendedDataPriMem(eventParam);
			storeFreezeFrameDataPerMem();
			break;

		case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
			//TODO:need to add corresponding event and extended deleting functions

			break;

		case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
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
 * Procedure:	lookupAgingRecPriMem
 * Description:	Returns the pointer to event id parameters of "eventId" in "*priMemAgingBuffer",
 * 				if not found NULL is returned.
 */
static boolean lookupAgingRecPriMem(Dem_EventIdType eventId, const HealingRecType **agingRec)
{
	uint16 i;
	boolean agingRecFound = FALSE;

	for (i = 0; i < DEM_MAX_NUMBER_AGING_PRI_MEM && (!agingRecFound); i++) {
		if(priMemAgingBuffer[i].eventId == eventId){
			agingRecFound = TRUE;
		}

	}

	if(agingRecFound){
		*agingRec = &priMemAgingBuffer[i-1];
	}
	else{
		*agingRec = NULL;
	}

	return agingRecFound;

}


/*
 * Procedure:	handleAging
 * Description:	according to the operation state of "operationCycleId" to "cycleState" , handle the aging relatived data
 * 				Returns E_OK if operation was successful else E_NOT_OK.
 */
static Std_ReturnType handleAging(Dem_OperationCycleIdType operationCycleId, Dem_OperationCycleStateType cycleState)
{
	uint16 i;
	Std_ReturnType returnCode = E_OK;
	HealingRecType *agingRecLocal = NULL;
	boolean agingRecFound = FALSE;

	if (operationCycleId < DEM_OPERATION_CYCLE_ID_ENDMARK) {
		switch (cycleState)
		{
		case DEM_CYCLE_STATE_START:
			break;

		case DEM_CYCLE_STATE_END:/** @req Dem490 */
			for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
				if(eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL){
					if(eventStatusBuffer[i].eventParamRef != NULL){
						if(eventStatusBuffer[i].eventParamRef->EventClass != NULL){
							if((eventStatusBuffer[i].eventParamRef->EventClass->HealingAllowed == TRUE)\
								&& (eventStatusBuffer[i].eventParamRef->EventClass->HealingCycleRef == operationCycleId)){
								if((eventStatusBuffer[i].eventStatusExtended & DEM_CONFIRMED_DTC)\
									&& (!(eventStatusBuffer[i].eventStatusExtended & DEM_TEST_FAILED_THIS_OPERATION_CYCLE))\
									&& (!(eventStatusBuffer[i].eventStatusExtended & DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE))){
									agingRecFound = lookupAgingRecPriMem(eventStatusBuffer[i].eventId, (const HealingRecType **)(&agingRecLocal));
									if(agingRecFound){
										agingRecLocal->agingCounter++;/** @req Dem489 */
										agingRecLocal->checksum = calcChecksum(agingRecLocal,sizeof(HealingRecType) - sizeof(ChecksumType));
										if(agingRecLocal->agingCounter > eventStatusBuffer[i].eventParamRef->EventClass->HealingCycleCounter){
											//deleteEventMemory(eventStatusBuffer[i].eventParamRef); /** @req Dem497 */

											deleteAgingRecPriMem(eventStatusBuffer[i].eventParamRef);

											eventStatusBuffer[i].eventStatusExtended &= (Dem_EventStatusExtendedType)(~DEM_CONFIRMED_DTC);
											eventStatusBuffer[i].eventStatusExtended &= (Dem_EventStatusExtendedType)(~DEM_PENDING_DTC);
											eventStatusBuffer[i].eventStatusExtended &= (Dem_EventStatusExtendedType)(~DEM_WARNING_INDICATOR_REQUESTED);
											eventStatusBuffer[i].confirmationCounter = 0;
											storeEventEvtMem(eventStatusBuffer[i].eventParamRef, &eventStatusBuffer[i]);
										}
										/* Set the flag,start up the storage of NVRam in main function. */
										AgingIsModified = TRUE;
									}
									else{
										/* If it does exist,establish a new record for the corresponding event */
										agingRecFound = lookupAgingRecPriMem(DEM_EVENT_ID_NULL, (const HealingRecType **)(&agingRecLocal));
										if(agingRecFound){
											agingRecLocal->eventId = eventStatusBuffer[i].eventId;
											agingRecLocal->agingCounter++;
											agingRecLocal->checksum = calcChecksum(agingRecLocal,sizeof(HealingRecType) - sizeof(ChecksumType));
											AgingIsModified = TRUE;
										}
										else{
											/* primary memory of aging records is full. */
										}
									}
								}
								else{
									/* If the status bit testFailed (bit 0) is set during the operation cycle, the counter shall be reset. */
									if(eventStatusBuffer[i].eventStatusExtended & DEM_TEST_FAILED){
										agingRecFound = lookupAgingRecPriMem(eventStatusBuffer[i].eventId, (const HealingRecType **)(&agingRecLocal));
										if(agingRecFound){
											if(agingRecLocal->agingCounter){
												agingRecLocal->agingCounter = 0;
												agingRecLocal->checksum = calcChecksum(agingRecLocal,sizeof(HealingRecType) - sizeof(ChecksumType));
												AgingIsModified = TRUE;
											}
										}
									}
								}
							}
						}
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
						storeEventEvtMem(eventStatusBuffer[i].eventParamRef, &eventStatusBuffer[i]);
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

	EventStatusRecType *eventStatusRecPtr;
	const Dem_EventParameterType *eventIdParamList;

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
		eventStatusBuffer[i].maxFaultDetectionCounter = 0;
		eventStatusBuffer[i].occurrence = 0;
		eventStatusBuffer[i].eventStatusExtended = DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE | DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR;
		eventStatusBuffer[i].errorStatusChanged = FALSE;
		eventStatusBuffer[i].confirmationCounter = 0;
	}

	// Insert all supported events into event status buffer
	uint16 index = 0;
	eventIdParamList = configSet->EventParameter;
	while( !eventIdParamList[index].Arc_EOL ) {
		// Find next free position in event status buffer
		lookupEventStatusRec(DEM_EVENT_ID_NULL, &eventStatusRecPtr);
		if(NULL != eventStatusRecPtr) {
			eventStatusRecPtr->eventId = eventIdParamList[index].EventID;
			eventStatusRecPtr->eventParamRef = &eventIdParamList[index];
		} else {
			// event status buffer is full
			DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_PREINIT_ID, DEM_E_EVENT_STATUS_BUFF_FULL);
		}
		index++;
	}

	//lint -save
	//lint -e568 -e685 //PC-Lint exception.
	//lint -e681 //PC-Lint exception to MISRA 14.1: Loop is not entered. This only happens when config variable is zero. Keep as it is for less complex code.
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
	//lint -restore

	for (i = 0; i < DEM_MAX_NUMBER_EXT_DATA_PRE_INIT; i++) {
		preInitExtDataBuffer[i].checksum = 0;
		preInitExtDataBuffer[i].eventId = DEM_EVENT_ID_NULL;
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
//	ChecksumType cSum;
	boolean entryValid = FALSE;
	const Dem_EventParameterType *eventParam;

	if(DEM_PREINITIALIZED != demState){
		/*
		 * Dem_PreInit was has not been called since last time Dem_Shutdown was called.
		 * This suggests that we are resuming from sleep. According to section 5.7 in
		 * EcuM specification, RAM content is assumed to be still valid from the previous cycle.
		 * Do not read from saved error log since buffers already contains this data.
		 */
		(void)setOperationCycleState(DEM_ACTIVE, DEM_CYCLE_STATE_START);

	} else {

		for(i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM; i++){
			if( E_NOT_OK == copyNvmMirror(FreezeFrameBlockId[i], (uint8 *)&priMemFreezeFrameBuffer[i], (const uint8 *)FreezeFrameMirrorBuffer[i], sizeof(FreezeFrameRecType)) ){
				//TODO:NVM is busy or block id is 0,report error or what?
			}
		}
		//recover Aging from NVRam to RAM
		if(E_OK == copyNvmMirror(HealingBlockId, (uint8*)priMemAgingBuffer, (const uint8*)HealingMirrorBuffer, sizeof(priMemAgingBuffer)) ){

		}

		// Validate aging records stored in primary memory
		for (i = 0; i < DEM_MAX_NUMBER_AGING_PRI_MEM; i++){
			entryValid = checkEntryValid(priMemAgingBuffer[i].eventId);
//			cSum = calcChecksum(&priMemAgingBuffer[i], sizeof(HealingRecType) - sizeof(ChecksumType));
			if ((priMemAgingBuffer[i].eventId == DEM_EVENT_ID_NULL) || (FALSE == entryValid)) {
				// Unlegal record, clear the record
				memset(&priMemAgingBuffer[i], 0, sizeof(HealingRecType));
				AgingIsModified = TRUE;
			}
		}

		// Validate extended data records stored in primary memory
		for (i = 0; i < DEM_MAX_NUMBER_EXT_DATA_PRI_MEM; i++) {
			entryValid = checkEntryValid(priMemExtDataBuffer[i].eventId);
//			cSum = calcChecksum(&priMemExtDataBuffer[i], sizeof(ExtDataRecType)-sizeof(ChecksumType));
			if ((priMemExtDataBuffer[i].eventId == DEM_EVENT_ID_NULL) || (FALSE == entryValid)) {
				// Unlegal record, clear the record
				memset(&priMemExtDataBuffer[i], 0, sizeof(ExtDataRecType));
			}
		}

		/* Transfer extended data to event memory if necessary */
		for (i = 0; i < DEM_MAX_NUMBER_EXT_DATA_PRE_INIT; i++) {
			if (preInitExtDataBuffer[i].eventId !=  DEM_EVENT_ID_NULL) {
				lookupEventIdParameter(preInitExtDataBuffer[i].eventId, &eventParam);
				mergeExtendedDataEvtMem(eventParam, &preInitExtDataBuffer[i]);
			}
		}

		// Validate event records stored in primary memory
		for (i = 0; i < DEM_MAX_NUMBER_EVENT_PRI_MEM; i++) {
			entryValid = checkEntryValid(priMemEventBuffer[i].eventId);
//			cSum = calcChecksum(&priMemEventBuffer[i], sizeof(EventRecType)-sizeof(ChecksumType));
			if ((priMemEventBuffer[i].eventId == DEM_EVENT_ID_NULL) || (FALSE == entryValid)) {
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

		//initialize the current timestamp and update the timestamp in pre init
		initCurrentFreezeFrameTimeStamp(&FF_TimeStamp);

		//lint -save
		//lint -e568 //PC-Lint exception.
		//lint -e685 //PC-Lint exception.
		//lint -e681 //PC-Lint exception to MISRA 14.1: Loop is not entered. This only happens when DEM_MAX_NUMBER_FF_DATA_PRE_INIT is zero. Keep as it is for less complex code.
		// Validate freeze frame records stored in primary memory
		for (i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM; i++) {
			entryValid = checkEntryValid(priMemFreezeFrameBuffer[i].eventId);
//			cSum = calcChecksum(&priMemFreezeFrameBuffer[i], sizeof(FreezeFrameRecType)-sizeof(ChecksumType));
			if ((priMemFreezeFrameBuffer[i].eventId == DEM_EVENT_ID_NULL) || (FALSE == entryValid)) {
				// Unlegal record, clear the record
				memset(&priMemFreezeFrameBuffer[i], 0, sizeof(FreezeFrameRecType));
			}
		}
		//lint -restore

		/* Transfer updated event data to event memory */
		for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
			if (eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL) {
				lookupEventIdParameter(eventStatusBuffer[i].eventId, &eventParam);
				storeEventEvtMem(eventParam, &eventStatusBuffer[i]);
			}
		}

		//lint -save
		//lint -e568 //PC-Lint exception.
		//lint -e685 //PC-Lint exception.
		//lint -e681 //PC-Lint exception to MISRA 14.1: Loop is not entered. This only happens when DEM_MAX_NUMBER_FF_DATA_PRE_INIT is zero. Keep as it is for less complex code.
		/* Transfer freeze frames to event memory */
		for (i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRE_INIT; i++) {
			if (preInitFreezeFrameBuffer[i].eventId != DEM_EVENT_ID_NULL) {
				lookupEventIdParameter(preInitFreezeFrameBuffer[i].eventId, &eventParam);
				storeFreezeFrameDataEvtMem(eventParam, &preInitFreezeFrameBuffer[i]);
			}
		}
		//lint -restore
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

	FFRecordFilterIndex = DEM_MAX_NUMBER_FF_DATA_PRI_MEM;

	demState = DEM_INITIALIZED;
}


/*
 * Procedure:	Dem_shutdown
 * Reentrant:	No
 */
void Dem_Shutdown(void)
{
	(void)setOperationCycleState(DEM_ACTIVE, DEM_CYCLE_STATE_END); /** @req DEM047 */

	/* handleAging() should be called behind setOperationCycleState() */
	(void)handleAging(DEM_ACTIVE, DEM_CYCLE_STATE_END);

	demState = DEM_UNINITIALIZED; /** @req DEM368 */
}


/*
 * Interface for basic software scheduler
 */
void Dem_MainFunction(void)/** @req DEM125 */
{
	if (FFIsModified) {
		storeFreezeFrameDataPerMem();
	}

	if (AgingIsModified) {
		storeAgingRecPerMem(HealingBlockId);
	}
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
		(void)handleAging(operationCycleId, cycleState);

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
void Dem_ReportErrorStatus( Dem_EventIdType eventId, Dem_EventStatusType eventStatus ) /** @req DEM206 */
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
						| DEM_CONFIRMED_DTC
						| DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR
						| DEM_TEST_FAILED_SINCE_LAST_CLEAR
						| DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE
						| DEM_WARNING_INDICATOR_REQUESTED
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
	uint8 dtcStatusAvailabilityMask;
	Dem_GetDTCStatusAvailabilityMask(&dtcStatusAvailabilityMask);

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
		dtcFilter.dtcStatusMask = dtcStatusMask & dtcStatusAvailabilityMask;
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
Dem_ReturnClearDTCType Dem_ClearDTC(uint32 dtc, Dem_DTCKindType dtcKind, Dem_DTCOriginType dtcOrigin) /** @req DEM009 *//** @req DEM241 */
{
	Dem_ReturnClearDTCType returnCode = DEM_CLEAR_OK;
	Dem_EventIdType eventId;
	const Dem_EventParameterType *eventParam;
	uint16 i, j;

	if (demState == DEM_INITIALIZED) {
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
								if (dtcOriginFound) {
									switch (dtcOrigin)
									{
									case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
										/** @req DEM077 */
										if ((eventParam->CallbackInitMforE != NULL) && (eventParam->CallbackInitMforE->CallbackInitMForEFnc != NULL))
											eventParam->CallbackInitMforE->CallbackInitMForEFnc(DEM_INIT_MONITOR_CLEAR);
										deleteEventPriMem(eventParam);
										deleteFreezeFrameDataPriMem(eventParam);
										deleteExtendedDataPriMem(eventParam);
										resetEventStatusRec(eventParam);
										storeFreezeFrameDataPerMem();
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
	const Dem_EventParameterType *eventParam;
	uint16 i;

	if (demState == DEM_INITIALIZED) {
		// TODO: Behavior is not defined if group or kind do not match active settings, therefore the filter is just switched off.
		(void)dtcGroup; (void)dtcKind;	// Just to make get rid of PC-Lint warnings

		for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
			if (eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL) {
				eventParam = eventStatusBuffer[i].eventParamRef;
				if (eventParam != NULL) {
					if ((DEM_CLEAR_ALL_EVENTS == STD_ON) || (eventParam->DTCClassRef != NULL)) {
						if (checkDtcKind(disableDtcStorage.dtcKind, eventParam)) {
							if (checkDtcGroup(disableDtcStorage.dtcGroup, eventParam)) {
								if ((eventParam->CallbackInitMforE != NULL) && (eventParam->CallbackInitMforE->CallbackInitMForEFnc != NULL))
									eventParam->CallbackInitMforE->CallbackInitMForEFnc(DEM_INIT_MONITOR_RESTART);
							}
						}
					}
				}
			}
		}

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
							if( extendedDataRecordClass->UpdateRule != DEM_UPDATE_RECORD_VOLATILE )
							{
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
								if( NULL != extendedDataRecordClass->CallbackGetExtDataRecord ) {
									extendedDataRecordClass->CallbackGetExtDataRecord(destBuffer);
									*bufSize = extendedDataRecordClass->DataSize;
									returnCode = DEM_RECORD_OK;
								}
								else if (DEM_NO_ELEMENT != extendedDataRecordClass->InternalDataElement ) {
									getInternalElement( eventRec->eventParamRef, extendedDataRecordClass->InternalDataElement, destBuffer );
									*bufSize = extendedDataRecordClass->DataSize;
									returnCode = DEM_RECORD_OK;
								}
								else {
									returnCode = DEM_RECORD_WRONG_DTC;
								}

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

/*
 * Procedure:	Dem_GetFreezeFrameDataByDTC
 * Reentrant:	No
 */
/** @req DEM236 */
Dem_ReturnGetFreezeFrameDataByDTCType Dem_GetFreezeFrameDataByDTC(uint32  dtc,Dem_DTCKindType  dtcKind,Dem_DTCOriginType  dtcOrigin,uint8  recordNumber,uint8*  destBuffer,uint8*  bufSize)
{
	Dem_ReturnGetFreezeFrameDataByDTCType returnCode = DEM_GET_FFDATABYDTC_WRONG_DTC;
	EventStatusRecType *eventRec;
	Dem_FreezeFrameClassType const *FFDataRecordClass = NULL;
	FreezeFrameRecType *freezeframe;
	uint16 FFDataSize = 0;

	if (demState == DEM_INITIALIZED) {
		if (lookupDtcEvent(dtc, &eventRec)) {
			if (checkDtcKind(dtcKind, eventRec->eventParamRef)) {
				if (checkDtcOrigin(dtcOrigin, eventRec->eventParamRef)) {
					if (lookupFreezeFrameDataRecNumParam(recordNumber, eventRec->eventParamRef, &FFDataRecordClass)) {
						lookupFreezeFrameDataSize(recordNumber, &FFDataRecordClass, &FFDataSize);
						if (*bufSize >= FFDataSize) {
							switch (dtcOrigin)
							{
							case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
								if (lookupFreezeFrameDataPriMem(eventRec->eventId,recordNumber, &freezeframe)) {
									memcpy(destBuffer, freezeframe->data, FFDataSize); /** @req DEM071 */
									*bufSize = FFDataSize;
									returnCode = DEM_GET_FFDATABYDTC_OK;
								}
								else {
									*bufSize = 0;
									returnCode = DEM_GET_FFDATABYDTC_OK;
								}
								break;

							case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
							case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
							case DEM_DTC_ORIGIN_MIRROR_MEMORY:
								// Not yet supported
								returnCode = DEM_GET_FFDATABYDTC_WRONG_DTCORIGIN;
								DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETFREEZEFRAMEDATARECORDBYDTC_ID, DEM_E_NOT_IMPLEMENTED_YET);
								break;
							default:
								returnCode = DEM_GET_FFDATABYDTC_WRONG_DTCORIGIN;
								break;
							}
						}
						else{
							returnCode = DEM_GET_FFDATABYDTC_BUFFERSIZE;
						}
					}
					else {
						returnCode = DEM_GET_FFDATABYDTC_RECORDNUMBER;
					}
				}
				else {
					returnCode = DEM_GET_FFDATABYDTC_WRONG_DTCORIGIN;
				}
			}
			else {
				returnCode = DEM_GET_FFDATABYDTC_WRONG_DTCKIND;
			}
		}
		else{
			returnCode = DEM_GET_FFDATABYDTC_WRONG_DTC;

		}
	} else {
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETFREEZEFRAMEDATARECORDBYDTC_ID, DEM_E_UNINIT);
		returnCode = DEM_GET_ID_PENDING;
	}

	return returnCode;


}

/*
 * Procedure:	Dem_GetFreezeFrameDataIdentifierByDTC
 * Reentrant:	No
 */
Dem_GetFreezeFameDataIdentifierByDTCType Dem_GetFreezeFrameDataIdentifierByDTC(uint32  dtc,
																						Dem_DTCKindType  dtcKind,
																						Dem_DTCOriginType  dtcOrigin,
																						uint8  recordNumber,
																						uint8*  arraySize,
																						const  uint16** dataId )/** @req DEM237 */
{
	Dem_GetFreezeFameDataIdentifierByDTCType returnCode = DEM_GET_ID_WRONG_FF_TYPE;
	Dem_FreezeFrameClassType const *FFDataRecordClass = NULL;
	EventStatusRecType *eventRec;
	uint8 didNum = 0;
	uint16 i = 0;
	if (demState == DEM_INITIALIZED) {
		if (lookupDtcEvent(dtc, &eventRec)) {
			if (checkDtcKind(dtcKind, eventRec->eventParamRef)) {
				if (checkDtcOrigin(dtcOrigin, eventRec->eventParamRef)) {
					if (lookupFreezeFrameDataRecNumParam(recordNumber, eventRec->eventParamRef, &FFDataRecordClass)) {
						if(FFDataRecordClass->FFIdClassRef != NULL){
							for(i=0; (i < DEM_MAX_NR_OF_RECORDS_IN_FREEZEFRAME_DATA) && (!(FFDataRecordClass->FFIdClassRef[i]->Arc_EOL)); i++){
								dataId[didNum] = &FFDataRecordClass->FFIdClassRef[i]->DidIdentifier;/** @req DEM073 */
								didNum++;
								returnCode = DEM_GET_ID_OK;

							}
							*arraySize = didNum;
						}
						else {
							returnCode = DEM_GET_ID_WRONG_FF_TYPE;
						}
					}
					else{
						returnCode = DEM_GET_ID_WRONG_FF_TYPE;
					}
				}
				else{
					returnCode = DEM_GET_ID_WRONG_DTCORIGIN;
				}
			}
			else{
				returnCode = DEM_GET_ID_WRONG_DTCKIND;
			}
		}
		else{
			returnCode = DEM_GET_ID_WRONG_DTC;
		}

	}
	else{
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETFREEZEFRAMEDATAIDENTIFIERBYDTC_ID, DEM_E_UNINIT);
	}

	return returnCode;

}

/*
 * Procedure:	Dem_GetSizeOfFreezeFrame
 * Reentrant:	No
 */
 /** @req DEM238 */
Dem_ReturnGetSizeOfFreezeFrameType Dem_GetSizeOfFreezeFrame(uint32  dtc,Dem_DTCKindType  dtcKind,Dem_DTCOriginType  dtcOrigin,uint8  recordNumber,uint16*  sizeOfFreezeFrame)
{
	Dem_ReturnGetSizeOfFreezeFrameType returnCode = DEM_GET_SIZEOFFF_PENDING;
	Dem_FreezeFrameClassType const *FFDataRecordClass = NULL;
	Std_ReturnType callbackReturnCode;
	EventStatusRecType *eventRec;
	uint16 dataSize = 0;
	uint16 i = 0;

	if (demState == DEM_INITIALIZED) {
		if (lookupDtcEvent(dtc, &eventRec)) {
			if (checkDtcKind(dtcKind, eventRec->eventParamRef)) {
				if (checkDtcOrigin(dtcOrigin, eventRec->eventParamRef)) {
					if (lookupFreezeFrameDataRecNumParam(recordNumber, eventRec->eventParamRef, &FFDataRecordClass)) {
						if(FFDataRecordClass->FFIdClassRef != NULL){
							/* Note - there is a function called lookupFreezeFrameDataSize that can be used here */
							for(i = 0; (i < DEM_MAX_NR_OF_RECORDS_IN_FREEZEFRAME_DATA) && (!(FFDataRecordClass->FFIdClassRef[i]->Arc_EOL)); i++){
								/* read out the did size */
								if(FFDataRecordClass->FFIdClassRef[i]->DidReadDataLengthFnc != NULL){
									callbackReturnCode = FFDataRecordClass->FFIdClassRef[i]->DidReadDataLengthFnc(&dataSize);
									if(callbackReturnCode != E_OK){
										return (returnCode = DEM_GET_SIZEOFFF_PENDING);
									}
								}
								else{
									dataSize = FFDataRecordClass->FFIdClassRef[i]->PidOrDidSize;
								}
								*sizeOfFreezeFrame += dataSize+DEM_DID_IDENTIFIER_SIZE_OF_BYTES;/** @req DEM074 */
								returnCode = DEM_GET_SIZEOFFF_OK;
							}
						}
						else {
							returnCode = DEM_GET_SIZEOFFF_WRONG_RNUM;
						}
					}
					else{
						returnCode = DEM_GET_SIZEOFFF_WRONG_RNUM;
					}
				}
				else{
					returnCode = DEM_GET_SIZEOFFF_WRONG_DTCOR;
				}
			}
			else{
				returnCode = DEM_GET_SIZEOFFF_WRONG_DTCKIND;
			}
		}
		else{
			returnCode = DEM_GET_SIZEOFFF_WRONG_DTC;
		}

	}
	else{
			DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETFREEZEFRAMEDATAIDENTIFIERBYDTC_ID, DEM_E_UNINIT);
			returnCode = DEM_GET_SIZEOFFF_PENDING;
		}

	return returnCode;


}

/** @req DEM209 */
Dem_ReturnSetDTCFilterType Dem_SetDTCFilterForRecords(uint16 *NumberOfFilteredRecords)
{
	Dem_ReturnSetDTCFilterType ret = DEM_WRONG_FILTER;
    if (demState != DEM_INITIALIZED) {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_SETDTCFILTERFORRECORDS_ID, DEM_E_UNINIT);
    } else {
        uint16 nofRecords = 0;
        for( uint16 i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM; i++ ) {
        	/* @req DEM210 */ /* We are searching in priMem buffer */
            if( DEM_EVENT_ID_NULL != priMemFreezeFrameBuffer[i].eventId ) {
                EventStatusRecType *eventStatusRecPtr = NULL;
                lookupEventStatusRec(priMemFreezeFrameBuffer[i].eventId, &eventStatusRecPtr);
                if( (NULL != eventStatusRecPtr) && (NULL != eventStatusRecPtr->eventParamRef->DTCClassRef) ) {
                    nofRecords++;
                }
            }
        }
        *NumberOfFilteredRecords = nofRecords;
        FFRecordFilterIndex = 0;
        ret = DEM_FILTER_ACCEPTED;
    }
    return ret;
}

/** @req DEM224 */
Dem_ReturnGetNextFilteredDTCType Dem_GetNextFilteredRecord(uint32 *DTC, uint8 *RecordNumber)
{
    Dem_ReturnGetNextFilteredDTCType ret = DEM_FILTERED_NO_MATCHING_DTC;

    EventStatusRecType *eventStatusRecPtr = NULL;
    if (demState == DEM_INITIALIZED) {

        /* Find the next record which has a DTC */
        boolean found = FALSE;
        for( uint16 i = FFRecordFilterIndex; (i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM) && !found; i++  ) {
            if( DEM_EVENT_ID_NULL != priMemFreezeFrameBuffer[i].eventId ) {
                lookupEventStatusRec(priMemFreezeFrameBuffer[i].eventId, &eventStatusRecPtr);
               if( (NULL != eventStatusRecPtr) && (NULL != eventStatusRecPtr->eventParamRef->DTCClassRef)) {
                   /* Found one! */
            	   /* @req DEM225 */
                   *RecordNumber = priMemFreezeFrameBuffer[i].recordNumber;
                   *DTC = eventStatusRecPtr->eventParamRef->DTCClassRef->DTC;
                   /* @req DEM226 */
                   FFRecordFilterIndex = i + 1;
                   found = TRUE;
                   ret = DEM_FILTERED_OK;
               }
            }
        }
    } else {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETNEXTFILTEREDRECORD_ID, DEM_E_UNINIT);
    }

    return ret;
}

#if (DEM_UNIT_TEST == STD_ON)
void getFFDataPreInit(FreezeFrameRecType **buf)
{
	*buf = &preInitFreezeFrameBuffer[0];
	return;
}
void getPriMemFFBufPtr(FreezeFrameRecType **buf)
{
	*buf = &priMemFreezeFrameBuffer[0];
	return;
}

uint32 getCurTimeStamp()
{
	return FF_TimeStamp;
}

void getPriMemEventRecBufPtr(EventStatusRecType **buf)
{
	*buf = &eventStatusBuffer[0];
	return;
}

void getPriMemAgingBufPtr(HealingRecType **buf)
{
	*buf = &priMemAgingBuffer[0];
	return;
}
#endif





/****************
 * OBD-specific *
 ***************/
/*
 * Procedure:	Dem_GetDTCOfOBDFreezeFrame
 * Reentrant:	No
 */
 /* @req OBD_DEM_REQ_3 */
Std_ReturnType Dem_GetDTCOfOBDFreezeFrame(uint8 FrameNumber, uint32* DTC )
{
	FreezeFrameRecType *freezeFrame = NULL;
	const Dem_EventParameterType *eventParameter = NULL;
	Std_ReturnType returnCode = E_NOT_OK;
	uint16 i = 0;

	if (demState == DEM_INITIALIZED){
		/* find the corresponding FF in FF buffer */
		/* @req OBD_DEM_REQ_1 */
		//if(DEM_FREEZEFRAME_RECORD_NUMBER_OBD == FrameNumber){
			for(i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM; i++){
				if((priMemFreezeFrameBuffer[i].eventId != DEM_EVENT_ID_NULL) \
					&& (DEM_FREEZE_FRAME_OBD == priMemFreezeFrameBuffer[i].kind)){
					freezeFrame = &priMemFreezeFrameBuffer[i];
					break;
				}
			}

		//}

		/*if FF found,find the corresponding eventParameter*/
		if(freezeFrame != NULL){
			lookupEventIdParameter(freezeFrame->eventId, &eventParameter);
			if(eventParameter != NULL){
				/* if DTCClass configured,get DTC value */
				if(eventParameter->DTCClassRef != NULL){
					*DTC = eventParameter->DTCClassRef->DTC;
					returnCode = E_OK;
				}
			}

		}

	}
	else{
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETDTCOFOBDFREEZEFRAME_ID, DEM_E_UNINIT);
	}	

	return returnCode;
	
}

/*
 * Procedure:	Dem_GetOBDFreezeFrameData
 * Reentrant:	No
 */
 /* @req OBD_DEM_REQ_2 */
Std_ReturnType Dem_GetOBDFreezeFrameData(uint8 PID, uint8* DestBuffer, uint8* BufSize)
{
	FreezeFrameRecType *freezeFrame = NULL;
	const Dem_EventParameterType *eventParameter = NULL;
	const Dem_FreezeFrameClassType *freezeFrameClass = NULL;
	Std_ReturnType returnCode = E_NOT_OK;
	boolean recNumFound = FALSE;
	boolean pidFound = FALSE;
	uint16 offset = 0;
	uint8 pidDataSize = 0;
	uint16 i = 0;

	if (demState == DEM_INITIALIZED){
		/*find the corresponding FF in FF buffer*/
		for(i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM; i++){
			/* @req OBD_DEM_REQ_1 */
			if((priMemFreezeFrameBuffer[i].eventId != DEM_EVENT_ID_NULL) \
				&& (DEM_FREEZE_FRAME_OBD == priMemFreezeFrameBuffer[i].kind)){
				freezeFrame = &priMemFreezeFrameBuffer[i];
				break;
			}
		}

		/*if FF found,find the corresponding eventParameter*/
		if(freezeFrame != NULL){
			lookupEventIdParameter(freezeFrame->eventId, &eventParameter);
			if(eventParameter != NULL){
				/*find the corresponding FF class*/
				recNumFound = lookupFreezeFrameDataRecNumParam(freezeFrame->recordNumber, eventParameter, &freezeFrameClass);
			}

		}

		/*if FF class found,find the corresponding PID*/
		if(recNumFound == TRUE){
			if(freezeFrameClass->FFKind == DEM_FREEZE_FRAME_OBD){
				if(freezeFrameClass->FFIdClassRef != NULL){
					for(i = 0; (i < DEM_MAX_NR_OF_RECORDS_IN_FREEZEFRAME_DATA) && ((freezeFrameClass->FFIdClassRef[i]->Arc_EOL) == FALSE); i++){
						offset += DEM_PID_IDENTIFIER_SIZE_OF_BYTES;
						if(freezeFrameClass->FFIdClassRef[i]->PidIndentifier == PID){
							pidDataSize = freezeFrameClass->FFIdClassRef[i]->PidOrDidSize;
							pidFound = TRUE;
							break;
						}
						else{
							offset += freezeFrameClass->FFIdClassRef[i]->PidOrDidSize;
						}
					}
				}
				
			}
		}

		if(pidFound == TRUE){
			if(((*BufSize) >= pidDataSize) && (PID == (freezeFrame->data[offset - DEM_PID_IDENTIFIER_SIZE_OF_BYTES])) \
				&& ((offset + pidDataSize) <= (freezeFrame->dataSize)) && ((offset + pidDataSize) <= DEM_MAX_SIZE_FF_DATA)){
				(void)memcpy(DestBuffer, freezeFrame->data + offset, pidDataSize);
				*BufSize = pidDataSize;
				returnCode = E_OK;
			}

		}
	}
	else{
		DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETOBDFREEZEFRAMEDATA_ID, DEM_E_UNINIT);
	}	

	return returnCode;
}


/*
 * Procedure:	getPidData
 * Description:	get OBD FF data,only called by getFreezeFrameData()		
 */
static void getPidData(const Dem_PidOrDidType ** const *pidClassPtr, FreezeFrameRecType **freezeFrame, uint16 *storeIndexPtr)
{
	const Dem_PidOrDidType **FFIdClassRef = NULL;
	Std_ReturnType callbackReturnCode;
	uint16 i = 0;
	uint16 storeIndex = 0;
	uint16 recordSize = 0;	
	FFIdClassRef = *pidClassPtr;
	//get all pids
	for (i = 0; ((i < DEM_MAX_NR_OF_RECORDS_IN_FREEZEFRAME_DATA) && (!FFIdClassRef[i]->Arc_EOL)); i++) {
		if(FFIdClassRef[i]->PidOrDidUsePort == FALSE){
			//get pid length
			recordSize = FFIdClassRef[i]->PidOrDidSize;			
			/* read out the pid data */
			if ((storeIndex + recordSize + DEM_PID_IDENTIFIER_SIZE_OF_BYTES) <= DEM_MAX_SIZE_FF_DATA) {
				/* store PID */
				(*freezeFrame)->data[storeIndex] = FFIdClassRef[i]->PidIndentifier;
				storeIndex++;
				/* store data */
				if(FFIdClassRef[i]->PidReadFnc != NULL){
					callbackReturnCode = FFIdClassRef[i]->PidReadFnc(&(*freezeFrame)->data[storeIndex]);
					if (callbackReturnCode != E_OK) {
						memset(&(*freezeFrame)->data[storeIndex], DEM_FREEZEFRAME_DEFAULT_VALUE, recordSize);
					}
					storeIndex += recordSize;

	 			}
				else{
					memset(&(*freezeFrame)->data[storeIndex], DEM_FREEZEFRAME_DEFAULT_VALUE, recordSize);
					storeIndex += recordSize;
				}						

			}
			else{
				DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GET_FREEZEFRAME_ID, DEM_E_FF_TOO_BIG);
				break;
			}
		}
		else{
			//TODO:RTE should provide the port
			DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GET_FREEZEFRAME_ID, DEM_DSP_DID_USE_PORT_IS_TRUE);
			break;
		}
	}
	//store storeIndex,it will be used for judge whether FF contains valid data.
	*storeIndexPtr = storeIndex;

}
/*
 * Procedure:	getDidData
 * Description:	get UDS FF data,only called by getFreezeFrameData()		
 */
 static void getDidData(const Dem_PidOrDidType ** const* didClassPtr, FreezeFrameRecType **freezeFrame, uint16 *storeIndexPtr)
{
	const Dem_PidOrDidType **FFIdClassRef = NULL;
	Std_ReturnType callbackReturnCode;
	uint16 i = 0;
	uint16 storeIndex = 0;
	uint16 recordSize = 0;
	Dcm_NegativeResponseCodeType errorCode;//should include Dcm_Lcfg.h
	
	FFIdClassRef = *didClassPtr;
	//get all dids
	for (i = 0; ((i < DEM_MAX_NR_OF_RECORDS_IN_FREEZEFRAME_DATA) && (!FFIdClassRef[i]->Arc_EOL)); i++) {
		if(FFIdClassRef[i]->PidOrDidUsePort == FALSE){
			if(FFIdClassRef[i]->DidReadDataLengthFnc != NULL){
				callbackReturnCode = FFIdClassRef[i]->DidReadDataLengthFnc(&recordSize);
				if(callbackReturnCode != E_OK){
					//if fail to read data length,discard the storage of FF
					(*freezeFrame)->eventId = DEM_EVENT_ID_NULL;
					DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GET_FREEZEFRAME_ID, DEM_READ_DATA_LENGTH_FAILED);
					return;
				}
			}
			/* if no readDidDataLengthFunction, then try the PidOrDidSize. */
			else{
				recordSize = FFIdClassRef[i]->PidOrDidSize;
			}
			/* read out the did data */
			if ((storeIndex + recordSize + DEM_DID_IDENTIFIER_SIZE_OF_BYTES) <= DEM_MAX_SIZE_FF_DATA) {
				/* store DID */
				(*freezeFrame)->data[storeIndex] = (FFIdClassRef[i]->DidIdentifier>> 8) & 0xFFu;
				storeIndex++;
				(*freezeFrame)->data[storeIndex] = FFIdClassRef[i]->DidIdentifier & 0xFFu;
				storeIndex++;
				/* store data */
				if(FFIdClassRef[i]->DidConditionCheckReadFnc != NULL){
					callbackReturnCode = FFIdClassRef[i]->DidConditionCheckReadFnc(&errorCode);
					if ((callbackReturnCode == E_OK) && (errorCode == DCM_E_POSITIVERESPONSE)) {
						if(FFIdClassRef[i]->DidReadFnc!= NULL){
							callbackReturnCode = FFIdClassRef[i]->DidReadFnc(&(*freezeFrame)->data[storeIndex]);
							if (callbackReturnCode != E_OK) {
								memset(&(*freezeFrame)->data[storeIndex], DEM_FREEZEFRAME_DEFAULT_VALUE, recordSize);
							}
							storeIndex += recordSize;

			 			}
						else{
							memset(&(*freezeFrame)->data[storeIndex], DEM_FREEZEFRAME_DEFAULT_VALUE, recordSize);
							storeIndex += recordSize;
						}

					}
					else{
						memset(&(*freezeFrame)->data[storeIndex], DEM_FREEZEFRAME_DEFAULT_VALUE, recordSize);
						storeIndex += recordSize;
					}
				}
				else{
					memset(&(*freezeFrame)->data[storeIndex], DEM_FREEZEFRAME_DEFAULT_VALUE, recordSize);
					storeIndex += recordSize;
				}

			}
			else{
				DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GET_FREEZEFRAME_ID, DEM_E_FF_TOO_BIG);
				break;
			}
		}
		else{
			//TODO:RTE should provide the port
			DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GET_FREEZEFRAME_ID, DEM_DSP_DID_USE_PORT_IS_TRUE);
			break;
		}
	}

	//store storeIndex,it will be used for judge whether FF contains valid data.
	*storeIndexPtr = storeIndex;
}


/*
 * Procedure:	storeOBDFreezeFrameDataPreInit
 * Description:	store FF in before  preInitFreezeFrameBuffer DEM's full initialization				
 */
static void storeOBDFreezeFrameDataPreInit(const Dem_EventParameterType * eventParam, 
											     const FreezeFrameRecType * freezeFrame)
{
	boolean eventIdFound = FALSE;
	boolean eventIdFreePositionFound=FALSE;
	FreezeFrameRecType *freezeFrameLocal = NULL;
	uint16 i;
	imask_t state;

	Irq_Save(state);

	/* Check if already stored */
	for (i = 0; (i<DEM_MAX_NUMBER_FF_DATA_PRE_INIT) && (!eventIdFound); i++){
		eventIdFound = ((preInitFreezeFrameBuffer[i].eventId != DEM_EVENT_ID_NULL) \
			&& (preInitFreezeFrameBuffer[i].kind == DEM_FREEZE_FRAME_OBD));
	}

	if(eventIdFound == FALSE){
		/* lookup first free position */
		for (i = 0; (i<DEM_MAX_NUMBER_FF_DATA_PRE_INIT) && (!eventIdFreePositionFound); i++){
			if(preInitFreezeFrameBuffer[i].eventId == DEM_EVENT_ID_NULL){
				eventIdFreePositionFound=TRUE;
			}
		}

		if (eventIdFreePositionFound) {
			memcpy(&preInitFreezeFrameBuffer[i-1], freezeFrame, sizeof(FreezeFrameRecType));
		}
		else {
			/* do displacement */
			if(lookupFreezeFrameForDisplacementPreInit(&freezeFrameLocal)){
				if(freezeFrameLocal != NULL){
					memcpy(freezeFrameLocal, freezeFrame, sizeof(FreezeFrameRecType));
				}				
			}
			else{
				DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_STORE_FF_DATA_PRE_INIT_ID, DEM_E_PRE_INIT_FF_DATA_BUFF_FULL);
			}
		}
		
	}

	Irq_Restore(state);
}

/*
 * Procedure:	storeOBDFreezeFrameDataPriMem
 * Description: store OBD FreezeFrame data record in primary memory
 */
static void storeOBDFreezeFrameDataPriMem(const Dem_EventParameterType *eventParam, const FreezeFrameRecType *freezeFrame)
{
	boolean eventIdFound = FALSE;
	boolean eventIdFreePositionFound=FALSE;
	boolean displacementPositionFound=FALSE;
	FreezeFrameRecType *freezeFrameLocal = NULL;
	uint16 i;
	imask_t state;

	Irq_Save(state);

	/* Check if already stored */
	for (i = 0; (i<DEM_MAX_NUMBER_FF_DATA_PRI_MEM) && (!eventIdFound); i++){
		eventIdFound = ((priMemFreezeFrameBuffer[i].eventId != DEM_EVENT_ID_NULL) \
			&& (priMemFreezeFrameBuffer[i].kind == DEM_FREEZE_FRAME_OBD));
	}

	if (eventIdFound == FALSE) {
		/* find the first free position */
		for (i = 0; (i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM) && (!eventIdFreePositionFound); i++){
			eventIdFreePositionFound =  (priMemFreezeFrameBuffer[i].eventId == DEM_EVENT_ID_NULL);
		}
		/* if found,copy it to this position */
		if (eventIdFreePositionFound) {
			memcpy(&priMemFreezeFrameBuffer[i-1], freezeFrame, sizeof(FreezeFrameRecType));
		}
		else {
			/* if not found,do displacement */
			displacementPositionFound = lookupFreezeFrameForDisplacement(&freezeFrameLocal);
			if(displacementPositionFound == TRUE){
				if(freezeFrameLocal != NULL){
					memcpy(freezeFrameLocal, freezeFrame, sizeof(FreezeFrameRecType));
				}				
			}
			else{
				DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_STORE_FF_DATA_PRI_MEM_ID, DEM_E_PRI_MEM_FF_DATA_BUFF_FULL);
			}
		}
	}

	Irq_Restore(state);
}

/*
 * Procedure:	Dem_GetFreezeFramePids
 * Reentrant:	No
 */
Std_ReturnType Dem_GetFreezeFramePids(uint8* array, uint8* arraySize)
{
	uint16 index = 0;
	Std_ReturnType ret = E_NOT_OK;
	boolean arrayFull = FALSE;
	FreezeFrameRecType *freezeFrame = NULL;
	const Dem_EventParameterType *eventParameter = NULL;
	const Dem_FreezeFrameClassType *freezeFrameClass = NULL;

	if ((demState == DEM_INITIALIZED) && (NULL != array) && (NULL != arraySize))
	{
		/*find the corresponding FF in FF buffer*/
		for(index = 0; (index < DEM_MAX_NUMBER_FF_DATA_PRI_MEM) && (NULL == freezeFrame); index++)
		{
			if((priMemFreezeFrameBuffer[index].eventId != DEM_EVENT_ID_NULL)
				&& (0 == priMemFreezeFrameBuffer[index].recordNumber))
			{
				freezeFrame = &priMemFreezeFrameBuffer[index];
			}
		}

		/*if FF found,find the corresponding eventParameter*/
		if(freezeFrame != NULL)
		{
			lookupEventIdParameter(freezeFrame->eventId, &eventParameter);
			if((eventParameter != NULL)
				&& (TRUE == lookupFreezeFrameDataRecNumParam(0, eventParameter, &freezeFrameClass))
				&& (NULL != freezeFrameClass->FFIdClassRef))
			{
				for(index = 0; (index < DEM_MAX_NR_OF_RECORDS_IN_FREEZEFRAME_DATA)
				&& ((freezeFrameClass->FFIdClassRef[index]->Arc_EOL) == FALSE) && (FALSE == arrayFull); index++)
				{
					if(*arraySize > index)
					{
						/* save PIDs to array. */
						array[index] = freezeFrameClass->FFIdClassRef[index]->PidIndentifier;
					}
					else
					{
						/* array is full. */
						arrayFull = TRUE;
					}
				}
				/* if successful to get PIDs,set arraySize with actual size and set E_Ok. */
				if((FALSE == arrayFull) && (index > 0))
				{
					*arraySize = index - 1;
					ret = E_OK;
				}
			}

		}
		else
		{
			/* if no found, set arraySize zero and return E_OK. */
			*arraySize = 0;
			ret = E_OK;
		}

	}

	return ret;

}

