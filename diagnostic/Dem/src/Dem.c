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
/*lint -w2 */

/* INPROVEMENT: This note should be enabled */
/*lint -e9025 */

/*
 *  General requirements
 */
/* @req DEM126 */
/* !req DEM151 Partially */
/* @req DEM152 */
/* !req DEM013 Only 14229-1 format supported */
/* @req DEM645 Both UDS and OBD format supported */
/* @req DEM277 Both UDS and OBD format supported */
/* @req DEM113 */
/* @req DEM267 */
/* !req DEM268 */
/* @req DEM364 */
/* @req DEM114 */
/* @req DEM124 */
/* @req DEM370 */
/* @req DEM386 UDS status bit 0 transitions */
/* @req DEM389 UDS status bit 1 transitions */
/* @req DEM390 UDS status bit 2 transitions */
/* @req DEM391 UDS status bit 3 transitions */
/* @req DEM392 UDS status bit 4 transitions */
/* @req DEM393 UDS status bit 5 transitions */
/* @req DEM394 UDS status bit 6 transitions */
/* @req DEM395 UDS status bit 7 transitions */
/* @req 4.2.2/SWS_Dem_01102 DTC suppression shall not stop event processing of the corresponding DTC. */
/* @req DEM551 *//* Block write triggered if configured*/
/* @req DEM552 *//* Block write not triggered if occurance has reached ImmediateNvStorageLimit */
/* @req DEM536 *//* Event combination type 1 and 2 supported */

#include <string.h>
#include "Dem.h"

#if defined(USE_NVM)
#include "NvM.h" /** @req DEM176.NvM */
#endif

#include "SchM_Dem.h"
#include "MemMap.h"
#include "Cpu.h"
#include "Dem_Types.h"
#include "Dem_Lcfg.h"
#include "Dem_Internal.h"
#if defined(USE_DEM_EXTENSION)
#include "Dem_Extension.h"
#endif
#define USE_DEBUG_PRINTF
#include "debug.h"

#if defined(USE_RTE)
/*lint -e18 duplicate declarations hidden behinde ifdef  */
#include "Rte_Dem.h"
#endif

#if (DEM_TRIGGER_DLT_REPORTS == STD_ON)
#include "Dlt.h"
#endif
#if defined(USE_FIM)
#include "FiM.h"
#endif
#include "Dem_NvM.h"
/*
 * Local defines
 */
#define DEM_EXT_DATA_IN_PRE_INIT (DEM_MAX_NUMBER_EXT_DATA_PRE_INIT > 0)
#define DEM_EXT_DATA_IN_PRI_MEM ((DEM_MAX_NUMBER_EXT_DATA_PRI_MEM > 0) && ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)))
#define DEM_EXT_DATA_IN_SEC_MEM ((DEM_MAX_NUMBER_EXT_DATA_SEC_MEM > 0) && (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON))
#define DEM_FF_DATA_IN_PRE_INIT (DEM_MAX_NUMBER_FF_DATA_PRE_INIT > 0)
#define DEM_FF_DATA_IN_PRI_MEM ((DEM_MAX_NUMBER_FF_DATA_PRI_MEM > 0) && ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)))
#define DEM_FF_DATA_IN_SEC_MEM ((DEM_MAX_NUMBER_FF_DATA_SEC_MEM > 0) && (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON))
#define DEM_DEFAULT_EVENT_STATUS (DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR | DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE)
#define DEM_PRESTORAGE_FF_DATA_IN_MEM (DEM_MAX_NUMBER_PRESTORED_FF > 0)

#define DEM_PID_IDENTIFIER_SIZE_OF_BYTES        1 // OBD
#define DEM_FAILURE_CNTR_MAX 255
#define DEM_AGING_CNTR_MAX 255
#define DEM_INDICATOR_CNTR_MAX 255
#define DEM_OCCURENCE_COUNTER_MAX 0xFFFF

#define MOST_RECENT_FF_RECORD 0xFF

#define FREE_PERMANENT_DTC_ENTRY 0u

#define ALL_EXTENDED_DATA_RECORDS 0xFF
#define IS_VALID_EXT_DATA_RECORD(_x)    ((0x01 <= (_x)) && (0xEF >= (_x)))

#define IS_VALID_EVENT_ID(_x)   (((_x) > 0) && ((_x) <= DEM_EVENT_ID_LAST_VALID_ID))

#define IS_VALID_INDICATOR_ID(_x)   ((_x) < DEM_NOF_INDICATORS)

#define IS_SUPPORTED_ORIGIN(_x) ((DEM_DTC_ORIGIN_PRIMARY_MEMORY == (_x)) || (DEM_DTC_ORIGIN_SECONDARY_MEMORY == (_x)))

#if (DEM_STORE_UDS_STATUS_BIT_SUBSET_FOR_ALL_EVENTS == STD_ON)
#define NUM_STORED_BITS 2u
#define NOF_EVENTS_PER_BYTE (8u/NUM_STORED_BITS)
#define GET_UDSBIT_BYTE_INDEX(_eventId) (((_eventId)-1u)/NOF_EVENTS_PER_BYTE)
#define GET_UDS_STARTBIT(_eventId) ((((_eventId)-1u)%NOF_EVENTS_PER_BYTE)*2u)
#define UDS_BITMASK 3u
#define UDS_TFSLC_BIT 0u
#define UDS_TNCSLC_BIT 1u
#define UDS_STATUS_BIT_MAGIC UDS_BITMASK
#define UDS_STATUS_BIT_MAGIC_INDEX ((DEM_MAX_NUMBER_EVENT + (NOF_EVENTS_PER_BYTE - 1u))/NOF_EVENTS_PER_BYTE)
#endif

#define DEM_REC_NUM_AND_NUM_DIDS_SIZE 2u

#define VALIDATE_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          DET_REPORTERROR(DEM_MODULE_ID, 0, _api, _err); \
          return _rv; \
        }

#define VALIDATE_NO_RV(_exp,_api,_err ) \
  if( !(_exp) ) { \
          DET_REPORTERROR(DEM_MODULE_ID, 0, _api, _err); \
          return; \
        }


#if (DEM_PTO_SUPPORT == STD_ON)
#error "DEM_PTO_SUPPORT is set to STD_ON, this is not supported by the code."
#endif

#if !((DEM_TYPE_OF_DTC_SUPPORTED == DEM_DTC_TRANSLATION_ISO15031_6) || (DEM_TYPE_OF_DTC_SUPPORTED == DEM_DTC_TRANSLATION_ISO14229_1))
#error "DEM_TYPE_OF_DTC_SUPPORTED is not set to ISO15031-6 or ISO14229-1. Only these are supported by the code."
#endif

#if !defined(USE_DEM_EXTENSION)
#if defined(DEM_FREEZE_FRAME_CAPTURE_EXTENSION)
#error "DEM_FREEZE_FRAME_CAPTURE cannot be DEM_TRIGGER_EXTENSION since Dem extension is not used!"
#endif
#if defined(DEM_EXTENDED_DATA_CAPTURE_EXTENSION)
#error "DEM_EXTENDED_DATA_CAPTURE cannot be DEM_TRIGGER_EXTENSION since Dem extension is not used!"
#endif
#if defined(DEM_DISPLACEMENT_PROCESSING_DEM_EXTENSION)
#error "DEM_DISPLACEMENT_PROCESSING_DEM_EXTENSION cannot be used since Dem extension is not used!"
#endif
#if defined(DEM_FAILURE_PROCESSING_DEM_EXTENSION)
#error "DEM_FAILURE_PROCESSING_DEM_EXTENSION cannot be used since Dem extension is not used!"
#endif
#if defined(DEM_AGING_PROCESSING_DEM_EXTENSION)
#error "DEM_AGING_PROCESSING_DEM_EXTENSION cannot be used since Dem extension is not used!"
#endif
#endif
#if defined(DEM_EXTENDED_DATA_CAPTURE_EVENT_MEMORY_STORAGE)
#error "DEM_EXTENDED_DATA_CAPTURE_EVENT_MEMORY_STORAGE is not supported!"
#endif

#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) || (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON) || (DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON)
#define DEM_USE_MEMORY_FUNCTIONS
#endif
#if defined(USE_NVM) && (DEM_USE_NVM == STD_ON)
#define DEM_ASSERT(_exp)        switch (1) {case 0: break; case (_exp): break; }
#endif

#if (DEM_TEST_FAILED_STORAGE == STD_ON)
#define GET_STORED_STATUS_BITS(_x) ((_x) & (DEM_TEST_FAILED_SINCE_LAST_CLEAR | DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR | DEM_PENDING_DTC | DEM_CONFIRMED_DTC | DEM_TEST_FAILED))
#else
#define GET_STORED_STATUS_BITS(_x) ((_x) & (DEM_TEST_FAILED_SINCE_LAST_CLEAR | DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR | DEM_PENDING_DTC | DEM_CONFIRMED_DTC))

#endif

#define IS_VALID_EVENT_STATUS(_x) ((DEM_EVENT_STATUS_PREPASSED == _x) || (DEM_EVENT_STATUS_PASSED == _x) || (DEM_EVENT_STATUS_PREFAILED == _x) || (DEM_EVENT_STATUS_FAILED == _x))

#if (DEM_NOF_EVENT_INDICATORS > 0)
#define DEM_USE_INDICATORS
#endif

#define TO_OBD_FORMAT(_x) ((_x)<<8u)
#define IS_VALID_DTC_FORMAT(_x) ((DEM_DTC_FORMAT_UDS == (_x)) || (DEM_DTC_FORMAT_OBD == (_x)))

#if (DEM_IMMEDIATE_NV_STORAGE == STD_ON)
#define DEM_USE_IMMEDIATE_NV_STORAGE
#endif

#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
#define TO_COMBINED_EVENT_ID(_x) ((_x) | 0x8000u)
#define IS_COMBINED_EVENT_ID(_x) (0u != ((_x) & 0x8000u))
#define TO_COMBINED_EVENT_CFG_IDX(_x) ((_x) & (uint16)(~0x8000u))
#define IS_VALID_COMBINED_ID(_x) ((_x) < DEM_NOF_COMBINED_DTCS)
/* Clear all combined event aging counter when sub event failes */
#define DEM_CLEAR_COMBINED_AGING_COUNTERS_ON_FAIL
#if 0
/* Require all sub events tested and passed to process aging */
#define DEM_COMBINED_DTC_STATUS_AGING
#endif
#endif
/*
 * Local types
 */

// DtcFilterType
typedef struct {
    Dem_EventStatusExtendedType dtcStatusMask;
    Dem_DTCKindType             dtcKind;
    Dem_DTCOriginType           dtcOrigin;
    Dem_FilterWithSeverityType  filterWithSeverity;
    Dem_DTCSeverityType         dtcSeverityMask;
    Dem_FilterForFDCType        filterForFaultDetectionCounter;
    uint16                      DTCIndex;
    Dem_DTCFormatType           dtcFormat;
} DtcFilterType;

// FreezeFrameRecordFilterType
typedef struct {
    uint16                      ffIndex;
    Dem_DTCFormatType           dtcFormat;
} FreezeFrameRecordFilterType;

// DisableDtcStorageType
typedef struct {
    boolean                     settingDisabled;
    Dem_DTCGroupType            dtcGroup;
    Dem_DTCKindType             dtcKind;
} DisableDtcSettingType;


// State variable
typedef enum
{
  DEM_UNINITIALIZED = 0,
  DEM_PREINITIALIZED,
  DEM_INITIALIZED,
  DEM_SHUTDOWN
} Dem_StateType; /** @req DEM169 */

static Dem_StateType demState = DEM_UNINITIALIZED;
#if defined(USE_FIM)
static boolean DemFiMInit = FALSE;
#endif

// Help pointer to configuration set
static const Dem_ConfigSetType *configSet;

/*
 * Allocation of DTC filter parameters
 */
static DtcFilterType dtcFilter;

/*
 * Allocation of freeze frame record filter
 */
static FreezeFrameRecordFilterType ffRecordFilter;


/*
 * Allocation of Disable/Enable DTC setting parameters
 */
static DisableDtcSettingType disableDtcSetting;

/*
 * Allocation of operation cycle state list
 */
/* NOTE: Do not change this without also changing generation of measurement tags */
static Dem_OperationCycleStateType operationCycleStateList[DEM_OPERATION_CYCLE_ID_ENDMARK];

/*
 * Allocation of local event status buffer
 */
/* NOTE: Do not change this without also changing generation of measurement tags */
static EventStatusRecType   eventStatusBuffer[DEM_MAX_NUMBER_EVENT];

/*
 * Allocation of pre-init event memory (used between pre-init and init). Only one
 * memory regardless of event destination.
 */
#if ( DEM_FF_DATA_IN_PRE_INIT )
static FreezeFrameRecType   preInitFreezeFrameBuffer[DEM_MAX_NUMBER_FF_DATA_PRE_INIT];
#endif
#if ( DEM_EXT_DATA_IN_PRE_INIT )
static ExtDataRecType       preInitExtDataBuffer[DEM_MAX_NUMBER_EXT_DATA_PRE_INIT];
#endif

/*
 * Allocation of primary event memory ramlog (after init) in uninitialized memory
 */
#define ADMIN_MAGIC 0xBABE
/** @req DEM162 */
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
#define PRI_MEM_EVENT_BUFFER_ADMIN_INDEX DEM_MAX_NUMBER_EVENT_PRI_MEM
EventRecType                priMemEventBuffer[DEM_MAX_NUMBER_EVENT_PRI_MEM + 1];/* + 1 for admin data */
static boolean              priMemOverflow = FALSE;/* @req DEM397 */
#if ( DEM_FF_DATA_IN_PRI_MEM )
FreezeFrameRecType          priMemFreezeFrameBuffer[DEM_MAX_NUMBER_FF_DATA_PRI_MEM];
#endif
#if (DEM_EXT_DATA_IN_PRI_MEM)
ExtDataRecType              priMemExtDataBuffer[DEM_MAX_NUMBER_EXT_DATA_PRI_MEM];
#endif
#endif
/*
 * Allocation of secondary event memory ramlog (after init) in uninitialized memory
 */
/** @req DEM162 */
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
#define SEC_MEM_EVENT_BUFFER_ADMIN_INDEX DEM_MAX_NUMBER_EVENT_SEC_MEM
EventRecType                secMemEventBuffer[DEM_MAX_NUMBER_EVENT_SEC_MEM + 1];/* + 1 for admin data */
static boolean              secMemOverflow = FALSE;/* @req DEM397 */
#if (DEM_FF_DATA_IN_SEC_MEM)
FreezeFrameRecType          secMemFreezeFrameBuffer[DEM_MAX_NUMBER_FF_DATA_SEC_MEM];
#endif
#if (DEM_EXT_DATA_IN_SEC_MEM)
ExtDataRecType              secMemExtDataBuffer[DEM_MAX_NUMBER_EXT_DATA_SEC_MEM];
#endif
#endif

/*
 * Allocation of permanent event memory ramlog (after init) in uninitialized memory
 */
/** @req DEM162 */
#if (DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON)
#define PERM_MEM_EVENT_BUFFER_ADMIN_INDEX DEM_MAX_NUMBER_EVENT_PERM_MEM
PermanentDTCType            permMemEventBuffer[DEM_MAX_NUMBER_EVENT_PERM_MEM]; /* Free entry == 0u */
#endif

/*
 * Allocation of event memory for pre storage of freeze frames
 */
/** @req DEM191 */
#if ( DEM_PRESTORAGE_FF_DATA_IN_MEM )
FreezeFrameRecType          memPreStoreFreezeFrameBuffer[DEM_MAX_NUMBER_PRESTORED_FF];
#endif

#if defined(DEM_USE_MEMORY_FUNCTIONS) && (DEM_STORE_UDS_STATUS_BIT_SUBSET_FOR_ALL_EVENTS == STD_ON)
/* Buffer for storing subset of UDS status bits for all events */
uint8 statusBitSubsetBuffer[DEM_MEM_STATUSBIT_BUFFER_SIZE];
#endif

#if (DEM_USE_TIMESTAMPS == STD_ON)
/* Timestamp for events */
static uint32 Event_TimeStamp = 0;

/* Timestamp for extended data */
static uint32 ExtData_TimeStamp = 0;

/*
*Allocation of freezeFrame storage timestamp,record the time order
*/
/**private variable for freezeframe */
static uint32 FF_TimeStamp = 0;
#endif

#if (DEM_DTC_SUPPRESSION_SUPPORT == STD_ON)
typedef struct {
    boolean SuppressedByDTC:1;/*lint !e46 *//*structure must remain the same,field type should be _Bool, unsigned int or signed int [MISRA 2004 6.4, 2012 6.1]*/
    boolean SuppressedByEvent:1;/*lint !e46 *//*structure must remain the same,field type should be _Bool, unsigned int or signed int [MISRA 2004 6.4, 2012 6.1]*/
}DemDTCSuppressionType;
static DemDTCSuppressionType DemDTCSuppressed[DEM_NOF_DTCS];
#endif

#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
static boolean DemEnableConditions[DEM_NUM_ENABLECONDITIONS];
#endif

#define NO_DTC_DISABLED 0xFFFFFFFFUL

typedef struct {
    uint32 DTC;
    Dem_DTCOriginType Origin;
}DtcRecordUpdateDisableType;

static DtcRecordUpdateDisableType DTCRecordDisabled;

#if defined(DEM_USE_INDICATORS)
/* @req DEM499 */
#define INDICATOR_FAILED_DURING_FAILURE_CYCLE   1u
#define INDICATOR_PASSED_DURING_FAILURE_CYCLE   (1u<<1u)
#define INDICATOR_FAILED_DURING_HEALING_CYCLE   (1u<<2u)
#define INDICATOR_PASSED_DURING_HEALING_CYCLE   (1u<<3u)

typedef struct {
    Dem_EventIdType EventID;
    uint16 InternalIndicatorId;
    uint8 FailureCounter;
    uint8 HealingCounter;
    uint8 OpCycleStatus;
}IndicatorStatusType;

typedef struct {
    Dem_EventIdType EventID;
    uint8 IndicatorId;
    uint8 FailureCounter;
    uint8 HealingCounter;
}IndicatorNvRecType;

/* Buffer for storing event indicators internally */
static IndicatorStatusType indicatorStatusBuffer[DEM_NOF_EVENT_INDICATORS];

#if defined(DEM_USE_MEMORY_FUNCTIONS)
/* Buffer for storing event indicator status in NvRam */
IndicatorNvRecType indicatorBuffer[DEM_NOF_EVENT_INDICATORS];
#endif
#endif

#if defined(DEM_USE_IUMPR)
// Each index corresponds to RatioID
Dem_RatiosNvm iumprBuffer;
static Dem_RatioStatusType iumprBufferLocal[DEM_IUMPR_REGISTERED_COUNT];

// IUMPR general denominator buffer
Dem_RatioGeneralDenominatorType generalDenominatorBuffer;

// IUMPT ignition cycle count buffer
uint16 ignitionCycleCountBuffer;

// IUMPR additional denominator conditions buffer
#define DEM_IUMPR_ADDITIONAL_DENOMINATORS_COUNT 4
static Dem_IumprDenomCondType iumprAddiDenomCondBuffer[DEM_IUMPR_ADDITIONAL_DENOMINATORS_COUNT];
#endif

/*
 * Local functions
 * */

#ifdef DEM_USE_MEMORY_FUNCTIONS
#if (DEM_STORE_UDS_STATUS_BIT_SUBSET_FOR_ALL_EVENTS == STD_ON)
static void SetDefaultUDSStatusBitSubset(void);
#endif

#if ( DEM_FF_DATA_IN_PRE_INIT || DEM_FF_DATA_IN_PRI_MEM )
static boolean storeOBDFreezeFrameDataMem(const Dem_EventParameterType *eventParam, const FreezeFrameRecType *freezeFrame,
                                          FreezeFrameRecType* freezeFrameBuffer, uint32 freezeFrameBufferSize,
                                          Dem_DTCOriginType origin);
#endif

#if ( DEM_FF_DATA_IN_PRE_INIT || DEM_FF_DATA_IN_PRI_MEM || DEM_FF_DATA_IN_SEC_MEM )
static boolean storeFreezeFrameDataMem(const Dem_EventParameterType *eventParam, const FreezeFrameRecType *freezeFrame,
                                    FreezeFrameRecType* freezeFrameBuffer, uint32 freezeFrameBufferSize,
                                    Dem_DTCOriginType origin);
#endif

static boolean deleteFreezeFrameDataMem(const Dem_EventParameterType *eventParam, Dem_DTCOriginType origin, boolean combinedDTC);
static boolean deleteExtendedDataMem(const Dem_EventParameterType *eventParam, Dem_DTCOriginType origin, boolean combinedDTC);
#endif

static Std_ReturnType getEventFailed(Dem_EventIdType eventId, boolean *eventFailed);

#if (DEM_USE_TIMESTAMPS == STD_ON) && defined(DEM_USE_MEMORY_FUNCTIONS)
#if (DEM_UNIT_TEST == STD_ON)
void rearrangeEventTimeStamp(uint32 *timeStamp);
#else
static void rearrangeEventTimeStamp(uint32 *timeStamp);
#endif
#endif

#if defined(DEM_USE_INDICATORS) && defined(DEM_USE_MEMORY_FUNCTIONS)
static void storeEventIndicators(const Dem_EventParameterType *eventParam);
#endif

static Std_ReturnType getEventStatus(Dem_EventIdType eventId, Dem_EventStatusExtendedType *eventStatusExtended);

static void getFFClassReference(const Dem_EventParameterType *eventParam, Dem_FreezeFrameClassType **ffClassTypeRef);
static Dem_FreezeFrameClassTypeRefIndex getFFIdx(const Dem_EventParameterType *eventParam);

#if defined(DEM_AGING_PROCESSING_DEM_INTERNAL)
uint8 getEventAgingCntr(const Dem_EventParameterType *eventParameter);
#endif

#if (DEM_UNIT_TEST == STD_ON)
/*
 * Procedure:   zeroPriMemBuffers
 * Description: Fill the primary buffers with zeroes
 */
/*lint -efunc(714, demZeroPriMemBuffers) */
void demZeroPriMemBuffers(void)
{
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
    memset(priMemEventBuffer, 0, sizeof(priMemEventBuffer));
    priMemOverflow = FALSE;
#if (DEM_FF_DATA_IN_PRI_MEM)
    memset(priMemFreezeFrameBuffer, 0, sizeof(priMemFreezeFrameBuffer));
#endif
#if (DEM_EXT_DATA_IN_PRI_MEM)
    memset(priMemExtDataBuffer, 0, sizeof(priMemExtDataBuffer));
#endif
#if defined(DEM_USE_INDICATORS) && defined(DEM_USE_MEMORY_FUNCTIONS)
    memset(indicatorBuffer, 0, sizeof(indicatorBuffer));
#endif
#if (DEM_STORE_UDS_STATUS_BIT_SUBSET_FOR_ALL_EVENTS == STD_ON)
    memset(statusBitSubsetBuffer, 0, sizeof(statusBitSubsetBuffer));
#endif
#endif
}

void demZeroSecMemBuffers(void)
{
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
    memset(secMemEventBuffer, 0, sizeof(secMemEventBuffer));
    secMemOverflow = FALSE;
#if ( DEM_FF_DATA_IN_SEC_MEM )
    memset(secMemFreezeFrameBuffer, 0, sizeof(secMemFreezeFrameBuffer));
#endif
#if ( DEM_EXT_DATA_IN_SEC_MEM )
    memset(secMemExtDataBuffer, 0, sizeof(secMemExtDataBuffer));
#endif
#if (DEM_STORE_UDS_STATUS_BIT_SUBSET_FOR_ALL_EVENTS == STD_ON)
    memset(statusBitSubsetBuffer, 0, sizeof(statusBitSubsetBuffer));
#endif
#endif
}

void demZeroPermMemBuffers(void)
{
#if (DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON)
    memset(permMemEventBuffer, FREE_PERMANENT_DTC_ENTRY, sizeof(permMemEventBuffer));
#endif
}

void demZeroPreStoreFFMemBuffer(void)
{
#if (DEM_PRESTORAGE_FF_DATA_IN_MEM)
	memset(memPreStoreFreezeFrameBuffer, 0x00, sizeof(memPreStoreFreezeFrameBuffer));
#endif
}
#if defined(DEM_USE_IUMPR)
void demZeroIumprBuffer(void)
{
	memset(&iumprBuffer, 0, sizeof(iumprBuffer));
}

void demSetIgnitionCounterToMax(void) {
	ignitionCycleCountBuffer = 65535;
}

void demSetDenominatorToMax(Dem_RatioIdType ratioId) {
	iumprBufferLocal[ratioId].denominator.value = 65535;
}

void demSetNumeratorToMax(Dem_RatioIdType ratioId) {
	iumprBufferLocal[ratioId].numerator.value = 65535;
}
#endif
#endif


#ifdef DEM_USE_MEMORY_FUNCTIONS
static boolean eventIsStoredInMem(Dem_EventIdType eventId, const EventRecType* eventBuffer, uint32 eventBufferSize)
{
    boolean eventIdFound = FALSE;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
    if( IS_COMBINED_EVENT_ID(eventId) ) {
        /* Entry is for a combined event */
        const Dem_CombinedDTCCfgType *CombDTCCfg = &configSet->CombinedDTCConfig[TO_COMBINED_EVENT_CFG_IDX(eventId)];
        const Dem_DTCClassType *DTCClass = CombDTCCfg->DTCClassRef;
        for (uint32 i = 0u;((i < eventBufferSize) && (eventIdFound == FALSE)); i++) {
            for(uint16 evIdx = 0; (evIdx < DTCClass->NofEvents) && (eventIdFound == FALSE); evIdx++) {
                eventIdFound = (eventBuffer[i].EventData.eventId == DTCClass->Events[evIdx])? TRUE: FALSE;
            }
        }
    }
    else {
        for (uint32 i = 0u;((i < eventBufferSize) && (eventIdFound == FALSE)); i++) {
            eventIdFound = (eventBuffer[i].EventData.eventId == eventId)? TRUE: FALSE;
        }
    }
#else
    for (uint32 i = 0u;((i < eventBufferSize) && (eventIdFound == FALSE)); i++) {
        eventIdFound = (eventBuffer[i].EventData.eventId == eventId)? TRUE: FALSE;
    }
#endif
    return eventIdFound;
}
#endif

/**
 * Determines if a DTC is available or not
 * @param DTCClass
 * @return TRUE: DTC available, FALSE: DTC NOT available
 */
static boolean DTCIsAvailable(const Dem_DTCClassType *DTCClass)
{
    if( (TRUE == DTCClass->DTCRef->DTCUsed)
#if (DEM_DTC_SUPPRESSION_SUPPORT == STD_ON)
            && (FALSE == DemDTCSuppressed[DTCClass->DTCIndex].SuppressedByDTC)
            && (FALSE == DemDTCSuppressed[DTCClass->DTCIndex].SuppressedByEvent)
#endif
    ) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/*
 * Procedure:   checkDtcKind
 * Description: Return TRUE if "dtcKind" match the events DTCKind or "dtcKind"
 *              is "DEM_DTC_KIND_ALL_DTCS" otherwise FALSE.
 */
static boolean checkDtcKind(Dem_DTCKindType dtcKind, const Dem_EventParameterType *eventParam)
{
    boolean result = FALSE;
    if( (NULL != eventParam->DTCClassRef) && (DTCIsAvailable(eventParam->DTCClassRef) == TRUE) ) {
        result = ( (dtcKind == DEM_DTC_KIND_ALL_DTCS) || (DEM_NON_EMISSION_RELATED != (Dem_Arc_EventDTCKindType) *eventParam->EventDTCKind) )? TRUE: FALSE;
    }

    return result;
}

/**
 * Checks if event status matches current filter.
 * @param filterMask
 * @param eventStatus
 * @return
 */
static boolean checkDtcStatusMask(Dem_EventStatusExtendedType filterMask, Dem_EventStatusExtendedType eventStatus)
{
    boolean result = FALSE;

    if( (DEM_DTC_STATUS_MASK_ALL == filterMask) || (0 != (eventStatus & filterMask)) ) {
        result = TRUE;
    }

    return result;
}
/**
 * Checks if DTC is available on specific format
 * @param eventParam
 * @param dtcFormat
 * @return TRUE: DTC is available on specific format, FALSE: Event is not available on specific format.
 */
static boolean eventHasDTCOnFormat(const Dem_EventParameterType *eventParam, Dem_DTCFormatType dtcFormat)
{
    boolean ret = FALSE;
    if(( NULL != eventParam) && (NULL != eventParam->DTCClassRef) ) {
        ret = ( ((DEM_DTC_FORMAT_UDS == dtcFormat) && (DEM_NO_DTC != eventParam->DTCClassRef->DTCRef->UDSDTC)) ||
                ((DEM_DTC_FORMAT_OBD == dtcFormat) && (DEM_NO_DTC != eventParam->DTCClassRef->DTCRef->OBDDTC)))? TRUE: FALSE;
    }
    return ret;
}

/**
 * Checks if DTC is avalailable on specific format.
 * @param eventParam
 * @param dtcFormat
 * @return
 */
static boolean DTCISAvailableOnFormat(const Dem_DTCClassType *DTCClass, Dem_DTCFormatType dtcFormat)
{
    boolean ret = FALSE;
    if( NULL != DTCClass ) {
        ret = ( ((DEM_DTC_FORMAT_UDS == dtcFormat) && (DEM_NO_DTC != DTCClass->DTCRef->UDSDTC)) ||
                ((DEM_DTC_FORMAT_OBD == dtcFormat) && (DEM_NO_DTC != DTCClass->DTCRef->OBDDTC)))? TRUE: FALSE;
    }
    return ret;
}

/**
 * Checks if dtc is a DTC group
 * @param dtc
 * @param dtcFormat
 * @param groupLower
 * @param groupUpper
 * @return TRUE: dtc is group, FALSE: dtc is NOT a group
 */
static boolean dtcIsGroup(uint32 dtc, Dem_DTCFormatType dtcFormat, uint32 *groupLower, uint32 *groupUpper)
{
    const Dem_GroupOfDtcType * DTCGroups = configSet->GroupOfDtc;
    boolean groupFound = FALSE;
    if( DEM_DTC_FORMAT_UDS == dtcFormat ) {
        while( (FALSE == DTCGroups->Arc_EOL) && (FALSE == groupFound) ) {
            if( dtc == DTCGroups->DemGroupDTCs ) {
                *groupLower = DTCGroups->DemGroupDTCs;
                groupFound = TRUE;
            }
            DTCGroups++;
        }
        *groupUpper = DTCGroups->DemGroupDTCs - 1u;
    }

    return groupFound;
}

/*
 * Procedure:   checkDtcGroup
 * Description: Return TRUE if "dtc" match the events DTC or "dtc" is
 *              "DEM_DTC_GROUP_ALL_DTCS" otherwise FALSE.
 */
/**
 * Checks is event has DTC matching "dtc". "dtc" can be a group or a specific DTC
 * @param dtc
 * @param eventParam
 * @param dtcFormat
 * @param checkFormat
 * @return TRUE: event has DTC matching "dtc", FALSE: DTC does NOT have DTC matching "dtc"
 */
static boolean checkDtcGroup(uint32 dtc, const Dem_EventParameterType *eventParam, Dem_DTCFormatType dtcFormat)
{
    /* NOTE: dtcFormat determines the format of dtc */
    boolean result = FALSE;

    if( (NULL != eventParam->DTCClassRef) && (TRUE == DTCIsAvailable(eventParam->DTCClassRef)) ) {
        if( DEM_DTC_GROUP_ALL_DTCS == dtc ) {
            result = (DEM_DTC_FORMAT_UDS == dtcFormat) ? (DEM_NO_DTC != eventParam->DTCClassRef->DTCRef->UDSDTC) : (DEM_NO_DTC != eventParam->DTCClassRef->DTCRef->OBDDTC);
        }
        else if( DEM_DTC_GROUP_EMISSION_REL_DTCS == dtc ) {
            result = (DEM_DTC_FORMAT_UDS == dtcFormat) ? ((DEM_NO_DTC != eventParam->DTCClassRef->DTCRef->UDSDTC) && (DEM_NO_DTC != eventParam->DTCClassRef->DTCRef->OBDDTC)) : (DEM_NO_DTC != eventParam->DTCClassRef->DTCRef->OBDDTC);
        }
        else {
            /* Not "ALL DTCs" */
            if( TRUE == eventHasDTCOnFormat(eventParam, dtcFormat) ) {
                uint32 DTCGroupLower;
                uint32 DTCGroupUpper;
                if( TRUE == dtcIsGroup(dtc, dtcFormat, &DTCGroupLower, &DTCGroupUpper) ) {
                    if( DEM_DTC_FORMAT_UDS == dtcFormat ) {
                        result = ((eventParam->DTCClassRef->DTCRef->UDSDTC >= DTCGroupLower) && (eventParam->DTCClassRef->DTCRef->UDSDTC < DTCGroupUpper))? TRUE: FALSE;
                    }
                    else {
                        result = ((eventParam->DTCClassRef->DTCRef->OBDDTC >= DTCGroupLower) && (eventParam->DTCClassRef->DTCRef->OBDDTC < DTCGroupUpper))? TRUE: FALSE;
                    }
                }
                else {
                    result = (DEM_DTC_FORMAT_UDS == dtcFormat) ? (dtc == eventParam->DTCClassRef->DTCRef->UDSDTC) : (dtc == TO_OBD_FORMAT(eventParam->DTCClassRef->DTCRef->OBDDTC));
                }
            }
        }
    }

    return result;
}
#if (DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON)
/**
 * Checks if an event is emission related and MIL activating
 * @param eventParam
 * @return TRUE: event is emission related, FALSE: event is not emission related
 */
static boolean eventIsEmissionRelatedMILActivating(const Dem_EventParameterType *eventParam)
{
    boolean ret = FALSE;
    if( DEM_EMISSION_RELATED_MIL_ACTIVATING == (Dem_Arc_EventDTCKindType) *eventParam->EventDTCKind ) {
        ret = TRUE;
    }
    return ret;
}
#endif

#if (DEM_OBD_DISPLACEMENT_SUPPORT == STD_ON) && (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && defined(DEM_DISPLACEMENT_PROCESSING_DEM_INTERNAL) && defined (DEM_USE_MEMORY_FUNCTIONS)
/**
 * Checks if an event is emission related
 * @param eventParam
 * @return TRUE: event is emission related, FALSE: event is not emission related
 */
static boolean eventIsEmissionRelated(const Dem_EventParameterType *eventParam)
{
    boolean ret = FALSE;
    if( (DEM_EMISSION_RELATED_MIL_ACTIVATING == (Dem_Arc_EventDTCKindType) *eventParam->EventDTCKind) || (DEM_EMISSION_RELATED == (Dem_Arc_EventDTCKindType) *eventParam->EventDTCKind) ){
        ret = TRUE;
    }
    return ret;
}
#endif
/*
 * Procedure:   checkDtcOrigin
 * Description: Return TRUE if "dtcOrigin" match any of the events DTCOrigin otherwise FALSE.
 */
static inline boolean checkDtcOrigin(Dem_DTCOriginType dtcOrigin, const Dem_EventParameterType *eventParam, boolean allowPermanentMemory)
{
    boolean originMatch = FALSE;
    if( (TRUE == allowPermanentMemory) && (DEM_DTC_ORIGIN_PERMANENT_MEMORY == dtcOrigin) ) {
#if (DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON)
        /* Check if dtc is stored in permanent memory*/
        /* @req DEM301 */
        if( TRUE == eventIsEmissionRelatedMILActivating(eventParam) ) {
            for( uint32 i = 0; (i < DEM_MAX_NUMBER_EVENT_PERM_MEM) && (FALSE == originMatch); i++) {
                if( eventParam->DTCClassRef->DTCRef->OBDDTC == permMemEventBuffer[i].OBDDTC) {
                    /* DTC is stored */
                    originMatch = TRUE;
                }
            }
        }
#endif
    }
    else {
        originMatch = (eventParam->EventClass->EventDestination == dtcOrigin)? TRUE: FALSE;
    }
    return originMatch;
}

/**
 * Return TRUE if "dtcSeverityMask" match the DTC severity otherwise FALSE.
 * @param dtcSeverityMask
 * @param DTCClass
 * @return
 */
static boolean checkDtcSeverityMask(const Dem_DTCSeverityType dtcSeverityMask, const Dem_DTCClassType *DTCClass)
{
    return ((NULL_PTR != DTCClass) && ( 0 != (DTCClass->DTCSeverity & dtcSeverityMask)))? TRUE: FALSE;
}

/*
 * Procedure:   lookupEventStatusRec
 * Description: Returns the pointer to event id parameters of "eventId" in "*eventStatusBuffer",
 *              if not found NULL is returned.
 */
void lookupEventStatusRec(Dem_EventIdType eventId, EventStatusRecType **const eventStatusRec)
{
    if ( IS_VALID_EVENT_ID(eventId)) {
        *eventStatusRec = &eventStatusBuffer[eventId - 1];
    } else {
        DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GLOBAL_ID, DEM_E_UNEXPECTED_EXECUTION);
        *eventStatusRec = NULL;
    }
}

/*
 * Procedure:   lookupEventIdParameter
 * Description: Returns the pointer to event id parameters of "eventId" in "*eventIdParam",
 *              if not found NULL is returned.
 */
void lookupEventIdParameter(Dem_EventIdType eventId, const Dem_EventParameterType **const eventIdParam)
{
    const Dem_EventParameterType *EventIdParamList = configSet->EventParameter;
    if (IS_VALID_EVENT_ID(eventId)) {
        *eventIdParam = &EventIdParamList[eventId - 1];
    } else {
        *eventIdParam = NULL;
    }
}
/*
 * Procedure:   checkEntryValid
 * Description: Returns whether event id "eventId" is a valid entry in primary memory
 */
#ifdef DEM_USE_MEMORY_FUNCTIONS
static boolean checkEntryValid(Dem_EventIdType eventId, Dem_DTCOriginType origin, boolean allowCombinedID){
    const Dem_EventParameterType *EventIdParam = NULL;
    EventStatusRecType *eventStatusRec = NULL;
    boolean isValid = FALSE;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
    if( (TRUE == allowCombinedID) && IS_COMBINED_EVENT_ID(eventId) ) {
        if( IS_VALID_COMBINED_ID(TO_COMBINED_EVENT_CFG_IDX(eventId)) ) {
            /* ID is valid, but is it valid for the destination */
            const Dem_CombinedDTCCfgType *CombDTCCfg = &configSet->CombinedDTCConfig[TO_COMBINED_EVENT_CFG_IDX(eventId)];
            if( (origin == CombDTCCfg->MemoryDestination) && (TRUE == DTCIsAvailable(CombDTCCfg->DTCClassRef)) ) {
                isValid = TRUE;
            }
        }
    }
    else {
        if( !IS_COMBINED_EVENT_ID(eventId) ) {
            lookupEventIdParameter(eventId, &EventIdParam);
            if (NULL != EventIdParam) {
                // Event was found
                lookupEventStatusRec(eventId, &eventStatusRec);
                // Event should be stored in destination memory?
                isValid = (checkDtcOrigin(origin, EventIdParam, FALSE) && (NULL != eventStatusRec) && eventStatusRec->isAvailable)? TRUE: FALSE;
            } else {
                // The event did not exist
            }
        }
    }
#else
    (void)allowCombinedID;
    lookupEventIdParameter(eventId, &EventIdParam);
    if (NULL != EventIdParam) {
        // Event was found
        lookupEventStatusRec(eventId, &eventStatusRec);
        // Event should be stored in destination memory?
        isValid = ((TRUE == checkDtcOrigin(origin, EventIdParam, FALSE)) && (NULL != eventStatusRec) && (TRUE == eventStatusRec->isAvailable))? TRUE: FALSE;
    } else {
        // The event did not exist
    }
#endif
    return isValid;
}
#endif

/**
 * Checks if an operation cycle is started
 * @param opCycle
 * @return
 */
boolean operationCycleIsStarted(Dem_OperationCycleIdType opCycle)
{
    boolean isStarted = FALSE;
    if (opCycle < DEM_OPERATION_CYCLE_ID_ENDMARK) {
        if (operationCycleStateList[opCycle] == DEM_CYCLE_STATE_START) {
            isStarted = TRUE;
        }
    }
    return isStarted;
}

#if defined(DEM_FAILURE_PROCESSING_DEM_INTERNAL)
static boolean failureCycleIsStarted(const Dem_EventParameterType *eventParam)
{
    return operationCycleIsStarted((Dem_OperationCycleIdType)*(eventParam->EventClass->FailureCycleRef));
}

static boolean faultConfirmationCriteriaFulfilled(const Dem_EventParameterType *eventParam, const EventStatusRecType *eventStatusRecPtr)
{
    if(((Dem_OperationCycleIdType)*(eventParam->EventClass->FailureCycleRef) != DEM_OPERATION_CYCLE_ID_ENDMARK) &&
            (eventStatusRecPtr->failureCounter >= eventParam->EventClass->FailureCycleCounterThresholdRef->Threshold) ) {
        return TRUE;
    } else {
        return FALSE;
    }
}
static void handleFaultConfirmation(const Dem_EventParameterType *eventParam, EventStatusRecType *eventStatusRecPtr)
{
    if( (TRUE == failureCycleIsStarted(eventParam)) && (FALSE == eventStatusRecPtr->failedDuringFailureCycle) ) {
        if( eventStatusRecPtr->failureCounter < DEM_FAILURE_CNTR_MAX ) {
            eventStatusRecPtr->failureCounter++;
            eventStatusRecPtr->errorStatusChanged = TRUE;
        }

        /* @req DEM530 */
        if( TRUE == faultConfirmationCriteriaFulfilled(eventParam, eventStatusRecPtr )) {
            eventStatusRecPtr->eventStatusExtended |= DEM_CONFIRMED_DTC;
            eventStatusRecPtr->errorStatusChanged = TRUE;
        }
        eventStatusRecPtr->failedDuringFailureCycle = TRUE;
    }
}
#endif

#if defined(DEM_USE_INDICATORS)

/**
 * Resets healing and failure counter for event indicators
 * @param eventParam
 * @return TRUE: counter value changed, FALSE: no change
 */
static boolean resetIndicatorCounters(const Dem_EventParameterType *eventParam)
{
    boolean countersChanged = FALSE;
    if( (NULL != eventParam->EventClass->IndicatorAttribute )  && (TRUE == (boolean)(*eventParam->EventClass->IndicatorAttribute->IndicatorValid)) ){
        const Dem_IndicatorAttributeType *indAttrPtr = eventParam->EventClass->IndicatorAttribute;
        while( FALSE == indAttrPtr->Arc_EOL) {
            if( (0 != indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].FailureCounter) || (0 != indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].HealingCounter)) {
                countersChanged = TRUE;
            }
            indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].FailureCounter = 0;
            indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].HealingCounter = 0;
            indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].OpCycleStatus = 0;
            indAttrPtr++;
        }
    }
#if defined(DEM_USE_MEMORY_FUNCTIONS)
    if(TRUE == countersChanged) {
        storeEventIndicators(eventParam);
    }
#endif
    return countersChanged;
}

/**
 * Checks if indicator failure cycle is started
 * @param eventParam
 * @param indAttr
 * @return TRUE: Failure cycle is started, FALSE: failure cycle not started
 */
static boolean indicatorFailureCycleIsStarted(const Dem_EventParameterType *eventParam, const Dem_IndicatorAttributeType *indAttr)
{
    if(DEM_FAILURE_CYCLE_INDICATOR == indAttr->IndicatorFailureCycleSource) {
        return operationCycleIsStarted((Dem_OperationCycleStateType)*(indAttr->IndicatorFailureCycle));
    } else {
        return operationCycleIsStarted((Dem_OperationCycleIdType)*(eventParam->EventClass->FailureCycleRef));
    }
}

/**
 * Checks if failure criteria for indicator is fulfilled
 * @param eventParam
 * @param indicatorAttribute
 * @return TRUE: criteria fulfilled, FALSE: criteria not fulfilled
 */
static boolean indicatorFailFulfilled(const Dem_EventParameterType *eventParam, const Dem_IndicatorAttributeType *indicatorAttribute)
{
    boolean fulfilled = FALSE;
    uint8 thresHold;
    Dem_OperationCycleIdType opCyc;

    if( DEM_FAILURE_CYCLE_INDICATOR == indicatorAttribute->IndicatorFailureCycleSource ) {
        thresHold = indicatorAttribute->IndicatorFailureCycleThreshold;
        opCyc = ((Dem_OperationCycleStateType)*(indicatorAttribute->IndicatorFailureCycle));
    } else {
        thresHold = eventParam->EventClass->FailureCycleCounterThresholdRef->Threshold;
        opCyc = (Dem_OperationCycleIdType)*(eventParam->EventClass->FailureCycleRef);
    }
    /* @req DEM501 */
    if( (opCyc < DEM_OPERATION_CYCLE_ID_ENDMARK) && (indicatorStatusBuffer[indicatorAttribute->IndicatorBufferIndex].FailureCounter >= thresHold) ) {
        fulfilled = TRUE;
    }
    return fulfilled;
}

/**
 * Checks if warningIndicatorOnCriteria is fulfilled for an event
 * @param eventParam
 * @return TRUE: criteria fulfilled, FALSE: criteria not fulfilled
 */
static boolean warningIndicatorOnCriteriaFulfilled(const Dem_EventParameterType *eventParam)
{
    boolean fulfilled = FALSE;
    if( (NULL != eventParam->EventClass->IndicatorAttribute) && (TRUE == (boolean)(*eventParam->EventClass->IndicatorAttribute->IndicatorValid)) ) {
        const Dem_IndicatorAttributeType *indAttrPtr = eventParam->EventClass->IndicatorAttribute;
        /* @req DEM566 */
        while( (FALSE == indAttrPtr->Arc_EOL) && (FALSE == fulfilled) ) {
            fulfilled = indicatorFailFulfilled(eventParam, indAttrPtr);
            indAttrPtr++;
        }
    }
    return fulfilled;
}

/**
 * Checks if indicator healing cycle is started
 * @param indAttr
 * @return TRUE: Healing cycle is started, FALSE: Healing cycle not started
 */
static boolean indicatorHealingCycleIsStarted(const Dem_IndicatorAttributeType *indAttr)
{
    return operationCycleIsStarted(indAttr->IndicatorHealingCycle);
}

/**
 * Prepares indicator status for a new failure/healing cycle
 * @param operationCycleId
 */
static void indicatorOpCycleStart(Dem_OperationCycleIdType operationCycleId) {
    Dem_OperationCycleIdType failCyc;
    const Dem_EventParameterType *eventIdParamList = configSet->EventParameter;
    uint16 indx = 0;
    while( FALSE == eventIdParamList[indx].Arc_EOL ) {
        if( NULL != eventIdParamList[indx].EventClass->IndicatorAttribute  ) {
            const Dem_IndicatorAttributeType *indAttrPtr = eventIdParamList[indx].EventClass->IndicatorAttribute;
            while( FALSE == indAttrPtr->Arc_EOL) {
                if( operationCycleId == indAttrPtr->IndicatorHealingCycle ) {
                    indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].OpCycleStatus &= (uint8)~(INDICATOR_FAILED_DURING_HEALING_CYCLE | INDICATOR_PASSED_DURING_HEALING_CYCLE);
                }
                if( DEM_FAILURE_CYCLE_INDICATOR == indAttrPtr->IndicatorFailureCycleSource ) {
                    failCyc = ((Dem_OperationCycleStateType)*indAttrPtr->IndicatorFailureCycle);
                } else {
                    failCyc = (Dem_OperationCycleIdType)*(eventIdParamList[indx].EventClass->FailureCycleRef);
                }
                if( operationCycleId == failCyc ) {
                    indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].OpCycleStatus &= (uint8)~(INDICATOR_FAILED_DURING_FAILURE_CYCLE | INDICATOR_PASSED_DURING_FAILURE_CYCLE);
                }
                indAttrPtr++;
            }
        }
        indx++;
    }
}

/**
 * Checks if warningIndicatorOffCriteria is fulfilled for an event
 * @param eventParam
 * @return TRUE: criteria fulfilled, FALSE: criteria not fulfilled
 */
static boolean warningIndicatorOffCriteriaFulfilled(const Dem_EventParameterType *eventParam)
{
    return (FALSE == warningIndicatorOnCriteriaFulfilled(eventParam))? TRUE :FALSE;
}

/**
 * Handles end of operation cycles for indicators
 * @param operationCycleId
 * @param eventStatusRecPtr
 * @return TRUE: Counter updated for at least one event
 */
static boolean indicatorOpCycleEnd(Dem_OperationCycleIdType operationCycleId, EventStatusRecType *eventStatusRecPtr) {
    /* @req DEM502 */
    /* @req DEM505 */
    Dem_OperationCycleIdType failCyc;
    boolean counterChanged = FALSE;
    uint8 healingCounterOld = 0u;
    uint8 failureCounterOld = 0u;
    const Dem_EventParameterType *eventParam = eventStatusRecPtr->eventParamRef;
    if( (NULL != eventParam) && (NULL != eventParam->EventClass->IndicatorAttribute)&& (TRUE == (boolean)(*eventParam->EventClass->IndicatorAttribute->IndicatorValid))  ) {
        const Dem_IndicatorAttributeType *indAttrPtr = eventParam->EventClass->IndicatorAttribute;
        while( FALSE == indAttrPtr->Arc_EOL) {
            healingCounterOld = indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].HealingCounter;
            failureCounterOld = indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].FailureCounter;
            if( (operationCycleId == indAttrPtr->IndicatorHealingCycle) &&
                    (0 == (indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].OpCycleStatus & INDICATOR_FAILED_DURING_HEALING_CYCLE)) &&
                    (0 != (indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].OpCycleStatus & INDICATOR_PASSED_DURING_HEALING_CYCLE)) &&
                    (TRUE == indicatorFailFulfilled(eventParam, indAttrPtr))) {
                /* Passed and didn't fail during the healing cycle */
                if( indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].HealingCounter < DEM_INDICATOR_CNTR_MAX ) {
                    indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].HealingCounter++;
                }
                if( indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].HealingCounter >= indAttrPtr->IndicatorHealingCycleThreshold ) {
                    /* @req DEM503 */
                    /* Healing condition fulfilled.
                     * Should we reset failure counter here? */
                    indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].FailureCounter = 0;
                }
            }

            if( DEM_FAILURE_CYCLE_INDICATOR == indAttrPtr->IndicatorFailureCycleSource ) {
                failCyc = ((Dem_OperationCycleStateType)*indAttrPtr->IndicatorFailureCycle);
            } else {
                failCyc = (Dem_OperationCycleIdType)*(eventParam->EventClass->FailureCycleRef);
            }
            if( (operationCycleId == failCyc) &&
                    (0 == (indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].OpCycleStatus & INDICATOR_FAILED_DURING_FAILURE_CYCLE)) &&
                    (0 != (indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].OpCycleStatus & INDICATOR_PASSED_DURING_FAILURE_CYCLE)) &&
                    (FALSE == indicatorFailFulfilled(eventParam, indAttrPtr))) {
                /* Passed and didn't fail during the failure cycle.
                 * Reset failure counter */
                indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].FailureCounter = 0;
            }
            if( (healingCounterOld != indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].HealingCounter) ||
                    (failureCounterOld != indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].FailureCounter) ) {
                counterChanged = TRUE;
            }
            indAttrPtr++;
        }
        /* @req DEM533 */
        if( TRUE == warningIndicatorOffCriteriaFulfilled(eventParam)) {
            eventStatusRecPtr->eventStatusExtended &= ~DEM_WARNING_INDICATOR_REQUESTED;
        }

#if defined(DEM_USE_MEMORY_FUNCTIONS)
        if(TRUE == counterChanged) {
            storeEventIndicators(eventParam);
        }
#endif
    }
    return counterChanged;
}

/**
 * Performs clearing of indicator healing counter if conditions fulfilled.
 * NOTE: This functions should only be called when event is FAILED.
 * @param eventParam
 * @param indAttrPtr
 */
static inline void handleHealingCounterOnFailed(const Dem_EventParameterType *eventParam, const Dem_IndicatorAttributeType *indAttrPtr)
{
#if defined(DEM_HEALING_COUNTER_CLEAR_ON_FAIL_DURING_FAILURE_CYCLE)
    if( TRUE == indicatorFailureCycleIsStarted(eventParam, indAttrPtr) ) {
        indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].HealingCounter = 0;
    }
#elif defined(DEM_HEALING_COUNTER_CLEAR_ON_FAIL_DURING_FAILURE_OR_HEALING_CYCLE)
    if( (TRUE == indicatorFailureCycleIsStarted(eventParam, indAttrPtr)) || (TRUE == indicatorHealingCycleIsStarted(indAttrPtr)) ) {
        indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].HealingCounter = 0;
    }
#elif defined(DEM_HEALING_COUNTER_CLEAR_ON_ALL_FAIL)
    (void)eventParam;/*lint !e920*/
    indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].HealingCounter = 0;
#else
#error "Dem: Unknown healing counter clear behavior"
#endif
}

/**
 * Handles updating of indicator failure counter and status bits
 * @param eventParam
 * @param eventStatusRecPtr
 * @param eventStatus
 * @return TRUE: counter was updated, FALSE: counter was not updated
 */
static boolean handleIndicators(const Dem_EventParameterType *eventParam, EventStatusRecType *eventStatusRecPtr, Dem_EventStatusType eventStatus)
{
    /* @req DEM506 */
    /* @req DEM510 */
    boolean cntrChanged = FALSE;
    if( (NULL != eventParam->EventClass->IndicatorAttribute) && (TRUE == (boolean)(*eventParam->EventClass->IndicatorAttribute->IndicatorValid)) ){
        const Dem_IndicatorAttributeType *indAttrPtr = eventParam->EventClass->IndicatorAttribute;
        while( FALSE == indAttrPtr->Arc_EOL) {
            switch(eventStatus) {
                case DEM_EVENT_STATUS_FAILED:
                    if( TRUE == indicatorFailureCycleIsStarted(eventParam, indAttrPtr) ) {
                        if( (0 == (indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].OpCycleStatus & INDICATOR_FAILED_DURING_FAILURE_CYCLE)) &&
                                (indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].FailureCounter < DEM_INDICATOR_CNTR_MAX) ) {
                            /* First fail during this failure cycle and incrementing failure counter would not overflow */
                            indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].FailureCounter++;
                            cntrChanged = TRUE;
                        }
                        indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].OpCycleStatus |= INDICATOR_FAILED_DURING_FAILURE_CYCLE;
                    }
                    if( TRUE == indicatorHealingCycleIsStarted(indAttrPtr) ) {
                        indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].OpCycleStatus |= INDICATOR_FAILED_DURING_HEALING_CYCLE;
                    }
                    break;
                case DEM_EVENT_STATUS_PASSED:
                    if( TRUE == indicatorFailureCycleIsStarted(eventParam, indAttrPtr) ) {
                        indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].OpCycleStatus |= INDICATOR_PASSED_DURING_FAILURE_CYCLE;
                    }
                    if( TRUE == indicatorHealingCycleIsStarted(indAttrPtr) ) {
                        indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].OpCycleStatus |= INDICATOR_PASSED_DURING_HEALING_CYCLE;
                    }
                    break;
                default:
                    break;
            }
            if( DEM_EVENT_STATUS_FAILED == eventStatus ) {
                handleHealingCounterOnFailed(eventParam, indAttrPtr);
            }
            indAttrPtr++;
        }
        /* @req DEM566 */
        if( TRUE == warningIndicatorOnCriteriaFulfilled(eventParam)) {
            eventStatusRecPtr->eventStatusExtended |= DEM_WARNING_INDICATOR_REQUESTED;
        }
    }
    return cntrChanged;
}

#if defined(DEM_USE_MEMORY_FUNCTIONS)
/**
 * Stores indicators for an event in memory destined for NvRam
 * @param eventParam
 */
static void storeEventIndicators(const Dem_EventParameterType *eventParam)
{
    const Dem_IndicatorAttributeType *indAttr;
    if( (NULL != eventParam->EventClass->IndicatorAttribute) && (TRUE == (boolean)(*eventParam->EventClass->IndicatorAttribute->IndicatorValid)) ){
        indAttr = eventParam->EventClass->IndicatorAttribute;
        while( FALSE == indAttr->Arc_EOL) {
            indicatorBuffer[indAttr->IndicatorBufferIndex].EventID = indicatorStatusBuffer[indAttr->IndicatorBufferIndex].EventID;
            indicatorBuffer[indAttr->IndicatorBufferIndex].FailureCounter = indicatorStatusBuffer[indAttr->IndicatorBufferIndex].FailureCounter;
            indicatorBuffer[indAttr->IndicatorBufferIndex].HealingCounter = indicatorStatusBuffer[indAttr->IndicatorBufferIndex].HealingCounter;
            indicatorBuffer[indAttr->IndicatorBufferIndex].IndicatorId = indAttr->IndicatorId;
            indAttr++;
        }
    }
}

/**
 * Merges indicator status read from NvRam with status held in ram
 */
static void mergeIndicatorBuffers(void) {
    const Dem_EventParameterType *eventParam;
    const Dem_IndicatorAttributeType *indAttr;
    for(uint32 i = 0; i < DEM_NOF_EVENT_INDICATORS; i++) {
        if(IS_VALID_EVENT_ID(indicatorBuffer[i].EventID) && IS_VALID_INDICATOR_ID(indicatorBuffer[i].IndicatorId)) {
            /* Valid event and indicator. Check that it is a valid indicator for this event */
            eventParam = NULL;
            lookupEventIdParameter(indicatorBuffer[i].EventID, &eventParam);
            if((NULL != eventParam) && (NULL != eventParam->EventClass->IndicatorAttribute) && (TRUE == (boolean)(*eventParam->EventClass->IndicatorAttribute->IndicatorValid)) ){
                indAttr = eventParam->EventClass->IndicatorAttribute;
                while(FALSE == indAttr->Arc_EOL) {
                    if( indAttr->IndicatorId == indicatorBuffer[i].IndicatorId ) {
                        /* Update healing counter. */
                        if( (0 != (indicatorStatusBuffer[indAttr->IndicatorBufferIndex].OpCycleStatus & INDICATOR_FAILED_DURING_FAILURE_CYCLE)) ||
                                (0 != (indicatorStatusBuffer[indAttr->IndicatorBufferIndex].OpCycleStatus & INDICATOR_FAILED_DURING_HEALING_CYCLE))) {
                            /* Failed at some point during PreInit.  */
                            indicatorStatusBuffer[indAttr->IndicatorBufferIndex].HealingCounter = 0;
                        } else {
                            indicatorStatusBuffer[indAttr->IndicatorBufferIndex].HealingCounter = indicatorBuffer[i].HealingCounter;
                        }

                        /* Update failure counter */
                        if( (DEM_INDICATOR_CNTR_MAX - indicatorBuffer[i].FailureCounter) > indicatorStatusBuffer[indAttr->IndicatorBufferIndex].FailureCounter) {
                            indicatorStatusBuffer[indAttr->IndicatorBufferIndex].FailureCounter += indicatorBuffer[i].FailureCounter;
                        } else {
                            indicatorStatusBuffer[indAttr->IndicatorBufferIndex].FailureCounter = DEM_INDICATOR_CNTR_MAX;
                        }
                    }
                    indAttr++;
                }
            }
        }
    }
#ifdef DEM_USE_MEMORY_FUNCTIONS
    /* Transfer content of indicatorStatusBuffer to indicatorBuffer */
    eventParam = configSet->EventParameter;
    while( FALSE == eventParam->Arc_EOL ) {
        storeEventIndicators(eventParam);
        eventParam++;
    }
    /* IMPROVEMENT: Only call this if the content of memory was changed */
    /* IMPROVEMENT: Add handling of immediate storage */
    Dem_NvM_SetIndicatorBlockChanged(FALSE);
#endif
}

#endif
#endif
#if (DEM_USE_TIMESTAMPS == STD_ON) && defined (DEM_USE_MEMORY_FUNCTIONS)
static void setEventTimeStamp(EventStatusRecType *eventStatusRecPtr)
{
    if( DEM_INITIALIZED == demState ) {
        if( Event_TimeStamp >= DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT ) {
            rearrangeEventTimeStamp(&Event_TimeStamp);

        }
        eventStatusRecPtr->timeStamp = Event_TimeStamp;
        Event_TimeStamp++;
    } else {
        eventStatusRecPtr->timeStamp = Event_TimeStamp;
        if( Event_TimeStamp < DEM_MAX_TIMESTAMP_FOR_PRE_INIT ) {
            Event_TimeStamp++;
        }
    }
}
#endif

#if (defined(DEM_USE_INDICATORS) && (DEM_OBD_DISPLACEMENT_SUPPORT == STD_ON)) || (DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON)
/**
 * Checks if event currently activates MIL
 * @param eventParam
 * @return
 */
static boolean eventActivatesMIL(const Dem_EventParameterType *eventParam)
{
    boolean fulfilled = FALSE;
    if( (NULL != eventParam->EventClass->IndicatorAttribute) && (TRUE == (boolean)(*eventParam->EventClass->IndicatorAttribute->IndicatorValid)) )  {
#if defined(DEM_USE_INDICATORS)
        const Dem_IndicatorAttributeType *indAttrPtr = eventParam->EventClass->IndicatorAttribute;
        /* @req DEM566 */
        while( (FALSE == indAttrPtr->Arc_EOL) && (FALSE == fulfilled) ) {
            if( DEM_MIL_INIDICATOR_ID == indAttrPtr->IndicatorId ) {
                fulfilled |= indicatorFailFulfilled(eventParam, indAttrPtr);
            }
            indAttrPtr++;
        }
#endif
    }
    return fulfilled;
}
#endif

#if (DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON)
/* !req DEM300 Do we handle according to regulations? */

/**
 * Stores a permanent DTC
 * @param DTCRef
 * @return TRUE: buffer was updated, FALSE: buffer NOT updated
 */
static boolean storeDTCPermanentMemory(const Arc_Dem_DTC *DTCRef)
{
    boolean alreadyStored = FALSE;
    boolean freeEntryFound = FALSE;
    boolean stored = FALSE;
    uint32 storeIndex = 0;
    /* Check if it is already stored and if there is a free entry */
    for( uint32 i = 0; (i < DEM_MAX_NUMBER_EVENT_PERM_MEM) && (FALSE == alreadyStored); i++) {
        if( FREE_PERMANENT_DTC_ENTRY == permMemEventBuffer[i].OBDDTC ) {
            /* Found a free entry */
            freeEntryFound = TRUE;
            storeIndex = i;
        }

        if( DTCRef->OBDDTC == permMemEventBuffer[i].OBDDTC) {
            /* DTC already stored */
            alreadyStored = TRUE;
        }
    }
    if( (FALSE == alreadyStored) && (TRUE == freeEntryFound) ) {
        permMemEventBuffer[storeIndex].OBDDTC = DTCRef->OBDDTC;
        stored = TRUE;
    }
    return stored;
}

/**
 * Deletes a permanent DTC
 * @param DTCRef
 * @return TRUE: buffer was updated, FALSE: buffer NOT updated
 */
static boolean deleteDTCPermanentMemory(const Arc_Dem_DTC *DTCRef)
{
    boolean deleted = FALSE;
    /* Check if it is already stored and if there is a free entry */
    for( uint32 i = 0; (i < DEM_MAX_NUMBER_EVENT_PERM_MEM) && (FALSE == deleted); i++) {
        if( DTCRef->OBDDTC == permMemEventBuffer[i].OBDDTC) {
            /* DTC already stored */
            permMemEventBuffer[i].OBDDTC = FREE_PERMANENT_DTC_ENTRY;
            deleted = TRUE;
        }
    }

    return deleted;
}

/**
 * Check if a DTC is a valid OBD DTC
 * @param dtc
 * @return
 */
static boolean isValidPermanentOBDDTC(uint32 dtc)
{
    boolean validDTC = FALSE;
    const Dem_EventParameterType *eventParam = configSet->EventParameter;
    while( (FALSE == eventParam->Arc_EOL) && (FALSE == validDTC)) {
        if( (DEM_EMISSION_RELATED_MIL_ACTIVATING == (Dem_Arc_EventDTCKindType) *eventParam->EventDTCKind) && (NULL != eventParam->DTCClassRef)){
            if( dtc == eventParam->DTCClassRef->DTCRef->OBDDTC ) {
                validDTC = TRUE;
            }
        }
        eventParam++;
    }

    return validDTC;
}

/**
 * Checks if condition for storing a permanent DTC is fulfilled
 * @param eventParam
 * @param evtStatus
 * @return TRUE: condition fulfilled, FALSE: condition NOT fulfilled
 */
static boolean permanentDTCStorageConditionFulfilled(const Dem_EventParameterType *eventParam, Dem_EventStatusExtendedType evtStatus)
{
    boolean fulfilled = FALSE;
    if( TRUE == eventIsEmissionRelatedMILActivating(eventParam) ) {
        /* This event has an emission related DTC.
         * Check if it is confirmed and activates MIL */
        if( (0u != (evtStatus & DEM_CONFIRMED_DTC)) && (TRUE == eventActivatesMIL(eventParam)) ) {
            /* Condition for storing as permanent is fulfilled*/
            fulfilled = TRUE;
        }
    }
    return fulfilled;
}

/**
 * Handles storage of permanent DTCs
 * @param eventParam
 * @param evtStatus
 * @return TRUE: permanent memory updated, FALSE: permanent memory NOT updated
 */
static boolean handlePermanentDTCStorage(const Dem_EventParameterType *eventParam, Dem_EventStatusExtendedType evtStatus)
{
    boolean memoryUpdated = FALSE;
    if( TRUE == permanentDTCStorageConditionFulfilled(eventParam, evtStatus) ) {
        /* Try store as permanent DTC */
        memoryUpdated = storeDTCPermanentMemory(eventParam->DTCClassRef->DTCRef);
    }
    return memoryUpdated;
}

/**
 * Handles erasing permanent DTCs.
 * NOTE: May only be called on operation cycle end!
 * @param eventParam
 * @param evtStatus
 */
static boolean handlePermanentDTCErase(const EventStatusRecType *eventRec, Dem_OperationCycleIdType operationCycleId)
{
    boolean MILHealOK = TRUE;
    const Dem_EventParameterType *eventParam = eventRec->eventParamRef;
    /* Check if emission related */
    boolean permanentMemoryUpdated = FALSE;
    if( TRUE == eventIsEmissionRelatedMILActivating(eventParam) ) {
        /* This event has an emission related DTC.
         * Check if it currently activates MIL */
        if( FALSE == eventActivatesMIL(eventParam)) {
#if defined(DEM_USE_INDICATORS)
            if( (NULL != eventParam->EventClass->IndicatorAttribute) && (TRUE == (boolean)(*eventParam->EventClass->IndicatorAttribute->IndicatorValid)) ) {
                const Dem_IndicatorAttributeType *indAttrPtr = eventParam->EventClass->IndicatorAttribute;
                /* @req DEM566 */
                while( (FALSE == indAttrPtr->Arc_EOL) && (FALSE != MILHealOK)) {
                    if( DEM_MIL_INIDICATOR_ID == indAttrPtr->IndicatorId ) {
                        if( (operationCycleId != indAttrPtr->IndicatorHealingCycle) ||
                            (0u != (indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].OpCycleStatus & INDICATOR_FAILED_DURING_HEALING_CYCLE)) ||
                            (0u == (indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].OpCycleStatus & INDICATOR_PASSED_DURING_HEALING_CYCLE))) {
                            /* Failed or did not pass.  */
                            MILHealOK = FALSE;
                        }
                    }
                    indAttrPtr++;
                }
            }
#endif
            if(TRUE == MILHealOK) {
                /* Delete permanent DTC */
                permanentMemoryUpdated = deleteDTCPermanentMemory(eventParam->DTCClassRef->DTCRef);
            }
        }
    }
    return permanentMemoryUpdated;
}

/**
 *
 */
static void ValidateAndUpdatePermanentBuffer(void)
{
    boolean newDataStored = FALSE;
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
    boolean immediateStorage = FALSE;
#endif
    /* Validate entries in the buffer */
    for( uint32 i = 0; i < DEM_MAX_NUMBER_EVENT_PERM_MEM; i++) {
        if( FALSE == isValidPermanentOBDDTC(permMemEventBuffer[i].OBDDTC)) {
            /* DTC is not a valid permanent DTC */
            permMemEventBuffer[i].OBDDTC = FREE_PERMANENT_DTC_ENTRY;
        }
    }

    /* Insert emission relates DTCs which are currently confirmed and activating MIL */
    for (uint16 i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
        if( DEM_EVENT_ID_NULL != eventStatusBuffer[i].eventId ) {
            if(TRUE == handlePermanentDTCStorage(eventStatusBuffer[i].eventParamRef, eventStatusBuffer[i].eventStatusExtended) ) {
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
                if( (NULL != eventStatusBuffer[i].eventParamRef->DTCClassRef) && (TRUE == eventStatusBuffer[i].eventParamRef->DTCClassRef->DTCRef->ImmediateNvStorage) &&
                        (eventStatusBuffer[i].occurrence <= DEM_IMMEDIATE_NV_STORAGE_LIMIT)) {
                    immediateStorage = TRUE;
                }
#endif
                newDataStored = TRUE;
            }
        }
    }
    /* Set block changed if new data was stored */
    if( TRUE == newDataStored ) {
        /* !req DEM590 */
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
        Dem_NvM_SetPermanentBlockChanged(immediateStorage);
#else
        Dem_NvM_SetPermanentBlockChanged(FALSE);
#endif
    }
}
#endif /* DEM_USE_PERMANENT_MEMORY_SUPPORT */

#if (DEM_PRESTORAGE_FF_DATA_IN_MEM)
static void ValidateAndUpdatePreStoredFreezeFramesBuffer(void)
{
    const Dem_EventParameterType *eventParam;
    boolean ffDeleted = FALSE;


    /* Validate entries in the buffer */
    for( uint32 i = 0; i < DEM_MAX_NUMBER_PRESTORED_FF; i++) {
        if (0u != memPreStoreFreezeFrameBuffer[i].eventId) {
            lookupEventIdParameter(memPreStoreFreezeFrameBuffer[i].eventId, &eventParam);

            if (NULL != eventParam ) {
                if (FALSE == eventParam->EventClass->FFPrestorageSupported) {
                    memset(&memPreStoreFreezeFrameBuffer[i], 0, sizeof(FreezeFrameRecType));
                    ffDeleted = TRUE;
                }
            }
            else {
                /* Invalid ID. Deletet it. */
                memset(&memPreStoreFreezeFrameBuffer[i], 0, sizeof(FreezeFrameRecType));
                ffDeleted = TRUE;
            }
        }

    }

   /* Set block changed if new data was stored */
    if( TRUE == ffDeleted ) {
        Dem_NvM_SetPreStoreFreezeFrameBlockChanged(FALSE);
    }
}
#endif

static void notifyEventStatusChange(const Dem_EventParameterType *eventParam, Dem_EventStatusExtendedType oldStatus, Dem_EventStatusExtendedType newStatus)
{
    uint8 j = 0;
    if( NULL != eventParam ) {
        if( NULL != eventParam->CallbackEventStatusChanged ) {
            /* @req Dem016 */ /* @req Dem615 */
            while( FALSE == eventParam->CallbackEventStatusChanged[j].Arc_EOL ) {
                if( TRUE == eventParam->CallbackEventStatusChanged[j].UsePort ) {
                    (void)eventParam->CallbackEventStatusChanged[j].CallbackEventStatusChangedFnc.eventStatusChangedWithoutId(oldStatus, newStatus);
                } else {
                    (void)eventParam->CallbackEventStatusChanged[j].CallbackEventStatusChangedFnc.eventStatusChangedWithId(eventParam->EventID, oldStatus, newStatus);
                }
                j++;
            }
        }
#if defined(USE_RTE) && (DEM_GENERAL_EVENT_STATUS_CB == STD_ON)
        /* @req Dem616 */
        (void)Rte_Call_GeneralCBStatusEvt_EventStatusChanged(eventParam->EventID, oldStatus, newStatus);
#endif

#if (DEM_TRIGGER_DLT_REPORTS == STD_ON)
        /* @req Dem517 */
        Dlt_DemTriggerOnEventStatus(eventParam->EventID, oldStatus, newStatus);
#endif

#if defined(USE_FIM) && (DEM_TRIGGER_FIM_REPORTS == STD_ON)
        /* @req Dem029 */
        if( TRUE == DemFiMInit ) {
            FiM_DemTriggerOnMonitorStatus(eventParam->EventID);
        }
#endif
    }
}

static void notifyEventDataChanged(const Dem_EventParameterType *eventParam) {
    /* @req DEM474 */
    if( (NULL != eventParam) && (NULL != eventParam->CallbackEventDataChanged)) {
        /* @req Dem618 */
        if( TRUE == eventParam->CallbackEventDataChanged->UsePort ) {
            (void)eventParam->CallbackEventDataChanged->CallbackEventDataChangedFnc.eventDataChangedWithoutId();
        } else {
            (void)eventParam->CallbackEventDataChanged->CallbackEventDataChangedFnc.eventDataChangedWithId(eventParam->EventID);
        }
    }

#if defined(USE_RTE) && (DEM_GENERAL_EVENT_DATA_CB == STD_ON)
    /* @req Dem619 */
    if( NULL != eventParam ) {
        (void)Rte_Call_GeneralCBDataEvt_EventDataChanged(eventParam->EventID);
    }
#endif
}

static void setDefaultEventStatus(EventStatusRecType *eventStatusRecPtr)
{
    eventStatusRecPtr->eventId = DEM_EVENT_ID_NULL;
    eventStatusRecPtr->eventParamRef = NULL;
    eventStatusRecPtr->fdcInternal = 0;
    eventStatusRecPtr->UDSFdc = 0;
    eventStatusRecPtr->maxUDSFdc = 0;
    eventStatusRecPtr->occurrence = 0;
    eventStatusRecPtr->eventStatusExtended = DEM_DEFAULT_EVENT_STATUS;
    eventStatusRecPtr->errorStatusChanged = FALSE;
    eventStatusRecPtr->extensionDataChanged = FALSE;
    eventStatusRecPtr->extensionDataStoreBitfield = 0;
#if defined(DEM_FAILURE_PROCESSING_DEM_INTERNAL)
    eventStatusRecPtr->failureCounter = 0;
    eventStatusRecPtr->failedDuringFailureCycle = FALSE;
    eventStatusRecPtr->passedDuringFailureCycle = FALSE;
#endif
#if defined(DEM_AGING_PROCESSING_DEM_INTERNAL)
    eventStatusRecPtr->agingCounter = 0;
    eventStatusRecPtr->passedDuringAgingCycle = FALSE;
    eventStatusRecPtr->failedDuringAgingCycle = FALSE;
#endif
    eventStatusRecPtr->timeStamp = 0;
}

#if defined(DEM_AGING_PROCESSING_DEM_INTERNAL)
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1) && defined(DEM_CLEAR_COMBINED_AGING_COUNTERS_ON_FAIL)
#if defined(DEM_USE_MEMORY_FUNCTIONS)
/**
 * Reset an existing aging counter in event memory.
 * @param eventId
 * @param buffer
 * @param bufferSize
 * @param origin
 */
static void resetAgingCounter(Dem_EventIdType eventId, EventRecType* buffer, uint32 bufferSize, Dem_DTCOriginType origin)
{
    boolean positionFound = FALSE;

    for (uint32 i = 0uL; (i < bufferSize) && (FALSE == positionFound); i++){
        if( buffer[i].EventData.eventId == eventId ) {
            buffer[i].EventData.agingCounter = 0u;
            Dem_NvM_SetEventBlockChanged(origin, FALSE);
        }
    }
}
#endif
/**
 * Resets aging counter in memory destination
 * @param eventId
 * @param DTCOrigin
 */
static void resetAgingCounterEvtMem(Dem_EventIdType eventId, Dem_DTCOriginType DTCOrigin)
{
    switch (DTCOrigin) {
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
            resetAgingCounter(eventId, priMemEventBuffer, DEM_MAX_NUMBER_EVENT_ENTRY_PRI, DEM_DTC_ORIGIN_PRIMARY_MEMORY);
#endif
            break;
        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
            resetAgingCounter(eventId, secMemEventBuffer, DEM_MAX_NUMBER_EVENT_ENTRY_SEC, DEM_DTC_ORIGIN_SECONDARY_MEMORY);
#endif
            break;
        default:
            /* Origin not supported */
            break;
    }
}
/**
 * Clears aging counter for all other sub-events of a combined event
 * @param eventParam
 */
static void clearCombinedEventAgingCounters(const Dem_EventParameterType *eventParam)
{
    EventStatusRecType *eventStatusRecPtr;
    const Dem_CombinedDTCCfgType *CombDTCCfg;
    const Dem_DTCClassType *DTCClass;
    if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID ) {
        CombDTCCfg = &configSet->CombinedDTCConfig[eventParam->CombinedDTCCID];
        DTCClass = CombDTCCfg->DTCClassRef;
        for(uint16 i = 0; i < DTCClass->NofEvents; i++) {
            if( eventParam->EventID != DTCClass->Events[i] ) {
                eventStatusRecPtr = NULL_PTR;
                lookupEventStatusRec(DTCClass->Events[i], &eventStatusRecPtr);
                if( NULL_PTR !=  eventStatusRecPtr ) {
                    if( (0u != (eventStatusRecPtr->eventStatusExtended & DEM_CONFIRMED_DTC)) && (0u != eventStatusRecPtr->agingCounter) ) {
                        eventStatusRecPtr->agingCounter = 0u;
                        resetAgingCounterEvtMem(DTCClass->Events[i], CombDTCCfg->MemoryDestination);
                    }
                }
            }
        }
    }
}
#endif
/**
 * Handles clearing of aging counter. Should only be called when operation cycle
 * is start and event is qualified as FAILED
 * @param eventParam
 * @param eventStatusRecPtr
 */
static inline void handleAgingCounterOnFailed(const Dem_EventParameterType *eventParam, EventStatusRecType *eventStatusRecPtr)
{
#if defined(DEM_AGING_COUNTER_CLEAR_ON_FAIL_DURING_FAILURE_CYCLE)
    if( TRUE == operationCycleIsStarted((Dem_OperationCycleIdType)*(eventParam->EventClass->FailureCycleRef)) ) {
        eventStatusRecPtr->agingCounter = 0;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1) && defined(DEM_CLEAR_COMBINED_AGING_COUNTERS_ON_FAIL)
        clearCombinedEventAgingCounters(eventParam);
#endif
    }
#elif defined(DEM_AGING_COUNTER_CLEAR_ON_FAIL_DURING_FAILURE_OR_AGING_CYCLE)
    if( (TRUE == operationCycleIsStarted((Dem_OperationCycleIdType)*(eventParam->EventClass->FailureCycleRef))) || (TRUE == operationCycleIsStarted(eventParam->EventClass->AgingCycleRef)) ) {
        eventStatusRecPtr->agingCounter = 0;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1) && defined(DEM_CLEAR_COMBINED_AGING_COUNTERS_ON_FAIL)
        clearCombinedEventAgingCounters(eventParam);
#endif
    }
#elif defined(DEM_AGING_COUNTER_CLEAR_ON_ALL_FAIL)
    (void)eventParam;/*lint !e920*/
    eventStatusRecPtr->agingCounter = 0;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1) && defined(DEM_CLEAR_COMBINED_AGING_COUNTERS_ON_FAIL)
        clearCombinedEventAgingCounters(eventParam);
#endif
#else
#error "Dem: Unknown aging counter clear behavior"
#endif
}
#endif

#if defined(DEM_USE_IUMPR)
/**
 * Increments IUMPR numerator of a specific component according to legislation
 * @param RatioID
 */
static Std_ReturnType incrementIumprNumerator(Dem_RatioIdType ratioId) {
	Std_ReturnType ret = E_NOT_OK;

	if (ratioId < DEM_IUMPR_REGISTERED_COUNT) { // is valid ID
		if (FALSE ==  iumprBufferLocal[ratioId].numerator.incrementedThisDrivingCycle) { // not incremented this driving cycle
			if (TRUE == operationCycleIsStarted(DEM_OBD_DCY)) {  /* IMPROVEMENT */
				uint16 boundEventId = Dem_RatiosList[ratioId].DiagnosticEventRef->EventID;
				EventStatusRecType* boundEvent;
				lookupEventStatusRec(boundEventId, &boundEvent);
				Dem_EventStatusExtendedType boundEventStatus = GET_STORED_STATUS_BITS(boundEvent->eventStatusExtended);

				/* @req DEM299 */
				if (0u == (boundEventStatus & DEM_PENDING_DTC)) { // not pending event
					// (C) If either the numerator or denominator for a specific component reaches
					// the maximum value of 65,535 ±2, both numbers shall be divided by two
					// before either is incremented again to avoid overflow problems.
					if (iumprBufferLocal[ratioId].numerator.value == 65535) {
						iumprBufferLocal[ratioId].numerator.value = 32767;

						iumprBufferLocal[ratioId].denominator.value = iumprBufferLocal[ratioId].denominator.value / 2;
					}

					iumprBufferLocal[ratioId].numerator.value++;

					iumprBufferLocal[ratioId].numerator.incrementedThisDrivingCycle = TRUE;

					ret = E_OK;
				}
			}
		}
	}

	return ret;
}

/**
 * Increments IUMPR denominator of a specific component according to legislation
 * @param RatioID
 */
static Std_ReturnType incrementIumprDenominator(Dem_RatioIdType ratioId) {
	Std_ReturnType ret = E_NOT_OK;

	if (ratioId < DEM_IUMPR_REGISTERED_COUNT) { // is valid ID
		if (FALSE == iumprBufferLocal[ratioId].denominator.isLocked) { // if denominator is NOT locked
			if (FALSE == iumprBufferLocal[ratioId].denominator.incrementedThisDrivingCycle) { // not incremented this driving cycle
				if (TRUE == operationCycleIsStarted(DEM_OBD_DCY)) {  /* IMPROVEMENT */
					uint16 boundEventId = Dem_RatiosList[ratioId].DiagnosticEventRef->EventID;
					EventStatusRecType* boundEvent;
					lookupEventStatusRec(boundEventId, &boundEvent);
					Dem_EventStatusExtendedType boundEventStatus = GET_STORED_STATUS_BITS(boundEvent->eventStatusExtended);

					/* @req DEM299 */
					if (0u == (boundEventStatus & DEM_PENDING_DTC)) { // not pending event
						// (C) If either the numerator or denominator for a specific component reaches
						// the maximum value of 65,535 ±2, both numbers shall be divided by two
						// before either is incremented again to avoid overflow problems.
						if (iumprBufferLocal[ratioId].denominator.value == 65535) {
							iumprBufferLocal[ratioId].denominator.value = 32767;

							iumprBufferLocal[ratioId].numerator.value = iumprBufferLocal[ratioId].numerator.value / 2;
						}

						iumprBufferLocal[ratioId].denominator.value++;

						iumprBufferLocal[ratioId].denominator.incrementedThisDrivingCycle = TRUE;

						ret = E_OK;
					}
				}
			}
		}
	}


	return ret;
}

/**
 * Resets the incrementedThisDrivingCycle flag to FALSE of all numerators and denominators in IUMPR buffer,
 * as well as general denominator, if current operating cycle is the bound driving cycle.
 *
 * @param operationCycleId
 */
static void resetIumprFlags(Dem_OperationCycleIdType operationCycleId) {
	if (operationCycleId == DEM_OBD_DCY) {
		for (Dem_RatioIdType i = 0; i < DEM_IUMPR_REGISTERED_COUNT; i++) {
			iumprBufferLocal[i].numerator.incrementedThisDrivingCycle = FALSE;
			iumprBufferLocal[i].denominator.incrementedThisDrivingCycle = FALSE;
		}

		// reset general denominator flags and status
		generalDenominatorBuffer.incrementedThisDrivingCycle = FALSE;
		(void) Dem_SetIUMPRDenCondition(DEM_IUMPR_GENERAL_OBDCOND, DEM_IUMPR_DEN_STATUS_NOT_REACHED);
	}
}

/**
 * Increments all unlocked denominators in IUMPR buffer,
 * if current operating cycle is the bound driving cycle.
 * @param operationCycleId
 */
static void incrementUnlockedIumprDenominators(Dem_OperationCycleIdType operationCycleId) {
	if (operationCycleId == DEM_OBD_DCY) {
		for (Dem_RatioIdType i = 0; i < DEM_IUMPR_REGISTERED_COUNT; i++) {
			(void) incrementIumprDenominator(i);
		}
	}
}

/**
 * Increments observer numerator in IUMPR buffer,
 * if its bound event is set to pass or failed, must be qualified.
 */
static void incrementObserverIumprNumerator(Dem_EventIdType eventId, Dem_EventStatusType eventStatus) {
	/* @req DEM359 */
	// find the ratio that has the given event bound to it
	if (eventStatus == DEM_EVENT_STATUS_FAILED || eventStatus == DEM_EVENT_STATUS_PASSED) {
		for (uint16 ratioId = 0; ratioId < DEM_IUMPR_REGISTERED_COUNT; ratioId++) {
			if (Dem_RatiosList[ratioId].RatioKind == DEM_RATIO_OBSERVER && eventId == Dem_RatiosList[ratioId].DiagnosticEventRef->EventID) {
				(void) incrementIumprNumerator(ratioId);
			}
		}
	}
}

/**
 * Initialise IUMPR additional denominator conditions buffer
 */
static void initIumprAddiDenomCondBuffer() {
	iumprAddiDenomCondBuffer[0].condition = DEM_IUMPR_DEN_COND_COLDSTART;
	iumprAddiDenomCondBuffer[1].condition = DEM_IUMPR_DEN_COND_EVAP;
	iumprAddiDenomCondBuffer[2].condition = DEM_IUMPR_DEN_COND_500MI;
	iumprAddiDenomCondBuffer[3].condition = DEM_IUMPR_GENERAL_OBDCOND;

	for (uint8 i = 0; i < DEM_IUMPR_ADDITIONAL_DENOMINATORS_COUNT; i++) {
		iumprAddiDenomCondBuffer[i].status = DEM_IUMPR_DEN_STATUS_NOT_REACHED;
	}
}

/**
 * Increment ignition cycle counter at the start of ignition cycle
 */
static void incrementIgnitionCycleCounter(Dem_OperationCycleIdType operationCycleId) {
	// If the ignition cycle counter reaches the maximum value of 65,535 ±2, the
	// ignition cycle counter shall rollover and increment to zero on the next
	// ignition cycle to avoid overflow problems.
	if (operationCycleId == DEM_IGNITION) {
		if (ignitionCycleCountBuffer == 65535) {
			ignitionCycleCountBuffer = 0;
		} else  {
			ignitionCycleCountBuffer++;
		}
	}
}
#endif

/**
 * Performs event updates when event is qualified as FAILED
 * @param eventParam
 * @param eventStatusRecPtr
 */
static inline void updateEventOnFAILED(const Dem_EventParameterType *eventParam, EventStatusRecType *eventStatusRecPtr) {
    if (0 == (eventStatusRecPtr->eventStatusExtended & DEM_TEST_FAILED)) {
        if( eventStatusRecPtr->occurrence < DEM_OCCURENCE_COUNTER_MAX ) {
            eventStatusRecPtr->occurrence++;/* @req DEM523 *//* @req DEM524 *//* !req DEM625 */
        }
        eventStatusRecPtr->errorStatusChanged = TRUE;
    }
#if defined(DEM_AGING_PROCESSING_DEM_INTERNAL)
    if( TRUE == operationCycleIsStarted(eventParam->EventClass->AgingCycleRef) ) {
        eventStatusRecPtr->failedDuringAgingCycle = TRUE;
    }

    handleAgingCounterOnFailed(eventParam, eventStatusRecPtr);
#endif
#if defined(DEM_FAILURE_PROCESSING_DEM_INTERNAL)
    /* Handle fault confirmation *//** @req DEM379.ConfirmedSet */
    handleFaultConfirmation(eventParam, eventStatusRecPtr);
#endif
    /** @req DEM036 */ /** @req DEM379.PendingSet */
    eventStatusRecPtr->eventStatusExtended |= (DEM_TEST_FAILED | DEM_TEST_FAILED_THIS_OPERATION_CYCLE | DEM_TEST_FAILED_SINCE_LAST_CLEAR | DEM_PENDING_DTC);
    eventStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)~(DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR | DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE);
}

/**
 * Performs event updates when event is qualified as PASSED
 * @param eventParam
 * @param eventStatusRecPtr
 */
static inline void updateEventOnPASSED(const Dem_EventParameterType *eventParam, EventStatusRecType *eventStatusRecPtr) {
    if ( 0 != (eventStatusRecPtr->eventStatusExtended & DEM_TEST_FAILED) ) {
        eventStatusRecPtr->errorStatusChanged = TRUE;
    }
    /** @req DEM036 */
    eventStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)~DEM_TEST_FAILED;
    eventStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)~(DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR | DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE);
#if defined(DEM_AGING_PROCESSING_DEM_INTERNAL)
    if( TRUE == operationCycleIsStarted(eventParam->EventClass->AgingCycleRef) ) {
        eventStatusRecPtr->passedDuringAgingCycle = TRUE;
    }
#endif
#if defined(DEM_FAILURE_PROCESSING_DEM_INTERNAL)
    if( TRUE == operationCycleIsStarted((Dem_OperationCycleIdType)*(eventParam->EventClass->FailureCycleRef))) {
        eventStatusRecPtr->passedDuringFailureCycle = TRUE;
    }
#endif
}
/*
 * Procedure:   updateEventStatusRec
 * Description: Update the status of "eventId"
 */
static void updateEventStatusRec(const Dem_EventParameterType *eventParam, Dem_EventStatusType reportedEventStatus, EventStatusRecType *eventStatusRecPtr)
{
    /* IMPROVEMENT: !req DEM544 */
    Dem_EventStatusType eventStatus = reportedEventStatus;

    if (eventStatusRecPtr != NULL) {

        eventStatus = RunPredebounce(reportedEventStatus, eventStatusRecPtr, eventParam);
        eventStatusRecPtr->errorStatusChanged = FALSE;
        eventStatusRecPtr->extensionDataChanged = FALSE;
        eventStatusRecPtr->indicatorDataChanged = FALSE;
        eventStatusRecPtr->extensionDataStoreBitfield = 0;

#if defined(USE_DEM_EXTENSION)
        Dem_EventStatusExtendedType eventStatusExtendedBeforeUpdate = eventStatusRecPtr->eventStatusExtended;
#endif

        switch(eventStatus) {
            case DEM_EVENT_STATUS_FAILED:
                updateEventOnFAILED(eventParam, eventStatusRecPtr);
                break;
            case DEM_EVENT_STATUS_PASSED:
                updateEventOnPASSED(eventParam, eventStatusRecPtr);
                break;
            default:
                break;
        }

#if defined(DEM_USE_IUMPR)
        incrementObserverIumprNumerator(eventParam->EventID, eventStatus);
#endif

#if defined(DEM_USE_INDICATORS)
        /** @req DEM379.WarningIndicatorSet */
        if(TRUE == handleIndicators(eventParam, eventStatusRecPtr, eventStatus)) {
            eventStatusRecPtr->indicatorDataChanged = TRUE;
        }
#endif

#if defined(USE_DEM_EXTENSION)
        Dem_Extension_UpdateEventstatus(eventStatusRecPtr, eventStatusExtendedBeforeUpdate, eventStatus);
#endif

        eventStatusRecPtr->maxUDSFdc = MAX(eventStatusRecPtr->maxUDSFdc, eventStatusRecPtr->UDSFdc);
#if (DEM_USE_TIMESTAMPS == STD_ON) && defined (DEM_USE_MEMORY_FUNCTIONS)
        if( (TRUE == eventStatusRecPtr->errorStatusChanged) && (eventStatus == DEM_EVENT_STATUS_FAILED) ) {
            /* Test just failed. Need to set timestamp */
            setEventTimeStamp(eventStatusRecPtr);
        }
#endif
    }
}


/*
 * Procedure:   mergeEventStatusRec
 * Description: Update the occurrence counter of status, if not exist a new record is created
 */
#ifdef DEM_USE_MEMORY_FUNCTIONS
static boolean mergeEventStatusRec(const EventRecType *eventRec)
{
    EventStatusRecType *eventStatusRecPtr;
    const Dem_EventParameterType *eventParam;
    boolean statusChanged = FALSE;

    // Lookup event ID
    lookupEventStatusRec(eventRec->EventData.eventId, &eventStatusRecPtr);
    lookupEventIdParameter(eventRec->EventData.eventId, &eventParam);

    if (eventStatusRecPtr != NULL) {
        // Update occurrence counter.
        eventStatusRecPtr->occurrence += eventRec->EventData.occurrence;
        // Merge event status extended with stored
        // TEST_FAILED_SINCE_LAST_CLEAR should be set if set if set in either
        eventStatusRecPtr->eventStatusExtended |= (Dem_EventStatusExtendedType)(eventRec->EventData.eventStatusExtended & DEM_TEST_FAILED_SINCE_LAST_CLEAR);
        // DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR should cleared if cleared in either
        if((eventRec->EventData.eventStatusExtended & eventStatusRecPtr->eventStatusExtended & DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR) == 0u) {
            eventStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)~(Dem_EventStatusExtendedType)DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR;
        }

        // DEM_PENDING_DTC and DEM_CONFIRMED_DTC should be set if set in either
        eventStatusRecPtr->eventStatusExtended |= (Dem_EventStatusExtendedType)(eventRec->EventData.eventStatusExtended & (DEM_PENDING_DTC | DEM_CONFIRMED_DTC));
        // DEM_WARNING_INDICATOR_REQUESTED should be set criteria fulfilled
#if defined(DEM_USE_INDICATORS)
        if( TRUE == warningIndicatorOnCriteriaFulfilled(eventParam) ) {
            eventStatusRecPtr->eventStatusExtended |= DEM_WARNING_INDICATOR_REQUESTED;
        }
#endif
#if defined(DEM_FAILURE_PROCESSING_DEM_INTERNAL)
        // Update confirmation counter
        if( (DEM_FAILURE_CNTR_MAX - eventRec->EventData.failureCounter) < eventStatusRecPtr->failureCounter) {
            /* Would overflow */
            eventStatusRecPtr->failureCounter = DEM_FAILURE_CNTR_MAX;
        } else {
            eventStatusRecPtr->failureCounter += eventRec->EventData.failureCounter;
        }

        if( (NULL != eventParam) && (TRUE == faultConfirmationCriteriaFulfilled(eventParam, eventStatusRecPtr)) ) {
            eventStatusRecPtr->eventStatusExtended |= (Dem_EventStatusExtendedType)DEM_CONFIRMED_DTC;
        }
#endif
#if defined(DEM_AGING_PROCESSING_DEM_INTERNAL)
        // Update confirmation counter
        if( (DEM_AGING_CNTR_MAX - eventRec->EventData.agingCounter) < eventStatusRecPtr->agingCounter) {
            /* Would overflow */
            eventStatusRecPtr->agingCounter = DEM_AGING_CNTR_MAX;
        } else {
            eventStatusRecPtr->agingCounter += eventRec->EventData.agingCounter;
        }

#endif
#if (DEM_TEST_FAILED_STORAGE == STD_ON)
        /* @req DEM387 */
        /* @req DEM388 */
        /* @req DEM525 */
        if( 0 != (eventStatusRecPtr->eventStatusExtended & DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE) ) {
            /* Test has not been completed this operation cycle. Set test failed bit as in stored */
            eventStatusRecPtr->eventStatusExtended |= (eventRec->EventData.eventStatusExtended & DEM_TEST_FAILED);
        }
#endif

#if (DEM_USE_TIMESTAMPS == STD_ON)
        if( 0u == (eventStatusRecPtr->eventStatusExtended & DEM_TEST_FAILED_THIS_OPERATION_CYCLE) ) {
            /* Test has not failed this operation cycle. Means that the that the timestamp
             * should be set to the one read from NvRam */
            eventStatusRecPtr->timeStamp = eventRec->EventData.timeStamp;
        }
#endif

        if( (eventStatusRecPtr->occurrence != eventRec->EventData.occurrence) ||
#if defined(DEM_FAILURE_PROCESSING_DEM_INTERNAL)
            (eventStatusRecPtr->failureCounter != eventRec->EventData.failureCounter) ||
#endif
            (GET_STORED_STATUS_BITS(eventStatusRecPtr->eventStatusExtended) != GET_STORED_STATUS_BITS(eventRec->EventData.eventStatusExtended)) ) {
            statusChanged = TRUE;
        }

        if( 0 == (eventStatusRecPtr->eventStatusExtended & DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE) ) {
            /* Test was completed during preInit, means that the eventStatus was changed in some way */
            Dem_EventStatusExtendedType oldStatus = (Dem_EventStatusExtendedType)(GET_STORED_STATUS_BITS(eventRec->EventData.eventStatusExtended));
            notifyEventStatusChange(eventStatusRecPtr->eventParamRef, oldStatus, eventStatusRecPtr->eventStatusExtended);
        }
    }

    return statusChanged;
}

/*
 * Procedure:   resetEventStatusRec
 * Description: Reset the status record of "eventParam->eventId" from "eventStatusBuffer".
 */
static void resetEventStatusRec(const Dem_EventParameterType *eventParam)
{
    EventStatusRecType *eventStatusRecPtr;

    // Lookup event ID
    lookupEventStatusRec(eventParam->EventID, &eventStatusRecPtr);

    if (eventStatusRecPtr != NULL) {
        // Reset event record
        resetDebounceCounter(eventStatusRecPtr);
        eventStatusRecPtr->eventStatusExtended = DEM_DEFAULT_EVENT_STATUS;/** @req DEM385 *//** @req DEM440 */
        eventStatusRecPtr->errorStatusChanged = FALSE;
        eventStatusRecPtr->occurrence = 0;
#if defined(DEM_FAILURE_PROCESSING_DEM_INTERNAL)
        eventStatusRecPtr->failureCounter = 0;
        eventStatusRecPtr->failedDuringFailureCycle = FALSE;
        eventStatusRecPtr->passedDuringFailureCycle = FALSE;
#endif
#if defined(DEM_AGING_PROCESSING_DEM_INTERNAL)
        eventStatusRecPtr->agingCounter = 0;
        eventStatusRecPtr->failedDuringAgingCycle = FALSE;
        eventStatusRecPtr->passedDuringAgingCycle = FALSE;
#endif
        eventStatusRecPtr->timeStamp = 0;
    }

}
#endif
/*
 * Procedure:   getEventStatusRec
 * Description: Returns the status record of "eventId" in "eventStatusRec"
 */
static void getEventStatusRec(Dem_EventIdType eventId, EventStatusRecType *eventStatusRec)
{
    EventStatusRecType *eventStatusRecPtr;

    // Lookup event ID
    lookupEventStatusRec(eventId, &eventStatusRecPtr);

    if (eventStatusRecPtr != NULL) {
        // Copy the record
        memcpy(eventStatusRec, eventStatusRecPtr, sizeof(EventStatusRecType));
    }
    else {
        eventStatusRec->eventId = DEM_EVENT_ID_NULL;
    }
}

/**
 * Sets overflow indication for a specific memory
 * @param origin
 * @param overflow
 */
#ifdef DEM_USE_MEMORY_FUNCTIONS
static void setOverflowIndication(Dem_DTCOriginType origin, boolean overflow)
{
    switch (origin) {
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
            priMemOverflow = overflow;
            if(overflow != priMemEventBuffer[PRI_MEM_EVENT_BUFFER_ADMIN_INDEX].AdminData.overflow) {
                priMemEventBuffer[PRI_MEM_EVENT_BUFFER_ADMIN_INDEX].AdminData.magic = ADMIN_MAGIC;
                priMemEventBuffer[PRI_MEM_EVENT_BUFFER_ADMIN_INDEX].AdminData.overflow = overflow;
                /* Overflow not stored immediately */
                Dem_NvM_SetEventBlockChanged(origin, FALSE);
            }
#endif
            break;
        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
            secMemOverflow = overflow;
            if( overflow != secMemEventBuffer[SEC_MEM_EVENT_BUFFER_ADMIN_INDEX].AdminData.overflow ) {
                secMemEventBuffer[SEC_MEM_EVENT_BUFFER_ADMIN_INDEX].AdminData.magic = ADMIN_MAGIC;
                secMemEventBuffer[SEC_MEM_EVENT_BUFFER_ADMIN_INDEX].AdminData.overflow = overflow;
                /* Overflow not stored immediately */
                Dem_NvM_SetEventBlockChanged(origin, FALSE);
            }
#endif
            break;
        case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
        case DEM_DTC_ORIGIN_MIRROR_MEMORY:
            // Not yet supported
            DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GLOBAL_ID, DEM_E_NOT_IMPLEMENTED_YET);
            break;
        default:
            break;
    }
}
#endif

/**
 * Returns the overflow indication for a specific memory
 * @param origin
 * @return E_OK: Operation successful, E_NOT_OK: Operation failed
 */
static Std_ReturnType getOverflowIndication(Dem_DTCOriginType origin, boolean *Overflow)
{
    Std_ReturnType ret = E_OK;
    switch (origin) {
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
            *Overflow = priMemOverflow;
#endif
            break;
        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
            *Overflow = secMemOverflow;
#endif
            break;
        case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
        case DEM_DTC_ORIGIN_MIRROR_MEMORY:
        default:
            /* Not yet supported */
            ret = E_NOT_OK;
            DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GLOBAL_ID, DEM_E_NOT_IMPLEMENTED_YET);
            break;
    }
    return ret;
}

/**
 * Returns the occurence counter
 * @param eventParameter
 * @return
 */
static uint16 getEventOccurence(const Dem_EventParameterType *eventParameter)
{
    EventStatusRecType *eventStatusRec = NULL_PTR;
    uint16 occurence = 0u;
    if( NULL != eventParameter ) {
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
        if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParameter->CombinedDTCCID ) {
            const Dem_CombinedDTCCfgType *CombDTCCfg = &configSet->CombinedDTCConfig[eventParameter->CombinedDTCCID];
            CombDTCCfg = &configSet->CombinedDTCConfig[eventParameter->CombinedDTCCID];
            for(uint16 evIdx = 0; evIdx < CombDTCCfg->DTCClassRef->NofEvents; evIdx++) {
                eventStatusRec = NULL_PTR;
                lookupEventStatusRec(CombDTCCfg->DTCClassRef->Events[evIdx], &eventStatusRec);
                if( (NULL_PTR != eventStatusRec) && (eventStatusRec->occurrence > occurence) ) {
                    occurence = eventStatusRec->occurrence;
                }
            }
        }
        else {
            lookupEventStatusRec(eventParameter->EventID, &eventStatusRec);
            if( NULL_PTR != eventStatusRec ) {
                occurence = eventStatusRec->occurrence;
            }
        }
#else
        lookupEventStatusRec(eventParameter->EventID, &eventStatusRec);
        if( NULL_PTR != eventStatusRec ) {
            occurence = eventStatusRec->occurrence;
        }
#endif
    }
    return occurence;
}

static sint8 getEventFDC(const Dem_EventParameterType *eventParameter, boolean maxFDC)
{
    EventStatusRecType *eventStatusRec = NULL_PTR;
    sint8 FDC = 0;
    if( NULL != eventParameter ) {
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
        if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParameter->CombinedDTCCID ) {
            const Dem_CombinedDTCCfgType *CombDTCCfg = &configSet->CombinedDTCConfig[eventParameter->CombinedDTCCID];
            sint8 tempFDC;
            for(uint16 evIdx = 0; evIdx < CombDTCCfg->DTCClassRef->NofEvents; evIdx++) {
                eventStatusRec = NULL_PTR;
                lookupEventStatusRec(CombDTCCfg->DTCClassRef->Events[evIdx], &eventStatusRec);
                if( NULL_PTR != eventStatusRec ) {
                    tempFDC = (TRUE == maxFDC) ? eventStatusRec->maxUDSFdc : eventStatusRec->UDSFdc;
                    if( tempFDC > FDC ) {
                        FDC = tempFDC;
                    }
                }
            }
        }
        else {
            lookupEventStatusRec(eventParameter->EventID, &eventStatusRec);
            if( NULL_PTR != eventStatusRec ) {
                FDC = (TRUE == maxFDC) ? eventStatusRec->maxUDSFdc : eventStatusRec->UDSFdc;
            }
        }
#else
        lookupEventStatusRec(eventParameter->EventID, &eventStatusRec);
        if( NULL_PTR != eventStatusRec ) {
            FDC = (TRUE == maxFDC) ? eventStatusRec->maxUDSFdc : eventStatusRec->UDSFdc;
        }
#endif
    }
    return FDC;
}

#if defined(DEM_AGING_PROCESSING_DEM_INTERNAL)
/**
 * Gets the aging counter
 * @param eventParameter
 * @return
 */
uint8 getEventAgingCntr(const Dem_EventParameterType *eventParameter)
{
    /* IMPROVEMENT: External aging.. */
    EventStatusRecType *eventStatusRec = NULL_PTR;
    uint8 agingCnt = 0u;
    if( NULL != eventParameter ) {
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
        if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParameter->CombinedDTCCID ) {
            const Dem_CombinedDTCCfgType *CombDTCCfg = &configSet->CombinedDTCConfig[eventParameter->CombinedDTCCID];
            agingCnt = 0xFFu;
            boolean cntrValid = FALSE;
            for(uint16 evIdx = 0; evIdx < CombDTCCfg->DTCClassRef->NofEvents; evIdx++) {
                eventStatusRec = NULL_PTR;
                lookupEventStatusRec(CombDTCCfg->DTCClassRef->Events[evIdx], &eventStatusRec);
                if( (NULL_PTR != eventStatusRec) && (TRUE == eventStatusRec->eventParamRef->EventClass->AgingAllowed) && (0u != (eventStatusRec->eventStatusExtended & DEM_CONFIRMED_DTC)) ) {
                    if( eventStatusRec->agingCounter < agingCnt ) {
                        agingCnt = eventStatusRec->agingCounter;
                    }
                    cntrValid = TRUE;
                }
            }
            if( FALSE == cntrValid ) {
                /* @req DEM646 */
                agingCnt = 0u;
            }
        }
        else {
            lookupEventStatusRec(eventParameter->EventID, &eventStatusRec);
            if( (NULL_PTR != eventStatusRec) && (TRUE == eventParameter->EventClass->AgingAllowed) ) {
                agingCnt = eventStatusRec->agingCounter;
            }
        }
#else
        lookupEventStatusRec(eventParameter->EventID, &eventStatusRec);
        if( (NULL_PTR != eventStatusRec) && (TRUE == eventParameter->EventClass->AgingAllowed) ) {
            agingCnt = eventStatusRec->agingCounter;
        }
#endif
    }
    return agingCnt;
}
#endif

#if defined(DEM_FAILURE_PROCESSING_DEM_INTERNAL)
/**
 * Gets the failure counter
 * @param eventParameter
 * @return
 */
static uint8 getEventFailureCnt(const Dem_EventParameterType *eventParameter)
{
    EventStatusRecType *eventStatusRec = NULL_PTR;
    uint8 failCnt = 0u;
    if( NULL != eventParameter ) {
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
        if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParameter->CombinedDTCCID ) {
            const Dem_CombinedDTCCfgType *CombDTCCfg = &configSet->CombinedDTCConfig[eventParameter->CombinedDTCCID];
            for(uint16 evIdx = 0; evIdx < CombDTCCfg->DTCClassRef->NofEvents; evIdx++) {
                eventStatusRec = NULL_PTR;
                lookupEventStatusRec(CombDTCCfg->DTCClassRef->Events[evIdx], &eventStatusRec);
                if( (NULL_PTR != eventStatusRec) && (eventStatusRec->failureCounter > failCnt) ) {
                    failCnt = eventStatusRec->failureCounter;
                }
            }
        }
        else {
            lookupEventStatusRec(eventParameter->EventID, &eventStatusRec);
            if( NULL_PTR != eventStatusRec ) {
                failCnt = eventStatusRec->failureCounter;
            }
        }
#else
        lookupEventStatusRec(eventParameter->EventID, &eventStatusRec);
        if( NULL_PTR != eventStatusRec ) {
            failCnt = eventStatusRec->failureCounter;
        }
#endif
    }
    return failCnt;
}
#endif

/**
 * Reads internal element
 * @param eventParameter
 * @param elementType
 * @param buf
 * @param size
 */
static void getInternalElement( const Dem_EventParameterType *eventParameter, Dem_InternalDataElementType elementType, uint8* buf, uint16 size )
{
    /* !req DEM592 *//* SIGNIFICANCE not supported */
    EventStatusRecType *eventStatusRec;
    lookupEventStatusRec(eventParameter->EventID, &eventStatusRec);
    uint16 occurrence;

    if( (DEM_EVENT_ID_NULL != eventStatusRec->eventId) && (size > 0) ) {
        memset(buf, 0, (size_t)size);
        switch(elementType) {
            case DEM_OCCCTR:
                /* @req DEM471 */
                occurrence = getEventOccurence(eventParameter);
                if(1 == size) {
                    buf[0] = (uint8)MIN(occurrence, 0xFF);
                } else  {
                    buf[size - 2] = (uint8)((occurrence & 0xff00u) >> 8u);
                    buf[size - 1] = (uint8)(occurrence & 0xffu);
                }
                break;
            case DEM_FAULTDETCTR:
                /* @req OEM_DEM_10185 */
                buf[size - 1] = (uint8)getEventFDC(eventParameter, FALSE);
                break;
            case DEM_MAXFAULTDETCTR:
                buf[size - 1] = (uint8)getEventFDC(eventParameter, TRUE);
                break;
            case DEM_OVFLIND:
            {
                /* @req DEM473 */
                boolean ovflw = FALSE;
                if( E_OK == getOverflowIndication(eventParameter->EventClass->EventDestination, &ovflw) ) {
                    buf[size - 1] = (TRUE == ovflw) ? 1 : 0;
                } else {
                    buf[size - 1] = 0;
                }
            }
                break;
#if defined(DEM_AGING_PROCESSING_DEM_INTERNAL)
            case DEM_AGINGCTR:
                /* @req DEM472 *//* !req DEM644 *//* !req DEM647 */
                /* @req DEM646 */
                /* set to 0 by memset above */
                buf[size - 1] = getEventAgingCntr(eventParameter);
                break;
#endif
#if defined(DEM_FAILURE_PROCESSING_DEM_INTERNAL)
            case DEM_CONFIRMATIONCNTR:
                buf[size - 1] = getEventFailureCnt(eventParameter);
                break;
#endif
            default:
#if defined(USE_DEM_EXTENSION)
                Dem_Extension_GetExtendedDataInternalElement(eventParameter->EventID, elementType, buf, size);
#endif
                break;
        }
    }
}

#if defined(USE_DEM_EXTENSION)
/*
 * Procedure:   lookupDtcEvent
 * Description: Returns TRUE if the DTC was found and "eventStatusRec" points
 *              to the event record found.
 */
/* @req 4.2.2/DEM_00915 */
boolean Dem_LookupEventOfUdsDTC(uint32 dtc, EventStatusRecType **eventStatusRec)
{
    boolean dtcFound = FALSE;
    uint16 i;

    *eventStatusRec = NULL;

    for (i = 0; (i < DEM_MAX_NUMBER_EVENT) && (dtcFound == FALSE); i++) {
        if (eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL) {
            if (eventStatusBuffer[i].eventParamRef->DTCClassRef != NULL) {

                /* Check DTC. Ignore suppressed DTCs *//* @req DEM587 */
                if ((eventStatusBuffer[i].eventParamRef->DTCClassRef->DTCRef->UDSDTC == dtc) &&
                        (DTCIsAvailable(eventStatusBuffer[i].eventParamRef->DTCClassRef))) {
                    *eventStatusRec = &eventStatusBuffer[i];
                    dtcFound = TRUE;
                }
            }
        }
    }

    return dtcFound;
}
#endif

/**
 * Function for finding configuration of UDS DTC.
 * @param dtc
 * @param DTCClass
 * @return
 */
static boolean LookupUdsDTC(uint32 dtc, const Dem_DTCClassType **DTCClass)
{
    boolean DTCFound = FALSE;
    const Dem_DTCClassType *DTCClassPtr = configSet->DTCClass;
    while( FALSE == DTCClassPtr->Arc_EOL ) {
        if( (DTCClassPtr->DTCRef->UDSDTC == dtc) ) {
            *DTCClass = DTCClassPtr;
            DTCFound = DTCIsAvailable(DTCClassPtr);
            break;
        }
        DTCClassPtr++;
    }
    return DTCFound;
}

/**
 * Gets the UDS status of a DTC in a specific origin.
 * @param DTCClass
 * @param DTCOrigin
 * @param status
 * @return
 */
static Dem_ReturnGetStatusOfDTCType GetDTCUDSStatus(const Dem_DTCClassType *DTCClass, Dem_DTCOriginType DTCOrigin, Dem_EventStatusExtendedType *status)
{
    Dem_ReturnGetStatusOfDTCType ret = DEM_STATUS_OK;
    EventStatusRecType *eventStatusRecPtr;
    uint8 mask = 0xFFU;
    uint16 nofEventsInOrigin = 0u;
    Dem_EventStatusExtendedType temp = 0u;

    for(uint16 i = 0; (i < DTCClass->NofEvents) && (DEM_STATUS_OK == ret); i++) {
        eventStatusRecPtr = NULL_PTR;
        lookupEventStatusRec(DTCClass->Events[i], &eventStatusRecPtr);
        if( NULL_PTR != eventStatusRecPtr ) {
            /* Event found for this DTC */
            if( TRUE == checkDtcOrigin(DTCOrigin,eventStatusRecPtr->eventParamRef, TRUE) ) {
                /* NOTE: Should the availability mask be used here? */
                /* @req DEM059 */
                /* @req DEM441 */
                if( TRUE == eventStatusRecPtr->isAvailable ) {
                    temp |= eventStatusRecPtr->eventStatusExtended;
                    nofEventsInOrigin++;
                }
            }
            else {
                /* Event not available in DTCOrigin */
                /** @req DEM172 */
            }
        }
        else {
            /* This is unexpected. Fail operation. */
            ret = DEM_STATUS_FAILED;
        }
    }

    if( DEM_STATUS_OK == ret ) {
        if( 0u == nofEventsInOrigin ) {
            /* No events where found in the origin. */
            ret = DEM_STATUS_WRONG_DTCORIGIN;
        }
        else if( (DTCClass->NofEvents > 1u) && (0u != nofEventsInOrigin) ) {
            /* This is a combined DTC. Bits have already been OR-ed above. Now we should and bits. */
            /* @req DEM441 */
            mask = ((temp & (1u << 5u)) >> 1u) | ((temp & (1u << 1u)) << 5u);
            mask = (uint8)((~mask) & 0xFFu);
        }
        else {
            /* One event found. Do nomasking. */
        }
    }
    *status = (temp & mask);
    return ret;
}
/*
 * Procedure:   matchEventWithDtcFilter
 * Description: Returns TRUE if the event pointed by "event" fulfill
 *              the "dtcFilter" global filter settings.
 */
/**
 * Checks if a DTC matches the current filter settings. If it matches the UDS status is returned.
 * @param DTCClass
 * @param UDSStatus
 * @return
 */
static boolean matchDTCWithDtcFilter(const Dem_DTCClassType *DTCClass, Dem_EventStatusExtendedType *UDSStatus)
{
    boolean dtcMatch = FALSE;
    Dem_EventStatusExtendedType DTCStatus;

    if( DEM_STATUS_OK == GetDTCUDSStatus(DTCClass, dtcFilter.dtcOrigin, &DTCStatus) ) {
        /* Status available in origin. */
        if ( TRUE == checkDtcStatusMask(dtcFilter.dtcStatusMask, DTCStatus) ) {
            /* Check the DTC kind. */
            if( (dtcFilter.dtcKind == DEM_DTC_KIND_ALL_DTCS) || (DEM_NO_DTC != DTCClass->DTCRef->OBDDTC) ) {

                /* Check severity */
                if ((dtcFilter.filterWithSeverity == DEM_FILTER_WITH_SEVERITY_NO) ||
                    ((dtcFilter.filterWithSeverity == DEM_FILTER_WITH_SEVERITY_YES) && (TRUE == checkDtcSeverityMask(dtcFilter.dtcSeverityMask, DTCClass)))) {

                    /* Check fault detection counter. No support for DEM_FILTER_FOR_FDC_YES */
                    if( dtcFilter.filterForFaultDetectionCounter == DEM_FILTER_FOR_FDC_NO) {

                        /* Check the DTC */
                        if( (TRUE == DTCIsAvailable(DTCClass)) && (TRUE == DTCISAvailableOnFormat(DTCClass, dtcFilter.dtcFormat)) ) {
                            dtcMatch = TRUE;
                            *UDSStatus = DTCStatus;
                        }
                    }
                }
            }
        }
    }

    return dtcMatch;
}

/* Function: eventDTCRecordDataUpdateDisabled
 * Description: Checks if update of event related data (extended data or freezeframe data) has been disabled
 */
static boolean eventDTCRecordDataUpdateDisabled(const Dem_EventParameterType *eventParam)
{
    boolean disabled = FALSE;
    if( (NO_DTC_DISABLED != DTCRecordDisabled.DTC) && /* There is a disabled DTC */
            (NULL != eventParam) && /* Argument ok */
            (NULL != eventParam->DTCClassRef) && /* Event has a DTC */
            (DEM_NO_DTC != eventParam->DTCClassRef->DTCRef->UDSDTC) && /* And a DTC on UDS format */
            (DTCRecordDisabled.DTC == eventParam->DTCClassRef->DTCRef->UDSDTC) && /* The disabled DTC is the DTC of the event */
            (DTCRecordDisabled.Origin == eventParam->EventClass->EventDestination)) { /* The disabled origin is the origin of the event */
        /* DTC record update for this event is disabled */
        disabled = TRUE;
    }
    return disabled;
}
#if (DEM_USE_TIMESTAMPS == STD_ON)
#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM) || \
    ((DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_SEC_MEM) || \
    (DEM_FF_DATA_IN_PRE_INIT)
/*
 * Functions for rearranging timestamps
 *
 * */
/**
 * Sorts entries in freeze frame buffers. Oldest freeze frame first, etc. Sets a new timestamp (0-"nof ff entries")
 * Returns the timestamp to use for next stored FF.
 * @param timeStamp, pointer to timestamp used by caller. Shall be set to the next timestamp to use.
 */
#if (DEM_UNIT_TEST == STD_ON)
void rearrangeFreezeFrameTimeStamp(uint32 *timeStamp)
#else
static void rearrangeFreezeFrameTimeStamp(uint32 *timeStamp )
#endif
{
    FreezeFrameRecType temp;
    uint32 i;
    uint32 j = 0;
    uint32 k = 0;
    uint32 bufferIndex;

    /* These two arrays are looped below must have the same size */
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM && (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_SEC_MEM
    uint32 ffBufferSizes[2] = {DEM_MAX_NUMBER_FF_DATA_PRI_MEM, DEM_MAX_NUMBER_FF_DATA_SEC_MEM};
    FreezeFrameRecType* ffBuffers[2] = {priMemFreezeFrameBuffer, secMemFreezeFrameBuffer};
    uint32 nofSupportedDestinations = 2;
#elif (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON && DEM_FF_DATA_IN_PRI_MEM)
    uint32 ffBufferSizes[1] = {DEM_MAX_NUMBER_FF_DATA_PRI_MEM};
    FreezeFrameRecType* ffBuffers[1] = {priMemFreezeFrameBuffer};
    uint32 nofSupportedDestinations = 1;
#elif (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_SEC_MEM
    uint32 ffBufferSizes[1] = {DEM_MAX_NUMBER_FF_DATA_SEC_MEM};
    FreezeFrameRecType* ffBuffers[1] = {secMemFreezeFrameBuffer};
    uint32 nofSupportedDestinations = 1;
#else
    uint32 ffBufferSizes[1] = {0};
    FreezeFrameRecType* ffBuffers[1] = {NULL};
    uint32 nofSupportedDestinations = 0;
#endif

    for (bufferIndex = 0; bufferIndex < nofSupportedDestinations; bufferIndex++) {

        FreezeFrameRecType* ffBuffer = ffBuffers[bufferIndex];
        uint32 ffBufferSize = ffBufferSizes[bufferIndex];

        /* Bubble sort:rearrange ffBuffer from little to big */
        for(i = 0; i < ffBufferSize; i++){
            if(ffBuffer[i].eventId != DEM_EVENT_ID_NULL){
                for( j = ffBufferSize - 1; j > i; j--){
                    if(ffBuffer[j].eventId != DEM_EVENT_ID_NULL){
                        if(ffBuffer[i].timeStamp > ffBuffer[j].timeStamp){
                            //exchange buffer data
                            memcpy(&temp,&ffBuffer[i],sizeof(FreezeFrameRecType));
                            memcpy(&ffBuffer[i],&ffBuffer[j],sizeof(FreezeFrameRecType));
                            memcpy(&ffBuffer[j],&temp,sizeof(FreezeFrameRecType));
                        }

                    }

                }
                ffBuffer[i].timeStamp = k++;
            }
        }
    }

    /* update the current timeStamp */
    *timeStamp = k;

}
#endif
#if (DEM_EXT_DATA_IN_PRE_INIT || DEM_EXT_DATA_IN_PRI_MEM || DEM_EXT_DATA_IN_SEC_MEM ) && defined(DEM_USE_MEMORY_FUNCTIONS)
/**
 * Sorts entries in extended buffers. Oldest extended data first, etc. Sets a new timestamp (0-"nof ff entries")
 * Returns the timestamp to use for next stored extended data.
 * @param timeStamp, pointer to timestamp used by caller. Shall be set to the next timestamp to use.
 */
#if (DEM_UNIT_TEST == STD_ON)
void rearrangeExtDataTimeStamp(uint32 *timeStamp)
#else
static void rearrangeExtDataTimeStamp(uint32 *timeStamp)
#endif
{
    ExtDataRecType temp;
    uint32 i;
    uint32 j = 0;
    uint32 k = 0;
    uint32 bufferIndex;

    /* These two arrays are looped below must have the same size */
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_EXT_DATA_IN_PRI_MEM && (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON) && DEM_EXT_DATA_IN_SEC_MEM
    uint32 extBufferSizes[2] = {DEM_MAX_NUMBER_EXT_DATA_PRI_MEM, DEM_MAX_NUMBER_EXT_DATA_SEC_MEM};
    ExtDataRecType* extBuffers[2] = {priMemExtDataBuffer, secMemExtDataBuffer};
    uint32 nofDestinations = 2;
#elif (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON && DEM_EXT_DATA_IN_PRI_MEM)
    uint32 extBufferSizes[1] = {DEM_MAX_NUMBER_EXT_DATA_PRI_MEM};
    ExtDataRecType* extBuffers[1] = {priMemExtDataBuffer};
    uint32 nofDestinations = 1;
#elif (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON && DEM_EXT_DATA_IN_SEC_MEM)
    uint32 extBufferSizes[1] = {DEM_MAX_NUMBER_EXT_DATA_SEC_MEM};
    ExtDataRecType* extBuffers[1] = {secMemExtDataBuffer};
    uint32 nofDestinations = 1;
#else
    uint32 extBufferSizes[1] = {0};
    ExtDataRecType* extBuffers[1] = {NULL};
    uint32 nofDestinations = 0;
#endif

    for (bufferIndex = 0; bufferIndex < nofDestinations; bufferIndex++) {

        ExtDataRecType* extBuffer = extBuffers[bufferIndex];
        uint32 extBufferSize = extBufferSizes[bufferIndex];

        /* Bubble sort:rearrange Buffer from little to big */
        for( i = 0; i < extBufferSize; i++ ){
            if( DEM_EVENT_ID_NULL != extBuffer[i].eventId ){
                for( j = extBufferSize - 1; j > i; j-- ){
                    if( DEM_EVENT_ID_NULL != extBuffer[j].eventId ){
                        if( extBuffer[i].timeStamp > extBuffer[j].timeStamp ){
                            //exchange buffer data
                            memcpy(&temp, &extBuffer[i], sizeof(ExtDataRecType));
                            memcpy(&extBuffer[i], &extBuffer[j], sizeof(ExtDataRecType));
                            memcpy(&extBuffer[j], &temp, sizeof(ExtDataRecType));
                        }
                    }
                }
                extBuffer[i].timeStamp = k++;
            }
        }
    }

    /* update the current timeStamp */
    *timeStamp = k;
}
#endif

#if defined(DEM_USE_MEMORY_FUNCTIONS)
static void initCurrentEventTimeStamp(uint32 *timeStampPtr)
{
    uint32 highestTimeStamp = 0;

    /* Rearrange events */
    rearrangeEventTimeStamp(timeStampPtr);

    for (uint16 i = 0; i < DEM_MAX_NUMBER_EVENT; i++){
        if( DEM_EVENT_ID_NULL != eventStatusBuffer[i].eventId ){
            eventStatusBuffer[i].timeStamp += *timeStampPtr;
            if( eventStatusBuffer[i].timeStamp > highestTimeStamp ) {
                highestTimeStamp = eventStatusBuffer[i].timeStamp;
            }
        }
    }
    *timeStampPtr = highestTimeStamp + 1;
}

/*
 * Functions for initializing timestamps
 * */
static void initCurrentFreezeFrameTimeStamp(uint32 *timeStampPtr)
{
#if ( DEM_FF_DATA_IN_PRE_INIT )
    uint32 highestTimeStamp = 0;

    /* Rearrange freeze frames */
    rearrangeFreezeFrameTimeStamp(timeStampPtr);

    for (uint16 i = 0; i<DEM_MAX_NUMBER_FF_DATA_PRE_INIT; i++){
        if(preInitFreezeFrameBuffer[i].eventId != DEM_EVENT_ID_NULL){
            preInitFreezeFrameBuffer[i].timeStamp += *timeStampPtr;
            if( preInitFreezeFrameBuffer[i].timeStamp > highestTimeStamp ) {
                highestTimeStamp = preInitFreezeFrameBuffer[i].timeStamp;
            }
        }
    }
    *timeStampPtr = highestTimeStamp + 1;
#endif
}

static void initCurrentExtDataTimeStamp(uint32 *timeStampPtr)
{
#if ( DEM_EXT_DATA_IN_PRE_INIT && DEM_FF_DATA_IN_PRE_INIT )
    uint32 highestTimeStamp = 0;

    /* Rearrange extended data in primary memory */
    rearrangeExtDataTimeStamp(timeStampPtr);

    /* Increment the timestamps in the pre init ext data buffer */
    for (uint16 i = 0; i < DEM_MAX_NUMBER_EXT_DATA_PRE_INIT; i++){
        if( DEM_EVENT_ID_NULL != preInitExtDataBuffer[i].eventId ){
            preInitExtDataBuffer[i].timeStamp += *timeStampPtr;
            if( preInitExtDataBuffer[i].timeStamp > highestTimeStamp ) {
                highestTimeStamp = preInitExtDataBuffer[i].timeStamp;
            }
        }
    }
    *timeStampPtr = highestTimeStamp + 1;
#else
    (void)timeStampPtr;
#endif
}

#if (DEM_UNIT_TEST == STD_ON)
void rearrangeEventTimeStamp(uint32 *timeStamp)
#else
static void rearrangeEventTimeStamp(uint32 *timeStamp)
#endif
{
    FreezeFrameRecType temp;
    uint32 bufferIndex;
    uint32 i;
    uint32 j = 0;
    uint32 k = 0;

    /* These two arrays are looped below must have the same size */
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
    uint32 eventBufferSizes[2] = {DEM_MAX_NUMBER_EVENT_ENTRY_PRI, DEM_MAX_NUMBER_EVENT_ENTRY_SEC};
    EventRecType* eventBuffers[2] = {priMemEventBuffer, secMemEventBuffer};
    uint32 nofDestinations = 2;
#elif (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
    uint32 eventBufferSizes[1] = {DEM_MAX_NUMBER_EVENT_ENTRY_PRI};
    EventRecType* eventBuffers[1] = {priMemEventBuffer};
    uint32 nofDestinations = 1;
#elif (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
    uint32 eventBufferSizes[1] = {DEM_MAX_NUMBER_EVENT_ENTRY_SEC};
    EventRecType* eventBuffers[1] = {secMemEventBuffer};
    uint32 nofDestinations = 1;
#else
    uint32 eventBufferSizes[1] = {0};
    EventRecType* eventBuffers[1] = {NULL};
    uint32 nofDestinations = 0;
#endif

    for (bufferIndex = 0; bufferIndex < nofDestinations; bufferIndex++) {

        EventRecType* eventBuffer = eventBuffers[bufferIndex];
        uint32 eventBufferSize = eventBufferSizes[bufferIndex];

        /* Bubble sort:rearrange event buffer from little to big */
        for( i = 0; i < eventBufferSize; i++ ){
            if( DEM_EVENT_ID_NULL != eventBuffer[i].EventData.eventId ){
                for( j = (eventBufferSize - 1); j > i; j-- ) {
                    if( DEM_EVENT_ID_NULL != eventBuffer[j].EventData.eventId ) {
                        if( eventBuffer[i].EventData.timeStamp > eventBuffer[j].EventData.timeStamp ) {
                            //exchange buffer data
                            memcpy(&temp, &eventBuffer[i].EventData, sizeof(EventRecType));
                            memcpy(&eventBuffer[i].EventData, &eventBuffer[j].EventData, sizeof(EventRecType));
                            memcpy(&eventBuffer[j].EventData, &temp, sizeof(EventRecType));
                        }
                    }
                }
                eventBuffer[i].EventData.timeStamp = k++;
            }
        }
    }

    /* update the current timeStamp */
    *timeStamp = k;
}
#endif /* DEM_USE_MEMORY_FUNCTIONS */
#endif /* (DEM_USE_TIMESTAMPS == STD_ON) */



#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && defined (DEM_USE_MEMORY_FUNCTIONS)
#if defined(DEM_DISPLACEMENT_PROCESSING_DEM_INTERNAL)
#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM && (DEM_OBD_DISPLACEMENT_SUPPORT == STD_ON))
/**
 * Check if an event currently hold the OBD FF
 * @param eventParam
 * @return TRUE: Event holds OBD FF. FALSE: event does NOT hold the OBD FF
 */
static boolean eventHoldsOBDFF(const Dem_EventParameterType *eventParam)
{
    boolean ret = FALSE;
    Dem_EventIdType idToFind;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
    if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID ) {
        idToFind = TO_COMBINED_EVENT_ID(eventParam->CombinedDTCCID);
    }
    else {
        idToFind = eventParam->EventID;
    }
#else
    idToFind = eventParam->EventID;
#endif
    for( uint32 i = 0; (i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM) && (FALSE == ret); i++ ) {
        if( (idToFind == priMemFreezeFrameBuffer[i].eventId) && (DEM_FREEZE_FRAME_OBD == priMemFreezeFrameBuffer[i].kind) ) {
            ret = TRUE;
        }
    }
    return ret;
}
#endif /* ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM && (DEM_OBD_DISPLACEMENT_SUPPORT == STD_ON)) */

/**
 * Checks if an event is an event may be displaced by another
 * @param candidateEventParam
 * @param eventParam
 * @return TRUE: Event may be displaced, FALSE: event may NOT be displaced
 */
static boolean obdEventDisplacementProhibited(const Dem_EventParameterType *candidateEventParam, const Dem_EventParameterType *eventParam)
{
#if (DEM_OBD_DISPLACEMENT_SUPPORT == STD_ON)
    Dem_EventStatusExtendedType evtStatus;
    boolean prohibited = FALSE;
    (void)getEventStatus(candidateEventParam->EventID, &evtStatus);
    if( (TRUE == eventIsEmissionRelated(candidateEventParam)) &&
        (
#if defined(DEM_USE_INDICATORS)
                (TRUE == eventActivatesMIL(candidateEventParam)) ||
#endif
#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM)
                ((TRUE == eventHoldsOBDFF(candidateEventParam)) && (candidateEventParam->EventClass->EventPriority <= eventParam->EventClass->EventPriority)) ||
#endif
        (0u != (evtStatus & DEM_PENDING_DTC))) ) {
        prohibited = TRUE;
    }
    return prohibited;
#else
    return FALSE;
#endif
}

/**
 * Returns whether event is considered passive or not
 * @param eventId
 * @return TRUE: Event passive, FALSE: Event NOT passive
 */
static boolean getEventPassive(Dem_EventIdType eventId)
{
    boolean eventPassive = FALSE;
    Dem_EventStatusExtendedType eventStatus = 0u;
    boolean statusFound = FALSE;

#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
    if( IS_COMBINED_EVENT_ID(eventId) ) {
        const Dem_CombinedDTCCfgType *CombDTCCfg = &configSet->CombinedDTCConfig[TO_COMBINED_EVENT_CFG_IDX(eventId)];
        CombDTCCfg = &configSet->CombinedDTCConfig[TO_COMBINED_EVENT_CFG_IDX(eventId)];
        if( DEM_STATUS_OK == GetDTCUDSStatus(CombDTCCfg->DTCClassRef, CombDTCCfg->MemoryDestination, &eventStatus) ) {
            statusFound = TRUE;
        }
    }
    else {
        const Dem_EventParameterType *eventParam = NULL;
        lookupEventIdParameter(eventId, &eventParam);
        if( NULL != eventParam ) {
            if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID ) {
                if( DEM_STATUS_OK == GetDTCUDSStatus(eventParam->DTCClassRef, eventParam->EventClass->EventDestination, &eventStatus) ) {
                    statusFound = TRUE;
                }
            }
            else {
                if( E_OK == getEventStatus(eventId, &eventStatus) ) {
                    statusFound = TRUE;
                }
            }
        }
    }
#else
    if( E_OK == getEventStatus(eventId, &eventStatus) ) {
        statusFound = TRUE;
    }
#endif
    if( TRUE == statusFound ) {
#if (DEM_TEST_FAILED_STORAGE == STD_ON)
        eventPassive = (0 == (eventStatus & DEM_TEST_FAILED));
#else
        eventPassive = (0 == (eventStatus & (DEM_TEST_FAILED | DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE)));
#endif
#if defined(CFG_DEM_TREAT_CONFIRMED_EVENTS_AS_ACTIVE)
        eventPassive = (0 != (eventStatus & DEM_CONFIRMED_DTC)) ? FALSE : eventPassive;
#endif
    }
    return eventPassive;

}

/**
 * Return the priority of an event
 * @param eventId
 * @return Priority of event
 */
static uint8 getEventPriority(Dem_EventIdType eventId)
{
    uint8 priority = 0xFF;

    const Dem_EventParameterType *eventParam = NULL;

#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
    if( IS_COMBINED_EVENT_ID(eventId) ) {
        const Dem_CombinedDTCCfgType *CombDTCCfg = &configSet->CombinedDTCConfig[TO_COMBINED_EVENT_CFG_IDX(eventId)];
        CombDTCCfg = &configSet->CombinedDTCConfig[TO_COMBINED_EVENT_CFG_IDX(eventId)];
        priority = CombDTCCfg->Priority;
    }
    else {
        lookupEventIdParameter(eventId, &eventParam);
        if( NULL != eventParam ) {
            if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID ) {
                const Dem_CombinedDTCCfgType *CombDTCCfg = &configSet->CombinedDTCConfig[eventParam->CombinedDTCCID];
                priority = CombDTCCfg->Priority;
            }
            else {
                priority = eventParam->EventClass->EventPriority;
            }
        }
    }
#else
    lookupEventIdParameter(eventId, &eventParam);
    if( NULL != eventParam ) {
        priority = eventParam->EventClass->EventPriority;
    }
#endif
    return priority;
}

#if ( DEM_FF_DATA_IN_PRI_MEM || DEM_FF_DATA_IN_SEC_MEM || DEM_FF_DATA_IN_PRE_INIT)
static Std_ReturnType getFFEventForDisplacement(const Dem_EventParameterType *eventParam, const FreezeFrameRecType *ffBuffer, uint32 bufferSize, Dem_EventIdType *eventToRemove)
{
    /* See figure 25 in ASR 4.0.3 */
    /* @req DEM403 */
    /* @req DEM404 */
    /* @req DEM405 */
    /* @req DEM406 */
    /* IMPROVEMENT: How do we handle the case when it is an OBD freeze frame that should be stored?
     * Should they be handled in the same way as "normal" freeze frames? */
    Std_ReturnType ret = E_NOT_OK;
    uint32 removeCandidateIndex = 0;
    uint32 oldestPassive_TimeStamp = DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT;
    uint32 oldestActive_TimeStamp = DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT;
    uint8 eventPrio = 0xFF;
    boolean eventPassive = FALSE;
    boolean passiveCandidateFound = FALSE;
    boolean activeCandidateFound = FALSE;
    boolean eventDataRemovalProhibited = FALSE;
    boolean obdProhibitsRemoval = FALSE;
    const Dem_EventParameterType *candidateEventParam;
    for( uint32 i = 0; i < bufferSize; i++ ) {
        if( (DEM_EVENT_ID_NULL != ffBuffer[i].eventId) && (eventParam->EventID != ffBuffer[i].eventId) && (DEM_FREEZE_FRAME_OBD != ffBuffer[i].kind) ) {
            candidateEventParam = NULL;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
            /* Event id may be a combined id. */
            if( IS_COMBINED_EVENT_ID(ffBuffer[i].eventId) ) {
                const Dem_CombinedDTCCfgType *CombDTCCfg = &configSet->CombinedDTCConfig[TO_COMBINED_EVENT_CFG_IDX(ffBuffer[i].eventId)];
                CombDTCCfg = &configSet->CombinedDTCConfig[TO_COMBINED_EVENT_CFG_IDX(ffBuffer[i].eventId)];
                /* Just grab the first event for this DTC. */
                lookupEventIdParameter(CombDTCCfg->DTCClassRef->Events[0u], &candidateEventParam);
            }
            else {
                lookupEventIdParameter(ffBuffer[i].eventId, &candidateEventParam);
            }
#else
            lookupEventIdParameter(ffBuffer[i].eventId, &candidateEventParam);
#endif
            if(NO_DTC_DISABLED != DTCRecordDisabled.DTC) {
                eventDataRemovalProhibited = eventDTCRecordDataUpdateDisabled(candidateEventParam);
            } else {
                eventDataRemovalProhibited = FALSE;
            }
            obdProhibitsRemoval = obdEventDisplacementProhibited(candidateEventParam, eventParam);
            if( (FALSE == eventDataRemovalProhibited) && (FALSE == obdProhibitsRemoval) ) {
                /* Check if priority if the event is higher or equal (low numerical value of priority means high priority..) *//* @req DEM383 */
                eventPrio = getEventPriority(ffBuffer[i].eventId);
                eventPassive = getEventPassive(ffBuffer[i].eventId);
                /* IMPROVEMENT: Remove the one with lowest priority? */
                if( (eventParam->EventClass->EventPriority <= eventPrio) && (TRUE == eventPassive) && (oldestPassive_TimeStamp > ffBuffer[i].timeStamp) ) {
                    /* This event has lower or equal priority to the reported event, it is passive
                     * and it is the oldest currently found. A candidate for removal. */
                    oldestPassive_TimeStamp = ffBuffer[i].timeStamp;
                    removeCandidateIndex = i;
                    passiveCandidateFound = TRUE;

                }
                if( FALSE == passiveCandidateFound ) {
                    /* Currently, a passive event with lower or equal priority has not been found.
                     * Check if the priority is less than for the reported event. Store the oldest. */
                    if( (eventParam->EventClass->EventPriority < eventPrio) &&  (oldestActive_TimeStamp > ffBuffer[i].timeStamp) ) {
                        oldestActive_TimeStamp = ffBuffer[i].timeStamp;
                        removeCandidateIndex = i;
                        activeCandidateFound = TRUE;
                    }
                }
            }
        }
    }

    if( (TRUE == passiveCandidateFound) || (TRUE == activeCandidateFound) ) {
        *eventToRemove = ffBuffer[removeCandidateIndex].eventId;
        ret = E_OK;
    }
    return ret;

}
#endif /* DEM_FF_DATA_IN_PRI_MEM || DEM_FF_DATA_IN_SEC_MEM || DEM_FF_DATA_IN_PRE_INIT */

#if (DEM_EXT_DATA_IN_PRE_INIT || DEM_EXT_DATA_IN_PRI_MEM || DEM_EXT_DATA_IN_SEC_MEM ) && defined(DEM_USE_MEMORY_FUNCTIONS)
static Std_ReturnType getExtDataEventForDisplacement(const Dem_EventParameterType *eventParam, const ExtDataRecType *extDataBuffer, uint32 bufferSize, Dem_EventIdType *eventToRemove)
{
    /* See figure 25 in ASR 4.0.3 */
    /* @req DEM403 */
    /* @req DEM404 */
    /* @req DEM405 */
    /* @req DEM406 */
    Std_ReturnType ret = E_NOT_OK;
    uint32 removeCandidateIndex = 0;
    uint32 oldestPassive_TimeStamp = DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT;
    uint32 oldestActive_TimeStamp = DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT;
    uint8 eventPrio = 0xFF;
    boolean eventPassive = FALSE;
    boolean passiveCandidateFound = FALSE;
    boolean activeCandidateFound = FALSE;
    boolean eventDataRemovalProhibited = FALSE;
    boolean obdProhibitsRemoval = FALSE;
    const Dem_EventParameterType *candidateEventParam = NULL;
    for( uint32 i = 0; i < bufferSize; i++ ) {
        if( DEM_EVENT_ID_NULL != extDataBuffer[i].eventId ) {
            /* Check if we are allowed to remove data for this event */
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
            /* Event id may be a combined id. */
            if( IS_COMBINED_EVENT_ID(extDataBuffer[i].eventId) ) {
                const Dem_CombinedDTCCfgType *CombDTCCfg = &configSet->CombinedDTCConfig[TO_COMBINED_EVENT_CFG_IDX(extDataBuffer[i].eventId)];
                CombDTCCfg = &configSet->CombinedDTCConfig[TO_COMBINED_EVENT_CFG_IDX(extDataBuffer[i].eventId)];
                /* Just grab the first event for this DTC. */
                lookupEventIdParameter(CombDTCCfg->DTCClassRef->Events[0u], &candidateEventParam);
            }
            else {
                lookupEventIdParameter(extDataBuffer[i].eventId, &candidateEventParam);
            }
#else
            lookupEventIdParameter(extDataBuffer[i].eventId, &candidateEventParam);
#endif
            if(NO_DTC_DISABLED != DTCRecordDisabled.DTC) {
                eventDataRemovalProhibited = eventDTCRecordDataUpdateDisabled(candidateEventParam);
            } else {
                eventDataRemovalProhibited = FALSE;
            }
            obdProhibitsRemoval = obdEventDisplacementProhibited(candidateEventParam, eventParam);
            if( (FALSE == eventDataRemovalProhibited) && (FALSE == obdProhibitsRemoval) ) {
                /* Check if priority if the event is higher or equal (low numerical value of priority means high priority..) *//* @req DEM383 */
                eventPrio = getEventPriority(extDataBuffer[i].eventId);
                eventPassive = getEventPassive(extDataBuffer[i].eventId);
                if( (eventParam->EventClass->EventPriority <= eventPrio) && (TRUE == eventPassive) && (oldestPassive_TimeStamp > extDataBuffer[i].timeStamp) ) {
                    /* This event has lower or equal priority to the reported event, it is passive
                     * and it is the oldest currently found. A candidate for removal. */
                    oldestPassive_TimeStamp = extDataBuffer[i].timeStamp;
                    removeCandidateIndex = i;
                    passiveCandidateFound = TRUE;

                }
                if( FALSE == passiveCandidateFound ) {
                    /* Currently, a passive event with lower or equal priority has not been found.
                     * Check if the priority is less than for the reported event. Store the oldest. */
                    if( (eventParam->EventClass->EventPriority < eventPrio) &&  (oldestActive_TimeStamp > extDataBuffer[i].timeStamp) ) {
                        oldestActive_TimeStamp = extDataBuffer[i].timeStamp;
                        removeCandidateIndex = i;
                        activeCandidateFound = TRUE;
                    }
                }
            }
        }
    }

    if( (TRUE == passiveCandidateFound) || (TRUE == activeCandidateFound) ) {
        *eventToRemove = extDataBuffer[removeCandidateIndex].eventId;
        ret = E_OK;
    }
    return ret;
}
#endif /* (DEM_EXT_DATA_IN_PRE_INIT || DEM_EXT_DATA_IN_PRI_MEM || DEM_EXT_DATA_IN_SEC_MEM ) && defined(DEM_USE_MEMORY_FUNCTIONS) */
#ifdef DEM_USE_MEMORY_FUNCTIONS
static Std_ReturnType getEventForDisplacement(const Dem_EventParameterType *eventParam, const EventRecType *eventBuffer,
                                              uint32 bufferSize, Dem_EventIdType *eventToRemove)
{
    /* See figure 25 in ASR 4.0.3 */
    /* @req DEM403 */
    /* @req DEM404 */
    /* @req DEM405 */
    /* @req DEM406 */
    Std_ReturnType ret = E_NOT_OK;
    uint32 removeCandidateIndex = 0;
    uint32 oldestPassive_TimeStamp = DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT;
    uint32 oldestActive_TimeStamp = DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT;
    uint8 eventPrio = 0xFF;
    boolean eventPassive = FALSE;
    boolean passiveCandidateFound = FALSE;
    boolean activeCandidateFound = FALSE;
    boolean eventDataRemovalProhibited = FALSE;
    boolean obdProhibitsRemoval = FALSE;
    const Dem_EventParameterType *candidateEventParam = NULL;
    for( uint32 i = 0; i < bufferSize; i++ ) {
        if( DEM_EVENT_ID_NULL != eventBuffer[i].EventData.eventId ) {
            lookupEventIdParameter(eventBuffer[i].EventData.eventId, &candidateEventParam);
            if(NO_DTC_DISABLED != DTCRecordDisabled.DTC) {
                eventDataRemovalProhibited = eventDTCRecordDataUpdateDisabled(candidateEventParam);
            } else {
                eventDataRemovalProhibited = FALSE;
            }
            obdProhibitsRemoval = obdEventDisplacementProhibited(candidateEventParam, eventParam);
            if( (FALSE == eventDataRemovalProhibited) && (FALSE == obdProhibitsRemoval) ) {
                /* Check if priority if the event is higher or equal (low numerical value of priority means high priority..) *//* @req DEM383 */
                eventPrio = getEventPriority(eventBuffer[i].EventData.eventId);
                eventPassive = getEventPassive(eventBuffer[i].EventData.eventId);
                if( (eventParam->EventClass->EventPriority <= eventPrio) && (TRUE == eventPassive) && (oldestPassive_TimeStamp > eventBuffer[i].EventData.timeStamp) ) {
                    /* This event has lower or equal priority to the reported event, it is passive
                     * and it is the oldest currently found. A candidate for removal. */
                    oldestPassive_TimeStamp = eventBuffer[i].EventData.timeStamp;
                    removeCandidateIndex = i;
                    passiveCandidateFound = TRUE;

                }
                if( FALSE == passiveCandidateFound ) {
                    /* Currently, a passive event with lower or equal priority has not been found.
                     * Check if the priority is less than for the reported event. Store the oldest. */
                    if( (eventParam->EventClass->EventPriority < eventPrio) &&  (oldestActive_TimeStamp > eventBuffer[i].EventData.timeStamp) ) {
                        oldestActive_TimeStamp = eventBuffer[i].EventData.timeStamp;
                        removeCandidateIndex = i;
                        activeCandidateFound = TRUE;
                    }
                }
            }
        }
    }

    if( (TRUE == passiveCandidateFound) || (TRUE == activeCandidateFound) ) {
        *eventToRemove = eventBuffer[removeCandidateIndex].EventData.eventId;
        ret = E_OK;
    }
    return ret;
}
#endif

#endif /* DEM_DISPLACEMENT_PROCESSING_DEM_INTERNAL */

#if ( DEM_FF_DATA_IN_PRE_INIT )
static boolean lookupFreezeFrameForDisplacementPreInit(const Dem_EventParameterType *eventParam, FreezeFrameRecType **freezeFrame)
{
    boolean freezeFrameFound = FALSE;
    Dem_EventIdType eventToRemove = DEM_EVENT_ID_NULL;

#if defined(DEM_DISPLACEMENT_PROCESSING_DEM_EXTENSION)
    Dem_Extension_GetFFEventForDisplacement(eventParam, preInitFreezeFrameBuffer, DEM_MAX_NUMBER_FF_DATA_PRE_INIT, &eventToRemove);
#elif defined(DEM_DISPLACEMENT_PROCESSING_DEM_INTERNAL)
    if( E_OK != getFFEventForDisplacement(eventParam, preInitFreezeFrameBuffer, DEM_MAX_NUMBER_FF_DATA_PRE_INIT, &eventToRemove) ) {
        eventToRemove = DEM_EVENT_ID_NULL;
    }
#else
#warning Unsupported displacement
#endif
    if( DEM_EVENT_ID_NULL != eventToRemove ) {
        /* Freeze frame for a less significant event was found.
         * Find the all entries in pre init freeze frame buffer and remove these. */
        for (uint16 indx = 0; (indx < DEM_MAX_NUMBER_FF_DATA_PRE_INIT); indx++) {
            if( preInitFreezeFrameBuffer[indx].eventId == eventToRemove ) {
                memset(&preInitFreezeFrameBuffer[indx], 0, sizeof(FreezeFrameRecType));
                if( FALSE == freezeFrameFound ) {
                    *freezeFrame = &preInitFreezeFrameBuffer[indx];
                    freezeFrameFound = TRUE;
                }
            }
        }
#if defined(USE_DEM_EXTENSION)
        if( TRUE == freezeFrameFound ) {
            Dem_Extension_EventFreezeFrameDataDisplaced(eventToRemove);
        }
#endif
    } else {
        /* Buffer is full and the currently stored data is more significant */
    }
    return freezeFrameFound;
}
#endif

#ifdef DEM_USE_MEMORY_FUNCTIONS
#if ( DEM_FF_DATA_IN_PRI_MEM || DEM_FF_DATA_IN_SEC_MEM || DEM_FF_DATA_IN_PRE_INIT)
static boolean lookupFreezeFrameForDisplacement(const Dem_EventParameterType *eventParam, FreezeFrameRecType **freezeFrame,
                                                FreezeFrameRecType* freezeFrameBuffer, uint32 freezeFrameBufferSize)
{
    boolean freezeFrameFound = FALSE;
    Dem_EventIdType eventToRemove = DEM_EVENT_ID_NULL;
    const Dem_EventParameterType *eventToRemoveParam = NULL;

#if defined(DEM_DISPLACEMENT_PROCESSING_DEM_EXTENSION)
    Dem_Extension_GetFFEventForDisplacement(eventParam, freezeFrameBuffer, freezeFrameBufferSize, &eventToRemove);
#elif defined(DEM_DISPLACEMENT_PROCESSING_DEM_INTERNAL)
    if( E_OK != getFFEventForDisplacement(eventParam, freezeFrameBuffer, freezeFrameBufferSize, &eventToRemove) ) {
        eventToRemove = DEM_EVENT_ID_NULL;
    }
#else
#warning Unsupported displacement
#endif
    if( DEM_EVENT_ID_NULL != eventToRemove ) {
        /* Freeze frame for a less significant event was found.
         * Find the all entries in freeze frame buffer and remove these. */
        for (uint16 indx = 0; (indx < freezeFrameBufferSize); indx++) {
            if( freezeFrameBuffer[indx].eventId == eventToRemove ) {
                memset(&freezeFrameBuffer[indx], 0, sizeof(FreezeFrameRecType));
                if( FALSE == freezeFrameFound ) {
                    *freezeFrame = &freezeFrameBuffer[indx];
                    freezeFrameFound = TRUE;
                }
            }
        }
#if defined(USE_DEM_EXTENSION)
        if( TRUE == freezeFrameFound ) {
            Dem_Extension_EventFreezeFrameDataDisplaced(eventToRemove);
        }
#endif
        if( TRUE == freezeFrameFound ) {
            lookupEventIdParameter(eventToRemove, &eventToRemoveParam);
            /* @req DEM475 */
            notifyEventDataChanged(eventToRemoveParam);
        }
    } else {
        /* Buffer is full and the currently stored data is more significant */
    }

    return freezeFrameFound;
}
#endif /* DEM_FF_DATA_IN_PRI_MEM || DEM_FF_DATA_IN_SEC_MEM || DEM_FF_DATA_IN_PRE_INIT */
#endif /* USE_MEMORY_FUNCTIONS */
#endif /* DEM_EVENT_DISPLACEMENT_SUPPORT */

static Std_ReturnType getNofStoredNonOBDFreezeFrames(const Dem_EventParameterType *eventParam, Dem_DTCOriginType origin, uint8 *nofStored)
{
    uint8 nofFound = 0;
    const FreezeFrameRecType *ffBufferPtr = NULL;
    uint8 bufferSize = 0;
    if( DEM_INITIALIZED == demState ) {
#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM)
        if (origin == DEM_DTC_ORIGIN_PRIMARY_MEMORY) {
            ffBufferPtr = &priMemFreezeFrameBuffer[0];
            bufferSize = DEM_MAX_NUMBER_FF_DATA_PRI_MEM;
        }
#endif
#if ((DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_SEC_MEM)
        if (origin == DEM_DTC_ORIGIN_SECONDARY_MEMORY) {
            ffBufferPtr = &secMemFreezeFrameBuffer[0];
            bufferSize = DEM_MAX_NUMBER_FF_DATA_SEC_MEM;
        }
#endif

#if !((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM) && \
     !((DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_SEC_MEM)
   /* Avoid compiler warning when no FF is used */
   (void)origin;
#endif

    } else {
#if ( DEM_FF_DATA_IN_PRE_INIT )
        ffBufferPtr = &preInitFreezeFrameBuffer[0];
        bufferSize = DEM_MAX_NUMBER_FF_DATA_PRE_INIT;
#endif
    }
    Dem_EventIdType idToFind;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
    if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID ) {
        idToFind = TO_COMBINED_EVENT_ID(eventParam->CombinedDTCCID);
    }
    else {
        idToFind = eventParam->EventID;
    }
#else
    idToFind = eventParam->EventID;
#endif
    for( uint8 i = 0; (i < bufferSize) && (ffBufferPtr != NULL); i++) {
        if((ffBufferPtr[i].eventId == idToFind) && (DEM_FREEZE_FRAME_NON_OBD == ffBufferPtr[i].kind)) {
            nofFound++;
        }
    }
    *nofStored = nofFound;
    return E_OK;
}

#if (DEM_USE_TIMESTAMPS == STD_ON)
#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM) || \
    ((DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_SEC_MEM) || \
    (DEM_FF_DATA_IN_PRE_INIT)
static void setFreezeFrameTimeStamp(FreezeFrameRecType *freezeFrame)
{
    if( DEM_INITIALIZED == demState ) {
        if(FF_TimeStamp >= DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT){
            rearrangeFreezeFrameTimeStamp(&FF_TimeStamp);
        }
        freezeFrame->timeStamp = FF_TimeStamp;
        FF_TimeStamp++;
    } else {
        freezeFrame->timeStamp = FF_TimeStamp;
        if( FF_TimeStamp < DEM_MAX_TIMESTAMP_FOR_PRE_INIT ) {
            FF_TimeStamp++;
        }
    }
}
#endif
#endif

static void getFFClassReference(const Dem_EventParameterType *eventParam, Dem_FreezeFrameClassType **ffClassTypeRef)
{
    Dem_FreezeFrameClassTypeRefIndex ffIdx = getFFIdx(eventParam);
    if ((ffIdx != DEM_FF_NULLREF) ) {
        *ffClassTypeRef = (Dem_FreezeFrameClassType *) &(configSet->GlobalFreezeFrameClassRef[ffIdx]);
    }
}

#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM) || \
    ((DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_SEC_MEM) || \
    (DEM_FF_DATA_IN_PRE_INIT)
/*
 * Procedure:   getPidData
 * Description: get OBD FF data,only called by getFreezeFrameData()
 */
static void getPidData(const Dem_PidOrDidType *const *const *pidClassPtr, FreezeFrameRecType *const *freezeFrame, uint16 *storeIndexPtr)
{
    uint16 storeIndex = 0;
#if (DEM_MAX_NR_OF_PIDS_IN_FREEZEFRAME_DATA > 0)
    const Dem_PidOrDidType *const *FFIdClassRef;
    Std_ReturnType callbackReturnCode;
    uint16 recordSize = 0u;
    boolean detError = FALSE;
    FFIdClassRef = *pidClassPtr;
    //get all pids
    for (uint16 i = 0; ((i < DEM_MAX_NR_OF_PIDS_IN_FREEZEFRAME_DATA) && (FALSE == FFIdClassRef[i]->Arc_EOL) && (FALSE == detError)); i++) {
        //get pid length
        recordSize = FFIdClassRef[i]->PidOrDidSize;
        /* read out the pid data */
        if ((storeIndex + recordSize + DEM_PID_IDENTIFIER_SIZE_OF_BYTES) <= DEM_MAX_SIZE_FF_DATA) {
            /* store PID */
            (*freezeFrame)->data[storeIndex] = FFIdClassRef[i]->PidIdentifier;
            storeIndex++;

            /* store data */
            if(FFIdClassRef[i]->PidReadFnc != NULL){
                callbackReturnCode = FFIdClassRef[i]->PidReadFnc(&(*freezeFrame)->data[storeIndex]);
                if (callbackReturnCode != E_OK) {
                    memset(&(*freezeFrame)->data[storeIndex], DEM_FREEZEFRAME_DEFAULT_VALUE, (size_t)recordSize);
                    DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GET_FREEZEFRAME_ID, DEM_E_NODATAAVAILABLE);
                }
                storeIndex += recordSize;

            } else {
                memset(&(*freezeFrame)->data[storeIndex], DEM_FREEZEFRAME_DEFAULT_VALUE, (size_t)recordSize);
                storeIndex += recordSize;
            }
        } else {
            DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GET_FREEZEFRAME_ID, DEM_E_FF_TOO_BIG);
            detError = TRUE;/* Will break the loop */
        }

    }
#else
    (void)pidClassPtr;/*lint !e920*/
    (void)freezeFrame;/*lint !e920*/
#endif
    //store storeIndex,it will be used for judge whether FF contains valid data.
    *storeIndexPtr = storeIndex;

}

/*
 * Procedure:   getDidData
 * Description: get UDS FF data,only called by getFreezeFrameData()
 */
 static void getDidData(const Dem_PidOrDidType *const *const *didClassPtr, FreezeFrameRecType *const *freezeFrame, uint16 *storeIndexPtr)
{
    const Dem_PidOrDidType *const *FFIdClassRef;
    Std_ReturnType callbackReturnCode;
    uint16 storeIndex = 0u;
    uint16 recordSize = 0u;
    boolean detError = FALSE;

    FFIdClassRef = *didClassPtr;
    //get all dids
    for (uint16 i = 0u; ((i < DEM_MAX_NR_OF_DIDS_IN_FREEZEFRAME_DATA) && (FALSE == FFIdClassRef[i]->Arc_EOL) && (FALSE == detError)); i++) {
        recordSize = FFIdClassRef[i]->PidOrDidSize;
        /* read out the did data */
        if ((storeIndex + recordSize + DEM_DID_IDENTIFIER_SIZE_OF_BYTES) <= DEM_MAX_SIZE_FF_DATA) {
            /* store DID */
            (*freezeFrame)->data[storeIndex] = (uint8)(FFIdClassRef[i]->DidIdentifier>> 8u) & 0xFFu;
            storeIndex++;
            (*freezeFrame)->data[storeIndex] = (uint8)(FFIdClassRef[i]->DidIdentifier & 0xFFu);
            storeIndex++;
            /* store data */
            if(FFIdClassRef[i]->DidReadFnc!= NULL) {
                callbackReturnCode = FFIdClassRef[i]->DidReadFnc(&(*freezeFrame)->data[storeIndex]);
                if (callbackReturnCode != E_OK) {
                    /* @req DEM463 */
                    memset(&(*freezeFrame)->data[storeIndex], DEM_FREEZEFRAME_DEFAULT_VALUE, (size_t)recordSize);
                    DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GET_FREEZEFRAME_ID, DEM_E_NODATAAVAILABLE);
                }
                storeIndex += recordSize;
            } else {
                memset(&(*freezeFrame)->data[storeIndex], DEM_FREEZEFRAME_DEFAULT_VALUE, (size_t)recordSize);
                storeIndex += recordSize;
            }
        } else {
            DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GET_FREEZEFRAME_ID, DEM_E_FF_TOO_BIG);
            detError = TRUE;/* Will break the loop */
        }
    }

    //store storeIndex,it will be used for judge whether FF contains valid data.
    *storeIndexPtr = storeIndex;
}

 static Std_ReturnType getNextFFRecordNumber(const Dem_EventParameterType *eventParam, uint8 *recNum, Dem_FreezeFrameKindType ffKind, Dem_DTCOriginType origin)
 {
     /* @req DEM574 */
     Std_ReturnType ret = E_OK;
     uint8 nofStored = 0;
     uint8 maxNofRecords;
     const Dem_FreezeFrameRecNumClass *FreezeFrameRecNumClass;
     if( DEM_FREEZE_FRAME_NON_OBD == ffKind) {
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
        if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID ) {
            const Dem_CombinedDTCCfgType *CombDTCCfg = &configSet->CombinedDTCConfig[eventParam->CombinedDTCCID];
            maxNofRecords = CombDTCCfg->MaxNumberFreezeFrameRecords;
            FreezeFrameRecNumClass = CombDTCCfg->FreezeFrameRecNumClassRef;
        }
        else {
            maxNofRecords = eventParam->MaxNumberFreezeFrameRecords;
            FreezeFrameRecNumClass = eventParam->FreezeFrameRecNumClassRef;
        }
#else
        maxNofRecords = eventParam->MaxNumberFreezeFrameRecords;
        FreezeFrameRecNumClass = eventParam->FreezeFrameRecNumClassRef;
#endif
        if( 0 != maxNofRecords ) {
            if( E_OK == getNofStoredNonOBDFreezeFrames(eventParam, origin, &nofStored) ) {
                /* Was ok! */
                if( nofStored < maxNofRecords ) {
                    *recNum = FreezeFrameRecNumClass->FreezeFrameRecordNumber[nofStored];
                } else {
                    /* @req DEM585 *//* All records stored so update the latest */
                    if( (0u == nofStored) || (maxNofRecords > 1u) ) {
                        *recNum = FreezeFrameRecNumClass->FreezeFrameRecordNumber[maxNofRecords - 1];
                    }
                    else {
                        /* Event only has one record and it is already stored */
                        ret = E_NOT_OK;
                    }
                }
            }
         } else {
             /* No freeze frames should be stored for this event */
             ret = E_NOT_OK;
         }
     } else {
         /* Always record 0 for OBD freeze frames */
         *recNum = 0;/* @req DEM291 */
     }
     return ret;
 }

#if (DEM_PRESTORAGE_FF_DATA_IN_MEM)
 static boolean findPreStoredFreezeFrame(Dem_EventIdType eventId,  Dem_FreezeFrameKindType ffKind, FreezeFrameRecType **freezeFrame)
 {
     boolean ffFound = FALSE;
     FreezeFrameRecType *freezeFrameBuffer = &memPreStoreFreezeFrameBuffer[0];
     if (freezeFrameBuffer != NULL) {
         for (uint16 i = 0; (i < DEM_MAX_NUMBER_PRESTORED_FF) && (FALSE == ffFound); i++) {
             ffFound = ((freezeFrameBuffer[i].eventId == eventId) && (freezeFrameBuffer[i].kind == ffKind) )? TRUE: FALSE;
             if(TRUE == ffFound) {
                 *freezeFrame = &freezeFrameBuffer[i];
             }
         }
     }
     return ffFound;
 }
#endif

/*
 * Procedure:   getFreezeFrameData
 * Description: get FF data according configuration
 */
static void getFreezeFrameData(const Dem_EventParameterType *eventParam,
                               FreezeFrameRecType *freezeFrame,
                               Dem_FreezeFrameKindType ffKind,
                               Dem_DTCOriginType origin,
                               boolean useCombinedID)
{
    uint16 storeIndex = 0;
    Dem_FreezeFrameClassType *FreezeFrameLocalClass = NULL;
#if (DEM_PRESTORAGE_FF_DATA_IN_MEM)
    FreezeFrameRecType *preStoredFF = NULL;
#endif
    boolean preStoredFFfound = FALSE;

    /* clear FF data record */
    memset(freezeFrame, 0, sizeof(FreezeFrameRecType ));

    /* Find out the corresponding FF class */
    Dem_FreezeFrameClassTypeRefIndex ffIdx = getFFIdx(eventParam);

    if( (DEM_FREEZE_FRAME_NON_OBD == ffKind) && (ffIdx != DEM_FF_NULLREF) ) {
        getFFClassReference(eventParam, &FreezeFrameLocalClass);
    } else if((DEM_FREEZE_FRAME_OBD == ffKind) && (NULL != configSet->GlobalOBDFreezeFrameClassRef)) {
        FreezeFrameLocalClass = (Dem_FreezeFrameClassType *) configSet->GlobalOBDFreezeFrameClassRef;
    } else {
        FreezeFrameLocalClass = NULL;
    }

    /* get the dids */
    if(FreezeFrameLocalClass != NULL){
        if(FreezeFrameLocalClass->FFIdClassRef != NULL){
            if( DEM_FREEZE_FRAME_NON_OBD == ffKind ) {
                getDidData(&FreezeFrameLocalClass->FFIdClassRef, &freezeFrame, &storeIndex);
            } else if(DEM_FREEZE_FRAME_OBD == ffKind) {
                /* Get the pids */
                getPidData(&FreezeFrameLocalClass->FFIdClassRef, &freezeFrame, &storeIndex);
            } else {
                /* IMPROVEMENT: Det error */
            }
        }
    } else {
        /* create an empty FF */
        freezeFrame->eventId = DEM_EVENT_ID_NULL;
    }

#if (DEM_PRESTORAGE_FF_DATA_IN_MEM)
    /* @req DEM464 */
    if (eventParam->EventClass->FFPrestorageSupported == TRUE) {
        /* Look for pre-stored FF in pre-stored buffer */
        if (findPreStoredFreezeFrame(eventParam->EventID, ffKind, (FreezeFrameRecType **) &preStoredFF) == TRUE) {
            if ( preStoredFF != NULL) {
                /* @req DEM465 */
                /* Remove FF from preStored buffer */
                memcpy(freezeFrame, preStoredFF, sizeof(FreezeFrameRecType));
                memset(preStoredFF, 0, sizeof(FreezeFrameRecType ));

                Dem_NvM_SetPreStoreFreezeFrameBlockChanged(FALSE);

                preStoredFFfound = TRUE;
            }
        }
    }
#endif

    if (preStoredFFfound == FALSE) {
        uint8 recNum = 0;
        /* Check if any data has been stored and that there is a record number */
        if ( (storeIndex != 0) && (E_OK == getNextFFRecordNumber(eventParam, &recNum, ffKind, origin))) {/*lint !e9007 */
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
            if( (DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID) && (TRUE == useCombinedID) ) {
                freezeFrame->eventId = TO_COMBINED_EVENT_ID(eventParam->CombinedDTCCID);
            }
            else {
                freezeFrame->eventId = eventParam->EventID;
            }
#else
            freezeFrame->eventId = eventParam->EventID;
#endif

            freezeFrame->dataSize = storeIndex;
            freezeFrame->recordNumber = recNum;
            freezeFrame->kind = ffKind;
#if (DEM_USE_TIMESTAMPS == STD_ON)
            setFreezeFrameTimeStamp(freezeFrame);
#endif
        } else {
            freezeFrame->eventId = DEM_EVENT_ID_NULL;
            freezeFrame->dataSize = storeIndex;
        }
    }
#if !defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
    (void)useCombinedID;
#endif
}
#endif

#if (defined(DEM_FREEZE_FRAME_CAPTURE_EXTENSION) && DEM_FF_DATA_IN_PRE_INIT)
static void deleteFreezeFrameDataPreInit(const Dem_EventParameterType *eventParam)
{
    /* Delete all freeze frames */
    for (uint16 i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRE_INIT; i++){
        if(preInitFreezeFrameBuffer[i].eventId == eventParam->EventID) {
            memset(&preInitFreezeFrameBuffer[i], 0, sizeof(FreezeFrameRecType));
        }
    }
}
#endif
/*
 * Procedure:   storeFreezeFrameDataPreInit
 * Description: store FF in before  preInitFreezeFrameBuffer DEM's full initialization
 */
#if ( DEM_FF_DATA_IN_PRE_INIT )
static void storeFreezeFrameDataPreInit(const Dem_EventParameterType *eventParam, const FreezeFrameRecType *freezeFrame)
{
    boolean eventIdFound = FALSE;
    boolean eventIdFreePositionFound=FALSE;
    uint16 i;
    /* Check if already stored */
    Dem_EventIdType idToFind;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
    if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID ) {
        idToFind = TO_COMBINED_EVENT_ID(eventParam->CombinedDTCCID);
    }
    else {
        idToFind = eventParam->EventID;
    }
#else
    idToFind = eventParam->EventID;
#endif
    for (i = 0; (i < DEM_MAX_NUMBER_FF_DATA_PRE_INIT) && (FALSE == eventIdFound); i++) {
        if( DEM_FREEZE_FRAME_NON_OBD == freezeFrame->kind ) {
            eventIdFound = ( (preInitFreezeFrameBuffer[i].eventId == idToFind) && (preInitFreezeFrameBuffer[i].recordNumber == freezeFrame->recordNumber))? TRUE: FALSE;
        } else {
            eventIdFound = ((DEM_EVENT_ID_NULL != preInitFreezeFrameBuffer[i].eventId) && (DEM_FREEZE_FRAME_OBD == preInitFreezeFrameBuffer[i].kind))? TRUE: FALSE;
        }
    }

    if( TRUE == eventIdFound ) {
        /* Entry found. Overwrite if not an OBD freeze frame*/
        if( DEM_FREEZE_FRAME_NON_OBD == preInitFreezeFrameBuffer[i-1].kind ) {
            /* overwrite existing */
            memcpy(&preInitFreezeFrameBuffer[i-1], freezeFrame, sizeof(FreezeFrameRecType));
        }
    }
    else {
        /* lookup first free position */
        for (i = 0; (i < DEM_MAX_NUMBER_FF_DATA_PRE_INIT) && (FALSE == eventIdFreePositionFound); i++) {
            if( preInitFreezeFrameBuffer[i].eventId == DEM_EVENT_ID_NULL ) {
                eventIdFreePositionFound = TRUE;
            }
        }

        if ( TRUE == eventIdFreePositionFound ) {
            memcpy(&preInitFreezeFrameBuffer[i-1], freezeFrame, sizeof(FreezeFrameRecType));
        } else {
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON)
            /* @req DEM400 */
            /* @req DEM407 */
            /* do displacement */
            FreezeFrameRecType *freezeFrameLocal = NULL;
            if( TRUE == lookupFreezeFrameForDisplacementPreInit(eventParam, &freezeFrameLocal) ) {
                memcpy(freezeFrameLocal, freezeFrame, sizeof(FreezeFrameRecType));
            }
#else
            /* @req DEM402*/ /* Req is not the Det-error.. */
            DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_STORE_FF_DATA_PRE_INIT_ID, DEM_E_PRE_INIT_FF_DATA_BUFF_FULL);
#endif
        }
    }

}
#endif

#ifdef DEM_USE_MEMORY_FUNCTIONS
#if ( DEM_FF_DATA_IN_PRE_INIT )
static boolean isValidRecordNumber(const Dem_FreezeFrameRecNumClass *FreezeFrameRecNumClass, uint8 maxNumRecords, uint8 recordNumber)
{
    boolean isValid = FALSE;
    if( NULL != FreezeFrameRecNumClass ) {
        for( uint8 i = 0; (i < maxNumRecords) && (FALSE == isValid); i++ ) {
            if( FreezeFrameRecNumClass->FreezeFrameRecordNumber[i] == recordNumber ) {
                isValid = TRUE;
            }
        }
    }
    return isValid;
}
#endif

#if ( DEM_FF_DATA_IN_PRE_INIT )
/**
 * Transfers non-OBD freeze frame stored in pre-Init buffer to event destination
 * @param eventId
 * @param freezeFrameBuffer
 * @param freezeFrameBufferSize
 * @param eventHandled
 * @param origin
 * @param removeOldFFRecords
 * @return
 */
static boolean transferNonOBDFreezeFramesEvtMem(Dem_EventIdType eventId, FreezeFrameRecType* freezeFrameBuffer, uint32 freezeFrameBufferSize,
                                                boolean* eventHandled, Dem_DTCOriginType origin, boolean removeOldFFRecords)
{
    /* Note: Don't touch the OBD freeze frame */
    uint16 nofStoredMemory = 0u;
    uint16 nofStoredPreInit = 0u;
    uint16 nofFFToMove;
    const Dem_EventParameterType *eventParam;
    uint8 recordToFind;
    uint16 findRecordStartIndex = 0u;
    uint16 setRecordStartIndex = 0u;
    boolean memoryChanged = FALSE;
    boolean dataUpdated = FALSE;

    uint8 maxNofRecords;
    const Dem_FreezeFrameRecNumClass *FreezeFrameRecNumClass;

#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
    /* Event id may be a combined id. */
    if( IS_COMBINED_EVENT_ID(eventId) ) {
        const Dem_CombinedDTCCfgType *CombDTCCfg = &configSet->CombinedDTCConfig[TO_COMBINED_EVENT_CFG_IDX(eventId)];
        CombDTCCfg = &configSet->CombinedDTCConfig[TO_COMBINED_EVENT_CFG_IDX(eventId)];
        maxNofRecords = CombDTCCfg->MaxNumberFreezeFrameRecords;
        FreezeFrameRecNumClass = CombDTCCfg->FreezeFrameRecNumClassRef;
        /* Just grab the first event for this DTC. */
        lookupEventIdParameter(CombDTCCfg->DTCClassRef->Events[0u], &eventParam);
    }
    else {
        lookupEventIdParameter(eventId, &eventParam);
        maxNofRecords = eventParam->MaxNumberFreezeFrameRecords;
        FreezeFrameRecNumClass = eventParam->FreezeFrameRecNumClassRef;
    }
#else
    lookupEventIdParameter(eventId, &eventParam);
    maxNofRecords = eventParam->MaxNumberFreezeFrameRecords;
    FreezeFrameRecNumClass = eventParam->FreezeFrameRecNumClassRef;
#endif
    /* Count the number of entries in destination memory for the event */
    for( uint32 j = 0u; j < freezeFrameBufferSize; j++ ) {
        if( (eventId == freezeFrameBuffer[j].eventId) && (DEM_FREEZE_FRAME_NON_OBD == freezeFrameBuffer[j].kind)) {
            if( TRUE == isValidRecordNumber(FreezeFrameRecNumClass, maxNofRecords, freezeFrameBuffer[j].recordNumber) ) {
                if( TRUE == removeOldFFRecords) {
                    /* We should remove the old records. Don't increment nofStoredMemory
                     * since no records will be stored in the buffer after all have been cleared */
                    memset(&freezeFrameBuffer[j], 0, sizeof(FreezeFrameRecType));
                    memoryChanged = TRUE;
                } else {
                    nofStoredMemory++;
                }
            } else {
                /* Invalid ff record number */
                memset(&freezeFrameBuffer[j], 0, sizeof(FreezeFrameRecType));
                memoryChanged = TRUE;
            }
        }
    }

    /* Count the number of entries in the pre init memory for the event */
    for( uint16 k = 0; k < DEM_MAX_NUMBER_FF_DATA_PRE_INIT; k++ ) {
        if( (eventId == preInitFreezeFrameBuffer[k].eventId) && (DEM_FREEZE_FRAME_NON_OBD == preInitFreezeFrameBuffer[k].kind)) {
            nofStoredPreInit++;
        }
    }

    /* Find out the number of FF to transfer from preInit buffer to memory */
    /* We can assume that we should transfer at least on record from the preInitBuffer
     * since we found this event in the preInit buffer*/
    if( 0u ==  maxNofRecords) {
        nofFFToMove = 0u;
    } else if( maxNofRecords == nofStoredMemory ) {
        /* All records already stored in primary memory. Just update the last record */
        nofFFToMove = 1u;
        findRecordStartIndex = nofStoredPreInit - 1u;
        setRecordStartIndex = maxNofRecords - 1u;
    } else if( maxNofRecords > nofStoredMemory ) {
        nofFFToMove = (uint16)(MIN(nofStoredPreInit, (uint16)(maxNofRecords - nofStoredMemory)));
        findRecordStartIndex = nofStoredPreInit - nofFFToMove;
        setRecordStartIndex = nofStoredMemory;
    } else {
        /* To many records stored in primary memory. And they are all valid records
         * as we check this.
         * IMPROVEMENT: How do we handle this?
         * For now, throw all records in buffer and store the ones
         * in the preinit buffer. */
        for( uint32 i = 0u; i < freezeFrameBufferSize; i++ ) {
            if( (eventId == freezeFrameBuffer[i].eventId) && (DEM_FREEZE_FRAME_NON_OBD == freezeFrameBuffer[i].kind)) {
                memset(&freezeFrameBuffer[i], 0, sizeof(FreezeFrameRecType));
            }
        }
        DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GLOBAL_ID, DEM_E_MEMORY_CORRUPT);
        nofFFToMove = nofStoredPreInit;
        findRecordStartIndex = 0u;
        setRecordStartIndex = 0u;
    }

    if( 0u != nofFFToMove) {
        boolean ffStored = FALSE;
        for( uint16 offset = 0u; offset < nofFFToMove; offset++ ) {
            recordToFind = FreezeFrameRecNumClass->FreezeFrameRecordNumber[findRecordStartIndex + offset];
            ffStored = FALSE;
            for( uint16 indx = 0u; (indx < DEM_MAX_NUMBER_FF_DATA_PRE_INIT) && (FALSE == ffStored); indx++ ) {
                if( (preInitFreezeFrameBuffer[indx].eventId == eventId) && (preInitFreezeFrameBuffer[indx].recordNumber == recordToFind) &&
                        (DEM_FREEZE_FRAME_NON_OBD == preInitFreezeFrameBuffer[indx].kind) && (FALSE == eventHandled[indx]) ) {
                    /* Found the record to update */
                    preInitFreezeFrameBuffer[indx].recordNumber =
                            FreezeFrameRecNumClass->FreezeFrameRecordNumber[setRecordStartIndex + offset];
                    /* Store the freeze frame */
                    if( TRUE == storeFreezeFrameDataMem(eventParam, &preInitFreezeFrameBuffer[indx], freezeFrameBuffer, freezeFrameBufferSize, origin) ) {
                        dataUpdated = TRUE;
                    }
                    /* Clear the event id in the preInit buffer */
                    eventHandled[indx] = TRUE;
                    ffStored = TRUE;
                }
            }
        }

        if( nofFFToMove < nofStoredPreInit ) {
            /* Did not move all freeze frames from the preInit buffer.
             * Need to clear the remaining */
            for( uint16 indx = 0u; indx < DEM_MAX_NUMBER_FF_DATA_PRE_INIT; indx++ ) {
                if( (preInitFreezeFrameBuffer[indx].eventId == eventId) && (DEM_FREEZE_FRAME_NON_OBD == preInitFreezeFrameBuffer[indx].kind) ) {
                    eventHandled[indx] = TRUE;
                }
            }
        }
    }
    if( TRUE == dataUpdated ) {
        /* Use errorStatusChanged in eventsStatusBuffer to signal that the event data was updated */
        EventStatusRecType *eventStatusRecPtr;
        lookupEventStatusRec(eventParam->EventID, &eventStatusRecPtr);
        if( NULL != eventStatusRecPtr ) {
            eventStatusRecPtr->errorStatusChanged = TRUE;
        }
    }
    return memoryChanged;
}
#endif

#if ( DEM_FF_DATA_IN_PRE_INIT )
static void transferOBDFreezeFramesEvtMem(const FreezeFrameRecType *freezeFrame, FreezeFrameRecType* freezeFrameBuffer,
                                          uint32 freezeFrameBufferSize, Dem_DTCOriginType origin)
{
    const Dem_EventParameterType *eventParam;
    lookupEventIdParameter(freezeFrame->eventId, &eventParam);

    if (origin != DEM_DTC_ORIGIN_PRIMARY_MEMORY) {
        DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GLOBAL_ID, DEM_E_OBD_NOT_ALLOWED_IN_SEC_MEM);
    } else if( NULL != eventParam ) {
        /* Assuming that this function will not store the OBD freeze frame
         * if there already is one stored. */
        if( TRUE == storeOBDFreezeFrameDataMem(eventParam, freezeFrame, freezeFrameBuffer, freezeFrameBufferSize, origin) ) {
            /* Use errorStatusChanged in eventsStatusBuffer to signal that the event data was updated */
            EventStatusRecType *eventStatusRecPtr;
            lookupEventStatusRec(eventParam->EventID, &eventStatusRecPtr);
            if( NULL != eventStatusRecPtr ) {
                eventStatusRecPtr->errorStatusChanged = TRUE;
            }
        }
    } else {
        /* Bad origin and no config available.. */
    }
}
#endif

#if ( DEM_FF_DATA_IN_PRE_INIT )
static boolean transferPreInitFreezeFramesEvtMem(FreezeFrameRecType* freezeFrameBuffer, uint32 freezeFrameBufferSize, EventRecType* eventBuffer, uint32 eventBufferSize, Dem_DTCOriginType origin)
{
    boolean priMemChanged = FALSE;
    boolean removeOldFFRecords;
    boolean skipFF;
    const Dem_EventParameterType *eventParam;
    EventStatusRecType* eventStatusRec;
    boolean eventHandled[DEM_MAX_NUMBER_FF_DATA_PRE_INIT] = {FALSE};
    Dem_DTCOriginType eventOrigin;
    Dem_EventStatusExtendedType eventStatus;

    for( uint16 i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRE_INIT; i++ ) {
        if( (DEM_EVENT_ID_NULL != preInitFreezeFrameBuffer[i].eventId) && (TRUE == checkEntryValid(preInitFreezeFrameBuffer[i].eventId, origin, TRUE)) ) {
            /* Check if this is a freeze frame which should be processed.
             * Ignore freeze frames for event not stored in memory or OBD
             * freeze frames for event which are not confirmed  */
            eventStatusRec = NULL;
            eventParam = NULL;
            skipFF = TRUE;
            eventOrigin = DEM_DTC_ORIGIN_NOT_USED;;
            eventStatus = 0xFFu;/* This combination of bits is invalid */
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
            if( IS_COMBINED_EVENT_ID(preInitFreezeFrameBuffer[i].eventId) ) {
                const Dem_CombinedDTCCfgType *CombDTCCfg = &configSet->CombinedDTCConfig[TO_COMBINED_EVENT_CFG_IDX(preInitFreezeFrameBuffer[i].eventId)];
                eventOrigin = CombDTCCfg->MemoryDestination;
                if( DEM_STATUS_OK != GetDTCUDSStatus(CombDTCCfg->DTCClassRef, origin, &eventStatus) ) {
                    eventStatus = 0xFFu;
                }
            }
            else {
                lookupEventStatusRec(preInitFreezeFrameBuffer[i].eventId, &eventStatusRec);
                lookupEventIdParameter(preInitFreezeFrameBuffer[i].eventId, &eventParam);
                if( (NULL != eventStatusRec) && (NULL != eventParam) ) {
                    eventOrigin = eventParam->EventClass->EventDestination;
                    eventStatus = eventStatusRec->eventStatusExtended;
                }
            }
#else
            lookupEventStatusRec(preInitFreezeFrameBuffer[i].eventId, &eventStatusRec);
            lookupEventIdParameter(preInitFreezeFrameBuffer[i].eventId, &eventParam);
            if( (NULL != eventStatusRec) && (NULL != eventParam) ) {
                eventOrigin = eventParam->EventClass->EventDestination;
                eventStatus = eventStatusRec->eventStatusExtended;
            }
#endif

            if( (DEM_DTC_ORIGIN_NOT_USED != eventOrigin) && (0xFFu != eventStatus) ) {
                skipFF = FALSE;
                if( origin == eventOrigin ) {
                    if( (FALSE == eventIsStoredInMem(preInitFreezeFrameBuffer[i].eventId, eventBuffer, eventBufferSize)) ||
                            ((DEM_FREEZE_FRAME_OBD == preInitFreezeFrameBuffer[i].kind) && (0u == (eventStatus & DEM_CONFIRMED_DTC))) ) {
                        /* Event is not stored or FF is OBD and event is not confirmed. Skip FF */
                        skipFF = TRUE;
                    }
                }
            }
            if( FALSE == skipFF ) {
                removeOldFFRecords = FALSE;
#if defined(USE_DEM_EXTENSION)
                Dem_Extension_PreTransferPreInitFreezeFrames(preInitFreezeFrameBuffer[i].eventId, &removeOldFFRecords, origin);
#endif
                if( DEM_FREEZE_FRAME_NON_OBD == preInitFreezeFrameBuffer[i].kind ) {
                    if (TRUE == transferNonOBDFreezeFramesEvtMem(preInitFreezeFrameBuffer[i].eventId,
                                                         freezeFrameBuffer,
                                                         freezeFrameBufferSize,
                                                         eventHandled,
                                                         origin,
                                                         removeOldFFRecords)) {
                        priMemChanged = TRUE;
                    }
                } else {
                    transferOBDFreezeFramesEvtMem(&preInitFreezeFrameBuffer[i], freezeFrameBuffer,
                                                  freezeFrameBufferSize, origin);
                }
            }
        }
    }
    return priMemChanged;
}
#endif

#endif /* DEM_USE_MEMORY_FUNCTIONS */


#if (DEM_USE_TIMESTAMPS == STD_ON)
static void setExtDataTimeStamp(ExtDataRecType *extData)
{

    if( DEM_INITIALIZED == demState ) {
        if(ExtData_TimeStamp >= DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT){
            rearrangeExtDataTimeStamp(&ExtData_TimeStamp);
        }
        extData->timeStamp = ExtData_TimeStamp;
        ExtData_TimeStamp++;
    } else {
        extData->timeStamp = ExtData_TimeStamp;
        if( ExtData_TimeStamp < DEM_MAX_TIMESTAMP_FOR_PRE_INIT ) {
            ExtData_TimeStamp++;
        }
    }

}
#endif
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && (DEM_EXT_DATA_IN_PRE_INIT || DEM_EXT_DATA_IN_PRI_MEM || DEM_EXT_DATA_IN_SEC_MEM ) && defined(DEM_USE_MEMORY_FUNCTIONS)
static Std_ReturnType lookupExtDataForDisplacement(const Dem_EventParameterType *eventParam, ExtDataRecType *extDataBufPtr, uint32 bufferSize, ExtDataRecType **extData )
{
    const Dem_EventParameterType *eventToRemoveParam = NULL;
    Std_ReturnType ret = E_NOT_OK;
    /* @req DEM400 */
    Dem_EventIdType eventToRemove = DEM_EVENT_ID_NULL;

#if defined(DEM_DISPLACEMENT_PROCESSING_DEM_EXTENSION)
    Dem_Extension_GetExtDataEventForDisplacement(eventParam, extDataBufPtr, bufferSize, &eventToRemove);
#elif defined(DEM_DISPLACEMENT_PROCESSING_DEM_INTERNAL)
    if( E_OK != getExtDataEventForDisplacement(eventParam, extDataBufPtr, bufferSize, &eventToRemove) ) {
        eventToRemove = DEM_EVENT_ID_NULL;
    }
#else
#warning Unsupported displacement
#endif
    if( DEM_EVENT_ID_NULL != eventToRemove ) {
        /* Extended data for a less significant event was found.
         * Find the entry in ext data buffer. */
        for (uint32 indx = 0; (indx < bufferSize) && (E_OK != ret); indx++) {
            if( extDataBufPtr[indx].eventId == eventToRemove ) {
                memset(&extDataBufPtr[indx], 0, sizeof(ExtDataRecType));
                *extData = &extDataBufPtr[indx];
                ret = E_OK;
#if defined(USE_DEM_EXTENSION)
                Dem_Extension_EventExtendedDataDisplaced(eventToRemove);
#endif
                lookupEventIdParameter(eventToRemove, &eventToRemoveParam);
                /* @req DEM475 */
                notifyEventDataChanged(eventToRemoveParam);
            }
        }
    } else {
        /* Buffer is full and the currently stored data is more significant *//* @req DEM407 */
    }
    return ret;
}
#endif /* (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && (DEM_EXT_DATA_IN_PRE_INIT || DEM_EXT_DATA_IN_PRI_MEM || DEM_EXT_DATA_IN_SEC_MEM ) && defined(DEM_USE_MEMORY_FUNCTIONS) */




#if ( DEM_EXT_DATA_IN_PRE_INIT || DEM_EXT_DATA_IN_PRI_MEM || DEM_EXT_DATA_IN_SEC_MEM ) && defined(DEM_USE_MEMORY_FUNCTIONS)
static boolean StoreExtDataInMem( const Dem_EventParameterType *eventParam, ExtDataRecType *extDataMem, uint16 bufferSize, Dem_DTCOriginType origin, boolean overrideOldData) {

    uint16 storeIndex = 0;
    uint16 recordSize;
    const Dem_ExtendedDataRecordClassType *extendedDataRecord;
    const Dem_ExtendedDataClassType *ExtendedDataClass;
    ExtDataRecType *extData = NULL;
    boolean eventIdFound = FALSE;
    boolean bStoredData = FALSE;

    Dem_EventIdType idToFind;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
    if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID ) {
        idToFind = TO_COMBINED_EVENT_ID(eventParam->CombinedDTCCID);
        ExtendedDataClass = configSet->CombinedDTCConfig[eventParam->CombinedDTCCID].ExtendedDataClassRef;
    }
    else {
        idToFind = eventParam->EventID;
        ExtendedDataClass = eventParam->ExtendedDataClassRef;
    }
#else
    idToFind = eventParam->EventID;
    ExtendedDataClass = eventParam->ExtendedDataClassRef;
#endif


    // Check if already stored
    for (uint16 i = 0; (i < bufferSize) && (FALSE == eventIdFound); i++){
        eventIdFound = (extDataMem[i].eventId == idToFind)? TRUE: FALSE;
        extData = &extDataMem[i];
    }

    if( FALSE == eventIdFound ) {
        extData = NULL;
        for (uint16 i = 0; (i < bufferSize) && (NULL == extData); i++){
            if( extDataMem[i].eventId == DEM_EVENT_ID_NULL ) {
                extData = &extDataMem[i];
            }
        }
        if( NULL == extData ) {
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON)
            /* @req DEM400 *//* @req DEM407 */
            if( E_OK != lookupExtDataForDisplacement(eventParam, extDataMem, bufferSize, &extData) ) {
                setOverflowIndication(eventParam->EventClass->EventDestination, TRUE);
                return FALSE;
            }
#else
            /* @req DEM402*//* Displacement supported disabled */
            setOverflowIndication(eventParam->EventClass->EventDestination, TRUE);
            DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_STORE_EXT_DATA_MEM_ID, DEM_E_MEM_EXT_DATA_BUFF_FULL);
            return FALSE;
#endif /* DEM_EVENT_DISPLACEMENT_SUPPORT */
        }
    }

    // Check if any pointer to extended data class
    if ( (NULL != extData) && (ExtendedDataClass != NULL) ) {
        // Request extended data and copy it to the buffer
        for (uint32 i = 0uL; (i < DEM_MAX_NR_OF_RECORDS_IN_EXTENDED_DATA) && (ExtendedDataClass->ExtendedDataRecordClassRef[i] != NULL); i++) {
            extendedDataRecord = ExtendedDataClass->ExtendedDataRecordClassRef[i];
            if( DEM_UPDATE_RECORD_VOLATILE != extendedDataRecord->UpdateRule ) {
                recordSize = extendedDataRecord->DataSize;
                if ((storeIndex + recordSize) <= DEM_MAX_SIZE_EXT_DATA) {
                    if( (DEM_UPDATE_RECORD_YES == extendedDataRecord->UpdateRule) ||
                            ((DEM_UPDATE_RECORD_NO == extendedDataRecord->UpdateRule) && (extData->eventId != eventParam->EventID)) ||
                            (TRUE == overrideOldData)) {
                        /* Either update rule YES, or update rule is NO and extended data was not previously stored for this event */
                        if( NULL != extendedDataRecord->CallbackGetExtDataRecord ) {
                            /** @req DEM282 */
                            if (E_OK != extendedDataRecord->CallbackGetExtDataRecord(&extData->data[storeIndex])) {
                                // Callback data currently not available, clear space.
                                memset(&extData->data[storeIndex], 0xFF, (size_t)recordSize);
                            }
                            bStoredData = TRUE;
                        } else if( DEM_NO_ELEMENT != extendedDataRecord->InternalDataElement ) {
                            getInternalElement( eventParam, extendedDataRecord->InternalDataElement, &extData->data[storeIndex], extendedDataRecord->DataSize );
                            bStoredData = TRUE;
                        } else {
                            /* No callback and not internal element..
                             * IMPROVMENT: Det error */
                        }
                    } else {
                        /* Should not update */
                    }
                    storeIndex += recordSize;
                } else {
                    // Error: Size of extended data record is bigger than reserved space.
                    DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GET_EXTENDED_DATA_ID, DEM_E_EXT_DATA_TOO_BIG);
                    break;  // Break the loop
                }
            }
        }
    }

    // Check if any data has been stored
    if ( (NULL != extData) && (TRUE == bStoredData) ) {
        extData->eventId = idToFind;
#if (DEM_USE_TIMESTAMPS == STD_ON)
        setExtDataTimeStamp(extData);
#endif

#ifdef DEM_USE_MEMORY_FUNCTIONS
        if( DEM_PREINITIALIZED != demState ) {
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
            boolean immediateStorage = FALSE;
            EventStatusRecType *eventStatusRecPtr = NULL;
            lookupEventStatusRec(eventParam->EventID, &eventStatusRecPtr);
            if( (NULL != eventStatusRecPtr) && (NULL != eventParam->DTCClassRef) && (TRUE == eventParam->DTCClassRef->DTCRef->ImmediateNvStorage)
                    && (eventStatusRecPtr->occurrence <= DEM_IMMEDIATE_NV_STORAGE_LIMIT)) {
                immediateStorage = TRUE;
            }
            Dem_NvM_SetExtendedDataBlockChanged(origin, immediateStorage);
#else
            Dem_NvM_SetExtendedDataBlockChanged(origin, FALSE);
#endif
        }
#endif
    }
    return bStoredData;
}
#endif /* DEM_EXT_DATA_IN_PRE_INIT || DEM_EXT_DATA_IN_PRI_MEM || DEM_EXT_DATA_IN_SEC_MEM */


/*
 * Procedure:   getExtendedData
 * Description: Collects the extended data according to "eventParam" and return it in "extData",
 *              if not found eventId is set to DEM_EVENT_ID_NULL.
 */
static boolean storeExtendedData(const Dem_EventParameterType *eventParam, boolean overrideOldData)
{
    boolean ret = FALSE;
    if( DEM_PREINITIALIZED == demState ) {
#if ( DEM_EXT_DATA_IN_PRE_INIT )
        (void)StoreExtDataInMem(eventParam, preInitExtDataBuffer, DEM_MAX_NUMBER_EXT_DATA_PRE_INIT, DEM_DTC_ORIGIN_NOT_USED, overrideOldData);/* @req DEM468 */
#endif
    } else {
        switch (eventParam->EventClass->EventDestination) {
            case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_EXT_DATA_IN_PRI_MEM)
                ret = StoreExtDataInMem(eventParam, priMemExtDataBuffer, DEM_MAX_NUMBER_EXT_DATA_PRI_MEM, DEM_DTC_ORIGIN_PRIMARY_MEMORY, overrideOldData);/* @req DEM468 */
#endif
                break;
            case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
#if ((DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON) && DEM_EXT_DATA_IN_SEC_MEM)
                ret = StoreExtDataInMem(eventParam, secMemExtDataBuffer, DEM_MAX_NUMBER_EXT_DATA_SEC_MEM, DEM_DTC_ORIGIN_SECONDARY_MEMORY, overrideOldData);/* @req DEM468 */
#endif
                break;
            case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
            case DEM_DTC_ORIGIN_MIRROR_MEMORY:
                // Not yet supported
                DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GLOBAL_ID, DEM_E_NOT_IMPLEMENTED_YET);
                break;
            default:
                break;
        }
    }
    return ret;
}

#ifdef DEM_USE_MEMORY_FUNCTIONS
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON)
static Std_ReturnType findAndDisplaceEvent(const Dem_EventParameterType *eventParam, EventRecType *eventBuffer, uint32 bufferSize,
        EventRecType **memEventStatusRec, Dem_DTCOriginType origin)
{
    Std_ReturnType ret = E_NOT_OK;
    for (uint32 i = 0; (i < bufferSize) && (E_OK != ret); i++) {
        if( eventBuffer[i].EventData.eventId == eventParam->EventID ) {
            memset(&eventBuffer[i].EventData, 0, sizeof(EventRecType));
            *memEventStatusRec = &eventBuffer[i];
            ret = E_OK;
#if defined(USE_DEM_EXTENSION)
            Dem_Extension_EventDataDisplaced(eventParam->EventID);
#endif
        }
    }
    /* Reset it */
    EventStatusRecType *eventStatusRecPtr;
    lookupEventStatusRec(eventParam->EventID, &eventStatusRecPtr);
    if( NULL != eventStatusRecPtr ) {
        Dem_EventStatusExtendedType oldStatus = eventStatusRecPtr->eventStatusExtended;
        /* @req DEM409 *//* @req DEM538 */
        eventStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)(~DEM_CONFIRMED_DTC);
#if defined(DEM_FAILURE_PROCESSING_DEM_INTERNAL)
        eventStatusRecPtr->failureCounter = 0;
        eventStatusRecPtr->failedDuringFailureCycle = FALSE;
        eventStatusRecPtr->passedDuringFailureCycle = FALSE;
#endif
#if defined(DEM_aging_PROCESSING_DEM_INTERNAL)
        eventStatusRecPtr->agingCounter = 0;
        eventStatusRecPtr->failedDuringAgingCycle = FALSE;
        eventStatusRecPtr->passedDuringAgingCycle = FALSE;
#endif
        if( oldStatus != eventStatusRecPtr->eventStatusExtended ) {
            /* @req DEM016 */
            notifyEventStatusChange(eventParam, oldStatus, eventStatusRecPtr->eventStatusExtended);
        }
    }

    /* Remove all event related data */
    /* @req DEM408 */
    /* @req DEM542 */
    boolean combinedDTC = FALSE;
    const Dem_ExtendedDataClassType *ExtendedDataClass;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
    if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID ) {
        ExtendedDataClass = configSet->CombinedDTCConfig[eventParam->CombinedDTCCID].ExtendedDataClassRef;
        combinedDTC = TRUE;
    }
    else {
        ExtendedDataClass = eventParam->ExtendedDataClassRef;
    }
#else
    ExtendedDataClass = eventParam->ExtendedDataClassRef;
#endif
    if( DEM_FF_NULLREF != getFFIdx(eventParam) ) {
        (void)deleteFreezeFrameDataMem(eventParam, origin, combinedDTC);
    }
    if( NULL_PTR != ExtendedDataClass ) {
        (void)deleteExtendedDataMem(eventParam, origin, combinedDTC);
    }
#if defined(DEM_USE_INDICATORS)
    if( TRUE == resetIndicatorCounters(eventParam) ) {
#ifdef DEM_USE_MEMORY_FUNCTIONS
        /* IPROVEMENT: Immediate storage when deleting events? */
        Dem_NvM_SetIndicatorBlockChanged(FALSE);
#endif
    }
#endif
    /* @req DEM475 */
    notifyEventDataChanged(eventParam);

    return ret;

}

Std_ReturnType lookupEventForDisplacement(const Dem_EventParameterType *eventParam, EventRecType *eventBuffer, uint32 bufferSize,
        EventRecType **memEventStatusRec, Dem_DTCOriginType origin)
{
    Std_ReturnType ret = E_NOT_OK;
    Dem_EventIdType eventToRemove = DEM_EVENT_ID_NULL;
    /* No free position found. See if any of the stored events may be removed */
#if defined(DEM_DISPLACEMENT_PROCESSING_DEM_EXTENSION)
    Dem_Extension_GetEventForDisplacement(eventParam, eventBuffer, bufferSize, &eventToRemove);
#elif defined(DEM_DISPLACEMENT_PROCESSING_DEM_INTERNAL)
    if( E_OK != getEventForDisplacement(eventParam, eventBuffer, bufferSize, &eventToRemove) ) {
        eventToRemove = DEM_EVENT_ID_NULL;
    }
#else
#warning Unsupported displacement
#endif
    if( DEM_EVENT_ID_NULL != eventToRemove ) {
        const Dem_EventParameterType *removeEventParam = NULL;
        lookupEventIdParameter(eventToRemove, &removeEventParam);
        if( NULL != removeEventParam ) {
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
            const Dem_DTCClassType *DTCClass = removeEventParam->DTCClassRef;
            if( NULL_PTR != DTCClass ) {
                /* @req DEM443 */
                for(uint16 evIdx = 0; evIdx < DTCClass->NofEvents; evIdx++) {
                    removeEventParam = NULL;
                    lookupEventIdParameter(DTCClass->Events[evIdx], &removeEventParam);
                    if( NULL != removeEventParam ) {
                        if( E_OK == findAndDisplaceEvent(removeEventParam, eventBuffer, bufferSize, memEventStatusRec, origin) ) {
                            ret = E_OK;
                        }
                    }
                }
            }
            else {
                ret = findAndDisplaceEvent(removeEventParam, eventBuffer, bufferSize, memEventStatusRec, origin);
            }
#else
            ret = findAndDisplaceEvent(removeEventParam, eventBuffer, bufferSize, memEventStatusRec, origin);
#endif /* DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1 */
        }
    } else {
        /* Buffer is full and the currently stored data is more significant */
    }
    return ret;
}
#endif

/*
 * Procedure:   storeEventMem
 * Description: Store the event data of "eventStatus->eventId" in eventBuffer (i.e.primary or secondary memory),
 *              if non existent a new entry is created.
 */
static Std_ReturnType storeEventMem(const Dem_EventParameterType *eventParam, const EventStatusRecType *eventStatus,
        EventRecType* buffer, uint32 bufferSize, Dem_DTCOriginType origin, boolean failedNow)
{
    boolean positionFound = FALSE;
    EventRecType *eventStatusRec = NULL;
    Std_ReturnType ret = E_OK;
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
    boolean immediateStorage = FALSE;
#endif

    // Lookup event ID
    for (uint32 i = 0uL; (i < bufferSize) && (FALSE == positionFound); i++){
        if( buffer[i].EventData.eventId == eventStatus->eventId ) {
            eventStatusRec = &buffer[i];
            positionFound = TRUE;
        }
    }

    if( FALSE == positionFound ) {
        /* Event is not already stored, Search for free position */
        for (uint32 i  = 0uL; (i < bufferSize) && (FALSE == positionFound); i++){
            if( buffer[i].EventData.eventId == DEM_EVENT_ID_NULL ) {
                eventStatusRec = &buffer[i];
                positionFound = TRUE;
            }
        }
        if( FALSE == positionFound ) {
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON)
        /* @req DEM400 *//* @req DEM407 */
            if( E_OK == lookupEventForDisplacement(eventParam, buffer, bufferSize, &eventStatusRec, origin) ) {
                positionFound = TRUE;
            } else {
                setOverflowIndication(eventParam->EventClass->EventDestination, TRUE);
            }
#else
            /* @req DEM402*/ /* No displacement should be done */
            setOverflowIndication(eventParam->EventClass->EventDestination, TRUE);
#endif /* DEM_EVENT_DISPLACEMENT_SUPPORT */
        }
    }

    if ((TRUE == positionFound) && (NULL != eventStatusRec)) {
        // Update event found
        eventStatusRec->EventData.eventId = eventStatus->eventId;
        eventStatusRec->EventData.occurrence = eventStatus->occurrence;
        eventStatusRec->EventData.eventStatusExtended = eventStatus->eventStatusExtended;
#if defined(DEM_FAILURE_PROCESSING_DEM_INTERNAL)
        eventStatusRec->EventData.failureCounter = eventStatus->failureCounter;
#endif
#if defined(DEM_AGING_PROCESSING_DEM_INTERNAL)
        eventStatusRec->EventData.agingCounter = eventStatus->agingCounter;
#endif
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && defined(DEM_DISPLACEMENT_PROCESSING_DEM_INTERNAL)
        eventStatusRec->EventData.timeStamp = eventStatus->timeStamp;
#endif
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
        if( (TRUE == failedNow) && (NULL != eventParam->DTCClassRef) && (TRUE == eventParam->DTCClassRef->DTCRef->ImmediateNvStorage)
                && (eventStatus->occurrence <= DEM_IMMEDIATE_NV_STORAGE_LIMIT)) {
            immediateStorage = TRUE;
        }
        Dem_NvM_SetEventBlockChanged(origin, immediateStorage);
#else
        (void)failedNow;/* Only used for immediate storage */
        Dem_NvM_SetEventBlockChanged(origin, FALSE);
#endif
    } else {
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && defined(DEM_DISPLACEMENT_PROCESSING_DEM_INTERNAL)
        /* Error: mem event buffer full */
        DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_STORE_EVENT_MEM_ID, DEM_E_MEM_EVENT_BUFF_FULL);
#endif
        /* Buffer is full and all stored events are more significant */
        ret = E_NOT_OK;
    }

    return ret;
}

/*
 * Procedure:   deleteEventMem
 * Description: Delete the event data of "eventParam->eventId" from event buffer".
 */
static boolean deleteEventMem(const Dem_EventParameterType *eventParam, EventRecType* eventMemory, uint32 eventMemorySize, Dem_DTCOriginType origin)
{
    boolean eventIdFound = FALSE;
    uint32 i;

    for (i = 0uL; (i < eventMemorySize) && (FALSE == eventIdFound); i++){
        eventIdFound = (eventMemory[i].EventData.eventId == eventParam->EventID)? TRUE: FALSE;
    }

    if (TRUE == eventIdFound) {
        memset(&eventMemory[i-1], 0, sizeof(EventRecType));
        /* IMPROVEMENT: Immediate storage when delecting event? */
        Dem_NvM_SetEventBlockChanged(origin, FALSE);
    }
    return eventIdFound;
}
#endif /* DEM_USE_MEMORY_FUNCTIONS */


/**
 * Deletes DTC data i.e. event data, ff data and extended data for an event.
 * @param eventParam - the event which data shall be deleted for
 * @param dtcOriginFound - TRUE if origin found otherwise FALSE
 * @return TRUE if any data deleted otherwise FALSE
 */
static boolean DeleteDTCData(const Dem_EventParameterType *eventParam, boolean resetEventstatus, boolean *dtcOriginFound, boolean combinedDTC) {

    boolean dataDeleted = FALSE;

    switch (eventParam->EventClass->EventDestination)
    {
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
    case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
        /** @req DEM077 */
        if( TRUE == deleteEventMem(eventParam, priMemEventBuffer, DEM_MAX_NUMBER_EVENT_PRI_MEM, DEM_DTC_ORIGIN_PRIMARY_MEMORY) ) {
            dataDeleted = TRUE;
        }
        if( TRUE == deleteFreezeFrameDataMem(eventParam, DEM_DTC_ORIGIN_PRIMARY_MEMORY, combinedDTC) ) {
            dataDeleted = TRUE;
        }
        if( TRUE == deleteExtendedDataMem(eventParam, DEM_DTC_ORIGIN_PRIMARY_MEMORY, combinedDTC) ) {
            dataDeleted = TRUE;
        }
        if( TRUE == resetEventstatus ) {
            resetEventStatusRec(eventParam);
        }
        *dtcOriginFound = TRUE;
        break;
#endif
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
    case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
        if( TRUE == deleteEventMem(eventParam, secMemEventBuffer, DEM_MAX_NUMBER_EVENT_SEC_MEM, DEM_DTC_ORIGIN_SECONDARY_MEMORY) ) {
            dataDeleted = TRUE;
        }
        if( TRUE == deleteFreezeFrameDataMem(eventParam, DEM_DTC_ORIGIN_SECONDARY_MEMORY, combinedDTC) ) {
            dataDeleted = TRUE;
        }
        if( TRUE == deleteExtendedDataMem(eventParam, DEM_DTC_ORIGIN_SECONDARY_MEMORY, combinedDTC) ) {
            dataDeleted = TRUE;
        }
        if( TRUE == resetEventstatus ) {
            resetEventStatusRec(eventParam);
        }
        *dtcOriginFound = TRUE;
        break;
#endif
    default:
        *dtcOriginFound = FALSE;
        break;
    }
    return dataDeleted;
}

#if defined(DEM_USE_MEMORY_FUNCTIONS)
/**
 * Checks if an event is stored in its event destination
 * @param eventParam
 * @return TRUE: Event is stored in event memory, FALSE: Event not stored in event memory
 */
static boolean isInEventMemory(const Dem_EventParameterType *eventParam)
{
    boolean found = FALSE;
    uint16 memSize = 0;
    const EventRecType *mem = NULL;
    switch (eventParam->EventClass->EventDestination) {
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
            mem = priMemEventBuffer;
            memSize = DEM_MAX_NUMBER_EVENT_ENTRY_PRI;
#endif
            break;
        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
            mem = secMemEventBuffer;
            memSize = DEM_MAX_NUMBER_EVENT_ENTRY_SEC;
#endif
            break;
        default:
            break;
    }

    if( NULL != mem ) {
        for(uint16 i = 0; (i < memSize) && (FALSE == found); i++) {
            if( eventParam->EventID == mem[i].EventData.eventId ) {
                found = TRUE;
            }
        }
    }
    return found;
}
#endif

/*
 * Procedure:   storeEventEvtMem
 * Description: Store the event data of "eventStatus->eventId" in event memory according to
 *              "eventParam" destination option.
 */
static Std_ReturnType storeEventEvtMem(const Dem_EventParameterType *eventParam, const EventStatusRecType *eventStatus, boolean failedNow)
{
    Std_ReturnType ret = E_NOT_OK;

    switch (eventParam->EventClass->EventDestination) {
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
            ret = storeEventMem(eventParam, eventStatus, priMemEventBuffer, DEM_MAX_NUMBER_EVENT_ENTRY_PRI, DEM_DTC_ORIGIN_PRIMARY_MEMORY, failedNow); /** @req DEM010 */
#endif
            break;
        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
            ret = storeEventMem(eventParam, eventStatus, secMemEventBuffer, DEM_MAX_NUMBER_EVENT_ENTRY_SEC, DEM_DTC_ORIGIN_SECONDARY_MEMORY, failedNow);    /** @req DEM548 */
#endif
            break;
        case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
        case DEM_DTC_ORIGIN_MIRROR_MEMORY:
            // Not yet supported
            DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GLOBAL_ID, DEM_E_NOT_IMPLEMENTED_YET);
            break;
        default:
            break;
    }

    return ret;
}


/*
 * Procedure:   getExtendedDataMem
 * Description: Get record from buffer if it exists, or pick next free if it doesn't
 */
#ifdef DEM_USE_MEMORY_FUNCTIONS
#if (DEM_EXT_DATA_IN_PRE_INIT )
static void getExtendedDataMem(const Dem_EventParameterType *eventParam, ExtDataRecType ** const extendedData,
                                  ExtDataRecType* extendedDataBuffer, uint32 extendedDataBufferSize) /** @req DEM041 */
{
    boolean eventIdFound = FALSE;
    boolean eventIdFreePositionFound=FALSE;
    uint16 i;
    Dem_EventIdType idToFind;

#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
    if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID ) {
        idToFind = TO_COMBINED_EVENT_ID(eventParam->CombinedDTCCID);
    }
    else {
        idToFind = eventParam->EventID;
    }
#else
    idToFind = eventParam->EventID;
#endif
    // Check if already stored
    for (i = 0; (i < extendedDataBufferSize) && (FALSE == eventIdFound); i++){
        if( extendedDataBuffer[i].eventId == idToFind ) {
            *extendedData = &extendedDataBuffer[i];
            eventIdFound = TRUE;
        }
    }

    if ( FALSE == eventIdFound ) {
        // No, lookup first free position
        for (i = 0; (i < extendedDataBufferSize) && (FALSE == eventIdFreePositionFound); i++){
            eventIdFreePositionFound =  (extendedDataBuffer[i].eventId == DEM_EVENT_ID_NULL);
        }
        if ( TRUE == eventIdFreePositionFound ) {
            *extendedData = &extendedDataBuffer[i-1];
        } else {
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON)
            if(E_OK != lookupExtDataForDisplacement(eventParam, extendedDataBuffer, extendedDataBufferSize, extendedData)) {
                *extendedData = NULL;
            }
#else
            /* Displacement supported disabled */
            /* Error: mem extended data buffer full */
            DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_STORE_EXT_DATA_MEM_ID, DEM_E_MEM_EXT_DATA_BUFF_FULL);

#endif /* DEM_EVENT_DISPLACEMENT_SUPPORT */
        }
    }
}
#endif

/*
 * Procedure:   deleteExtendedDataMem
 * Description: Delete the extended data of "eventParam->eventId" from "priMemExtDataBuffer".
 */
static boolean deleteExtendedDataMem(const Dem_EventParameterType *eventParam, Dem_DTCOriginType origin, boolean combinedDTC)
{
    boolean eventIdFound = FALSE;
    uint32 i;

    ExtDataRecType* extBuffer = NULL;
    uint32 bufferSize = 0;
    Dem_EventIdType idToFind;

    switch (origin) {
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_EXT_DATA_IN_PRI_MEM)
            extBuffer = priMemExtDataBuffer;
            bufferSize = DEM_MAX_NUMBER_EXT_DATA_PRI_MEM;
#endif
            break;
        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
#if ((DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON) && DEM_EXT_DATA_IN_SEC_MEM)
            extBuffer = secMemExtDataBuffer;
            bufferSize = DEM_MAX_NUMBER_EXT_DATA_SEC_MEM;
#endif
            break;
        case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
        case DEM_DTC_ORIGIN_MIRROR_MEMORY:
            // Not yet supported
            DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GLOBAL_ID, DEM_E_NOT_IMPLEMENTED_YET);
            break;
        default:
            break;
    }
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
    if( (TRUE == combinedDTC) && (DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID) ) {
        idToFind = TO_COMBINED_EVENT_ID(eventParam->CombinedDTCCID);
    }
    else {
        idToFind = eventParam->EventID;
    }
#else
    (void)combinedDTC;
    idToFind = eventParam->EventID;
#endif
    if( NULL != extBuffer ) {
        // Check if already stored
        for (i = 0uL; (i < bufferSize) && (FALSE == eventIdFound); i++){
            eventIdFound = (extBuffer[i].eventId == idToFind)? TRUE: FALSE;
        }

        if (TRUE == eventIdFound) {
            // Yes, clear record
            memset(&extBuffer[i-1], 0, sizeof(ExtDataRecType));
            /* IMPROVEMENT: Immediate storage when deleting data? */
            Dem_NvM_SetExtendedDataBlockChanged(origin, FALSE);
        }
    }
    return eventIdFound;
}

/*
 * Procedure:   storeExtendedDataEvtMem
 * Description: Store the extended data in event memory according to
 *              "eventParam" destination option
 */
#if ( DEM_EXT_DATA_IN_PRE_INIT )
static boolean mergeExtendedDataEvtMem(const ExtDataRecType *extendedData, ExtDataRecType* extendedDataBuffer,
                                    uint32 extendedDataBufferSize, Dem_DTCOriginType origin, boolean updateAllExtData)
{
    uint16 i;
    const Dem_ExtendedDataRecordClassType *extendedDataRecordClass;
    ExtDataRecType *memExtDataRec = NULL;
    uint16 storeIndex = 0;
    boolean bCopiedData = FALSE;
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
    boolean immediateStorage = FALSE;
#endif
    const Dem_EventParameterType *eventParam = NULL_PTR;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
    if( IS_COMBINED_EVENT_ID(extendedData->eventId) ) {
        /* This is a combined event entry. */
        /* Get DTC config */
        const Dem_CombinedDTCCfgType *CombDTCCfg = &configSet->CombinedDTCConfig[TO_COMBINED_EVENT_CFG_IDX(extendedData->eventId)];
        /* Just grab the first event for this DTC. */
        lookupEventIdParameter(CombDTCCfg->DTCClassRef->Events[0u], &eventParam);
    }
    else {
        lookupEventIdParameter(extendedData->eventId, &eventParam);
    }
#else
    lookupEventIdParameter(extendedData->eventId, &eventParam);
#endif
    if( (NULL_PTR != eventParam) && (eventParam->EventClass->EventDestination == origin) ) {
        /* Management is only relevant for events stored in destinatio mem (i.e. nvram) */

        getExtendedDataMem(eventParam, &memExtDataRec, extendedDataBuffer, extendedDataBufferSize);

        if( NULL != memExtDataRec ) {
            /* We found an old record or could allocate a new slot */

            /* Only copy extended data related to event set during pre-init */
            for(i = 0; (i < DEM_MAX_NR_OF_RECORDS_IN_EXTENDED_DATA) && (eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i] != NULL); i++) {
                extendedDataRecordClass = eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i];
                if( DEM_UPDATE_RECORD_VOLATILE != extendedDataRecordClass->UpdateRule ) {
                    if( DEM_UPDATE_RECORD_YES == extendedDataRecordClass->UpdateRule ) {
                        /* Copy records that failed during pre init */
                        memcpy(&memExtDataRec->data[storeIndex], &extendedData->data[storeIndex],extendedDataRecordClass->DataSize);
                        bCopiedData = TRUE;
                    }
                    else if( DEM_UPDATE_RECORD_NO == extendedDataRecordClass->UpdateRule ) {
                        if( (eventParam->EventID != memExtDataRec->eventId)  || (TRUE == updateAllExtData) ) {
                            /* Extended data was not previously stored for this event. */
                            memcpy(&memExtDataRec->data[storeIndex], &extendedData->data[storeIndex],extendedDataRecordClass->DataSize);
                            bCopiedData = TRUE;
                        }
                    }
                    else {
                        /* DET FEL */
                    }
                    storeIndex += extendedDataRecordClass->DataSize;
                }
            }
            if( TRUE == bCopiedData ) {
                memExtDataRec->eventId = extendedData->eventId;
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && defined(DEM_DISPLACEMENT_PROCESSING_DEM_INTERNAL)
                memExtDataRec->timeStamp = extendedData->timeStamp;
#endif
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
                EventStatusRecType *eventStatusRecPtr = NULL;
                lookupEventStatusRec(eventParam->EventID, &eventStatusRecPtr);
                if( (NULL != eventStatusRecPtr) && (NULL != eventParam->DTCClassRef) &&
                        (TRUE == eventParam->DTCClassRef->DTCRef->ImmediateNvStorage) && (eventStatusRecPtr->occurrence <= DEM_IMMEDIATE_NV_STORAGE_LIMIT)) {
                    immediateStorage = TRUE;
                }
                Dem_NvM_SetExtendedDataBlockChanged(origin, immediateStorage);
#else
                Dem_NvM_SetExtendedDataBlockChanged(origin, FALSE);
#endif
            }
        }
        else {
            /* DET FEL */
        }
    }
    return bCopiedData;
}
#endif
#endif /* DEM_USE_MEMORY_FUNCTIONS */


/*
 * Procedure:   lookupExtendedDataRecNumParam
 * Description: Returns TRUE if the requested extended data number was found among the configured records for the event.
 *              "extDataRecClassPtr" returns a pointer to the record class, "posInExtData" returns the position in stored extended data.
 */
static boolean lookupExtendedDataRecNumParam(uint8 extendedDataNumber, const Dem_EventParameterType *eventParam, Dem_ExtendedDataRecordClassType const **extDataRecClassPtr, uint16 *posInExtData)
{
    boolean recNumFound = FALSE;
    const Dem_ExtendedDataClassType *ExtendedDataClass;

#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
    if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID ) {
        ExtendedDataClass = configSet->CombinedDTCConfig[eventParam->CombinedDTCCID].ExtendedDataClassRef;
    }
    else {
        ExtendedDataClass = eventParam->ExtendedDataClassRef;
    }
#else
    ExtendedDataClass = eventParam->ExtendedDataClassRef;
#endif

    if (ExtendedDataClass != NULL) {
        uint16  byteCnt = 0;
        uint32 i;

        // Request extended data and copy it to the buffer
        for (i = 0uL; (i < DEM_MAX_NR_OF_RECORDS_IN_EXTENDED_DATA) && (ExtendedDataClass->ExtendedDataRecordClassRef[i] != NULL) && (recNumFound == FALSE); i++) {
            if (ExtendedDataClass->ExtendedDataRecordClassRef[i]->RecordNumber == extendedDataNumber) {
                *extDataRecClassPtr =  ExtendedDataClass->ExtendedDataRecordClassRef[i];
                *posInExtData = byteCnt;
                recNumFound = TRUE;
            }
            if(DEM_UPDATE_RECORD_VOLATILE != ExtendedDataClass->ExtendedDataRecordClassRef[i]->UpdateRule) {
                byteCnt += ExtendedDataClass->ExtendedDataRecordClassRef[i]->DataSize;
            }
        }
    }

    return recNumFound;
}


/*
 * Procedure:   lookupExtendedDataMem
 * Description: Returns TRUE if the requested event id is found, "extData" points to the found data.
 */
static boolean lookupExtendedDataMem(Dem_EventIdType eventId, ExtDataRecType **extData, Dem_DTCOriginType origin)
{
    boolean eventIdFound = FALSE;
    uint32 i;
    ExtDataRecType* extBuffer = NULL;
    uint32 extBufferSize = 0;

    switch (origin) {
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_EXT_DATA_IN_PRI_MEM)
            extBuffer = priMemExtDataBuffer;
            extBufferSize = DEM_MAX_NUMBER_EXT_DATA_PRI_MEM;
#endif
            break;
        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
#if ((DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON) && DEM_EXT_DATA_IN_SEC_MEM)
            extBuffer = secMemExtDataBuffer;
            extBufferSize = DEM_MAX_NUMBER_EXT_DATA_SEC_MEM;
#endif
            break;
        default:
            break;
    }
    if( NULL == extBuffer ) {
        return FALSE;
    }
    // Lookup corresponding extended data
    for (i = 0uL; (i < extBufferSize) && (FALSE == eventIdFound); i++) {
        eventIdFound = (extBuffer[i].eventId == eventId)? TRUE: FALSE;
    }

    if (TRUE == eventIdFound) {
        // Yes, return pointer
        *extData = &extBuffer[i-1];
    }

    return eventIdFound;
}

#if (DEM_USE_TIMESTAMPS == STD_ON) && (DEM_EVENT_COMB_TYPE2_REPORT_OLDEST_DATA == STD_ON)
/**
 * Searcher buffer for older (compared to timestamp provided) extended data entry. Timestamp check is done based on parameter checkTimeStamp.
 * @param eventId
 * @param extData
 * @param origin
 * @param timestamp
 * @param checkTimeStamp
 * @return
 */
static boolean lookupOlderExtendedDataMem(Dem_EventIdType eventId, ExtDataRecType **extData, Dem_DTCOriginType origin, uint32 *timestamp, boolean checkTimeStamp)
{
    boolean eventIdFound = FALSE;
    uint32 i;
    ExtDataRecType* extBuffer = NULL;
    uint32 extBufferSize = 0;

    switch (origin) {
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_EXT_DATA_IN_PRI_MEM)
            extBuffer = priMemExtDataBuffer;
            extBufferSize = DEM_MAX_NUMBER_EXT_DATA_PRI_MEM;
#endif
            break;
        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
#if ((DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON) && DEM_EXT_DATA_IN_SEC_MEM)
            extBuffer = secMemExtDataBuffer;
            extBufferSize = DEM_MAX_NUMBER_EXT_DATA_SEC_MEM;
#endif
            break;
        default:
            break;
    }
    if( NULL == extBuffer ) {
        return FALSE;
    }
    // Lookup corresponding extended data
    for (i = 0uL; (i < extBufferSize) && (FALSE == eventIdFound); i++) {
        eventIdFound = (extBuffer[i].eventId == eventId)? TRUE: FALSE;
    }

    if ( TRUE == eventIdFound ) {
        if( (FALSE == checkTimeStamp) || (*timestamp > extBuffer[i-1u].timeStamp) ) {
            // Yes, return pointer
            *timestamp = extBuffer[i-1u].timeStamp;
            *extData = &extBuffer[i-1u];
        }
        else {
            eventIdFound = FALSE;
        }
    }

    return eventIdFound;
}
#endif
/*
 * Procedure:   storeFreezeFrameDataMem
 * Description: store FreezeFrame data record in primary memory
 */
#ifdef DEM_USE_MEMORY_FUNCTIONS
#if ( DEM_FF_DATA_IN_PRE_INIT || DEM_FF_DATA_IN_PRI_MEM || DEM_FF_DATA_IN_SEC_MEM )
static boolean storeFreezeFrameDataMem(const Dem_EventParameterType *eventParam, const FreezeFrameRecType *freezeFrame,
                                       FreezeFrameRecType* freezeFrameBuffer, uint32 freezeFrameBufferSize,
                                       Dem_DTCOriginType  origin)
{
    boolean eventIdFound = FALSE;
    boolean eventIdFreePositionFound=FALSE;
    boolean ffUpdated = FALSE;
    uint32 i;

    /* Check if already stored */
    for (i = 0uL; (i < freezeFrameBufferSize) && (FALSE == eventIdFound); i++){
        eventIdFound = ((freezeFrameBuffer[i].eventId == freezeFrame->eventId) && (freezeFrameBuffer[i].recordNumber == freezeFrame->recordNumber))? TRUE: FALSE;
    }

    if ( TRUE == eventIdFound ) {
        memcpy(&freezeFrameBuffer[i-1], freezeFrame, sizeof(FreezeFrameRecType));
        ffUpdated = TRUE;
    }
    else {
        for (i = 0uL; (i < freezeFrameBufferSize) && (FALSE == eventIdFreePositionFound); i++){
            eventIdFreePositionFound =  (freezeFrameBuffer[i].eventId == DEM_EVENT_ID_NULL)? TRUE: FALSE;
        }
        if ( TRUE == eventIdFreePositionFound ) {
            memcpy(&freezeFrameBuffer[i-1], freezeFrame, sizeof(FreezeFrameRecType));
            ffUpdated = TRUE;
        } else {
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON)
            /* @req DEM400 *//* @req DEM407 */
            FreezeFrameRecType *freezeFrameLocal;
            if( TRUE == lookupFreezeFrameForDisplacement(eventParam, &freezeFrameLocal, freezeFrameBuffer, freezeFrameBufferSize) ){
                memcpy(freezeFrameLocal, freezeFrame, sizeof(FreezeFrameRecType));
                ffUpdated = TRUE;
            } else {
                setOverflowIndication(eventParam->EventClass->EventDestination, TRUE);
            }
#else
            /* @req DEM402*/ /* Req is not the Det-error.. */
            setOverflowIndication(eventParam->EventClass->EventDestination, TRUE);
            DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_STORE_FF_DATA_MEM_ID, DEM_E_MEM_FF_DATA_BUFF_FULL);
#endif
        }
    }

    if( TRUE == ffUpdated ) {
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
        boolean immediateStorage = FALSE;
        EventStatusRecType *eventStatusRecPtr = NULL;
        lookupEventStatusRec(eventParam->EventID, &eventStatusRecPtr);
        if( (NULL != eventStatusRecPtr) && (NULL != eventParam->DTCClassRef) &&
                (TRUE == eventParam->DTCClassRef->DTCRef->ImmediateNvStorage) && (eventStatusRecPtr->occurrence <= DEM_IMMEDIATE_NV_STORAGE_LIMIT)) {
            immediateStorage = TRUE;
        }
#if (DEM_PRESTORAGE_FF_DATA_IN_MEM)
        if ( (TRUE == eventParam->EventClass->FFPrestorageSupported) && (origin == DEM_DTC_ORIGIN_NOT_USED) ) {
        	Dem_NvM_SetPreStoreFreezeFrameBlockChanged(immediateStorage);
        } else {
        	Dem_NvM_SetFreezeFrameBlockChanged(origin, immediateStorage);
        }
#else
        Dem_NvM_SetFreezeFrameBlockChanged(origin, immediateStorage);
#endif
#else
#if (DEM_PRESTORAGE_FF_DATA_IN_MEM)
        if (eventParam->EventClass->FFPrestorageSupported && origin == DEM_DTC_ORIGIN_NOT_USED ) {
        	Dem_NvM_SetPreStoreFreezeFrameBlockChanged(FALSE);
        } else {
        	Dem_NvM_SetFreezeFrameBlockChanged(origin, FALSE);
        }
#else
        Dem_NvM_SetFreezeFrameBlockChanged(origin, FALSE);
#endif
#endif
    }
    return ffUpdated;
}
#endif

static boolean deleteFreezeFrameDataMem(const Dem_EventParameterType *eventParam, Dem_DTCOriginType origin, boolean combinedDTC)
{
    uint32 i;
    boolean ffDeleted = FALSE;
    FreezeFrameRecType* freezeFrameBuffer = NULL_PTR;
    uint32 bufferSize = 0uL;
    Dem_EventIdType idToFind;

    switch (origin) {
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
#if ( DEM_FF_DATA_IN_PRI_MEM )
            freezeFrameBuffer = priMemFreezeFrameBuffer;
            bufferSize = DEM_MAX_NUMBER_FF_DATA_PRI_MEM;
#else
            freezeFrameBuffer = NULL_PTR;
            bufferSize = 0uL;
#endif
            break;
#endif
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
#if ( DEM_FF_DATA_IN_SEC_MEM )
            freezeFrameBuffer = secMemFreezeFrameBuffer;
            bufferSize = DEM_MAX_NUMBER_FF_DATA_SEC_MEM;
#endif
            break;
#endif
        case DEM_DTC_ORIGIN_NOT_USED:
#if (DEM_PRESTORAGE_FF_DATA_IN_MEM)
        	 /* It could be a pre-stored freeze frame */
			if (TRUE == eventParam->EventClass->FFPrestorageSupported ) {
				freezeFrameBuffer = memPreStoreFreezeFrameBuffer;
				bufferSize = DEM_MAX_NUMBER_PRESTORED_FF;
			}
#endif
			break;
        default:
            break;
    }

#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
    if( (TRUE == combinedDTC) && (DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID) ) {
        idToFind = TO_COMBINED_EVENT_ID(eventParam->CombinedDTCCID);
    }
    else {
        idToFind = eventParam->EventID;
    }
#else
    (void)combinedDTC;
    idToFind = eventParam->EventID;
#endif

    if( NULL != freezeFrameBuffer ) {
        for (i = 0uL; i < bufferSize; i++){
            if (freezeFrameBuffer[i].eventId == idToFind){
                memset(&freezeFrameBuffer[i], 0, sizeof(FreezeFrameRecType));
                ffDeleted = TRUE;
            }
        }

        if( TRUE == ffDeleted ) {
            /* IMPROVEMENT: Immediate storage when deleting data? */
#if (DEM_PRESTORAGE_FF_DATA_IN_MEM)
        	if ((TRUE == eventParam->EventClass->FFPrestorageSupported) && (origin == DEM_DTC_ORIGIN_NOT_USED) ) {
        		Dem_NvM_SetPreStoreFreezeFrameBlockChanged(FALSE);
        	} else {
        		 Dem_NvM_SetFreezeFrameBlockChanged(origin, FALSE);
        	}
#else
        	Dem_NvM_SetFreezeFrameBlockChanged(origin, FALSE);
#endif
        }
    }
    return ffDeleted;
}
#endif /* DEM_USE_MEMORY_FUNCTIONS */
/*
 * Procedure:   storeFreezeFrameDataEvtMem
 * Description: Store the freeze frame data in event memory according to
 *              "eventParam" destination option
 */
static boolean storeFreezeFrameDataEvtMem(const Dem_EventParameterType *eventParam, FreezeFrameRecType *freezeFrame,
                                       Dem_FreezeFrameKindType ffKind)
{
    boolean ret = FALSE;
    switch (eventParam->EventClass->EventDestination) {
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM)
            getFreezeFrameData(eventParam, freezeFrame, ffKind, DEM_DTC_ORIGIN_PRIMARY_MEMORY, TRUE);
            if (freezeFrame->eventId != DEM_EVENT_ID_NULL) {
                if(freezeFrame->kind == DEM_FREEZE_FRAME_OBD){
                    ret = storeOBDFreezeFrameDataMem(eventParam, freezeFrame,priMemFreezeFrameBuffer,
                            DEM_MAX_NUMBER_FF_DATA_PRI_MEM, DEM_DTC_ORIGIN_PRIMARY_MEMORY);
                }
                else {
                    ret = storeFreezeFrameDataMem(eventParam, freezeFrame, priMemFreezeFrameBuffer,
                                            DEM_MAX_NUMBER_FF_DATA_PRI_MEM, DEM_DTC_ORIGIN_PRIMARY_MEMORY); /** @req DEM190 */
                }
            }
#endif
            break;
        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
#if ((DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_SEC_MEM)
            getFreezeFrameData(eventParam, freezeFrame, ffKind, DEM_DTC_ORIGIN_SECONDARY_MEMORY, TRUE);
            if (freezeFrame->eventId != DEM_EVENT_ID_NULL) {
                if(freezeFrame->kind == DEM_FREEZE_FRAME_OBD){
                    DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GLOBAL_ID, DEM_E_OBD_NOT_ALLOWED_IN_SEC_MEM);
                }
                else {
                    ret = storeFreezeFrameDataMem(eventParam, freezeFrame, secMemFreezeFrameBuffer,
                                            DEM_MAX_NUMBER_FF_DATA_SEC_MEM, DEM_DTC_ORIGIN_SECONDARY_MEMORY); /** @req DEM190 */
                }
            }
#endif
            break;
        case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
        case DEM_DTC_ORIGIN_MIRROR_MEMORY:
            // Not yet supported
            DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GLOBAL_ID, DEM_E_NOT_IMPLEMENTED_YET);
            break;
        default:
            (void)freezeFrame; /*lint !e920 Avoid compiler warning (variable not used) */
            (void)ffKind;
            break;
    }
    return ret;
}

/*
 * Procedure:   lookupFreezeFrameDataRecNumParam
 * Description: Returns TRUE if the requested freezeFrame data number was found among the configured records for the event.
 *              "freezeFrameClassPtr" returns a pointer to the record class.
 */
static boolean lookupFreezeFrameDataRecNumParam(uint8 recordNumber, const Dem_EventParameterType *eventParam, Dem_FreezeFrameClassType const **freezeFrameClassPtr)
{
    boolean recNumFound = FALSE;
    uint8 maxNofRecords;
    const Dem_FreezeFrameRecNumClass *FreezeFrameRecNumClass;
    Dem_FreezeFrameClassTypeRefIndex ffIdx = DEM_FF_NULLREF;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
    if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID ) {
        const Dem_CombinedDTCCfgType *CombDTCCfg = &configSet->CombinedDTCConfig[eventParam->CombinedDTCCID];
        maxNofRecords = CombDTCCfg->MaxNumberFreezeFrameRecords;
        FreezeFrameRecNumClass = CombDTCCfg->FreezeFrameRecNumClassRef;
        ffIdx = CombDTCCfg->Calib->FreezeFrameClassIdx;
    }
    else {
        maxNofRecords = eventParam->MaxNumberFreezeFrameRecords;
        FreezeFrameRecNumClass = eventParam->FreezeFrameRecNumClassRef;
        ffIdx = *(eventParam->FreezeFrameClassRefIdx);
    }
#else
    maxNofRecords = eventParam->MaxNumberFreezeFrameRecords;
    FreezeFrameRecNumClass = eventParam->FreezeFrameRecNumClassRef;
    ffIdx = *(eventParam->FreezeFrameClassRefIdx);
#endif

    if ( (ffIdx != DEM_FF_NULLREF) && (NULL != FreezeFrameRecNumClass)) {
        for( uint8 i = 0; (i < maxNofRecords) && (FALSE == recNumFound); i++ ) {
            if( FreezeFrameRecNumClass->FreezeFrameRecordNumber[i] == recordNumber ) {
                recNumFound = TRUE;
                getFFClassReference(eventParam, (Dem_FreezeFrameClassType **) freezeFrameClassPtr);
            }
        }
    }

    return recNumFound;
}

/*
 * Procedure:   lookupFreezeFrameDataSize
 * Description: Returns TRUE if the requested freezeFrame data size was obtained successfully from the configuration.
 *              "dataSize" returns a pointer to the data size.
 */
static boolean lookupFreezeFrameDataSize(uint8 recordNumber, const Dem_FreezeFrameClassType const  * const *freezeFrameClassPtr, uint16 *dataSize)
{
    boolean dataSizeFound = FALSE;
    uint16 i;

    (void)recordNumber; /* Avoid compiler warning - can this be removed */
    *dataSize = 0;
    if (*freezeFrameClassPtr != NULL) {
        dataSizeFound = TRUE;
        for (i = 0u; (i < DEM_MAX_NR_OF_DIDS_IN_FREEZEFRAME_DATA) && ((*freezeFrameClassPtr)->FFIdClassRef[i]->Arc_EOL != TRUE); i++) {
            *dataSize += (uint16)(*freezeFrameClassPtr)->FFIdClassRef[i]->PidOrDidSize + DEM_DID_IDENTIFIER_SIZE_OF_BYTES;
        }
    }

    return dataSizeFound;
}

/**
 * Looks for a stored freeze frame with correct record number has been stored for an event
 * @param eventId
 * @param recordNumber
 * @param dtcOrigin
 * @param freezeFrame
 * @return TRUE: if a freeze frame with the correct record number was stored for this event, FALSE: Otherwise
 */
static boolean getStoredFreezeFrame(Dem_EventIdType eventId, uint8 recordNumber, Dem_DTCOriginType dtcOrigin, FreezeFrameRecType **freezeFrame)
{
    boolean ffFound = FALSE;

    FreezeFrameRecType* freezeFrameBuffer = NULL;
    uint32 freezeFrameBufferSize = 0;

    switch (dtcOrigin) {
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM)
            freezeFrameBuffer = priMemFreezeFrameBuffer;
            freezeFrameBufferSize = DEM_MAX_NUMBER_FF_DATA_PRI_MEM;
#endif
            break;
        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
#if ((DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_SEC_MEM)
            freezeFrameBuffer = secMemFreezeFrameBuffer;
            freezeFrameBufferSize = DEM_MAX_NUMBER_FF_DATA_SEC_MEM;
#endif
            break;
        default:
            DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GLOBAL_ID, DEM_E_NOT_IMPLEMENTED_YET);
            break;
    }

    if (freezeFrameBuffer != NULL) {

        for (uint32 i = 0uL; (i < freezeFrameBufferSize) && (FALSE == ffFound); i++) {
            ffFound = ((freezeFrameBuffer[i].eventId == eventId) && (freezeFrameBuffer[i].recordNumber == recordNumber))? TRUE: FALSE;
            if(TRUE == ffFound) {
                *freezeFrame = &freezeFrameBuffer[i];
            }
        }
    }
    return ffFound;
}

#if (DEM_USE_TIMESTAMPS == STD_ON) && (DEM_EVENT_COMB_TYPE2_REPORT_OLDEST_DATA == STD_ON)
/**
 * Gets and updates freeze frame data if older than provided timestamp
 * @param eventId
 * @param recordNumber
 * @param dtcOrigin
 * @param destBuffer
 * @param bufSize
 * @param FFDataSize
 * @param timestamp
 * @param checkTimeStamp
 * @return
 */
static boolean getOlderFreezeFrameRecord(Dem_EventIdType eventId, uint8 recordNumber, Dem_DTCOriginType dtcOrigin, uint8* destBuffer, uint16* bufSize,
        const uint16 FFDataSize, uint32 *timestamp, boolean checkTimeStamp)
{

    boolean ffFound = FALSE;

    FreezeFrameRecType* freezeFrame = NULL;

    if( TRUE == getStoredFreezeFrame(eventId, recordNumber, dtcOrigin, &freezeFrame) ) {
        if( (FALSE == checkTimeStamp) || (*timestamp > freezeFrame->timeStamp) ) {
            memcpy(destBuffer, freezeFrame->data, FFDataSize); /** @req DEM071 */
            *bufSize = FFDataSize;
            *timestamp = freezeFrame->timeStamp;

            ffFound = TRUE;
        }
    }
    return ffFound;
}
#else
/*
 * Procedure:   getFreezeFrameRecord
 * Description: Returns TRUE if the requested event id is found, "freezeFrame" points to the found data.
 */
static boolean getFreezeFrameRecord(Dem_EventIdType eventId, uint8 recordNumber, Dem_DTCOriginType dtcOrigin, uint8* destBuffer, uint16*  bufSize, const uint16 FFDataSize)
{

    boolean ffFound = FALSE;

    FreezeFrameRecType* freezeFrame = NULL;

    if(TRUE == getStoredFreezeFrame(eventId, recordNumber, dtcOrigin, &freezeFrame)) {

        memcpy(destBuffer, freezeFrame->data, FFDataSize); /** @req DEM071 */
        *bufSize = FFDataSize;

        ffFound = TRUE;
    }
    return ffFound;
}
#endif
/**
 * Gets conditions for data storage
 * @param eventFailedNow
 * @param eventDataUppdated
 * @param extensionStorageBitfield
 * @param storeFFData
 * @param storeExtData
 * @param overrideOldExtdata
 */
static void getStorageConditions(boolean eventFailedNow, boolean eventDataUpdated, uint8 extensionStorageBitfield, boolean *storeFFData, boolean *storeExtData, boolean *overrideOldExtData)
{
#if defined(DEM_EXTENDED_DATA_CAPTURE_EVENT_MEMORY_STORAGE)
                    *storeExtData = eventDataUpdated;
                    *overrideOldExtData = FALSE;
#elif defined(DEM_EXTENDED_DATA_CAPTURE_TESTFAILED)
                    *storeExtData = eventFailedNow;
                    *overrideOldExtData = FALSE;
#elif defined(DEM_EXTENDED_DATA_CAPTURE_EXTENSION)
                    /* @req OEM_DEM_10169 DEM_TRIGGER_EXTENSION */
                    *overrideOldExtData = (0 != (extensionStorageBitfield & DEM_EXT_CLEAR_BEFORE_STORE_EXT_DATA_BIT));
                    *storeExtData = (0 != (extensionStorageBitfield & DEM_EXT_STORE_EXT_DATA_BIT));
#else
                    *storeExtData = FALSE;
                    *overrideOldExtData = FALSE;
#endif
#if defined(DEM_FREEZE_FRAME_CAPTURE_EVENT_MEMORY_STORAGE)
                    *storeFFData = eventDataUpdated;
#elif defined(DEM_FREEZE_FRAME_CAPTURE_TESTFAILED)
                    *storeFFData = eventFailedNow;
#elif defined(DEM_FREEZE_FRAME_CAPTURE_EXTENSION)
                    /* @req OEM_DEM_10170 DEM_TRIGGER_EXTENSION */
                    *storeFFData = (0 != (extensionStorageBitfield & DEM_EXT_STORE_FF_BIT));
#else
                    *storeFFData = FALSE;
                    (void)eventFailedNow;  /* Avoid compiler warning */
                    (void)eventDataUpdated; /* Avoid compiler warning */
                    (void)extensionStorageBitfield; /* Avoid compiler warning */
#endif
}

/*
 * Procedure:   handlePreInitEvent
 * Description: Handle the updating of event status and storing of
 *              event related data in preInit buffers.
 */
static void handlePreInitEvent(Dem_EventIdType eventId, Dem_EventStatusType eventStatus)
{
    const Dem_EventParameterType *eventParam;
    EventStatusRecType *eventStatusRec = NULL;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
    Dem_EventStatusExtendedType newDTCStatus = DEM_DEFAULT_EVENT_STATUS;
    Dem_EventStatusExtendedType oldDTCStatus = DEM_DEFAULT_EVENT_STATUS;
#endif
    lookupEventIdParameter(eventId, &eventParam);
    if (eventParam != NULL) {
        if ( TRUE == operationCycleIsStarted(eventParam->EventClass->OperationCycleRef) ) {
            lookupEventStatusRec(eventId, &eventStatusRec);
            if( NULL != eventStatusRec ) {
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
                if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID ) {
                    if( DEM_STATUS_OK != GetDTCUDSStatus(eventParam->DTCClassRef, eventParam->EventClass->EventDestination, &oldDTCStatus) ) {
                        oldDTCStatus = eventStatusRec->eventStatusExtended;
                    }
                }
                else {
                    oldDTCStatus = eventStatusRec->eventStatusExtended;
                }
#endif
                updateEventStatusRec(eventParam, eventStatus, eventStatusRec);
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
                if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID ) {
                    if( DEM_STATUS_OK != GetDTCUDSStatus(eventParam->DTCClassRef, eventParam->EventClass->EventDestination, &newDTCStatus) ) {
                        newDTCStatus = eventStatusRec->eventStatusExtended;
                    }
                }
                else {
                    newDTCStatus = eventStatusRec->eventStatusExtended;
                }
#endif
                if ( (0 != eventStatusRec->errorStatusChanged) || (0 != eventStatusRec->extensionDataChanged) ) {
                    boolean storeExtData;
                    boolean overrideOldExtData;
                    boolean storeFFData;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
                    boolean dtcFailedNow = (((0u == (oldDTCStatus & DEM_TEST_FAILED))) && ((0 != (newDTCStatus & DEM_TEST_FAILED)))) ? TRUE: FALSE;
#else
                    boolean eventFailedNow = ((TRUE == eventStatusRec->errorStatusChanged) && (0 != (eventStatusRec->eventStatusExtended & DEM_TEST_FAILED)))? TRUE: FALSE;
#endif
                    /* Get conditions for data storage */
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
                    /* @req DEM163 */
                    getStorageConditions(dtcFailedNow, TRUE, eventStatusRec->extensionDataStoreBitfield, &storeFFData, &storeExtData, &overrideOldExtData);
#else
                    /* @req DEM539 */
                    getStorageConditions(eventFailedNow, TRUE, eventStatusRec->extensionDataStoreBitfield, &storeFFData, &storeExtData, &overrideOldExtData);
#endif
                    if( (TRUE == storeExtData) && (NULL != eventParam->ExtendedDataClassRef) ) {
                        (void)storeExtendedData(eventParam, overrideOldExtData);
                    }
#if( DEM_FF_DATA_IN_PRE_INIT )
                    if( TRUE == storeFFData) {
                        FreezeFrameRecType freezeFrameLocal;
                        if( DEM_FF_NULLREF != getFFIdx(eventParam) ) {
#if defined(DEM_FREEZE_FRAME_CAPTURE_EXTENSION)
                            /* Allow extension to decide if ffs should be deleted before storing */
                            if( 0 != (eventStatusRec->extensionDataStoreBitfield & DEM_EXT_CLEAR_BEFORE_STORE_FF_BIT) ) {
                                deleteFreezeFrameDataPreInit(eventParam);
                            }
#endif
                            getFreezeFrameData(eventParam, &freezeFrameLocal, DEM_FREEZE_FRAME_NON_OBD, DEM_DTC_ORIGIN_NOT_USED, TRUE);
                            if (freezeFrameLocal.eventId != DEM_EVENT_ID_NULL) {
                                storeFreezeFrameDataPreInit(eventParam, &freezeFrameLocal);
                            }
                        }
                        if( (NULL != eventParam->DTCClassRef) && (DEM_NON_EMISSION_RELATED != (Dem_Arc_EventDTCKindType) *eventParam->EventDTCKind) ) {
                            getFreezeFrameData(eventParam, &freezeFrameLocal, DEM_FREEZE_FRAME_OBD, DEM_DTC_ORIGIN_NOT_USED, TRUE);
                            if (freezeFrameLocal.eventId != DEM_EVENT_ID_NULL) {
                                storeFreezeFrameDataPreInit(eventParam, &freezeFrameLocal);
                            }
                        }
                    }
#endif /* DEM_FF_DATA_IN_PRE_INIT */
                }
            }
        }
        else {
            // Operation cycle not set or not started
            // IMPROVEMENT: Report error?
        }
    }
    else {
        // Event ID not configured
        // IMPROVEMENT: Report error?
    }
}

#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
static boolean enableConditionsSet(const Dem_EventClassType *eventClass)
{
    /* @req DEM449 */
    /* @req DEM450 */
    boolean conditionsSet = TRUE;
    if( NULL != eventClass->EnableConditionGroupRef ) {
        /* Each group must reference at least one enable condition. Or this won't work.. */
        const Dem_EnableConditionGroupType *enableConditionGroupPtr = eventClass->EnableConditionGroupRef;
        for( uint8 i = 0; i < enableConditionGroupPtr->nofEnableConditions; i++ ) {
            if( FALSE == DemEnableConditions[enableConditionGroupPtr->EnableCondition[i]->EnableConditionID] ) {
                conditionsSet = FALSE;
            }
        }
    }
    return conditionsSet;
}
#endif

/**
 * Checks whether DTC setting for event is disabled. If the event does not have a DTC (or its DTC is suppressed)
 * setting is NOT disabled.
 * @param eventParam
 * @return TRUE: Disabled, FALSE: NOT disabled
 */
static boolean DTCSettingDisabled(const Dem_EventParameterType *eventParam)
{
    /* @req DEM587 */
    boolean eventDTCSettingDisabled = FALSE;
    if( (disableDtcSetting.settingDisabled == TRUE) && (NULL != eventParam->DTCClassRef) && (DTCIsAvailable(eventParam->DTCClassRef) == TRUE) ) {
        /* DTC setting is disabled and the event has a DTC (which is not suppressed) */
        if( (checkDtcGroup(disableDtcSetting.dtcGroup, eventParam, DEM_DTC_FORMAT_UDS) == TRUE) &&
                (checkDtcKind(disableDtcSetting.dtcKind, eventParam) == TRUE) ) {
            /* Setting of DTC for this event is disabled. */
            eventDTCSettingDisabled = TRUE;
        }
    }
    return eventDTCSettingDisabled;
}

/**
 * Checks whether event processing is allowed
 * @param eventParam
 * @return TRUE: Processing allowed, FALSE: Processing not allowed
 */
static boolean eventProcessingAllowed(const Dem_EventParameterType *eventParam)
{
    /* Event processing is not allowed if event has DTC and DTC setting has been disabled,
     * or if event has enable conditions and these are not set. */
    if ( ( FALSE == DTCSettingDisabled(eventParam) ) /* @req DEM626 */
#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
            && (TRUE == enableConditionsSet(eventParam->EventClass))/* @req DEM447 */
#endif
        )  {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * Checks if condition for storing OBD freeze frame is fulfilled
 * @param eventParam
 * @param oldStatus
 * @param status
 * @return
 */
static boolean checkOBDFFStorageCondition(Dem_EventStatusExtendedType oldStatus, Dem_EventStatusExtendedType status)
{
    if( (0u == (oldStatus & DEM_CONFIRMED_DTC)) && (0u != (status & DEM_CONFIRMED_DTC)) ) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * Get index for event FF
 * @param eventParam
 * @return DEM_FF_NULLREF: no FF configured
 */
static Dem_FreezeFrameClassTypeRefIndex getFFIdx(const Dem_EventParameterType *eventParam)
{
    Dem_FreezeFrameClassTypeRefIndex ffIdx = DEM_FF_NULLREF;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
    if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID ) {
        const Dem_CombinedDTCCfgType *CombDTCCfg = &configSet->CombinedDTCConfig[eventParam->CombinedDTCCID];
        ffIdx = CombDTCCfg->Calib->FreezeFrameClassIdx;
    }
    else {
        if( NULL_PTR != eventParam->FreezeFrameClassRefIdx ) {
            ffIdx = *(eventParam->FreezeFrameClassRefIdx);
        }
    }
#else
    if( NULL_PTR != eventParam->FreezeFrameClassRefIdx ) {
        ffIdx = *(eventParam->FreezeFrameClassRefIdx);
    }
#endif
    return ffIdx;
}

#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
/**
 * Gets the status of DTC referenced bu event
 * @param eventParam
 * @param eventStatus
 * @return Status of DTC
 */
static Dem_EventStatusExtendedType getEventDTCStatus(const Dem_EventParameterType *eventParam, Dem_EventStatusExtendedType eventStatus)
{
    Dem_EventStatusExtendedType DTCStatus = DEM_DEFAULT_EVENT_STATUS;
    if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID ) {
        if( DEM_STATUS_OK != GetDTCUDSStatus(eventParam->DTCClassRef, eventParam->EventClass->EventDestination, &DTCStatus) ) {
            DTCStatus = eventStatus;
        }
    }
    else {
        DTCStatus = eventStatus;
    }
    return DTCStatus;
}
#endif

/*
 * Procedure:   handleEvent
 * Description: Handle the updating of event status and storing of
 *              event related data in event memory.
 */
Std_ReturnType handleEvent(Dem_EventIdType eventId, Dem_EventStatusType eventStatus)
{
    Std_ReturnType returnCode = E_OK;
    const Dem_EventParameterType *eventParam;
    EventStatusRecType *eventStatusRec;
    FreezeFrameRecType freezeFrameLocal;
    Dem_EventStatusExtendedType oldEventStatus = DEM_DEFAULT_EVENT_STATUS;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
    Dem_EventStatusExtendedType newDTCStatus = DEM_DEFAULT_EVENT_STATUS;
    Dem_EventStatusExtendedType oldDTCStatus = DEM_DEFAULT_EVENT_STATUS;
#endif
    Std_ReturnType eventStoreStatus = E_OK;

    lookupEventIdParameter(eventId, &eventParam);
    lookupEventStatusRec(eventId, &eventStatusRec);
    if ( (eventParam != NULL) && (NULL != eventStatusRec) && (TRUE == eventStatusRec->isAvailable) ) {
        if ( TRUE == operationCycleIsStarted(eventParam->EventClass->OperationCycleRef) ) {/* @req DEM481 */
            /* Check if event processing is allowed (DTC setting, enable condition) */
            if ( TRUE == eventProcessingAllowed(eventParam))  {
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
                oldDTCStatus = getEventDTCStatus(eventParam, eventStatusRec->eventStatusExtended);
#endif
                oldEventStatus = eventStatusRec->eventStatusExtended;

                updateEventStatusRec(eventParam, eventStatus, eventStatusRec);
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
                newDTCStatus = getEventDTCStatus(eventParam, eventStatusRec->eventStatusExtended);
#endif
                if ( (0 != eventStatusRec->errorStatusChanged) || (0 != eventStatusRec->extensionDataChanged) ) {
                    boolean eventFailedNow = ((TRUE == eventStatusRec->errorStatusChanged) && (0 != (eventStatusRec->eventStatusExtended & DEM_TEST_FAILED)))? TRUE: FALSE;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
                    boolean dtcFailedNow = (((0u == (oldDTCStatus & DEM_TEST_FAILED))) && ((0 != (newDTCStatus & DEM_TEST_FAILED)))) ? TRUE: FALSE;
#endif
                    eventStoreStatus = storeEventEvtMem(eventParam, eventStatusRec, eventFailedNow); /** @req DEM184 *//** @req DEM396 */
                    boolean storeExtData;
                    boolean storeFFData;
                    boolean overrideOldExtData;
                    boolean eventDataUpdated = (E_OK == eventStoreStatus)? TRUE: FALSE;

                    /* Get conditions for data storage */
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
                    /* @req DEM163 */
                    getStorageConditions(dtcFailedNow, eventDataUpdated, eventStatusRec->extensionDataStoreBitfield, &storeFFData, &storeExtData, &overrideOldExtData);
#else
                    /* @req DEM539 */
                    getStorageConditions(eventFailedNow, eventDataUpdated, eventStatusRec->extensionDataStoreBitfield, &storeFFData, &storeExtData, &overrideOldExtData);
#endif
                    if( FALSE == eventDTCRecordDataUpdateDisabled(eventParam) ) {
                        if ( (TRUE == storeExtData) && (NULL != eventParam->ExtendedDataClassRef) ) {
                            if( TRUE == storeExtendedData(eventParam, overrideOldExtData) ) {
                                eventDataUpdated = TRUE;
                            }
                        }
                        if ( TRUE == storeFFData ) {
                            if( DEM_FF_NULLREF != getFFIdx(eventParam) ) {
#if defined(DEM_USE_MEMORY_FUNCTIONS) && defined(DEM_FREEZE_FRAME_CAPTURE_EXTENSION)
                                /* Allow extension to decide if ffs should be deleted before storing */
                                if( 0 != (eventStatusRec->extensionDataStoreBitfield & DEM_EXT_CLEAR_BEFORE_STORE_FF_BIT) ) {
                                    if( TRUE == deleteFreezeFrameDataMem(eventParam, eventParam->EventClass->EventDestination, FALSE) ) {
                                        eventDataUpdated = TRUE;
                                    }
                                }
#endif
                                if( TRUE == storeFreezeFrameDataEvtMem(eventParam, &freezeFrameLocal, DEM_FREEZE_FRAME_NON_OBD) ) { /** @req DEM190 */
                                    eventDataUpdated = TRUE;
                                }
                        	}
                            if( (DEM_NON_EMISSION_RELATED != (Dem_Arc_EventDTCKindType) *(eventParam->EventDTCKind)) &&
                                    (TRUE == eventDataUpdated) &&
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
                                    (TRUE == checkOBDFFStorageCondition(oldDTCStatus, newDTCStatus))
#else
                                    (TRUE == checkOBDFFStorageCondition(oldEventStatus, eventStatusRec->eventStatusExtended))
#endif
                                    ) {
                                if( TRUE == storeFreezeFrameDataEvtMem(eventParam, &freezeFrameLocal, DEM_FREEZE_FRAME_OBD) ) { /** @req DEM190 */
                                    eventDataUpdated = TRUE;
                                }
                            }
                        }
                    }
                    if( TRUE == eventDataUpdated ) {
                        /* @req DEM475 */
                        notifyEventDataChanged(eventParam);
                    }
                }
                if( E_NOT_OK == eventStoreStatus ) {
                    /* Tried to store event but did not succeed (eventStoreStatus initialized to E_OK).
                     * Make sure confirmed bit is not set. */
                    eventStatusRec->eventStatusExtended &= ~DEM_CONFIRMED_DTC;
                }
                if( oldEventStatus != eventStatusRec->eventStatusExtended ) {
                    /* @req DEM016 */
                    notifyEventStatusChange(eventStatusRec->eventParamRef, oldEventStatus, eventStatusRec->eventStatusExtended);
                }
#if defined(DEM_USE_INDICATORS) && defined(DEM_USE_MEMORY_FUNCTIONS)
                if((TRUE == eventStatusRec->indicatorDataChanged) && (TRUE == isInEventMemory(eventParam))) {
                    storeEventIndicators(eventParam);
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
                    boolean immediateStorage = FALSE;
                    if( (NULL != eventParam->DTCClassRef) && (TRUE == eventParam->DTCClassRef->DTCRef->ImmediateNvStorage) &&
                            (eventStatusRec->occurrence <= DEM_IMMEDIATE_NV_STORAGE_LIMIT)) {
                        immediateStorage = TRUE;
                    }
                    Dem_NvM_SetIndicatorBlockChanged(immediateStorage);
#else
                    Dem_NvM_SetIndicatorBlockChanged(FALSE);
#endif
                }
#endif
#if (DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON)
                if(TRUE == handlePermanentDTCStorage(eventParam, eventStatusRec->eventStatusExtended)) {
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
                    boolean immediateStorage = FALSE;
                    if( (NULL != eventParam->DTCClassRef) && (TRUE == eventParam->DTCClassRef->DTCRef->ImmediateNvStorage) && (eventStatusRec->occurrence <= DEM_IMMEDIATE_NV_STORAGE_LIMIT)) {
                        immediateStorage = TRUE;
                    }
                    Dem_NvM_SetPermanentBlockChanged(immediateStorage);
#else
                    Dem_NvM_SetPermanentBlockChanged(FALSE);
#endif
                }
#endif
            } else {
                /* Enable conditions not set or DTC disabled */
                returnCode = E_NOT_OK;
            }
        } else {
            returnCode = E_NOT_OK; // Operation cycle not valid or not started /* @req DEM482 */
        }
    } else {
        returnCode = E_NOT_OK; // Event ID not configured or set to not available
    }

    return returnCode;
}

/*
 * Procedure:   resetEventStatus
 * Description: Resets the events status of eventId.
 */
static Std_ReturnType resetEventStatus(Dem_EventIdType eventId)
{
    EventStatusRecType *eventStatusRecPtr;
    Std_ReturnType ret = E_OK;
    lookupEventStatusRec(eventId, &eventStatusRecPtr);
    if (eventStatusRecPtr != NULL) {
        if( (TRUE == eventStatusRecPtr->isAvailable) && (0 != (eventStatusRecPtr->eventStatusExtended & DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE)) ) {
            Dem_EventStatusExtendedType oldStatus = eventStatusRecPtr->eventStatusExtended;
            eventStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)~DEM_TEST_FAILED; /** @req DEM187 */
            resetDebounceCounter(eventStatusRecPtr);
            if( oldStatus != eventStatusRecPtr->eventStatusExtended ) {
                /* @req DEM016 */
                notifyEventStatusChange(eventStatusRecPtr->eventParamRef, oldStatus, eventStatusRecPtr->eventStatusExtended);
            }
            /* NOTE: Should we store in "event destination" if DEM_TEST_FAILED_STORAGE == STD_ON) */
#if 0
#if DEM_TEST_FAILED_STORAGE == STD_ON
            if((0 != (oldStatus & DEM_TEST_FAILED)) && (E_OK == storeEventEvtMem(eventStatusRecPtr->eventParamRef, eventStatusRecPtr))) {
                notifyEventDataChanged(eventStatusRecPtr->eventParamRef);
            }
#endif
#endif
        } else {
            /* @req DEM638 */
            ret = E_NOT_OK;
        }
    }
    return ret;
}


/*
 * Procedure:   getEventStatus
 * Description: Returns the extended event status bitmask of eventId in "eventStatusExtended".
 */
static Std_ReturnType getEventStatus(Dem_EventIdType eventId, Dem_EventStatusExtendedType *eventStatusExtended)
{
    Std_ReturnType ret = E_OK;
    EventStatusRecType eventStatusLocal;

    // Get recorded status
    getEventStatusRec(eventId, &eventStatusLocal);
    if ( (eventStatusLocal.eventId == eventId) && (TRUE == eventStatusLocal.isAvailable) ) {
        *eventStatusExtended = eventStatusLocal.eventStatusExtended; /** @req DEM051 */
    }
    else {
        // Event Id not found, no report received.
        *eventStatusExtended = DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE | DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR;
        ret = E_NOT_OK;
    }
    return ret;
}

/*
 * Procedure:   getEventFailed
 * Description: Returns the TRUE or FALSE of "eventId" in "eventFailed" depending on current status.
 */
static Std_ReturnType getEventFailed(Dem_EventIdType eventId, boolean *eventFailed)
{
    Std_ReturnType ret = E_OK;
    EventStatusRecType eventStatusLocal;

    // Get recorded status
    getEventStatusRec(eventId, &eventStatusLocal);
    if ( (eventStatusLocal.eventId == eventId) && (TRUE == eventStatusLocal.isAvailable) ) {
        if ( 0 != (eventStatusLocal.eventStatusExtended & DEM_TEST_FAILED)) { /** @req DEM052 */
            *eventFailed = TRUE;
        }
        else {
            *eventFailed = FALSE;
        }
    }
    else {
        // Event Id not found or not available.
        *eventFailed = FALSE;
        ret = E_NOT_OK;
    }
    return ret;
}

/*
 * Procedure:   getEventTested
 * Description: Returns the TRUE or FALSE of "eventId" in "eventTested" depending on
 *              current status the "test not completed this operation cycle" bit.
 */
static Std_ReturnType getEventTested(Dem_EventIdType eventId, boolean *eventTested)
{
    Std_ReturnType ret = E_OK;
    EventStatusRecType eventStatusLocal;

    // Get recorded status
    getEventStatusRec(eventId, &eventStatusLocal);
    if ( (eventStatusLocal.eventId == eventId) && (TRUE == eventStatusLocal.isAvailable) )  {
        if ( 0 == (eventStatusLocal.eventStatusExtended & DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE)) { /** @req DEM053 */
            *eventTested = TRUE;
        }
        else {
            *eventTested = FALSE;
        }
    }
    else {
        // Event Id not found, not tested.
        *eventTested = FALSE;
        ret = E_NOT_OK;
    }
    return ret;
}

#if defined(DEM_AGING_PROCESSING_DEM_INTERNAL) && defined(DEM_USE_MEMORY_FUNCTIONS)
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1) && defined(DEM_COMBINED_DTC_STATUS_AGING)
/**
 * Gets the combined status (passed/failed) during aging cycle
 * @param eventParam
 * @param passed
 * @param failed
 * @return
 */
static Std_ReturnType getCombTypeStatusDuringAgingCycle(const Dem_EventParameterType *eventParam, boolean *passed, boolean *failed)
{
    Std_ReturnType ret = E_NOT_OK;
    const Dem_CombinedDTCCfgType *CombDTCCfg;
    const Dem_DTCClassType *DTCClass;
    EventStatusRecType* eventStatusRecPtr;
    *failed = FALSE;
    *passed = TRUE;
    if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID ) {
        ret = E_OK;
        CombDTCCfg = &configSet->CombinedDTCConfig[eventParam->CombinedDTCCID];
        DTCClass = CombDTCCfg->DTCClassRef;
        for(uint16 i = 0; (i < DTCClass->NofEvents) && (E_OK == ret); i++) {
            if( eventParam->EventID != DTCClass->Events[i] ) {
                eventStatusRecPtr = NULL_PTR;
                lookupEventStatusRec(DTCClass->Events[i], &eventStatusRecPtr);
                if( NULL_PTR != eventStatusRecPtr ) {
                    if( FALSE == eventStatusRecPtr->passedDuringAgingCycle  ) {
                        *passed = FALSE;
                    }
                    if( TRUE == eventStatusRecPtr->failedDuringAgingCycle  ) {
                        *failed = FALSE;
                    }
                }
                else {
                    /* This is unexpected */
                    ret = E_NOT_OK;
                }
            }
        }
    }
    return ret;
}
#endif

#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
/**
 * Deletes all DTC data if a combined DTC is aged
 * @param eventParam
 * @return TRUE: data was deleted, FALSE: No data deleted
 */
static boolean deletDTCDataIfAged(const Dem_EventParameterType *eventParam)
{
    boolean dataDeleted = FALSE;
    boolean dummy;
    if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID ) {
        Dem_EventStatusExtendedType DTCStatus;
        if( DEM_STATUS_OK == GetDTCUDSStatus(eventParam->DTCClassRef, eventParam->EventClass->EventDestination, &DTCStatus) ) {
            if( 0u == (DTCStatus & DEM_CONFIRMED_DTC) ) {
                /* Confirmed bit was cleared for combined event *//* @req DEM442 */
                if( TRUE == DeleteDTCData(eventParam, FALSE, &dummy, TRUE) ) {
                    dataDeleted = TRUE;
                }
            }
        }
    }
    return dataDeleted;
}
#endif

static boolean ageEvent(EventStatusRecType* evtStatusRecPtr) {
    /* @req DEM643 */
    boolean updatedMemory = FALSE;
    boolean dummy;
    boolean eventDeleted = FALSE;
    Dem_EventStatusExtendedType oldStatus = evtStatusRecPtr->eventStatusExtended;
    boolean passedDuringAgingCycle;
    boolean failedDuringAgingCycle;

    /* @req DEM489 *//* If it is confirmed it should be stored in event memory */
    if( 0u != (evtStatusRecPtr->eventStatusExtended & DEM_CONFIRMED_DTC)) {
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1) && defined(DEM_COMBINED_DTC_STATUS_AGING)
        if( DEM_COMBINED_EVENT_NO_DTC_ID != evtStatusRecPtr->eventParamRef->CombinedDTCCID ) {
            passedDuringAgingCycle = evtStatusRecPtr->passedDuringAgingCycle;
            failedDuringAgingCycle = evtStatusRecPtr->failedDuringAgingCycle;
            if( E_OK != getCombTypeStatusDuringAgingCycle(evtStatusRecPtr->eventParamRef, &passedDuringAgingCycle, &failedDuringAgingCycle) ) {
                passedDuringAgingCycle = FALSE;
                failedDuringAgingCycle = FALSE;
            }
        }
        else {
            passedDuringAgingCycle = evtStatusRecPtr->passedDuringAgingCycle;
            failedDuringAgingCycle = evtStatusRecPtr->failedDuringAgingCycle;
        }
#else
        passedDuringAgingCycle = evtStatusRecPtr->passedDuringAgingCycle;
        failedDuringAgingCycle = evtStatusRecPtr->failedDuringAgingCycle;
#endif
        if( (TRUE == passedDuringAgingCycle) && (FALSE == evtStatusRecPtr->failedDuringAgingCycle) ) {
            /* Event was PASSED but NOT FAILED during aging cycle.
             * Increment aging counter */
            if( evtStatusRecPtr->agingCounter < DEM_AGING_CNTR_MAX ) {
                evtStatusRecPtr->agingCounter++;
                /* Set the flag,start up the storage of NVRam in main function. */
                updatedMemory = TRUE;
            }
            if((NULL != evtStatusRecPtr->eventParamRef->EventClass->AgingCycleCounterThresholdPtr) &&
                     (evtStatusRecPtr->agingCounter >= *evtStatusRecPtr->eventParamRef->EventClass->AgingCycleCounterThresholdPtr) ) { /* @req DEM493 */
                /* @req DEM497 *//* Delete ff and ext data */
                /* @req DEM161 */
                /* @req DEM541 */
                evtStatusRecPtr->agingCounter = 0;
                /* !req DEM498 *//* IMPROVEMNT: Only reset confirmed bit */
                evtStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)(~DEM_CONFIRMED_DTC);
                evtStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)(~DEM_PENDING_DTC);
                if(TRUE == DeleteDTCData(evtStatusRecPtr->eventParamRef, FALSE, &dummy, FALSE)) {
                    /* Set the flag,start up the storage of NVRam in main function. */
                    updatedMemory = TRUE;
                    eventDeleted = TRUE;
                }

#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
                if( TRUE == deletDTCDataIfAged(evtStatusRecPtr->eventParamRef ) ) {
                    /* Set the flag,start up the storage of NVRam in main function. */
                    updatedMemory = TRUE;
                }
#endif
#if defined(DEM_FAILURE_PROCESSING_DEM_INTERNAL)
                evtStatusRecPtr->failureCounter = 0;
#endif
#if defined(USE_DEM_EXTENSION)
                Dem_Extension_HealedEvent(evtStatusRecPtr->eventId);
#endif
            }
        }
        else if( (TRUE == failedDuringAgingCycle) && (0u != evtStatusRecPtr->agingCounter)) {
            /* Event failed during the aging cycle. Reset aging counter */
            evtStatusRecPtr->agingCounter = 0;
            updatedMemory = TRUE;
        } else {
            /* Do nothing.. */
        }
    }
    if( oldStatus != evtStatusRecPtr->eventStatusExtended ) {
        /* @req DEM016 */
        notifyEventStatusChange(evtStatusRecPtr->eventParamRef, oldStatus, evtStatusRecPtr->eventStatusExtended);
    }
    if( TRUE == updatedMemory ) {
        if( FALSE == eventDeleted ) {
            if(E_OK == storeEventEvtMem(evtStatusRecPtr->eventParamRef, evtStatusRecPtr, FALSE) ) {
                /* @req DEM475 */
                notifyEventDataChanged(evtStatusRecPtr->eventParamRef);
            }
        } else {
            /* Event was deleted. So don't store again but notify event data changed */
            notifyEventDataChanged(evtStatusRecPtr->eventParamRef);
#if defined(DEM_USE_MEMORY_FUNCTIONS)
            /* IMPROVEMENT: Immediate storage when deleting data? */
            Dem_NvM_SetEventBlockChanged(evtStatusRecPtr->eventParamRef->EventClass->EventDestination, FALSE);
#endif
        }

    }
    return updatedMemory;
}


/*
 * Procedure:   handleAging
 * Description: according to the operation state of "operationCycleId" to "cycleState" , handle the aging relatived data
 *              Returns E_OK if operation was successful else E_NOT_OK.
 */
static Std_ReturnType handleAging(Dem_OperationCycleIdType operationCycleId)
{
    uint16 i;
    Std_ReturnType returnCode = E_OK;
    boolean agingUpdatedSecondaryMemory = FALSE;
    boolean agingUpdatedPrimaryMemory = FALSE;
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
    boolean immediateStoragePrimary = FALSE;
    boolean immediateStorageSecondary = FALSE;
#endif
    if (operationCycleId < DEM_OPERATION_CYCLE_ID_ENDMARK) {
        /** @req Dem490 */
        for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
            if(eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL){
                if(eventStatusBuffer[i].eventParamRef != NULL){
                    if(eventStatusBuffer[i].eventParamRef->EventClass != NULL){
                        if((eventStatusBuffer[i].eventParamRef->EventClass->AgingAllowed == TRUE)
                            && (eventStatusBuffer[i].eventParamRef->EventClass->AgingCycleRef == operationCycleId)) {
                            /* Loop all destination memories e.g. primary and secondary */
                            Dem_DTCOriginType origin = eventStatusBuffer[i].eventParamRef->EventClass->EventDestination;
                            if (origin == DEM_DTC_ORIGIN_SECONDARY_MEMORY) {
                                agingUpdatedSecondaryMemory = ageEvent(&eventStatusBuffer[i]);
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
                                if( (NULL != eventStatusBuffer[i].eventParamRef->DTCClassRef) && (TRUE == eventStatusBuffer[i].eventParamRef->DTCClassRef->DTCRef->ImmediateNvStorage) && (eventStatusBuffer[i].occurrence <= DEM_IMMEDIATE_NV_STORAGE_LIMIT)) {
                                    immediateStorageSecondary = TRUE;
                                }
#endif
                            } else if (origin == DEM_DTC_ORIGIN_PRIMARY_MEMORY) {
                                agingUpdatedPrimaryMemory = ageEvent(&eventStatusBuffer[i]);
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
                                if( (NULL != eventStatusBuffer[i].eventParamRef->DTCClassRef) && (TRUE == eventStatusBuffer[i].eventParamRef->DTCClassRef->DTCRef->ImmediateNvStorage) && (eventStatusBuffer[i].occurrence <= DEM_IMMEDIATE_NV_STORAGE_LIMIT)) {
                                    immediateStoragePrimary = TRUE;
                                }
#endif
                            }
                        }
                    }
                }
            }
        }
    } else {
        DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_SETOPERATIONCYCLESTATE_ID, DEM_E_PARAM_DATA);
        returnCode = E_NOT_OK;
    }
#if defined(DEM_USE_MEMORY_FUNCTIONS)
    if( TRUE == agingUpdatedPrimaryMemory ) {
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
        Dem_NvM_SetEventBlockChanged(DEM_DTC_ORIGIN_PRIMARY_MEMORY, immediateStoragePrimary);
#else
        Dem_NvM_SetEventBlockChanged(DEM_DTC_ORIGIN_PRIMARY_MEMORY, FALSE);
#endif
    }
    if( TRUE == agingUpdatedSecondaryMemory ) {
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
        Dem_NvM_SetEventBlockChanged(DEM_DTC_ORIGIN_SECONDARY_MEMORY, immediateStorageSecondary);
#else
        Dem_NvM_SetEventBlockChanged(DEM_DTC_ORIGIN_SECONDARY_MEMORY, FALSE);
#endif
    }
#endif
    return returnCode;

}
#endif

/**
 * Handles starting an operation cycle
 * @param operationCycleId
 */
static void operationCycleStart(Dem_OperationCycleIdType operationCycleId)
{
    Dem_EventStatusExtendedType oldStatus;
    operationCycleStateList[operationCycleId] = DEM_CYCLE_STATE_START;
    // Lookup event ID
    for (uint16 i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
        if( (eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL) && (TRUE == eventStatusBuffer[i].isAvailable) ) {
            if( eventStatusBuffer[i].eventParamRef->EventClass->OperationCycleRef == operationCycleId ) {
                oldStatus = eventStatusBuffer[i].eventStatusExtended;
                eventStatusBuffer[i].eventStatusExtended &= (Dem_EventStatusExtendedType)~DEM_TEST_FAILED_THIS_OPERATION_CYCLE;
                eventStatusBuffer[i].eventStatusExtended |= DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE;
                resetDebounceCounter(&eventStatusBuffer[i]);
                eventStatusBuffer[i].isDisabled = FALSE;

                if( oldStatus != eventStatusBuffer[i].eventStatusExtended ) {
                    /* @req DEM016 */
                    notifyEventStatusChange(eventStatusBuffer[i].eventParamRef, oldStatus, eventStatusBuffer[i].eventStatusExtended);
                }
                if( NULL != eventStatusBuffer[i].eventParamRef->CallbackInitMforE ) {
                    /* @req DEM376 */
                    (void)eventStatusBuffer[i].eventParamRef->CallbackInitMforE(DEM_INIT_MONITOR_RESTART);
                }

            }
#if defined(USE_DEM_EXTENSION)
            Dem_Extension_OperationCycleStart(operationCycleId, &eventStatusBuffer[i]);
#endif
        }
#if defined(DEM_FAILURE_PROCESSING_DEM_INTERNAL)
        if( (eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL) && ((Dem_OperationCycleIdType)*eventStatusBuffer[i].eventParamRef->EventClass->FailureCycleRef == operationCycleId) ) {
            eventStatusBuffer[i].failedDuringFailureCycle = FALSE;
            eventStatusBuffer[i].passedDuringFailureCycle = FALSE;
        }
#endif
#if defined(DEM_AGING_PROCESSING_DEM_INTERNAL)
        if( (eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL) && (eventStatusBuffer[i].eventParamRef->EventClass->AgingCycleRef == operationCycleId) ) {
            eventStatusBuffer[i].passedDuringAgingCycle = FALSE;
            eventStatusBuffer[i].failedDuringAgingCycle = FALSE;
        }
#endif
    }
#if defined(DEM_USE_INDICATORS)
    indicatorOpCycleStart(operationCycleId);
#endif

#if defined(DEM_USE_IUMPR)
    resetIumprFlags(operationCycleId);

    incrementUnlockedIumprDenominators(operationCycleId);

    incrementIgnitionCycleCounter(operationCycleId);
#endif
}

#if (DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON)
/**
 * Check if an event is stored in permanent memory
 * @param eventParam
 * @return
 */
static boolean isStoredInPermanentMemory(const Dem_EventParameterType *eventParam)
{
    boolean isStored = FALSE;
    for( uint32 i = 0; (i < DEM_MAX_NUMBER_EVENT_PERM_MEM) && (FALSE == isStored); i++) {
        if( (NULL != eventParam->DTCClassRef) && (eventParam->DTCClassRef->DTCRef->OBDDTC == permMemEventBuffer[i].OBDDTC)) {
            /* DTC is stored */
            isStored = TRUE;
        }
    }
    return isStored;
}

/**
 * Transfers a number of events to permanent memory
 * @param nofEntries
 */
static boolean transferEventToPermanent(uint16 nofEntries)
{
    uint32 oldestTimestamp;
    uint16 storeIndex = 0;
    boolean candidateFound = TRUE;
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
    boolean immediateStorage = FALSE;
#endif
    /* Find the oldest */
    for( uint16 cnt = 0; (cnt < nofEntries) && (TRUE == candidateFound); cnt++ ) {
        candidateFound = FALSE;
        oldestTimestamp = 0xFFFFFFFF;
        for (uint16 i = 0; (i < DEM_MAX_NUMBER_EVENT) ; i++) {
            if( (eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL) && (TRUE == eventStatusBuffer[i].isAvailable) ) {
                if( (TRUE == permanentDTCStorageConditionFulfilled(eventStatusBuffer[i].eventParamRef, eventStatusBuffer[i].eventStatusExtended)) &&
                        (FALSE == isStoredInPermanentMemory(eventStatusBuffer[i].eventParamRef)) &&
                        (eventStatusBuffer[i].timeStamp < oldestTimestamp)) {
                    storeIndex = i;
                    oldestTimestamp = eventStatusBuffer[i].timeStamp;
                    candidateFound = TRUE;
                }
            }
        }
        if( TRUE == candidateFound ) {
            if( TRUE == handlePermanentDTCStorage(eventStatusBuffer[storeIndex].eventParamRef, eventStatusBuffer[storeIndex].eventStatusExtended) ) {
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
                if( (NULL != eventStatusBuffer[storeIndex].eventParamRef->DTCClassRef) &&
                        (TRUE == eventStatusBuffer[storeIndex].eventParamRef->DTCClassRef->DTCRef->ImmediateNvStorage) && (eventStatusBuffer[storeIndex].occurrence <= DEM_IMMEDIATE_NV_STORAGE_LIMIT)) {
                    immediateStorage = TRUE;
                }
#endif
            }
        }
    }
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
    return immediateStorage;
#else
    return FALSE;
#endif
}
#endif

/**
 * Handles ending an operation cycle
 * @param operationCycleId
 */
static void operationCycleEnd(Dem_OperationCycleIdType operationCycleId)
{
    Dem_EventStatusExtendedType oldStatus;
#if (DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON)
    boolean permanentMemoryUpdated = FALSE;
    uint16 nofErasedPermanentDTCs = 0;
#endif
#if defined(DEM_USE_INDICATORS)
    boolean indicatorsUpdated = FALSE;
#endif
    operationCycleStateList[operationCycleId] = DEM_CYCLE_STATE_END;
    // Lookup event ID
    for (uint16 i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
        boolean storeEvtMem = FALSE;

        if ((eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL) && (TRUE == eventStatusBuffer[i].isAvailable)) {
            oldStatus = eventStatusBuffer[i].eventStatusExtended;
#if defined(DEM_USE_INDICATORS)
            if(TRUE == indicatorOpCycleEnd(operationCycleId, &eventStatusBuffer[i])) {
                indicatorsUpdated = TRUE;
            }
#endif
#if (DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON)
            /* Handle the permanent DTC */
            if(TRUE == handlePermanentDTCErase(&eventStatusBuffer[i], operationCycleId) ) {
                nofErasedPermanentDTCs++;
                permanentMemoryUpdated = TRUE;
            }
#endif
            if ((0 == (eventStatusBuffer[i].eventStatusExtended & DEM_TEST_FAILED_THIS_OPERATION_CYCLE)) && (0 == (eventStatusBuffer[i].eventStatusExtended & DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE))) {
                if( eventStatusBuffer[i].eventParamRef->EventClass->OperationCycleRef == operationCycleId ) {
                    eventStatusBuffer[i].eventStatusExtended &= (Dem_EventStatusExtendedType)~DEM_PENDING_DTC;      // Clear pendingDTC bit /** @req DEM379.PendingClear
                    if( oldStatus != eventStatusBuffer[i].eventStatusExtended ) {
                        storeEvtMem = TRUE;
                    }
                }
            }
#if defined(DEM_FAILURE_PROCESSING_DEM_INTERNAL)
            if( ((Dem_OperationCycleIdType)*(eventStatusBuffer[i].eventParamRef->EventClass->FailureCycleRef) == operationCycleId) &&
                    (TRUE == eventStatusBuffer[i].passedDuringFailureCycle) && (FALSE == eventStatusBuffer[i].failedDuringFailureCycle) ) {
                /* @dev DEM: Spec. does not say when this counter should be cleared */
                if( 0 != eventStatusBuffer[i].failureCounter ) {
                    eventStatusBuffer[i].failureCounter = 0;
                    storeEvtMem = TRUE;
                }
            }
#endif
#if defined(USE_DEM_EXTENSION)
            Dem_Extension_OperationCycleEnd(operationCycleId, &eventStatusBuffer[i]);
#endif
            if( oldStatus != eventStatusBuffer[i].eventStatusExtended ) {
                /* @req DEM016 */
                notifyEventStatusChange(eventStatusBuffer[i].eventParamRef, oldStatus, eventStatusBuffer[i].eventStatusExtended);
            }
            if( TRUE == storeEvtMem ) {
                /* Transfer to event memory.  */
                if( E_OK == storeEventEvtMem(eventStatusBuffer[i].eventParamRef, &eventStatusBuffer[i], FALSE) ) {
                    notifyEventDataChanged(eventStatusBuffer[i].eventParamRef);
                }
            }
        }
    }
#if defined(DEM_USE_INDICATORS)
    if( TRUE == indicatorsUpdated ) {
#ifdef DEM_USE_MEMORY_FUNCTIONS
        /* IMPROVEMENT: Immediate storage? */
        Dem_NvM_SetIndicatorBlockChanged(FALSE);
#endif
    }
#endif
#if (DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON)
    if( TRUE == permanentMemoryUpdated ) {
        /* DTC was deleted from permanent memory. Check if we should store some other DTC */
        boolean immediateStorage = transferEventToPermanent(nofErasedPermanentDTCs);
        Dem_NvM_SetPermanentBlockChanged(immediateStorage);
    }
#endif
}
/*
 * Procedure:   setOperationCycleState
 * Description: Change the operation state of "operationCycleId" to "cycleState" and updates stored
 *              event connected to this cycle id.
 *              Returns E_OK if operation was successful else E_NOT_OK.
 */
static Std_ReturnType setOperationCycleState(Dem_OperationCycleIdType operationCycleId, Dem_OperationCycleStateType cycleState) /** @req DEM338 */
{
    Std_ReturnType returnCode = E_OK;
    /* @req DEM338 */
    if (operationCycleId < DEM_OPERATION_CYCLE_ID_ENDMARK) {
        switch (cycleState) {
        case DEM_CYCLE_STATE_START:
            /* @req DEM483 */
            operationCycleStart(operationCycleId);
            break;

        case DEM_CYCLE_STATE_END:
            if(operationCycleStateList[operationCycleId] != DEM_CYCLE_STATE_END) {
                /* @req DEM484 */
                operationCycleEnd(operationCycleId);
#if defined(DEM_AGING_PROCESSING_DEM_INTERNAL) && defined(DEM_USE_MEMORY_FUNCTIONS)
                (void)handleAging(operationCycleId);
#endif
            }
            break;
        default:
            DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_SETOPERATIONCYCLESTATE_ID, DEM_E_PARAM_DATA);
            returnCode = E_NOT_OK;
            break;
        }
    } else {
        DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_SETOPERATIONCYCLESTATE_ID, DEM_E_PARAM_DATA);
        returnCode = E_NOT_OK;
    }

    return returnCode;
}


static inline void initEventStatusBuffer(const Dem_EventParameterType *eventIdParamList)
{
    // Insert all supported events into event status buffer
    const Dem_EventParameterType *eventParam = eventIdParamList;
    EventStatusRecType *eventStatusRecPtr;
    while( FALSE == eventParam->Arc_EOL ) {
        // Find next free position in event status buffer
        lookupEventStatusRec(eventParam->EventID, &eventStatusRecPtr);
        if(NULL != eventStatusRecPtr) {
            eventStatusRecPtr->eventId = eventParam->EventID;
            eventStatusRecPtr->eventParamRef = eventParam;
            sint8 startUdsFdc = getDefaultUDSFdc(eventParam->EventID);
            eventStatusRecPtr->UDSFdc = startUdsFdc;/* @req DEM438 */
            eventStatusRecPtr->maxUDSFdc = startUdsFdc;
            eventStatusRecPtr->fdcInternal = 0;
            eventStatusRecPtr->isAvailable = *eventParam->EventClass->EventAvailableByCalibration;
            if(FALSE == eventStatusRecPtr->isAvailable) {
                eventStatusRecPtr->eventStatusExtended = 0x0u;
            }
#if (DEM_DTC_SUPPRESSION_SUPPORT == STD_ON)
            /* Check if suppression of DTC is affected */
            boolean suppressed = TRUE;
            const Dem_EventParameterType *dtcEventParam;
            if( (NULL != eventParam->DTCClassRef) && (NULL != eventParam->DTCClassRef->Events) ) {
                for( uint16 i = 0; (i < eventParam->DTCClassRef->NofEvents) && (TRUE == suppressed); i++ ) {
                    dtcEventParam = NULL;
                    lookupEventIdParameter(eventParam->DTCClassRef->Events[i], &dtcEventParam);
                    if( (NULL != dtcEventParam) && (TRUE == *dtcEventParam->EventClass->EventAvailableByCalibration) ) {
                        /* Event is available -> DTC NOT suppressed */
                        suppressed = FALSE;
                    }
                }
                if( 0 != eventParam->DTCClassRef->NofEvents ) {
                    DemDTCSuppressed[eventParam->DTCClassRef->DTCIndex].SuppressedByEvent = suppressed;
                }
            }
#endif
        }
        eventParam++;
    }
}

#if ( DEM_FF_DATA_IN_PRE_INIT )
static inline void initPreInitFreezeFrameBuffer(void)
{
    for (uint16 i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRE_INIT; i++) {
        preInitFreezeFrameBuffer[i].eventId = DEM_EVENT_ID_NULL;
        preInitFreezeFrameBuffer[i].dataSize = 0;
#if (DEM_USE_TIMESTAMPS == STD_ON)
        preInitFreezeFrameBuffer[i].timeStamp = 0;
#endif
        for (uint16 j = 0; j < DEM_MAX_SIZE_FF_DATA;j++){
            preInitFreezeFrameBuffer[i].data[j] = 0;
        }
    }
}
#endif

static inline void initPreInitExtDataBuffer(void)
{
#if ( DEM_EXT_DATA_IN_PRE_INIT )
    for (uint16 i = 0; i < DEM_MAX_NUMBER_EXT_DATA_PRE_INIT; i++) {
#if (DEM_USE_TIMESTAMPS == STD_ON)
        preInitExtDataBuffer[i].timeStamp = 0;
#endif
        preInitExtDataBuffer[i].eventId = DEM_EVENT_ID_NULL;
        for (uint16 j = 0; j < DEM_MAX_SIZE_EXT_DATA;j++){
            preInitExtDataBuffer[i].data[j] = 0;
        }
    }
#endif
}

#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
static inline void initEnableConditions(void)
{
    /* Initialize the enable conditions */
    const Dem_EnableConditionType *enableCondition = configSet->EnableCondition;
    while( enableCondition->EnableConditionID != DEM_ENABLE_CONDITION_EOL) {
        DemEnableConditions[enableCondition->EnableConditionID] = enableCondition->EnableConditionStatus;
        enableCondition++;
    }
}
#endif

#ifdef DEM_USE_MEMORY_FUNCTIONS
static boolean validateFreezeFrames(FreezeFrameRecType* freezeFrameBuffer, uint32 freezeFrameBufferSize, Dem_DTCOriginType origin)
{
    /* IMPROVEMENT: Delete OBD freeze frames if the event is not emission related */
    boolean freezeFrameBlockChanged = FALSE;
    // Validate freeze frame records stored in primary memory
    for (uint32 i = 0u; i < freezeFrameBufferSize; i++) {
        if ((freezeFrameBuffer[i].eventId == DEM_EVENT_ID_NULL) || (FALSE == checkEntryValid(freezeFrameBuffer[i].eventId, origin, TRUE))) {
            // Unlegal record, clear the record
            memset(&freezeFrameBuffer[i], 0, sizeof(FreezeFrameRecType));
            freezeFrameBlockChanged = TRUE;
        }
    }
    return freezeFrameBlockChanged;
}

static boolean validateExtendedData(ExtDataRecType* extendedDataBuffer, uint32 extendedDataBufferSize, Dem_DTCOriginType origin)
{
    boolean extendedDataBlockChanged = FALSE;
    for (uint32 i = 0uL; i < extendedDataBufferSize; i++) {
        if ((extendedDataBuffer[i].eventId == DEM_EVENT_ID_NULL) || (FALSE == checkEntryValid(extendedDataBuffer[i].eventId, origin, TRUE))) {
            // Unlegal record, clear the record
            memset(&extendedDataBuffer[i], 0, sizeof(ExtDataRecType));
            extendedDataBlockChanged = TRUE;
        }
    }
    return extendedDataBlockChanged;
}
#endif /* DEM_USE_MEMORY_FUNCTIONS */

/**
 * Looks for freeze frame data for a specific record number (a specific record or the most recent). Returns pointer to data,
 * the record number found and the type of freeze frame data (OBD or NON-OBD)
 * @param eventParam
 * @param recNum
 * @param freezeFrameData
 * @param ffRecNumFound
 * @param ffKind
 * @return TRUE: freeze frame data found, FALSE: freeze frame data not found
 */
static boolean getFFRecData(const Dem_EventParameterType *eventParam, uint8 recNum, uint8 **freezeFrameData, uint8 *ffRecNumFound, Dem_FreezeFrameKindType *ffKind)
{
    boolean isStored = FALSE;
    uint8 nofStoredRecord = 0;
    uint8 recordToFind = recNum;
    boolean failed = FALSE;
    if( (NULL != eventParam->FreezeFrameClassRefIdx) && (*(eventParam->FreezeFrameClassRefIdx) != DEM_FF_NULLREF) && (NULL != eventParam->FreezeFrameRecNumClassRef) ) {
        if( MOST_RECENT_FF_RECORD == recNum ) {
            /* Should find the most recent record */
            if(E_OK == getNofStoredNonOBDFreezeFrames(eventParam, eventParam->EventClass->EventDestination, &nofStoredRecord)){
                if( 0 == nofStoredRecord ) {
                    failed = TRUE;
                } else {
                    recordToFind = eventParam->FreezeFrameRecNumClassRef->FreezeFrameRecordNumber[nofStoredRecord - 1];
                }
            }
        }
        if( FALSE == failed ) {
            /* Have a record number to look for */
            FreezeFrameRecType *freezeFrame = NULL;
            if((TRUE == getStoredFreezeFrame(eventParam->EventID, recordToFind, eventParam->EventClass->EventDestination, &freezeFrame)) && (NULL != freezeFrame)) {
                *freezeFrameData = freezeFrame->data;
                *ffKind = DEM_FREEZE_FRAME_NON_OBD;
                isStored = TRUE;
            }
        }
    } else {
#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM)
        /* Could be OBD... */
        recordToFind = 0; /* Always 0 for OBD freeze frame */
        if( (NULL != configSet->GlobalOBDFreezeFrameClassRef) && (NULL != eventParam->DTCClassRef) && (DEM_NON_EMISSION_RELATED != (Dem_Arc_EventDTCKindType) *(eventParam->EventDTCKind))) {
            /* Event is event related */
            if( (0 == recNum) || (MOST_RECENT_FF_RECORD == recNum) ) {
                /*find the corresponding FF in FF buffer*/
                for(uint16 i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM; i++){
                    if((DEM_FREEZE_FRAME_OBD == priMemFreezeFrameBuffer[i].kind) && (priMemFreezeFrameBuffer[i].eventId == eventParam->EventID)){
                        *freezeFrameData = priMemFreezeFrameBuffer[i].data;
                        *ffKind = DEM_FREEZE_FRAME_OBD;
                        isStored = TRUE;
                        break;
                    }
                }
            }
        }
#endif
    }
    *ffRecNumFound = recordToFind;
    return isStored;
}

/**
 * Checks if an event may be cleared
 * @param eventParam
 * @return TRUE: Event may be cleared, FALSE: Event may NOT be cleared
 */
static boolean clearEventAllowed(const Dem_EventParameterType *eventParam)
{
    boolean clearAllowed = TRUE;
    /* @req DEM514 */
    if(NULL != eventParam->CallbackClearEventAllowed) {
        /* @req DEM515 */
        if( E_OK != eventParam->CallbackClearEventAllowed(&clearAllowed)) {
            /* @req DEM516 */
            clearAllowed = TRUE;
        }
    }
    return clearAllowed;
}
//==============================================================================//
//                                                                              //
//                    E X T E R N A L   F U N C T I O N S                       //
//                                                                              //
//==============================================================================//

/*********************************************
 * Interface for upper layer modules (8.3.1) *
 *********************************************/

/*
 * Procedure:   Dem_GetVersionInfo
 * Reentrant:   Yes
 */
// Defined in Dem.h


/***********************************************
 * Interface ECU State Manager <-> DEM (8.3.2) *
 ***********************************************/

#if defined(DEM_USE_INDICATORS)
static void initIndicatorStatusBuffer(const Dem_EventParameterType *eventIdParamList)
{
    uint16 indx = 0;
    while( FALSE == eventIdParamList[indx].Arc_EOL ) {
        if( NULL != eventIdParamList[indx].EventClass->IndicatorAttribute  ) {
            const Dem_IndicatorAttributeType *indAttrPtr = eventIdParamList[indx].EventClass->IndicatorAttribute;
            while( FALSE == indAttrPtr->Arc_EOL) {
                indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].EventID = eventIdParamList[indx].EventID;
                indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].InternalIndicatorId = indAttrPtr->IndicatorBufferIndex;
                indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].FailureCounter = 0;
                indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].HealingCounter = 0;
                indicatorStatusBuffer[indAttrPtr->IndicatorBufferIndex].OpCycleStatus = 0;
                indAttrPtr++;
            }
        }
        indx++;
    }
}
#endif

#if defined(USE_NVM) && (DEM_USE_NVM == STD_ON)
/**
 * Validates configured NvM block sizes
 */

/*lint --e{522} CONFIGURATION */
static void validateNvMBlockSizes(void)
{
    /* Check sizes of used NvM blocks */
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
    DEM_ASSERT( (0 == DEM_EVENT_PRIMARY_NVM_BLOCK_HANDLE) ||
                (DEM_EVENT_PRIMARY_NVM_BLOCK_SIZE == sizeof(priMemEventBuffer)));/*lint !e506 CONFIGURATION */
#if ( DEM_FF_DATA_IN_PRI_MEM )
    DEM_ASSERT( (0 == DEM_FREEZE_FRAME_PRIMARY_NVM_BLOCK_HANDLE) ||
                (DEM_FREEZE_FRAME_PRIMARY_NVM_BLOCK_SIZE == sizeof(priMemFreezeFrameBuffer)));/*lint !e506 CONFIGURATION */
#endif

#if ( DEM_EXT_DATA_IN_PRI_MEM )
    DEM_ASSERT( (0 == DEM_EXTENDED_DATA_PRIMARY_NVM_BLOCK_HANDLE) ||
                (DEM_EXTENDED_DATA_PRIMARY_NVM_BLOCK_SIZE == sizeof(priMemExtDataBuffer)));/*lint !e506 CONFIGURATION */
#endif
#endif

#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
    DEM_ASSERT( (0 == DEM_EVENT_SECONDARY_NVM_BLOCK_HANDLE) ||
                (DEM_EVENT_SECONDARY_NVM_BLOCK_SIZE == sizeof(secMemEventBuffer)));/*lint !e506 CONFIGURATION */
#if ( DEM_FF_DATA_IN_SEC_MEM )
    DEM_ASSERT( (0 == DEM_FREEZE_FRAME_SECONDARY_NVM_BLOCK_HANDLE) ||
                (DEM_FREEZE_FRAME_SECONDARY_NVM_BLOCK_SIZE == sizeof(secMemFreezeFrameBuffer)));/*lint !e506 CONFIGURATION */
#endif
#if ( DEM_EXT_DATA_IN_SEC_MEM )
    DEM_ASSERT( (0 == DEM_EXTENDED_DATA_SECONDARY_NVM_BLOCK_HANDLE ) ||
                (DEM_EXTENDED_DATA_SECONDARY_NVM_BLOCK_SIZE == sizeof(secMemExtDataBuffer)));/*lint !e506 CONFIGURATION */
#endif
#endif

#if defined(DEM_USE_INDICATORS) && defined(DEM_USE_MEMORY_FUNCTIONS)
    DEM_ASSERT( (0 == DEM_INDICATOR_NVM_BLOCK_HANDLE ) ||
                (DEM_INDICATOR_NVM_BLOCK_SIZE == sizeof(indicatorBuffer)));/*lint !e506 CONFIGURATION */
#endif

#if (DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON)
    DEM_ASSERT( (0 == DEM_PERMANENT_NVM_BLOCK_HANDLE ) ||
                (DEM_PERMANENT_NVM_BLOCK_SIZE == sizeof(permMemEventBuffer)));/*lint !e506 CONFIGURATION */
#endif

#if (DEM_PRESTORAGE_FF_DATA_IN_MEM)
    DEM_ASSERT( (0 == DEM_PRESTORE_FF_NVM_BLOCK_HANDLE ) ||
                  (DEM_PRESTORE_FF_NVM_BLOCK_SIZE == sizeof(memPreStoreFreezeFrameBuffer)));/*lint !e506 CONFIGURATION */
#endif
}
#endif

/*
 * Procedure:   Dem_PreInit
 * Reentrant:   No
 */
void Dem_PreInit(const Dem_ConfigType *ConfigPtr)
{
    /** @req DEM180 */
    uint16 i;

    VALIDATE_NO_RV(ConfigPtr != NULL, DEM_PREINIT_ID, DEM_E_CONFIG_PTR_INVALID);
    VALIDATE_NO_RV(ConfigPtr->ConfigSet != NULL, DEM_PREINIT_ID, DEM_E_CONFIG_PTR_INVALID);


#if defined(USE_NVM) && (DEM_USE_NVM == STD_ON)
    validateNvMBlockSizes();
#endif

    configSet = ConfigPtr->ConfigSet;

#if (DEM_DTC_SUPPRESSION_SUPPORT == STD_ON)
    const Dem_DTCClassType *DTCClass = configSet->DTCClass;
    while(FALSE == DTCClass->Arc_EOL) {
        DemDTCSuppressed[DTCClass->DTCIndex].SuppressedByDTC = FALSE;
        DemDTCSuppressed[DTCClass->DTCIndex].SuppressedByEvent = FALSE;
        DTCClass++;
    }
#endif

    // Initializion of operation cycle states.
    for (i = 0; i < DEM_OPERATION_CYCLE_ID_ENDMARK; i++) {
        operationCycleStateList[i] = DEM_CYCLE_STATE_END;
    }

    // Initialize the event status buffer
    for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
        setDefaultEventStatus(&eventStatusBuffer[i]);
    }

#if defined(DEM_USE_TIME_BASE_PREDEBOUNCE)
    InitTimeBasedDebounce();
#endif

#if (DEM_STORE_UDS_STATUS_BIT_SUBSET_FOR_ALL_EVENTS == STD_ON) && defined(DEM_USE_MEMORY_FUNCTIONS)
    SetDefaultUDSStatusBitSubset();
#endif

    // Initialize the eventstatus buffer (Insert all supported events into event status buffer)
    initEventStatusBuffer(configSet->EventParameter);

    /* Initialize the preInit freeze frame buffer */
#if( DEM_FF_DATA_IN_PRE_INIT )
    initPreInitFreezeFrameBuffer();
#endif

    /* Initialize the preInit extended data buffer */
    initPreInitExtDataBuffer();

#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
    /* Initialize the enable conditions */
    initEnableConditions();
#endif

#if defined(USE_DEM_EXTENSION)
    Dem_Extension_PreInit(ConfigPtr);
#endif

#if (DEM_USE_TIMESTAMPS == STD_ON)
    /* Reset freze frame time stamp */
    FF_TimeStamp = 0;

    /* Reset event time stamp */
    Event_TimeStamp = 0;

    /* Reset extended data timestamp */
    ExtData_TimeStamp = 0;
#endif

#if defined(DEM_USE_INDICATORS)
    initIndicatorStatusBuffer(configSet->EventParameter);
#endif

    disableDtcSetting.settingDisabled = FALSE;

    (void)setOperationCycleState(DEM_ACTIVE, DEM_CYCLE_STATE_START);

    /* Init the DTC record update disable */
    DTCRecordDisabled.DTC = NO_DTC_DISABLED;

#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
    priMemOverflow = FALSE;
#endif
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
    secMemOverflow = FALSE;
#endif

    Dem_NvM_Init();
#if defined(USE_FIM)
    DemFiMInit = FALSE;
#endif
    demState = DEM_PREINITIALIZED;
}

#ifdef DEM_USE_MEMORY_FUNCTIONS
static boolean ValidateAndMergeEventRecords(EventRecType* eventBuffer, uint32 eventBufferSize, boolean* eventEntryChanged,
                                    uint32* Evt_TimeStamp, Dem_DTCOriginType origin ) {


    boolean eventBlockChanged = FALSE;
    uint32 i;

    // Validate event records stored in memory
    for (i = 0; i < eventBufferSize; i++) {
        eventEntryChanged[i] = FALSE;

        if ((eventBuffer[i].EventData.eventId == DEM_EVENT_ID_NULL) || (checkEntryValid(eventBuffer[i].EventData.eventId, origin, FALSE) == FALSE )) {
            // Unlegal record, clear the record
            memset(&eventBuffer[i], 0, sizeof(EventRecType));
            eventBlockChanged = TRUE;
        }
    }

#if (DEM_USE_TIMESTAMPS == STD_ON)
    /* initialize the current timestamp and update the timestamp in pre init */
    initCurrentEventTimeStamp(Evt_TimeStamp);
#else
    (void)Evt_TimeStamp;/*lint !e920 *//* Avoid compiler warning */
#endif
    /* Merge events read from NvRam */
    for (i = 0; i < eventBufferSize; i++) {
        eventEntryChanged[i] = FALSE;
        if( DEM_EVENT_ID_NULL != eventBuffer[i].EventData.eventId ) {
            eventEntryChanged[i] = mergeEventStatusRec(&eventBuffer[i]);
        }
    }

    return eventBlockChanged;

}

static void MergeBuffer(Dem_DTCOriginType origin) {

    uint32 i;
    boolean eventBlockChanged;
    boolean extendedDataBlockChanged;
    boolean freezeFrameBlockChanged;
    boolean eventEntryChanged[DEM_MAX_NUMBER_EVENT_ENTRY] = {FALSE};/*lint !e506 */
    const Dem_EventParameterType *eventParam;
    EventRecType* eventBuffer = NULL;
    uint32 eventBufferSize = 0;
    FreezeFrameRecType* freezeFrameBuffer = NULL;
    uint32 freezeFrameBufferSize = 0;
    ExtDataRecType* extendedDataBuffer = NULL;
    uint32 extendedDataBufferSize = 0;
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
    boolean immediateEventStorage = FALSE;
    boolean immediateFFStorage = FALSE;
    boolean immediateExtDataStorage = FALSE;
#endif

    /* Setup variables for merging */
    switch (origin) {
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
            eventBuffer = priMemEventBuffer;
            eventBufferSize = DEM_MAX_NUMBER_EVENT_PRI_MEM;
#if ( DEM_FF_DATA_IN_PRI_MEM )
            freezeFrameBuffer = priMemFreezeFrameBuffer;
            freezeFrameBufferSize = DEM_MAX_NUMBER_FF_DATA_PRI_MEM;
#endif
#if ( DEM_EXT_DATA_IN_PRI_MEM )
            extendedDataBuffer = priMemExtDataBuffer;
            extendedDataBufferSize = DEM_MAX_NUMBER_EXT_DATA_PRI_MEM;
#endif
#endif
            break;

        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
            eventBuffer = secMemEventBuffer;
            eventBufferSize = DEM_MAX_NUMBER_EVENT_SEC_MEM;
#if ( DEM_FF_DATA_IN_SEC_MEM )
            freezeFrameBuffer = secMemFreezeFrameBuffer;
            freezeFrameBufferSize = DEM_MAX_NUMBER_FF_DATA_SEC_MEM;
#endif
#if ( DEM_EXT_DATA_IN_SEC_MEM )
            extendedDataBuffer = secMemExtDataBuffer;
            extendedDataBufferSize = DEM_MAX_NUMBER_EXT_DATA_SEC_MEM;
#endif
#endif
            break;
        default:
            DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GLOBAL_ID, DEM_E_NOT_IMPLEMENTED_YET);
            break;

    }

#if (DEM_USE_TIMESTAMPS != STD_ON)
        /* The timestamp isn't actually used. This just to make it compile.. */
    uint32 Event_TimeStamp = 0;
#endif
    eventBlockChanged = ValidateAndMergeEventRecords(eventBuffer, eventBufferSize, eventEntryChanged, &Event_TimeStamp, origin);

#if defined(USE_DEM_EXTENSION)
    Dem_Extension_Init_PostEventMerge(origin);
#endif

#if (DEM_USE_TIMESTAMPS == STD_ON)
    //initialize the current timestamp and update the timestamp in pre init
    initCurrentFreezeFrameTimeStamp(&FF_TimeStamp);
#endif

    /* Validate freeze frames stored in memory */
    freezeFrameBlockChanged = validateFreezeFrames(freezeFrameBuffer, freezeFrameBufferSize, origin);

    /* Transfer updated event data to event memory */
    for (i = 0u; (i < eventBufferSize) && (NULL != eventBuffer); i++) {
        if ( (eventBuffer[i].EventData.eventId != DEM_EVENT_ID_NULL) && (TRUE == eventEntryChanged[i]) ) {
            EventStatusRecType *eventStatusRecPtr = NULL;
            eventParam = NULL;
            lookupEventIdParameter(eventBuffer[i].EventData.eventId, &eventParam);
             /* Transfer to event memory. */
            lookupEventStatusRec(eventBuffer[i].EventData.eventId, &eventStatusRecPtr);
            if( (NULL != eventStatusRecPtr) && (NULL != eventParam) ) {
                if( E_OK == storeEventEvtMem(eventParam, eventStatusRecPtr, FALSE) ) {
                    /* Use errorStatusChanged in eventsStatusBuffer to signal that the event data was updated */
                    eventStatusRecPtr->errorStatusChanged = TRUE;
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
                    if( (NULL != eventParam->DTCClassRef) && (TRUE == eventParam->DTCClassRef->DTCRef->ImmediateNvStorage) && (eventStatusRecPtr->occurrence <= DEM_IMMEDIATE_NV_STORAGE_LIMIT)) {
                        immediateEventStorage = TRUE;
                    }
#endif
                }
            }
        }
    }

    /* Now we need to store events that was reported during preInit.
     * That is, events not already stored in eventBuffer.  */
    for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
        if( (DEM_EVENT_ID_NULL != eventStatusBuffer[i].eventId) &&
             (0 == (eventStatusBuffer[i].eventStatusExtended & DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE)) &&
             (FALSE == eventIsStoredInMem(eventStatusBuffer[i].eventId, eventBuffer, eventBufferSize)) ) {

            lookupEventIdParameter(eventStatusBuffer[i].eventId, &eventParam);
            if( (NULL != eventParam) && (eventParam->EventClass->EventDestination == origin)) {
                /* Destination check is needed two avoid notifying status change twice */
                notifyEventStatusChange(eventParam, DEM_DEFAULT_EVENT_STATUS, eventStatusBuffer[i].eventStatusExtended);
            }
            if( 0 != (eventStatusBuffer[i].eventStatusExtended & DEM_TEST_FAILED_THIS_OPERATION_CYCLE) ) {
                if( E_OK == storeEventEvtMem(eventParam, &eventStatusBuffer[i], FALSE) ) {
                    /* Use errorStatusChanged in eventsStatusBuffer to signal that the event data was updated */
                    eventStatusBuffer[i].errorStatusChanged = TRUE;
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
                    if( (NULL != eventParam) && (NULL != eventParam->DTCClassRef) &&
                            (TRUE == eventParam->DTCClassRef->DTCRef->ImmediateNvStorage) && (eventStatusBuffer[i].occurrence <= DEM_IMMEDIATE_NV_STORAGE_LIMIT)) {
                        immediateEventStorage = TRUE;
                    }
#endif
                }
            }
        }
    }

    // Validate extended data records stored in primary memory
    extendedDataBlockChanged = validateExtendedData(extendedDataBuffer, extendedDataBufferSize, origin);

#if (DEM_USE_TIMESTAMPS == STD_ON)
    //initialize the current timestamp and update the timestamp in pre init
    initCurrentExtDataTimeStamp(&ExtData_TimeStamp);
#endif

#if ( DEM_EXT_DATA_IN_PRE_INIT )
    /* Transfer extended data to event memory if necessary */
    for (i = 0; i < DEM_MAX_NUMBER_EXT_DATA_PRE_INIT; i++) {
        if ( preInitExtDataBuffer[i].eventId != DEM_EVENT_ID_NULL ) {
            boolean updateAllExtData = FALSE;
#if defined(USE_DEM_EXTENSION)
            Dem_Extension_PreMergeExtendedData(preInitExtDataBuffer[i].eventId, &updateAllExtData);
#endif
            if( TRUE == mergeExtendedDataEvtMem(&preInitExtDataBuffer[i], extendedDataBuffer, extendedDataBufferSize, origin, updateAllExtData) ) {
                /* Use errorStatusChanged in eventsStatusBuffer to signal that the event data was updated */
                EventStatusRecType *eventStatusRecPtr = NULL_PTR;
                eventParam = NULL_PTR;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
                /* Event id may be a combined id. */
                if( IS_COMBINED_EVENT_ID(preInitExtDataBuffer[i].eventId) ) {
                    const Dem_CombinedDTCCfgType *CombDTCCfg = &configSet->CombinedDTCConfig[TO_COMBINED_EVENT_CFG_IDX(preInitExtDataBuffer[i].eventId)];
                    CombDTCCfg = &configSet->CombinedDTCConfig[TO_COMBINED_EVENT_CFG_IDX(preInitExtDataBuffer[i].eventId)];
                    /* Just grab the first event for this DTC. */
                    lookupEventIdParameter(CombDTCCfg->DTCClassRef->Events[0u], &eventParam);
                }
                else {
                    lookupEventIdParameter(preInitExtDataBuffer[i].eventId, &eventParam);
                }
#else
                lookupEventIdParameter(preInitExtDataBuffer[i].eventId, &eventParam);
#endif
                if( NULL_PTR != eventParam ) {
                    lookupEventStatusRec(eventParam->EventID, &eventStatusRecPtr);
                }
                if( NULL_PTR != eventStatusRecPtr ) {
                    eventStatusRecPtr->errorStatusChanged = TRUE;
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
                    if( (NULL_PTR != eventParam) && (NULL != eventParam->DTCClassRef) &&
                            (TRUE == eventParam->DTCClassRef->DTCRef->ImmediateNvStorage) && (eventStatusRecPtr->occurrence <= DEM_IMMEDIATE_NV_STORAGE_LIMIT)) {
                        immediateExtDataStorage = TRUE;
                    }
#endif
                }
            }
        }
    }
#endif

    /* Transfer freeze frames stored during preInit to event memory */
#if ( DEM_FF_DATA_IN_PRE_INIT )
    if( TRUE == transferPreInitFreezeFramesEvtMem(freezeFrameBuffer, freezeFrameBufferSize, eventBuffer, eventBufferSize, origin) ){
        freezeFrameBlockChanged = TRUE;
    }
#endif
    if( TRUE == eventBlockChanged ) {
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
        Dem_NvM_SetEventBlockChanged(origin, immediateEventStorage);
#else
        Dem_NvM_SetEventBlockChanged(origin, FALSE);
#endif
    }
    if( TRUE == extendedDataBlockChanged ) {
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
        Dem_NvM_SetExtendedDataBlockChanged(origin, immediateExtDataStorage);
#else
        Dem_NvM_SetExtendedDataBlockChanged(origin, FALSE);
#endif
    }
    if( TRUE == freezeFrameBlockChanged ) {
#if defined(DEM_USE_IMMEDIATE_NV_STORAGE)
        /* IMPROVEMENT: Immediate storage */
        Dem_NvM_SetFreezeFrameBlockChanged(origin, immediateFFStorage);
#else
        Dem_NvM_SetFreezeFrameBlockChanged(origin, FALSE);
#endif
    }
}

#if (DEM_STORE_UDS_STATUS_BIT_SUBSET_FOR_ALL_EVENTS == STD_ON)
/**
 * Set the default value for UDS status bit subset if buffer is considered
 * invalid.
 */
static void SetDefaultUDSStatusBitSubset(void)
{
    if( UDS_STATUS_BIT_MAGIC != statusBitSubsetBuffer[UDS_STATUS_BIT_MAGIC_INDEX] ) {
        memset(statusBitSubsetBuffer, 0u, sizeof(statusBitSubsetBuffer));
        for(Dem_EventIdType i = (Dem_EventIdType)0u; i < DEM_MAX_NUMBER_EVENT; i++) {
            statusBitSubsetBuffer[GET_UDSBIT_BYTE_INDEX(i+1u)] |= 1u<<(GET_UDS_STARTBIT(i+1u) + UDS_TNCSLC_BIT);
        }
    }

}

/**
 * Merges UDS status bit subset to event buffer
 */
static void MergeUDSStatusBitSubset(void)
{
    EventStatusRecType *eventStatusRec;
    if( UDS_STATUS_BIT_MAGIC == statusBitSubsetBuffer[UDS_STATUS_BIT_MAGIC_INDEX] ) {
        for(Dem_EventIdType i = (Dem_EventIdType)0; i < DEM_MAX_NUMBER_EVENT; i++) {
            eventStatusRec = NULL;
            lookupEventStatusRec(i + 1u, &eventStatusRec);
            if( (NULL != eventStatusRec) && (TRUE == eventStatusRec->isAvailable) ) {
                if( 0 == (statusBitSubsetBuffer[GET_UDSBIT_BYTE_INDEX(i+1u)] & (1u<<(GET_UDS_STARTBIT(i+1u) + UDS_TNCSLC_BIT))) ) {
                    eventStatusRec->eventStatusExtended &= ~(DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR);
                }
                if( 0 != (statusBitSubsetBuffer[GET_UDSBIT_BYTE_INDEX(i+1u)] & (1u<<(GET_UDS_STARTBIT(i+1u) + UDS_TFSLC_BIT))) ) {
                    eventStatusRec->eventStatusExtended |= DEM_TEST_FAILED_SINCE_LAST_CLEAR;
                }
            }
        }
    }
}

/**
 * Transfers subset of UDS status bits from event buffer to buffer for NvM storage
 */
static void StoreUDSStatusBitSubset(void)
{
    Dem_EventStatusExtendedType eventStatus;
    const Dem_EventParameterType *eventParam;

    memset(statusBitSubsetBuffer, 0u, sizeof(statusBitSubsetBuffer));
    for(Dem_EventIdType i = (Dem_EventIdType)0; i < DEM_MAX_NUMBER_EVENT; i++) {
        if(E_OK == getEventStatus(i + 1u, &eventStatus)) {
            eventParam = NULL;
            lookupEventIdParameter(i + 1u, &eventParam);
            if( (NULL != eventParam) && (DEM_DTC_ORIGIN_NOT_USED != eventParam->EventClass->EventDestination)) {
                if( 0 != (eventStatus & DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR) ) {
                    statusBitSubsetBuffer[GET_UDSBIT_BYTE_INDEX(i+1u)] |= 1u<<(GET_UDS_STARTBIT(i+1u) + UDS_TNCSLC_BIT);
                }
                if( 0 != (eventStatus & DEM_TEST_FAILED_SINCE_LAST_CLEAR) ) {
                    statusBitSubsetBuffer[GET_UDSBIT_BYTE_INDEX(i+1u)] |= 1u<<(GET_UDS_STARTBIT(i+1u) + UDS_TFSLC_BIT);
                }
            }
        }
    }
    statusBitSubsetBuffer[UDS_STATUS_BIT_MAGIC_INDEX] = UDS_STATUS_BIT_MAGIC;
    /* IPROVEMENT: Immediate storage? */
    Dem_NvM_SetStatusBitSubsetBlockChanged(FALSE);
}
#endif
#endif /* DEM_USE_MEMORY_FUNCTIONS */

#if defined(DEM_USE_IUMPR)
static void mergeIumprBuffer(void) {
    for (Dem_RatioIdType i = 0; i < DEM_IUMPR_REGISTERED_COUNT; i++) {
		iumprBufferLocal[i].denominator.value = iumprBuffer.ratios[i].denominator;
		iumprBufferLocal[i].numerator.value = iumprBuffer.ratios[i].numerator;
    }

    generalDenominatorBuffer.value = iumprBuffer.generalDenominatorCount;

	ignitionCycleCountBuffer = iumprBuffer.ignitionCycleCount;
}

static void storeIumprBuffer(void) {
    for (Dem_RatioIdType i = 0; i < DEM_IUMPR_REGISTERED_COUNT; i++) {
    	iumprBuffer.ratios[i].denominator = iumprBufferLocal[i].denominator.value;
    	iumprBuffer.ratios[i].numerator = iumprBufferLocal[i].numerator.value;
    }

    iumprBuffer.generalDenominatorCount = generalDenominatorBuffer.value;

    iumprBuffer.ignitionCycleCount = ignitionCycleCountBuffer;

    Dem_Nvm_SetIumprBlockChanged(TRUE);
}
#endif

/*
 * Procedure:   Dem_Init
 * Reentrant:   No
 */
void Dem_Init(void)
{
    /* @req DEM340 */
   //// SchM_Enter_Dem_EA_0();
    for(uint16 i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
        eventStatusBuffer[i].errorStatusChanged = FALSE;
    }
    if(DEM_PREINITIALIZED != demState) {
        /*
         * Dem_PreInit was has not been called since last time Dem_Shutdown was called.
         * This suggests that we are resuming from sleep. According to section 5.7 in
         * EcuM specification, RAM content is assumed to be still valid from the previous cycle.
         * Do not read from saved error log since buffers already contains this data.
         */
        (void)setOperationCycleState(DEM_ACTIVE, DEM_CYCLE_STATE_START);

    } else {
#if defined(DEM_USE_MEMORY_FUNCTIONS) && (DEM_STORE_UDS_STATUS_BIT_SUBSET_FOR_ALL_EVENTS == STD_ON)
        MergeUDSStatusBitSubset();
#endif
#if defined(DEM_USE_INDICATORS) && defined(DEM_USE_MEMORY_FUNCTIONS)
        mergeIndicatorBuffers();
#endif
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
        MergeBuffer(DEM_DTC_ORIGIN_PRIMARY_MEMORY);
#endif
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
        MergeBuffer(DEM_DTC_ORIGIN_SECONDARY_MEMORY);
#endif
#if (DEM_USE_PERMANENT_MEMORY_SUPPORT == STD_ON)
        ValidateAndUpdatePermanentBuffer();
#endif
#if (DEM_PRESTORAGE_FF_DATA_IN_MEM)
        ValidateAndUpdatePreStoredFreezeFramesBuffer();
#endif

    }
#if defined(USE_FIM)
    /* @req 4.3.0/SWS_Dem_01189 */
    if( FALSE == DemFiMInit ) {
        FiM_DemInit();
        DemFiMInit = TRUE;
    }
#endif

    /* Notify application if event data was updated */
    for(uint16 i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
        /* @req DEM475 */
        if( 0 != eventStatusBuffer[i].errorStatusChanged ) {
            notifyEventDataChanged(eventStatusBuffer[i].eventParamRef);
            eventStatusBuffer[i].errorStatusChanged = FALSE;
        }
    }
#if defined(USE_DEM_EXTENSION)
    Dem_Extension_Init_Complete();
#endif

#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
    if(ADMIN_MAGIC == priMemEventBuffer[PRI_MEM_EVENT_BUFFER_ADMIN_INDEX].AdminData.magic) {
        priMemOverflow = priMemEventBuffer[PRI_MEM_EVENT_BUFFER_ADMIN_INDEX].AdminData.overflow;
    }
#endif
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
    if(ADMIN_MAGIC == secMemEventBuffer[SEC_MEM_EVENT_BUFFER_ADMIN_INDEX].AdminData.magic) {
        secMemOverflow = secMemEventBuffer[SEC_MEM_EVENT_BUFFER_ADMIN_INDEX].AdminData.overflow;
    }
#endif
    // Init the dtc filter
    dtcFilter.dtcStatusMask = DEM_DTC_STATUS_MASK_ALL;                  // All allowed
    dtcFilter.dtcKind = DEM_DTC_KIND_ALL_DTCS;                          // All kinds of DTCs
    dtcFilter.dtcOrigin = DEM_DTC_ORIGIN_PRIMARY_MEMORY;                // Primary memory
    dtcFilter.filterWithSeverity = DEM_FILTER_WITH_SEVERITY_NO;         // No Severity filtering
    dtcFilter.dtcSeverityMask = DEM_SEVERITY_NO_SEVERITY;               // Not used when filterWithSeverity is FALSE
    dtcFilter.filterForFaultDetectionCounter = DEM_FILTER_FOR_FDC_NO;   // No fault detection counter filtering
    dtcFilter.DTCIndex = 0u;

    disableDtcSetting.settingDisabled = FALSE;

    ffRecordFilter.ffIndex = DEM_MAX_NUMBER_FF_DATA_PRI_MEM;
    ffRecordFilter.dtcFormat = 0xff;

#if defined(DEM_USE_IUMPR)
    mergeIumprBuffer();

    initIumprAddiDenomCondBuffer();
#endif

    demState = DEM_INITIALIZED;

  ////  SchM_Exit_Dem_EA_0();
}


/*
 * Procedure:   Dem_shutdown
 * Reentrant:   No
 */
void Dem_Shutdown(void)
{
    VALIDATE_NO_RV(DEM_INITIALIZED == demState, DEM_SHUTDOWN_ID, DEM_E_UNINIT);
    /* @req DEM102 */
    SchM_Enter_Dem_EA_0();

    (void)setOperationCycleState(DEM_ACTIVE, DEM_CYCLE_STATE_END);
#if defined(DEM_USE_MEMORY_FUNCTIONS) && (DEM_STORE_UDS_STATUS_BIT_SUBSET_FOR_ALL_EVENTS == STD_ON)
    StoreUDSStatusBitSubset();
#endif
#if defined(DEM_USE_IUMPR)
    storeIumprBuffer();
#endif
#if defined(USE_DEM_EXTENSION)
    Dem_Extension_Shutdown();
#endif
    demState = DEM_SHUTDOWN; /** @req DEM368 */

    SchM_Exit_Dem_EA_0();
}

/*
 * Interface for basic software scheduler
 */

void Dem_MainFunction(void)/** @req DEM125 */
{
    VALIDATE_NO_RV(DEM_UNINITIALIZED != demState, DEM_MAINFUNCTION_ID, DEM_E_UNINIT);

#ifdef DEM_USE_MEMORY_FUNCTIONS
    Dem_NvM_MainFunction();
#endif /* DEM_USE_MEMORY_FUNCTIONS */
#if defined(USE_DEM_EXTENSION)
    Dem_Extension_MainFunction();
#endif

#if defined(DEM_USE_TIME_BASE_PREDEBOUNCE)
    /* Handle time based predebounce */
    TimeBasedDebounceMainFunction();
#endif
}

/**
 * Gets the indicator status derived from the event status
 * @param IndicatorId
 * @param IndicatorStatus
 * @return E_OK: Operation was successful, E_NOT_OK: Operation failed or is not supported
 */
/*lint -efunc(818,Dem_GetIndicatorStatus) Dem_IndicatorStatusType cannot be declared as pointing to const as API defined by AUTOSAR  */
Std_ReturnType Dem_GetIndicatorStatus( uint8 IndicatorId, Dem_IndicatorStatusType* IndicatorStatus )
{
    /* @req DEM046 */
    /* @req DEM508 */
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_GETINDICATORSTATUS_ID, DEM_E_UNINIT, E_NOT_OK);
    VALIDATE_RV(NULL != IndicatorStatus, DEM_GETINDICATORSTATUS_ID, DEM_E_PARAM_POINTER, E_NOT_OK);
#if defined(DEM_USE_INDICATORS)
    VALIDATE_RV(DEM_NOF_INDICATORS > IndicatorId, DEM_GETINDICATORSTATUS_ID, DEM_E_PARAM_CONFIG, E_NOT_OK);
    Std_ReturnType ret = E_NOT_OK;
    const Dem_IndicatorType *indConfig;
    const Dem_EventParameterType *eventParam;
    uint8 currPrio = 0xff;
    if( IndicatorId < DEM_NOF_INDICATORS  ) {
        indConfig = &configSet->Indicators[IndicatorId];
        *IndicatorStatus = DEM_INDICATOR_OFF;
        for( uint8 indx = 0; indx < indConfig->EventListSize; indx++ ) {
            eventParam = NULL;
            lookupEventIdParameter(indConfig->EventList[indx], &eventParam);
            if( (NULL != eventParam) && (NULL != eventParam->EventClass->IndicatorAttribute) && (TRUE == (boolean)(*eventParam->EventClass->IndicatorAttribute->IndicatorValid)) ) {
                const Dem_IndicatorAttributeType *indAttrPtr = eventParam->EventClass->IndicatorAttribute;
                while( FALSE == indAttrPtr->Arc_EOL ) {
                    if( indAttrPtr->IndicatorId == IndicatorId ) {
                        /* Found a match */
                        ret = E_OK;
                        if( TRUE == indicatorFailFulfilled(eventParam, indAttrPtr) ) {
                            if( eventParam->EventClass->EventPriority < currPrio ) {
                                *IndicatorStatus = indAttrPtr->IndicatorBehaviour;
                                currPrio = eventParam->EventClass->EventPriority;
                            }
                        }
                    }
                    indAttrPtr++;
                }
            }
        }
    }
    return ret;
#else
    (void)IndicatorId;
    DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GETINDICATORSTATUS_ID, DEM_E_PARAM_CONFIG);
    return E_NOT_OK;
#endif
}

/***************************************************
 * Interface SW-Components via RTE <-> DEM (8.3.3) *
 ***************************************************/

/*
 * Procedure:   Dem_SetEventStatus
 * Reentrant:   Yes
 */
/* @req DEM183 */
Std_ReturnType Dem_SetEventStatus(Dem_EventIdType eventId, Dem_EventStatusType eventStatus) /** @req DEM330 */
{
    /* @req DEM330 */
	Std_ReturnType returnCode = E_NOT_OK;
	VALIDATE_RV(((DEM_INITIALIZED == demState) || (DEM_SHUTDOWN == demState)), DEM_SETEVENTSTATUS_ID, DEM_E_UNINIT, E_NOT_OK);
	VALIDATE_RV(IS_VALID_EVENT_STATUS(eventStatus), DEM_SETEVENTSTATUS_ID, DEM_E_PARAM_DATA, E_NOT_OK);
	// Ignore this API call after Dem_Shutdown()
	if (DEM_SHUTDOWN != demState) {
		SchM_Enter_Dem_EA_0();

		returnCode = handleEvent(eventId, eventStatus);

		SchM_Exit_Dem_EA_0();
	}
    return returnCode;
}


/*
 * Procedure:   Dem_ResetEventStatus
 * Reentrant:   Yes
 */
/* @req DEM185 */
Std_ReturnType Dem_ResetEventStatus(Dem_EventIdType eventId) /** @req DEM331 */
{
    /* @req DEM331 */
    Std_ReturnType returnCode;
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_RESETEVENTSTATUS_ID, DEM_E_UNINIT, E_NOT_OK);

    SchM_Enter_Dem_EA_0();

    /* Function resetEventStatus will notify application if there is a change in the status bits */
    returnCode = resetEventStatus(eventId);


    SchM_Exit_Dem_EA_0();
    return returnCode;
}


/*
 * Procedure:   Dem_GetEventStatus
 * Reentrant:   Yes
 */
Std_ReturnType Dem_GetEventStatus(Dem_EventIdType eventId, Dem_EventStatusExtendedType *eventStatusExtended)
{
    Std_ReturnType returnCode;
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_GETEVENTSTATUS_ID, DEM_E_UNINIT, E_NOT_OK);
    VALIDATE_RV(NULL != eventStatusExtended, DEM_GETEVENTSTATUS_ID, DEM_E_PARAM_POINTER, E_NOT_OK);
    SchM_Enter_Dem_EA_0();

    returnCode = getEventStatus(eventId, eventStatusExtended);

    SchM_Exit_Dem_EA_0();

    return returnCode;
}


/*
 * Procedure:   Dem_GetEventFailed
 * Reentrant:   Yes
 */
Std_ReturnType Dem_GetEventFailed(Dem_EventIdType eventId, boolean *eventFailed) /** @req DEM333 */
{
    /* @req DEM333 */
    Std_ReturnType returnCode;
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_GETEVENTFAILED_ID, DEM_E_UNINIT, E_NOT_OK);

    SchM_Enter_Dem_EA_0();

    returnCode = getEventFailed(eventId, eventFailed);

    SchM_Exit_Dem_EA_0();

    return returnCode;
}


/*
 * Procedure:   Dem_GetEventTested
 * Reentrant:   Yes
 */
Std_ReturnType Dem_GetEventTested(Dem_EventIdType eventId, boolean *eventTested)
{
    /* @req DEM333 */
    Std_ReturnType returnCode;
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_GETEVENTTESTED_ID, DEM_E_UNINIT, E_NOT_OK);

    SchM_Enter_Dem_EA_0();

    returnCode = getEventTested(eventId, eventTested);

    SchM_Exit_Dem_EA_0();

    return returnCode;
}


/*
 * Procedure:   Dem_GetFaultDetectionCounter
 * Reentrant:   No
 */
Std_ReturnType Dem_GetFaultDetectionCounter(Dem_EventIdType eventId, sint8 *counter)
{
    /* @req DEM204 */
    Std_ReturnType returnCode;
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_GETFAULTDETECTIONCOUNTER_ID, DEM_E_UNINIT, E_NOT_OK);
    SchM_Enter_Dem_EA_0();

    returnCode = getFaultDetectionCounter(eventId, counter);

    SchM_Exit_Dem_EA_0();
    return returnCode;
}


/*
 * Procedure:   Dem_SetOperationCycleState
 * Reentrant:   No
 */
Std_ReturnType Dem_SetOperationCycleState(Dem_OperationCycleIdType operationCycleId, Dem_OperationCycleStateType cycleState)
{
    Std_ReturnType returnCode = E_OK;
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_SETOPERATIONCYCLESTATE_ID, DEM_E_UNINIT, E_NOT_OK);
    SchM_Enter_Dem_EA_0();

    if( DEM_ACTIVE == operationCycleId ) {
        /* Handled internally */
        DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_SETOPERATIONCYCLESTATE_ID, DEM_E_UNEXPECTED_EXECUTION);
        returnCode = E_NOT_OK;
    } else {
        returnCode = setOperationCycleState(operationCycleId, cycleState);
    }

    SchM_Exit_Dem_EA_0();
    return returnCode;
}


/*
 * Procedure:   Dem_GetDTCOfEvent
 * Reentrant:   Yes
 */
Std_ReturnType Dem_GetDTCOfEvent(Dem_EventIdType eventId, Dem_DTCFormatType dtcFormat, uint32* dtcOfEvent)
{
    Std_ReturnType returnCode = E_NO_DTC_AVAILABLE;
    const Dem_EventParameterType *eventParam;
    EventStatusRecType * eventStatusRec;

    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_GETDTCOFEVENT_ID, DEM_E_UNINIT, E_NOT_OK);
    VALIDATE_RV(IS_VALID_DTC_FORMAT(dtcFormat), DEM_GETDTCOFEVENT_ID, DEM_E_PARAM_DATA, E_NOT_OK);

    SchM_Enter_Dem_EA_0();

    lookupEventIdParameter(eventId, &eventParam);
    lookupEventStatusRec(eventId, &eventStatusRec);
    if ( (eventParam != NULL) && (NULL != eventStatusRec) && (TRUE == eventStatusRec->isAvailable)) {
        if ((eventParam->DTCClassRef != NULL) && (TRUE == eventParam->DTCClassRef->DTCRef->DTCUsed)) {
            if( TRUE == eventHasDTCOnFormat(eventParam, dtcFormat) ) {
                *dtcOfEvent = (DEM_DTC_FORMAT_UDS == dtcFormat) ? eventParam->DTCClassRef->DTCRef->UDSDTC : TO_OBD_FORMAT(eventParam->DTCClassRef->DTCRef->OBDDTC);/** @req DEM269 */
                returnCode = E_OK;
            }
        }
    } else {
        // Event Id not found
        returnCode = E_NOT_OK;
    }

    SchM_Exit_Dem_EA_0();
    return returnCode;
}


/********************************************
 * Interface BSW-Components <-> DEM (8.3.4) *
 ********************************************/

/*
 * Procedure:   Dem_ReportErrorStatus
 * Reentrant:   Yes
 */
void Dem_ReportErrorStatus( Dem_EventIdType eventId, Dem_EventStatusType eventStatus ) /** @req DEM206 */
{
    /* @req DEM330 */
    /* @req DEM107 */
    VALIDATE_NO_RV((DEM_UNINITIALIZED != demState), DEM_REPORTERRORSTATUS_ID, DEM_E_UNINIT);
    VALIDATE_NO_RV(IS_VALID_EVENT_STATUS(eventStatus), DEM_REPORTERRORSTATUS_ID, DEM_E_PARAM_DATA);

    SchM_Enter_Dem_EA_0();

    switch (demState) {
        case DEM_PREINITIALIZED:
            // Update status and check if is to be stored
            if ((eventStatus == DEM_EVENT_STATUS_PASSED) || (eventStatus == DEM_EVENT_STATUS_FAILED)) {
                handlePreInitEvent(eventId, eventStatus); /** @req DEM167 */
            }
            break;

        case DEM_INITIALIZED:
            (void)handleEvent(eventId, eventStatus);
            break;

        case DEM_SHUTDOWN:
        default:
            // Ignore api call
            break;

    } // switch (demState)

    SchM_Exit_Dem_EA_0();
}

/*********************************
 * Interface DCM <-> DEM (8.3.5) *
 *********************************/
/*
 * Procedure:   Dem_GetDTCStatusAvailabilityMask
 * Reentrant:   No
 */
/*lint -esym(793, Dem_GetDTCStatusAvailabilityMask) Function name defined by AUTOSAR. */
Std_ReturnType Dem_GetDTCStatusAvailabilityMask(uint8 *dtcStatusMask) /** @req DEM014 */
{
    /** @req DEM060 */
    *dtcStatusMask =    DEM_DTC_STATUS_AVAILABILITY_MASK;       // User configuration mask
    return E_OK;
}


/*
 * Procedure:   Dem_SetDTCFilter
 * Reentrant:   No
 */
Dem_ReturnSetFilterType Dem_SetDTCFilter(uint8 dtcStatusMask,
        Dem_DTCKindType dtcKind,
        Dem_DTCFormatType dtcFormat,
        Dem_DTCOriginType dtcOrigin,
        Dem_FilterWithSeverityType filterWithSeverity,
        Dem_DTCSeverityType dtcSeverityMask,
        Dem_FilterForFDCType filterForFaultDetectionCounter)
{
    Dem_ReturnSetFilterType returnCode = DEM_FILTER_ACCEPTED;
    uint8 dtcStatusAvailabilityMask;
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_SETDTCFILTER_ID, DEM_E_UNINIT, E_NOT_OK);

    // Check dtcKind parameter
    VALIDATE_RV((dtcKind == DEM_DTC_KIND_ALL_DTCS) || (dtcKind == DEM_DTC_KIND_EMISSION_REL_DTCS), DEM_SETDTCFILTER_ID, DEM_E_PARAM_DATA, DEM_WRONG_FILTER);

    // Check dtcOrigin parameter
    VALIDATE_RV((dtcOrigin == DEM_DTC_ORIGIN_SECONDARY_MEMORY) || (dtcOrigin == DEM_DTC_ORIGIN_PRIMARY_MEMORY)|| (dtcOrigin == DEM_DTC_ORIGIN_PERMANENT_MEMORY), DEM_SETDTCFILTER_ID, DEM_E_PARAM_DATA, DEM_WRONG_FILTER);

    // Check filterWithSeverity and dtcSeverityMask parameter
    VALIDATE_RV(((filterWithSeverity == DEM_FILTER_WITH_SEVERITY_NO)
                || ((filterWithSeverity == DEM_FILTER_WITH_SEVERITY_YES)
                    && (0 == (dtcSeverityMask & (Dem_DTCSeverityType)~(DEM_SEVERITY_MAINTENANCE_ONLY | DEM_SEVERITY_CHECK_AT_NEXT_HALT | DEM_SEVERITY_CHECK_IMMEDIATELY))))), DEM_SETDTCFILTER_ID, DEM_E_PARAM_DATA, DEM_WRONG_FILTER);

    // Check filterForFaultDetectionCounter parameter
    VALIDATE_RV((filterForFaultDetectionCounter == DEM_FILTER_FOR_FDC_YES) || (filterForFaultDetectionCounter ==  DEM_FILTER_FOR_FDC_NO), DEM_SETDTCFILTER_ID, DEM_E_PARAM_DATA, DEM_WRONG_FILTER);

    VALIDATE_RV( IS_VALID_DTC_FORMAT(dtcFormat), DEM_SETDTCFILTER_ID, DEM_E_PARAM_DATA, DEM_WRONG_FILTER);

    (void)Dem_GetDTCStatusAvailabilityMask(&dtcStatusAvailabilityMask);

    if( (0u == (dtcStatusMask & dtcStatusAvailabilityMask)) && (DEM_DTC_STATUS_MASK_ALL != dtcStatusMask) ) {
        /* No bit in the filter mask supported. */
        returnCode = DEM_WRONG_FILTER;
    } else {
        // Yes all parameters correct, set the new filters.  /** @req DEM057 */
        dtcFilter.dtcStatusMask = dtcStatusMask & dtcStatusAvailabilityMask;
        dtcFilter.dtcKind = dtcKind;
        dtcFilter.dtcOrigin = dtcOrigin;
        dtcFilter.filterWithSeverity = filterWithSeverity;
        dtcFilter.dtcSeverityMask = dtcSeverityMask;
        dtcFilter.filterForFaultDetectionCounter = filterForFaultDetectionCounter;
        dtcFilter.DTCIndex = 0u;
        dtcFilter.dtcFormat = dtcFormat;
    }

    return returnCode;
}


/*
 * Procedure:   Dem_GetStatusOfDTC
 * Reentrant:   No
 */
Dem_ReturnGetStatusOfDTCType Dem_GetStatusOfDTC(uint32 dtc, Dem_DTCOriginType dtcOrigin, Dem_EventStatusExtendedType* status) {
    /* NOTE: dtc is in UDS format according to DEM212 */
    Dem_ReturnGetStatusOfDTCType returnCode = DEM_STATUS_FAILED;
    EventStatusRecType *eventRec;
    const Dem_DTCClassType *DTCClass;

    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_GETSTATUSOFDTC_ID, DEM_E_UNINIT, DEM_STATUS_FAILED);
    VALIDATE_RV(IS_SUPPORTED_ORIGIN(dtcOrigin), DEM_GETSTATUSOFDTC_ID, DEM_E_PARAM_DATA, DEM_STATUS_WRONG_DTCORIGIN);/** @req DEM171 */

    SchM_Enter_Dem_EA_0();

    Dem_EventStatusExtendedType temp = 0u;
    if ( TRUE == LookupUdsDTC(dtc, &DTCClass)) {
        returnCode = DEM_STATUS_OK;
        for(uint16 i = 0; (i < DTCClass->NofEvents) && (DEM_STATUS_OK == returnCode); i++) {
            returnCode = DEM_STATUS_OK;
            eventRec = NULL_PTR;
            lookupEventStatusRec(DTCClass->Events[i], &eventRec);
            if( NULL_PTR != eventRec ) {
                /* Event found for this DTC */
                if (checkDtcOrigin(dtcOrigin,eventRec->eventParamRef, FALSE) == TRUE) {
                    /* NOTE: Should the availability mask be used here? */
                    /* @req DEM059 */
                    /* @req DEM441 */
                    if( TRUE == eventRec->isAvailable ) {
                        temp |= eventRec->eventStatusExtended;
                    }

                } else {
                    /* Here we know that dtcOrigin is a supported one */
                    returnCode = DEM_STATUS_WRONG_DTC; /** @req DEM172 */
                }
            }
            else {
                returnCode = DEM_STATUS_FAILED;
            }
        }
        uint8 mask = 0xFFU;
        if( (DEM_STATUS_OK == returnCode) && (DTCClass->NofEvents > 1u) ) {
            /* This is a combined DTC. Bits have already been OR-ed above. Now we should and bits. */
            /* @req DEM441 */
            mask = ((temp & (1u << 5u)) >> 1u) | ((temp & (1u << 1u)) << 5u);
            mask = (uint8)((~mask) & 0xFFu);
        }
        *status = (temp & mask);
    } else {
        /* Event has no DTC or DTC is suppressed */
        /* @req 4.2.2/SWS_Dem_01100 *//* @req DEM587 */
        returnCode = DEM_STATUS_WRONG_DTC;
    }

    SchM_Exit_Dem_EA_0();

    return returnCode;
}


/*
 * Procedure:   Dem_GetNumberOfFilteredDtc
 * Reentrant:   No
 */
Dem_ReturnGetNumberOfFilteredDTCType Dem_GetNumberOfFilteredDtc(uint16 *numberOfFilteredDTC) {

    uint16 numberOfFaults = 0;
    Dem_ReturnGetNumberOfFilteredDTCType returnCode = DEM_NUMBER_OK;

    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_GETNUMBEROFFILTEREDDTC_ID, DEM_E_UNINIT, DEM_NUMBER_FAILED);
    VALIDATE_RV(NULL != numberOfFilteredDTC, DEM_GETNUMBEROFFILTEREDDTC_ID, DEM_E_PARAM_POINTER, DEM_NUMBER_FAILED);
    SchM_Enter_Dem_EA_0();
    const Dem_DTCClassType *DTCClass = configSet->DTCClass;
    Dem_EventStatusExtendedType DTCStatus;
    /* Find all DTCs matching filter. Ignore suppressed DTCs *//* @req DEM587 *//* @req 4.2.2/SWS_Dem_01101 */
    while( FALSE == DTCClass->Arc_EOL ) {
        if( TRUE == matchDTCWithDtcFilter(DTCClass, &DTCStatus) ) {
            numberOfFaults++;
        }
        DTCClass++;
    }

    *numberOfFilteredDTC = numberOfFaults; /** @req DEM061 */

    SchM_Exit_Dem_EA_0();
    return returnCode;
}


/*
 * Procedure:   Dem_GetNextFilteredDTC
 * Reentrant:   No
 */
Dem_ReturnGetNextFilteredDTCType Dem_GetNextFilteredDTC(uint32 *dtc, Dem_EventStatusExtendedType *dtcStatus)
{
    Dem_ReturnGetNextFilteredDTCType returnCode = DEM_FILTERED_OK;
    boolean dtcFound = FALSE;
    Dem_EventStatusExtendedType DTCStatus;

    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_GETNEXTFILTEREDDTC_ID, DEM_E_UNINIT, DEM_FILTERED_NO_MATCHING_DTC);
    VALIDATE_RV(NULL != dtc, DEM_GETNEXTFILTEREDDTC_ID, DEM_E_PARAM_POINTER, DEM_FILTERED_NO_MATCHING_DTC);
    VALIDATE_RV(NULL != dtcStatus, DEM_GETNEXTFILTEREDDTC_ID, DEM_E_PARAM_POINTER, DEM_FILTERED_NO_MATCHING_DTC);

    SchM_Enter_Dem_EA_0();

    /* Find the next DTC matching filter. Ignore suppressed DTCs *//* @req DEM587 *//* @req 4.2.2/SWS_Dem_01101 */
    /* @req DEM217 */
    const Dem_DTCClassType *DTCClass = &configSet->DTCClass[dtcFilter.DTCIndex];
    while( (dtcFound == FALSE) && (FALSE == DTCClass->Arc_EOL) ) {
        if( TRUE == matchDTCWithDtcFilter(DTCClass, &DTCStatus) ) {
            if( DEM_DTC_FORMAT_UDS == dtcFilter.dtcFormat ) {
                *dtc = DTCClass->DTCRef->UDSDTC; /** @req DEM216 */
            }
            else {
                *dtc = TO_OBD_FORMAT(DTCClass->DTCRef->OBDDTC);
            }
            *dtcStatus = DTCStatus;
            dtcFound = TRUE;
        }
        dtcFilter.DTCIndex++;
        DTCClass++;
    }

    if( FALSE == dtcFound ) {
        dtcFilter.DTCIndex = 0u;
        returnCode = DEM_FILTERED_NO_MATCHING_DTC;
    }

    SchM_Exit_Dem_EA_0();
    return returnCode;
}


/*
 * Procedure:   Dem_GetTranslationType
 * Reentrant:   No
 */
Dem_DTCTranslationFormatType Dem_GetTranslationType(void)
{
    return DEM_TYPE_OF_DTC_SUPPORTED; /** @req DEM231 */
}

/*
 * Procedure:   Dem_ClearDTC
 * Comment:     Stating the dtcOrigin makes no since when reading the reqiurements in the specification.
 * Reentrant:   No
 */
Dem_ReturnClearDTCType Dem_ClearDTC(uint32 dtc, Dem_DTCFormatType dtcFormat, Dem_DTCOriginType dtcOrigin) /** @req DEM009 *//** @req DEM241 */
{
    Dem_ReturnClearDTCType returnCode = DEM_CLEAR_WRONG_DTCORIGIN;
    const Dem_EventParameterType *eventParam;
    Dem_EventStatusExtendedType oldStatus;
    boolean dataDeleted;
#ifdef DEM_USE_MEMORY_FUNCTIONS
    boolean allClearOK = TRUE;
#endif
    (void)dtcOrigin;
#if defined(DEM_USE_INDICATORS) && defined(DEM_USE_MEMORY_FUNCTIONS)
    boolean indicatorsChanged = FALSE;
#endif
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_CLEARDTC_ID, DEM_E_UNINIT, DEM_CLEAR_FAILED);
    VALIDATE_RV(IS_VALID_DTC_FORMAT(dtcFormat), DEM_CLEARDTC_ID, DEM_E_PARAM_DATA, DEM_CLEAR_FAILED);


    for (uint16 i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
       SchM_Enter_Dem_EA_0();
        dataDeleted = FALSE;
        if ((DEM_EVENT_ID_NULL != eventStatusBuffer[i].eventId) && (NULL != eventStatusBuffer[i].eventParamRef)) {
            eventParam = eventStatusBuffer[i].eventParamRef;
            if ((DEM_CLEAR_ALL_EVENTS == STD_ON) || (eventParam->DTCClassRef != NULL)) {/*lint !e506 !e774*/
                if (checkDtcGroup(dtc, eventParam, dtcFormat) == TRUE) {
                    if( eventParam->EventClass->EventDestination == dtcOrigin ) {
                        if(FALSE == eventDTCRecordDataUpdateDisabled(eventParam)) {
                            if( clearEventAllowed(eventParam) == TRUE) {
                                boolean dtcOriginFound = FALSE;
                                oldStatus = eventStatusBuffer[i].eventStatusExtended;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
                                dataDeleted = DeleteDTCData(eventParam, TRUE, &dtcOriginFound, (DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID));/* @req DEM343 */
#else
                                dataDeleted = DeleteDTCData(eventParam, TRUE, &dtcOriginFound, FALSE);/* @req DEM343 */
#endif
                                if (dtcOriginFound == FALSE) {
                                    returnCode = DEM_CLEAR_WRONG_DTCORIGIN;
                                    DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_CLEARDTC_ID, DEM_E_NOT_IMPLEMENTED_YET);
                                } else {
#if defined(DEM_USE_INDICATORS)
                                    if( TRUE == resetIndicatorCounters(eventParam) ) {
#ifdef DEM_USE_MEMORY_FUNCTIONS
                                        indicatorsChanged = TRUE;
#endif
                                    }
#endif
#if defined(USE_DEM_EXTENSION)
                                    Dem_Extension_ClearEvent(eventParam);
#endif

                                    if( dataDeleted == TRUE) {
                                        /* @req DEM475 */
                                        notifyEventDataChanged(eventParam);
                                    }
                                    if( oldStatus != eventStatusBuffer[i].eventStatusExtended ) {
                                        /* @req DEM016 */
                                        notifyEventStatusChange(eventParam, oldStatus, eventStatusBuffer[i].eventStatusExtended);
                                    }
                                    if( NULL != eventParam->CallbackInitMforE ) {
                                        /* @req DEM376 */
                                        (void)eventParam->CallbackInitMforE(DEM_INIT_MONITOR_CLEAR);
                                    }
                                    /* Have cleared at least one, OK */
                                    returnCode = DEM_CLEAR_OK;
                                }
                            } else {
                                returnCode = DEM_CLEAR_FAILED; /* CallbackClearEventAllowed returned not allowed to clear */
#ifdef DEM_USE_MEMORY_FUNCTIONS
                                /* Clear was not allowed */
                                allClearOK = FALSE;
#endif
                            }
                        }
                    }
                } else {
                    if( (((DEM_DTC_FORMAT_UDS == dtcFormat) && (dtc == eventParam->DTCClassRef->DTCRef->UDSDTC)) ||
                            ((DEM_DTC_FORMAT_OBD == dtcFormat) && (dtc == TO_OBD_FORMAT(eventParam->DTCClassRef->DTCRef->OBDDTC))))
                            && (DTCIsAvailable(eventParam->DTCClassRef) == FALSE) ) {
                        /* This DTC is suppressed *//* @req 4.2.2/SWS_Dem_01101 */
                        returnCode = DEM_CLEAR_WRONG_DTC;
                    }
                }
            }
        } else {
            // Fatal error, no event parameters found for the event!
            DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_CLEARDTC_ID, DEM_E_UNEXPECTED_EXECUTION);
        }
    SchM_Exit_Dem_EA_0();
    }


  SchM_Enter_Dem_EA_0();
#ifdef DEM_USE_MEMORY_FUNCTIONS
#if defined(DEM_USE_INDICATORS)
    if( indicatorsChanged == TRUE ) {
        /* IMPROVEMENT: Immediate storage when deleting? */
        Dem_NvM_SetIndicatorBlockChanged(FALSE);
    }
#endif
    if( (DEM_DTC_GROUP_ALL_DTCS == dtc) && (allClearOK == TRUE)) {
        /* @req DEM399 */
        setOverflowIndication(dtcOrigin, FALSE);
    }
#endif

    SchM_Exit_Dem_EA_0();

    return returnCode;
}


/*
 * Procedure:   Dem_DisableDTCStorage
 * Reentrant:   No
 */
Dem_ReturnControlDTCStorageType Dem_DisableDTCSetting(Dem_DTCGroupType dtcGroup, Dem_DTCKindType dtcKind) /** @req DEM035 */
{
    Dem_ReturnControlDTCStorageType returnCode = DEM_CONTROL_DTC_STORAGE_OK;
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_DISABLEDTCSETTING_ID, DEM_E_UNINIT, DEM_CONTROL_DTC_STORAGE_N_OK);
    // Check dtcGroup parameter
    uint32 DTCGroupLower;
    uint32 DTCGroupUpper;
    if ( (dtcGroup == DEM_DTC_GROUP_ALL_DTCS) || (DEM_DTC_GROUP_EMISSION_REL_DTCS == dtcGroup) || (TRUE == dtcIsGroup(dtcGroup, DEM_DTC_FORMAT_UDS, &DTCGroupLower, &DTCGroupUpper))) {
        // Check dtcKind parameter
        if ((dtcKind == DEM_DTC_KIND_ALL_DTCS) || (dtcKind ==  DEM_DTC_KIND_EMISSION_REL_DTCS)) {
            /** @req DEM079 */
            disableDtcSetting.dtcGroup = dtcGroup;
            disableDtcSetting.dtcKind = dtcKind;
            disableDtcSetting.settingDisabled = TRUE;
        } else {
            returnCode = DEM_CONTROL_DTC_STORAGE_N_OK;
        }
    } else {
        returnCode = DEM_CONTROL_DTC_WRONG_DTCGROUP;
    }

    return returnCode;
}


/*
 * Procedure:   Dem_EnableDTCStorage
 * Reentrant:   No
 */
Dem_ReturnControlDTCStorageType Dem_EnableDTCSetting(Dem_DTCGroupType dtcGroup, Dem_DTCKindType dtcKind)
{
    Dem_ReturnControlDTCStorageType returnCode = DEM_CONTROL_DTC_STORAGE_OK;
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_ENABLEDTCSETTING_ID, DEM_E_UNINIT, DEM_CONTROL_DTC_STORAGE_N_OK);

    // NOTE: Behavior is not defined if group or kind do not match active settings, therefore the filter is just switched off.
    (void)dtcGroup; (void)dtcKind;  // Just to make get rid of PC-Lint warnings
    disableDtcSetting.settingDisabled = FALSE; /** @req DEM080 */

    return returnCode;
}



/*
 * Procedure:   Dem_GetExtendedDataRecordByDTC
 * Reentrant:   No
 */
Dem_ReturnGetExtendedDataRecordByDTCType Dem_GetExtendedDataRecordByDTC(uint32 dtc, Dem_DTCOriginType dtcOrigin, uint8 extendedDataNumber, uint8 *destBuffer, uint16 *bufSize)
{
    /* IMPROVEMENT: Handle record numbers 0xFE and 0xFF */
    /* NOTE: dtc is in UDS format according to DEM239 */
    /* @req DEM540 */
    Dem_ReturnGetExtendedDataRecordByDTCType returnCode = DEM_RECORD_WRONG_DTC;
    const Dem_EventParameterType *eventParam;
    Dem_ExtendedDataRecordClassType const *extendedDataRecordClass = NULL;
    ExtDataRecType *extData;
    uint16 posInExtData = 0;
    uint16 nofBytesCopied = 0;
    uint16 bufSizeLeft ;
    boolean oneRecordOK = FALSE;
#if (DEM_EVENT_COMB_TYPE2_REPORT_OLDEST_DATA == STD_ON)
    uint32 timestamp = 0;
    boolean dataCopied = FALSE;
#endif
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_GETEXTENDEDDATARECORDBYDTC_ID, DEM_E_UNINIT, DEM_RECORD_WRONG_DTC);
    VALIDATE_RV((NULL != destBuffer), DEM_GETEXTENDEDDATARECORDBYDTC_ID, DEM_E_PARAM_POINTER, DEM_RECORD_WRONG_DTC);
    VALIDATE_RV((NULL != bufSize), DEM_GETEXTENDEDDATARECORDBYDTC_ID, DEM_E_PARAM_POINTER, DEM_RECORD_WRONG_DTC);
    SchM_Enter_Dem_EA_0();
    bufSizeLeft = *bufSize;
    if( extendedDataNumber <= DEM_HIGHEST_EXT_DATA_REC_NUM ) {
        /* Get the DTC config */
        const Dem_DTCClassType *DTCClass;
        if ( TRUE == LookupUdsDTC(dtc, &DTCClass) ) {
            for(uint16 i = 0; i < DTCClass->NofEvents; i++) {
                eventParam = NULL_PTR;
                lookupEventIdParameter(DTCClass->Events[i], &eventParam);
                if( NULL_PTR != eventParam ) {
                    if (checkDtcOrigin(dtcOrigin, eventParam, FALSE)==TRUE) {
                        if (lookupExtendedDataRecNumParam(extendedDataNumber, eventParam, &extendedDataRecordClass, &posInExtData)==TRUE) {
                            if (bufSizeLeft >= extendedDataRecordClass->DataSize) {
                                oneRecordOK = TRUE;
#if (DEM_EVENT_COMB_TYPE2_REPORT_OLDEST_DATA == STD_OFF)
                                Dem_EventIdType idToFind;
#endif
                                if( extendedDataRecordClass->UpdateRule != DEM_UPDATE_RECORD_VOLATILE ) {
                                    switch (dtcOrigin) {
                                        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
                                        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
#if (DEM_EVENT_COMB_TYPE2_REPORT_OLDEST_DATA == STD_ON)
                                            if (lookupOlderExtendedDataMem(eventParam->EventID, &extData, dtcOrigin, &timestamp, dataCopied) == TRUE) {
                                                // Yes all conditions met, copy the extended data record to destination buffer.
                                                memcpy(destBuffer, &extData->data[posInExtData], extendedDataRecordClass->DataSize); /** @req DEM075 */
                                                nofBytesCopied = extendedDataRecordClass->DataSize;/* @req DEM076 */
                                                dataCopied = TRUE;
                                                returnCode = DEM_RECORD_OK;
                                            }

#else

#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
                                            if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID ) {
                                                /* @req DEM537 */
                                                idToFind = TO_COMBINED_EVENT_ID(eventParam->CombinedDTCCID);
                                            }
                                            else {
                                                idToFind = eventParam->EventID;
                                            }
#else
                                            idToFind = eventParam->EventID;
#endif
                                            if (lookupExtendedDataMem(idToFind, &extData, dtcOrigin) == TRUE) {
                                                // Yes all conditions met, copy the extended data record to destination buffer.
                                                memcpy(&destBuffer[nofBytesCopied], &extData->data[posInExtData], extendedDataRecordClass->DataSize); /** @req DEM075 */
#if !defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
                                                bufSizeLeft -= extendedDataRecordClass->DataSize;/* @req DEM076 */
#endif
                                                nofBytesCopied += extendedDataRecordClass->DataSize;
                                                returnCode = DEM_RECORD_OK;
                                            } else {
                                                /* The record number is legal but no record was found for the DTC *//* @req DEM631 */
                                                returnCode = DEM_RECORD_OK;
                                            }
#endif
                                            break;
                                        case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
                                        case DEM_DTC_ORIGIN_MIRROR_MEMORY:
                                            // Not yet supported
                                            returnCode = DEM_RECORD_WRONG_DTCORIGIN;
                                            DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GETEXTENDEDDATARECORDBYDTC_ID, DEM_E_NOT_IMPLEMENTED_YET);
                                            break;
                                        default:
                                            returnCode = DEM_RECORD_WRONG_DTCORIGIN;
                                            break;
                                    }
                                } else {
#if (DEM_EVENT_COMB_TYPE2_REPORT_OLDEST_DATA == STD_ON)
                                    if( FALSE == dataCopied ) {
                                        /* No data copied*/
                                        if( NULL != extendedDataRecordClass->CallbackGetExtDataRecord ) {
                                            /* IMPROVEMENT: Handle return value? */
                                            (void)extendedDataRecordClass->CallbackGetExtDataRecord(&destBuffer[nofBytesCopied]);
                                            bufSizeLeft -= extendedDataRecordClass->DataSize;/* @req DEM076 */
                                            nofBytesCopied += extendedDataRecordClass->DataSize;
                                            returnCode = DEM_RECORD_OK;
                                        } else if (DEM_NO_ELEMENT != extendedDataRecordClass->InternalDataElement ) {
                                            getInternalElement( eventParam, extendedDataRecordClass->InternalDataElement, &destBuffer[nofBytesCopied], extendedDataRecordClass->DataSize );
                                            bufSizeLeft -= extendedDataRecordClass->DataSize;/* @req DEM076 */
                                            nofBytesCopied += extendedDataRecordClass->DataSize;
                                            returnCode = DEM_RECORD_OK;
                                        } else {
                                            returnCode = DEM_RECORD_WRONG_DTC;
                                        }
                                    }
                                    else {
                                        /* Data has already been copied. This means that a stored record was found. Assume this to be older than "live" data. */
                                    }
#else
                                    if( NULL != extendedDataRecordClass->CallbackGetExtDataRecord ) {
                                        /* IMPROVEMENT: Handle return value? */
                                        (void)extendedDataRecordClass->CallbackGetExtDataRecord(destBuffer);
#if !defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
                                        bufSizeLeft -= extendedDataRecordClass->DataSize;/* @req DEM076 */
#endif
                                        nofBytesCopied += extendedDataRecordClass->DataSize;
                                        returnCode = DEM_RECORD_OK;
                                    } else if (DEM_NO_ELEMENT != extendedDataRecordClass->InternalDataElement ) {
                                        getInternalElement( eventParam, extendedDataRecordClass->InternalDataElement, destBuffer, extendedDataRecordClass->DataSize );
#if !defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
                                        bufSizeLeft -= extendedDataRecordClass->DataSize;/* @req DEM076 */
#endif
                                        nofBytesCopied += extendedDataRecordClass->DataSize;
                                        returnCode = DEM_RECORD_OK;
                                    } else {
                                        returnCode = DEM_RECORD_WRONG_DTC;
                                    }
#endif
                                }
                            } else {
                                DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GETEXTENDEDDATARECORDBYDTC_ID, DEM_E_PARAM_LENGTH);
                                returnCode = DEM_RECORD_BUFFERSIZE;
                            }
                        } else {
                            returnCode = DEM_RECORD_NUMBER;
                        }
                    } else {
                        returnCode = DEM_RECORD_WRONG_DTCORIGIN;
                    }
                }
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
                break;
#endif
            }
            if( TRUE == oneRecordOK ) {
                returnCode = DEM_RECORD_OK;
                *bufSize = nofBytesCopied;
            }
        } else {
            /* Event has no DTC or DTC is suppressed */
            /* @req 4.2.2/SWS_Dem_01100 */
            /* @req 4.2.2/SWS_Dem_01101 */
            /* @req DEM587 */
            returnCode = DEM_RECORD_WRONG_DTC;
        }
    } else {
        returnCode = DEM_RECORD_NUMBER;
    }

    SchM_Exit_Dem_EA_0();
    return returnCode;
}


/*
 * Procedure:   Dem_GetSizeOfExtendedDataRecordByDTC
 * Reentrant:   No
 */
/*lint -esym(793, Dem_GetSizeOfExtendedDataRecordByDTC) Function name defined by AUTOSAR. */
Dem_ReturnGetSizeOfExtendedDataRecordByDTCType Dem_GetSizeOfExtendedDataRecordByDTC(uint32 dtc, Dem_DTCOriginType dtcOrigin, uint8 extendedDataNumber, uint16 *sizeOfExtendedDataRecord)
{
    /* NOTE: dtc is in UDS format according to DEM240 */
    Dem_ReturnGetExtendedDataRecordByDTCType returnCode = DEM_GET_SIZEOFEDRBYDTC_W_DTC;
    Dem_ExtendedDataRecordClassType const *extendedDataRecordClass = NULL_PTR;
    const Dem_EventParameterType *eventParam;
    uint16 posInExtData;
    boolean oneRecordOK = FALSE;

    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_GETSIZEOFEXTENDEDDATARECORDBYDTC_ID, DEM_E_UNINIT, DEM_GET_SIZEOFEDRBYDTC_W_DTC);
    VALIDATE_RV(NULL != sizeOfExtendedDataRecord, DEM_GETSIZEOFEXTENDEDDATARECORDBYDTC_ID, DEM_E_PARAM_POINTER, DEM_GET_SIZEOFEDRBYDTC_W_DTC);
    SchM_Enter_Dem_EA_0();

    /* Check if event has DTC and that the DTC is not suppressed *//* @req DEM587 */
    /* @req 4.2.2/SWS_Dem_01100 */
    /* @req 4.2.2/SWS_Dem_01101 */
    /* Get the DTC config */
    const Dem_DTCClassType *DTCClass;
    if ( TRUE == LookupUdsDTC(dtc, &DTCClass) ) {
        *sizeOfExtendedDataRecord = 0u;
        for(uint16 i = 0; i < DTCClass->NofEvents; i++) {
            eventParam = NULL_PTR;
            lookupEventIdParameter(DTCClass->Events[i], &eventParam);
            if( NULL_PTR != eventParam ) {
                if (checkDtcOrigin(dtcOrigin, eventParam, FALSE) == TRUE) {
                    if (lookupExtendedDataRecNumParam(extendedDataNumber, eventParam, &extendedDataRecordClass, &posInExtData) == TRUE) {
#if (DEM_EVENT_COMB_TYPE2_REPORT_OLDEST_DATA == STD_ON)
                        if( extendedDataRecordClass->DataSize > *sizeOfExtendedDataRecord) {
                            *sizeOfExtendedDataRecord = extendedDataRecordClass->DataSize; /** @req DEM076 */
                        }
#else
                        *sizeOfExtendedDataRecord += extendedDataRecordClass->DataSize;
#endif
                        oneRecordOK = TRUE;
                    }
                    else {
                        returnCode = DEM_GET_SIZEOFEDRBYDTC_W_RNUM;
                    }
                }
                else {
                    returnCode = DEM_GET_SIZEOFEDRBYDTC_W_DTCOR;
                }
            }
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
            break;
#endif
        }
        if( TRUE == oneRecordOK ) {
            returnCode = DEM_GET_SIZEOFEDRBYDTC_OK;
        }
    }

    SchM_Exit_Dem_EA_0();
    return returnCode;
}

/*
 * Procedure:   Dem_GetFreezeFrameDataByDTC
 * Reentrant:   No
 */
/** @req DEM236 */
Dem_ReturnGetFreezeFrameDataByDTCType Dem_GetFreezeFrameDataByDTC(uint32 dtc, Dem_DTCOriginType dtcOrigin, uint8 recordNumber, uint8* destBuffer, uint16*  bufSize)
{
    /* !req DEM576 */
    /* @req DEM540 */
    /* NOTE: dtc is in UDS format according to DEM236 */
    Dem_ReturnGetFreezeFrameDataByDTCType returnCode = DEM_GET_FFDATABYDTC_WRONG_DTC;
    const Dem_EventParameterType *eventParam;
    Dem_FreezeFrameClassType const *FFDataRecordClass = NULL;
    uint16 FFDataSize = 0;
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_GETFREEZEFRAMEDATABYDTC_ID, DEM_E_UNINIT, DEM_GET_ID_PENDING);
    VALIDATE_RV((NULL != destBuffer), DEM_GETFREEZEFRAMEDATABYDTC_ID, DEM_E_PARAM_POINTER, DEM_GET_FFDATABYDTC_WRONG_DTC);
    VALIDATE_RV((NULL != bufSize), DEM_GETFREEZEFRAMEDATABYDTC_ID, DEM_E_PARAM_POINTER, DEM_GET_FFDATABYDTC_WRONG_DTC);
    SchM_Enter_Dem_EA_0();

    uint16 bufSizeLeft = *bufSize;
    uint16 bufferSize;
    uint16 nofBytesCopied = 0u;
    boolean oneRecordOK = FALSE;
#if (DEM_EVENT_COMB_TYPE2_REPORT_OLDEST_DATA == STD_ON)
    uint32 timestamp = 0;
    boolean useTimestamp = FALSE;
#endif
    if( *bufSize >= DEM_REC_NUM_AND_NUM_DIDS_SIZE ) {
        bufSizeLeft -= DEM_REC_NUM_AND_NUM_DIDS_SIZE;
        if( recordNumber <= DEM_HIGHEST_FF_REC_NUM ) {
            /* Get the DTC config */
            const Dem_DTCClassType *DTCClass;
            if ( TRUE == LookupUdsDTC(dtc, &DTCClass) ) {
                destBuffer[0] = recordNumber;
                destBuffer[1] = 0u;
                for(uint16 i = 0; i < DTCClass->NofEvents; i++) {
                    eventParam = NULL_PTR;
                    lookupEventIdParameter(DTCClass->Events[i], &eventParam);
                    if( NULL_PTR != eventParam ) {
                        if (checkDtcOrigin(dtcOrigin, eventParam, FALSE) == TRUE) {
                            if (lookupFreezeFrameDataRecNumParam(recordNumber, eventParam, &FFDataRecordClass) == TRUE) {
                                /* NOTE: Handle return value? */
                                (void)lookupFreezeFrameDataSize(recordNumber, &FFDataRecordClass, &FFDataSize);
                                if (bufSizeLeft >= FFDataSize) {
                                    oneRecordOK = TRUE;
                                    switch (dtcOrigin) {
                                        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
                                        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
                                            returnCode = DEM_GET_FFDATABYDTC_OK;
                                            bufferSize = bufSizeLeft;
#if (DEM_EVENT_COMB_TYPE2_REPORT_OLDEST_DATA == STD_ON)
                                            if( TRUE == getOlderFreezeFrameRecord(eventParam->EventID, recordNumber, dtcOrigin, &destBuffer[2u], &bufferSize, FFDataSize, &timestamp,  useTimestamp)) {
                                                destBuffer[1] = FFDataRecordClass->NofXids;
                                                useTimestamp = TRUE;
                                                nofBytesCopied = bufferSize;
                                            }
#else
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
                                            Dem_EventIdType eventId;
                                            if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID ) {
                                                /* @req DEM537 */
                                                eventId = TO_COMBINED_EVENT_ID(eventParam->CombinedDTCCID);
                                            }
                                            else {
                                                eventId = eventParam->EventID;
                                            }
                                            if (getFreezeFrameRecord(eventId, recordNumber, dtcOrigin, &destBuffer[nofBytesCopied + 2u], &bufferSize, FFDataSize) == TRUE) {
#else
                                            if (getFreezeFrameRecord(eventParam->EventID, recordNumber, dtcOrigin, &destBuffer[nofBytesCopied + 2u], &bufferSize, FFDataSize) == TRUE) {
#endif
                                                destBuffer[1] += FFDataRecordClass->NofXids;
#if !defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
                                                bufSizeLeft -= bufferSize;
#endif
                                                nofBytesCopied += bufferSize;
                                            } else {
                                                /* @req DEM630 */
                                            }
#endif
                                            break;
                                        case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
                                        case DEM_DTC_ORIGIN_MIRROR_MEMORY:
                                            // Not yet supported
                                            returnCode = DEM_GET_FFDATABYDTC_WRONG_DTCORIGIN;
                                            DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GETFREEZEFRAMEDATABYDTC_ID, DEM_E_NOT_IMPLEMENTED_YET);
                                            break;
                                        default:
                                            returnCode = DEM_GET_FFDATABYDTC_WRONG_DTCORIGIN;
                                            break;
                                    }
                                } else {
                                    DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GETFREEZEFRAMEDATABYDTC_ID, DEM_E_PARAM_LENGTH);
                                    returnCode = DEM_GET_FFDATABYDTC_BUFFERSIZE;
                                }
                            } else {
                                returnCode = DEM_GET_FFDATABYDTC_RECORDNUMBER;
                            }
                        } else {
                            returnCode = DEM_GET_FFDATABYDTC_WRONG_DTCORIGIN;
                        }
                    }
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
                    break;
#endif
                }
                if( (DEM_GET_FFDATABYDTC_OK == returnCode) || (TRUE == oneRecordOK) ) {
                    *bufSize = nofBytesCopied;
                    if( 0u != nofBytesCopied ) {
                        /* Data was found. Add size of RecordNumber and NumOfDIDs */
                        *bufSize += DEM_REC_NUM_AND_NUM_DIDS_SIZE;
                    }
                    returnCode = DEM_GET_FFDATABYDTC_OK;
                }
            } else {
                /* Event has no DTC or DTC is suppressed */
                /* @req 4.2.2/SWS_Dem_01100 */
                /* @req 4.2.2/SWS_Dem_01101 */
                /* @req DEM587 */
                returnCode = DEM_GET_FFDATABYDTC_WRONG_DTC;

            }
        } else {
            returnCode = DEM_GET_FFDATABYDTC_RECORDNUMBER;
        }
    }
    else {
        DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GETFREEZEFRAMEDATABYDTC_ID, DEM_E_PARAM_LENGTH);
        returnCode = DEM_GET_FFDATABYDTC_BUFFERSIZE;
    }

    SchM_Exit_Dem_EA_0();

    return returnCode;


}

/*
 * Procedure:   Dem_GetSizeOfFreezeFrame
 * Reentrant:   No
 */
 /** @req DEM238 */
Dem_ReturnGetSizeOfFreezeFrameType Dem_GetSizeOfFreezeFrameByDTC(uint32 dtc, Dem_DTCOriginType dtcOrigin, uint8 recordNumber, uint16* sizeOfFreezeFrame)
{
    /* NOTE: dtc is in UDS format according to DEM238 */
    Dem_ReturnGetSizeOfFreezeFrameType returnCode = DEM_GET_SIZEOFFF_PENDING;
    Dem_FreezeFrameClassType const *FFDataRecordClass = NULL;
    const Dem_EventParameterType *eventParam;
#if (DEM_EVENT_COMB_TYPE2_REPORT_OLDEST_DATA == STD_ON)
    uint16 tempSize;
#endif
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_GETSIZEOFFREEZEFRAMEBYDTC_ID, DEM_E_UNINIT, DEM_GET_SIZEOFFF_PENDING);
    VALIDATE_RV((NULL != sizeOfFreezeFrame), DEM_GETSIZEOFFREEZEFRAMEBYDTC_ID, DEM_E_PARAM_POINTER, DEM_GET_SIZEOFFF_WRONG_DTC);
    SchM_Enter_Dem_EA_0();
    const Dem_DTCClassType *DTCClass;
    if ( TRUE == LookupUdsDTC(dtc, &DTCClass) ) {
        *sizeOfFreezeFrame = 0u;
        for(uint16 i = 0; i < DTCClass->NofEvents; i++) {
            eventParam = NULL_PTR;
            lookupEventIdParameter(DTCClass->Events[i], &eventParam);
            if( NULL_PTR != eventParam ) {
                if (checkDtcOrigin(dtcOrigin, eventParam, FALSE) == TRUE) {
                    if (lookupFreezeFrameDataRecNumParam(recordNumber, eventParam, &FFDataRecordClass) == TRUE) {
                        if(FFDataRecordClass->FFIdClassRef != NULL){
                            /* Note - there is a function called lookupFreezeFrameDataSize that can be used here */
                            for(uint16 j = 0; (j < DEM_MAX_NR_OF_DIDS_IN_FREEZEFRAME_DATA) && ((FFDataRecordClass->FFIdClassRef[j]->Arc_EOL == FALSE)); j++){
                                /* read out the did size */
#if (DEM_EVENT_COMB_TYPE2_REPORT_OLDEST_DATA == STD_ON)
                                /* Report the biggest */
                                tempSize = (uint16)(FFDataRecordClass->FFIdClassRef[j]->PidOrDidSize + DEM_DID_IDENTIFIER_SIZE_OF_BYTES);
                                if( tempSize > *sizeOfFreezeFrame ) {
                                    *sizeOfFreezeFrame = tempSize;
                                }
#else
                                /* Report the total size */
                                *sizeOfFreezeFrame += (uint16)(FFDataRecordClass->FFIdClassRef[j]->PidOrDidSize + DEM_DID_IDENTIFIER_SIZE_OF_BYTES);/** @req DEM074 */
#endif
                                returnCode = DEM_GET_SIZEOFFF_OK;
                            }
                        } else {
                            returnCode = DEM_GET_SIZEOFFF_WRONG_RNUM;
                        }
                    } else {
                        returnCode = DEM_GET_SIZEOFFF_WRONG_RNUM;
                    }
                } else {
                    returnCode = DEM_GET_SIZEOFFF_WRONG_DTCOR;
                }
            }
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
            break;
#endif
        }
    } else {
        /* Event has no DTC or DTC is suppressed */
        /* @req 4.2.2/SWS_Dem_01100 */
        /* @req 4.2.2/SWS_Dem_01101 */
        /* @req DEM587 */
        returnCode = DEM_GET_SIZEOFFF_WRONG_DTC;
    }

    SchM_Exit_Dem_EA_0();
    return returnCode;


}

/**
 *
 * @param DTCFormat
 * @param NumberOfFilteredRecords
 * @return
 */
Dem_ReturnSetFilterType Dem_SetFreezeFrameRecordFilter(Dem_DTCFormatType DTCFormat, uint16 *NumberOfFilteredRecords)
{
    Dem_ReturnSetFilterType ret = DEM_WRONG_FILTER;
    uint16 nofRecords = 0;
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_SETFREEZEFRAMERECORDFILTER_ID, DEM_E_UNINIT, DEM_WRONG_FILTER);
    VALIDATE_RV((NULL != NumberOfFilteredRecords), DEM_SETFREEZEFRAMERECORDFILTER_ID, DEM_E_PARAM_POINTER, DEM_WRONG_FILTER);
    VALIDATE_RV(IS_VALID_DTC_FORMAT(DTCFormat), DEM_SETFREEZEFRAMERECORDFILTER_ID, DEM_E_PARAM_DATA, DEM_WRONG_FILTER);

    SchM_Enter_Dem_EA_0();

    /* @req DEM210 Only applies to primary memory */
#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM)
    for( uint16 i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM; i++ ) {
        if( DEM_EVENT_ID_NULL != priMemFreezeFrameBuffer[i].eventId ) {
            EventStatusRecType *eventStatusRecPtr = NULL_PTR;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
            if( !IS_COMBINED_EVENT_ID(priMemFreezeFrameBuffer[i].eventId) ) {
                /* Only do this if the entry is NOT a combined event entry. This to avoid Det error. */
                lookupEventStatusRec(priMemFreezeFrameBuffer[i].eventId, &eventStatusRecPtr);
            }
#else
            lookupEventStatusRec(priMemFreezeFrameBuffer[i].eventId, &eventStatusRecPtr);
#endif
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE2)
            /* Check if this record has already been counted. */
            boolean alreadyCounted = FALSE;
            for( uint16 j = 0; j < i; j++ ) {
                if( DEM_EVENT_ID_NULL != priMemFreezeFrameBuffer[j].eventId ) {
                    EventStatusRecType *eventStatusRecPtr2 = NULL_PTR;
                    lookupEventStatusRec(priMemFreezeFrameBuffer[j].eventId, &eventStatusRecPtr2);
                    if(  NULL_PTR != eventStatusRecPtr2) {
                        if( (eventStatusRecPtr->eventParamRef->DTCClassRef == eventStatusRecPtr2->eventParamRef->DTCClassRef) &&
                                (priMemFreezeFrameBuffer[j].recordNumber == priMemFreezeFrameBuffer[i].recordNumber)) {
                            /* Same DTC and record found earlier in buffer -> already counted. */
                            alreadyCounted = TRUE;
                        }
                    }
                }
            }
#endif
            if( (NULL_PTR != eventStatusRecPtr) && (TRUE == eventHasDTCOnFormat(eventStatusRecPtr->eventParamRef, DTCFormat)) &&
                    (TRUE == DTCIsAvailable(eventStatusRecPtr->eventParamRef->DTCClassRef))
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE2)
                    && (FALSE == alreadyCounted)
#endif
                    ) {
                nofRecords++;
            }
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
            if( (NULL_PTR == eventStatusRecPtr) && IS_COMBINED_EVENT_ID(priMemFreezeFrameBuffer[i].eventId) ) {
                /* This is a combined event entry. */
                /* Get DTC config */
                const Dem_CombinedDTCCfgType *CombDTCCfg = &configSet->CombinedDTCConfig[TO_COMBINED_EVENT_CFG_IDX(priMemFreezeFrameBuffer[i].eventId)];
                if( (TRUE == DTCISAvailableOnFormat(CombDTCCfg->DTCClassRef, DTCFormat)) &&
                        (TRUE == DTCIsAvailable(CombDTCCfg->DTCClassRef)) ) {
                    nofRecords++;
                }
            }
#endif
        }
    }
#endif
    *NumberOfFilteredRecords = nofRecords;
    /* @req DEM595 */
    ffRecordFilter.dtcFormat = DTCFormat;
    ffRecordFilter.ffIndex = 0;

    ret = DEM_FILTER_ACCEPTED;

    SchM_Exit_Dem_EA_0();
    return ret;
}

Dem_ReturnGetNextFilteredDTCType Dem_GetNextFilteredRecord(uint32 *DTC, uint8 *RecordNumber)
{
    /* No requirement on checking the pointers but do it anyway. */
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_GETNEXTFILTEREDRECORD_ID, DEM_E_UNINIT, DEM_FILTERED_NO_MATCHING_DTC);
    VALIDATE_RV(NULL != DTC, DEM_GETNEXTFILTEREDRECORD_ID, DEM_E_PARAM_POINTER, DEM_FILTERED_NO_MATCHING_DTC);
    VALIDATE_RV(NULL != RecordNumber, DEM_GETNEXTFILTEREDRECORD_ID, DEM_E_PARAM_POINTER, DEM_FILTERED_NO_MATCHING_DTC);
    Dem_ReturnGetNextFilteredDTCType ret = DEM_FILTERED_NO_MATCHING_DTC;
    SchM_Enter_Dem_EA_0();

#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM)
    /* Find the next record which has a DTC */
    EventStatusRecType *eventStatusRecPtr;
    boolean found = FALSE;
    for( uint16 i = ffRecordFilter.ffIndex; (i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM) && (FALSE == found); i++  ) {
        if( DEM_EVENT_ID_NULL != priMemFreezeFrameBuffer[i].eventId ) {
            eventStatusRecPtr = NULL_PTR;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
            if( !IS_COMBINED_EVENT_ID(priMemFreezeFrameBuffer[i].eventId) ) {
                /* Only do this if the entry is NOT a combined event entry. This to avoid Det error. */
                lookupEventStatusRec(priMemFreezeFrameBuffer[i].eventId, &eventStatusRecPtr);
            }
#else
            lookupEventStatusRec(priMemFreezeFrameBuffer[i].eventId, &eventStatusRecPtr);
#endif
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE2)
            /* Check if this record has already been counted. */
            boolean alreadyCounted = FALSE;
            for( uint16 j = 0; j < i; j++ ) {
                if( DEM_EVENT_ID_NULL != priMemFreezeFrameBuffer[j].eventId ) {
                    EventStatusRecType *eventStatusRecPtr2 = NULL_PTR;
                    lookupEventStatusRec(priMemFreezeFrameBuffer[j].eventId, &eventStatusRecPtr2);
                    if(  NULL_PTR != eventStatusRecPtr2) {
                        if( (eventStatusRecPtr->eventParamRef->DTCClassRef == eventStatusRecPtr2->eventParamRef->DTCClassRef) &&
                                (priMemFreezeFrameBuffer[j].recordNumber == priMemFreezeFrameBuffer[i].recordNumber)) {
                            /* Same DTC and record found earlier in buffer -> already counted. */
                            alreadyCounted = TRUE;
                        }
                    }
                }
            }
#endif
            /* @req 4.2.2/SWS_Dem_01101 *//* @req DEM587 */
            if( (NULL != eventStatusRecPtr) && (TRUE == eventHasDTCOnFormat(eventStatusRecPtr->eventParamRef, ffRecordFilter.dtcFormat)) &&
                    (TRUE == DTCIsAvailable(eventStatusRecPtr->eventParamRef->DTCClassRef))
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE2)
                    && (FALSE == alreadyCounted)
#endif
            ) {
                /* Found one not already reported! */
                /* @req DEM225 */
                *RecordNumber = priMemFreezeFrameBuffer[i].recordNumber;
                *DTC = (DEM_DTC_FORMAT_UDS == ffRecordFilter.dtcFormat) ?
                        eventStatusRecPtr->eventParamRef->DTCClassRef->DTCRef->UDSDTC : TO_OBD_FORMAT(eventStatusRecPtr->eventParamRef->DTCClassRef->DTCRef->OBDDTC);
                /* @req DEM226 */
                ffRecordFilter.ffIndex = i + 1;
                found = TRUE;
                ret = DEM_FILTERED_OK;
            }
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
            if( (NULL_PTR == eventStatusRecPtr) && IS_COMBINED_EVENT_ID(priMemFreezeFrameBuffer[i].eventId) ) {
                /* This is a combined event entry. */
                /* Get DTC config */
                const Dem_CombinedDTCCfgType *CombDTCCfg = &configSet->CombinedDTCConfig[TO_COMBINED_EVENT_CFG_IDX(priMemFreezeFrameBuffer[i].eventId)];
                if( (TRUE == DTCISAvailableOnFormat(CombDTCCfg->DTCClassRef, ffRecordFilter.dtcFormat)) &&
                        (TRUE == DTCIsAvailable(CombDTCCfg->DTCClassRef)) ) {
                    *RecordNumber = priMemFreezeFrameBuffer[i].recordNumber;
                    *DTC = (DEM_DTC_FORMAT_UDS == ffRecordFilter.dtcFormat) ?
                            CombDTCCfg->DTCClassRef->DTCRef->UDSDTC : TO_OBD_FORMAT(CombDTCCfg->DTCClassRef->DTCRef->OBDDTC);
                    ffRecordFilter.ffIndex = i + 1;
                    found = TRUE;
                    ret = DEM_FILTERED_OK;
                }
            }
#endif
        }
    }
#endif

    SchM_Exit_Dem_EA_0();
    return ret;
    /*lint -e{818} *RecordNumber and *DTC these pointers are updated under DEM_USE_PRIMARY_MEMORY_SUPPORT condition   */
}

#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
/* @req DEM202 */
Std_ReturnType Dem_SetEnableCondition(uint8 EnableConditionID, boolean ConditionFulfilled)
{
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_SETENABLECONDITION_ID, DEM_E_UNINIT, E_NOT_OK);
    VALIDATE_RV((EnableConditionID < DEM_NUM_ENABLECONDITIONS), DEM_SETENABLECONDITION_ID, DEM_E_PARAM_DATA, E_NOT_OK);

    DemEnableConditions[EnableConditionID] = ConditionFulfilled;

    return E_OK;
}
#endif

/* Function: Dem_GetSeverityOfDTC
 * Description: Gets the severity of a DTC
 */
Dem_ReturnGetSeverityOfDTCType Dem_GetSeverityOfDTC(uint32 DTC, Dem_DTCSeverityType* DTCSeverity)
{
    /* NOTE: DTC is on UDS format according to DEM232 */
    Dem_ReturnGetSeverityOfDTCType ret = DEM_GET_SEVERITYOFDTC_WRONG_DTC;
    boolean isDone = FALSE;
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_GETSEVERITYOFDTC_ID, DEM_E_UNINIT, DEM_GET_SEVERITYOFDTC_PENDING);
    VALIDATE_RV((NULL != DTCSeverity), DEM_GETSEVERITYOFDTC_ID, DEM_E_PARAM_POINTER, DEM_GET_SEVERITYOFDTC_PENDING);

    const Dem_DTCClassType *DTCPtr = configSet->DTCClass;
    while( (DTCPtr->Arc_EOL == FALSE) && (isDone == FALSE) ) {
        if( (DEM_NO_DTC != DTCPtr->DTCRef->UDSDTC) && (DTC == DTCPtr->DTCRef->UDSDTC) ) {
            /* Dtc found */
            isDone = TRUE;
            if( DTCIsAvailable(DTCPtr) == TRUE ) {
                *DTCSeverity = DTCPtr->DTCSeverity;
                if( DEM_SEVERITY_NO_SEVERITY == DTCPtr->DTCSeverity ) {
                    ret = DEM_GET_SEVERITYOFDTC_NOSEVERITY;
                } else {
                    ret = DEM_GET_SEVERITYOFDTC_OK;
                }
            }
            else {
                /* @req 4.2.2/SWS_Dem_01100 */
                /* Ignore suppressed DTCs *//* @req DEM587 */
                ret = DEM_GET_SEVERITYOFDTC_WRONG_DTC;
            }
        }
        DTCPtr++;
    }

    return ret;
}

/* Function: Dem_DisableDTCRecordUpdate
 * Description: Disables the event memory update of a specific DTC (only one at one time)
 */
Dem_ReturnDisableDTCRecordUpdateType Dem_DisableDTCRecordUpdate(uint32 DTC, Dem_DTCOriginType DTCOrigin)
{
    /* NOTE: DTC in in UDS format according to DEM233 */
    Dem_ReturnDisableDTCRecordUpdateType ret = DEM_DISABLE_DTCRECUP_WRONG_DTC;

    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_DISABLEDTCRECORDUPDATE_ID, DEM_E_UNINIT, DEM_DISABLE_DTCRECUP_PENDING);

    if(NO_DTC_DISABLED == DTCRecordDisabled.DTC) {
        const Dem_EventParameterType *eventIdParamPtr =  configSet->EventParameter;
        while( (eventIdParamPtr->Arc_EOL == FALSE) && (DEM_DISABLE_DTCRECUP_OK != ret)) {
            if( (NULL != eventIdParamPtr->DTCClassRef) && (eventHasDTCOnFormat(eventIdParamPtr, DEM_DTC_FORMAT_UDS) == TRUE) &&
                    (eventIdParamPtr->DTCClassRef->DTCRef->UDSDTC == DTC) && (DTCIsAvailable(eventIdParamPtr->DTCClassRef) == TRUE)) {
                /* Event references this DTC */
                ret = DEM_DISABLE_DTCRECUP_WRONG_DTCORIGIN;
                if( eventIdParamPtr->EventClass->EventDestination == DTCOrigin ) {
                    /* Event destination match. Disable update for this DTC and the event destination */
                    /* @req DEM270 */
                    DTCRecordDisabled.DTC = DTC;
                    DTCRecordDisabled.Origin = DTCOrigin;
                    ret = DEM_DISABLE_DTCRECUP_OK;
                }
            }
            eventIdParamPtr++;
        }
    } else if (DTCRecordDisabled.DTC != DTC) {
        /* The previously disabled DTC has not been enabled */
        /* @req DEM648 *//* @req DEM518 */
        DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_DISABLEDTCRECORDUPDATE_ID, DEM_E_WRONG_CONDITION);
        ret = DEM_DISABLE_DTCRECUP_PENDING;
    } else {
        ret = DEM_DISABLE_DTCRECUP_OK;
    }

    return ret;
}

/* Function: Dem_EnableDTCRecordUpdate
 * Description: Enables the event memory update of the DTC disabled by Dem_DisableDTCRecordUpdate() before.
 */
Std_ReturnType Dem_EnableDTCRecordUpdate(void)
{
    Std_ReturnType ret = E_NOT_OK;
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_ENABLEDTCRECORDUPDATE_ID, DEM_E_UNINIT, E_NOT_OK);

    if(NO_DTC_DISABLED != DTCRecordDisabled.DTC) {
        /* @req DEM271 */
        DTCRecordDisabled.DTC = NO_DTC_DISABLED;
        ret = E_OK;
    } else {
        /* No DTC record update has been disabled */
        DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_ENABLEDTCRECORDUPDATE_ID, DEM_E_SEQUENCE_ERROR);
    }

    return ret;
}


/**
 * Gets the data of a freeze frame by event
 * @param EventId
 * @param RecordNumber
 * @param ReportTotalRecord
 * @param DataId
 * @param DestBuffer
 * @param BufSize
 * @param CareAboutBufsize
 * @return E_OK: Operation was successful, E_NOT_OK: Operation failed
 */
static Std_ReturnType Dem_GetEventFreezeFrameData_Internal(Dem_EventIdType EventId, uint8 RecordNumber, boolean ReportTotalRecord,
        uint16 DataId, uint8* DestBuffer, uint8* BufSize, boolean CareAboutBufsize)
{
    /* @req DEM478*/
    /* @req DEM479 */
    Std_ReturnType ret = E_NOT_OK;
    const Dem_EventParameterType *eventIdParamPtr = NULL_PTR;
    uint8 recordToReport = 0u;
    uint8 destBufferIndex = 0u;
    uint8 *ffRecordData;
    EventStatusRecType * eventStatusRec = NULL_PTR;
    Dem_FreezeFrameKindType ffKind = DEM_FREEZE_FRAME_NON_OBD;
    if( DEM_INITIALIZED == demState  ) {
        lookupEventIdParameter(EventId, &eventIdParamPtr);

        lookupEventStatusRec(EventId, &eventStatusRec);
        if( (NULL != eventIdParamPtr) && (NULL != eventStatusRec) && (TRUE == eventStatusRec->isAvailable) ) {
            /* Event has freeze frames configured */
            if(getFFRecData(eventIdParamPtr, RecordNumber, &ffRecordData, &recordToReport, &ffKind) == TRUE) {
                /* And the record we are looking for was found */
                const Dem_FreezeFrameClassType *freezeFrameClass = NULL_PTR;
                const Dem_PidOrDidType * const *xidPtr;
                if(DEM_FREEZE_FRAME_NON_OBD == ffKind ) {
                	getFFClassReference(eventIdParamPtr, (Dem_FreezeFrameClassType **) &freezeFrameClass);
                    xidPtr = freezeFrameClass->FFIdClassRef;
                } else {
                    freezeFrameClass = configSet->GlobalOBDFreezeFrameClassRef;
                    xidPtr = freezeFrameClass->FFIdClassRef;
                }
                uint16 ffDataIndex = 0u;
                boolean done = FALSE;
                while( ((*xidPtr)->Arc_EOL == FALSE) && (done == FALSE)) {
                    if(DEM_FREEZE_FRAME_NON_OBD == ffKind ) {
                        ffDataIndex += DEM_DID_IDENTIFIER_SIZE_OF_BYTES;
                    } else {
                        ffDataIndex += DEM_PID_IDENTIFIER_SIZE_OF_BYTES;
                    }
                    if( (ReportTotalRecord == TRUE) ||
                       ((DEM_FREEZE_FRAME_NON_OBD == ffKind) && (DataId == (*xidPtr)->DidIdentifier)) ||
                       ((DEM_FREEZE_FRAME_NON_OBD != ffKind) && (DataId == (*xidPtr)->PidIdentifier))) {
                        if(CareAboutBufsize == TRUE){
                            if(((*xidPtr)->PidOrDidSize + destBufferIndex) > *BufSize){
                                *BufSize = destBufferIndex;
                                return E_NOT_OK; /* buffer full, no info in DLT spec what to do for this case so we return operation failed */
                            }
                        }
                        memcpy(&DestBuffer[destBufferIndex], &ffRecordData[ffDataIndex], (size_t)((*xidPtr)->PidOrDidSize));
                        destBufferIndex += (*xidPtr)->PidOrDidSize;
                        done = (ReportTotalRecord == FALSE)? TRUE: FALSE;
                        ret = E_OK;
                    }
                    ffDataIndex += (uint16)((*xidPtr)->PidOrDidSize);
                    xidPtr++;
                }
            }
        }
    } else {
        DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GETEVENTFREEZEFRAMEDATA_ID, DEM_E_UNINIT);
    }

    *BufSize = destBufferIndex;
    return ret;
}

/**
 * Gets the data of a freeze frame by event
 * @param EventId
 * @param RecordNumber
 * @param ReportTotalRecord
 * @param DataId
 * @param DestBuffer
 * @return E_OK: Operation was successful, E_NOT_OK: Operation failed
 */
Std_ReturnType Dem_GetEventFreezeFrameData(Dem_EventIdType EventId, uint8 RecordNumber, boolean ReportTotalRecord, uint16 DataId, uint8* DestBuffer)
{
    uint8 BufSize = 0x0; /* dummy size not used */
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_GETEVENTFREEZEFRAMEDATA_ID, DEM_E_UNINIT, E_NOT_OK);
    VALIDATE_RV((NULL != DestBuffer), DEM_GETEVENTFREEZEFRAMEDATA_ID, DEM_E_PARAM_POINTER, E_NOT_OK);
    return Dem_GetEventFreezeFrameData_Internal(EventId,RecordNumber,ReportTotalRecord,DataId,DestBuffer, &BufSize, FALSE);
}

#if (DEM_TRIGGER_DLT_REPORTS == STD_ON)
/**
 * Gets the most recent data of a freeze frame by event for DLT
 * @param EventId
 * @param DestBuffer
 * @param BufSize
 * @return E_OK: Operation was successful, E_NOT_OK: Operation failed
 */
Std_ReturnType Dem_DltGetMostRecentFreezeFrameRecordData(Dem_EventIdType EventId, uint8* DestBuffer, uint8* BufSize){
    /* @req DEM632 */
    /* @req DEM633 */
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_DLTGETMOSTRECENTFREEZEFRAMERECORDDATA_ID, DEM_E_UNINIT, E_NOT_OK);
    VALIDATE_RV(((NULL != DestBuffer) && (NULL != BufSize)), DEM_DLTGETMOSTRECENTFREEZEFRAMERECORDDATA_ID, DEM_E_PARAM_POINTER, E_NOT_OK);
	return Dem_GetEventFreezeFrameData_Internal(EventId, MOST_RECENT_FF_RECORD, TRUE,0, DestBuffer, BufSize, TRUE);
}
#endif


/**
 * Gets the data of an extended data record by event
 * @param EventId
 * @param RecordNumber
 * @param DestBuffer
 * @param BufSize
 * @param CareAboutBufsize
 * @return E_OK: Operation was successful, E_NOT_OK: Operation failed
 */
static Std_ReturnType Dem_GetEventExtendedDataRecord_Internal(Dem_EventIdType EventId, uint8 RecordNumber, uint8* DestBuffer, uint8* BufSize, boolean CareAboutBufsize)
{
    /* @req DEM476 */
    /* @req DEM477 */
    Std_ReturnType ret = E_NOT_OK;
    const Dem_EventParameterType *eventIdParamPtr = NULL_PTR;
    Dem_ExtendedDataRecordClassType const *extendedDataRecordClass = NULL_PTR;
    ExtDataRecType *extData;
    uint16 posInExtData = 0u;
    uint16 extdataIndex = 0u;
    uint8 extendedDataNumber = RecordNumber;
    boolean done = FALSE;
    boolean readFailed = FALSE;
    uint8 destBufferIndex = 0u;
    EventStatusRecType * eventStatusRec = NULL_PTR;

    if( DEM_INITIALIZED == demState  ) {
        if( IS_VALID_EXT_DATA_RECORD(RecordNumber) || (ALL_EXTENDED_DATA_RECORDS == RecordNumber) ) {
            /* Record number ok */
            lookupEventIdParameter(EventId, &eventIdParamPtr);
            lookupEventStatusRec(EventId, &eventStatusRec);
            if( (NULL != eventIdParamPtr) && (NULL != eventIdParamPtr->ExtendedDataClassRef) &&
                    (NULL != eventStatusRec) && (TRUE == eventStatusRec->isAvailable) ) {
                /* Event ok and has extended data */
                ret = E_OK;
                while( (done == FALSE) && (NULL != eventIdParamPtr->ExtendedDataClassRef->ExtendedDataRecordClassRef[extdataIndex])) {
                    readFailed = TRUE;
                    if( ALL_EXTENDED_DATA_RECORDS == RecordNumber) {
                        extendedDataNumber = eventIdParamPtr->ExtendedDataClassRef->ExtendedDataRecordClassRef[extdataIndex]->RecordNumber;
                    } else {
                        /* Should only read one specific record */
                        done = TRUE;
                    }
                    if (lookupExtendedDataRecNumParam(extendedDataNumber, eventIdParamPtr, &extendedDataRecordClass, &posInExtData) == TRUE) {
                        if(CareAboutBufsize == TRUE){
                            if((extendedDataRecordClass->DataSize + destBufferIndex) > *BufSize){
                                *BufSize = destBufferIndex;
                                return E_NOT_OK; /* buffer full, no info in DLT spec what to do for this case so we return operation failed */
                            }
                        }
                        if( extendedDataRecordClass->UpdateRule != DEM_UPDATE_RECORD_VOLATILE ) {
                            if (lookupExtendedDataMem(EventId, &extData, eventIdParamPtr->EventClass->EventDestination) == TRUE ) {
                                // Yes all conditions met, copy the extended data record to destination buffer.
                                memcpy(&DestBuffer[destBufferIndex], &extData->data[posInExtData], (size_t)extendedDataRecordClass->DataSize); /** @req DEM075 */
                                destBufferIndex += (uint8)extendedDataRecordClass->DataSize;
                                readFailed = FALSE;
                            }
                        } else {
                            if( NULL != extendedDataRecordClass->CallbackGetExtDataRecord ) {
                                if(E_OK == extendedDataRecordClass->CallbackGetExtDataRecord(&DestBuffer[destBufferIndex])) {
                                    readFailed = FALSE;
                                }
                                destBufferIndex += (uint8)extendedDataRecordClass->DataSize;
                            }  else if (DEM_NO_ELEMENT != extendedDataRecordClass->InternalDataElement ) {
                                getInternalElement(eventIdParamPtr, extendedDataRecordClass->InternalDataElement, &DestBuffer[destBufferIndex], extendedDataRecordClass->DataSize );
                                destBufferIndex += (uint8)extendedDataRecordClass->DataSize;
                                readFailed = FALSE;
                            } else {
                               /* No callback and no internal element.
                                * IMPROVMENT: Det_error */
                            }
                        }
                    }
                    if( readFailed == TRUE) {
                        /* Something failed reading the data */
                        done = TRUE;
                        ret = E_NOT_OK;
                    }
                    extdataIndex++;
                }
            }
        }
    } else {
        DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GETEVENTEXTENDEDDATARECORD_ID, DEM_E_UNINIT);
    }

    *BufSize = destBufferIndex;
    return ret;
}

/**
 * Gets the data of an extended data record by event
 * @param EventId
 * @param RecordNumber
 * @param DestBuffer
 * @return E_OK: Operation was successful, E_NOT_OK: Operation failed
 */
Std_ReturnType Dem_GetEventExtendedDataRecord(Dem_EventIdType EventId, uint8 RecordNumber, uint8* DestBuffer)
{
    uint8 Bufsize = 0; /* Dummy size not used */
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_GETEVENTEXTENDEDDATARECORD_ID, DEM_E_UNINIT, E_NOT_OK);
    VALIDATE_RV((NULL != DestBuffer), DEM_GETEVENTEXTENDEDDATARECORD_ID, DEM_E_PARAM_POINTER, E_NOT_OK);
    return Dem_GetEventExtendedDataRecord_Internal(EventId, RecordNumber, DestBuffer, &Bufsize, FALSE);
}

#if (DEM_TRIGGER_DLT_REPORTS == STD_ON)
/**
 * Gets all the data of an extended data record by event for DLT
 * @param EventId
 * @param DestBuffer
 * @param BufSize
 * @return E_OK: Operation was successful, E_NOT_OK: Operation failed
 */
Std_ReturnType Dem_DltGetAllExtendedDataRecords(Dem_EventIdType EventId, uint8* DestBuffer, uint8* BufSize){
    /* @req DEM634 */
    /* @req DEM635 */
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_DLTGETALLEXTENDEDDATARECORDS_ID, DEM_E_UNINIT, E_NOT_OK);
    VALIDATE_RV(((NULL != DestBuffer) && (NULL != BufSize)), DEM_DLTGETALLEXTENDEDDATARECORDS_ID, DEM_E_PARAM_POINTER, E_NOT_OK);
 	return Dem_GetEventExtendedDataRecord_Internal(EventId, ALL_EXTENDED_DATA_RECORDS, DestBuffer, BufSize, TRUE);
}
#endif

#if ( DEM_PRESTORAGE_FF_DATA_IN_MEM )
/**
 * Captures the freeze frame data for a specific event.
 * @param EventId
 * @return E_OK: Freeze frame prestorage was successful, E_NOT_OK: Freeze frame prestorage failed
 */
/* @req DEM188 */
/* @req DEM189 */
/* @req DEM334 */
Std_ReturnType Dem_PrestoreFreezeFrame(Dem_EventIdType EventId)
{
    Std_ReturnType ret = E_NOT_OK;
    const Dem_EventParameterType *eventParam;
    EventStatusRecType *eventStatusRec = NULL;
    FreezeFrameRecType freezeFrame = {0};

    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_PRE_STORE_FF_ID, DEM_E_UNINIT, E_NOT_OK);
    VALIDATE_RV(IS_VALID_EVENT_ID(EventId), DEM_PRE_STORE_FF_ID, DEM_E_PARAM_DATA, E_NOT_OK);

    /* Find eventParameter to each eventId belongs */
    lookupEventIdParameter(EventId, &eventParam);

    /* Find event status */
    lookupEventStatusRec(EventId, &eventStatusRec);

    if ( (eventParam != NULL) && (eventStatusRec != NULL) ) {
        if (eventStatusRec->isAvailable == TRUE) {
            if (eventParam->EventClass->FFPrestorageSupported == TRUE) {
                /* To see if it is NON-OBD check if there is a FF class configured for this event */
                if( DEM_FF_NULLREF != getFFIdx(eventParam)) {
                    getFreezeFrameData(eventParam, &freezeFrame, DEM_FREEZE_FRAME_NON_OBD, DEM_DTC_ORIGIN_NOT_USED, TRUE);

                    if (freezeFrame.eventId != DEM_EVENT_ID_NULL) {
                        /* is there already pre-stored FFs */
                        if (storeFreezeFrameDataMem(eventParam, &freezeFrame, memPreStoreFreezeFrameBuffer, DEM_MAX_NUMBER_PRESTORED_FF, DEM_DTC_ORIGIN_NOT_USED) == TRUE) { /** @req DEM190 */
                            ret = E_OK;
                        }
                    }
                }

                /* check for OBD */
                if( DEM_NON_EMISSION_RELATED != (Dem_Arc_EventDTCKindType) *eventParam->EventDTCKind ) {
                    getFreezeFrameData(eventParam, &freezeFrame, DEM_FREEZE_FRAME_OBD, DEM_DTC_ORIGIN_NOT_USED, FALSE);

                    if (freezeFrame.eventId != DEM_EVENT_ID_NULL) {
                        /* is there already pre-stored OBD FFs */
                        if (storeFreezeFrameDataMem(eventParam, &freezeFrame, memPreStoreFreezeFrameBuffer, DEM_MAX_NUMBER_PRESTORED_FF, DEM_DTC_ORIGIN_NOT_USED) == TRUE) { /** @req DEM190 */
                            ret = E_OK;
                        }
                    }
                }
            }
        }
    }
    return ret;
}

/**
 * Clears a prestored freeze frame of a specific event.
 * @param EventId
 * @return E_OK: Clear prestored freeze frame was successful, E_NOT_OK: Clear prestored freeze frame failed
 */
/* @req DEM193 */
/* @req DEM050 */
/* @req DEM334 */
Std_ReturnType Dem_ClearPrestoredFreezeFrame(Dem_EventIdType EventId)
{
    Std_ReturnType ret = E_NOT_OK;
    const Dem_EventParameterType *eventParam;

    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_CLEAR_PRE_STORED_FF_ID, DEM_E_UNINIT, E_NOT_OK);
    VALIDATE_RV(IS_VALID_EVENT_ID(EventId), DEM_CLEAR_PRE_STORED_FF_ID, DEM_E_PARAM_DATA, E_NOT_OK);

    /* Find eventParameter to each eventId belongs */
    lookupEventIdParameter(EventId, &eventParam);

    if ( eventParam != NULL ) {
        if (eventParam->EventClass->FFPrestorageSupported == TRUE) {
            boolean combinedDTC = FALSE;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
            if( DEM_COMBINED_EVENT_NO_DTC_ID != eventParam->CombinedDTCCID ) {
                combinedDTC = TRUE;
            }
#endif
            /* It could be OBD or non-OBD */
            if (deleteFreezeFrameDataMem(eventParam, DEM_DTC_ORIGIN_NOT_USED, combinedDTC) == TRUE) { /** @req DEM190 */
                ret = E_OK;
            }
        } else {
            ret = E_NOT_OK; // DemFFPrestorage is not supported for this EventClass
        }
    } else {
        ret = E_NOT_OK; // Event ID not configured or set to not available
    }

    return ret;
}
#endif

/**
 * Gets the event memory overflow indication status
 * @param DTCOrigin
 * @param OverflowIndication
 * @return E_OK: Operation was successful, E_NOT_OK: Operation failed or is not supported
 */
/* @req DEM559 *//* @req DEM398 */
Std_ReturnType Dem_GetEventMemoryOverflow(Dem_DTCOriginType DTCOrigin, boolean *OverflowIndication)
{
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_GETEVENTMEMORYOVERFLOW_ID, DEM_E_UNINIT, E_NOT_OK)
    VALIDATE_RV(NULL != OverflowIndication, DEM_GETEVENTMEMORYOVERFLOW_ID, DEM_E_PARAM_POINTER, E_NOT_OK);
    return getOverflowIndication(DTCOrigin, OverflowIndication);
}

#if (DEM_UNIT_TEST == STD_ON)
#if ( DEM_FF_DATA_IN_PRE_INIT )
void getFFDataPreInit(FreezeFrameRecType **buf);
void getFFDataPreInit(FreezeFrameRecType **buf)
{
    *buf = &preInitFreezeFrameBuffer[0];
    return;
}
#endif
#if (DEM_USE_TIMESTAMPS == STD_ON)
uint32 getCurTimeStamp(void)
{
    return FF_TimeStamp;
}
#endif
void getEventStatusBufPtr(EventStatusRecType **buf);
void getEventStatusBufPtr(EventStatusRecType **buf)
{
    *buf = &eventStatusBuffer[0];
    return;
}
#endif /* DEM_UNIT_TEST */





/****************
 * OBD-specific *
 ***************/
/*
 * Procedure:   Dem_GetDTCOfOBDFreezeFrame
 * Reentrant:   No
 */
 /* @req OBD_DEM_REQ_3 */
Std_ReturnType Dem_GetDTCOfOBDFreezeFrame(uint8 FrameNumber, uint32* DTC )
{
    /* @req DEM623 */
    const FreezeFrameRecType *freezeFrame = NULL;
    const Dem_EventParameterType *eventParameter = NULL;
    Std_ReturnType returnCode = E_NOT_OK;
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_GETDTCOFOBDFREEZEFRAME_ID, DEM_E_UNINIT, E_NOT_OK);
    VALIDATE_RV(NULL != DTC, DEM_GETDTCOFOBDFREEZEFRAME_ID, DEM_E_PARAM_POINTER, E_NOT_OK);
    VALIDATE_RV(0 == FrameNumber, DEM_GETDTCOFOBDFREEZEFRAME_ID, DEM_E_PARAM_DATA, E_NOT_OK);

    /* find the corresponding FF in FF buffer */
    /* @req OBD_DEM_REQ_1 */
#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM)
        for(uint16 i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM; i++){
            if((priMemFreezeFrameBuffer[i].eventId != DEM_EVENT_ID_NULL)
                && (DEM_FREEZE_FRAME_OBD == priMemFreezeFrameBuffer[i].kind)){
                freezeFrame = &priMemFreezeFrameBuffer[i];
                break;
            }
        }
#endif
    /*if FF found,find the corresponding eventParameter*/
    if( freezeFrame != NULL ) {
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
        if( !IS_COMBINED_EVENT_ID(freezeFrame->eventId) ) {
            /* Only do this if the entry is NOT a combined event entry. This to avoid Det error. */
            lookupEventIdParameter(freezeFrame->eventId, &eventParameter);
        }
#else
        lookupEventIdParameter(freezeFrame->eventId, &eventParameter);
#endif

        if(eventParameter != NULL){
            /* if DTCClass configured,get DTC value */
            if((eventParameter->DTCClassRef != NULL) && (DTCIsAvailable(eventParameter->DTCClassRef) == TRUE) && (eventHasDTCOnFormat(eventParameter, DEM_DTC_FORMAT_OBD) == TRUE)){
                *DTC = TO_OBD_FORMAT(eventParameter->DTCClassRef->DTCRef->OBDDTC);
                returnCode = E_OK;
            }
            else {
                /* Event has no DTC or DTC is suppressed */
                /* @req 4.2.2/SWS_Dem_01101 *//* @req DEM587 */
            }
        }
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
        else {
            if( IS_COMBINED_EVENT_ID(freezeFrame->eventId) ) {
                const Dem_CombinedDTCCfgType *CombDTCCfg = &configSet->CombinedDTCConfig[TO_COMBINED_EVENT_CFG_IDX(freezeFrame->eventId)];
                if( (TRUE == DTCISAvailableOnFormat(CombDTCCfg->DTCClassRef, DEM_DTC_FORMAT_OBD)) &&
                        (TRUE == DTCIsAvailable(CombDTCCfg->DTCClassRef)) ) {
                    *DTC = TO_OBD_FORMAT(CombDTCCfg->DTCClassRef->DTCRef->OBDDTC);
                    returnCode = E_OK;
                }
            }
        }
#endif

    }

    return returnCode;

}

/*
 * Procedure:   Dem_ReadDataOfOBDFreezeFrame
 * Reentrant:   No
 */
 /* @req OBD_DEM_REQ_2 */
/*lint -efunc(818,Dem_ReadDataOfOBDFreezeFrame) Pointers cannot be declared as pointing to const as API defined by AUTOSAR  */
Std_ReturnType Dem_ReadDataOfOBDFreezeFrame(uint8 PID, uint8 DataElementIndexOfPid, uint8* DestBuffer, uint8* BufSize)
{
    /* IMPROVEMENT: Validate parameters */
    /* @req DEM596 */
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_READDATAOFOBDFREEZEFRAME_ID, DEM_E_UNINIT, E_NOT_OK);
    VALIDATE_RV(NULL != DestBuffer, DEM_READDATAOFOBDFREEZEFRAME_ID, DEM_E_PARAM_POINTER, E_NOT_OK);
    VALIDATE_RV(NULL != BufSize, DEM_READDATAOFOBDFREEZEFRAME_ID, DEM_E_PARAM_POINTER, E_NOT_OK);
    Std_ReturnType returnCode = E_NOT_OK;

    /* IMPROVEMENT: DataElementIndexOfPid should be used to get the data of the Pid. But we only support 1 data element
     * per Pid.. */
    (void)DataElementIndexOfPid;

#if (DEM_MAX_NR_OF_PIDS_IN_FREEZEFRAME_DATA > 0)
    const FreezeFrameRecType *freezeFrame = NULL;
    const Dem_FreezeFrameClassType *freezeFrameClass;
    boolean pidFound = FALSE;
    uint16 offset = 0;
    uint8 pidDataSize = 0u;
    SchM_Enter_Dem_EA_0();
    freezeFrameClass = configSet->GlobalOBDFreezeFrameClassRef;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE2) && ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM)
    uint8 bufSizeLeft = *BufSize;
    uint32 timestamp = 0u;
    for(uint16 i = 0u; i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM; i++) {
        /* @req OBD_DEM_REQ_1 */
        if((priMemFreezeFrameBuffer[i].eventId != DEM_EVENT_ID_NULL) && (DEM_FREEZE_FRAME_OBD == priMemFreezeFrameBuffer[i].kind)) {
            freezeFrame = &priMemFreezeFrameBuffer[i];
            if( (freezeFrameClass->FFIdClassRef != NULL) && ((FALSE == pidFound) || (priMemFreezeFrameBuffer[i].timeStamp < timestamp)) ) {
                timestamp = priMemFreezeFrameBuffer[i].timeStamp;
                offset = 0u;
                for(uint16 pidIdx = 0u; (pidIdx < DEM_MAX_NR_OF_PIDS_IN_FREEZEFRAME_DATA) && ((freezeFrameClass->FFIdClassRef[pidIdx]->Arc_EOL) == FALSE); pidIdx++) {
                    offset += DEM_PID_IDENTIFIER_SIZE_OF_BYTES;
                    if(freezeFrameClass->FFIdClassRef[pidIdx]->PidIdentifier == PID) {
                        pidFound = TRUE;
                        /* Found. Copy the data. */
                        if( (bufSizeLeft >= freezeFrameClass->FFIdClassRef[pidIdx]->PidOrDidSize) && (PID == (freezeFrame->data[offset - DEM_PID_IDENTIFIER_SIZE_OF_BYTES]))
                            && ((offset + (uint16)freezeFrameClass->FFIdClassRef[pidIdx]->PidOrDidSize) <= (uint16)(freezeFrame->dataSize))
                            && ((offset + (uint16)freezeFrameClass->FFIdClassRef[pidIdx]->PidOrDidSize) <= DEM_MAX_SIZE_FF_DATA)) {
                            memcpy(&DestBuffer[pidDataSize], &freezeFrame->data[offset], (size_t)freezeFrameClass->FFIdClassRef[pidIdx]->PidOrDidSize);
                            returnCode = E_OK;
                        }
                        else {
                            /* Something wrong */
                            returnCode = E_NOT_OK;
                        }
                        pidDataSize = freezeFrameClass->FFIdClassRef[pidIdx]->PidOrDidSize;
                        break;
                    }
                    else {
                        offset += (uint16)freezeFrameClass->FFIdClassRef[pidIdx]->PidOrDidSize;
                    }
                }
            }
        }
    }
    if( (E_OK == returnCode) && (TRUE == pidFound) ) {
        *BufSize = pidDataSize;
    }
#else
#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM)
    /*find the corresponding FF in FF buffer*/
    for(uint16 i = 0u; i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM; i++) {
        /* @req OBD_DEM_REQ_1 */
        if((priMemFreezeFrameBuffer[i].eventId != DEM_EVENT_ID_NULL)
            && (DEM_FREEZE_FRAME_OBD == priMemFreezeFrameBuffer[i].kind)) {
            freezeFrame = &priMemFreezeFrameBuffer[i];
            break;
        }
    }
#endif
    /*if FF class found,find the corresponding PID*/
    if(NULL != freezeFrame) {
        offset = 0u;
        if(freezeFrameClass->FFKind == DEM_FREEZE_FRAME_OBD) {
            if(freezeFrameClass->FFIdClassRef != NULL){
                for(uint16 i = 0u; (i < DEM_MAX_NR_OF_PIDS_IN_FREEZEFRAME_DATA) && ((freezeFrameClass->FFIdClassRef[i]->Arc_EOL) == FALSE); i++) {
                    offset += DEM_PID_IDENTIFIER_SIZE_OF_BYTES;
                    if(freezeFrameClass->FFIdClassRef[i]->PidIdentifier == PID){
                        pidDataSize = freezeFrameClass->FFIdClassRef[i]->PidOrDidSize;
                        pidFound = TRUE;
                        break;
                    } else{
                        offset += (uint16)freezeFrameClass->FFIdClassRef[i]->PidOrDidSize;
                    }
                }
            }
        }
    }

    if( (TRUE == pidFound) && (NULL != freezeFrame) && (offset >= DEM_PID_IDENTIFIER_SIZE_OF_BYTES) ) {
        if(((*BufSize) >= pidDataSize) && (PID == (freezeFrame->data[offset - DEM_PID_IDENTIFIER_SIZE_OF_BYTES]))
            && ((offset + (uint16)pidDataSize) <= (uint16)(freezeFrame->dataSize))
			&& ((offset + (uint16)pidDataSize) <= DEM_MAX_SIZE_FF_DATA)) {
            memcpy(DestBuffer, &freezeFrame->data[offset], (size_t)pidDataSize);
            *BufSize = pidDataSize;
            returnCode = E_OK;
        }
    }
#endif
    SchM_Exit_Dem_EA_0();
#else
    (void)PID;
#endif

    return returnCode;
}

/*
 * Procedure:   storeOBDFreezeFrameDataMem
 * Description: store OBD FreezeFrame data record in primary memory
 */
#ifdef DEM_USE_MEMORY_FUNCTIONS
#if ( DEM_FF_DATA_IN_PRE_INIT || DEM_FF_DATA_IN_PRI_MEM )
static boolean storeOBDFreezeFrameDataMem(const Dem_EventParameterType *eventParam, const FreezeFrameRecType *freezeFrame,
                                       FreezeFrameRecType* freezeFrameBuffer, uint32 freezeFrameBufferSize,
                                       Dem_DTCOriginType origin)
{
    boolean eventIdFound = FALSE;
    boolean eventIdFreePositionFound = FALSE;
    uint32 i;
    boolean dataStored = FALSE;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE2) && (DEM_MAX_NUM_OBD_FFS > 1)
    const Dem_EventParameterType *eventParameter;
#endif
    (void)origin;

    /* Check if already stored */
    for (i = 0uL; (i < freezeFrameBufferSize) && (FALSE == eventIdFound); i++){
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE2) && (DEM_MAX_NUM_OBD_FFS > 1)
        /* Need to be able to store more than one OBD FF. Allow storage if not already stored for this
         * event or for some other DTC. */
        if( (freezeFrameBuffer[i].eventId != DEM_EVENT_ID_NULL) && (freezeFrameBuffer[i].kind == DEM_FREEZE_FRAME_OBD) ) {
            /* Found an OBD freeze frame. Check event ID. */
            if( freezeFrameBuffer[i].eventId != eventParam->EventID ) {
                eventParameter = NULL_PTR;
                lookupEventIdParameter(freezeFrameBuffer[i].eventId, &eventParameter);
                if( NULL_PTR != eventParameter ) {
                    if( (NULL_PTR != eventParameter->DTCClassRef) && (NULL_PTR != eventParam->DTCClassRef) && (eventParameter->DTCClassRef != eventParam->DTCClassRef) ) {
                        /* Freeze frame is for different DTC. */
                        eventIdFound = TRUE;
                    }
                }
            }
            else {
                /* Same ID. */
                eventIdFound = TRUE;
            }
        }
#else
        eventIdFound = ((freezeFrameBuffer[i].eventId != DEM_EVENT_ID_NULL)
            && (freezeFrameBuffer[i].kind == DEM_FREEZE_FRAME_OBD))? TRUE: FALSE;
#endif /* DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE2 */
    }

    if ( FALSE == eventIdFound ) {
        /* find the first free position */
        for (i = 0uL; (i < freezeFrameBufferSize) && (FALSE == eventIdFreePositionFound); i++){
            eventIdFreePositionFound =  (freezeFrameBuffer[i].eventId == DEM_EVENT_ID_NULL)? TRUE: FALSE;
        }
        /* if found,copy it to this position */
        if ( TRUE == eventIdFreePositionFound ) {
            memcpy(&freezeFrameBuffer[i-1], freezeFrame, sizeof(FreezeFrameRecType));
        } else {
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON)
            /* if not found,do displacement */
            FreezeFrameRecType *freezeFrameLocal = NULL;
            if( TRUE == lookupFreezeFrameForDisplacement(eventParam, &freezeFrameLocal, freezeFrameBuffer, freezeFrameBufferSize) ) {
                if(freezeFrameLocal != NULL){
                    memcpy(freezeFrameLocal, freezeFrame, sizeof(FreezeFrameRecType));
                    dataStored = TRUE;
                } else {
                    setOverflowIndication(eventParam->EventClass->EventDestination, TRUE);
                }
            }
#else
            setOverflowIndication(eventParam->EventClass->EventDestination, TRUE);
            DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_STORE_FF_DATA_MEM_ID, DEM_E_MEM_FF_DATA_BUFF_FULL);
#endif /* DEM_EVENT_DISPLACEMENT_SUPPORT */
        }
    } else {
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE2) && (DEM_MAX_NUM_OBD_FFS > 1)
        /* OBD freeze frame was already stored. Check if we should replace it.
         * We replace it if the new event has higher priority than all the ones
         * previously stored. */
        /* Check all OBD FFs stored */
        boolean replaceOBDFF = TRUE;
        const Dem_EventParameterType *storedEventParam;
        for (uint32 idx = 0uL; (idx < freezeFrameBufferSize) && (TRUE == replaceOBDFF); idx++){
            if( (freezeFrameBuffer[idx].eventId != DEM_EVENT_ID_NULL) && (freezeFrameBuffer[idx].kind == DEM_FREEZE_FRAME_OBD) ) {
                storedEventParam = NULL_PTR;
                lookupEventIdParameter(freezeFrameBuffer[idx].eventId, &storedEventParam);
                if( NULL_PTR != storedEventParam ) {
                    if( storedEventParam->EventClass->EventPriority <= eventParam->EventClass->EventPriority ) {
                        /* Priority of stored event is higher. We should not replace the FF. */
                        replaceOBDFF = FALSE;
                    }
                }
            }
        }
        if( TRUE == replaceOBDFF ) {
            /* We should replace the currently stored OBF FF. There could be more than one stored so
             * we need to loop through the whole buffer again. */
            for (uint32 idx = 0UL; idx < freezeFrameBufferSize; idx++){
                if( (freezeFrameBuffer[idx].eventId != DEM_EVENT_ID_NULL) && (freezeFrameBuffer[idx].kind == DEM_FREEZE_FRAME_OBD) ) {
#if 0
                    memset(&freezeFrameBuffer[idx], 0, sizeof(FreezeFrameRecType));
#else
                    freezeFrameBuffer[idx].eventId = DEM_EVENT_ID_NULL;
#endif
                }
            }
            memcpy(&freezeFrameBuffer[i-1], freezeFrame, sizeof(FreezeFrameRecType));
            dataStored = TRUE;
        }

#else
        /* OBD freeze frame was already stored. Check if we should replace it.
         * We replace it if the new event has higher priority. */
        const Dem_EventParameterType *storedEventParam = NULL;
        boolean replaceOBDFF = TRUE;
        uint8 priorityStored = 0xFFu;/* Lowest prio. */
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
        if( IS_COMBINED_EVENT_ID(freezeFrameBuffer[i-1].eventId) ) {
            const Dem_CombinedDTCCfgType *CombDTCCfg = &configSet->CombinedDTCConfig[TO_COMBINED_EVENT_CFG_IDX(freezeFrameBuffer[i-1].eventId)];
            priorityStored = CombDTCCfg->Priority;
        }
        else {
            lookupEventIdParameter(freezeFrameBuffer[i-1].eventId, &storedEventParam);
            if( NULL != storedEventParam ) {
                priorityStored = storedEventParam->EventClass->EventPriority;
            }
        }
#else
        lookupEventIdParameter(freezeFrameBuffer[i-1].eventId, &storedEventParam);
        if( NULL != storedEventParam ) {
            priorityStored = storedEventParam->EventClass->EventPriority;
        }
#endif
        if( priorityStored <= eventParam->EventClass->EventPriority  ) {
            /* Priority of the new event is lower or equal to the stored event.
             * Should NOT replace the FF. */
            replaceOBDFF = FALSE;
        }
        if( TRUE == replaceOBDFF ) {
            memcpy(&freezeFrameBuffer[i-1], freezeFrame, sizeof(FreezeFrameRecType));
            dataStored = TRUE;
        }
#endif/* DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE2 */
    }
    return dataStored;
}
#endif /* DEM_FF_DATA_IN_PRE_INIT || DEM_FF_DATA_IN_PRI_MEM  */
#endif /* DEM_USE_MEMORY_FUNCTIONS */

#if (DEM_OBD_SUPPORT == STD_ON)
/**
 * Service for reporting the event as disabled to the Dem for the PID $41 computation.
 *
 * @param EventId: identification of an event by assigned EventId.
 * @return E_OK set of event to disabled was successful.
 */
Std_ReturnType Dem_SetEventDisabled(Dem_EventIdType EventId )
{
	/* @req Dem312 */
	/* @req Dem348 */
	/* @req Dem294 */
	EventStatusRecType *eventStatusRec = NULL;

	SchM_Enter_Dem_EA_0();
	lookupEventStatusRec(EventId, &eventStatusRec);
	eventStatusRec->isDisabled = 1;
	SchM_Exit_Dem_EA_0();

    return E_OK;
}

/**
 * Gets the number of confirmed OBD DTCs.
 *
 * @return the number of confirmed OBD DTCs, between 0 and 127.
 */
static uint8_t getNumberOfConfirmedObdDTCs(void) {
	/* @req DEM351 */
	uint8_t confirmedBitMask = DEM_CONFIRMED_DTC;
	uint16_t confirmedDTCs = 0;
	if (DEM_FILTER_ACCEPTED == Dem_SetDTCFilter(confirmedBitMask, DEM_DTC_KIND_EMISSION_REL_DTCS, DEM_DTC_FORMAT_OBD, DEM_DTC_ORIGIN_PRIMARY_MEMORY, DEM_FILTER_WITH_SEVERITY_NO, DEM_SEVERITY_NO_SEVERITY, DEM_FILTER_FOR_FDC_NO)) {
		while (Dem_GetNumberOfFilteredDtc(&confirmedDTCs) == DEM_NUMBER_PENDING) {
			// wait until it either succeeded or failed
		}

		// Can store at most a non-negative number of 6 bits (127)
		if (confirmedDTCs > 127) {
			confirmedDTCs = 127;
		}
	}

	return (uint8_t) confirmedDTCs;
}

/**
 * Gets the MIL status.
 *
 * @param numberOfConfirmedDtcs: The number of confirmed OBD DTCs.
 * @return TRUE if MIL is ON, otherwise FALSE.
 */
static uint8_t isObdMilOn(uint8_t numberOfConfirmedDtcs) {
	/* @req DEM352 */
	uint8 milStatus = 0;
	// According to req. DEM544, SAE J1979 and CARB OBD legislation,
	// this status should reflect if there is any confirmed DTC,
	// NOT if the MIL bulb is lit up, as it can be ON for different reasons as well (see SAE J1979).
	if (0u != numberOfConfirmedDtcs) {
		milStatus = 1;
	}

	return milStatus;
}

/**
 * Checks if the the event (test) has been marked as completed since last clear.
 * If the event (test) failed, it will be reported as incomplete.
 *
 * @param eventIndex: the event's current location in the event status buffer.
 * @return TRUE if test is complete, otherwise FALSE.
 */
static boolean isMonitoringNotCompleteSinceLastClear(uint16 eventIndex) {
	boolean testFailedSinceLastClear = (0 != (eventStatusBuffer[eventIndex].eventStatusExtended & DEM_TEST_FAILED_SINCE_LAST_CLEAR))? TRUE: FALSE;
	boolean monitoringNotComplete = (0 != (eventStatusBuffer[eventIndex].eventStatusExtended & DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR))? TRUE: FALSE;
	monitoringNotComplete |= testFailedSinceLastClear;
	return monitoringNotComplete;
}

#if (DEM_OBD_ENGINE_TYPE == DEM_IGNITION_SPARK)
/**
 * Service to report the value of PID $01 computed by the Dem.
 * Reentrant: Yes
 *
 * FOR SPARK ENGINE CONFIGURATION
 *
 * @param PID01value: Buffer containing the contents of PID $01 computed by the Dem.
 * @return Always E_OK is returned, as E_NOT_OK will never appear..
 */
Std_ReturnType Dem_DcmReadDataOfPID01(uint8* PID01value) {
	// Reset PID value
	PID01value[0] = 0; // Byte A
	PID01value[1] = 0; // Byte B
	PID01value[2] = 0; // Byte C
	PID01value[3] = 0; // Byte D

	/**
	 * Get number of confirmed OBD DTCs (Byte A)
	 */
	uint8_t confirmedDTCs = getNumberOfConfirmedObdDTCs();

	/**
	 * Get MIL status (Byte A)
	 */
	uint8_t milStatus = isObdMilOn(confirmedDTCs);

	/**
	 * Get engine systems' monitors availability (Byte B and C)
	 *
	 * Get engine systems' monitors readiness (Byte B and D)
	 */
	boolean ENG_TYPE  	= 0; // Compression ignition monitoring supported

	boolean MIS_SUP		= 0; // Misfire monitoring supported (All)
	boolean FUEL_SUP	= 0; // Fuel system monitoring supported (All)
	boolean CCM_SUP		= 0; // Comprehensive component monitoring supported (All)
	boolean CAT_SUP		= 0; // Catalyst monitoring supported (Gasoline)
	boolean HCAT_SUP	= 0; // Heated catalyst monitoring supported (Gasoline)
	boolean EVAP_SUP	= 0; // Evaporative system monitoring supported (Gasoline)
	boolean AIR_SUP		= 0; // Secondary air system monitoring supported (Gasoline)
	boolean ACRF_SUP	= 0; // A/C system refrigerant monitoring supported (Gasoline)
	boolean O2S_SUP		= 0; // Oxygen sensor monitoring supported (Gasoline)
	boolean HTR_SUP		= 0; // Oxygen sensor heater monitoring supported (Gasoline)
	boolean EGR_SUP		= 0; // EGR system monitoring supported (All)

	boolean MIS_RDY		= 0; // Misfire monitoring ready (All)
	boolean FUEL_RDY	= 0; // Fuel system monitoring ready (All)
	boolean CCM_RDY		= 0; // Comprehensive component monitoring ready (All)
	boolean CAT_RDY		= 0; // Catalyst monitoring ready (Gasoline)
	boolean HCAT_RDY	= 0; // Heated catalyst monitoring ready (Gasoline)
	boolean EVAP_RDY	= 0; // Evaporative system monitoring ready (Gasoline)
	boolean AIR_RDY		= 0; // Secondary air system monitoring ready (Gasoline)
	boolean ACRF_RDY	= 0; // A/C system refrigerant monitoring ready (Gasoline)
	boolean O2S_RDY		= 0; // Oxygen sensor monitoring ready (Gasoline)
	boolean HTR_RDY		= 0; // Oxygen sensor heater monitoring ready (Gasoline)
	boolean EGR_RDY		= 0; // EGR system monitoring ready (All)

	// Quoted from ASR 4.3:
	// According to SAEJ1979, the group AirCondition Component (ACRF) shall not be supported anymore.
	// However, it is still included in ISO 15031-5.

	/* @req DEM354 */
	for (uint16 i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
		if (DEM_EVENT_ID_NULL != eventStatusBuffer[i].eventId) {
			Dem_EventOBDReadinessGroup readinessGroup = eventStatusBuffer[i].eventParamRef->EventClass->OBDReadinessGroup;
			boolean monitoringNotComplete = isMonitoringNotCompleteSinceLastClear(i);

			switch(readinessGroup) {
				case DEM_OBD_RDY_MISF:
					MIS_SUP   = 1;
					MIS_RDY	  = 0; // Always complete
					break;
				case DEM_OBD_RDY_FLSYS:
					FUEL_SUP  = 1;
					FUEL_RDY  = 0; // Always complete
					break;
				case DEM_OBD_RDY_CMPRCMPT:
					CCM_SUP   = 1;
					CCM_RDY	  = 0; // Always complete
					break;
				case DEM_OBD_RDY_CAT:
					CAT_SUP   = 1;
					CAT_RDY  |= monitoringNotComplete;
					break;
				case DEM_OBD_RDY_HTCAT:
					HCAT_SUP  = 1;
					HCAT_RDY |= monitoringNotComplete;
					break;
				case DEM_OBD_RDY_EVAP:
					EVAP_SUP  = 1;
					EVAP_RDY |= monitoringNotComplete;
					break;
				case DEM_OBD_RDY_SECAIR:
					AIR_SUP   = 1;
					AIR_RDY  |= monitoringNotComplete;
					break;
				case DEM_OBD_RDY_AC:
					ACRF_SUP  = 1;
					ACRF_RDY |= monitoringNotComplete;
					break;
				case DEM_OBD_RDY_O2SENS:
					O2S_SUP   = 1;
					O2S_RDY  |= monitoringNotComplete;
					break;
				case DEM_OBD_RDY_O2SENSHT:
					HTR_SUP   = 1;
					HTR_RDY  |= monitoringNotComplete;
					break;
				case DEM_OBD_RDY_ERG:
					EGR_SUP   = 1;
					EGR_RDY  |= monitoringNotComplete;
					break;
			}
		}
	}

	/**
	 * Pack availability and readiness values
	 */
	// Byte A
	PID01value[0] = confirmedDTCs  | (milStatus << 6);
	// Byte B
	PID01value[1] = (uint8)((MIS_SUP << 0) | (FUEL_SUP << 1) | (CCM_SUP << 2)  | (ENG_TYPE << 3) | (MIS_RDY << 4)  | (FUEL_RDY << 5) | (CCM_RDY << 6)); // Bit 7 reserved
	// Byte C
	PID01value[2] =  (uint8)((CAT_SUP << 0) | (HCAT_SUP << 1) | (EVAP_SUP << 2) | (AIR_SUP << 3)  | (ACRF_SUP << 4) | (O2S_SUP << 5)  | (HTR_SUP << 6) | (EGR_SUP << 7));
	// Byte D
	PID01value[3] =  (uint8)((CAT_RDY << 0) | (HCAT_RDY << 1) | (EVAP_RDY << 2) | (AIR_RDY << 3)  | (ACRF_RDY << 4) | (O2S_RDY << 5)  | (HTR_RDY << 6) | (EGR_RDY << 7));

	return E_OK;
}
#elif (DEM_OBD_ENGINE_TYPE == DEM_IGNITION_COMPR)
/**
 * Service to report the value of PID $01 computed by the Dem.
 * Reentrant: Yes
 *
 * FOR COMPRESSION ENGINE CONFIGURATION
 *
 * @param PID01value: buffer containing the contents of PID $01 computed by the Dem.
 * @return Always E_OK is returned, as E_NOT_OK will never appear.
 */
Std_ReturnType Dem_DcmReadDataOfPID01(uint8* PID01value) {
	// Reset PID value
	PID01value[0] = 0; // Byte A
	PID01value[1] = 0; // Byte B
	PID01value[2] = 0; // Byte C
	PID01value[3] = 0; // Byte D

	/**
	 * Get number of confirmed OBD DTCs (Byte A)
	 */
	uint8_t confirmedDTCs = getNumberOfConfirmedObdDTCs();

	/**
	 * Get MIL status (Byte A)
	 */
	uint8_t milStatus = isObdMilOn(confirmedDTCs);

	/**
	 * Get engine systems' monitors availability (Byte B and C)
	 *
	 * Get engine systems' monitors readiness (Byte B and D)
	 */
	boolean ENG_TYPE  	= 0; // Compression ignition monitoring supported

	boolean MIS_SUP		= 0; // Misfire monitoring supported (All)
	boolean FUEL_SUP	= 0; // Fuel system monitoring supported (All)
	boolean CCM_SUP		= 0; // Comprehensive component monitoring supported (All)
	boolean HCCATSUP  	= 0; // NMHC catalyst monitoring supported (Diesel)
	boolean NCAT_SUP	= 0; // NOx aftertreatment monitoring supported (Diesel)
	boolean BP_SUP    	= 0; // Boost pressure system monitoring supported (Diesel)
	boolean EGS_SUP		= 0; // Exhaust gas sensor monitoring supported (Diesel)
	boolean PM_SUP		= 0; // PM Filter monitoring supported (Diesel)
	boolean EGR_SUP		= 0; // EGR system monitoring supported (All)

	boolean MIS_RDY		= 0; // Misfire monitoring ready (All)
	boolean FUEL_RDY	= 0; // Fuel system monitoring ready (All)
	boolean CCM_RDY		= 0; // Comprehensive component monitoring ready (All)
	boolean HCCATRDY  	= 0; // NMHC catalyst monitoring ready (Diesel)
	boolean NCAT_RDY	= 0; // NOx aftertreatment monitoring ready (Diesel)
	boolean BP_RDY    	= 0; // Boost pressure system monitoring ready (Diesel)
	boolean EGS_RDY		= 0; // Exhaust gas sensor monitoring ready (Diesel)
	boolean PM_RDY		= 0; // PM Filter monitoring ready (Diesel)
	boolean EGR_RDY		= 0; // EGR system monitoring ready (All)

	ENG_TYPE = 1;

	/* @req DEM354 */
	for (uint16 i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
		if (DEM_EVENT_ID_NULL != eventStatusBuffer[i].eventId) {
			Dem_EventOBDReadinessGroup readinessGroup = eventStatusBuffer[i].eventParamRef->EventClass->OBDReadinessGroup;
			boolean monitoringNotComplete = isMonitoringNotCompleteSinceLastClear(i);

			switch(readinessGroup) {
				case DEM_OBD_RDY_MISF:
					MIS_SUP   = 1;
					MIS_RDY	 |= monitoringNotComplete;
					break;
				case DEM_OBD_RDY_FLSYS:
					FUEL_SUP  = 1;
					FUEL_RDY |= monitoringNotComplete;
					break;
				case DEM_OBD_RDY_CMPRCMPT:
					CCM_SUP   = 1;
					CCM_RDY	  = 0;
					break;
				case DEM_OBD_RDY_HCCAT:
					HCCATSUP  = 1;
					HCCATRDY |= monitoringNotComplete;
					break;
				case DEM_OBD_RDY_NOXCAT:
					NCAT_SUP  = 1;
					NCAT_RDY |= monitoringNotComplete;
					break;
				case DEM_OBD_RDY_BOOSTPR:
					BP_SUP    = 1;
					BP_RDY   |= monitoringNotComplete;
					break;
				case DEM_OBD_RDY_EGSENS:
					EGS_SUP   = 1;
					EGS_RDY  |= monitoringNotComplete;
					break;
				case DEM_OBD_RDY_PMFLT:
					PM_SUP    = 1;
					PM_RDY   |= monitoringNotComplete;
					break;
				case DEM_OBD_RDY_ERG:
					EGR_SUP   = 1;
					EGR_RDY  |= monitoringNotComplete;
					break;
			}
		}
	}

	/**
	 * Pack availability and readiness values
	 */
	// Byte A
	PID01value[0] = ((uint8) confirmedDTCs) | (milStatus << 6);
	// Byte B
	PID01value[1] = (MIS_SUP << 0)  | (FUEL_SUP << 1) | (CCM_SUP << 2) | (ENG_TYPE << 3) | (MIS_RDY << 4) | (FUEL_RDY << 5) | (CCM_RDY << 6); // Bit 7 reserved
	// Byte C
	PID01value[2] = (HCCATSUP << 0) | (NCAT_SUP << 1) | (BP_SUP << 3)  | (EGS_SUP << 5)  | (PM_SUP << 6)  | (EGR_SUP << 7); // bit 2 reserved // bit 4 reserved
	// Byte D
	PID01value[3] = (HCCATRDY << 0) | (NCAT_RDY << 1) | (BP_RDY << 3)  | (EGS_RDY << 5)  | (PM_RDY << 6)  | (EGR_RDY << 7); // bit 2 reserved // bit 4 reserved

	return E_OK;
}
#endif

/**
 * Checks whether the event (test) has been marked as completed in current driving cycle.
 * If the event (test) failed, it will be reported as incomplete.
 *
 * @param eventIndex: the event's current location in the event status buffer.
 * @return TRUE if test is complete, otherwise FALSE.
 */
static DEM_TRISTATE isMonitoringNotCompleteThisDrivingCycle(uint16 eventIndex) {
	DEM_TRISTATE testFailedThisOperationCycle = (0 != (eventStatusBuffer[eventIndex].eventStatusExtended & DEM_TEST_FAILED_THIS_OPERATION_CYCLE))? 1: 0;
	DEM_TRISTATE monitoringNotComplete = (0 != (eventStatusBuffer[eventIndex].eventStatusExtended & DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE))? 1: 0;
	monitoringNotComplete |= testFailedThisOperationCycle;

	DEM_TRISTATE isEnabledThisDrivingCycle = (FALSE == eventStatusBuffer[eventIndex].isDisabled)? 1: 0;
	// If the monitor for the component is disabled for this driving cycle then
	// set all the relevant tests to complete
	if (isEnabledThisDrivingCycle == DEM_T_FALSE) {
		monitoringNotComplete = DEM_T_FALSE;
	}

	return monitoringNotComplete;
}

/**
 * Sets all NULL values to FALSE.
 *
 * @param total: number of values to check.
 * @param monitorValues: monitor values to reset or keep.
 */
static void resetOrKeepMonitorValues(uint8_t total, DEM_TRISTATE **monitorValues) {
	for (uint8_t i = 0; i < total; i++)  {
		*monitorValues[i] = (*monitorValues[i] == DEM_T_NULL) ? DEM_T_FALSE : *monitorValues[i];
	}
}

#if (DEM_OBD_ENGINE_TYPE == DEM_IGNITION_SPARK)
/**
 * Service to report the value of PID $41 computed by the Dem.
 * Reentrant: Yes
 *
 * FOR SPARK ENGINE CONFIGURATION
 *
 * @param PID41value: buffer containing the contents of PID $41 computed by the Dem.
 * @return Always E_OK is returned, as E_NOT_OK will never appear.
 */
Std_ReturnType Dem_DcmReadDataOfPID41(uint8* PID41value) {
	// Reset PID value
	PID41value[0] = 0; // Byte A
	PID41value[1] = 0; // Byte B
	PID41value[2] = 0; // Byte C
	PID41value[3] = 0; // Byte D

	/**
	 * Get number of confirmed OBD DTCs (Byte A)
	 */
	// Does not report the number of confirmed DTCs, keep at 0

	/**
	 * Get MIL status (Byte A)
	 */
	// Does not report the MIL status, keep at 0

	/**
	 * Get engine systems' monitors status (Byte B and C)
	 *
	 * Get engine systems' monitors completion (Byte B and D)
	 */
	DEM_TRISTATE  ENG_TYPE  = DEM_T_FALSE; // Compression ignition monitoring supported

	DEM_TRISTATE  MIS_ENA	= DEM_T_NULL; // Misfire monitoring enabled (All)
	DEM_TRISTATE  FUEL_ENA	= DEM_T_NULL; // Fuel system monitoring enabled (All)
	DEM_TRISTATE  CCM_ENA	= DEM_T_NULL; // Comprehensive component monitoring enabled (All)
	DEM_TRISTATE  CAT_ENA	= DEM_T_NULL; // Catalyst monitoring enabled (Gasoline)
	DEM_TRISTATE  HCAT_ENA	= DEM_T_NULL; // Heated catalyst monitoring enabled (Gasoline)
	DEM_TRISTATE  EVAP_ENA	= DEM_T_NULL; // Evaporative system monitoring enabled (Gasoline)
	DEM_TRISTATE  AIR_ENA	= DEM_T_NULL; // Secondary air system monitoring enabled (Gasoline)
	DEM_TRISTATE  ACRF_ENA	= DEM_T_NULL; // A/C system refrigerant monitoring enabled (Gasoline)
	DEM_TRISTATE  O2S_ENA	= DEM_T_NULL; // Oxygen sensor monitoring enabled (Gasoline)
	DEM_TRISTATE  HTR_ENA	= DEM_T_NULL; // Oxygen sensor heater monitoring enabled (Gasoline)
	DEM_TRISTATE  EGR_ENA	= DEM_T_NULL; // EGR system monitoring enabled (All)

	DEM_TRISTATE  MIS_CMPL	= DEM_T_NULL; // Misfire monitoring complete (All)
	DEM_TRISTATE  FUELCMPL	= DEM_T_NULL; // Fuel system monitoring complete (All)
	DEM_TRISTATE  CCM_CMPL	= DEM_T_NULL; // Comprehensive component monitoring complete (All)
	DEM_TRISTATE  CAT_CMPL	= DEM_T_NULL; // Catalyst monitoring complete (Gasoline)
	DEM_TRISTATE  HCATCMPL	= DEM_T_NULL; // Heated catalyst monitoring complete (Gasoline)
	DEM_TRISTATE  EVAPCMPL	= DEM_T_NULL; // Evaporative system monitoring complete (Gasoline)
	DEM_TRISTATE  AIR_CMPL	= DEM_T_NULL; // Secondary air system monitoring complete (Gasoline)
	DEM_TRISTATE  ACRFCMPL	= DEM_T_NULL; // A/C system refrigerant monitoring complete (Gasoline)
	DEM_TRISTATE  O2S_CMPL	= DEM_T_NULL; // Oxygen sensor monitoring complete (Gasoline)
	DEM_TRISTATE  HTR_CMPL	= DEM_T_NULL; // Oxygen sensor heater monitoring complete (Gasoline)
	DEM_TRISTATE  EGR_CMPL	= DEM_T_NULL; // EGR system monitoring complete (All)

	// Quoted from ASR 4.3:
	// According to SAEJ1979, the group AirCondition Component shall not be supported anymore.
	// However, it is still included in ISO 15031-5.

	/* @req DEM355 */
	for (uint16 i = 0u; i < DEM_MAX_NUMBER_EVENT; i++){
		if (DEM_EVENT_ID_NULL != eventStatusBuffer[i].eventId) {
			Dem_EventOBDReadinessGroup readinessGroup = eventStatusBuffer[i].eventParamRef->EventClass->OBDReadinessGroup;

			DEM_TRISTATE monitoringNotComplete = isMonitoringNotCompleteThisDrivingCycle(i);
			DEM_TRISTATE isEnabledThisDrivingCycle = (FALSE == eventStatusBuffer[i].isDisabled)? 1 : 0;

			switch(readinessGroup) {
				case DEM_OBD_RDY_MISF:
					MIS_ENA  = (MIS_ENA   == DEM_T_NULL) ? isEnabledThisDrivingCycle  : (MIS_ENA & isEnabledThisDrivingCycle);
					MIS_CMPL = DEM_T_FALSE; // Always on

					break;
				case DEM_OBD_RDY_FLSYS:
					FUEL_ENA  = (FUEL_ENA == DEM_T_NULL) ? isEnabledThisDrivingCycle  : (FUEL_ENA  & isEnabledThisDrivingCycle);
					FUELCMPL = DEM_T_FALSE; // Always on

					break;
				case DEM_OBD_RDY_CMPRCMPT:
					CCM_ENA  = (CCM_ENA   == DEM_T_NULL) ? isEnabledThisDrivingCycle  : (CCM_ENA  & isEnabledThisDrivingCycle);
					CCM_CMPL = DEM_T_FALSE; // Always on

					break;
				case DEM_OBD_RDY_CAT:
					CAT_ENA  = (CAT_ENA   == DEM_T_NULL) ? isEnabledThisDrivingCycle  : (CAT_ENA  & isEnabledThisDrivingCycle);
					CAT_CMPL = (CAT_CMPL  == DEM_T_NULL) ? monitoringNotComplete      : (CAT_CMPL & monitoringNotComplete);

					break;
				case DEM_OBD_RDY_HTCAT:
					HCAT_ENA = (HCAT_ENA  == DEM_T_NULL) ? isEnabledThisDrivingCycle  : (HCAT_ENA & isEnabledThisDrivingCycle);
					HCATCMPL = (HCATCMPL  == DEM_T_NULL) ? monitoringNotComplete      : (HCATCMPL & monitoringNotComplete);

					break;
				case DEM_OBD_RDY_EVAP:
					EVAP_ENA = (EVAP_ENA  == DEM_T_NULL) ? isEnabledThisDrivingCycle  : (EVAP_ENA & isEnabledThisDrivingCycle);
					EVAPCMPL = (EVAPCMPL  == DEM_T_NULL) ? monitoringNotComplete      : (EVAPCMPL & monitoringNotComplete);

					break;
				case DEM_OBD_RDY_SECAIR:
					AIR_ENA  = (AIR_ENA   == DEM_T_NULL) ? isEnabledThisDrivingCycle  : (AIR_ENA  & isEnabledThisDrivingCycle);
					AIR_CMPL = (AIR_CMPL  == DEM_T_NULL) ? monitoringNotComplete	  : (AIR_CMPL & monitoringNotComplete);

					break;
				case DEM_OBD_RDY_AC:
					ACRF_ENA = (ACRF_ENA  == DEM_T_NULL) ? isEnabledThisDrivingCycle  : (ACRF_ENA & isEnabledThisDrivingCycle);
					ACRFCMPL = (ACRFCMPL  == DEM_T_NULL) ? monitoringNotComplete 	  : (ACRFCMPL & monitoringNotComplete);

					break;
				case DEM_OBD_RDY_O2SENS:
					O2S_ENA  = (O2S_ENA   == DEM_T_NULL) ? isEnabledThisDrivingCycle  : (O2S_ENA  & isEnabledThisDrivingCycle);
					O2S_CMPL = (O2S_CMPL  == DEM_T_NULL) ? monitoringNotComplete	  : (O2S_CMPL & monitoringNotComplete);

					break;
				case DEM_OBD_RDY_O2SENSHT:
					HTR_ENA  = (HTR_ENA   == DEM_T_NULL) ? isEnabledThisDrivingCycle  : (HTR_ENA  & isEnabledThisDrivingCycle);
					HTR_CMPL = (HTR_CMPL  == DEM_T_NULL) ? monitoringNotComplete      : (HTR_CMPL & monitoringNotComplete);

					break;
				case DEM_OBD_RDY_ERG:
					EGR_ENA  = (EGR_ENA   == DEM_T_NULL) ? isEnabledThisDrivingCycle  : (EGR_ENA  & isEnabledThisDrivingCycle);
					EGR_CMPL = (EGR_CMPL  == DEM_T_NULL) ? monitoringNotComplete      : (EGR_CMPL & monitoringNotComplete);

					break;
			}
		}
	}

	/**
	 * Set remaining NULL systems to FALSE
	 */
	DEM_TRISTATE *monitorsEnabled[]  = {&MIS_ENA, &FUEL_ENA, &CCM_ENA, &CAT_ENA, &HCAT_ENA, &EVAP_ENA, &AIR_ENA, &ACRF_ENA, &O2S_ENA, &HTR_ENA, &EGR_ENA};
	resetOrKeepMonitorValues(11, monitorsEnabled);

	DEM_TRISTATE *monitorsComplete[] = {&MIS_CMPL, &FUELCMPL, &CCM_CMPL, &CAT_CMPL, &HCATCMPL, &EVAPCMPL, &AIR_CMPL, &ACRFCMPL, &O2S_CMPL, &HTR_CMPL, &EGR_CMPL};
	resetOrKeepMonitorValues(11, monitorsComplete);

	/**
	 * Pack status and completion values
	 */
	// Byte A
	// Both 0, does not report
	// Byte B
	PID41value[1] = (uint8)((MIS_ENA << 0)  | (FUEL_ENA << 1) | (CCM_ENA << 2)  | (ENG_TYPE << 3) | (MIS_CMPL << 4) | (FUELCMPL << 5) | (CCM_CMPL << 6)); // Bit 7 reserved
	// Byte C
	PID41value[2] = (uint8)((CAT_ENA << 0)  | (HCAT_ENA << 1) | (EVAP_ENA << 2) | (AIR_ENA << 3)  | (ACRF_ENA << 4) | (O2S_ENA << 5)  | (HTR_ENA << 6) | (EGR_ENA << 7));
	// Byte D
	PID41value[3] = (uint8)((CAT_CMPL << 0) | (HCATCMPL << 1) | (EVAPCMPL << 2) | (AIR_CMPL << 3) | (ACRFCMPL << 4) | (O2S_CMPL << 5) | (HTR_CMPL << 6) | (EGR_CMPL << 7));

	return E_OK;
}
#elif (DEM_OBD_ENGINE_TYPE == DEM_IGNITION_COMPR)
/**
 * Service to report the value of PID $41 computed by the Dem.
 * Reentrant: Yes
 *
 * FOR COMPRESSION ENGINE CONFIGURATION
 *
 * @param PID41value: buffer containing the contents of PID $41 computed by the Dem.
 * @return Always E_OK is returned, as E_NOT_OK will never appear.
 */
Std_ReturnType Dem_DcmReadDataOfPID41(uint8* PID41value) {
	// Reset PID value
	PID41value[0] = 0; // Byte A
	PID41value[1] = 0; // Byte B
	PID41value[2] = 0; // Byte C
	PID41value[3] = 0; // Byte D

	/**
	 * Get number of confirmed OBD DTCs (Byte A)
	 */
	// Does not report the number of confirmed DTCs, keep at 0

	/**
	 * Get MIL status (Byte A)
	 */
	// Does not report the MIL status, keep at 0

	/**
	 * Get engine systems' monitors status (Byte B and C)
	 *
	 * Get engine systems' monitors completion (Byte B and D)
	 */
	DEM_TRISTATE ENG_TYPE  	= DEM_T_FALSE; // Compression ignition monitoring supported

	DEM_TRISTATE MIS_ENA	= DEM_T_NULL; // Misfire monitoring enabled (All)
	DEM_TRISTATE FUEL_ENA	= DEM_T_NULL; // Fuel system monitoring enabled (All)
	DEM_TRISTATE CCM_ENA	= DEM_T_NULL; // Comprehensive component monitoring enabled (All)
	DEM_TRISTATE HCCATENA 	= DEM_T_NULL; // NMHC catalyst monitoring enabled (Diesel)
	DEM_TRISTATE NCAT_ENA	= DEM_T_NULL; // NOx aftertreatment monitoring enabled (Diesel)
	DEM_TRISTATE BP_ENA    	= DEM_T_NULL; // Boost pressure system monitoring enabled (Diesel)
	DEM_TRISTATE EGS_ENA	= DEM_T_NULL; // Exhaust gas sensor monitoring enabled (Diesel)
	DEM_TRISTATE PM_ENA		= DEM_T_NULL; // PM Filter monitoring enabled (Diesel)
	DEM_TRISTATE EGR_ENA	= DEM_T_NULL; // EGR system monitoring enabled (All)

	DEM_TRISTATE MIS_CMPL	= DEM_T_NULL; // Misfire monitoring complete (All)
	DEM_TRISTATE FUELCMPL	= DEM_T_NULL; // Fuel system monitoring complete (All)
	DEM_TRISTATE CCM_CMPL	= DEM_T_NULL; // Comprehensive component monitoring complete (All)
	DEM_TRISTATE HCCATCMP  	= DEM_T_NULL; // NMHC catalyst monitoring complete (Diesel)
	DEM_TRISTATE NCATCMPL	= DEM_T_NULL; // NOx aftertreatment monitoring complete (Diesel)
	DEM_TRISTATE BP_CMPL   	= DEM_T_NULL; // Boost pressure system monitoring complete (Diesel)
	DEM_TRISTATE EGS_CMPL	= DEM_T_NULL; // Exhaust gas sensor monitoring complete (Diesel)
	DEM_TRISTATE PM_CMPL	= DEM_T_NULL; // PM Filter monitoring complete (Diesel)
	DEM_TRISTATE EGR_CMPL	= DEM_T_NULL; // EGR system monitoring complete (All)

	ENG_TYPE = 1;

	/* @req DEM355 */
	for (uint16 i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
		if (DEM_EVENT_ID_NULL != eventStatusBuffer[i].eventId) {
			Dem_EventOBDReadinessGroup readinessGroup = eventStatusBuffer[i].eventParamRef->EventClass->OBDReadinessGroup;

			DEM_TRISTATE monitoringNotComplete = isMonitoringNotCompleteThisDrivingCycle(i);
			DEM_TRISTATE isEnabledThisDrivingCycle = (! eventStatusBuffer[i].isDisabled);

			switch(readinessGroup) {
				case DEM_OBD_RDY_MISF:
					MIS_ENA   = (MIS_ENA   == DEM_T_NULL) ? isEnabledThisDrivingCycle   : (MIS_ENA  & isEnabledThisDrivingCycle);
					MIS_CMPL  = (MIS_CMPL  == DEM_T_NULL) ? monitoringNotComplete       : (MIS_CMPL & monitoringNotComplete);

					break;
				case DEM_OBD_RDY_FLSYS:
					FUEL_ENA  = (FUEL_ENA  == DEM_T_NULL) ? isEnabledThisDrivingCycle   : (FUEL_ENA & isEnabledThisDrivingCycle);
					FUELCMPL  = (FUELCMPL  == DEM_T_NULL) ? monitoringNotComplete       : (FUELCMPL & monitoringNotComplete);

					break;
				case DEM_OBD_RDY_CMPRCMPT:
					CCM_ENA  = (CCM_ENA == DEM_T_NULL)    ? isEnabledThisDrivingCycle   : (CCM_ENA  & isEnabledThisDrivingCycle);
					CCM_CMPL = DEM_T_FALSE; // Always on

					break;
				case DEM_OBD_RDY_HCCAT:
					HCCATENA  = (HCCATENA  == DEM_T_NULL) ? isEnabledThisDrivingCycle   : (HCCATENA & isEnabledThisDrivingCycle);
					HCCATCMP  = (HCCATCMP  == DEM_T_NULL) ? monitoringNotComplete       : (HCCATCMP & monitoringNotComplete);

					break;
				case DEM_OBD_RDY_NOXCAT:
					NCAT_ENA  = (NCAT_ENA  == DEM_T_NULL) ? isEnabledThisDrivingCycle   : (NCAT_ENA & isEnabledThisDrivingCycle);
					NCATCMPL  = (NCATCMPL  == DEM_T_NULL) ? monitoringNotComplete       : (NCATCMPL & monitoringNotComplete);

					break;
				case DEM_OBD_RDY_BOOSTPR:
					BP_ENA    = (BP_ENA  == DEM_T_NULL)   ? isEnabledThisDrivingCycle   : (BP_ENA & isEnabledThisDrivingCycle);
					BP_CMPL   = (BP_CMPL == DEM_T_NULL)   ? monitoringNotComplete       : (BP_CMPL & monitoringNotComplete);

					break;
				case DEM_OBD_RDY_EGSENS:
					EGS_ENA   = (EGS_ENA  == DEM_T_NULL)  ? isEnabledThisDrivingCycle   : (EGS_ENA & isEnabledThisDrivingCycle);
					EGS_CMPL  = (EGS_CMPL  == DEM_T_NULL) ? monitoringNotComplete       : (EGS_CMPL & monitoringNotComplete);

					break;
				case DEM_OBD_RDY_PMFLT:
					PM_ENA    = (PM_ENA  == DEM_T_NULL)   ? isEnabledThisDrivingCycle   : (PM_ENA & isEnabledThisDrivingCycle);
					PM_CMPL   = (PM_CMPL == DEM_T_NULL)   ? monitoringNotComplete       : (PM_CMPL & monitoringNotComplete);

					break;
				case DEM_OBD_RDY_ERG:
					EGR_ENA   = (EGR_ENA  == DEM_T_NULL)  ? isEnabledThisDrivingCycle   : (EGR_ENA & isEnabledThisDrivingCycle);
					EGR_CMPL  = (EGR_CMPL == DEM_T_NULL)  ? monitoringNotComplete       : (EGR_CMPL & monitoringNotComplete);

					break;
			}
		}
	}

	/**
	 * Set remaining NULL systems to FALSE
	 */
	DEM_TRISTATE *monitorsEnabled[]  = {&MIS_ENA, &FUEL_ENA, &CCM_ENA, &HCCATENA, &NCAT_ENA, &BP_ENA, &EGS_ENA, &PM_ENA, &EGR_ENA};
	resetOrKeepMonitorValues(9, monitorsEnabled);

	DEM_TRISTATE *monitorsComplete[] = {&MIS_CMPL, &FUELCMPL, &CCM_CMPL, &HCCATCMP, &NCATCMPL, &BP_CMPL, &EGS_CMPL, &PM_CMPL, &EGR_CMPL};
	resetOrKeepMonitorValues(9, monitorsComplete);

	/**
	 * Pack status and completion values
	 */
	// Byte A
	// Both 0, does not report
	// Byte B
	PID41value[1] = (MIS_ENA << 0)  | (FUEL_ENA << 1) | (CCM_ENA << 2) | (ENG_TYPE << 3) | (MIS_CMPL << 4) | (FUELCMPL << 5) | (CCM_CMPL << 6); // Bit 7 reserved
	// Byte C
	PID41value[2] = (HCCATENA << 0) | (NCAT_ENA << 1) | (BP_ENA << 3)  | (EGS_ENA << 5)  | (PM_ENA << 6)   | (EGR_ENA << 7); // bit 2 reserved // bit 4 reserved
	// Byte D
	PID41value[3] = (HCCATCMP << 0) | (NCATCMPL << 1) | (BP_CMPL << 3) | (EGS_CMPL << 5) | (PM_CMPL << 6)  | (EGR_CMPL << 7); // bit 2 reserved // bit 4 reserved

	return E_OK;
}
#endif

#if defined(DEM_USE_IUMPR)
/**
 * Writes current denominator and numerator of a specific ratio to the given data buffer.
 *
 * @param dataBuffer
 * @param startIndex
 * @param ratioId
 */
static void getNumeratorDenominator(uint8* dataBuffer, uint8 startIndex, Dem_RatioIdType ratioId) {
	uint8 currentIndex = startIndex;

	// write numerator
	dataBuffer[currentIndex++] = (uint8)(iumprBufferLocal[ratioId].numerator.value   >> 8);
	dataBuffer[currentIndex++] = (uint8)(iumprBufferLocal[ratioId].numerator.value   & 0xFF);
	// write denominator
	dataBuffer[currentIndex++] = (uint8)(iumprBufferLocal[ratioId].denominator.value >> 8);
	dataBuffer[currentIndex]   = (uint8)(iumprBufferLocal[ratioId].denominator.value & 0xFF);
}

/**
 * Writes general denominator count to data buffer
 *
 * @param dataBuffer
 */
static void getGeneralDenominatorCount(uint8* dataBuffer) {
	dataBuffer[0] = (uint8)(generalDenominatorBuffer.value >> 8);
	dataBuffer[1] = (uint8)(generalDenominatorBuffer.value & 0xFF);
}

/**
 * Writes ignition cycle count to data buffer
 *
 * @param dataBuffer
 */
static void getIgnitionCycleCount(uint8* dataBuffer) {
	dataBuffer[2] = (uint8)(ignitionCycleCountBuffer >> 8);
	dataBuffer[3] = (uint8)(ignitionCycleCountBuffer & 0xFF);
}

#if (DEM_OBD_ENGINE_TYPE == DEM_IGNITION_SPARK) /* @req DEM357 */
/**
 * Writes denominator and numerator counts to InfoType0 data buffer depending on IUMPR group
 *
 * @param iumprGroup
 * @param Iumprdata08
 */
static void setInfoType08NumsDenoms(Dem_IUMPRGroup iumprGroup, uint8* Iumprdata08, Dem_RatioIdType ratioId) {
	switch(iumprGroup) {
		case DEM_IUMPR_CAT1:
			getNumeratorDenominator(Iumprdata08, 4, ratioId);
			break;
		case DEM_IUMPR_CAT2:
			getNumeratorDenominator(Iumprdata08, 8, ratioId);
			break;
		case DEM_IUMPR_OXS1:
			getNumeratorDenominator(Iumprdata08, 12, ratioId);
			break;
		case DEM_IUMPR_OXS2:
			getNumeratorDenominator(Iumprdata08, 16, ratioId);
			break;
		case DEM_IUMPR_EGR:
			getNumeratorDenominator(Iumprdata08, 20, ratioId);
			break;
		case DEM_IUMPR_SAIR:
			getNumeratorDenominator(Iumprdata08, 24, ratioId);
			break;
		case DEM_IUMPR_EVAP:
			getNumeratorDenominator(Iumprdata08, 28, ratioId);
			break;
		case DEM_IUMPR_SECOXS1:
			getNumeratorDenominator(Iumprdata08, 32, ratioId);
			break;
		case DEM_IUMPR_SECOXS2:
			getNumeratorDenominator(Iumprdata08, 36, ratioId);
			break;
		case DEM_IUMPR_AFRI1:
			getNumeratorDenominator(Iumprdata08, 40, ratioId);
			break;
		case DEM_IUMPR_AFRI2:
			getNumeratorDenominator(Iumprdata08, 44, ratioId);
			break;
		case DEM_IUMPR_PF1:
			getNumeratorDenominator(Iumprdata08, 48, ratioId);
			break;
		case DEM_IUMPR_PF2:
			getNumeratorDenominator(Iumprdata08, 52, ratioId);
			break;
	}
}
#endif /* DEM_USE_IUMPR */
/**
 * Service is used to request for IUMPR data according to InfoType $08.
 * Reentrant: Yes
 *
 * @param Iumprdata08: Buffer containing the contents of InfoType $08.
 * The buffer is provided by the Dcm.
 * @return Always E_OK is returned, as E_PENDING and E_NOT_OK will never appear.
 */
Std_ReturnType Dem_GetInfoTypeValue08(uint8* Iumprdata08)
{
	/* @req DEM316 */
	/* @req DEM298 */
	if (Iumprdata08 != NULL_PTR && demState == DEM_INITIALIZED) {
		// reset data
		memset(Iumprdata08, 0, 56);

		// get general denominator count
		getGeneralDenominatorCount(Iumprdata08);

		// get ignition cycle count
		getIgnitionCycleCount(Iumprdata08);

		for (Dem_RatioIdType ratioId = 0; ratioId < DEM_IUMPR_REGISTERED_COUNT; ratioId++) {
			Dem_IUMPRGroup iumprGroup = Dem_RatiosList[ratioId].IumprGroup;

			setInfoType08NumsDenoms(iumprGroup, Iumprdata08, ratioId);
		}
	}

	return E_OK;
}
#elif (DEM_OBD_ENGINE_TYPE == DEM_IGNITION_COMPR) /* @req DEM358 */
static void setInfoType0BNumsDenoms(Dem_IUMPRGroup iumprGroup, uint8* Iumprdata0B, Dem_RatioIdType ratioId) {
	switch(iumprGroup) {
		case DEM_IUMPR_NMHCCAT:
			getNumeratorDenominator(Iumprdata0B, 4, ratioId);
			break;
		case DEM_IUMPR_NOXCAT:
			getNumeratorDenominator(Iumprdata0B, 8, ratioId);
			break;
		case DEM_IUMPR_NOXADSORB:
			getNumeratorDenominator(Iumprdata0B, 12, ratioId);
			break;
		case DEM_IUMPR_PMFILTER:
			getNumeratorDenominator(Iumprdata0B, 16, ratioId);
			break;
		case DEM_IUMPR_EGSENSOR:
			getNumeratorDenominator(Iumprdata0B, 20, ratioId);
			break;
		case DEM_IUMPR_EGR:
			getNumeratorDenominator(Iumprdata0B, 24, ratioId);
			break;
		case DEM_IUMPR_BOOSTPRS:
			getNumeratorDenominator(Iumprdata0B, 28, ratioId);
			break;
		case DEM_IUMPR_FLSYS:
			getNumeratorDenominator(Iumprdata0B, 32, ratioId);
			break;
	}
}

/**
 * Service is used to request for IUMPR data according to InfoType $0B.
 * Reentrant: Yes
 *
 * @param Iumprdata08: Buffer containing the contents of InfoType $0B.
 * The buffer is provided by the Dcm.
 * @return Always E_OK is returned, as E_PENDING and E_NOT_OK will never appear.
 */
Std_ReturnType Dem_GetInfoTypeValue0B(uint8* Iumprdata0B)
{
	/* @req DEM317 */
	/* @req DEM298 */
	if ((demState == DEM_INITIALIZED) && (Iumprdata0B != NULL_PTR)) {
		// reset data
		memset(Iumprdata0B, 0, 36);

		// get general denominator count
		getGeneralDenominatorCount(Iumprdata0B);

		// get ignition cycle count
		getIgnitionCycleCount(Iumprdata0B);

		for (Dem_RatioIdType ratioId = 0; ratioId < DEM_IUMPR_REGISTERED_COUNT; ratioId++) {
			Dem_IUMPRGroup iumprGroup = Dem_RatiosList[ratioId].IumprGroup;

			setInfoType0BNumsDenoms(iumprGroup, Iumprdata0B, ratioId);
		}
	}

	return E_OK;
}
#endif

/**
 * Service for reporting that faults are possibly found because all conditions are fulfilled.
 * Reentrant: No
 *
 * @param RatioID: Ratio Identifier reporting that a respective monitor could have
 * found a fault - only used when interface option "API" is selected.
 * @return E_OK report of IUMPR result was successfully reported.
 */
Std_ReturnType Dem_RepIUMPRFaultDetect(Dem_RatioIdType RatioID)
{
	VALIDATE_RV(DEM_INITIALIZED == demState, DEM_REPIUMPRFAULTDETECT_ID, DEM_E_UNINIT, E_NOT_OK);
#if defined(DEM_USE_IUMPR)
	// valid RatioID
	VALIDATE_RV(RatioID < DEM_IUMPR_REGISTERED_COUNT, DEM_REPIUMPRFAULTDETECT_ID, DEM_E_PARAM_DATA, E_NOT_OK);

	/* @req DEM313 */
	/* @req DEM360 */
	Std_ReturnType ret = E_NOT_OK;

	SchM_Enter_Dem_EA_0();
	/* @req DEM296 */
	if (Dem_RatiosList[RatioID].RatioKind == DEM_RATIO_API) { // is API call
		ret = incrementIumprNumerator(RatioID);
	}
	SchM_Exit_Dem_EA_0();

	return ret;
#else
	(void)RatioID;
	return E_NOT_OK;
#endif
}

/**
 * Service is used to lock a denominator of a specific monitor.
 * Reentrant: Yes
 *
 * @param RatioID: Ratio Identifier reporting that specific denominator is locked (for
 * physical reasons - e.g. temperature conditions or minimum activity).
 * @return  E_OK report of IUMPR denominator status was successfully reported.
 * E_NOT_OK report of IUMPR denominator status was not successfully reported.
 */
Std_ReturnType Dem_RepIUMPRDenLock(Dem_RatioIdType RatioID)
{
	Std_ReturnType ret = E_NOT_OK;

	VALIDATE_RV(DEM_INITIALIZED == demState, DEM_REPIUMPRFAULTDETECT_ID, DEM_E_UNINIT, E_NOT_OK);
#if defined(DEM_USE_IUMPR)
	// valid RatioID
	VALIDATE_RV(RatioID < DEM_IUMPR_REGISTERED_COUNT, DEM_REPIUMPRFAULTDETECT_ID, DEM_E_PARAM_DATA, E_NOT_OK);


	/* @req DEM314 */
	/* @req DEM362 */
	/* @req DEM297 */
	if (FALSE == iumprBufferLocal[RatioID].denominator.isLocked) {
		iumprBufferLocal[RatioID].denominator.isLocked = TRUE;

		ret = E_OK;
	}
#else
	(void)RatioID;
#endif
	return ret;
}

/**
 * Service is used to release a denominator of a specific monitor.
 * Reentrant: Yes
 *
 * @param RatioID: Ratio Identifier reporting that specific denominator is released
 * (for physical reasons - e.g. temperature conditions or minimum activity).
 * @return  E_OK report of IUMPR denominator status was successfully reported.
 * E_NOT_OK report of IUMPR denominator status was not successfully reported.
 */
Std_ReturnType Dem_RepIUMPRDenRelease(Dem_RatioIdType RatioID)
{
	Std_ReturnType ret = E_NOT_OK;

	VALIDATE_RV(DEM_INITIALIZED == demState, DEM_REPIUMPRFAULTDETECT_ID, DEM_E_UNINIT, E_NOT_OK);
#if defined(DEM_USE_IUMPR)
	// valid RatioID
	VALIDATE_RV(RatioID < DEM_IUMPR_REGISTERED_COUNT, DEM_REPIUMPRFAULTDETECT_ID, DEM_E_PARAM_DATA, E_NOT_OK);

	/* @req Dem315 */
	/* @req Dem362 */
	/* @req Dem308 */
	if (TRUE == iumprBufferLocal[RatioID].denominator.isLocked) {
		iumprBufferLocal[RatioID].denominator.isLocked = FALSE;

		// consider this as qualified driving cycle (if applicable), increment denominator immediately
		(void) incrementIumprDenominator(RatioID);

		ret = E_OK;
	}
#else
	(void)RatioID;
#endif
	return ret;
}

#if defined(DEM_USE_IUMPR)
/**
 * Increment general denominator if set condition is general denominator and status is reached.
 *
 * @param ConditionId: Identification of a IUMPR denominator condition ID (General Denominator, Cold start, EVAP, 500mi).
 * @param ConditionStatus: Status of the IUMPR denominator condition (Notreached, reached, not reachable / inhibited).
 */
static void incrementGeneralDenominator(Dem_IumprDenomCondIdType ConditionId, Dem_IumprDenomCondStatusType ConditionStatus) {
	if (ConditionId == DEM_IUMPR_GENERAL_OBDCOND && ConditionStatus == DEM_IUMPR_DEN_STATUS_REACHED) {
		if (FALSE == generalDenominatorBuffer.incrementedThisDrivingCycle) {
			// If driving cycle started
			if (TRUE == operationCycleIsStarted(DEM_OBD_DCY)) {
				// If the general denominator reaches the maximum value of 65,535 ±2, the
				// general denominator shall rollover and increment to zero on the next
				// driving cycle that meets the general denominator definition to avoid
				// overflow problems.
				if (generalDenominatorBuffer.value == 65535) {
					generalDenominatorBuffer.value = 0;
				} else {
					generalDenominatorBuffer.value++;
				}

				generalDenominatorBuffer.incrementedThisDrivingCycle = TRUE;
			}
		}
	}
}
#endif

/**
 * In order to communicate the status of the (additional) denominator conditions among the OBD relevant ECUs,
 * the API is used to forward the condition status to a Dem of a particular ECU.
 * API is needed in OBD-relevant ECUs only.
 *
 * @param ConditionId: Identification of a IUMPR denominator condition ID (General Denominator, Cold start, EVAP, 500mi).
 * @param ConditionStatus: Status of the IUMPR denominator condition (Notreached, reached, not reachable / inhibited).
 * @return E_OK: set of IUMPR denominator condition was successful,
 * E_NOT_OK: set of IUMPR denominator condition failed or could not be accepted.
 */
Std_ReturnType Dem_SetIUMPRDenCondition(Dem_IumprDenomCondIdType ConditionId, Dem_IumprDenomCondStatusType ConditionStatus) {
	Std_ReturnType ret = E_NOT_OK;

	// dem started
	VALIDATE_RV(DEM_INITIALIZED == demState, DEM_REPIUMPRFAULTDETECT_ID, DEM_E_UNINIT, E_NOT_OK);

	// valid condition ID
	VALIDATE_RV(((ConditionId == DEM_IUMPR_DEN_COND_COLDSTART) || (ConditionId == DEM_IUMPR_DEN_COND_EVAP) || (ConditionId == DEM_IUMPR_DEN_COND_500MI) || (ConditionId == DEM_IUMPR_GENERAL_OBDCOND)), DEM_REPIUMPRFAULTDETECT_ID, DEM_E_PARAM_DATA, E_NOT_OK);

	// valid condition status
	VALIDATE_RV(ConditionStatus < 0x03, DEM_REPIUMPRFAULTDETECT_ID, DEM_E_PARAM_DATA, E_NOT_OK);
#if defined(DEM_USE_IUMPR)
	for (uint8 i = 0; i < DEM_IUMPR_ADDITIONAL_DENOMINATORS_COUNT; i++) {
		if (iumprAddiDenomCondBuffer[i].condition == ConditionId) {
			if (iumprAddiDenomCondBuffer[i].status != ConditionStatus) {
				iumprAddiDenomCondBuffer[i].status = ConditionStatus;

				ret = E_OK;

				// if applicable
				incrementGeneralDenominator(ConditionId, ConditionStatus);
			}

			break;
		}
	}
#endif
	return ret;
}

/**
 * In order to communicate the status of the (additional) denominator conditions among the OBD relevant ECUs, the API is used to retrieve
 * the condition status from the Dem of the ECU where the conditions are computed. API is needed in OBD-relevant ECUs only.
 *
 * @param ConditionId: Identification of a IUMPR denominator condition ID (General Denominator, Cold start, EVAP, 500mi).
 * @param ConditionStatus: Status of the IUMPR denominator condition (Notreached, reached, not reachable / inhibited)
 * @return E_OK: get of IUMPR denominator condition status was successful,
 * E_NOT_OK: get of condition status failed.
 */
Std_ReturnType Dem_GetIUMPRDenCondition(Dem_IumprDenomCondIdType ConditionId, Dem_IumprDenomCondStatusType* ConditionStatus) {
	Std_ReturnType ret = E_NOT_OK;

	// dem started
	VALIDATE_RV(DEM_INITIALIZED == demState, DEM_REPIUMPRFAULTDETECT_ID, DEM_E_UNINIT, E_NOT_OK);

	// valid condition ID
	VALIDATE_RV(((ConditionId == DEM_IUMPR_DEN_COND_COLDSTART) || (ConditionId == DEM_IUMPR_DEN_COND_EVAP) || (ConditionId == DEM_IUMPR_DEN_COND_500MI) || (ConditionId == DEM_IUMPR_GENERAL_OBDCOND)), DEM_REPIUMPRFAULTDETECT_ID, DEM_E_PARAM_DATA, E_NOT_OK);
#if defined(DEM_USE_IUMPR)
	for (uint8 i = 0; i < DEM_IUMPR_ADDITIONAL_DENOMINATORS_COUNT; i++) {
		if (iumprAddiDenomCondBuffer[i].condition == ConditionId) {
			*ConditionStatus = iumprAddiDenomCondBuffer[i].status;

			ret = E_OK;
		}
	}
#endif
	return ret;
}

#endif

/**
 * Set the suppression status of a specific DTC.
 * @param DTC
 * @param DTCFormat
 * @param SuppressionStatus
 * @return E_OK (Operation was successful), E_NOT_OK (operation failed or event entry for this DTC still exists)
 */
/* @req DEM589 */
/* @req 4.2.2/SWS_Dem_01047 */
#if (DEM_DTC_SUPPRESSION_SUPPORT == STD_ON)
/* @req 4.2.2/SWS_Dem_00586 */
Std_ReturnType Dem_SetDTCSuppression(uint32 DTC, Dem_DTCFormatType DTCFormat, boolean SuppressionStatus)
{
    /* Requirement tag intentionally incorrect. Handled in DEM.py */
    /* !req DEM588 Allowing suppression of DTC even if event memory entry exists (this requirement is removed in ASR 4.2.2) */
    Std_ReturnType ret = E_NOT_OK;
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_SETDTCSUPPRESSION_ID, DEM_E_UNINIT, E_NOT_OK);
    VALIDATE_RV(IS_VALID_DTC_FORMAT(DTCFormat), DEM_SETDTCSUPPRESSION_ID, DEM_E_PARAM_DATA, E_NOT_OK);
    const Dem_DTCClassType *DTCClassPtr = configSet->DTCClass;
    while( FALSE == DTCClassPtr->Arc_EOL ) {
        if( ((DEM_DTC_FORMAT_UDS == DTCFormat) && (DTCClassPtr->DTCRef->UDSDTC == DTC)) ||
            ((DEM_DTC_FORMAT_OBD == DTCFormat) && (TO_OBD_FORMAT(DTCClassPtr->DTCRef->OBDDTC) == DTC))) {
            DemDTCSuppressed[DTCClassPtr->DTCIndex].SuppressedByDTC = SuppressionStatus;
            ret = E_OK;
            break;
        }
        DTCClassPtr++;
    }
    return ret;
}
#endif

#if defined(DEM_USE_MEMORY_FUNCTIONS)
/**
 * Check if an event is stored in freeze frame buffer
 * @param eventId
 * @param dtcOrigin
 * @return TRUE: Event stored in FF buffer, FALSE: Event NOT stored in FF buffer
 */
static boolean isInFFBuffer(Dem_EventIdType eventId, Dem_DTCOriginType dtcOrigin)
{
    boolean ffFound = FALSE;

    FreezeFrameRecType* freezeFrameBuffer = NULL;
    uint32 freezeFrameBufferSize = 0;

    switch (dtcOrigin) {
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
#if ((DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_PRI_MEM)
            freezeFrameBuffer = priMemFreezeFrameBuffer;
            freezeFrameBufferSize = DEM_MAX_NUMBER_FF_DATA_PRI_MEM;
#endif
            break;
        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
#if ((DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON) && DEM_FF_DATA_IN_SEC_MEM)
            freezeFrameBuffer = secMemFreezeFrameBuffer;
            freezeFrameBufferSize = DEM_MAX_NUMBER_FF_DATA_SEC_MEM;
#endif
            break;
        default:
            DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GLOBAL_ID, DEM_E_NOT_IMPLEMENTED_YET);
            break;
    }

    if (freezeFrameBuffer != NULL) {

        for (uint32 i = 0uL; (i < freezeFrameBufferSize) && (FALSE == ffFound); i++) {
            ffFound = (freezeFrameBuffer[i].eventId == eventId)? TRUE: FALSE;
        }
    }
    return ffFound;
}
#endif
/**
 * Checks if event is stored in event memory
 * @param EventId
 * @return TRUE: Event stored in memory, FALSE: event NOT stored in memory
 */
static boolean EventIsStoredInMemory(Dem_EventIdType EventId)
{
#if defined(DEM_USE_MEMORY_FUNCTIONS)
    const Dem_EventParameterType *eventParam;
    ExtDataRecType *extData;
    boolean isStored = FALSE;

    lookupEventIdParameter(EventId, &eventParam);
    if( (NULL != eventParam) &&  (DEM_DTC_ORIGIN_NOT_USED != eventParam->EventClass->EventDestination) ) {
        isStored = ((TRUE == isInEventMemory(eventParam)) || (TRUE == isInFFBuffer(EventId, eventParam->EventClass->EventDestination)) || (TRUE == lookupExtendedDataMem(EventId, &extData, eventParam->EventClass->EventDestination)))? TRUE: FALSE;
    }
    return isStored;
#else
    (void)EventId;
    return FALSE;
#endif
}

/**
 * Set the available status of a specific Event.
 * @param EventId: Identification of an event by assigned EventId.
 * @param AvailableStatus: This parameter specifies whether the respective Event shall be available (TRUE) or not (FALSE).
 * @return: E_OK: Operation was successful, E_NOT_OK: change of available status not accepted
 */
/* @req 4.2.2/SWS_Dem_01080 */
Std_ReturnType Dem_SetEventAvailable(Dem_EventIdType EventId, boolean AvailableStatus)
{
#if (DEM_SET_EVENT_AVAILABLE_PREINIT == STD_ON)
    VALIDATE_RV(DEM_UNINITIALIZED != demState, DEM_SETEVENTAVAILABLE_ID, DEM_E_UNINIT, E_NOT_OK);
#else
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_SETEVENTAVAILABLE_ID, DEM_E_UNINIT, E_NOT_OK);
#endif
    VALIDATE_RV(IS_VALID_EVENT_ID(EventId), DEM_SETEVENTAVAILABLE_ID, DEM_E_PARAM_DATA, E_NOT_OK);

    const Dem_EventParameterType *eventParam = NULL;
    Dem_EventStatusExtendedType oldStatus;
    EventStatusRecType *eventStatusRec = NULL;
    Std_ReturnType ret = E_NOT_OK;

    if ( (demState == DEM_UNINITIALIZED)
#if (DEM_SET_EVENT_AVAILABLE_PREINIT == STD_OFF)
            || (demState == DEM_PREINITIALIZED)
#endif
    ) {
        return E_NOT_OK;
    }

    SchM_Enter_Dem_EA_0();

    lookupEventStatusRec(EventId, &eventStatusRec);
    lookupEventIdParameter(EventId, &eventParam);
    /* @req 4.2.2/SWS_Dem_01109 */
    if( (NULL != eventStatusRec) && (NULL != eventParam) && (*eventParam->EventClass->EventAvailableByCalibration == TRUE) &&
            (0u == (eventStatusRec->eventStatusExtended & DEM_TEST_FAILED)) && ((DEM_PREINITIALIZED == demState) || (FALSE == EventIsStoredInMemory(EventId))) ) {
        if( eventStatusRec->isAvailable != AvailableStatus ) {
            /* Event availability changed */
            eventStatusRec->isAvailable = AvailableStatus;
            oldStatus = eventStatusRec->eventStatusExtended;
            if( FALSE == AvailableStatus ) {
                /* @req 4.2.2/SWS_Dem_01110 */
                eventStatusRec->eventStatusExtended = 0x00;
            } else {
                /* @req 4.2.2/SWS_Dem_01111 */
                eventStatusRec->eventStatusExtended = 0x50;
            }

            if( oldStatus != eventStatusRec->eventStatusExtended ) {
                notifyEventStatusChange(eventParam, oldStatus, eventStatusRec->eventStatusExtended);
            }
#if (DEM_DTC_SUPPRESSION_SUPPORT == STD_ON)
            /* Check if suppression of DTC is affected */
            boolean suppressed = TRUE;
            EventStatusRecType *dtcEventStatusRec;
            if( (NULL != eventParam->DTCClassRef) && (NULL != eventParam->DTCClassRef->Events) ) {
                for( uint16 i = 0; (i < eventParam->DTCClassRef->NofEvents) && (TRUE == suppressed); i++ ) {
                    dtcEventStatusRec = NULL;
                    lookupEventStatusRec(eventParam->DTCClassRef->Events[i], &dtcEventStatusRec);
                    if( (NULL != dtcEventStatusRec) && (TRUE == dtcEventStatusRec->isAvailable) ) {
                        /* Event is available -> DTC NOT suppressed */
                        suppressed = FALSE;
                    }
                }
                if( 0 != eventParam->DTCClassRef->NofEvents ) {
                    DemDTCSuppressed[eventParam->DTCClassRef->DTCIndex].SuppressedByEvent = suppressed;
                }
            }
#endif
        }
        ret = E_OK;
    }
    SchM_Exit_Dem_EA_0();
    return ret;
}

/**
 * Gets the current monitor status for an event.
 * @param EventID
 * @param MonitorStatus
 * @return E_OK: Get monitor status was successful, E_NOT_OK: getting the monitor status failed (e.g.an invalid event id was provided).
 */
/* @req 4.3.0/SWS_Dem_91007 */
Std_ReturnType Dem_GetMonitorStatus(Dem_EventIdType EventID, Dem_MonitorStatusType* MonitorStatus)
{
    Std_ReturnType ret = E_NOT_OK;
    EventStatusRecType * eventStatusRec;
    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_GETMONITORSTATUS_ID, DEM_E_UNINIT, E_NOT_OK);
    VALIDATE_RV((NULL != MonitorStatus), DEM_GETMONITORSTATUS_ID, DEM_E_PARAM_POINTER, E_NOT_OK);

    if( IS_VALID_EVENT_ID(EventID) ) {
        /* @req 4.3.0/SWS_Dem_01287 */
        lookupEventStatusRec(EventID, &eventStatusRec);
        if( NULL != eventStatusRec ) {
            *MonitorStatus = 0u;
            if( 0u != (eventStatusRec->eventStatusExtended & DEM_TEST_FAILED) ) {
                *MonitorStatus |= DEM_MONITOR_STATUS_TF;
            }
            if( 0u != (eventStatusRec->eventStatusExtended & DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE) ) {
                *MonitorStatus |= DEM_MONITOR_STATUS_TNCTOC;
            }
            ret = E_OK;
        }
    }
    else {
        DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GETMONITORSTATUS_ID, DEM_E_PARAM_DATA);
        /* @req 4.3.0/SWS_Dem_01288 */
    }
    return ret;
}
