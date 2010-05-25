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








#ifndef DEM_H_
#define DEM_H_

#define DEM_SW_MAJOR_VERSION    1
#define DEM_SW_MINOR_VERSION   	0
#define DEM_SW_PATCH_VERSION    0
#define DEM_AR_MAJOR_VERSION    3
#define DEM_AR_MINOR_VERSION    0
#define DEM_AR_PATCH_VERSION    1

#include "Dem_Types.h"
#include "Dem_Cfg.h"
#include "Dem_Lcfg.h"
#include "Dem_IntErrId.h"
#include "Dem_IntEvtId.h"
// #include "Rte_Dem.h"


#if (DEM_DEV_ERROR_DETECT == STD_ON)
// Error codes produced by this module
#define DEM_E_CONFIG_PTR_INVALID			0x01
#define DEM_E_PARAM_CONFIG					0x10
#define DEM_E_PARAM_ADDRESS					0x11
#define DEM_E_PARAM_DATA					0x12
#define DEM_E_PARAM_LENGTH					0x13
#define DEM_E_UNINIT						0x20
#define DEM_E_NODATAAVAILABLE				0x30

#define DEM_E_EVENT_STATUS_BUFF_FULL		0x40
#define DEM_E_EXT_DATA_TO_BIG				0x41
#define DEM_E_PRE_INIT_EXT_DATA_BUFF_FULL	0x42
#define DEM_E_PRI_MEM_EVENT_BUFF_FULL		0x43
#define DEM_E_PRI_MEM_EXT_DATA_BUFF_FULL	0x44

#define DEM_E_UNEXPECTED_EXECUTION			0xfe
#define DEM_E_NOT_IMPLEMENTED_YET			0xff

// Service ID in this module
#define DEM_PREINIT_ID						0x01
#define DEM_INIT_ID							0x02
#define DEM_SETEVENTSTATUS_ID				0x04
#define DEM_RESETEVENTSTATUS_ID				0x05
#define DEM_SETOPERATIONCYCLESTATE_ID		0x08
#define DEM_GETEVENTSTATUS_ID				0x0A
#define DEM_GETEVENTFAILED_ID				0x0B
#define DEM_GETEVENTTESTED_ID				0x0C
#define DEM_REPORTERRORSTATUS_ID 			0x0F
#define DEM_GETEXTENDEDDATARECORDBYDTC_ID	0x20
#define DEM_CLEARDTC_ID						0x22
#define DEM_GETFAULTDETECTIONCOUNTER_ID 	0x3E

#define DEM_UPDATE_EVENT_STATUS_ID			0x80
#define DEM_MERGE_EVENT_STATUS_ID			0x81
#define DEM_GET_EXTENDED_DATA_ID			0x82
#define DEM_STORE_EXT_DATA_PRE_INIT_ID		0x83
#define DEM_STORE_EVENT_PRI_MEM_ID			0x84
#define DEM_STORE_EXT_DATA_PRI_MEM_ID		0x85

#define DEM_GLOBAL_ID						0xff

#endif


/*
 * Interface for upper layer modules (8.3.1)
 */

#if (DEM_VERSION_INFO_API == STD_ON)
void Dem_GetVersionInfo(Std_VersionInfoType *versionInfo);
#endif /* DEM_VERSION_INFO_API */

/*
 * Interface ECU State Manager <-> DEM (8.3.2)
 */
void Dem_PreInit( void );
void Dem_Init( void );
void Dem_Shutdown( void );


/*
 * Interface for basic software scheduler
 */
void Dem_MainFunction( void );


/*
 * Interface SW-Components via RTE <-> DEM (8.3.3)
 */
Std_ReturnType Dem_SetEventStatus(Dem_EventIdType eventId, uint8 eventStatus);
Std_ReturnType Dem_ResetEventStatus(Dem_EventIdType eventId);
Std_ReturnType Dem_GetEventStatus(Dem_EventIdType eventId, Dem_EventStatusExtendedType *eventStatusExtended);
Std_ReturnType Dem_GetEventFailed(Dem_EventIdType eventId, boolean *eventFailed);
Std_ReturnType Dem_GetEventTested(Dem_EventIdType eventId, boolean *eventTested);
Std_ReturnType Dem_GetFaultDetectionCounter(Dem_EventIdType eventId, sint8 *counter);
Std_ReturnType Dem_SetOperationCycleState(Dem_OperationCycleIdType OperationCycleId, Dem_OperationCycleStateType CycleState);
Std_ReturnType Dem_GetDTCOfEvent(Dem_EventIdType eventId, Dem_DTCKindType dtcKind, uint32* dtcOfEvent);


/*
 * Interface BSW-Components <-> DEM (8.3.4)
 */
void Dem_ReportErrorStatus(Dem_EventIdType eventId ,uint8 eventStatus);


/*
 * Interface DCM <-> DEM (8.3.5)
 */
Dem_ReturnClearDTCType Dem_ClearDTC(uint32 dtc, Dem_DTCKindType dtcKind, Dem_DTCOriginType dtcOrigin);
Dem_ReturnSetDTCFilterType Dem_SetDTCFilter(uint8 dtcStatusMask,
		Dem_DTCKindType dtcKind,
		Dem_DTCOriginType dtcOrigin,
		Dem_FilterWithSeverityType filterWithSeverity,
		Dem_DTCSeverityType dtcSeverityMask,
		Dem_FilterForFDCType filterForFaultDetectionCounter);
Dem_ReturnGetStatusOfDTCType Dem_GetStatusOfDTC(uint32 dtc, Dem_DTCKindType dtcKind, Dem_DTCOriginType dtcOrigin, Dem_EventStatusExtendedType* dtcStatus);
Std_ReturnType Dem_GetDTCStatusAvailabilityMask(uint8 *dtcStatusMask);
Dem_ReturnGetNumberOfFilteredDTCType Dem_GetNumberOfFilteredDtc(uint16* numberOfFilteredDTC);
Dem_ReturnGetNextFilteredDTCType Dem_GetNextFilteredDTC(uint32* dtc, Dem_EventStatusExtendedType* dtcStatus);
Dem_ReturnTypeOfDtcSupportedType Dem_GetTranslationType(void);
Dem_ReturnControlDTCStorageType Dem_DisableDTCStorage(Dem_DTCGroupType dtcGroup, Dem_DTCKindType dtcKind);
Dem_ReturnControlDTCStorageType Dem_EnableDTCStorage(Dem_DTCGroupType dtcGroup, Dem_DTCKindType dtcKind);
Dem_ReturnGetExtendedDataRecordByDTCType Dem_GetExtendedDataRecordByDTC(uint32 dtc, Dem_DTCKindType dtcKind, Dem_DTCOriginType dtcOrigin, uint8 extendedDataNumber, uint8 *destBuffer, uint8 *bufSize);
Dem_ReturnGetSizeOfExtendedDataRecordByDTCType Dem_GetSizeOfExtendedDataRecordByDTC(uint32 dtc, Dem_DTCKindType dtcKind, Dem_DTCOriginType dtcOrigin, uint8 extendedDataNumber, uint16 *sizeOfExtendedDataRecord);



/*
 * OBD-specific Interfaces (8.3.6)
 */

#endif /*DEM_H_*/
