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








#ifndef DEM_H_
#define DEM_H_

/* @req DEM628 */
#define DEM_MODULE_ID			54u
#define DEM_VENDOR_ID			60u

#define DEM_AR_RELEASE_MAJOR_VERSION   		4u
#define DEM_AR_RELEASE_MINOR_VERSION   		0u
#define DEM_AR_RELEASE_REVISION_VERSION   	3u

#define DEM_SW_MAJOR_VERSION    5u
#define DEM_SW_MINOR_VERSION   	19u
#define DEM_SW_PATCH_VERSION    0u
#define DEM_AR_MAJOR_VERSION    DEM_AR_RELEASE_MAJOR_VERSION
#define DEM_AR_MINOR_VERSION    DEM_AR_RELEASE_MINOR_VERSION
#define DEM_AR_PATCH_VERSION    DEM_AR_RELEASE_REVISION_VERSION

#include "Dem_Types.h"
#include "Dem_Cfg.h"
#include "Dem_Lcfg.h"
#include "Dem_IntErrId.h" /** @req DEM115 */
#include "Dem_IntEvtId.h"
#include "Dem_EnableCondId.h"
#include <limits.h>
/** @req DEM153 */ /** @req DEM154 */ /** @req DEM024 *//* Realized in Dem_IntErrId.h and Dem_IntEvtId.h */
// #include "Rte_Dem.h"


#if (DEM_DEV_ERROR_DETECT == STD_ON)
// Error codes reported by this module defined by AUTOSAR /** @req DEM116 */ /** @req DEM173 */
#define DEM_E_PARAM_CONFIG                  0x10u
#define DEM_E_PARAM_POINTER                 0x11u
#define DEM_E_PARAM_DATA                    0x12u
#define DEM_E_PARAM_LENGTH                  0x13u
#define DEM_E_UNINIT                        0x20u
#define DEM_E_NODATAAVAILABLE               0x30u
#define DEM_E_WRONG_CONDITION               0x40u
// Other error codes reported by this module
#define DEM_E_CONFIG_PTR_INVALID            0x50u
#define DEM_E_EXT_DATA_TOO_BIG              0x52u
#define DEM_E_PRE_INIT_EXT_DATA_BUFF_FULL   0x53u
#define DEM_E_MEM_EVENT_BUFF_FULL           0x54u
#define DEM_E_MEM_EXT_DATA_BUFF_FULL        0x55u
#define DEM_E_FF_TOO_BIG                    0x56u
#define DEM_E_PRE_INIT_FF_DATA_BUFF_FULL	0x57u
#define DEM_E_MEM_FF_DATA_BUFF_FULL	        0x58u
#define DEM_E_OBD_NOT_ALLOWED_IN_SEC_MEM    0x59u
#define DEM_E_MEMORY_CORRUPT                0x5Au



#define DEM_E_SEQUENCE_ERROR                0xfdu
#define DEM_E_UNEXPECTED_EXECUTION          0xfeu
#define DEM_E_NOT_IMPLEMENTED_YET           0xffu


// Service ID in this module
#define DEM_PREINIT_ID							0x01u
#define DEM_INIT_ID								0x02u
#define DEM_SHUTDOWN_ID							0x03u
#define DEM_SETEVENTSTATUS_ID					0x04u
#define DEM_RESETEVENTSTATUS_ID					0x05u
#define DEM_SETOPERATIONCYCLESTATE_ID			0x08u
#define DEM_GETEVENTSTATUS_ID					0x0Au
#define DEM_GETEVENTFAILED_ID					0x0Bu
#define DEM_GETEVENTTESTED_ID					0x0Cu
#define DEM_GETDTCOFEVENT_ID					0x0Du
#define DEM_GETSEVERITYOFDTC_ID                 0x0Eu
#define DEM_REPORTERRORSTATUS_ID 				0x0Fu
#define DEM_SETDTCFILTER_ID						0x13u
#define DEM_GETSTATUSOFDTC_ID					0x15u
#define DEM_GETDTCSTATUSAVAILABILITYMASK_ID		0x16u
#define DEM_GETNUMBEROFFILTEREDDTC_ID			0x17u
#define DEM_GETNEXTFILTEREDDTC_ID				0x18u
#define DEM_DISABLEDTCRECORDUPDATE_ID           0x1Au
#define DEM_ENABLEDTCRECORDUPDATE_ID            0x1Bu
#define DEM_GETFREEZEFRAMEDATABYDTC_ID          0x1Du
#define DEM_GETSIZEOFFREEZEFRAMEBYDTC_ID        0x1Fu
#define DEM_GETEXTENDEDDATARECORDBYDTC_ID		0x20u
#define DEM_GETSIZEOFEXTENDEDDATARECORDBYDTC_ID	0x21u
#define DEM_CLEARDTC_ID							0x22u
#define DEM_DISABLEDTCSETTING_ID				0x24u
#define DEM_ENABLEDTCSETTING_ID					0x25u

#define DEM_GETINDICATORSTATUS_ID               0x29u

#define DEM_GETEVENTEXTENDEDDATARECORD_ID       0x30u
#define DEM_GETEVENTFREEZEFRAMEDATA_ID          0x31u

#define DEM_GETEVENTMEMORYOVERFLOW_ID           0x32u
#define DEM_SETDTCSUPPRESSION_ID                0x33u
#define DEM_SETEVENTAVAILABLE_ID                0x37u /* ASR 4.2.2 API */
#define DEM_GETNEXTFILTEREDRECORD_ID            0x3au

#define DEM_GETTRANSLATIONTYPE_ID				0x3cu
#define DEM_SETENABLECONDITION_ID               0x39u
#define DEM_GETFAULTDETECTIONCOUNTER_ID 		0x3Eu
#define DEM_SETFREEZEFRAMERECORDFILTER_ID       0x3Fu
#define DEM_DLTGETALLEXTENDEDDATARECORDS_ID     0x40u
#define DEM_DLTGETMOSTRECENTFREEZEFRAMERECORDDATA_ID 0x41u
#define DEM_READDATAOFOBDFREEZEFRAME_ID         0X52u
#define DEM_GETDTCOFOBDFREEZEFRAME_ID           0x53u
#define DEM_MAINFUNCTION_ID						0x55u
#define DEM_REPIUMPRFAULTDETECT_ID				0x73u
#define DEM_UPDATE_EVENT_STATUS_ID				0x80u
#define DEM_MERGE_EVENT_STATUS_ID				0x81u
#define DEM_GET_EXTENDED_DATA_ID				0x82u
#define DEM_STORE_EXT_DATA_PRE_INIT_ID			0x83u
#define DEM_STORE_EVENT_MEM_ID				    0x84u
#define DEM_STORE_EXT_DATA_MEM_ID			    0x85u
#define DEM_PREDEBOUNCE_NONE_ID					0x86u
#define DEM_PREDEBOUNCE_COUNTER_BASED_ID		0x87u
#define DEM_GET_FREEZEFRAME_ID					0x88u
#define DEM_STORE_FF_DATA_PRE_INIT_ID			0x89u
#define DEM_STORE_FF_DATA_MEM_ID			    0x90u

#define DEM_PRE_STORE_FF_ID						0x91u
#define DEM_CLEAR_PRE_STORED_FF_ID  			0x92u
#define DEM_READ_DATA_LENGTH_FAILED				0x93u

#define DEM_FF_NULLREF 							255u

#define DEM_GETMONITORSTATUS_ID				    0xB5u /* ASR 4.3.0 API */

#define DEM_GLOBAL_ID							0xffu

#endif

#define DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT     UINT32_MAX //when timestamp up to the max value,rearrangement starts.
#define DEM_MAX_TIMESTAMP_FOR_PRE_INIT          (uint32)(UINT32_MAX/2)
/*
 * Interface for upper layer modules
 */
#if ( DEM_VERSION_INFO_API == STD_ON ) /** @req DEM111 */
#define Dem_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,DEM) /** @req DEM177 */ /** @req DEM110 */
#endif /* DEM_VERSION_INFO_API */

/*
 * Interface ECU State Manager <-> DEM
 */
void Dem_PreInit( const Dem_ConfigType *ConfigPtr ); /** @req DEM179 */
void Dem_Init( void ); /** @req DEM181 */
void Dem_Shutdown( void ); /** @req DEM182 */


/*
 * Interface for basic software scheduler
 */
void Dem_MainFunction( void ); /** @req DEM266 */


/*
 * Interface BSW modules/SW-Components via RTE <-> DEM
 */
void Dem_ReportErrorStatus(Dem_EventIdType eventId ,Dem_EventStatusType eventStatus); /** @req DEM206 */
#if !defined(USE_RTE)
Std_ReturnType Dem_SetEventStatus(Dem_EventIdType eventId, Dem_EventStatusType eventStatus);
Std_ReturnType Dem_ResetEventStatus(Dem_EventIdType eventId);
#endif
Std_ReturnType Dem_SetOperationCycleState(Dem_OperationCycleIdType operationCycleId, Dem_OperationCycleStateType cycleState); /** @req DEM194 */
Std_ReturnType Dem_GetEventStatus(Dem_EventIdType eventId, Dem_EventStatusExtendedType *eventStatusExtended); /** @req DEM195 */
Std_ReturnType Dem_GetEventFailed(Dem_EventIdType eventId, boolean *eventFailed); /** @req DEM196 */
Std_ReturnType Dem_GetEventTested(Dem_EventIdType eventId, boolean *eventTested); /** @req DEM197 */
Std_ReturnType Dem_GetDTCOfEvent(Dem_EventIdType eventId, Dem_DTCFormatType dtcFormat, uint32* dtcOfEvent); /** @req DEM198 */
#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
Std_ReturnType Dem_SetEnableCondition(uint8 EnableConditionID, boolean ConditionFulfilled); /** @req DEM201*/
#endif
Std_ReturnType Dem_GetFaultDetectionCounter(Dem_EventIdType eventId, sint8 *counter); /** @req DEM203 */
Std_ReturnType Dem_GetIndicatorStatus( uint8 IndicatorId, Dem_IndicatorStatusType* IndicatorStatus ); /* @req DEM205 */
Std_ReturnType Dem_GetEventFreezeFrameData(Dem_EventIdType EventId, uint8 RecordNumber, boolean ReportTotalRecord, uint16 DataId, uint8* DestBuffer);/* @req DEM558 */
Std_ReturnType Dem_GetEventExtendedDataRecord(Dem_EventIdType EventId, uint8 RecordNumber, uint8* DestBuffer);/* @req DEM557 */
Std_ReturnType Dem_GetEventMemoryOverflow(Dem_DTCOriginType DTCOrigin, boolean *OverflowIndication); /* @req DEM559 */

#if (DEM_DTC_SUPPRESSION_SUPPORT == STD_ON)
Std_ReturnType Dem_SetDTCSuppression(uint32 DTC, Dem_DTCFormatType DTCFormat, boolean SuppressionStatus);/* @req 4.2.2/SWS_Dem_01047 *//* @req DEM583 */
#endif
Std_ReturnType Dem_SetEventAvailable(Dem_EventIdType EventId, boolean AvailableStatus);/* @req 4.2.2/SWS_Dem_01080 */
/*
 * Interface DCM <-> DEM
 */
/* Access DTCs and status information */
Dem_ReturnSetFilterType Dem_SetDTCFilter(uint8 dtcStatusMask, Dem_DTCKindType dtcKind, Dem_DTCFormatType dtcFormat, Dem_DTCOriginType dtcOrigin, Dem_FilterWithSeverityType filterWithSeverity, Dem_DTCSeverityType dtcSeverityMask, Dem_FilterForFDCType filterForFaultDetectionCounter);  /** @req DEM208 */
Dem_ReturnSetFilterType Dem_SetFreezeFrameRecordFilter(Dem_DTCFormatType dtcFormat, uint16 *NumberOfFilteredRecords);/* @req DEM209 */
Dem_ReturnGetStatusOfDTCType Dem_GetStatusOfDTC(uint32 dtc, Dem_DTCOriginType dtcOrigin, Dem_EventStatusExtendedType* status); /** @req DEM212 */
Std_ReturnType Dem_GetDTCStatusAvailabilityMask(uint8 *dtcStatusMask); /** @req DEM213 */
Dem_ReturnGetNumberOfFilteredDTCType Dem_GetNumberOfFilteredDtc(uint16* numberOfFilteredDTC); /** @req DEM214 */
Dem_ReturnGetNextFilteredDTCType Dem_GetNextFilteredDTC(uint32* dtc, Dem_EventStatusExtendedType* dtcStatus); /** @req DEM215 */
Dem_ReturnGetNextFilteredDTCType Dem_GetNextFilteredRecord(uint32 *DTC, uint8 *RecordNumber); /* @req DEM224 */
Dem_DTCTranslationFormatType Dem_GetTranslationType(void); /** @req DEM230 */
Dem_ReturnGetSeverityOfDTCType Dem_GetSeverityOfDTC(uint32 DTC, Dem_DTCSeverityType* DTCSeverity);/** @req DEM232 */

/* Access extended data records and FreezeFrame data */
Dem_ReturnDisableDTCRecordUpdateType Dem_DisableDTCRecordUpdate(uint32 DTC, Dem_DTCOriginType DTCOrigin);/* @req DEM233 */
Std_ReturnType Dem_EnableDTCRecordUpdate(void);/* @req DEM234 */
Dem_ReturnGetFreezeFrameDataByDTCType Dem_GetFreezeFrameDataByDTC(uint32 dtc, Dem_DTCOriginType dtcOrigin,uint8 recordNumber, uint8* destBuffer, uint16* bufSize);/** @req DEM236 */
Dem_ReturnGetSizeOfFreezeFrameType Dem_GetSizeOfFreezeFrameByDTC(uint32 dtc, Dem_DTCOriginType dtcOrigin, uint8 recordNumber, uint16* sizeOfFreezeFrame);/** @req DEM238 */
Dem_ReturnGetExtendedDataRecordByDTCType Dem_GetExtendedDataRecordByDTC(uint32 dtc, Dem_DTCOriginType dtcOrigin, uint8 extendedDataNumber, uint8 *destBuffer, uint16 *bufSize); /** @req DEM239 */
Dem_ReturnGetSizeOfExtendedDataRecordByDTCType Dem_GetSizeOfExtendedDataRecordByDTC(uint32 dtc, Dem_DTCOriginType dtcOrigin, uint8 extendedDataNumber, uint16 *sizeOfExtendedDataRecord); /** @req DEM240 */

/* DTC storage */
/** @req DEM241 Deviation from specification. Prototype exists regardless if RTE is used or not. The specification states
                that it should be prototyped by the RTE but this causes a compilation warning since Dem.h is included 
                from the Dcm */
Dem_ReturnClearDTCType Dem_ClearDTC(uint32 dtc, Dem_DTCFormatType dtcFormat, Dem_DTCOriginType dtcOrigin); 


Dem_ReturnControlDTCStorageType Dem_DisableDTCSetting(Dem_DTCGroupType dtcGroup, Dem_DTCKindType dtcKind); /** @req DEM242 */
Dem_ReturnControlDTCStorageType Dem_EnableDTCSetting(Dem_DTCGroupType dtcGroup, Dem_DTCKindType dtcKind); /** @req DEM243 */

/*
 * OBD-specific Interfaces
 */
Std_ReturnType Dem_ReadDataOfOBDFreezeFrame(uint8 PID, uint8 DataElementIndexOfPid, uint8* DestBuffer, uint8* BufSize);/* @req DEM327 */
Std_ReturnType Dem_GetDTCOfOBDFreezeFrame(uint8 FrameNumber, uint32* DTC );/* @req DEM624 */
#if (DEM_OBD_SUPPORT == STD_ON)
Std_ReturnType Dem_SetEventDisabled(Dem_EventIdType EventId);
Std_ReturnType Dem_DcmReadDataOfPID01(uint8* PID01value);
Std_ReturnType Dem_DcmReadDataOfPID41(uint8* PID41value);
Std_ReturnType Dem_RepIUMPRDenLock(Dem_RatioIdType RatioID);
Std_ReturnType Dem_RepIUMPRDenRelease(Dem_RatioIdType RatioID);
Std_ReturnType Dem_RepIUMPRFaultDetect(Dem_RatioIdType RatioID);
Std_ReturnType Dem_SetIUMPRDenCondition(Dem_IumprDenomCondIdType ConditionId, Dem_IumprDenomCondStatusType ConditionStatus);
Std_ReturnType Dem_GetIUMPRDenCondition(Dem_IumprDenomCondIdType ConditionId, Dem_IumprDenomCondStatusType* ConditionStatus);
#if (DEM_OBD_ENGINE_TYPE == DEM_IGNITION_SPARK) /* DEM357 */
Std_ReturnType Dem_GetInfoTypeValue08(uint8* Iumprdata08);
#elif (DEM_OBD_ENGINE_TYPE == DEM_IGNITION_COMPR) /* DEM358 */
Std_ReturnType Dem_GetInfoTypeValue0B(uint8* Iumprdata0B);
#endif
#endif
/*
 * Interface DLT <-> DEM
 */
#if (DEM_TRIGGER_DLT_REPORTS == STD_ON)
Std_ReturnType Dem_DltGetAllExtendedDataRecords(Dem_EventIdType EventId, uint8* DestBuffer, uint8* BufSize); /** @req DEM637 */
Std_ReturnType Dem_DltGetMostRecentFreezeFrameRecordData(Dem_EventIdType EventId, uint8* DestBuffer, uint8* BufSize);/** @req DEM636 */
#endif

Std_ReturnType Dem_GetMonitorStatus(Dem_EventIdType EventID, Dem_MonitorStatusType* MonitorStatus);/** @req 4.3.0/SWS_Dem_91007 */

#endif /*DEM_H_*/
