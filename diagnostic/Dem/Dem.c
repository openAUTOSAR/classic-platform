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
	Dem_EventIdType		eventId;
	uint16				occurrence;
	Dem_EventStatusType	eventStatus;
	boolean				eventStatusChanged;
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
	sint16				data[DEM_MAX_SIZE_FF_DATA];
	ChecksumType		checksum;
} FreezeFrameRecType;

typedef struct {
	Dem_EventIdType		eventId;
	uint8				data[DEM_MAX_SIZE_EXT_DATA];
	ChecksumType		checksum;
} ExtDataRecType;

/*
 * Prototypes of local functions
 */
void setZero(void *ptr, uint16 nrOfBytes);
ChecksumType calcChecksum(void *data, uint16 nrOfBytes);
void handlePreInitEvent(Dem_EventIdType eventId, Dem_EventStatusType eventStatus);
void handleEvent(Dem_EventIdType eventId, Dem_EventStatusType eventStatus);
void updateEventStatusRec(Dem_EventIdType eventId, Dem_EventStatusType eventStatus, boolean createIfNotExist);
void mergeEventStatusRec(EventRecType *eventRec);
void getEventStatusRec(Dem_EventIdType eventId, EventStatusRecType *eventStatusRec);
void lookupEventIdParameter(Dem_EventIdType eventId, Dem_EventParameterType **eventIdParam);
void storeEventEvtMem(Dem_EventParameterType *eventParam, EventStatusRecType *eventStatus);
void storeExtendedDataEvtMem(Dem_EventParameterType *eventParam, ExtDataRecType *extendedData);
void storeFreezeFrameDataEvtMem(Dem_EventParameterType *eventParam, FreezeFrameRecType *freezeFrame);
void updateFreezeFrameOccurrencePreInit(EventRecType *EventBuffer);


// State variable
typedef enum
{
  DEM_UNINITIALIZED = 0,
  DEM_PREINITIALIZED,
  DEM_INITIALIZED
} Dem_StateType;

static Dem_StateType _demState = DEM_UNINITIALIZED;

// Help pointer to config set
static Dem_ConfigSetType *ConfigSet;

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
void Dem_PreInit( void ) {
	int i, j;

	// TODO: Make a proper initializion via the function call
	ConfigSet = DEM_Config.ConfigSet;

	// Initialize the event status buffer
	for (i=0; i<DEM_MAX_NUMBER_EVENT; i++) {
		eventStatusBuffer[i].eventId = DEM_EVENT_ID_NULL;
		eventStatusBuffer[i].occurrence = 0;
		eventStatusBuffer[i].eventStatus = DEM_EVENT_STATUS_PASSED;
		eventStatusBuffer[i].eventStatusChanged = FALSE;
	}

	for (i=0; i<DEM_MAX_NUMBER_FF_DATA_PRE_INIT; i++) {
		preInitFreezeFrameBuffer[i].checksum = 0;
		preInitFreezeFrameBuffer[i].eventId = DEM_EVENT_ID_NULL;
		preInitFreezeFrameBuffer[i].occurrence = 0;
		for (j=0; j<DEM_MAX_SIZE_FF_DATA;j++)
			preInitFreezeFrameBuffer[i].data[j] = 0;
	}

	for (i=0; i<DEM_MAX_NUMBER_EXT_DATA_PRE_INIT; i++) {
		preInitExtDataBuffer[i].checksum = 0;
		preInitExtDataBuffer[i].eventId = DEM_EVENT_ID_NULL;
		for (j=0; j<DEM_MAX_SIZE_EXT_DATA;j++)
			preInitExtDataBuffer[i].data[j] = 0;
	}
	_demState = DEM_PREINITIALIZED;
}


/*
 * Procedure:	Dem_Init
 * Reentrant:	No
 */
void Dem_Init( void )
{
	uint16 i;
	ChecksumType cSum;
	EventStatusRecType eventStatusLocal;
	Dem_EventParameterType *eventParam;

	/*
	 *  Validate and read out saved error log from non volatile memory
	 */

	// Validate event records stored in primary memory
	for (i=0; i<DEM_MAX_NUMBER_EVENT_PRI_MEM; i++) {
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
	for (i=0; i<DEM_MAX_NUMBER_EXT_DATA_PRI_MEM; i++) {
		cSum = calcChecksum(&priMemExtDataBuffer[i], sizeof(ExtDataRecType)-sizeof(ChecksumType));
		if ((cSum != priMemExtDataBuffer[i].checksum) || priMemExtDataBuffer[i].eventId == DEM_EVENT_ID_NULL) {
			// Unlegal record, clear the record
			setZero(&priMemExtDataBuffer[i], sizeof(ExtDataRecType));
		}
	}

	// Validate freeze frame records stored in primary memory
	for (i=0; i<DEM_MAX_NUMBER_FF_DATA_PRI_MEM; i++) {
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
	for (i=0; i<DEM_MAX_NUMBER_EVENT; i++) {
		if (eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL) {
			// Update the event memory
			lookupEventIdParameter(eventStatusBuffer[i].eventId, &eventParam);
			storeEventEvtMem(eventParam, &eventStatusBuffer[i]);
		}
	}

	// Transfer extended data to event memory if necessary
	for (i=0; i < DEM_MAX_NUMBER_EXT_DATA_PRE_INIT; i++) {
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
	for (i=0; i < DEM_MAX_NUMBER_FF_DATA_PRE_INIT; i++) {
		if (preInitFreezeFrameBuffer[i].eventId != DEM_EVENT_ID_NULL) {
			lookupEventIdParameter(preInitFreezeFrameBuffer[i].eventId, &eventParam);
			storeFreezeFrameDataEvtMem(eventParam, &preInitFreezeFrameBuffer[i]);
		}
	}
	
	_demState = DEM_INITIALIZED;
}


/*
 * Procedure:	Dem_shutdown
 * Reentrant:	No
 */
void Dem_shutdown( void ) {
	// Save error log to non volatile memory
	// TODO: Save error log to NV-RAM

	_demState = DEM_UNINITIALIZED;
}


/*
 * Interface for basic software scheduler
 */
void Dem_MainFunction( void ){

}


/***************************************************
 * Interface SW-Components via RTE <-> DEM (8.3.3) *
 ***************************************************/

/*
 * Procedure:	Dem_SetEventStatus
 * Reentrant:	Yes
 */
Std_ReturnType Dem_SetEventStatus(Dem_EventIdType eventId, Dem_EventStatusType eventStatus) {
	Std_ReturnType _returnCode = E_OK;

	if (_demState == DEM_INITIALIZED) // No action is taken if the module is not started
	{
		// TODO:
	}
	else
	{
#if (DEM_DEV_ERROR_DETECT == STD_ON)
		Det_ReportError(MODULE_ID_DEM, 0, DEM_SETEVENTSTATUS_ID, DEM_E_UNINIT);
		_returnCode = E_NOT_OK;
#endif
	}

	return _returnCode;
}


Std_ReturnType Dem_ResetEventStatus(Dem_EventIdType eventId) {
	Std_ReturnType _returnCode = E_OK;

	if (_demState == DEM_INITIALIZED) // No action is taken if the module is not started
	{
		// TODO:
	}
	else
	{
#if (DEM_DEV_ERROR_DETECT == STD_ON)
		Det_ReportError(MODULE_ID_DEM, 0, DEM_RESETEVENTSTATUS_ID, DEM_E_UNINIT);
		_returnCode = E_NOT_OK;
#endif
	}

	return _returnCode;
}


Std_ReturnType Dem_GetEventStatus(Dem_EventIdType eventId, Dem_EventStatusExtendedType *eventStatusExtended) {
	Std_ReturnType _returnCode = E_OK;

	if (_demState == DEM_INITIALIZED) // No action is taken if the module is not started
	{
		*eventStatusExtended = 0; // TODO

	}
	else
	{
#if (DEM_DEV_ERROR_DETECT == STD_ON)
		Det_ReportError(MODULE_ID_DEM, 0, DEM_GETEVENTSTATUS_ID, DEM_E_UNINIT);
		_returnCode = E_NOT_OK;
#endif
	}

	return _returnCode;
}


Std_ReturnType Dem_GetEventFailed(Dem_EventIdType eventId, boolean *eventFailed) {
	Std_ReturnType _returnCode = E_OK;

	if (_demState == DEM_INITIALIZED) // No action is taken if the module is not started
	{
		*eventFailed = FALSE; // TODO:

	}
	else
	{
#if (DEM_DEV_ERROR_DETECT == STD_ON)
		Det_ReportError(MODULE_ID_DEM, 0, DEM_GETEVENTFAILED_ID, DEM_E_UNINIT);
		_returnCode = E_NOT_OK;
#endif
	}

	return _returnCode;
}


Std_ReturnType Dem_GetEventTested(Dem_EventIdType eventId, boolean *eventTested) {
	Std_ReturnType _returnCode = E_OK;

	if (_demState == DEM_INITIALIZED) // No action is taken if the module is not started
	{
		*eventTested = FALSE; // TODO:

	}
	else
	{
#if (DEM_DEV_ERROR_DETECT == STD_ON)
		Det_ReportError(MODULE_ID_DEM, 0, DEM_GETEVENTTESTED_ID, DEM_E_UNINIT);
		_returnCode = E_NOT_OK;
#endif
	}

	return _returnCode;
}


Std_ReturnType Dem_GetFaultDetectionCounter(Dem_EventIdType eventId, sint8 *counter) {
	Std_ReturnType _returnCode = E_OK;

	if (_demState == DEM_INITIALIZED) // No action is taken if the module is not started
	{
		*counter = 0;	// TODO:
	}
	else
	{
#if (DEM_DEV_ERROR_DETECT == STD_ON)
		Det_ReportError(MODULE_ID_DEM, 0, DEM_GETFAULTDETECTIONCOUNTER_ID, DEM_E_UNINIT);
		_returnCode = E_NOT_OK;
#endif
	}

	return _returnCode;
}


/********************************************
 * Interface BSW-Components <-> DEM (8.3.4) *
 ********************************************/

/*
 * Procedure:	Dem_ReportErrorStatus
 * Reentrant:	Yes
 */
void Dem_ReportErrorStatus( Dem_EventIdType eventId, Dem_EventStatusType eventStatus ) {

	switch (_demState) {
		case DEM_PREINITIALIZED:
			// Update status and check if is to be stored
			if ((eventStatus == DEM_EVENT_STATUS_PASSED) || (eventStatus == DEM_EVENT_STATUS_FAILED)) {
				handlePreInitEvent(eventId, eventStatus);
			}
			break;

		case DEM_INITIALIZED:
			// Handle report
			if ((eventStatus == DEM_EVENT_STATUS_PASSED) || (eventStatus == DEM_EVENT_STATUS_FAILED)) {
				handleEvent(eventId, eventStatus);
			}
			break;

		case DEM_UNINITIALIZED:
		default:
			// Uninitialized can not do anything
#if (DEM_DEV_ERROR_DETECT == STD_ON)
			Det_ReportError(MODULE_ID_DEM, 0, DEM_REPORTERRORSTATUS_ID, DEM_E_UNINIT);
#endif
			break;

	} // switch (_demState)
}

/*********************************
 * Interface DCM <-> DEM (8.3.5) *
 *********************************/

/***********************************
 * OBD-specific Interfaces (8.3.6) *
 ***********************************/



/******************
 * Help functions *
 ******************/
/*
 * Procedure:	setZero
 * Description:	Fill the *ptr to *(ptr+nrOfBytes-1) area with zeroes
 */
void setZero(void *ptr, uint16 nrOfBytes){
	uint8 *clrPtr = (uint8*)ptr;

	*clrPtr = 0x00;
	memcpy(clrPtr+1, clrPtr, nrOfBytes-1);
}

/*
 * Procedure:	calcChecksum
 * Description:	Calculate checksum over *data to *(data+nrOfBytes-1) area
 */
ChecksumType calcChecksum(void *data, uint16 nrOfBytes)
{
	uint16 i;
	uint8 *ptr = (uint8*)data;
	ChecksumType sum=0;

	for (i=0; i<nrOfBytes; i++)
		sum += *ptr++;
	sum ^= 0xaaaa;
	return sum;
}

/*
 * Procedure:	updateEventStatusRec
 * Description:	Update the status of "eventId", if not exist and "createIfNotExist" is
 * 				true a new record is created
 */
void updateEventStatusRec(Dem_EventIdType eventId, Dem_EventStatusType eventStatus, boolean createIfNotExist)
{
	uint16 i;
	imask_t state = McuE_EnterCriticalSection();

	// Lookup event ID
	for (i=0; (eventStatusBuffer[i].eventId != eventId) && (i < DEM_MAX_NUMBER_EVENT); i++);

	if (i < DEM_MAX_NUMBER_EVENT) {
		// Update event found
		if	 (eventStatusBuffer[i].eventStatus != eventStatus) {
			if (eventStatus == DEM_EVENT_STATUS_FAILED)
				eventStatusBuffer[i].occurrence++;
			eventStatusBuffer[i].eventStatus = eventStatus;
			eventStatusBuffer[i].eventStatusChanged = TRUE;
		}
		else {
			eventStatusBuffer[i].eventStatusChanged = FALSE;
		}
	}
	else if (createIfNotExist) {
		// Search for free position
		for (i=0; (eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL) && (i < DEM_MAX_NUMBER_EVENT); i++);

		if (i < DEM_MAX_NUMBER_EVENT) {
			// Create new event
			eventStatusBuffer[i].eventId = eventId;
			eventStatusBuffer[i].occurrence = 1;
			eventStatusBuffer[i].eventStatus = eventStatus;
			eventStatusBuffer[i].eventStatusChanged = TRUE;
		}
		else {
			// TODO: Handle event status buffer full
		}
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
	for (i=0; (eventStatusBuffer[i].eventId != eventRec->eventId) && (i < DEM_MAX_NUMBER_EVENT); i++);

	if (i < DEM_MAX_NUMBER_EVENT) {
		// Update occurrence counter, rest of pre init state is kept.
		eventStatusBuffer[i].occurrence += eventRec->occurrence;

	}
	else {
		// Search for free position
		for (i=0; (eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL) && (i < DEM_MAX_NUMBER_EVENT); i++);

		if (i < DEM_MAX_NUMBER_EVENT) {
			// Create new event, from stored event
			eventStatusBuffer[i].eventId = eventRec->eventId;
			eventStatusBuffer[i].occurrence = eventRec->occurrence;
			eventStatusBuffer[i].eventStatus = DEM_EVENT_STATUS_PASSED;
			eventStatusBuffer[i].eventStatusChanged = FALSE;
		}
		else {
			// TODO: Handle event status buffer full
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
	for (i=0; (eventStatusBuffer[i].eventId != eventId) && (i < DEM_MAX_NUMBER_EVENT); i++);

	if (i < DEM_MAX_NUMBER_EVENT) {
		memcpy(eventStatusRec, &eventStatusBuffer[i], sizeof(EventStatusRecType));
		/*
		eventStatusRec->eventId = EventStatusBuffer[i].eventId;
		eventStatusRec->eventStatus = EventStatusBuffer[i].eventStatus;
		eventStatusRec->occurrence = EventStatusBuffer[i].occurrence;
		eventStatusRec->eventStatusChanged = EventStatusBuffer[i].eventStatusChanged;
		*/
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
void lookupEventIdParameter(Dem_EventIdType eventId, Dem_EventParameterType **eventIdParam)
{
	uint16 i;
	*eventIdParam = NULL;

	// Lookup the correct event id parameters
	for (i=0; !ConfigSet->EventParameter[i].EcoreEOL; i++)
	{
		if (ConfigSet->EventParameter[i].EventID == eventId) {
			*eventIdParam = &ConfigSet->EventParameter[i];
			return;
		}
	}
	// Id not found return with NULL pointer
}


void getFreezeFrameData(Dem_EventParameterType *eventParam, FreezeFrameRecType *freezeFrame)
{
	// TODO: Fill out
}


void storeFreezeFrameDataPreInit(Dem_EventParameterType *eventParam, FreezeFrameRecType *freezeFrame)
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
void getExtendedData(Dem_EventParameterType *eventParam, ExtDataRecType *extData)
{
	uint16 i;
	uint16 storeIndex = 0;
	uint16 recordSize;

	// Clear ext data record
	setZero(extData, sizeof(ExtDataRecType));

	// Check if any pointer to extended data class
	if (eventParam->ExtendedDataClassRef != NULL) {
		// Request extended data and copy it to the buffer
		for (i=0; (i<DEM_MAX_NR_OF_RECORDS_IN_EXTENDED_DATA) && (eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i] != NULL); i++) {
			recordSize = eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i]->DataSize;
			if ((storeIndex+recordSize) <= DEM_MAX_SIZE_EXT_DATA) {
				eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i]->CallbackGetExtDataRecord(&extData->data[storeIndex]);
				storeIndex += recordSize;
			}
			else {
				// TODO: Handle size of data record is bigger than reserved space.
				break;	// Break the loop
			}
		}
	}

	// Check if any data has been stored
	if (storeIndex != 0) {
		extData->eventId = eventParam->EventID;
		extData->checksum = calcChecksum(extData, sizeof(ExtDataRecType)-sizeof(ChecksumType));
	}
	else {
		extData->eventId = DEM_EVENT_ID_NULL;
		extData->checksum = 0;
	}
}


/*
 * Procedure:	storeExtendedDataPreInit
 * Description:	Store the extended data pointed by "extendedData" to the "preInitExtDataBuffer"
 */
void storeExtendedDataPreInit(Dem_EventParameterType *eventParam, ExtDataRecType *extendedData)
{
	uint16 i;
	imask_t state = McuE_EnterCriticalSection();

	// Lookup first free position
	for (i=0; (preInitExtDataBuffer[i].eventId !=0) && (i<DEM_MAX_NUMBER_EXT_DATA_PRE_INIT); i++);
	if (i<DEM_MAX_NUMBER_EXT_DATA_PRE_INIT) {
		memcpy(&preInitExtDataBuffer[i], extendedData, sizeof(ExtDataRecType));
	}
	else {
		// TODO: Handle no free positions in buffer error
	}

	McuE_ExitCriticalSection(state);
}


/*
 * Procedure:	storeEventPriMem
 * Description:	Store the event data of "eventStatus->eventId" in "priMemEventBuffer",
 * 				if non existent a new entry is created.
 */
void storeEventPriMem(Dem_EventParameterType *eventParam, EventStatusRecType *eventStatus)
{
	uint16 i;
	imask_t state = McuE_EnterCriticalSection();


	// Lookup event ID
	for (i=0; (priMemEventBuffer[i].eventId != eventStatus->eventId) && (i < DEM_MAX_NUMBER_EVENT_ENTRY_PRI); i++);

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
			// TODO: Handle no free positions in buffer error
		}
	}

	McuE_ExitCriticalSection(state);
}


/*
 * Procedure:	storeEventEvtMem
 * Description:	Store the event data of "eventStatus->eventId" in event memory according to
 * 				"eventParam" destination option.
 */
void storeEventEvtMem(Dem_EventParameterType *eventParam, EventStatusRecType *eventStatus)
{
	uint16 i;

	for (i=0; (i < DEM_MAX_NR_OF_EVENT_DESTINATION) && (eventParam->EventClass->EventDestination[i] != NULL); i++) {
		switch (eventParam->EventClass->EventDestination[i])
		{
		case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
			storeEventPriMem(eventParam, eventStatus);
			break;

		case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
		case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
		case DEM_DTC_ORIGIN_MIRROR_MEMORY:
			// Not yet supported
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
void storeExtendedDataPriMem(Dem_EventParameterType *eventParam, ExtDataRecType *extendedData)
{
	uint16 i;
	imask_t state = McuE_EnterCriticalSection();

	// Lookup first free position
	for (i=0; (priMemExtDataBuffer[i].eventId != DEM_EVENT_ID_NULL) && (i<DEM_MAX_NUMBER_EXT_DATA_PRI_MEM); i++);
	if (i < DEM_MAX_NUMBER_EXT_DATA_PRI_MEM) {
		memcpy(&priMemExtDataBuffer[i], extendedData, sizeof(ExtDataRecType));
	}
	else {
		// TODO: Handle no free positions in buffer error
	}

	McuE_ExitCriticalSection(state);
}


/*
 * Procedure:	storeExtendedDataEvtMem
 * Description:	Store the extended data in event memory according to
 * 				"eventParam" destination option
 */
void storeExtendedDataEvtMem(Dem_EventParameterType *eventParam, ExtDataRecType *extendedData)
{
	uint16 i;

	for (i=0; (i < DEM_MAX_NR_OF_EVENT_DESTINATION) && (eventParam->EventClass->EventDestination[i] != NULL); i++) {
		switch (eventParam->EventClass->EventDestination[i])
		{
		case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
			storeExtendedDataPriMem(eventParam, extendedData);
			break;

		case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
		case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
		case DEM_DTC_ORIGIN_MIRROR_MEMORY:
			// Not yet supported
			break;
		default:
			break;
		}
	}
}


void storeFreezeFrameDataPriMem(Dem_EventParameterType *eventParam, FreezeFrameRecType *freezeFrame)
{
	// TODO: Fill out
}


/*
 * Procedure:	storeFreezeFrameDataEvtMem
 * Description:	Store the freeze frame data in event memory according to
 * 				"eventParam" destination option
 */
void storeFreezeFrameDataEvtMem(Dem_EventParameterType *eventParam, FreezeFrameRecType *freezeFrame)
{
	uint16 i;

	for (i=0; (i < DEM_MAX_NR_OF_EVENT_DESTINATION) && (eventParam->EventClass->EventDestination[i] != NULL); i++) {
		switch (eventParam->EventClass->EventDestination[i])
		{
		case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
			storeFreezeFrameDataPriMem(eventParam, freezeFrame);
			break;

		case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
		case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
		case DEM_DTC_ORIGIN_MIRROR_MEMORY:
			// Not yet supported
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
	Dem_EventParameterType *eventParam;
	EventStatusRecType eventStatusLocal;
	FreezeFrameRecType freezeFrameLocal;
	ExtDataRecType extendedDataLocal;

	if (eventStatus == DEM_EVENT_STATUS_PASSED) {
		updateEventStatusRec(eventId, eventStatus, FALSE);
	}

	if (eventStatus == DEM_EVENT_STATUS_FAILED) {
		updateEventStatusRec(eventId, eventStatus, TRUE);
		getEventStatusRec(eventId, &eventStatusLocal);
		if (eventStatusLocal.eventStatusChanged)
		{
			lookupEventIdParameter(eventId, &eventParam);
			if (eventParam != NULL) {
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
			} // EventId params found
		} // if (eventStatusLocal.eventStatusChanged)
	} // if (eventStatus == DEM_EVENT_STATUS_FAILED)
}


/*
 * Procedure:	handleEvent
 * Description:	Handle the updating of event status and storing of
 * 				event related data in event memory.
 */
void handleEvent(Dem_EventIdType eventId, Dem_EventStatusType eventStatus)
{
	Dem_EventParameterType *eventParam;
	EventStatusRecType eventStatusLocal;
	FreezeFrameRecType freezeFrameLocal;
	ExtDataRecType extendedDataLocal;

	if (eventStatus == DEM_EVENT_STATUS_PASSED) {
		updateEventStatusRec(eventId, eventStatus, FALSE);
	}

	if (eventStatus == DEM_EVENT_STATUS_FAILED) {
		updateEventStatusRec(eventId, eventStatus, TRUE);
		getEventStatusRec(eventId, &eventStatusLocal);
		if (eventStatusLocal.eventStatusChanged)
		{
			lookupEventIdParameter(eventId, &eventParam);
			if (eventParam != NULL) {
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
			} // EventId params found
		} // if (eventStatusLocal.eventStatusChanged)
	} // if (eventStatus == DEM_EVENT_STATUS_FAILED)
}
