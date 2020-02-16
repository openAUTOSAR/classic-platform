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

#include "Modules.h"

#define DEM_MODULE_ID			MODULE_ID_DEM
#define DEM_VENDOR_ID			VENDOR_ID_ARCCORE

#define DEM_AR_RELEASE_MAJOR_VERSION   		4
#define DEM_AR_RELEASE_MINOR_VERSION   		0
#define DEM_AR_RELEASE_REVISION_VERSION   	3

#define DEM_SW_MAJOR_VERSION    3
#define DEM_SW_MINOR_VERSION   	0
#define DEM_SW_PATCH_VERSION    0
#define DEM_AR_MAJOR_VERSION    DEM_AR_RELEASE_MAJOR_VERSION
#define DEM_AR_MINOR_VERSION    DEM_AR_RELEASE_MINOR_VERSION
#define DEM_AR_PATCH_VERSION    DEM_AR_RELEASE_REVISION_VERSION

/** @req DCM067.Cfg-files */
#include "Dem_Types.h"
#include "Dem_Cfg.h"
#include "Dem_Lcfg.h"
#include "Dem_IntErrId.h" /** @req DEM115 */
#include "Dem_IntEvtId.h"
#include "Dem_EnableCondId.h"
#include <limits.h>
/** @req DEM153 */ /** @req DEM154 */ /* Realized in Dem_IntErrId.h and Dem_IntEvtId.h */
// #include "Rte_Dem.h"


#if (DEM_DEV_ERROR_DETECT == STD_ON)
// Error codes reported by this module defined by AUTOSAR /** @req DEM116 */ /** @req DEM173 */
#define DEM_E_PARAM_CONFIG					0x10
#define DEM_E_PARAM_POINTER					0x11
#define DEM_E_PARAM_DATA					0x12
#define DEM_E_PARAM_LENGTH					0x13
#define DEM_E_UNINIT						0x20
#define DEM_E_NODATAAVAILABLE				0x30

// Other error codes reported by this module
#define DEM_E_CONFIG_PTR_INVALID			0x40
#define DEM_E_EVENT_STATUS_BUFF_FULL		0x41
#define DEM_E_EXT_DATA_TOO_BIG				0x42
#define DEM_E_PRE_INIT_EXT_DATA_BUFF_FULL	0x43
#define DEM_E_MEM_EVENT_BUFF_FULL		    0x44
#define DEM_E_MEM_EXT_DATA_BUFF_FULL	    0x45

#define DEM_E_FF_TOO_BIG				0x46
#define DEM_E_PRE_INIT_FF_DATA_BUFF_FULL	0x47
#define DEM_E_MEM_FF_DATA_BUFF_FULL	        0x48
#define DEM_E_OBD_NOT_ALLOWED_IN_SEC_MEM    0x49




#define DEM_E_UNEXPECTED_EXECUTION			0xfe
#define DEM_E_NOT_IMPLEMENTED_YET			0xff


// Service ID in this module
#define DEM_PREINIT_ID							0x01
#define DEM_INIT_ID								0x02
#define DEM_SHUTDOWN_ID							0x03
#define DEM_SETEVENTSTATUS_ID					0x04
#define DEM_RESETEVENTSTATUS_ID					0x05
#define DEM_SETOPERATIONCYCLESTATE_ID			0x08
#define DEM_GETEVENTSTATUS_ID					0x0A
#define DEM_GETEVENTFAILED_ID					0x0B
#define DEM_GETEVENTTESTED_ID					0x0C
#define DEM_GETDTCOFEVENT_ID					0x0D
#define DEM_REPORTERRORSTATUS_ID 				0x0F
#define DEM_SETDTCFILTER_ID						0x13
#define DEM_GETSTATUSOFDTC_ID					0x15
#define DEM_GETDTCSTATUSAVAILABILITYMASK_ID		0x16
#define DEM_GETNUMBEROFFILTEREDDTC_ID			0x17
#define DEM_GETNEXTFILTEREDDTC_ID				0x18
#define DEM_GETEXTENDEDDATARECORDBYDTC_ID		0x20
#define DEM_GETSIZEOFEXTENDEDDATARECORDBYDTC_ID	0x21
#define DEM_CLEARDTC_ID							0x22
#define DEM_DISABLEDTCSTORAGE_ID				0x24
#define DEM_ENABLEDTCSTORAGE_ID					0x25

#define DEM_GETFREEZEFRAMEDATARECORDBYDTC_ID		0x26
#define DEM_GETFREEZEFRAMEDATAIDENTIFIERBYDTC_ID		0x27

#define DEM_GETDTCOFOBDFREEZEFRAME_ID			0x28
#define DEM_GETOBDFREEZEFRAMEDATA_ID			0x29


#define DEM_GETNEXTFILTEREDRECORD_ID            0x3a
#define DEM_GETOBDFREEZEFRAMEDATA_ID			0x29

#define DEM_GETTRANSLATIONTYPE_ID				0x3c
#define DEM_SETFREEZEFRAMERECORDFILTER_ID       0x37
#define DEM_SETENABLECONDITION_ID               0x39
#define DEM_GETFAULTDETECTIONCOUNTER_ID 		0x3E
#define DEM_MAINFUNCTION_ID						0x55

#define DEM_UPDATE_EVENT_STATUS_ID				0x80
#define DEM_MERGE_EVENT_STATUS_ID				0x81
#define DEM_GET_EXTENDED_DATA_ID				0x82
#define DEM_STORE_EXT_DATA_PRE_INIT_ID			0x83
#define DEM_STORE_EVENT_MEM_ID				    0x84
#define DEM_STORE_EXT_DATA_MEM_ID			    0x85
#define DEM_PREDEBOUNCE_NONE_ID					0x86
#define DEM_PREDEBOUNCE_COUNTER_BASED_ID		0x87
#define DEM_GET_FREEZEFRAME_ID					0x88
#define DEM_STORE_FF_DATA_PRE_INIT_ID			0x89
#define DEM_STORE_FF_DATA_MEM_ID			    0x90

#define DEM_DSP_DID_USE_PORT_IS_TRUE            0x91
#define DEM_READ_DATA_LENGTH_FAILED				0x92

#define DEM_GLOBAL_ID							0xff

#endif

#define DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT     ULONG_MAX //when timestamp up to the max value,rearrangement starts.
#define DEM_MAX_TIMESTAMP_FOR_PRE_INIT          (uint32)(ULONG_MAX/2)
/*
 * Interface for upper layer modules (8.3.1)
 */
#if ( DEM_VERSION_INFO_API == STD_ON ) /** @req DEM111 */
#define Dem_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,DEM) /** @req DEM177 */ /** @req DEM110 */ /** @req DEM178 */
#endif /* DEM_VERSION_INFO_API */

/*
 * Interface ECU State Manager <-> DEM (8.3.2)
 */
void Dem_PreInit( const Dem_ConfigType *ConfigPtr ); /** @req DEM179 */
void Dem_Init( void ); /** @req DEM181 */
void Dem_Shutdown( void ); /** @req DEM182 */


/*
 * Interface for basic software scheduler
 */
void Dem_MainFunction( void ); /** @req DEM266 */


/*
 * Interface SW-Components via RTE <-> DEM (8.3.3)
 */
Std_ReturnType Dem_GetEventStatus(Dem_EventIdType eventId, Dem_EventStatusExtendedType *eventStatusExtended); /** @req DEM195 */
Std_ReturnType Dem_GetEventFailed(Dem_EventIdType eventId, boolean *eventFailed); /** @req DEM196 */
Std_ReturnType Dem_GetEventTested(Dem_EventIdType eventId, boolean *eventTested); /** @req DEM197 */
Std_ReturnType Dem_GetFaultDetectionCounter(Dem_EventIdType eventId, sint8 *counter); /** @req DEM203 */
Std_ReturnType Dem_SetOperationCycleState(Dem_OperationCycleIdType operationCycleId, Dem_OperationCycleStateType cycleState); /** @req DEM194 */
Std_ReturnType Dem_GetDTCOfEvent(Dem_EventIdType eventId, Dem_DTCFormatType dtcFormat, uint32* dtcOfEvent); /** @req DEM198 */
#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
Std_ReturnType Dem_SetEnableCondition(uint8 EnableConditionID, boolean ConditionFulfilled); /** @req DEM201*/
#endif


/*
 * Interface BSW-Components <-> DEM (8.3.4)
 */
void Dem_ReportErrorStatus(Dem_EventIdType eventId ,Dem_EventStatusType eventStatus); /** @req DEM206 */


/*
 * Interface DCM <-> DEM (8.3.5)
 */
/* Access DTCs and status information */
Dem_ReturnSetFilterType Dem_SetDTCFilter(uint8 dtcStatusMask, Dem_DTCKindType dtcKind, Dem_DTCFormatType dtcFormat, Dem_DTCOriginType dtcOrigin, Dem_FilterWithSeverityType filterWithSeverity, Dem_DTCSeverityType dtcSeverityMask, Dem_FilterForFDCType filterForFaultDetectionCounter);  /** @req DEM208 */
Dem_ReturnGetStatusOfDTCType Dem_GetStatusOfDTC(uint32 dtc, Dem_DTCOriginType dtcOrigin, Dem_EventStatusExtendedType* status); /** @req DEM212 */
Std_ReturnType Dem_GetDTCStatusAvailabilityMask(uint8 *dtcStatusMask); /** @req DEM213 */
Dem_ReturnGetNumberOfFilteredDTCType Dem_GetNumberOfFilteredDtc(uint16* numberOfFilteredDTC); /** @req DEM214 */
Dem_ReturnGetNextFilteredDTCType Dem_GetNextFilteredDTC(uint32* dtc, Dem_EventStatusExtendedType* dtcStatus); /** @req DEM215 */
Dem_DTCTranslationFormatType Dem_GetTranslationType(void); /** @req DEM230 */

/* Access extended data records and FreezeFrame data */
//Dem_GetFreezeFrameByRecord(uint8 recordNumber, Dem_DTCOriginType dtcOrigin, uint32* dtc, uint8* destBuffer, uint16*bufSize);
Dem_ReturnGetFreezeFrameDataByDTCType Dem_GetFreezeFrameDataByDTC(uint32 dtc, Dem_DTCOriginType dtcOrigin,uint8 recordNumber, uint8* destBuffer, uint16* bufSize);/** @req DEM236 */
Dem_ReturnGetSizeOfFreezeFrameType Dem_GetSizeOfFreezeFrameByDTC(uint32 dtc, Dem_DTCOriginType dtcOrigin, uint8 recordNumber, uint16* sizeOfFreezeFrame);/** @req DEM238 */
Dem_ReturnGetExtendedDataRecordByDTCType Dem_GetExtendedDataRecordByDTC(uint32 dtc, Dem_DTCOriginType dtcOrigin, uint8 extendedDataNumber, uint8 *destBuffer, uint16 *bufSize); /** @req DEM239 */
Dem_ReturnGetSizeOfExtendedDataRecordByDTCType Dem_GetSizeOfExtendedDataRecordByDTC(uint32 dtc, Dem_DTCOriginType dtcOrigin, uint8 extendedDataNumber, uint16 *sizeOfExtendedDataRecord); /** @req DEM240 */
Dem_ReturnSetFilterType Dem_SetFreezeFrameRecordFilter(Dem_DTCFormatType dtcFormat, uint16 *NumberOfFilteredRecords);/* @req DEM209 */
Dem_ReturnGetNextFilteredDTCType Dem_GetNextFilteredRecord(uint32 *DTC, uint8 *RecordNumber); /* @req DEM224 */

/* DTC storage */
Dem_ReturnClearDTCType Dem_ClearDTC(uint32 dtc, Dem_DTCFormatType dtcFormat, Dem_DTCOriginType dtcOrigin); /** @req DEM241 */
Dem_ReturnControlDTCStorageType Dem_DisableDTCSetting(Dem_DTCGroupType dtcGroup, Dem_DTCKindType dtcKind); /** @req DEM242 */
Dem_ReturnControlDTCStorageType Dem_EnableDTCSetting(Dem_DTCGroupType dtcGroup, Dem_DTCKindType dtcKind); /** @req DEM243 */



/*
 * OBD-specific Interfaces (8.3.6)
 */
Std_ReturnType Dem_GetDTCOfOBDFreezeFrame(uint8 FrameNumber, uint32* DTC );
Std_ReturnType Dem_GetOBDFreezeFrameData(uint8 PID, uint8 DataElementIndexOfPid, uint8* DestBuffer, uint8* BufSize);

#endif /*DEM_H_*/
