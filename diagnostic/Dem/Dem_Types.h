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








#ifndef DEM_TYPES_H_
#define DEM_TYPES_H_
#include "Std_Types.h"

/*
 * Dem_EventIdType
 */
typedef uint16 Dem_EventIdType;


/*
 * DemDTCKindType
 */
typedef uint8 Dem_DTCKindType;
#define	DEM_DTC_KIND_ALL_DTCS			0x01
#define	DEM_DTC_KIND_EMISSON_REL_DTCS	0x02

/*
 * DemDTCOriginType
 */
typedef uint8 Dem_DTCOriginType;
#define	DEM_DTC_ORIGIN_MIRROR_MEMORY	0x04
#define	DEM_DTC_ORIGIN_PERMANENT_MEMORY	0x03
#define	DEM_DTC_ORIGIN_PRIMARY_MEMORY	0x02
#define	DEM_DTC_ORIGIN_SECONDARY_MEMORY	0x01


/*
 * DemEventStatusExtendedType
 */
typedef uint8 Dem_EventStatusExtendedType;
#define	DEM_WARNING_INDICATOR_REQUESTED 0x80
#define	DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE 0x40
#define	DEM_TEST_FAILED_SINCE_LAST_CLEAR 0x20
#define	DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR 0x10
#define	DEM_CONFIRMED_DTC 0x08
#define	DEM_PENDING_DTC 0x04
#define	DEM_TEST_FAILED_THIS_OPERATION_CYCLE 0x02
#define	DEM_TEST_FAILED 0x01

/*
 * DemOperationCycleType
 */
typedef uint8 Dem_OperationCycleType;	// TODO: Check type and values
#define DEM_IGNITION	0x01			// Ignition ON/OF Cycle
#define	DEM_OBD_DCY		0x02			// OBD Driving Cycle
#define	DEM_POWER		0x03			// Power ON/OFF Cycle
#define	DEM_WARMUP		0x04			// OBD Warm up Cycle

/*
 * DemFreezeFrameKindType
 */
typedef uint8 Dem_FreezeFrameKindType;	// TODO: Check type and values
#define	DEM_FREEZE_FRAME_NON_OBD 	0x01
#define DEM_FREEZE_FRAME_OBD		0x02

/*
 * DemEventKindType
 */
typedef uint8 Dem_EventKindType;			// TODO: Check type and values
#define DEM_EVENT_KIND_BSW		0x01
#define DEM_EVENT_KIND_SWC		0x02

/*
 * Dem_EventStatusType
 */
typedef uint8 Dem_EventStatusType;
#define DEM_EVENT_STATUS_PASSED		0
#define DEM_EVENT_STATUS_FAILED		1
#define DEM_EVENT_STATUS_PREPASSED	2
#define DEM_EVENT_STATUS_PREFAILED	3

/*
 * Dem_DTCType
 */
typedef uint32 Dem_DTCType;

/*
 * Dem_InitMonitorKindType
 */
typedef uint8 Dem_InitMonitorKindType;
#define DEM_INIT_MONITOR_CLEAR		1
#define DEM_INIT_MONITOR_RESTART	2

/*
 * Dem_OperationCycleSTateType
 */
typedef uint8 Dem_OperationCycleSTateType;
#define DEM_CYCLE_STATE_START		1
#define DEM_CYCLE_STATE_END			2

/*
 * Dem_FaultDetectionCpunterType
 */
typedef sint8 Dem_FaultDetectionCounterType;

/*
 * Dem_IndicatorStatusType
 */
typedef uint8 Dem_IndicatorStatusType;
#define DEM_INDICATOR_OFF			0
#define DEM_INDICATOR_CONTINUOUS	1
#define DEM_INDICATOR_BLINKING		2
#define DEM_INDICATOR_BLINK_CONT	3



#endif /*DEM_TYPES_H_*/
