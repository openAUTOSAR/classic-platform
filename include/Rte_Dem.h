/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/

#ifndef RTE_DEM_H_
#define RTE_DEM_H_

typedef uint8 Dem_DTCKindType;
typedef uint32 Dem_DTCType;
typedef uint16 Dem_EventIdType;
typedef sint8 Dem_FaultDetectionCounterType;
typedef uint8 Dem_IndicatorStatusType;
typedef uint8 Dem_InitMonitorReasonType;
typedef uint8 Dem_OperationCycleIdType;
/*
 * Dem_OperationCycleStateType definitions
 */
typedef uint8 Dem_OperationCycleStateType;
#if !defined(_DEFINED_TYPEDEF_FOR_Dem_OperationCycleStateType_)
#define DEM_CYCLE_STATE_START       (Dem_OperationCycleStateType)1
#define DEM_CYCLE_STATE_END         (Dem_OperationCycleStateType)2
#endif


/*
 * Dem_EventStatusType
 */
#if !defined(_DEFINED_TYPEDEF_FOR_Dem_EventStatusType_)
typedef uint8 Dem_EventStatusType;
#define DEM_EVENT_STATUS_PASSED     (Dem_EventStatusType)0
#define DEM_EVENT_STATUS_FAILED     (Dem_EventStatusType)1
#define DEM_EVENT_STATUS_PREPASSED  (Dem_EventStatusType)2
#define DEM_EVENT_STATUS_PREFAILED  (Dem_EventStatusType)3
#endif

/*
 * DemEventStatusExtendedType definitions
 */
typedef uint8 Dem_EventStatusExtendedType;
#define DEM_TEST_FAILED                             (Dem_EventStatusExtendedType)0x01
#define DEM_TEST_FAILED_THIS_OPERATION_CYCLE        (Dem_EventStatusExtendedType)0x02
#define DEM_PENDING_DTC                             (Dem_EventStatusExtendedType)0x04
#define DEM_CONFIRMED_DTC                           (Dem_EventStatusExtendedType)0x08
#define DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR     (Dem_EventStatusExtendedType)0x10
#define DEM_TEST_FAILED_SINCE_LAST_CLEAR            (Dem_EventStatusExtendedType)0x20
#define DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE (Dem_EventStatusExtendedType)0x40
#define DEM_WARNING_INDICATOR_REQUESTED             (Dem_EventStatusExtendedType)0x80

/*
 * Dem_DTCFormatType
 */
typedef uint8 Dem_DTCFormatType;
#define DEM_DTC_FORMAT_OBD          (Dem_DTCFormatType)0x00
#define DEM_DTC_FORMAT_UDS          (Dem_DTCFormatType)0x01

/*
 * DemDTCOriginType
 */
typedef uint8 Dem_DTCOriginType;
#define DEM_DTC_ORIGIN_NOT_USED         (Dem_DTCOriginType)0x00
#define DEM_DTC_ORIGIN_SECONDARY_MEMORY (Dem_DTCOriginType)0x01
#define DEM_DTC_ORIGIN_PRIMARY_MEMORY   (Dem_DTCOriginType)0x02
#define DEM_DTC_ORIGIN_PERMANENT_MEMORY (Dem_DTCOriginType)0x03
#define DEM_DTC_ORIGIN_MIRROR_MEMORY    (Dem_DTCOriginType)0x04

Std_ReturnType Dem_SetEventStatus(Dem_EventIdType EventId, Dem_EventStatusType EventStatus);

#endif /* RTE_DEM_H_ */
