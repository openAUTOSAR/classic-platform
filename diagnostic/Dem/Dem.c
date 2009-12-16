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
#include "Dem.h"
#include "Det.h"
//#include "Fim.h"
//#include "Nvm.h"
//#include "SchM_Dem.h"
#include "MemMap.h"
#include "Mcu.h"

/*
 * Local types
 */

typedef uint16 ChecksumType;

// For keeping track of the events status
typedef struct {
	Dem_EventIdType				eventId;
	uint16						occurrence;
	Dem_EventStatusType			eventStatus;
	boolean						eventStatusChanged;
	Dem_OperationCycleType		operationCycleId;
	Dem_EventStatusExtendedType	eventStatusExtended;
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
} Dem_StateType;

static Dem_StateType demState = DEM_UNINITIALIZED;

// Help pointer to configuration set
static const Dem_ConfigSetType *configSet;

#if (DEM_VERSION_INFO_API == STD_ON)
static Std_VersionInfoType _Dem_VersionInfo =
{
  .vendorID   = (uint16)1,
  .moduleID   = (uint16)1,
  .instanceID = (uint8)1,
  .sw_major_version = (uint8)DEM_SW_MAJOR_VERSION,
  .sw_minor_version = (uint8)DEM_SW_MINOR_VERSION,
  .sw_patch_version = (uint8)DEM_SW_PATCH_VERSION,
  .ar_major_version = (uint8)DEM_AR_MAJOR_VERSION,
  .ar_minor_version = (uint8)DEM_AR_MINOR_VERSION,
  .ar_patch_version = (uint8)DEM_AR_PATCH_VERSION,
};
#endif /* DEM_VERSION_INFO_API */

/*
 * Allocation of operation cycle state list
 */

static Dem_OperationCycleStateType operationCycleStateList[DEM_OPERATION_CYCLE_TYPE_ENDMARK];
/*
 * Allocation of local event status buffer
 */
static EventStatusRecType	eventStatusBuffer[DEM_MAX_NUMBER_EVENT];

/*
 * Allocation of pre-init event memory (used between pre-init and init)
 */
static FreezeFrameRecType	preInitFreezeFrameBuffer[DEM_MAX_NUMBER_FF_DATA_PRE_INIT];
static ExtDataRecType		preInitExtDataBuffer[DEM_MAX_NUMBER_EXT_DATA_PRE_INIT];

/*
 * Allocation of primary event memory ramlog (after init)
 */
static EventRecType 		priMemEventBuffer[DEM_MAX_NUMBER_EVENT_PRI_MEM];
static FreezeFrameRecType	priMemFreezeFrameBuffer[DEM_MAX_NUMBER_FF_DATA_PRI_MEM];
static ExtDataRecType		priMemExtDataBuffer[DEM_MAX_NUMBER_EXT_DATA_PRI_MEM];


/*
 * Procedure:	setZero
 * Description:	Fill the *ptr to *(ptr+nrOfBytes-1) area with zeroes
 */
void setZero(void *ptr, uint16 nrOfBytes)
{
	uint8 *clrPtr = (uint8*)ptr;

	if (nrOfBytes > 0)
	{
		*clrPtr = 0x00;
		memcpy(clrPtr+1, clrPtr, nrOfBytes-1);
	}
}

/*
 * Procedure:	zeroPriMemBuffers
 * Description:	Fill the primary buffers with zeroes
 */
void zeroPriMemBuffers(void)
{
	setZero(priMemEventBuffer, sizeof(priMemEventBuffer));
	setZero(priMemFreezeFrameBuffer, sizeof(priMemFreezeFrameBuffer));
	setZero(priMemExtDataBuffer, sizeof(priMemExtDataBuffer));
}

/*
 * Procedure:	calcChecksum
 * Description:	Calculate checksum over *data to *(data+nrOfBytes-1) area
 */
ChecksumType calcChecksum(void *data, uint16 nrOfBytes)
{
	uint16 i;
	uint8 *ptr = (uint8*)data;
	ChecksumType sum = 0;

	for (i = 0; i < nrOfBytes; i++)
		sum += *ptr++;
	sum ^= 0xaaaa;
	return sum;
}

/*
 * Procedure:	updateEventStatusRec
 * Description:	Update the status of "eventId", if not exist and "createIfNotExist" is
 * 				true a new record is created
 */
void updateEventStatusRec(const Dem_EventParameterType *eventParam, Dem_EventStatusType eventStatus, boolean createIfNotExist, EventStatusRecType *eventStatusRec)
{
	uint16 i;
	imask_t state = McuE_EnterCriticalSection();

	// Lookup event ID
	for (i = 0; (eventStatusBuffer[i].eventId != eventParam->EventID) && (i < DEM_MAX_NUMBER_EVENT); i++);

	if ((i == DEM_MAX_NUMBER_EVENT) && (createIfNotExist)) {
		// Search for free position
		for (i = 0; (eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL) && (i < DEM_MAX_NUMBER_EVENT); i++);

		if (i < DEM_MAX_NUMBER_EVENT) {
			// Create new event record
			eventStatusBuffer[i].eventId = eventParam->EventID;
			eventStatusBuffer[i].occurrence = 0;
			eventStatusBuffer[i].eventStatus = DEM_EVENT_STATUS_PASSED;
			eventStatusBuffer[i].eventStatusChanged = FALSE;
			eventStatusBuffer[i].operationCycleId = eventParam->EventClass->OperationCycleRef;
			eventStatusBuffer[i].eventStatusExtended = DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR | DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE;
		}
		else {
			// Error: Event status buffer full
			Det_ReportError(MODULE_ID_DEM, 0, DEM_UPDATE_EVENT_STATUS_ID, DEM_E_EVENT_STATUS_BUFF_FULL);
		}
	}

	if (i < DEM_MAX_NUMBER_EVENT) {
		// Update event record
		eventStatusBuffer[i].eventStatusExtended &= ~(DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR | DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE);

		if (eventStatus == DEM_EVENT_STATUS_FAILED) {
			eventStatusBuffer[i].eventStatusExtended |= (DEM_TEST_FAILED | DEM_TEST_FAILED_THIS_OPERATION_CYCLE | DEM_TEST_FAILED_SINCE_LAST_CLEAR);
			if	 (eventStatusBuffer[i].eventStatus != eventStatus) {
				eventStatusBuffer[i].occurrence++;
			}
		}

		if (eventStatus == DEM_EVENT_STATUS_PASSED) {
			eventStatusBuffer[i].eventStatusExtended &= ~DEM_TEST_FAILED;
		}

		if	 (eventStatusBuffer[i].eventStatus != eventStatus) {
			eventStatusBuffer[i].eventStatus = eventStatus;
			eventStatusBuffer[i].eventStatusChanged = TRUE;
		}
		else {
			eventStatusBuffer[i].eventStatusChanged = FALSE;
		}

		// Copy the record
		memcpy(eventStatusRec, &eventStatusBuffer[i], sizeof(EventStatusRecType));
	}
	else {
		// Copy an empty record to return data
		eventStatusRec->eventId = DEM_EVENT_ID_NULL;
		eventStatusRec->eventStatus = DEM_EVENT_STATUS_PASSED;
		eventStatusRec->occurrence = 0;
		eventStatusBuffer[i].eventStatusChanged = FALSE;
		eventStatusBuffer[i].eventStatusExtended = DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE | DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR;
	}


	McuE_ExitCriticalSection(state);
}


/*
 * Procedure:	mergeEventStatusRec
 * Description:	Update the occurrence counter of status, if not exist a new record is created
 */
void mergeEventStatusRec(EventRecType *eventRec)
{
	uint16 i;
	imask_t state = McuE_EnterCriticalSection();

	// Lookup event ID
	for (i = 0; (eventStatusBuffer[i].eventId != eventRec->eventId) && (i < DEM_MAX_NUMBER_EVENT); i++);

	if (i < DEM_MAX_NUMBER_EVENT) {
		// Update occurrence counter, rest of pre init state is kept.
		eventStatusBuffer[i].occurrence += eventRec->occurrence;

	}
	else {
		// Search for free position
		for (i = 0; (eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL) && (i < DEM_MAX_NUMBER_EVENT); i++);

		if (i < DEM_MAX_NUMBER_EVENT) {
			// Create new event, from stored event
			eventStatusBuffer[i].eventId = eventRec->eventId;
			eventStatusBuffer[i].occurrence = eventRec->occurrence;
			eventStatusBuffer[i].eventStatus = DEM_EVENT_STATUS_PASSED;
			eventStatusBuffer[i].eventStatusChanged = FALSE;
		}
		else {
			// Error: Event status buffer full
			Det_ReportError(MODULE_ID_DEM, 0, DEM_MERGE_EVENT_STATUS_ID, DEM_E_EVENT_STATUS_BUFF_FULL);
		}
	}

	McuE_ExitCriticalSection(state);
}

/*
 * Procedure:	getEventStatusRec
 * Description:	Returns the status record of "eventId" in "eventStatusRec"
 */
void getEventStatusRec(Dem_EventIdType eventId, EventStatusRecType *eventStatusRec)
{
	uint16 i;
	// Lookup event ID
	for (i = 0; (eventStatusBuffer[i].eventId != eventId) && (i < DEM_MAX_NUMBER_EVENT); i++);

	if (i < DEM_MAX_NUMBER_EVENT) {
		// Copy the record
		memcpy(eventStatusRec, &eventStatusBuffer[i], sizeof(EventStatusRecType));
	}
	else {
		eventStatusRec->eventId = DEM_EVENT_ID_NULL;
		eventStatusRec->eventStatus = DEM_EVENT_STATUS_PASSED;
		eventStatusRec->occurrence = 0;
	}
}


/*
 * Procedure:	lookupEventIdParameter
 * Description:	Returns the pointer to event id parameters of "eventId" in "*eventIdParam",
 * 				if not found NULL is returned.
 */
void lookupEventIdParameter(Dem_EventIdType eventId, const Dem_EventParameterType **const eventIdParam)
{
	uint16 i;
	*eventIdParam = NULL;

	// Lookup the correct event id parameters
	for (i = 0; !configSet->EventParameter[i].Arc_EOL; i++) {
		if (configSet->EventParameter[i].EventID == eventId) {
			*eventIdParam = &configSet->EventParameter[i];
			return;
		}
	}
	// Id not found return with NULL pointer
}


void getFreezeFrameData(const Dem_EventParameterType *eventParam, FreezeFrameRecType *freezeFrame)
{
	// TODO: Fill out
}


void storeFreezeFrameDataPreInit(const Dem_EventParameterType *eventParam, FreezeFrameRecType *freezeFrame)
{
	// TODO: Fill out
}


void updateFreezeFrameOccurrencePreInit(EventRecType *EventBuffer)
{
	// TODO: Fill out
}

/*
 * Procedure:	getExtendedData
 * Description:	Collects the extended data according to "eventParam" and return it in "extData",
 * 				if not found eventId is set to DEM_EVENT_ID_NULL.
 */
void getExtendedData(const Dem_EventParameterType *eventParam, ExtDataRecType *extData)
{
	Std_ReturnType callbackReturnCode;
	uint16 i;
	uint16 storeIndex = 0;
	uint16 recordSize;

	// Clear ext data record
	setZero(extData, sizeof(ExtDataRecType));

	// Check if any pointer to extended data class
	if (eventParam->ExtendedDataClassRef != NULL) {
		// Request extended data and copy it to the buffer
		for (i = 0; (i < DEM_MAX_NR_OF_RECORDS_IN_EXTENDED_DATA) && (eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i] != NULL); i++) {
			recordSize = eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i]->DataSize;
			if ((storeIndex + recordSize) <= DEM_MAX_SIZE_EXT_DATA) {
				callbackReturnCode = eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i]->CallbackGetExtDataRecord(&extData->data[storeIndex]);
				if (callbackReturnCode != E_OK) {
					// Callback data currently not available, clear space.
					setZero(&extData->data[storeIndex], recordSize);
				}
				storeIndex += recordSize;
			}
			else {
				// Error: Size of extended data record is bigger than reserved space.
				Det_ReportError(MODULE_ID_DEM, 0, DEM_GET_EXTENDED_DATA_ID, DEM_E_EXT_DATA_TO_BIG);
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
 * Description:	Store the extended data pointed by "extendedData" to the "preInitExtDataBuffer"
 */
void storeExtendedDataPreInit(const Dem_EventParameterType *eventParam, ExtDataRecType *extendedData)
{
	uint16 i;
	imask_t state = McuE_EnterCriticalSection();

	// Lookup first free position
	for (i = 0; (preInitExtDataBuffer[i].eventId !=0) && (i<DEM_MAX_NUMBER_EXT_DATA_PRE_INIT); i++);

	if (i < DEM_MAX_NUMBER_EXT_DATA_PRE_INIT) {
		memcpy(&preInitExtDataBuffer[i], extendedData, sizeof(ExtDataRecType));
	}
	else {
		// Error: Pre init extended data buffer full
		Det_ReportError(MODULE_ID_DEM, 0, DEM_STORE_EXT_DATA_PRE_INIT_ID, DEM_E_PRE_INIT_EXT_DATA_BUFF_FULL);
	}

	McuE_ExitCriticalSection(state);
}


/*
 * Procedure:	storeEventPriMem
 * Description:	Store the event data of "eventStatus->eventId" in "priMemEventBuffer",
 * 				if non existent a new entry is created.
 */
void storeEventPriMem(const Dem_EventParameterType *eventParam, EventStatusRecType *eventStatus)
{
	uint16 i;
	imask_t state = McuE_EnterCriticalSection();


	// Lookup event ID
	for (i = 0; (priMemEventBuffer[i].eventId != eventStatus->eventId) && (i < DEM_MAX_NUMBER_EVENT_ENTRY_PRI); i++);

	if (i < DEM_MAX_NUMBER_EVENT_ENTRY_PRI) {
		// Update event found
		priMemEventBuffer[i].occurrence = eventStatus->occurrence;
		priMemEventBuffer[i].checksum = calcChecksum(&priMemEventBuffer[i], sizeof(EventRecType)-sizeof(ChecksumType));
	}
	else {
		// Search for free position
		for (i=0; (priMemEventBuffer[i].eventId != DEM_EVENT_ID_NULL) && (i < DEM_MAX_NUMBER_EVENT_ENTRY_PRI); i++);

		if (i < DEM_MAX_NUMBER_EVENT_ENTRY_PRI) {
			priMemEventBuffer[i].eventId = eventStatus->eventId;
			priMemEventBuffer[i].occurrence = eventStatus->occurrence;
			priMemEventBuffer[i].checksum = calcChecksum(&priMemEventBuffer[i], sizeof(EventRecType)-sizeof(ChecksumType));
		}
		else {
			// Error: Pri mem event buffer full
			Det_ReportError(MODULE_ID_DEM, 0, DEM_STORE_EVENT_PRI_MEM_ID, DEM_E_PRI_MEM_EVENT_BUFF_FULL);
		}
	}

	McuE_ExitCriticalSection(state);
}


/*
 * Procedure:	storeEventEvtMem
 * Description:	Store the event data of "eventStatus->eventId" in event memory according to
 * 				"eventParam" destination option.
 */
void storeEventEvtMem(const Dem_EventParameterType *eventParam, EventStatusRecType *eventStatus)
{
	uint16 i;

	for (i = 0; (i < DEM_MAX_NR_OF_EVENT_DESTINATION) && (eventParam->EventClass->EventDestination[i] != NULL); i++) {
		switch (eventParam->EventClass->EventDestination[i])
		{
		case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
			storeEventPriMem(eventParam, eventStatus);
			break;

		case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
		case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
		case DEM_DTC_ORIGIN_MIRROR_MEMORY:
			// Not yet supported
			Det_ReportError(MODULE_ID_DEM, 0, DEM_GLOBAL_ID, DEM_E_NOT_IMPLEMENTED_YET);
			break;
		default:
			break;
		}
	}
}


/*
 * Procedure:	storeExtendedDataPriMem
 * Description:	Creates new an extended data record in "priMemExtDataBuffer".
 */
void storeExtendedDataPriMem(const Dem_EventParameterType *eventParam, ExtDataRecType *extendedData)
{
	uint16 i;
	imask_t state = McuE_EnterCriticalSection();

	// Lookup first free position
	for (i = 0; (priMemExtDataBuffer[i].eventId != DEM_EVENT_ID_NULL) && (i < DEM_MAX_NUMBER_EXT_DATA_PRI_MEM); i++);
	if (i < DEM_MAX_NUMBER_EXT_DATA_PRI_MEM) {
		memcpy(&priMemExtDataBuffer[i], extendedData, sizeof(ExtDataRecType));
	}
	else {
		// Error: Pri mem extended data buffer full
		Det_ReportError(MODULE_ID_DEM, 0, DEM_STORE_EXT_DATA_PRI_MEM_ID, DEM_E_PRI_MEM_EXT_DATA_BUFF_FULL);
	}

	McuE_ExitCriticalSection(state);
}


/*
 * Procedure:	storeExtendedDataEvtMem
 * Description:	Store the extended data in event memory according to
 * 				"eventParam" destination option
 */
void storeExtendedDataEvtMem(const Dem_EventParameterType *eventParam, ExtDataRecType *extendedData)
{
	uint16 i;

	for (i = 0; (i < DEM_MAX_NR_OF_EVENT_DESTINATION) && (eventParam->EventClass->EventDestination[i] != NULL); i++) {
		switch (eventParam->EventClass->EventDestination[i])
		{
		case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
			storeExtendedDataPriMem(eventParam, extendedData);
			break;

		case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
		case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
		case DEM_DTC_ORIGIN_MIRROR_MEMORY:
			// Not yet supported
			Det_ReportError(MODULE_ID_DEM, 0, DEM_GLOBAL_ID, DEM_E_NOT_IMPLEMENTED_YET);
			break;
		default:
			break;
		}
	}
}


void storeFreezeFrameDataPriMem(const Dem_EventParameterType *eventParam, FreezeFrameRecType *freezeFrame)
{
	// TODO: Fill out
}


/*
 * Procedure:	storeFreezeFrameDataEvtMem
 * Description:	Store the freeze frame data in event memory according to
 * 				"eventParam" destination option
 */
void storeFreezeFrameDataEvtMem(const Dem_EventParameterType *eventParam, FreezeFrameRecType *freezeFrame)
{
	uint16 i;

	for (i = 0; (i < DEM_MAX_NR_OF_EVENT_DESTINATION) && (eventParam->EventClass->EventDestination[i] != NULL); i++) {
		switch (eventParam->EventClass->EventDestination[i])
		{
		case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
			storeFreezeFrameDataPriMem(eventParam, freezeFrame);
			break;

		case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
		case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
		case DEM_DTC_ORIGIN_MIRROR_MEMORY:
			// Not yet supported
			Det_ReportError(MODULE_ID_DEM, 0, DEM_GLOBAL_ID, DEM_E_NOT_IMPLEMENTED_YET);
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
void handlePreInitEvent(Dem_EventIdType eventId, Dem_EventStatusType eventStatus)
{
	const Dem_EventParameterType *eventParam;
	EventStatusRecType eventStatusLocal;
	FreezeFrameRecType freezeFrameLocal;
	ExtDataRecType extendedDataLocal;

	// Find configuration for this event
	lookupEventIdParameter(eventId, &eventParam);
	if (eventParam != NULL) {
		if (eventParam->EventClass->OperationCycleRef < DEM_OPERATION_CYCLE_TYPE_ENDMARK) {
			if (operationCycleStateList[eventParam->EventClass->OperationCycleRef] == DEM_CYCLE_STATE_START) {
				if (eventStatus == DEM_EVENT_STATUS_PASSED) {
					updateEventStatusRec(eventParam, eventStatus, FALSE, &eventStatusLocal);
				}
				else {
					updateEventStatusRec(eventParam, eventStatus, TRUE, &eventStatusLocal);
				}

				if (eventStatusLocal.eventStatusChanged) {

					if (eventStatusLocal.eventStatus == DEM_EVENT_STATUS_FAILED) {
						// Collect freeze frame data
						getFreezeFrameData(eventParam, &freezeFrameLocal);
						storeFreezeFrameDataPreInit(eventParam, &freezeFrameLocal);

						// Check if first time -> store extended data
						if (eventStatusLocal.occurrence == 1) {
							// Collect extended data
							getExtendedData(eventParam, &extendedDataLocal);
							if (extendedDataLocal.eventId != DEM_EVENT_ID_NULL)
							{
								storeExtendedDataPreInit(eventParam, &extendedDataLocal);
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
Std_ReturnType handleEvent(Dem_EventIdType eventId, Dem_EventStatusType eventStatus)
{
	Std_ReturnType returnCode = E_OK;
	const Dem_EventParameterType *eventParam;
	EventStatusRecType eventStatusLocal;
	FreezeFrameRecType freezeFrameLocal;
	ExtDataRecType extendedDataLocal;

	// Find configuration for this event
	lookupEventIdParameter(eventId, &eventParam);
	if (eventParam != NULL) {
		if (eventParam->EventClass->OperationCycleRef < DEM_OPERATION_CYCLE_TYPE_ENDMARK) {
			if (operationCycleStateList[eventParam->EventClass->OperationCycleRef] == DEM_CYCLE_STATE_START) {
				updateEventStatusRec(eventParam, eventStatus, TRUE, &eventStatusLocal);

				if (eventStatusLocal.eventStatusChanged) {

					if (eventStatusLocal.eventStatus == DEM_EVENT_STATUS_FAILED) {
						storeEventEvtMem(eventParam, &eventStatusLocal);
						// Collect freeze frame data
						getFreezeFrameData(eventParam, &freezeFrameLocal);
						storeFreezeFrameDataEvtMem(eventParam, &freezeFrameLocal);

						// Check if first time -> store extended data
						if (eventStatusLocal.occurrence == 1) {
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
 * Procedure:	getEventStatus
 * Description:	Returns the extended event status bitmask of eventId in "eventStatusExtended".
 */
void getEventStatus(Dem_EventIdType eventId, Dem_EventStatusExtendedType *eventStatusExtended)
{
	EventStatusRecType eventStatusLocal;

	// Get recorded status
	getEventStatusRec(eventId, &eventStatusLocal);
	if (eventStatusLocal.eventId == eventId) {
		*eventStatusExtended = eventStatusLocal.eventStatusExtended;
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
void getEventFailed(Dem_EventIdType eventId, boolean *eventFailed)
{
	EventStatusRecType eventStatusLocal;

	// Get recorded status
	getEventStatusRec(eventId, &eventStatusLocal);
	if (eventStatusLocal.eventId == eventId) {
		if (eventStatusLocal.eventStatusExtended & DEM_TEST_FAILED) {
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
void getEventTested(Dem_EventIdType eventId, boolean *eventTested)
{
	EventStatusRecType eventStatusLocal;

	// Get recorded status
	getEventStatusRec(eventId, &eventStatusLocal);
	if (eventStatusLocal.eventId == eventId) {
		if ( !(eventStatusLocal.eventStatusExtended & DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE)) {
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
 * 			the counter was available else E_NOT_OK.
 */
Std_ReturnType getFaultDetectionCounter(Dem_EventIdType eventId, sint8 *counter)
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
					returnCode = eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceAlgorithm.PreDebounceMonitorInternal->CallbackGetFDCnt(counter);
				}
				break;

			case DEM_PRE_DEBOUNCE_COUNTER_BASED:
			case DEM_PRE_DEBOUNCE_FREQUENCY_BASED:
			case DEM_PRE_DEBOUNCE_TIME_BASED:
#if (DEM_DEV_ERROR_DETECT == STD_ON)
				Det_ReportError(MODULE_ID_DEM, 0, DEM_GETFAULTDETECTIONCOUNTER_ID, DEM_E_NOT_IMPLEMENTED_YET);
#endif
				break;

			default:
#if (DEM_DEV_ERROR_DETECT == STD_ON)
				Det_ReportError(MODULE_ID_DEM, 0, DEM_GETFAULTDETECTIONCOUNTER_ID, DEM_E_PARAM_DATA);
#endif
				break;
			}
		}
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
#if (DEM_VERSION_INFO_API == STD_ON)
void Dem_GetVersionInfo(Std_VersionInfoType *versionInfo) {
	memcpy(versionInfo, &_Dem_VersionInfo, sizeof(Std_VersionInfoType));
}
#endif /* DEM_VERSION_INFO_API */


/***********************************************
 * Interface ECU State Manager <-> DEM (8.3.2) *
 ***********************************************/

/*
 * Procedure:	Dem_PreInit
 * Reentrant:	No
 */
void Dem_PreInit(void)
{
	int i, j;

	if (DEM_Config.ConfigSet == NULL) {
		Det_ReportError(MODULE_ID_DEM, 0, DEM_PREINIT_ID, DEM_E_CONFIG_PTR_INVALID);
		return;
	} else {
		configSet = DEM_Config.ConfigSet;
	}

	// Initializion of operation cycle states.
	for (i = 0; i < DEM_OPERATION_CYCLE_TYPE_ENDMARK; i++) {
		operationCycleStateList[i] = DEM_CYCLE_STATE_END;
	}

	// Initialize the event status buffer
	for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
		eventStatusBuffer[i].eventId = DEM_EVENT_ID_NULL;
		eventStatusBuffer[i].occurrence = 0;
		eventStatusBuffer[i].eventStatusExtended = DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE | DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR;
		eventStatusBuffer[i].eventStatus = DEM_EVENT_STATUS_PASSED;
		eventStatusBuffer[i].eventStatusChanged = FALSE;
	}

	for (i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRE_INIT; i++) {
		preInitFreezeFrameBuffer[i].checksum = 0;
		preInitFreezeFrameBuffer[i].eventId = DEM_EVENT_ID_NULL;
		preInitFreezeFrameBuffer[i].occurrence = 0;
		preInitFreezeFrameBuffer[i].dataSize = 0;
		for (j = 0; j < DEM_MAX_SIZE_FF_DATA;j++)
			preInitFreezeFrameBuffer[i].data[j] = 0;
	}

	for (i = 0; i < DEM_MAX_NUMBER_EXT_DATA_PRE_INIT; i++) {
		preInitExtDataBuffer[i].checksum = 0;
		preInitExtDataBuffer[i].eventId = DEM_EVENT_ID_NULL;
		preInitExtDataBuffer[i].dataSize = 0;
		for (j = 0; j < DEM_MAX_SIZE_EXT_DATA;j++)
			preInitExtDataBuffer[i].data[j] = 0;
	}
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
	EventStatusRecType eventStatusLocal;
	const Dem_EventParameterType *eventParam;

	/*
	 *  Validate and read out saved error log from non volatile memory
	 */

	// Validate event records stored in primary memory
	for (i = 0; i < DEM_MAX_NUMBER_EVENT_PRI_MEM; i++) {
		cSum = calcChecksum(&priMemEventBuffer[i], sizeof(EventRecType)-sizeof(ChecksumType));
		if ((cSum != priMemEventBuffer[i].checksum) || priMemEventBuffer[i].eventId == DEM_EVENT_ID_NULL) {
			// Not valid, clear the record
			setZero(&priMemEventBuffer[i], sizeof(EventRecType));
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
		if ((cSum != priMemExtDataBuffer[i].checksum) || priMemExtDataBuffer[i].eventId == DEM_EVENT_ID_NULL) {
			// Unlegal record, clear the record
			setZero(&priMemExtDataBuffer[i], sizeof(ExtDataRecType));
		}
	}

	// Validate freeze frame records stored in primary memory
	for (i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM; i++) {
		cSum = calcChecksum(&priMemFreezeFrameBuffer[i], sizeof(FreezeFrameRecType)-sizeof(ChecksumType));
		if ((cSum != priMemFreezeFrameBuffer[i].checksum) || (priMemFreezeFrameBuffer[i].eventId == DEM_EVENT_ID_NULL)) {
			// Wrong checksum, clear the record
			setZero(&priMemFreezeFrameBuffer[i], sizeof(FreezeFrameRecType));
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
			getEventStatusRec(preInitExtDataBuffer[i].eventId, &eventStatusLocal);
			// Check if new or old error event
			if (eventStatusLocal.occurrence == 1) {
				// It has not been stored before so store it.
				lookupEventIdParameter(preInitExtDataBuffer[i].eventId, &eventParam);
				storeExtendedDataEvtMem(eventParam, &preInitExtDataBuffer[i]);
			}
		}
	}

	// Transfer freeze frames to event memory
	for (i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRE_INIT; i++) {
		if (preInitFreezeFrameBuffer[i].eventId != DEM_EVENT_ID_NULL) {
			lookupEventIdParameter(preInitFreezeFrameBuffer[i].eventId, &eventParam);
			storeFreezeFrameDataEvtMem(eventParam, &preInitFreezeFrameBuffer[i]);
		}
	}

	demState = DEM_INITIALIZED;
}


/*
 * Procedure:	Dem_shutdown
 * Reentrant:	No
 */
void Dem_shutdown(void)
{

	demState = DEM_UNINITIALIZED;
}


/*
 * Interface for basic software scheduler
 */
void Dem_MainFunction(void)
{

}


/***************************************************
 * Interface SW-Components via RTE <-> DEM (8.3.3) *
 ***************************************************/

/*
 * Procedure:	Dem_SetEventStatus
 * Reentrant:	Yes
 */
Std_ReturnType Dem_SetEventStatus(Dem_EventIdType eventId, Dem_EventStatusType eventStatus)
{
	Std_ReturnType returnCode = E_OK;

	if (demState == DEM_INITIALIZED) // No action is taken if the module is not started
	{
		returnCode = handleEvent(eventId, eventStatus);
	}
	else
	{
#if (DEM_DEV_ERROR_DETECT == STD_ON)
		Det_ReportError(MODULE_ID_DEM, 0, DEM_SETEVENTSTATUS_ID, DEM_E_UNINIT);
		returnCode = E_NOT_OK;
#endif
	}

	return returnCode;
}


/*
 * Procedure:	Dem_ResetEventStatus
 * Reentrant:	Yes
 */
Std_ReturnType Dem_ResetEventStatus(Dem_EventIdType eventId)
{
	const Dem_EventParameterType *eventParam;
	EventStatusRecType eventStatusLocal;
	Std_ReturnType returnCode = E_OK;

	if (demState == DEM_INITIALIZED) // No action is taken if the module is not started
	{
		lookupEventIdParameter(eventId, &eventParam);
		if (eventParam != NULL) {
			updateEventStatusRec(eventParam, DEM_EVENT_STATUS_PASSED, FALSE, &eventStatusLocal);
		}
	}
	else
	{
#if (DEM_DEV_ERROR_DETECT == STD_ON)
		Det_ReportError(MODULE_ID_DEM, 0, DEM_RESETEVENTSTATUS_ID, DEM_E_UNINIT);
#endif
		returnCode = E_NOT_OK;
	}

	return returnCode;
}


Std_ReturnType Dem_GetEventStatus(Dem_EventIdType eventId, Dem_EventStatusExtendedType *eventStatusExtended)
{
	Std_ReturnType returnCode = E_OK;

	if (demState == DEM_INITIALIZED) // No action is taken if the module is not started
	{
		getEventStatus(eventId, eventStatusExtended);
	}
	else
	{
#if (DEM_DEV_ERROR_DETECT == STD_ON)
		Det_ReportError(MODULE_ID_DEM, 0, DEM_GETEVENTSTATUS_ID, DEM_E_UNINIT);
#endif
		returnCode = E_NOT_OK;
	}

	return returnCode;
}


Std_ReturnType Dem_GetEventFailed(Dem_EventIdType eventId, boolean *eventFailed)
{
	Std_ReturnType returnCode = E_OK;

	if (demState == DEM_INITIALIZED) // No action is taken if the module is not started
	{
		getEventFailed(eventId, eventFailed);
	}
	else
	{
#if (DEM_DEV_ERROR_DETECT == STD_ON)
		Det_ReportError(MODULE_ID_DEM, 0, DEM_GETEVENTFAILED_ID, DEM_E_UNINIT);
#endif
		returnCode = E_NOT_OK;
	}

	return returnCode;
}


Std_ReturnType Dem_GetEventTested(Dem_EventIdType eventId, boolean *eventTested)
{
	Std_ReturnType returnCode = E_OK;

	if (demState == DEM_INITIALIZED) // No action is taken if the module is not started
	{
		getEventTested(eventId, eventTested);
	}
	else
	{
#if (DEM_DEV_ERROR_DETECT == STD_ON)
		Det_ReportError(MODULE_ID_DEM, 0, DEM_GETEVENTTESTED_ID, DEM_E_UNINIT);
#endif
		returnCode = E_NOT_OK;
	}

	return returnCode;
}


Std_ReturnType Dem_GetFaultDetectionCounter(Dem_EventIdType eventId, sint8 *counter)
{
	Std_ReturnType returnCode = E_OK;

	if (demState == DEM_INITIALIZED) // No action is taken if the module is not started
	{
		returnCode = getFaultDetectionCounter(eventId, counter);
	}
	else
	{
#if (DEM_DEV_ERROR_DETECT == STD_ON)
		Det_ReportError(MODULE_ID_DEM, 0, DEM_GETFAULTDETECTIONCOUNTER_ID, DEM_E_UNINIT);
#endif
		returnCode = E_NOT_OK;
	}

	return returnCode;
}


Std_ReturnType Dem_SetOperationCycleState(Dem_OperationCycleType operationCycleId, Dem_OperationCycleStateType cycleState)
{
	uint16 i;
	Std_ReturnType returnCode = E_OK;

	if (demState == DEM_INITIALIZED) // No action is taken if the module is not started
	{
		if (operationCycleId < DEM_OPERATION_CYCLE_TYPE_ENDMARK) {
			switch (cycleState)
			{
			case DEM_CYCLE_STATE_START:
				operationCycleStateList[operationCycleId] = cycleState;
				// Lookup event ID
				for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
					if ((eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL) && (eventStatusBuffer[i].operationCycleId == operationCycleId)) {
						eventStatusBuffer[i].eventStatusExtended &= ~DEM_TEST_FAILED_THIS_OPERATION_CYCLE;
						eventStatusBuffer[i].eventStatusExtended |= DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE;
					}
				}
				break;

			case DEM_CYCLE_STATE_END:
				operationCycleStateList[operationCycleId] = cycleState;
				break;
			default:
#if (DEM_DEV_ERROR_DETECT == STD_ON)
				Det_ReportError(MODULE_ID_DEM, 0, DEM_SETOPERATIONCYCLESTATE_ID, DEM_E_PARAM_DATA);
#endif
				returnCode = E_NOT_OK;
				break;
			}
		}
		else {
#if (DEM_DEV_ERROR_DETECT == STD_ON)
			Det_ReportError(MODULE_ID_DEM, 0, DEM_SETOPERATIONCYCLESTATE_ID, DEM_E_PARAM_DATA);
#endif
			returnCode = E_NOT_OK;
		}
	}
	else
	{
#if (DEM_DEV_ERROR_DETECT == STD_ON)
		Det_ReportError(MODULE_ID_DEM, 0, DEM_SETOPERATIONCYCLESTATE_ID, DEM_E_UNINIT);
#endif
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
void Dem_ReportErrorStatus( Dem_EventIdType eventId, Dem_EventStatusType eventStatus )
{

	switch (demState) {
		case DEM_PREINITIALIZED:
			// Update status and check if is to be stored
			if ((eventStatus == DEM_EVENT_STATUS_PASSED) || (eventStatus == DEM_EVENT_STATUS_FAILED)) {
				handlePreInitEvent(eventId, eventStatus);
			}
			break;

		case DEM_INITIALIZED:
			// Handle report
			if ((eventStatus == DEM_EVENT_STATUS_PASSED) || (eventStatus == DEM_EVENT_STATUS_FAILED)) {
				(void)handleEvent(eventId, eventStatus);
			}
			break;

		case DEM_UNINITIALIZED:
		default:
			// Uninitialized can not do anything
#if (DEM_DEV_ERROR_DETECT == STD_ON)
			Det_ReportError(MODULE_ID_DEM, 0, DEM_REPORTERRORSTATUS_ID, DEM_E_UNINIT);
#endif
			break;

	} // switch (demState)
}

/*********************************
 * Interface DCM <-> DEM (8.3.5) *
 *********************************/

/***********************************
 * OBD-specific Interfaces (8.3.6) *
 ***********************************/



