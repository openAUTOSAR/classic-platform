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

#include "Dem_Types.h"
#include "Dem_Cfg.h"
#include "Dem_Lcfg.h"
#include "Dem_IntErrId.h"
#include "Dem_IntEvtId.h"
// #include "Rte_Dem.h"


#if (DEM_DEV_ERROR_DETECT == STD_ON)
// Error codes produced by this module
#define DEM_E_PARAM_CONFIG		0x10
#define DEM_E_PARAM_ADDRESS		0x11
#define DEM_E_PARAM_DATA		0x12
#define DEM_E_PARAM_LENGTH		0x13
#define DEM_E_UNINIT			0x20
#define DEM_E_NODATAAVAILABLE	0x30

// Service ID in this module
#define DEM_SETEVENTSTATUS_ID			(0x10)
#define DEM_RESETEVENTSTATUS_ID			(0x11)
#define DEM_GETEVENTSTATUS_ID			(0x12)
#define DEM_GETEVENTFAILED_ID			(0x13)
#define DEM_GETEVENTTESTED_ID			(0x14)
#define DEM_GETFAULTDETECTIONCOUNTER_ID (0x15)
#define DEM_REPORTERRORSTATUS_ID 		(0x20)

#endif


/*
 * Interface for upper layer modules (8.3.1)
 */

#if (DEM_VERSION_INFO_API == STD_ON)
#define DEM_SW_MAJOR_VERSION    0	// TODO: What numbers here?
#define DEM_SW_MINOR_VERSION   	0
#define DEM_SW_PATCH_VERSION    1
#define DEM_AR_MAJOR_VERSION    3
#define DEM_AR_MINOR_VERSION    0
#define DEM_AR_PATCH_VERSION    1

void Dem_GetVersionInfo(Std_VersionInfoType *versionInfo);
#endif /* DEM_VERSION_INFO_API */

/*
 * Interface ECU State Manager <-> DEM (8.3.2)
 */
void Dem_PreInit( void );
void Dem_Init( void );
void Dem_shutdown( void );

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


/*
 * Interface BSW-Components <-> DEM (8.3.4)
 */
void Dem_ReportErrorStatus( Dem_EventIdType eventId ,uint8 eventStatus );

/*
 * Interface DCM <-> DEM (8.3.5)
 */

/*
 * OBD-specific Interfaces (8.3.6)
 */

#endif /*DEM_H_*/
