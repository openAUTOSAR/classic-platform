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

/*
 * DTC storage types
 */
typedef uint8 Dem_ReturnTypeOfDtcSupportedType;
#define DEM_ISO15031_6		0x00
#define DEM_ISO14229_1		0x01
#define DEM_SAEJ1939_73		0x02
#define DEM_ISO_11992_4		0x03

/*
 * Dem_EventIdType
 */
typedef uint16 Dem_EventIdType;

/*
 * Dem_DTCGroupType
 */
typedef uint32 Dem_DTCGroupType;
#define	DEM_DTC_GROUP_ALL_DTCS			0xffffff

/*
 * Dem status type
 */
#define DEM_DTC_STATUS_MASK_ALL			0x00


/*
 * DemDTCKindType
 */
typedef uint8 Dem_DTCKindType;
#define	DEM_DTC_KIND_ALL_DTCS			0x01
#define	DEM_DTC_KIND_EMISSION_REL_DTCS	0x02

/*
 * DemDTCOriginType
 */
typedef uint8 Dem_DTCOriginType;
#define	DEM_DTC_ORIGIN_SECONDARY_MEMORY	0x01
#define	DEM_DTC_ORIGIN_PRIMARY_MEMORY	0x02
#define	DEM_DTC_ORIGIN_PERMANENT_MEMORY	0x03
#define	DEM_DTC_ORIGIN_MIRROR_MEMORY	0x04

/*
 * DemEventStatusExtendedType
 */
typedef uint8 Dem_EventStatusExtendedType;
#define	DEM_TEST_FAILED 							0x01
#define	DEM_TEST_FAILED_THIS_OPERATION_CYCLE 		0x02
#define	DEM_PENDING_DTC 							0x04
#define	DEM_CONFIRMED_DTC 							0x08
#define	DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR 	0x10
#define	DEM_TEST_FAILED_SINCE_LAST_CLEAR 			0x20
#define	DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE 0x40
#define	DEM_WARNING_INDICATOR_REQUESTED 			0x80

/*
 * DemOperationCycleType
 */
typedef uint8 Dem_OperationCycleIdType;	// TODO: Check type and values
enum {
	DEM_ACTIVE,		// Started by DEM on Dem_PreInit and stopped on Dem_Shutdown

	DEM_POWER,		// Power ON/OFF Cycle
	DEM_IGNITION,	// Ignition ON/OF Cycle
	DEM_WARMUP,		// OBD Warm up Cycle
	DEM_OBD_DCY,	// OBD Driving Cycle

	DEM_OPERATION_CYCLE_ID_ENDMARK
}; /** @req DEM142 */

/*
 * Dem_OperationCycleStateType
 */
typedef uint8 Dem_OperationCycleStateType;
#define DEM_CYCLE_STATE_START		1
#define DEM_CYCLE_STATE_END			2

/*
 * Dem_FreezeFrameKindType
 */
typedef uint8 Dem_FreezeFrameKindType;	// TODO: Check type and values
#define	DEM_FREEZE_FRAME_NON_OBD 	0x01
#define DEM_FREEZE_FRAME_OBD		0x02

/*
 * Dem_EventKindType
 */
typedef uint8 Dem_EventKindType;		// TODO: Check type and values
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
 * Dem_IndicatorStatusType
 */
typedef uint8 Dem_IndicatorStatusType;
#define DEM_INDICATOR_OFF			0
#define DEM_INDICATOR_CONTINUOUS	1
#define DEM_INDICATOR_BLINKING		2
#define DEM_INDICATOR_BLINK_CONT	3

/*
 * Dem_FaultDetectionCpunterType
 */
typedef sint8 Dem_FaultDetectionCounterType;

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
#define DEM_FILTER_WITH_SEVERITY_YES    0x00
#define DEM_FILTER_WITH_SEVERITY_NO     0x01

/*
 * Dem_FilterForFDCType
 */
typedef uint8 Dem_FilterForFDCType;
#define DEM_FILTER_FOR_FDC_YES          0x00
#define DEM_FILTER_FOR_FDC_NO           0x01

/*
 * Dem_DTCSeverityType
 */
typedef uint8 Dem_DTCSeverityType;
#define DEM_SEVERITY_NO_SEVERITY        0x00 // No severity information available
#define DEM_SEVERITY_MAINTENANCE_ONLY   0x20
#define DEM_SEVERITY_CHECK_AT_NEXT_FALT 0x40
#define DEM_SEVERITY_CHECK_IMMEDIATELY  0x80

/*
 * Dem_ReturnSetDTCFilterType
 */
typedef uint8 Dem_ReturnSetDTCFilterType;
#define DEM_FILTER_ACCEPTED	0x00
#define DEM_WRONG_FILTER	0x01

/*
 * Dem_ReturnGetStatusOfDTCType
 */
typedef uint8 Dem_ReturnGetStatusOfDTCType;
#define DEM_STATUS_OK						0x00
#define DEM_STATUS_WRONG_DTC				0x01
#define DEM_STATUS_WRONG_DTCORIGIN			0x02
#define DEM_STATUS_FAILED					0x04
#define DEM_STATUS_WRONG_DTCKIND			0x03

/*
 * Dem_ReturnGetNextFilteredDTCType
 */
typedef uint8 Dem_ReturnGetNextFilteredDTCType;
#define DEM_FILTERED_OK						0x00
#define DEM_FILTERED_NO_MATCHING_DTC		0x01
#define DEM_FILTERED_WRONG_DTCKIND			0x02
#define DEM_FILTERED_PENDING				0x03

/*
 * Dem_ReturnGetNumberOfFilteredDTCType
 */
typedef uint8 Dem_ReturnGetNumberOfFilteredDTCType;
#define DEM_NUMBER_OK						0x00
#define DEM_NUMBER_FAILED					0x01
#define DEM_NUMBER_PENDING					0x02

/*
 * Dem_ReturnClearDTCType
 */
typedef uint8 Dem_ReturnClearDTCType;
#define DEM_CLEAR_OK						0x00
#define DEM_CLEAR_WRONG_DTC					0x01
#define DEM_CLEAR_WRONG_DTCORIGIN			0x02
#define DEM_CLEAR_WRONG_DTCKIND				0x03
#define DEM_CLEAR_FAILED					0x04
#define DEM_DTC_PENDING						0x05

/*
 * Dem_ReturnControlDTCStorageType
 */
typedef uint8 Dem_ReturnControlDTCStorageType;
#define DEM_CONTROL_DTC_STORAGE_OK			0x00
#define DEM_CONTROL_DTC_STORAGE_N_OK		0x01
#define DEM_CONTROL_DTC_WRONG_DTCGROUP		0x02

/*
 * Dem_ReturnControlEventUpdateType
 */
typedef uint8 Dem_ReturnControlEventUpdateType;
#define DEM_CONTROL_EVENT_UPDATE_OK			0x00
#define DEM_CONTROL_EVENT_N_OK				0x01
#define DEM_CONTROL_EVENT_WRONG_DTCGROUP	0x02

/*
 * Dem_ReturnGetDTCOfFreezeframeRecordType
 */
typedef uint8 Dem_ReturnGetDTCOfFreezeframeRecordType;
#define DEM_GET_DTCOFFF_OK					0x00
#define DEM_GET_DTCOFFF_WRONG_RECORD		0x01
#define DEM_GET_DTCOFFF_NO_DTC_FOR_RECORD	0x02
#define DEM_GET_DTCOFFF_WRONG_DTCKIND		0x03

/*
 * Dem_GetFreezeFameDataIdentifierByDTCType
 */
typedef uint8 Dem_GetFreezeFameDataIdentifierByDTCType;
#define DEM_GET_ID_OK						0x00
#define DEM_GET_ID_WRONG_DTC				0x01
#define DEM_GET_ID_WRONG_DTCORIGIN			0x02
#define DEM_GET_ID_WRONG_DTCKIND			0x03
#define DEM_GET_ID_WRONG_FF_TYPE			0x04

/*
 * Dem_ReturnGetExtendedDataRecordByDTCType
 */
typedef uint8 Dem_ReturnGetExtendedDataRecordByDTCType;
#define DEM_RECORD_OK						0x00
#define DEM_RECORD_WRONG_DTC				0x01
#define DEM_RECORD_WRONG_DTCORIGIN			0x02
#define DEM_RECORD_DTCKIND					0x03
#define DEM_RECORD_NUMBER					0x04
#define DEM_RECORD_BUFFERSIZE				0x05
#define DEM_RECORD_PENDING					0x06

/*
 * Dem_ReturnGetDTCByOccurenceTimeType
 */
typedef uint8 Dem_ReturnGetDTCByOccurenceTimeType;
#define DEM_OCCURR_OK						0x00
#define DEM_OCCURR_WRONG_DTCKIND			0x01
#define DEM_OCCURR_FAILED					0x02

/*
 * Dem_ReturnGetFreezeFrameDataByDTCType
 */
typedef uint8 Dem_ReturnGetFreezeFrameDataByDTCType;
#define DEM_GET_FFDATABYDTC_OK				0x00
#define DEM_GET_FFDATABYDTC_WRONG_DTC		0x01
#define DEM_GET_FFDATABYDTC_WRONG_DTCORIGIN	0x02
#define DEM_GET_FFDATABYDTC_WRONG_DTCKIND	0x03
#define DEM_GET_FFDATABYDTC_RECORDNUMBER	0x04
#define DEM_GET_FFDATABYDTC_WRONG_DATAID	0x05
#define DEM_GET_FFDATABYDTC_BUFFERSIZE		0x06
#define DEM_GET_ID_PENDING					0x07

/*
 * Dem_ReturnGetSizeOfExtendedDataRecordByDTCType
 */
typedef uint8 Dem_ReturnGetSizeOfExtendedDataRecordByDTCType;
#define DEM_GET_SIZEOFEDRBYDTC_OK			0x00
#define DEM_GET_SIZEOFEDRBYDTC_W_DTC		0x01
#define DEM_GET_SIZEOFEDRBYDTC_W_DTCOR		0x02
#define DEM_GET_SIZEOFEDRBYDTC_W_DTCKI		0x03
#define DEM_GET_SIZEOFEDRBYDTC_W_RNUM		0x04
#define DEM_GET_SIZEOFEDRBYDTC_PENDING		0x05

/*
 * Dem_ReturnGetSizeOfFreezeFrameType
 */
typedef uint8 Dem_ReturnGetSizeOfFreezeFrameType;
#define DEM_GET_SIZEOFFF_OK					0x00
#define DEM_GET_SIZEOFFF_WRONG_DTC			0x01
#define DEM_GET_SIZEOFFF_WRONG_DTCOR		0x02
#define DEM_GET_SIZEOFFF_WRONG_DTCKIND		0x03
#define DEM_GET_SIZEOFFF_WRONG_RNUM			0x04
#define DEM_GET_SIZEOFFF_PENDING			0x05

/*
 * Dem_ReturnGetSeverityOfDTCType
 */
typedef uint8 Dem_ReturnGetSeverityOfDTCType;
#define DEM_GET_SEVERITYOFDTC_OK			0x00
#define DEM_GET_SEVERITYOFDTC_WRONG_DTC		0x01
#define DEM_GET_SEVERITYOFDTC_WRONG_ORIGIN	0x02
#define DEM_GET_SEVERITYOFDTC_NOSEVERITY	0x03



#endif /*DEM_TYPES_H_*/
