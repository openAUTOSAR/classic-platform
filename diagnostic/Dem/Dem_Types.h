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
#include "Std_Types.h" /** @req DEM176.Std */
#include "Rte_Dem.h"
#define DEM_EVENT_DESTINATION_END_OF_LIST 0

/*
 * DTC storage types
 */
typedef uint8 Dem_ReturnTypeOfDtcSupportedType;
#define DEM_ISO15031_6		0x00
#define DEM_ISO14229_1		0x01
#define DEM_SAEJ1939_73		0x02
#define DEM_ISO_11992_4		0x03

/*
 * Dem_DTCGroupType
 */
typedef uint32 Dem_DTCGroupType;
#define	DEM_DTC_GROUP_ALL_DTCS			(Dem_DTCGroupType)0xffffff

/*
 * Dem status type
 */
#define DEM_DTC_STATUS_MASK_ALL			(uint8)0x00


/*
 * DemDTCOriginType
 */
typedef uint8 Dem_DTCOriginType;
#define	DEM_DTC_ORIGIN_SECONDARY_MEMORY	(Dem_DTCOriginType)0x01
#define	DEM_DTC_ORIGIN_PRIMARY_MEMORY	(Dem_DTCOriginType)0x02
#define	DEM_DTC_ORIGIN_PERMANENT_MEMORY	(Dem_DTCOriginType)0x03
#define	DEM_DTC_ORIGIN_MIRROR_MEMORY	(Dem_DTCOriginType)0x04

/*
 * Dem_FreezeFrameKindType
 */
typedef uint8 Dem_FreezeFrameKindType;	// TODO: Check type and values
#define	DEM_FREEZE_FRAME_NON_OBD 	(Dem_FreezeFrameKindType)0x01
#define DEM_FREEZE_FRAME_OBD		(Dem_FreezeFrameKindType)0x02

/*
 * Dem_EventKindType
 */
typedef uint8 Dem_EventKindType;		// TODO: Check type and values
#define DEM_EVENT_KIND_BSW		(Dem_EventKindType)0x01
#define DEM_EVENT_KIND_SWC		(Dem_EventKindType)0x02

/*
 * Dem_EventStatusType
 */
typedef uint8 Dem_EventStatusType;
#define DEM_EVENT_STATUS_PASSED		(Dem_EventStatusType)0
#define DEM_EVENT_STATUS_FAILED		(Dem_EventStatusType)1
#define DEM_EVENT_STATUS_PREPASSED	(Dem_EventStatusType)2
#define DEM_EVENT_STATUS_PREFAILED	(Dem_EventStatusType)3

/*
 * Dem_PreDebounceNameType
 */
typedef uint8 Dem_PreDebounceNameType;
enum {
	DEM_NO_PRE_DEBOUNCE,
	DEM_PRE_DEBOUNCE_COUNTER_BASED,
	DEM_PRE_DEBOUNCE_FREQUENCY_BASED,
	DEM_PRE_DEBOUNCE_TIME_BASED
};

/*
 * Dem_FilterWithSeverityType
 */
typedef uint8 Dem_FilterWithSeverityType;
#define DEM_FILTER_WITH_SEVERITY_YES    (Dem_FilterWithSeverityType)0x00
#define DEM_FILTER_WITH_SEVERITY_NO     (Dem_FilterWithSeverityType)0x01

/*
 * Dem_FilterForFDCType
 */
typedef uint8 Dem_FilterForFDCType;
#define DEM_FILTER_FOR_FDC_YES          (Dem_FilterForFDCType)0x00
#define DEM_FILTER_FOR_FDC_NO           (Dem_FilterForFDCType)0x01

/*
 * Dem_DTCSeverityType
 */
typedef uint8 Dem_DTCSeverityType;
#define DEM_SEVERITY_NO_SEVERITY        (Dem_DTCSeverityType)0x00 // No severity information available
#define DEM_SEVERITY_MAINTENANCE_ONLY   (Dem_DTCSeverityType)0x20
#define DEM_SEVERITY_CHECK_AT_NEXT_FALT (Dem_DTCSeverityType)0x40
#define DEM_SEVERITY_CHECK_IMMEDIATELY  (Dem_DTCSeverityType)0x80

/*
 * Dem_ReturnSetDTCFilterType
 */
typedef uint8 Dem_ReturnSetDTCFilterType;
#define DEM_FILTER_ACCEPTED	(Dem_ReturnSetDTCFilterType)0x00
#define DEM_WRONG_FILTER	(Dem_ReturnSetDTCFilterType)0x01

/*
 * Dem_ReturnGetStatusOfDTCType
 */
typedef uint8 Dem_ReturnGetStatusOfDTCType;
#define DEM_STATUS_OK						(Dem_ReturnGetStatusOfDTCType)0x00
#define DEM_STATUS_WRONG_DTC				(Dem_ReturnGetStatusOfDTCType)0x01
#define DEM_STATUS_WRONG_DTCORIGIN			(Dem_ReturnGetStatusOfDTCType)0x02
#define DEM_STATUS_FAILED					(Dem_ReturnGetStatusOfDTCType)0x04
#define DEM_STATUS_WRONG_DTCKIND			(Dem_ReturnGetStatusOfDTCType)0x03

/*
 * Dem_ReturnGetNextFilteredDTCType
 */
typedef uint8 Dem_ReturnGetNextFilteredDTCType;
#define DEM_FILTERED_OK						(Dem_ReturnGetNextFilteredDTCType)0x00
#define DEM_FILTERED_NO_MATCHING_DTC		(Dem_ReturnGetNextFilteredDTCType)0x01
#define DEM_FILTERED_WRONG_DTCKIND			(Dem_ReturnGetNextFilteredDTCType)0x02
#define DEM_FILTERED_PENDING				(Dem_ReturnGetNextFilteredDTCType)0x03

/*
 * Dem_ReturnGetNumberOfFilteredDTCType
 */
typedef uint8 Dem_ReturnGetNumberOfFilteredDTCType;
#define DEM_NUMBER_OK						(Dem_ReturnGetNumberOfFilteredDTCType)0x00
#define DEM_NUMBER_FAILED					(Dem_ReturnGetNumberOfFilteredDTCType)0x01
#define DEM_NUMBER_PENDING					(Dem_ReturnGetNumberOfFilteredDTCType)0x02

/*
 * Dem_ReturnClearDTCType
 */
typedef uint8 Dem_ReturnClearDTCType;
#define DEM_CLEAR_OK						(Dem_ReturnClearDTCType)0x00
#define DEM_CLEAR_WRONG_DTC					(Dem_ReturnClearDTCType)0x01
#define DEM_CLEAR_WRONG_DTCORIGIN			(Dem_ReturnClearDTCType)0x02
#define DEM_CLEAR_WRONG_DTCKIND				(Dem_ReturnClearDTCType)0x03
#define DEM_CLEAR_FAILED					(Dem_ReturnClearDTCType)0x04
#define DEM_DTC_PENDING						(Dem_ReturnClearDTCType)0x05

/*
 * Dem_ReturnControlDTCStorageType
 */
typedef uint8 Dem_ReturnControlDTCStorageType;
#define DEM_CONTROL_DTC_STORAGE_OK			(Dem_ReturnControlDTCStorageType)0x00
#define DEM_CONTROL_DTC_STORAGE_N_OK		(Dem_ReturnControlDTCStorageType)0x01
#define DEM_CONTROL_DTC_WRONG_DTCGROUP		(Dem_ReturnControlDTCStorageType)0x02

/*
 * Dem_ReturnControlEventUpdateType
 */
typedef uint8 Dem_ReturnControlEventUpdateType;
#define DEM_CONTROL_EVENT_UPDATE_OK			(Dem_ReturnControlEventUpdateType)0x00
#define DEM_CONTROL_EVENT_N_OK				(Dem_ReturnControlEventUpdateType)0x01
#define DEM_CONTROL_EVENT_WRONG_DTCGROUP	(Dem_ReturnControlEventUpdateType)0x02

/*
 * Dem_ReturnGetDTCOfFreezeframeRecordType
 */
typedef uint8 Dem_ReturnGetDTCOfFreezeframeRecordType;
#define DEM_GET_DTCOFFF_OK					(Dem_ReturnGetDTCOfFreezeframeRecordType)0x00
#define DEM_GET_DTCOFFF_WRONG_RECORD		(Dem_ReturnGetDTCOfFreezeframeRecordType)0x01
#define DEM_GET_DTCOFFF_NO_DTC_FOR_RECORD	(Dem_ReturnGetDTCOfFreezeframeRecordType)0x02
#define DEM_GET_DTCOFFF_WRONG_DTCKIND		(Dem_ReturnGetDTCOfFreezeframeRecordType)0x03

/*
 * Dem_GetFreezeFameDataIdentifierByDTCType
 */
typedef uint8 Dem_GetFreezeFameDataIdentifierByDTCType;
#define DEM_GET_ID_OK						(Dem_GetFreezeFameDataIdentifierByDTCType)0x00
#define DEM_GET_ID_WRONG_DTC				(Dem_GetFreezeFameDataIdentifierByDTCType)0x01
#define DEM_GET_ID_WRONG_DTCORIGIN			(Dem_GetFreezeFameDataIdentifierByDTCType)0x02
#define DEM_GET_ID_WRONG_DTCKIND			(Dem_GetFreezeFameDataIdentifierByDTCType)0x03
#define DEM_GET_ID_WRONG_FF_TYPE			(Dem_GetFreezeFameDataIdentifierByDTCType)0x04

/*
 * Dem_ReturnGetExtendedDataRecordByDTCType
 */
typedef uint8 Dem_ReturnGetExtendedDataRecordByDTCType;
#define DEM_RECORD_OK						(Dem_ReturnGetExtendedDataRecordByDTCType)0x00
#define DEM_RECORD_WRONG_DTC				(Dem_ReturnGetExtendedDataRecordByDTCType)0x01
#define DEM_RECORD_WRONG_DTCORIGIN			(Dem_ReturnGetExtendedDataRecordByDTCType)0x02
#define DEM_RECORD_DTCKIND					(Dem_ReturnGetExtendedDataRecordByDTCType)0x03
#define DEM_RECORD_NUMBER					(Dem_ReturnGetExtendedDataRecordByDTCType)0x04
#define DEM_RECORD_BUFFERSIZE				(Dem_ReturnGetExtendedDataRecordByDTCType)0x05
#define DEM_RECORD_PENDING					(Dem_ReturnGetExtendedDataRecordByDTCType)0x06

/*
 * Dem_ReturnGetDTCByOccurenceTimeType
 */
typedef uint8 Dem_ReturnGetDTCByOccurenceTimeType;
#define DEM_OCCURR_OK						(Dem_ReturnGetDTCByOccurenceTimeType)0x00
#define DEM_OCCURR_WRONG_DTCKIND			(Dem_ReturnGetDTCByOccurenceTimeType)0x01
#define DEM_OCCURR_FAILED					(Dem_ReturnGetDTCByOccurenceTimeType)0x02

/*
 * Dem_ReturnGetFreezeFrameDataByDTCType
 */
typedef uint8 Dem_ReturnGetFreezeFrameDataByDTCType;
#define DEM_GET_FFDATABYDTC_OK				(Dem_ReturnGetFreezeFrameDataByDTCType)0x00
#define DEM_GET_FFDATABYDTC_WRONG_DTC		(Dem_ReturnGetFreezeFrameDataByDTCType)0x01
#define DEM_GET_FFDATABYDTC_WRONG_DTCORIGIN	(Dem_ReturnGetFreezeFrameDataByDTCType)0x02
#define DEM_GET_FFDATABYDTC_WRONG_DTCKIND	(Dem_ReturnGetFreezeFrameDataByDTCType)0x03
#define DEM_GET_FFDATABYDTC_RECORDNUMBER	(Dem_ReturnGetFreezeFrameDataByDTCType)0x04
#define DEM_GET_FFDATABYDTC_WRONG_DATAID	(Dem_ReturnGetFreezeFrameDataByDTCType)0x05
#define DEM_GET_FFDATABYDTC_BUFFERSIZE		(Dem_ReturnGetFreezeFrameDataByDTCType)0x06
#define DEM_GET_ID_PENDING					(Dem_ReturnGetFreezeFrameDataByDTCType)0x07

/*
 * Dem_ReturnGetSizeOfExtendedDataRecordByDTCType
 */
typedef uint8 Dem_ReturnGetSizeOfExtendedDataRecordByDTCType;
#define DEM_GET_SIZEOFEDRBYDTC_OK			(Dem_ReturnGetSizeOfExtendedDataRecordByDTCType)0x00
#define DEM_GET_SIZEOFEDRBYDTC_W_DTC		(Dem_ReturnGetSizeOfExtendedDataRecordByDTCType)0x01
#define DEM_GET_SIZEOFEDRBYDTC_W_DTCOR		(Dem_ReturnGetSizeOfExtendedDataRecordByDTCType)0x02
#define DEM_GET_SIZEOFEDRBYDTC_W_DTCKI		(Dem_ReturnGetSizeOfExtendedDataRecordByDTCType)0x03
#define DEM_GET_SIZEOFEDRBYDTC_W_RNUM		(Dem_ReturnGetSizeOfExtendedDataRecordByDTCType)0x04
#define DEM_GET_SIZEOFEDRBYDTC_PENDING		(Dem_ReturnGetSizeOfExtendedDataRecordByDTCType)0x05

/*
 * Dem_ReturnGetSizeOfFreezeFrameType
 */
typedef uint8 Dem_ReturnGetSizeOfFreezeFrameType;
#define DEM_GET_SIZEOFFF_OK					(Dem_ReturnGetSizeOfFreezeFrameType)0x00
#define DEM_GET_SIZEOFFF_WRONG_DTC			(Dem_ReturnGetSizeOfFreezeFrameType)0x01
#define DEM_GET_SIZEOFFF_WRONG_DTCOR		(Dem_ReturnGetSizeOfFreezeFrameType)0x02
#define DEM_GET_SIZEOFFF_WRONG_DTCKIND		(Dem_ReturnGetSizeOfFreezeFrameType)0x03
#define DEM_GET_SIZEOFFF_WRONG_RNUM			(Dem_ReturnGetSizeOfFreezeFrameType)0x04
#define DEM_GET_SIZEOFFF_PENDING			(Dem_ReturnGetSizeOfFreezeFrameType)0x05

/*
 * Dem_ReturnGetSeverityOfDTCType
 */
typedef uint8 Dem_ReturnGetSeverityOfDTCType;
#define DEM_GET_SEVERITYOFDTC_OK			(Dem_ReturnGetSeverityOfDTCType)0x00
#define DEM_GET_SEVERITYOFDTC_WRONG_DTC		(Dem_ReturnGetSeverityOfDTCType)0x01
#define DEM_GET_SEVERITYOFDTC_WRONG_ORIGIN	(Dem_ReturnGetSeverityOfDTCType)0x02
#define DEM_GET_SEVERITYOFDTC_NOSEVERITY	(Dem_ReturnGetSeverityOfDTCType)0x03


/*******************************************************
 * Definitions where the type is declared in Rte_Dem.h *
 *******************************************************/
/*
 * DemEventStatusExtendedType definitions
 */
#define	DEM_TEST_FAILED 							(Dem_EventStatusExtendedType)0x01
#define	DEM_TEST_FAILED_THIS_OPERATION_CYCLE 		(Dem_EventStatusExtendedType)0x02
#define	DEM_PENDING_DTC 							(Dem_EventStatusExtendedType)0x04
#define	DEM_CONFIRMED_DTC 							(Dem_EventStatusExtendedType)0x08
#define	DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR 	(Dem_EventStatusExtendedType)0x10
#define	DEM_TEST_FAILED_SINCE_LAST_CLEAR 			(Dem_EventStatusExtendedType)0x20
#define	DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE (Dem_EventStatusExtendedType)0x40
#define	DEM_WARNING_INDICATOR_REQUESTED 			(Dem_EventStatusExtendedType)0x80

/*
 * DemDTCKindType definitions
 */
#define	DEM_DTC_KIND_ALL_DTCS			(Dem_DTCKindType)0x01
#define	DEM_DTC_KIND_EMISSION_REL_DTCS	(Dem_DTCKindType)0x02

/*
 * Dem_InitMonitorKindType definitions
 */
#define DEM_INIT_MONITOR_CLEAR		(Dem_InitMonitorKindType)1
#define DEM_INIT_MONITOR_RESTART	(Dem_InitMonitorKindType)2


/*
 * Dem_IndicatorStatusType definitions
 */
#define DEM_INDICATOR_OFF			(Dem_IndicatorStatusType)0
#define DEM_INDICATOR_CONTINUOUS	(Dem_IndicatorStatusType)1
#define DEM_INDICATOR_BLINKING		(Dem_IndicatorStatusType)2
#define DEM_INDICATOR_BLINK_CONT	(Dem_IndicatorStatusType)3

/*
 * DemOperationCycleType definitions
 */
enum {
	DEM_ACTIVE,		// Started by DEM on Dem_PreInit and stopped on Dem_Shutdown

	DEM_POWER,		// Power ON/OFF Cycle
	DEM_IGNITION,	// Ignition ON/OF Cycle
	DEM_WARMUP,		// OBD Warm up Cycle
	DEM_OBD_DCY,	// OBD Driving Cycle

	DEM_OPERATION_CYCLE_ID_ENDMARK
}; /** @req DEM142 */

/*
 * Dem_OperationCycleStateType definitions
 */
#define DEM_CYCLE_STATE_START		(Dem_OperationCycleStateType)1
#define DEM_CYCLE_STATE_END			(Dem_OperationCycleStateType)2



#endif /*DEM_TYPES_H_*/
