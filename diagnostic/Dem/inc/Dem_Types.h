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


#ifndef DEM_TYPES_H_
#define DEM_TYPES_H_
#include "Std_Types.h" /** @req DEM176.Std */
#include "Rte_Dem_Type.h"

/** Enum literals for Dem_DTCFormatType */
#ifndef DEM_DTC_FORMAT_OBD
#define DEM_DTC_FORMAT_OBD 0U
#endif /* DEM_DTC_FORMAT_OBD */

#ifndef DEM_DTC_FORMAT_UDS
#define DEM_DTC_FORMAT_UDS 1U
#endif /* DEM_DTC_FORMAT_UDS */

/** Enum literals for Dem_DTCOriginType */
#ifndef DEM_DTC_ORIGIN_NOT_USED
#define DEM_DTC_ORIGIN_NOT_USED 0U
#endif /* DEM_DTC_ORIGIN_NOT_USED */

#ifndef DEM_DTC_ORIGIN_PRIMARY_MEMORY
#define DEM_DTC_ORIGIN_PRIMARY_MEMORY 1U
#endif /* DEM_DTC_ORIGIN_PRIMARY_MEMORY */

#ifndef DEM_DTC_ORIGIN_MIRROR_MEMORY
#define DEM_DTC_ORIGIN_MIRROR_MEMORY 2U
#endif /* DEM_DTC_ORIGIN_MIRROR_MEMORY */

#ifndef DEM_DTC_ORIGIN_PERMANENT_MEMORY
#define DEM_DTC_ORIGIN_PERMANENT_MEMORY 3U
#endif /* DEM_DTC_ORIGIN_PERMANENT_MEMORY */

#ifndef DEM_DTC_ORIGIN_SECONDARY_MEMORY
#define DEM_DTC_ORIGIN_SECONDARY_MEMORY 4U
#endif /* DEM_DTC_ORIGIN_SECONDARY_MEMORY */

/** Enum literals for Dem_EventStatusExtendedType */
#ifndef DEM_TEST_FAILED
#define DEM_TEST_FAILED 1U
#endif /* DEM_TEST_FAILED */

#ifndef DEM_TEST_FAILED_THIS_OPERATION_CYCLE
#define DEM_TEST_FAILED_THIS_OPERATION_CYCLE 2U
#endif /* DEM_TEST_FAILED_THIS_OPERATION_CYCLE */

#ifndef DEM_PENDING_DTC
#define DEM_PENDING_DTC 4U
#endif /* DEM_PENDING_DTC */

#ifndef DEM_CONFIRMED_DTC
#define DEM_CONFIRMED_DTC 8U
#endif /* DEM_CONFIRMED_DTC */

#ifndef DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR
#define DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR 16U
#endif /* DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR */

#ifndef DEM_TEST_FAILED_SINCE_LAST_CLEAR
#define DEM_TEST_FAILED_SINCE_LAST_CLEAR 32U
#endif /* DEM_TEST_FAILED_SINCE_LAST_CLEAR */

#ifndef DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE
#define DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE 64U
#endif /* DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE */

#ifndef DEM_WARNING_INDICATOR_REQUESTED
#define DEM_WARNING_INDICATOR_REQUESTED 128U
#endif /* DEM_WARNING_INDICATOR_REQUESTED */

/** Enum literals for Dem_EventStatusType */
#ifndef DEM_EVENT_STATUS_PASSED
#define DEM_EVENT_STATUS_PASSED 0U
#endif /* DEM_EVENT_STATUS_PASSED */

#ifndef DEM_EVENT_STATUS_FAILED
#define DEM_EVENT_STATUS_FAILED 1U
#endif /* DEM_EVENT_STATUS_FAILED */

#ifndef DEM_EVENT_STATUS_PREPASSED
#define DEM_EVENT_STATUS_PREPASSED 2U
#endif /* DEM_EVENT_STATUS_PREPASSED */

#ifndef DEM_EVENT_STATUS_PREFAILED
#define DEM_EVENT_STATUS_PREFAILED 3U
#endif /* DEM_EVENT_STATUS_PREFAILED */

/** Enum literals for Dem_OperationCycleStateType */
#ifndef DEM_CYCLE_STATE_START
#define DEM_CYCLE_STATE_START 0U
#endif /* DEM_CYCLE_STATE_START */

#ifndef DEM_CYCLE_STATE_END
#define DEM_CYCLE_STATE_END 1U
#endif /* DEM_CYCLE_STATE_END */

/** Enum literals for Dem_ReturnClearDTCType */
#ifndef DEM_CLEAR_OK
#define DEM_CLEAR_OK 0U
#endif /* DEM_CLEAR_OK */

#ifndef DEM_CLEAR_WRONG_DTC
#define DEM_CLEAR_WRONG_DTC 1U
#endif /* DEM_CLEAR_WRONG_DTC */

#ifndef DEM_CLEAR_WRONG_DTCORIGIN
#define DEM_CLEAR_WRONG_DTCORIGIN 2U
#endif /* DEM_CLEAR_WRONG_DTCORIGIN */

#ifndef DEM_CLEAR_WRONG_DTCKIND
#define DEM_CLEAR_WRONG_DTCKIND 3U
#endif /* DEM_CLEAR_WRONG_DTCKIND */

#ifndef DEM_CLEAR_FAILED
#define DEM_CLEAR_FAILED 4U
#endif /* DEM_CLEAR_FAILED */

#ifndef DEM_DTC_PENDING
#define DEM_DTC_PENDING 5U
#endif /* DEM_DTC_PENDING */

#define DEM_EVENT_DESTINATION_END_OF_LIST 0

/*
 * DTC storage types
 */
typedef uint8 Dem_DTCTranslationFormatType;
#define DEM_DTC_TRANSLATION_ISO15031_6      0x00
#define DEM_DTC_TRANSLATION_ISO14229_1      0x01
#define DEM_DTC_TRANSLATION_SAEJ1939_73     0x02
#define DEM_DTC_TRANSLATION_ISO11992_4      0x03

/*
 * Dem_DTCGroupType
 */
typedef uint32 Dem_DTCGroupType;
#define DEM_DTC_GROUP_EMISSION_REL_DTCS (Dem_DTCGroupType)0x0
#define	DEM_DTC_GROUP_ALL_DTCS			(Dem_DTCGroupType)0xffffff

/*
 * Dem status type
 */
#define DEM_DTC_STATUS_MASK_ALL			(uint8)0x00


/*
 * Dem_FreezeFrameKindType
 */
typedef uint8 Dem_FreezeFrameKindType;	// NOTE: Check type and values
#define	DEM_FREEZE_FRAME_NON_OBD 	(Dem_FreezeFrameKindType)0x01
#define DEM_FREEZE_FRAME_OBD		(Dem_FreezeFrameKindType)0x02

/*
 * Dem_EventKindType
 */
typedef uint8 Dem_EventKindType;		// NOTE: Check type and values
#define DEM_EVENT_KIND_BSW		(Dem_EventKindType)0x01
#define DEM_EVENT_KIND_SWC		(Dem_EventKindType)0x02

/*
 * Dem_PreDebounceNameType
 */
typedef uint8 Dem_PreDebounceNameType;
enum {
    DEM_NO_PRE_DEBOUNCE,
    DEM_PRE_DEBOUNCE_COUNTER_BASED,
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
#define DEM_SEVERITY_NO_SEVERITY        0x00u /* No severity information available */
#define DEM_SEVERITY_MAINTENANCE_ONLY   0x20u
#define DEM_SEVERITY_CHECK_AT_NEXT_HALT 0x40u
#define DEM_SEVERITY_CHECK_IMMEDIATELY  0x80u

/*
 * Dem_ReturnSetDTCFilterType
 */
typedef uint8 Dem_ReturnSetFilterType;
#define DEM_FILTER_ACCEPTED	(Dem_ReturnSetFilterType)0x00
#define DEM_WRONG_FILTER	(Dem_ReturnSetFilterType)0x01

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

/*******************************************************
 * Definitions where the type is declared in Rte_Dem.h *
 *******************************************************/
/*
 * DemDTCKindType definitions
 */
typedef uint8 Dem_DTCKindType;

#ifndef DEM_DTC_KIND_ALL_DTCS
#define DEM_DTC_KIND_ALL_DTCS (Dem_DTCKindType)0x01
#endif /* DEM_DTC_KIND_ALL_DTCS */

#ifndef DEM_DTC_KIND_EMISSION_REL_DTCS
#define DEM_DTC_KIND_EMISSION_REL_DTCS (Dem_DTCKindType)0x02
#endif /* DEM_DTC_KIND_EMISSION_REL_DTCS */

/*
 * Dem_InitMonitorKindType definitions
 */
#ifndef DEM_INIT_MONITOR_CLEAR
#define DEM_INIT_MONITOR_CLEAR (Dem_InitMonitorReasonType)1
#endif /* DEM_INIT_MONITOR_CLEAR */

#ifndef DEM_INIT_MONITOR_RESTART
#define DEM_INIT_MONITOR_RESTART (Dem_InitMonitorReasonType)2
#endif /* DEM_INIT_MONITOR_RESTART */


/*
 * Dem_IndicatorStatusType definitions
 */
#ifndef DEM_INDICATOR_OFF
#define DEM_INDICATOR_OFF 0U
#endif /* DEM_INDICATOR_OFF */
#ifndef DEM_INDICATOR_CONTINUOUS
#define DEM_INDICATOR_CONTINUOUS 1U
#endif /* DEM_INDICATOR_CONTINUOUS */
#ifndef DEM_INDICATOR_BLINKING
#define DEM_INDICATOR_BLINKING 2U
#endif /* DEM_INDICATOR_BLINKING */
#ifndef DEM_INDICATOR_BLINK_CONT
#define DEM_INDICATOR_BLINK_CONT 3U
#endif /* DEM_INDICATOR_BLINK_CONT */


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
};/** @req DEM480 */

/*
 * Dem_ReturnGetSeverityOfDTCType
 */
typedef uint8 Dem_ReturnGetSeverityOfDTCType;
#define DEM_GET_SEVERITYOFDTC_OK 			(Dem_ReturnGetSeverityOfDTCType)0x00
#define DEM_GET_SEVERITYOFDTC_WRONG_DTC 	(Dem_ReturnGetSeverityOfDTCType)0x01
#define DEM_GET_SEVERITYOFDTC_NOSEVERITY 	(Dem_ReturnGetSeverityOfDTCType)0x02
#define DEM_GET_SEVERITYOFDTC_PENDING 		(Dem_ReturnGetSeverityOfDTCType)0x03

/*
 * Dem_ReturnDisableDTCRecordUpdateType
 */
typedef uint8 Dem_ReturnDisableDTCRecordUpdateType;
#define DEM_DISABLE_DTCRECUP_OK 				(Dem_ReturnDisableDTCRecordUpdateType)0x00
#define DEM_DISABLE_DTCRECUP_WRONG_DTC 			(Dem_ReturnDisableDTCRecordUpdateType)0x01
#define DEM_DISABLE_DTCRECUP_WRONG_DTCORIGIN 	(Dem_ReturnDisableDTCRecordUpdateType)0x02
#define DEM_DISABLE_DTCRECUP_PENDING 			(Dem_ReturnDisableDTCRecordUpdateType)0x03

/*
 * Dem_ReturnGetSizeOfFreezeFrameByDTCType
 */
typedef uint8 Dem_ReturnGetSizeOfFreezeFrameByDTCType;
//conflict in type check
//#define DEM_GET_SIZEOFFF_OK 			(Dem_ReturnGetSizeOfFreezeFrameByDTCType)0x00
//#define DEM_GET_SIZEOFFF_WRONG_DTC 		(Dem_ReturnGetSizeOfFreezeFrameByDTCType)0x01
//#define DEM_GET_SIZEOFFF_WRONG_DTCOR 	(Dem_ReturnGetSizeOfFreezeFrameByDTCType)0x02
//#define DEM_GET_SIZEOFFF_WRONG_RNUM 	(Dem_ReturnGetSizeOfFreezeFrameByDTCType)0x03
//#define DEM_GET_SIZEOFFF_PENDING 		(Dem_ReturnGetSizeOfFreezeFrameByDTCType)0x04

/*
 * Dem_ReturnGetFreezeFrameDataByDTCType
 */
//typedef uint8 Dem_ReturnGetFreezeFrameDataByDTCType;
//#define DEM_GET_FFDATABYDTC_OK					(Dem_ReturnGetFreezeFrameDataByDTCType)0x00
//#define DEM_GET_FFDATABYDTC_WRONG_DTC			(Dem_ReturnGetFreezeFrameDataByDTCType)0x01
//#define DEM_GET_FFDATABYDTC_WRONG_DTCORIGIN		(Dem_ReturnGetFreezeFrameDataByDTCType)0x02
//#define DEM_GET_FFDATABYDTC_WRONG_RECORDNUMBER 	(Dem_ReturnGetFreezeFrameDataByDTCType)0x03
//#define DEM_GET_FFDATABYDTC_WRONG_BUFFERSIZE 	(Dem_ReturnGetFreezeFrameDataByDTCType)0x04
#define DEM_GET_FFDATABYDTC_PENDING 			(Dem_ReturnGetFreezeFrameDataByDTCType)0x05

#define DEM_RECORD_WRONG_NUMBER (Dem_ReturnGetExtendedDataRecordByDTCType)0x03

/* Type from Dem 4.3.0. Used by FiM. Should be moved but kept here as it is not used in Dem service component. */
typedef uint8 Dem_MonitorStatusType;
#define DEM_MONITOR_STATUS_TF 0x01u
#define DEM_MONITOR_STATUS_TNCTOC 0x02u

/*
 * DemOBDEngineType
 */
#define DEM_IGNITION_COMPR 0x00
#define DEM_IGNITION_SPARK 0x01

/*
 * DemEventOBDReadinessGroup
 */
/* @req DEM349 */
typedef uint8 Dem_EventOBDReadinessGroup;
#define DEM_OBD_RDY_NONE				(Dem_EventOBDReadinessGroup)0x00
#define DEM_OBD_RDY_AC					(Dem_EventOBDReadinessGroup)0x01
#define DEM_OBD_RDY_BOOSTPR				(Dem_EventOBDReadinessGroup)0x02
#define DEM_OBD_RDY_CAT					(Dem_EventOBDReadinessGroup)0x03
#define DEM_OBD_RDY_CMPRCMPT			(Dem_EventOBDReadinessGroup)0x04
#define DEM_OBD_RDY_EGSENS				(Dem_EventOBDReadinessGroup)0x05
#define DEM_OBD_RDY_ERG					(Dem_EventOBDReadinessGroup)0x06
#define DEM_OBD_RDY_EVAP				(Dem_EventOBDReadinessGroup)0x07
#define DEM_OBD_RDY_FLSYS				(Dem_EventOBDReadinessGroup)0x08
#define DEM_OBD_RDY_HCCAT				(Dem_EventOBDReadinessGroup)0x09
#define DEM_OBD_RDY_HTCAT				(Dem_EventOBDReadinessGroup)0x0A
#define DEM_OBD_RDY_MISF				(Dem_EventOBDReadinessGroup)0x0B
#define DEM_OBD_RDY_NOXCAT				(Dem_EventOBDReadinessGroup)0x0C
#define DEM_OBD_RDY_O2SENS				(Dem_EventOBDReadinessGroup)0x0D
#define DEM_OBD_RDY_O2SENSHT			(Dem_EventOBDReadinessGroup)0x0E
#define DEM_OBD_RDY_PMFLT				(Dem_EventOBDReadinessGroup)0x0F
#define DEM_OBD_RDY_SECAIR				(Dem_EventOBDReadinessGroup)0x10

typedef uint8 Dem_IUMPRGroup;
#define DEM_IUMPR_AFRI1					(Dem_IUMPRGroup)0x00
#define DEM_IUMPR_AFRI2					(Dem_IUMPRGroup)0x01
#define DEM_IUMPR_BOOSTPRS				(Dem_IUMPRGroup)0x02
#define DEM_IUMPR_CAT1					(Dem_IUMPRGroup)0x03
#define DEM_IUMPR_CAT2					(Dem_IUMPRGroup)0x04
#define DEM_IUMPR_EGR					(Dem_IUMPRGroup)0x05
#define DEM_IUMPR_EGSENSOR				(Dem_IUMPRGroup)0x06
#define DEM_IUMPR_EVAP					(Dem_IUMPRGroup)0x07
#define DEM_IUMPR_FLSYS					(Dem_IUMPRGroup)0x08
#define DEM_IUMPR_NMHCCAT				(Dem_IUMPRGroup)0x09
#define DEM_IUMPR_NOXADSORB				(Dem_IUMPRGroup)0x0A
#define DEM_IUMPR_NOXCAT				(Dem_IUMPRGroup)0x0B
#define DEM_IUMPR_OXS1					(Dem_IUMPRGroup)0x0C
#define DEM_IUMPR_OXS2					(Dem_IUMPRGroup)0x0D
#define DEM_IUMPR_PF1					(Dem_IUMPRGroup)0x0E
#define DEM_IUMPR_PF2					(Dem_IUMPRGroup)0x0F
#define DEM_IUMPR_PMFILTER				(Dem_IUMPRGroup)0x10
#define DEM_IUMPR_PRIVATE				(Dem_IUMPRGroup)0x11
#define DEM_IUMPR_SAIR					(Dem_IUMPRGroup)0x12
#define DEM_IUMPR_SECOXS1				(Dem_IUMPRGroup)0x13
#define DEM_IUMPR_SECOXS2				(Dem_IUMPRGroup)0x14

typedef uint8 Dem_RatioKindType;
#define DEM_RATIO_API					(Dem_RatioKindType) 0x00
#define DEM_RATIO_OBSERVER				(Dem_RatioKindType) 0x01

#endif /*DEM_TYPES_H_*/
