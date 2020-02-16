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

// 904 PC-Lint MISRA 14.7: OK. Allow VALIDATE, VALIDATE_RV and VALIDATE_NO_RV to return value.
//lint -emacro(904,VALIDATE_RV,VALIDATE_NO_RV,VALIDATE)

// 522 PC-Lint exception for empty functions
//lint -esym(522,storeFreezeFrameDataEvtMem)
//lint -esym(522,deleteFreezeFrameDataMem)
//lint -esym(522,storeFreezeFrameDataPreInit)
//lint -esym(522,storeFreezeFrameDataMem)

// PC-Lint misunderstanding of MISRA 8.12, 18.1: Empty arrays are OK as long as size 0 is specified.
//lint -esym(85,preInitFreezeFrameBuffer)
//lint -esym(85,priMemFreezeFrameBuffer)


/*
 *  General requirements
 */
/** @req DEM126 */
/** @req DEM151.partially */
/** @req DEM152 */
/** @req DEM013.14229-1 */
/** @req DEM277 */
/** @req DEM113 */ /** @req DEM174 */
/** @req DEM286 */
/** @req DEM267 */
/** @req DEM364 */
/** @req DEM114 */
/** @req DEM124 */
/** @req DEM370 */



#include <string.h>
#include "Dem.h"
//#include "Fim.h"
#if defined(USE_NVM)
#if (DEM_USE_NVM == STD_ON)
#include "NvM.h"
#endif
#endif
//#include "SchM_Dem.h"
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
/*
 * Local defines
 */
#define DEM_DEFAULT_EVENT_STATUS (DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR | DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE)
#define DEM_UDS_TEST_FAILED_TRESHOLD  (sint32)127
#define DEM_UDS_TEST_PASSED_TRESHOLD (sint32)(-128)
#define DEM_UDS_FDC_RANGE (sint32)(DEM_UDS_TEST_FAILED_TRESHOLD - (DEM_UDS_TEST_PASSED_TRESHOLD))

#define DEM_PID_IDENTIFIER_SIZE_OF_BYTES        1 // OBD
#define DEM_CONFIRMATION_CNTR_MAX 255

#if  ( DEM_DEV_ERROR_DETECT == STD_ON )
#if defined(USE_DET)
#include "Det.h"
#endif
/** @req DEM117 */
#define DET_REPORTERROR(_x,_y,_z,_q) Det_ReportError(_x, _y, _z, _q)

#else
#define DET_REPORTERROR(_x,_y,_z,_q)
#endif

#define VALIDATE_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          DET_REPORTERROR(MODULE_ID_DEM, 0, _api, _err); \
          return _rv; \
        }

#define VALIDATE_NO_RV(_exp,_api,_err ) \
  if( !(_exp) ) { \
          DET_REPORTERROR(MODULE_ID_DEM, 0, _api, _err); \
          return; \
        }

#if (DEM_OBD_SUPPORT == STD_ON)
#error "DEM_OBD_SUPPORT is set to STD_ON, this is not supported by the code."
#endif

#if (DEM_PTO_SUPPORT == STD_ON)
#error "DEM_PTO_SUPPORT is set to STD_ON, this is not supported by the code."
#endif

#if !(DEM_TYPE_OF_DTC_SUPPORTED == DEM_DTC_TRANSLATION_ISO15031_6 || DEM_TYPE_OF_DTC_SUPPORTED == DEM_DTC_TRANSLATION_ISO14229_1)
#error "DEM_TYPE_OF_DTC_SUPPORTED is not set to ISO15031-6 or ISO14229-1. Only these are supported by the code."
#endif

#if !defined(USE_DEM_EXTENSION)
#if defined(DEM_FREEZE_FRAME_CAPTURE_EXTENSION)
#error "DEM_FREEZE_FRAME_CAPTURE cannot be DEM_TRIGGER_EXTENSION since Dem extension is not used!"
#endif
#if defined(DEM_EXTENDED_DATA_CAPTURE_EXTENSION)
#error "DEM_EXTENDED_DATA_CAPTURE cannot be DEM_TRIGGER_EXTENSION since Dem extension is not used!"
#endif
#if defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
#error "DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION cannot be used since Dem extension is not used!"
#endif
#if defined(CFG_DEM_CONFIRMATION_PROCESSING_EXTENSION)
#error "CFG_DEM_CONFIRMATION_PROCESSING_EXTENSION cannot be used since Dem extension is not used!"
#endif
#if defined(CFG_DEM_AGING_PROCESSING_EXTENSION)
#error "CFG_DEM_AGING_PROCESSING_EXTENSION cannot be used since Dem extension is not used!"
#endif
#endif
#if defined(DEM_EXTENDED_DATA_CAPTURE_EVENT_MEMORY_STORAGE)
#error "DEM_EXTENDED_DATA_CAPTURE_EVENT_MEMORY_STORAGE is not supported!"
#endif

#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) || (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
#define DEM_USE_MEMORY_FUNCTIONS
#endif

#define DEM_ASSERT(_exp)        switch (1) {case 0: break; case _exp: break; }

#if (DEM_TEST_FAILED_STORAGE == STD_ON)
#define GET_STORED_STATUS_BITS(_x) ((_x) & (DEM_TEST_FAILED_SINCE_LAST_CLEAR | DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR | DEM_PENDING_DTC | DEM_CONFIRMED_DTC | DEM_TEST_FAILED))
#else
#define GET_STORED_STATUS_BITS(_x) ((_x) & (DEM_TEST_FAILED_SINCE_LAST_CLEAR | DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR | DEM_PENDING_DTC | DEM_CONFIRMED_DTC))

#endif

#define IS_VALID_EVENT_STATUS(_x) ((DEM_EVENT_STATUS_PREPASSED == _x) || (DEM_EVENT_STATUS_PASSED == _x) || (DEM_EVENT_STATUS_PREFAILED == _x) || (DEM_EVENT_STATUS_FAILED == _x))
/*
 * Local types
 */
#if !defined(USE_DCM)
typedef uint8 Dcm_NegativeResponseCodeType;
#define DCM_E_POSITIVERESPONSE ((Dcm_NegativeResponseCodeType)0x00)
#endif

// DtcFilterType
typedef struct {
    Dem_EventStatusExtendedType dtcStatusMask;
    Dem_DTCKindType             dtcKind;
    Dem_DTCOriginType           dtcOrigin;
    Dem_FilterWithSeverityType  filterWithSeverity;
    Dem_DTCSeverityType         dtcSeverityMask;
    Dem_FilterForFDCType        filterForFaultDetectionCounter;
    uint16                      faultIndex;
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
  DEM_INITIALIZED
} Dem_StateType; /** @req DEM169 */


static Dem_StateType demState = DEM_UNINITIALIZED;

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
static Dem_OperationCycleStateType operationCycleStateList[DEM_OPERATION_CYCLE_ID_ENDMARK];

/*
 * Allocation of local event status buffer
 */
static EventStatusRecType   eventStatusBuffer[DEM_MAX_NUMBER_EVENT];

/*
 * Allocation of pre-init event memory (used between pre-init and init). Only one
 * memory regardless of event destination.
 */
/** @req DEM207 */
static FreezeFrameRecType   preInitFreezeFrameBuffer[DEM_MAX_NUMBER_FF_DATA_PRE_INIT];
static ExtDataRecType       preInitExtDataBuffer[DEM_MAX_NUMBER_EXT_DATA_PRE_INIT];

/*
 * Allocation of primary event memory ramlog (after init) in uninitialized memory
 */
/** @req DEM162 */
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
EventRecType                priMemEventBuffer[DEM_MAX_NUMBER_EVENT_PRI_MEM];
FreezeFrameRecType          priMemFreezeFrameBuffer[DEM_MAX_NUMBER_FF_DATA_PRI_MEM];
ExtDataRecType              priMemExtDataBuffer[DEM_MAX_NUMBER_EXT_DATA_PRI_MEM];
#if !defined(CFG_DEM_AGING_PROCESSING_EXTENSION)
HealingRecType              priMemAgingBuffer[DEM_MAX_NUMBER_AGING_PRI_MEM];
#endif
#endif
/*
 * Allocation of secondary event memory ramlog (after init) in uninitialized memory
 */
/** @req DEM162 */
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
EventRecType                secMemEventBuffer[DEM_MAX_NUMBER_EVENT_SEC_MEM];
FreezeFrameRecType          secMemFreezeFrameBuffer[DEM_MAX_NUMBER_FF_DATA_SEC_MEM];
ExtDataRecType              secMemExtDataBuffer[DEM_MAX_NUMBER_EXT_DATA_SEC_MEM];
#if !defined(CFG_DEM_AGING_PROCESSING_EXTENSION)
HealingRecType              secMemAgingBuffer[DEM_MAX_NUMBER_AGING_SEC_MEM];
#endif
#endif




#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
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

typedef struct {
    NvM_BlockIdType blockId;
    Dem_DTCOriginType origin;
    boolean dataModified; /* ture if buffer changed */
} BufferInfo_t;


/* This buffers keep track of the buffer modifications */
#ifdef DEM_USE_MEMORY_FUNCTIONS
static BufferInfo_t FFIsModified[DEM_MAX_NR_OF_EVENT_DESTINATION];

#if !defined(CFG_DEM_AGING_PROCESSING_EXTENSION)
static BufferInfo_t AgingIsModified[DEM_MAX_NR_OF_EVENT_DESTINATION];
#endif

static BufferInfo_t EventIsModified[DEM_MAX_NR_OF_EVENT_DESTINATION];
static BufferInfo_t ExtendedDataIsModified[DEM_MAX_NR_OF_EVENT_DESTINATION];
#endif

#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
static boolean DemEnableConditions[DEM_NUM_ENABLECONDITIONS];
#endif
/*
 * Local functions
 * */
static void getPidData(const Dem_PidOrDidType **const*pidClassPtr, FreezeFrameRecType **freezeFrame, uint16 *storeIndexPtr);
static void getDidData(const Dem_PidOrDidType **const*didClassPtr, FreezeFrameRecType **freezeFrame, uint16 *storeIndexPtr);

#ifdef DEM_USE_MEMORY_FUNCTIONS
static boolean storeOBDFreezeFrameDataMem(const Dem_EventParameterType *eventParam, const FreezeFrameRecType *freezeFrame,
                                          FreezeFrameRecType* freezeFrameBuffer, uint32 freezeFrameBufferSize,
                                          Dem_DTCOriginType origin);

static boolean storeFreezeFrameDataMem(const Dem_EventParameterType *eventParam, const FreezeFrameRecType *freezeFrame,
                                    FreezeFrameRecType* freezeFrameBuffer, uint32 freezeFrameBufferSize,
                                    Dem_DTCOriginType origin);

static boolean deleteFreezeFrameDataMem(const Dem_EventParameterType *eventParam, Dem_DTCOriginType origin);
static boolean deleteExtendedDataMem(const Dem_EventParameterType *eventParam, Dem_DTCOriginType origin);
#endif

static void getEventFailed(Dem_EventIdType eventId, boolean *eventFailed);

#if !defined(CFG_DEM_AGING_PROCESSING_EXTENSION)
static void deleteAgingRecMem(const Dem_EventParameterType *eventParam, Dem_DTCOriginType origin);
#endif
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
#if (DEM_UNIT_TEST == STD_ON)
void rearrangeEventTimeStamp(uint32 *timeStamp);
#else
static void rearrangeEventTimeStamp(uint32 *timeStamp);
#endif
#endif


#if (DEM_UNIT_TEST == STD_ON)
/*
 * Procedure:   zeroPriMemBuffers
 * Description: Fill the primary buffers with zeroes
 */


//lint -save
//lint -e84 //PC-Lint exception, size 0 is OK.
//lint -e957    PC-Lint exception - Used only by DemTest
void demZeroPriMemBuffers(void)
{
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
    memset(priMemEventBuffer, 0, sizeof(priMemEventBuffer));
    memset(priMemFreezeFrameBuffer, 0, sizeof(priMemFreezeFrameBuffer));
    memset(priMemExtDataBuffer, 0, sizeof(priMemExtDataBuffer));
#endif
}

void demZeroSecMemBuffers(void)
{
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
    memset(secMemEventBuffer, 0, sizeof(secMemEventBuffer));
    memset(secMemFreezeFrameBuffer, 0, sizeof(secMemFreezeFrameBuffer));
    memset(secMemExtDataBuffer, 0, sizeof(secMemExtDataBuffer));
#endif
}

#endif

//lint -restore

boolean isOperationCycleStarted(Dem_OperationCycleIdType opCycleId) {
    if( DEM_OPERATION_CYCLE_ID_ENDMARK > opCycleId ) {
        return (DEM_CYCLE_STATE_START == operationCycleStateList[opCycleId]);
    } else {
        return FALSE;
    }
}
#ifdef DEM_USE_MEMORY_FUNCTIONS
static boolean eventIsStoredInMem(Dem_EventIdType eventId, EventRecType* eventBuffer, uint32 eventBufferSize)
{
    boolean eventIdFound = FALSE;

    for (uint16 i = 0; (i < eventBufferSize) && (!eventIdFound); i++) {
        eventIdFound = (eventBuffer[i].eventId == eventId);
    }
    return eventIdFound;
}
#endif


/*
 * Procedure:   checkDtcKind
 * Description: Return TRUE if "dtcKind" match the events DTCKind or "dtcKind"
 *              is "DEM_DTC_KIND_ALL_DTCS" otherwise FALSE.
 */
static boolean checkDtcKind(Dem_DTCKindType dtcKind, const Dem_EventParameterType *eventParam)
{
    boolean result = FALSE;

    if (dtcKind == DEM_DTC_KIND_ALL_DTCS) {
        result = TRUE;
    }
    else {
        if (eventParam->DTCClassRef != NULL) {
            if (eventParam->DTCClassRef->DTCKind == dtcKind) {
                result = TRUE;
            }
        }
    }
    return result;
}


/*
 * Procedure:   checkDtcGroup
 * Description: Return TRUE if "dtc" match the events DTC or "dtc" is
 *              "DEM_DTC_GROUP_ALL_DTCS" otherwise FALSE.
 */
static boolean checkDtcGroup(uint32 dtc, const Dem_EventParameterType *eventParam)
{
    boolean result = FALSE;

    if (dtc == DEM_DTC_GROUP_ALL_DTCS) {
        result = TRUE;
    }
    else {
        if (eventParam->DTCClassRef != NULL) {
            if (eventParam->DTCClassRef->DTC == dtc) {
                result = TRUE;
            }
        }
    }
    return result;
}


/*
 * Procedure:   checkDtcOrigin
 * Description: Return TRUE if "dtcOrigin" match any of the events DTCOrigin otherwise FALSE.
 */
static inline boolean checkDtcOrigin(Dem_DTCOriginType dtcOrigin, const Dem_EventParameterType *eventParam)
{
    return (eventParam->EventClass->EventDestination == dtcOrigin);
}

/*
 * Procedure:   checkDtcSeverityMask
 * Description: Return TRUE if "dtcSeverityMask" match any of the events DTC severity otherwise FALSE.
 */
// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715}      Symbol not referenced
static boolean checkDtcSeverityMask(Dem_DTCSeverityType dtcSeverityMask, const Dem_EventParameterType *eventParam)
{
    boolean result = TRUE;

    (void)dtcSeverityMask;
    (void)eventParam;

    // TODO: This function is optional, may be implemented here.

    return result;
}


/*
 * Procedure:   checkDtcFaultDetectionCounterMask
 * Description: TBD.
 */
// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715}      Symbol not referenced
static boolean checkDtcFaultDetectionCounter(const Dem_EventParameterType *eventParam)
{
    boolean result = TRUE;

    (void)eventParam;
    // TODO: Not implemented yet.

    return result;
}


/*
 * Procedure:   lookupEventStatusRec
 * Description: Returns the pointer to event id parameters of "eventId" in "*eventStatusBuffer",
 *              if not found NULL is returned.
 */
void lookupEventStatusRec(Dem_EventIdType eventId, EventStatusRecType **const eventStatusRec)
{
    uint8 i;
    boolean eventIdFound = FALSE;

    for (i = 0; (i < DEM_MAX_NUMBER_EVENT) && (!eventIdFound); i++) {
        eventIdFound = (eventStatusBuffer[i].eventId == eventId);
    }

    if (eventIdFound) {
        *eventStatusRec = &eventStatusBuffer[i-1];
    } else {
        *eventStatusRec = NULL;
    }
}

/*
 * Procedure:   lookupEventIdParameter
 * Description: Returns the pointer to event id parameters of "eventId" in "*eventIdParam",
 *              if not found NULL is returned.
 */
static void lookupEventIdParameter(Dem_EventIdType eventId, const Dem_EventParameterType **const eventIdParam)
{
    const Dem_EventParameterType *EventIdParamList = configSet->EventParameter;

    /* Lookup the correct event id parameters */
    uint16 i=0;
    while ((EventIdParamList[i].EventID != eventId) && (!EventIdParamList[i].Arc_EOL)) {
        i++;
    }

    if (!EventIdParamList[i].Arc_EOL) {
        *eventIdParam = &EventIdParamList[i];
    } else {
        *eventIdParam = NULL;
    }
}
/*
 * Procedure:   checkEntryValid
 * Description: Returns whether event id "eventId" is a valid entry in primary memory
 */
#ifdef DEM_USE_MEMORY_FUNCTIONS
static boolean checkEntryValid(Dem_EventIdType eventId, Dem_DTCOriginType origin){
    const Dem_EventParameterType *EventIdParamList = configSet->EventParameter;
    boolean isValid = FALSE;
    uint16 i=0;
    while ((EventIdParamList[i].EventID != eventId) && (!EventIdParamList[i].Arc_EOL)) {
        i++;
    }

    if (!EventIdParamList[i].Arc_EOL) {
        // Event was found
        // Event should be stored in destination memory?
        isValid = checkDtcOrigin(origin, &EventIdParamList[i]);
    } else {
        // The event did not exist
    }
    return isValid;
}
#endif
/*
 * Procedure:   preDebounceNone
 * Description: Returns the result of the debouncing.
 */
static Dem_EventStatusType preDebounceNone(const Dem_EventStatusType reportedStatus, const EventStatusRecType* statusRecord) {
    Dem_EventStatusType returnCode;
    (void)statusRecord;     // Just to get rid of PC-Lint warnings

    switch (reportedStatus) {
    case DEM_EVENT_STATUS_FAILED:
    case DEM_EVENT_STATUS_PASSED:
        // Already debounced, do nothing.
        break;

    default:
        // TODO: What to do with PREFAIL and PREPASSED on no debouncing?
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_PREDEBOUNCE_NONE_ID, DEM_E_PARAM_DATA);
        break;
    }

    returnCode = reportedStatus;
    return returnCode;
}

static sint8 fdcInternalToUDSFdc(sint16 fdcInternal, const Dem_PreDebounceCounterBasedType* pdVars)
{
    /* Map the internal counter to the corresponding UDS fdc. I.e. map from [FailedThreshold, PassedThreshold] to [-128, 127]. */
    sint32 pdRange = (sint32)(pdVars->FailedThreshold - pdVars->PassedThreshold);
    sint32 temp = DEM_UDS_FDC_RANGE*((sint32)((sint32)fdcInternal - (sint32)pdVars->PassedThreshold)) + (DEM_UDS_TEST_PASSED_TRESHOLD*pdRange);
    return (sint8)(temp/pdRange);
}

static sint8 getDefaultUDSFdc(Dem_EventIdType eventId)
{
    sint8 udsFdc = 0;
    const Dem_EventParameterType *eventParam = NULL;
    lookupEventIdParameter(eventId, &eventParam);
    if( NULL != eventParam ) {
        if (eventParam->EventClass->PreDebounceAlgorithmClass != NULL) {
            switch (eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceName) { /** @req DEM342 */
            case DEM_PRE_DEBOUNCE_COUNTER_BASED:
                udsFdc = fdcInternalToUDSFdc(0, eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceAlgorithm.PreDebounceCounterBased);
                break;
            default:
                break;
            }
        }
    }
    return udsFdc;
}
/*
 * Procedure:   preDebounceCounterBased
 * Description: Returns the result of the debouncing.
 */
static Dem_EventStatusType preDebounceCounterBased(Dem_EventStatusType reportedStatus, EventStatusRecType* statusRecord) {
    Dem_EventStatusType returnCode;
    const Dem_PreDebounceCounterBasedType* pdVars = statusRecord->eventParamRef->EventClass->PreDebounceAlgorithmClass->PreDebounceAlgorithm.PreDebounceCounterBased;

    switch (reportedStatus) {
    case DEM_EVENT_STATUS_PREFAILED:
        if (statusRecord->fdcInternal < pdVars->FailedThreshold) {
            if ((pdVars->JumpUp) && (statusRecord->fdcInternal < pdVars->JumpUpValue)) {
                statusRecord->fdcInternal = pdVars->JumpUpValue;/* @req DEM423 */
            }
                if (((sint32)statusRecord->fdcInternal + (sint32)pdVars->IncrementStepSize) < pdVars->FailedThreshold) {
                    statusRecord->fdcInternal += pdVars->IncrementStepSize;/** @req DEM418 */
                } else {
                    statusRecord->fdcInternal = pdVars->FailedThreshold;
                }
            }
        break;

    case DEM_EVENT_STATUS_PREPASSED:
        if (statusRecord->fdcInternal > pdVars->PassedThreshold) {
            if ((pdVars->JumpDown) && (statusRecord->fdcInternal > pdVars->JumpDownValue)) {
                statusRecord->fdcInternal = pdVars->JumpDownValue;/* @req DEM425 */
            }
                if (((sint32)statusRecord->fdcInternal - (sint32)pdVars->DecrementStepSize) > pdVars->PassedThreshold) {
                    statusRecord->fdcInternal -= pdVars->DecrementStepSize;/** @req DEM419 */
                } else {
                    statusRecord->fdcInternal = pdVars->PassedThreshold;
                }
            }
        break;

    case DEM_EVENT_STATUS_FAILED:
        statusRecord->fdcInternal = pdVars->FailedThreshold; /** @req DEM420 */
        break;

    case DEM_EVENT_STATUS_PASSED:
        statusRecord->fdcInternal = pdVars->PassedThreshold; /** @req DEM421 */
        break;

    default:
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_PREDEBOUNCE_COUNTER_BASED_ID, DEM_E_PARAM_DATA);
        break;

    }

#if defined(USE_DEM_EXTENSION)
    Dem_Extension_PostPreDebounceCounterBased(reportedStatus, statusRecord);
#endif

    if( statusRecord->fdcInternal >= pdVars->FailedThreshold ) {
        returnCode = DEM_EVENT_STATUS_FAILED;
    } else if( statusRecord->fdcInternal <= pdVars->PassedThreshold ) {
        returnCode = DEM_EVENT_STATUS_PASSED;
    } else {
        returnCode = reportedStatus;
    }
    /* @req DEM415 */
    statusRecord->UDSFdc = fdcInternalToUDSFdc(statusRecord->fdcInternal, pdVars);
    statusRecord->maxUDSFdc = MAX(statusRecord->maxUDSFdc, statusRecord->UDSFdc);
    return returnCode;
}
#if !defined(CFG_DEM_CONFIRMATION_PROCESSING_EXTENSION)
static boolean confirmationCycleIsStarted(const Dem_EventParameterType *eventParam)
{
    boolean isStarted = FALSE;
    if (eventParam->EventClass->ConfirmationCycleRef < DEM_OPERATION_CYCLE_ID_ENDMARK) {
        if (operationCycleStateList[eventParam->EventClass->ConfirmationCycleRef] == DEM_CYCLE_STATE_START) {
            isStarted = TRUE;
        }
    }
    return isStarted;
}

static boolean faultConfirmationCriteriaFulfilled(const Dem_EventParameterType *eventParam, const EventStatusRecType *eventStatusRecPtr)
{
    if( eventStatusRecPtr->confirmationCounter >= eventParam->EventClass->ConfirmationCycleCounterThreshold ) {
        return TRUE;
    } else {
        return FALSE;
    }
}
static void handleFaultConfirmation(const Dem_EventParameterType *eventParam, EventStatusRecType *eventStatusRecPtr)
{
    /* It is assumed that this function is only called when eventstatusExtended bit DEM_EVENT_STATUS_FAILED
     * changes from 0 to 1*/
    if( confirmationCycleIsStarted(eventParam) && !eventStatusRecPtr->confCntrUpdatedThisConfCycle ) {
        if( eventStatusRecPtr->confirmationCounter < DEM_CONFIRMATION_CNTR_MAX ) {
            eventStatusRecPtr->confirmationCounter++;
        }

        /* @req DEM530 */
        if( faultConfirmationCriteriaFulfilled(eventParam, eventStatusRecPtr )) {
            eventStatusRecPtr->eventStatusExtended |= DEM_CONFIRMED_DTC;
        }
        eventStatusRecPtr->confCntrUpdatedThisConfCycle = TRUE;
    }
}
#endif
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
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

static void notifyEventStatusChange(const Dem_EventParameterType *eventParam, Dem_EventStatusExtendedType oldStatus, Dem_EventStatusExtendedType newStatus)
{
    uint8 j = 0;
    if( (NULL != eventParam) && (NULL != eventParam->CallbackEventStatusChanged)) {
        /* DEM016 */
        while( !eventParam->CallbackEventStatusChanged[j].Arc_EOL ) {
            if( eventParam->CallbackEventStatusChanged[j].UsePort ) {
                eventParam->CallbackEventStatusChanged[j].CallbackEventStatusChangedFnc.eventStatusChangedWithoutId(oldStatus, newStatus);
            } else {
                eventParam->CallbackEventStatusChanged[j].CallbackEventStatusChangedFnc.eventStatusChangedWithId(eventParam->EventID, oldStatus, newStatus);
            }
            j++;
        }
    }
}

static void notifyEventDataChanged(const Dem_EventParameterType *eventParam) {
    if( (NULL != eventParam) && (NULL != eventParam->CallbackEventDataChanged)) {
        if( eventParam->CallbackEventDataChanged->UsePort ) {
            eventParam->CallbackEventDataChanged->CallbackEventDataChangedFnc.eventDataChangedWithoutId();
        } else {
            eventParam->CallbackEventDataChanged->CallbackEventDataChangedFnc.eventDataChangedWithId(eventParam->EventID);
        }
    }
}

void setDefaultEventStatus(EventStatusRecType *eventStatusRecPtr)
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
#if !defined(CFG_DEM_CONFIRMATION_PROCESSING_EXTENSION)
    eventStatusRecPtr->confirmationCounter = 0;
    eventStatusRecPtr->confCntrUpdatedThisConfCycle = FALSE;
#endif
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
    eventStatusRecPtr->timeStamp = 0;
#endif
}
/*
 * Procedure:   updateEventStatusRec
 * Description: Update the status of "eventId", if not exist and "createIfNotExist" is
 *              true a new record is created
 */
static void updateEventStatusRec(const Dem_EventParameterType *eventParam, Dem_EventStatusType eventStatus, boolean createIfNotExist, EventStatusRecType *eventStatusRec)
{
    EventStatusRecType *eventStatusRecPtr;

    lookupEventStatusRec(eventParam->EventID, &eventStatusRecPtr);

    if ((eventStatusRecPtr == NULL) && (createIfNotExist)) {
        lookupEventStatusRec(DEM_EVENT_ID_NULL, &eventStatusRecPtr);
        if (eventStatusRecPtr != NULL) {
            setDefaultEventStatus(eventStatusRecPtr);
            eventStatusRecPtr->eventId = eventParam->EventID;
            eventStatusRecPtr->eventParamRef = eventParam;
        } else {
            DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_UPDATE_EVENT_STATUS_ID, DEM_E_EVENT_STATUS_BUFF_FULL);
        }
    }

    if (eventStatusRecPtr != NULL) {

        if (eventParam->EventClass->PreDebounceAlgorithmClass != NULL) {
            switch (eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceName) { /** @req DEM342 */
            case DEM_NO_PRE_DEBOUNCE:
                eventStatus = preDebounceNone(eventStatus, eventStatusRecPtr);
                break;

            case DEM_PRE_DEBOUNCE_COUNTER_BASED:
                eventStatus = preDebounceCounterBased(eventStatus, eventStatusRecPtr);
                break;

            default:
                DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_UPDATE_EVENT_STATUS_ID, DEM_E_NOT_IMPLEMENTED_YET);
                break;
            }
        }

        eventStatusRecPtr->errorStatusChanged = FALSE;
        eventStatusRecPtr->extensionDataChanged = FALSE;
        eventStatusRecPtr->extensionDataStoreBitfield = 0;

#if defined(USE_DEM_EXTENSION)
        Dem_EventStatusExtendedType eventStatusExtendedBeforeUpdate = eventStatusRecPtr->eventStatusExtended;
#endif

        if (eventStatus == DEM_EVENT_STATUS_FAILED) {
            if (!(eventStatusRecPtr->eventStatusExtended & DEM_TEST_FAILED)) {
                eventStatusRecPtr->occurrence++;
                eventStatusRecPtr->errorStatusChanged = TRUE;
#if !defined(CFG_DEM_CONFIRMATION_PROCESSING_EXTENSION)
                /* Handle fault confirmation */
                handleFaultConfirmation(eventParam, eventStatusRecPtr);
#endif
            }
            /** @req DEM036 */ /** @req DEM379.PendingSet */
            eventStatusRecPtr->eventStatusExtended |= (DEM_TEST_FAILED | DEM_TEST_FAILED_THIS_OPERATION_CYCLE | DEM_TEST_FAILED_SINCE_LAST_CLEAR | DEM_PENDING_DTC);
            eventStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)~(DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR | DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE);
            //TODO: Warning indicator light problem review
            if ((eventParam->DTCClassRef != NULL) && (eventParam->DTCClassRef->DTCKind == DEM_DTC_KIND_EMISSION_REL_DTCS)) {
                    eventStatusRecPtr->eventStatusExtended |= DEM_WARNING_INDICATOR_REQUESTED;
            }
        }

        if (eventStatus == DEM_EVENT_STATUS_PASSED) {
            if (eventStatusRecPtr->eventStatusExtended & (DEM_TEST_FAILED | DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE)) {
                eventStatusRecPtr->errorStatusChanged = TRUE;
            }
            /** @req DEM036 */
            eventStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)~DEM_TEST_FAILED;
            eventStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)~(DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR | DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE);
            //TODO: Warning indicator light problem review
            if ((eventParam->DTCClassRef != NULL) && (eventParam->DTCClassRef->DTCKind == DEM_DTC_KIND_EMISSION_REL_DTCS)) {
                eventStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)~DEM_WARNING_INDICATOR_REQUESTED;
            }
        }

#if defined(USE_DEM_EXTENSION)
        Dem_Extension_UpdateEventstatus(eventStatusRecPtr, eventStatusExtendedBeforeUpdate, eventStatus);
#endif

        eventStatusRecPtr->maxUDSFdc = MAX(eventStatusRecPtr->maxUDSFdc, eventStatusRecPtr->UDSFdc);
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
        if( eventStatusRecPtr->errorStatusChanged && (eventStatus == DEM_EVENT_STATUS_FAILED) ) {
            /* Test just failed. Need to set timestamp */
            setEventTimeStamp(eventStatusRecPtr);
        }
#endif
        memcpy(eventStatusRec, eventStatusRecPtr, sizeof(EventStatusRecType));
    }
    else {
        setDefaultEventStatus(eventStatusRec);
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
    lookupEventStatusRec(eventRec->eventId, &eventStatusRecPtr);
    lookupEventIdParameter(eventRec->eventId, &eventParam);

    if (eventStatusRecPtr != NULL) {
        // Update occurrence counter.
        eventStatusRecPtr->occurrence += eventRec->occurrence;
        // Merge event status extended with stored
        // TEST_FAILED_SINCE_LAST_CLEAR should be set if set if set in either
        eventStatusRecPtr->eventStatusExtended |= (Dem_EventStatusExtendedType)(eventRec->eventStatusExtended & DEM_TEST_FAILED_SINCE_LAST_CLEAR);
        // DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR should cleared if cleared in either
        if((eventRec->eventStatusExtended & eventStatusRecPtr->eventStatusExtended & DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR) == 0u) {
            eventStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)~(Dem_EventStatusExtendedType)DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR;
        }

        // DEM_PENDING_DTC and DEM_CONFIRMED_DTC should be set if set in either
        eventStatusRecPtr->eventStatusExtended |= (Dem_EventStatusExtendedType)(eventRec->eventStatusExtended & (DEM_PENDING_DTC | DEM_CONFIRMED_DTC));
        // DEM_WARNING_INDICATOR_REQUESTED should be set if set in either
        eventStatusRecPtr->eventStatusExtended |= (Dem_EventStatusExtendedType)(eventRec->eventStatusExtended & DEM_WARNING_INDICATOR_REQUESTED);
#if !defined(CFG_DEM_CONFIRMATION_PROCESSING_EXTENSION)
        // Update confirmation counter
        if( (DEM_CONFIRMATION_CNTR_MAX - eventRec->confirmationCounter) < eventStatusRecPtr->confirmationCounter) {
            /* Would overflow */
            eventStatusRecPtr->confirmationCounter = DEM_CONFIRMATION_CNTR_MAX;
        } else {
            eventStatusRecPtr->confirmationCounter += eventRec->confirmationCounter;
        }

        if( (NULL != eventParam) && faultConfirmationCriteriaFulfilled(eventParam, eventStatusRecPtr) ) {
            eventStatusRecPtr->eventStatusExtended |= (Dem_EventStatusExtendedType)DEM_CONFIRMED_DTC;
        }
#endif

#if (DEM_TEST_FAILED_STORAGE == STD_ON)
        /* @req DEM387 */
        /* @req DEM388 */
        /* @req DEM525 */
        if( eventStatusRecPtr->eventStatusExtended & DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE ) {
            /* Test has not been completed this operation cycle. Set test failed bit as in stored */
            eventStatusRecPtr->eventStatusExtended |= (eventRec->eventStatusExtended & DEM_TEST_FAILED);
        }
#endif

#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
        if( !(eventStatusRecPtr->eventStatusExtended & DEM_TEST_FAILED_THIS_OPERATION_CYCLE) ) {
            /* Test has not failed this operation cycle. Means that the that the timestamp
             * should be set to the one read from NvRam */
            eventStatusRecPtr->timeStamp = eventRec->timeStamp;
        }
#endif

        if( (eventStatusRecPtr->occurrence != eventRec->occurrence) ||
#if !defined(CFG_DEM_CONFIRMATION_PROCESSING_EXTENSION)
            (eventStatusRecPtr->confirmationCounter != eventRec->confirmationCounter) ||
#endif
            (GET_STORED_STATUS_BITS(eventStatusRecPtr->eventStatusExtended) != GET_STORED_STATUS_BITS(eventRec->eventStatusExtended)) ) {
            statusChanged = TRUE;
        }

        if( 0 == (eventStatusRecPtr->eventStatusExtended & DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE) ) {
            /* Test was completed during preInit, means that the eventStatus was changed in some way */
            Dem_EventStatusExtendedType oldStatus = (Dem_EventStatusExtendedType)(GET_STORED_STATUS_BITS(eventRec->eventStatusExtended));
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
        sint8 startUdsFdc = getDefaultUDSFdc(eventParam->EventID);
        eventStatusRecPtr->UDSFdc = startUdsFdc;
        eventStatusRecPtr->maxUDSFdc = startUdsFdc;
        eventStatusRecPtr->fdcInternal = 0;
        eventStatusRecPtr->eventStatusExtended = DEM_DEFAULT_EVENT_STATUS;
        eventStatusRecPtr->errorStatusChanged = FALSE;
        eventStatusRecPtr->occurrence = 0;
#if !defined(CFG_DEM_CONFIRMATION_PROCESSING_EXTENSION)
        eventStatusRecPtr->confirmationCounter = 0;
        eventStatusRecPtr->confCntrUpdatedThisConfCycle = FALSE;
#endif
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
        eventStatusRecPtr->timeStamp = 0;
#endif
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

static void getInternalElement( const Dem_EventParameterType *eventParameter, Dem_InternalDataElementType elementType, uint8_t* buf )
{
    EventStatusRecType eventStatusRec;

    getEventStatusRec(eventParameter->EventID, &eventStatusRec);

    if( DEM_EVENT_ID_NULL != eventStatusRec.eventId ) {
        if( DEM_OCCCTR == elementType ) {
            buf[0] = eventStatusRec.occurrence >> 8;
            buf[1] = eventStatusRec.occurrence;
        }
        else if( DEM_FAULTDETCTR == elementType ) {
            buf[0] = eventStatusRec.UDSFdc;
        }
        else if( DEM_MAXFAULTDETCTR == elementType ) {
            buf[0] = eventStatusRec.maxUDSFdc;
#if !defined(CFG_DEM_CONFIRMATION_PROCESSING_EXTENSION)
        } else if( DEM_CONFIRMATIONCNTR == elementType ) {
            buf[0] = eventStatusRec.confirmationCounter;
#endif
        } else {
#if defined(USE_DEM_EXTENSION)
            Dem_Extension_GetExtendedDataInternalElement(eventParameter->EventID, elementType, buf);
#endif
        }
    }
}

/*
 * Procedure:   lookupDtcEvent
 * Description: Returns TRUE if the DTC was found and "eventStatusRec" points
 *              to the event record found.
 */
static boolean lookupDtcEvent(uint32 dtc, EventStatusRecType **eventStatusRec)
{
    boolean dtcFound = FALSE;
    uint16 i;

    *eventStatusRec = NULL;

    for (i = 0; (i < DEM_MAX_NUMBER_EVENT) && (!dtcFound); i++) {
        if (eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL) {
            if (eventStatusBuffer[i].eventParamRef->DTCClassRef != NULL) {

                // Check DTC
                if (eventStatusBuffer[i].eventParamRef->DTCClassRef->DTC == dtc) {
                    *eventStatusRec = &eventStatusBuffer[i];
                    dtcFound = TRUE;
                }
            }
        }
    }

    return dtcFound;
}


/*
 * Procedure:   matchEventWithDtcFilter
 * Description: Returns TRUE if the event pointed by "event" fulfill
 *              the "dtcFilter" global filter settings.
 */
static boolean matchEventWithDtcFilter(const EventStatusRecType *eventRec)
{
    boolean dtcMatch = FALSE;

    // Check status
    if ((dtcFilter.dtcStatusMask == DEM_DTC_STATUS_MASK_ALL) || (eventRec->eventStatusExtended & dtcFilter.dtcStatusMask)) {
        if (eventRec->eventParamRef != NULL) {

            // Check dtcKind
            if (checkDtcKind(dtcFilter.dtcKind, eventRec->eventParamRef)) {

                // Check dtcOrigin
                if (checkDtcOrigin(dtcFilter.dtcOrigin, eventRec->eventParamRef)) {

                    // Check severity
                    if ((dtcFilter.filterWithSeverity == DEM_FILTER_WITH_SEVERITY_NO)
                        || ((dtcFilter.filterWithSeverity == DEM_FILTER_WITH_SEVERITY_YES) && (checkDtcSeverityMask(dtcFilter.dtcSeverityMask, eventRec->eventParamRef)))) {

                        // Check fault detection counter
                        if ((dtcFilter.filterForFaultDetectionCounter == DEM_FILTER_FOR_FDC_NO)
                            || ((dtcFilter.filterWithSeverity == DEM_FILTER_FOR_FDC_YES) && (checkDtcFaultDetectionCounter(eventRec->eventParamRef)))) {
                            dtcMatch = TRUE;
                        }
                    }
                }
            }
        }
    }

    return dtcMatch;
}

#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON)
#if !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
static uint8 getEventPriority(Dem_EventIdType eventId)
{
    uint8 priority = 0xFF;

    uint16 index = 0;
    const Dem_EventParameterType *eventIdParamList = configSet->EventParameter;
    while( !eventIdParamList[index].Arc_EOL ) {
        if( eventIdParamList[index].EventID == eventId ) {
            priority = eventIdParamList[index].EventClass->EventPriority;
            break;
        }
        index++;
    }

    return priority;
}

static Std_ReturnType getFFEventForDisplacement(const Dem_EventParameterType *eventParam, const FreezeFrameRecType *ffBuffer, uint16 bufferSize, Dem_EventIdType *eventToRemove)
{
    /* See figure 25 in ASR 4.0.3 */
    /* @req DEM403 */
    /* @req DEM404 */
    /* @req DEM405 */
    /* @req DEM406 */
    /* TODO: How do we handle the case when it is an OBD freeze frame that should be stored?
     * Should they be handled in the same way as "normal" freeze frames? */
    Std_ReturnType ret = E_NOT_OK;
    uint16 removeCandidateIndex = 0;
    uint32 oldestPassive_TimeStamp = DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT;
    uint32 oldestActive_TimeStamp = DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT;
    uint8 eventPrio = 0xFF;
    boolean eventFailed = FALSE;
    boolean passiveCandidateFound = FALSE;
    boolean activeCandidateFound = FALSE;
    for( uint16 i = 0; i < bufferSize; i++ ) {
        if( (DEM_EVENT_ID_NULL != ffBuffer[i].eventId) && (eventParam->EventID != ffBuffer[i].eventId) && (DEM_FREEZE_FRAME_OBD != ffBuffer[i].kind) ) {
            /* Check if priority if the event is higher or equal (low numerical value of priority means high priority..) */
            eventPrio = getEventPriority(ffBuffer[i].eventId);
            getEventFailed(ffBuffer[i].eventId, &eventFailed);
            if( (eventParam->EventClass->EventPriority <= eventPrio) && !eventFailed && (oldestPassive_TimeStamp > ffBuffer[i].timeStamp)) {
                /* This event has lower or equal priority to the reported event, it is passive
                 * and it is the oldest currently found. A candidate for removal. */
                oldestPassive_TimeStamp = ffBuffer[i].timeStamp;
                removeCandidateIndex = i;
                passiveCandidateFound = TRUE;

            }
            if( !passiveCandidateFound ) {
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

    if( passiveCandidateFound || activeCandidateFound) {
        *eventToRemove = ffBuffer[removeCandidateIndex].eventId;
        ret = E_OK;
    }
    return ret;

}

static Std_ReturnType getExtDataEventForDisplacement(const Dem_EventParameterType *eventParam, const ExtDataRecType *extDataBuffer, uint16 bufferSize, Dem_EventIdType *eventToRemove)
{
    /* See figure 25 in ASR 4.0.3 */
    /* @req DEM403 */
    /* @req DEM404 */
    /* @req DEM405 */
    /* @req DEM406 */
    Std_ReturnType ret = E_NOT_OK;
    uint16 removeCandidateIndex = 0;
    uint32 oldestPassive_TimeStamp = DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT;
    uint32 oldestActive_TimeStamp = DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT;
    uint8 eventPrio = 0xFF;
    boolean eventFailed = FALSE;
    boolean passiveCandidateFound = FALSE;
    boolean activeCandidateFound = FALSE;
    for( uint16 i = 0; i < bufferSize; i++ ) {
        if( DEM_EVENT_ID_NULL != extDataBuffer[i].eventId ) {
            /* Check if priority if the event is higher or equal (low numerical value of priority means high priority..) */
            eventPrio = getEventPriority(extDataBuffer[i].eventId);
            getEventFailed(extDataBuffer[i].eventId, &eventFailed);
            if( (eventParam->EventClass->EventPriority <= eventPrio) && !eventFailed && (oldestPassive_TimeStamp > extDataBuffer[i].timeStamp)) {
                /* This event has lower or equal priority to the reported event, it is passive
                 * and it is the oldest currently found. A candidate for removal. */
                oldestPassive_TimeStamp = extDataBuffer[i].timeStamp;
                removeCandidateIndex = i;
                passiveCandidateFound = TRUE;

            }
            if( !passiveCandidateFound ) {
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

    if( passiveCandidateFound || activeCandidateFound) {
        *eventToRemove = extDataBuffer[removeCandidateIndex].eventId;
        ret = E_OK;
    }
    return ret;
}
#ifdef DEM_USE_MEMORY_FUNCTIONS
static Std_ReturnType getEventForDisplacement(const Dem_EventParameterType *eventParam, const EventRecType *eventBuffer,
                                              uint16 bufferSize, Dem_EventIdType *eventToRemove)
{
    /* See figure 25 in ASR 4.0.3 */
    /* @req DEM403 */
    /* @req DEM404 */
    /* @req DEM405 */
    /* @req DEM406 */
    Std_ReturnType ret = E_NOT_OK;
    uint16 removeCandidateIndex = 0;
    uint32 oldestPassive_TimeStamp = DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT;
    uint32 oldestActive_TimeStamp = DEM_MAX_TIMESTAMP_FOR_REARRANGEMENT;
    uint8 eventPrio = 0xFF;
    boolean eventFailed = FALSE;
    boolean passiveCandidateFound = FALSE;
    boolean activeCandidateFound = FALSE;
    for( uint16 i = 0; i < bufferSize; i++ ) {
        if( DEM_EVENT_ID_NULL != eventBuffer[i].eventId ) {
            /* Check if priority if the event is higher or equal (low numerical value of priority means high priority..) */
            eventPrio = getEventPriority(eventBuffer[i].eventId);
            getEventFailed(eventBuffer[i].eventId, &eventFailed);
            eventFailed = ( 0 != (DEM_TEST_FAILED & eventBuffer[i].eventStatusExtended));
            if( (eventParam->EventClass->EventPriority <= eventPrio) && !eventFailed && (oldestPassive_TimeStamp > eventBuffer[i].timeStamp)) {
                /* This event has lower or equal priority to the reported event, it is passive
                 * and it is the oldest currently found. A candidate for removal. */
                oldestPassive_TimeStamp = eventBuffer[i].timeStamp;
                removeCandidateIndex = i;
                passiveCandidateFound = TRUE;

            }
            if( !passiveCandidateFound ) {
                /* Currently, a passive event with lower or equal priority has not been found.
                 * Check if the priority is less than for the reported event. Store the oldest. */
                if( (eventParam->EventClass->EventPriority < eventPrio) &&  (oldestActive_TimeStamp > eventBuffer[i].timeStamp) ) {
                    oldestActive_TimeStamp = eventBuffer[i].timeStamp;
                    removeCandidateIndex = i;
                    activeCandidateFound = TRUE;
                }
            }
        }
    }

    if( passiveCandidateFound || activeCandidateFound) {
        *eventToRemove = eventBuffer[removeCandidateIndex].eventId;
        ret = E_OK;
    }
    return ret;
}
#endif
/*
 * Functions for rearranging timestamps
 * */
#if (DEM_UNIT_TEST == STD_ON)
void rearrangeFreezeFrameTimeStamp(uint32 *timeStamp)
#else
static void rearrangeFreezeFrameTimeStamp(uint32 *timeStamp )
#endif
{
    FreezeFrameRecType temp;
    uint32 i = 0;
    uint32 j = 0;
    uint32 k = 0;
    uint32 bufferIndex = 0;

    /* These two arrays are looped below must have the same size */
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
    uint32 ffBufferSizes[2] = {DEM_MAX_NUMBER_FF_DATA_PRI_MEM, DEM_MAX_NUMBER_FF_DATA_SEC_MEM};
    FreezeFrameRecType* ffBuffers[2] = {priMemFreezeFrameBuffer, secMemFreezeFrameBuffer};
    uint32 nofSupportedDestinations = 2;
#elif (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
    uint32 ffBufferSizes[1] = {DEM_MAX_NUMBER_FF_DATA_PRI_MEM};
    FreezeFrameRecType* ffBuffers[1] = {priMemFreezeFrameBuffer};
    uint32 nofSupportedDestinations = 1;
#elif (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
    uint32 ffBufferSizes[1] = {DEM_MAX_NUMBER_FF_DATA_SEC_MEM};
    FreezeFrameRecType* ffBuffers[1] = {secMemFreezeFrameBuffer};
    uint32 nofSupportedDestinations = 1;
#else
    uint32 ffBufferSizes[1] = {0};
    FreezeFrameRecType* ffBuffers[1] = {NULL};
    uint32 nofSupportedDestinations = 0;
#endif

    for (bufferIndex=0;bufferIndex<nofSupportedDestinations ;bufferIndex++) {

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
#if (DEM_UNIT_TEST == STD_ON)
void rearrangeExtDataTimeStamp(uint32 *timeStamp)
#else
static void rearrangeExtDataTimeStamp(uint32 *timeStamp)
#endif
{
    ExtDataRecType temp;
    uint32 i = 0;
    uint32 j = 0;
    uint32 k = 0;
    uint32 bufferIndex = 0;

    /* These two arrays are looped below must have the same size */
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON) && (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
    uint32 extBufferSizes[2] = {DEM_MAX_NUMBER_EXT_DATA_PRI_MEM, DEM_MAX_NUMBER_EXT_DATA_SEC_MEM};
    ExtDataRecType* extBuffers[2] = {priMemExtDataBuffer, secMemExtDataBuffer};
    uint32 nofDestinations = 2;
#elif (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
    uint32 extBufferSizes[1] = {DEM_MAX_NUMBER_EXT_DATA_PRI_MEM};
    ExtDataRecType* extBuffers[1] = {priMemExtDataBuffer};
    uint32 nofDestinations = 1;
#elif (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
    uint32 extBufferSizes[1] = {DEM_MAX_NUMBER_EXT_DATA_SEC_MEM};
    ExtDataRecType* extBuffers[1] = {secMemExtDataBuffer};
    uint32 nofDestinations = 1;
#else
    uint32 extBufferSizes[1] = {0};
    ExtDataRecType* extBuffers[1] = {NULL};
    uint32 nofDestinations = 0;
#endif

    for (bufferIndex=0;bufferIndex<nofDestinations ;bufferIndex++) {

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


#if (DEM_UNIT_TEST == STD_ON)
void rearrangeEventTimeStamp(uint32 *timeStamp)
#else
static void rearrangeEventTimeStamp(uint32 *timeStamp)
#endif
{
    FreezeFrameRecType temp;
    uint32 bufferIndex = 0;
    uint32 i = 0;
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

    for (bufferIndex=0;bufferIndex<nofDestinations ;bufferIndex++) {

        EventRecType* eventBuffer = eventBuffers[bufferIndex];
        uint32 eventBufferSize = eventBufferSizes[bufferIndex];

        /* Bubble sort:rearrange event buffer from little to big */
        for( i = 0; i < eventBufferSize; i++ ){
            if( DEM_EVENT_ID_NULL != eventBuffer[i].eventId ){
                for( j = (eventBufferSize - 1); j > i; j-- ) {
                    if( DEM_EVENT_ID_NULL != eventBuffer[j].eventId ) {
                        if( eventBuffer[i].timeStamp > eventBuffer[j].timeStamp ) {
                            //exchange buffer data
                            memcpy(&temp, &eventBuffer[i], sizeof(EventRecType));
                            memcpy(&eventBuffer[i], &eventBuffer[j], sizeof(EventRecType));
                            memcpy(&eventBuffer[j], &temp, sizeof(EventRecType));
                        }
                    }
                }
                eventBuffer[i].timeStamp = k++;
            }
        }
    }

    /* update the current timeStamp */
    *timeStamp = k;
}

/*
 * Functions for initializing timestamps
 * */
#ifdef DEM_USE_MEMORY_FUNCTIONS
static void initCurrentFreezeFrameTimeStamp(uint32 *timeStampPtr)
{
    uint16 i = 0;
    uint32 highestTimeStamp = 0;

    /* Rearrange freeze frames */
    rearrangeFreezeFrameTimeStamp(timeStampPtr);

    for (i = 0; i<DEM_MAX_NUMBER_FF_DATA_PRE_INIT; i++){
        if(preInitFreezeFrameBuffer[i].eventId != DEM_EVENT_ID_NULL){
            preInitFreezeFrameBuffer[i].timeStamp += *timeStampPtr;
            if( preInitFreezeFrameBuffer[i].timeStamp > highestTimeStamp ) {
                highestTimeStamp = preInitFreezeFrameBuffer[i].timeStamp;
            }
        }
    }
    *timeStampPtr = highestTimeStamp + 1;
}

static void initCurrentExtDataTimeStamp(uint32 *timeStampPtr)
{
    uint16 i = 0;
    uint32 highestTimeStamp = 0;

    /* Rearrange extended data in primary memory */
    rearrangeExtDataTimeStamp(timeStampPtr);

    /* Increment the timestamps in the pre init ext data buffer */
    for (i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRE_INIT; i++){
        if( DEM_EVENT_ID_NULL != preInitFreezeFrameBuffer[i].eventId ){
            preInitExtDataBuffer[i].timeStamp += *timeStampPtr;
            if( preInitExtDataBuffer[i].timeStamp > highestTimeStamp ) {
                highestTimeStamp = preInitExtDataBuffer[i].timeStamp;
            }
        }
    }
    *timeStampPtr = highestTimeStamp + 1;
}

static void initCurrentEventTimeStamp(uint32 *timeStampPtr)
{
    uint16 i = 0;
    uint32 highestTimeStamp = 0;

    /* Rearrange events */
    rearrangeEventTimeStamp(timeStampPtr);

    for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++){
        if( DEM_EVENT_ID_NULL != eventStatusBuffer[i].eventId ){
            eventStatusBuffer[i].timeStamp += *timeStampPtr;
            if( eventStatusBuffer[i].timeStamp > highestTimeStamp ) {
                highestTimeStamp = eventStatusBuffer[i].timeStamp;
            }
        }
    }
    *timeStampPtr = highestTimeStamp + 1;
}
#endif
#endif /* DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION */

static boolean lookupFreezeFrameForDisplacementPreInit(const Dem_EventParameterType *eventParam, FreezeFrameRecType **freezeFrame)
{
    boolean freezeFrameFound = FALSE;
    Dem_EventIdType eventToRemove = DEM_EVENT_ID_NULL;

#if defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
    Dem_Extension_GetFFEventForDisplacement(eventParam, preInitFreezeFrameBuffer, DEM_MAX_NUMBER_FF_DATA_PRE_INIT, &eventToRemove);
#else
    if( E_OK != getFFEventForDisplacement(eventParam, preInitFreezeFrameBuffer, DEM_MAX_NUMBER_FF_DATA_PRE_INIT, &eventToRemove) ) {
        eventToRemove = DEM_EVENT_ID_NULL;
    }
#endif
    if( DEM_EVENT_ID_NULL != eventToRemove ) {
        /* Freeze frame for a less significant event was found.
         * Find the all entries in pre init freeze frame buffer and remove these. */
        for (uint16 index = 0; (index < DEM_MAX_NUMBER_FF_DATA_PRE_INIT); index++) {
            if( preInitFreezeFrameBuffer[index].eventId == eventToRemove ) {
                memset(&preInitFreezeFrameBuffer[index], 0, sizeof(FreezeFrameRecType));
                if( !freezeFrameFound ) {
                    *freezeFrame = &preInitFreezeFrameBuffer[index];
                    freezeFrameFound = TRUE;
                }
            }
        }
#if defined(USE_DEM_EXTENSION)
        if( freezeFrameFound ) {
            Dem_Extension_EventFreezeFrameDataDisplaced(eventToRemove);
        }
#endif
    } else {
        /* Buffer is full and the currently stored data is more significant */
    }

    return freezeFrameFound;
}

#ifdef DEM_USE_MEMORY_FUNCTIONS
static boolean lookupFreezeFrameForDisplacement(const Dem_EventParameterType *eventParam, FreezeFrameRecType **freezeFrame,
                                                FreezeFrameRecType* freezeFrameBuffer, uint32 freezeFrameBufferSize)
{
    boolean freezeFrameFound = FALSE;
    Dem_EventIdType eventToRemove = DEM_EVENT_ID_NULL;
    const Dem_EventParameterType *eventToRemoveParam = NULL;

#if defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
    Dem_Extension_GetFFEventForDisplacement(eventParam, freezeFrameBuffer, freezeFrameBufferSize, &eventToRemove);
#else
    if( E_OK != getFFEventForDisplacement(eventParam, freezeFrameBuffer, freezeFrameBufferSize, &eventToRemove) ) {
        eventToRemove = DEM_EVENT_ID_NULL;
    }
#endif
    if( DEM_EVENT_ID_NULL != eventToRemove ) {
        /* Freeze frame for a less significant event was found.
         * Find the all entries in freeze frame buffer and remove these. */
        for (uint16 index = 0; (index < freezeFrameBufferSize); index++) {
            if( freezeFrameBuffer[index].eventId == eventToRemove ) {
                memset(&freezeFrameBuffer[index], 0, sizeof(FreezeFrameRecType));
                if( !freezeFrameFound ) {
                    *freezeFrame = &freezeFrameBuffer[index];
                    freezeFrameFound = TRUE;
                }
            }
        }
#if defined(USE_DEM_EXTENSION)
        if( freezeFrameFound ) {
            Dem_Extension_EventFreezeFrameDataDisplaced(eventToRemove);
        }
#endif
        if( freezeFrameFound ) {
            lookupEventIdParameter(eventToRemove, &eventToRemoveParam);
            /* @req DEM475 */
            notifyEventDataChanged(eventToRemoveParam);
        }
    } else {
        /* Buffer is full and the currently stored data is more significant */
    }

    return freezeFrameFound;
}
#endif
#endif /* DEM_EVENT_DISPLACEMENT_SUPPORT */

static Std_ReturnType getNofStoredNonOBDFreezeFrames(const Dem_EventParameterType *eventStatusRec, Dem_DTCOriginType origin, uint8 *nofStored)
{
    uint8 nofFound = 0;
    FreezeFrameRecType *ffBufferPtr = NULL;
    uint8 bufferSize = 0;
    if( DEM_INITIALIZED == demState ) {
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
        if (origin == DEM_DTC_ORIGIN_PRIMARY_MEMORY) {
            ffBufferPtr = &priMemFreezeFrameBuffer[0];
            bufferSize = DEM_MAX_NUMBER_FF_DATA_PRI_MEM;
        }
#endif
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
        if (origin == DEM_DTC_ORIGIN_SECONDARY_MEMORY) {
            ffBufferPtr = &secMemFreezeFrameBuffer[0];
            bufferSize = DEM_MAX_NUMBER_FF_DATA_SEC_MEM;
        }
#endif

    } else {
        ffBufferPtr = &preInitFreezeFrameBuffer[0];
        bufferSize = DEM_MAX_NUMBER_FF_DATA_PRE_INIT;
    }
    for( uint8 i = 0; (i < bufferSize) && (ffBufferPtr != NULL); i++) {
        if((ffBufferPtr[i].eventId == eventStatusRec->EventID) && (DEM_FREEZE_FRAME_NON_OBD == ffBufferPtr[i].kind)) {
            nofFound++;
        }
    }
    *nofStored = nofFound;
    return E_OK;
}

static Std_ReturnType getNextFFRecordNumber(const Dem_EventParameterType *eventParam, uint8 *recNum, Dem_FreezeFrameKindType ffKind, Dem_DTCOriginType origin)
{
    Std_ReturnType ret = E_OK;
    uint8 nofStored = 0;
    if( DEM_FREEZE_FRAME_NON_OBD == ffKind) {
        if( 0 != eventParam->MaxNumberFreezeFrameRecords ) {
            if( E_OK == getNofStoredNonOBDFreezeFrames(eventParam, origin, &nofStored) ) {
                /* Was ok! */
                if( nofStored < eventParam->MaxNumberFreezeFrameRecords ) {
                    *recNum = eventParam->FreezeFrameRecNumClassRef->FreezeFrameRecordNumber[nofStored];
                } else {
                    *recNum = eventParam->FreezeFrameRecNumClassRef->FreezeFrameRecordNumber[eventParam->MaxNumberFreezeFrameRecords - 1];
                }
            }
        } else {
            /* No freeze frames should be stored for this event */
            ret = E_NOT_OK;
        }
    } else {
        /* Always record 0 for OBD freeze frames */
        *recNum = 0;
    }
    return ret;
}

#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
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
/*
 * Procedure:   getFreezeFrameData
 * Description: get FF data according configuration
 */
static void getFreezeFrameData(const Dem_EventParameterType *eventParam,
                               FreezeFrameRecType *freezeFrame,
                               Dem_EventStatusType eventStatus,
                               EventStatusRecType *eventStatusRec,
                               Dem_FreezeFrameKindType ffKind,
                               Dem_DTCOriginType origin)
{
    uint16 storeIndex = 0;
    const Dem_FreezeFrameClassType *FreezeFrameLocal = NULL;

    (void)eventStatus; /* Avoid compiler warning - can this be removed */
    (void)eventStatusRec; /* Avoid compiler warning - can this be removed */
    /* clear FF data record */
    memset(freezeFrame, 0, sizeof(FreezeFrameRecType ));

    /* Find out the corresponding FF class */
    if( (DEM_FREEZE_FRAME_NON_OBD == ffKind) && (eventParam->FreezeFrameClassRef != NULL) ) {
        FreezeFrameLocal = eventParam->FreezeFrameClassRef;
    } else if((DEM_FREEZE_FRAME_OBD == ffKind) && (NULL != configSet->GlobalOBDFreezeFrameClassRef)) {
        FreezeFrameLocal = configSet->GlobalOBDFreezeFrameClassRef;
    }

    /* get the dids */
    if(FreezeFrameLocal != NULL){
        if(FreezeFrameLocal->FFIdClassRef != NULL){
            if( DEM_FREEZE_FRAME_NON_OBD == ffKind ) {
                getDidData(&FreezeFrameLocal->FFIdClassRef, &freezeFrame, &storeIndex);
            } else if(DEM_FREEZE_FRAME_OBD == ffKind) {
                /* Get the pids */
                getPidData(&FreezeFrameLocal->FFIdClassRef, &freezeFrame, &storeIndex);
            }
        }
    }else{
        /* create an empty FF */
        freezeFrame->eventId = DEM_EVENT_ID_NULL;
    }

    uint8 recNum = 0;
    /* Check if any data has been stored and that there is a record number */
    if ( (storeIndex != 0) && (E_OK == getNextFFRecordNumber(eventParam, &recNum, ffKind, origin))) {
        freezeFrame->eventId = eventParam->EventID;
        freezeFrame->dataSize = storeIndex;
        freezeFrame->recordNumber = recNum;
        freezeFrame->kind = ffKind;
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
        setFreezeFrameTimeStamp(freezeFrame);
#endif
    }
    else{
        freezeFrame->eventId = DEM_EVENT_ID_NULL;
        freezeFrame->dataSize = storeIndex;
    }
}

#if defined(DEM_FREEZE_FRAME_CAPTURE_EXTENSION)
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
static void storeFreezeFrameDataPreInit(const Dem_EventParameterType *eventParam, const EventStatusRecType *eventStatusRec, const FreezeFrameRecType *freezeFrame)
{
    boolean eventIdFound = FALSE;
    boolean eventIdFreePositionFound=FALSE;
    uint16 i;
    (void)eventStatusRec; /* Avoid compiler warning - can this be removed */
    /* Check if already stored */
    for (i = 0; (i<DEM_MAX_NUMBER_FF_DATA_PRE_INIT) && (!eventIdFound); i++) {
        if( DEM_FREEZE_FRAME_NON_OBD == freezeFrame->kind ) {
            eventIdFound = ( (preInitFreezeFrameBuffer[i].eventId == eventParam->EventID) && (preInitFreezeFrameBuffer[i].recordNumber == freezeFrame->recordNumber));
        } else {
            eventIdFound = ((DEM_EVENT_ID_NULL != preInitFreezeFrameBuffer[i].eventId) && (DEM_FREEZE_FRAME_OBD == preInitFreezeFrameBuffer[i].kind));
        }
    }

    if(eventIdFound){
        /* Entry found. Overwrite if not an OBD freeze frame*/
        if( DEM_FREEZE_FRAME_NON_OBD == preInitFreezeFrameBuffer[i-1].kind ) {
            /* overwrite existing */
            memcpy(&preInitFreezeFrameBuffer[i-1], freezeFrame, sizeof(FreezeFrameRecType));
        }
    } else{
        /* lookup first free position */
        for (i = 0; (i<DEM_MAX_NUMBER_FF_DATA_PRE_INIT) && (!eventIdFreePositionFound); i++){
            if(preInitFreezeFrameBuffer[i].eventId == DEM_EVENT_ID_NULL){
                eventIdFreePositionFound=TRUE;
            }
        }

        if (eventIdFreePositionFound) {
            memcpy(&preInitFreezeFrameBuffer[i-1], freezeFrame, sizeof(FreezeFrameRecType));
        } else {
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON)
            /* @req DEM400 */
            /* do displacement */
            FreezeFrameRecType *freezeFrameLocal = NULL;
            if(lookupFreezeFrameForDisplacementPreInit(eventParam, &freezeFrameLocal)){
                memcpy(freezeFrameLocal, freezeFrame, sizeof(FreezeFrameRecType));
            }
#else
            /* @req DEM402*/ /* Req is not the Det-error.. */
            DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_STORE_FF_DATA_PRE_INIT_ID, DEM_E_PRE_INIT_FF_DATA_BUFF_FULL);
#endif
        }
    }

}

#ifdef DEM_USE_MEMORY_FUNCTIONS
static boolean isValidRecordNumber(const Dem_EventParameterType *eventParam, uint8 recordNumber)
{
    boolean isValid = FALSE;
    if( NULL != eventParam ) {
        for( uint8 i = 0; (i < eventParam->MaxNumberFreezeFrameRecords) && !isValid; i++ ) {
            if( eventParam->FreezeFrameRecNumClassRef->FreezeFrameRecordNumber[i] == recordNumber ) {
                isValid = TRUE;
            }
        }
    }
    return isValid;
}

static boolean transferNonOBDFreezeFramesEvtMem(Dem_EventIdType eventId, FreezeFrameRecType* freezeFrameBuffer, uint32 freezeFrameBufferSize,
                                                boolean* eventHandled, Dem_DTCOriginType origin, boolean removeOldFFRecords)
{
    /* Note: Don't touch the OBD freeze frame */
    uint16 nofStoredMemory = 0;
    uint16 nofStoredPreInit = 0;
    sint16 nofFFToMove;
    const Dem_EventParameterType *eventParam;
    uint8 recordToFind;
    uint16 findRecordStartIndex;
    uint16 setRecordStartIndex;
    boolean memoryChanged = FALSE;
    boolean dataUpdated = FALSE;
    lookupEventIdParameter(eventId, &eventParam);


    /* Count the number of entries in destination memory for the event */
    for( uint16 j = 0; j < freezeFrameBufferSize; j++ ) {
        if( (eventId == freezeFrameBuffer[j].eventId) && (DEM_FREEZE_FRAME_NON_OBD == freezeFrameBuffer[j].kind)) {

            if( isValidRecordNumber(eventParam, freezeFrameBuffer[j].recordNumber) ) {
                if( removeOldFFRecords) {
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
    if( eventParam->MaxNumberFreezeFrameRecords == nofStoredMemory ) {
        /* All records already stored in primary memory. Just update the last record */
        nofFFToMove = 1;
        findRecordStartIndex = nofStoredPreInit - 1;
        setRecordStartIndex = eventParam->MaxNumberFreezeFrameRecords - 1;
    } else {
        nofFFToMove = MIN(nofStoredPreInit, (eventParam->MaxNumberFreezeFrameRecords - nofStoredMemory));
        findRecordStartIndex = nofStoredPreInit - nofFFToMove;
        setRecordStartIndex = nofStoredMemory;
    }

    if( 0 < nofFFToMove) {
        boolean ffStored = FALSE;
        for( uint16 offset = 0; offset < (uint16)nofFFToMove; offset++ ) {
            recordToFind = eventParam->FreezeFrameRecNumClassRef->FreezeFrameRecordNumber[findRecordStartIndex + offset];
            ffStored = FALSE;
            for( uint16 index = 0; (index < DEM_MAX_NUMBER_FF_DATA_PRE_INIT) && !ffStored; index++ ) {
                if( (preInitFreezeFrameBuffer[index].eventId == eventId) && (preInitFreezeFrameBuffer[index].recordNumber == recordToFind) &&
                        (DEM_FREEZE_FRAME_NON_OBD == preInitFreezeFrameBuffer[index].kind) && (!eventHandled[index]) ) {
                    /* Found the record to update */
                    preInitFreezeFrameBuffer[index].recordNumber =
                        eventParam->FreezeFrameRecNumClassRef->FreezeFrameRecordNumber[setRecordStartIndex + offset];
                    /* Store the freeze frame */
                    if( storeFreezeFrameDataMem(eventParam, &preInitFreezeFrameBuffer[index], freezeFrameBuffer, freezeFrameBufferSize, origin) ) {
                        dataUpdated = TRUE;
                    }
                    /* Clear the event id in the preInit buffer */
                    eventHandled[index] = TRUE;
                    ffStored = TRUE;
                }
            }
        }

        if( nofFFToMove < nofStoredPreInit ) {
            /* Did not move all freeze frames from the preInit buffer.
             * Need to clear the remaining */
            for( uint16 index = 0; index < DEM_MAX_NUMBER_FF_DATA_PRE_INIT; index++ ) {
                if( (preInitFreezeFrameBuffer[index].eventId == eventId) && (DEM_FREEZE_FRAME_NON_OBD == preInitFreezeFrameBuffer[index].kind) ) {
                    eventHandled[index] = TRUE;
                }
            }
        }
    }
    if( dataUpdated ) {
        /* Use errorStatusChanged in eventsStatusBuffer to signal that the event data was updated */
        EventStatusRecType *eventStatusRecPtr;
        lookupEventStatusRec(eventParam->EventID, &eventStatusRecPtr);
        if( NULL != eventStatusRecPtr ) {
            eventStatusRecPtr->errorStatusChanged = TRUE;
        }
    }
    return memoryChanged;
}

static void transferOBDFreezeFramesEvtMem(FreezeFrameRecType *freezeFrame, FreezeFrameRecType* freezeFrameBuffer,
                                          uint32 freezeFrameBufferSize, Dem_DTCOriginType origin)
{
    const Dem_EventParameterType *eventParam;
    lookupEventIdParameter(freezeFrame->eventId, &eventParam);

    if (origin != DEM_DTC_ORIGIN_PRIMARY_MEMORY) {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GLOBAL_ID, DEM_E_OBD_NOT_ALLOWED_IN_SEC_MEM);
    }
    else if( NULL != eventParam ) {
        /* Assuming that this function will not store the OBD freeze frame
         * if there already is one stored. */
        if( storeOBDFreezeFrameDataMem(eventParam, freezeFrame, freezeFrameBuffer, freezeFrameBufferSize, origin) ) {
            /* Use errorStatusChanged in eventsStatusBuffer to signal that the event data was updated */
            EventStatusRecType *eventStatusRecPtr;
            lookupEventStatusRec(eventParam->EventID, &eventStatusRecPtr);
            if( NULL != eventStatusRecPtr ) {
                eventStatusRecPtr->errorStatusChanged = TRUE;
            }
        }
    }
}

static boolean transferPreInitFreezeFramesEvtMem(FreezeFrameRecType* freezeFrameBuffer, uint32 freezeFrameBufferSize, Dem_DTCOriginType origin)
{
    boolean priMemChanged = FALSE;

    boolean eventHandled[DEM_MAX_NUMBER_FF_DATA_PRE_INIT] = {FALSE};


    for( uint16 i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRE_INIT; i++ ) {
        if( (DEM_EVENT_ID_NULL != preInitFreezeFrameBuffer[i].eventId) && checkEntryValid(preInitFreezeFrameBuffer[i].eventId, origin) ) {
            boolean removeOldFFRecords = FALSE;
#if defined(USE_DEM_EXTENSION)
            Dem_Extension_PreTransferPreInitFreezeFrames(preInitFreezeFrameBuffer[i].eventId, &removeOldFFRecords, origin);
#endif
            if( DEM_FREEZE_FRAME_NON_OBD == preInitFreezeFrameBuffer[i].kind ) {
                if (transferNonOBDFreezeFramesEvtMem(preInitFreezeFrameBuffer[i].eventId,
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
    return priMemChanged;
}

static Std_ReturnType setNvMBlockChanged(NvM_BlockIdType blockId) {

#if defined(USE_NVM) && (DEM_USE_NVM == STD_ON)
    NvM_RequestResultType requestResult = NVM_REQ_PENDING;
    Std_ReturnType ret = E_OK;
    if( 0 != blockId ) {
        ret = NvM_GetErrorStatus(blockId, &requestResult);
        if((E_OK == ret) && (requestResult != NVM_REQ_PENDING) ) {
#if (NVM_SET_RAM_BLOCK_STATUS_API == STD_ON)
            (void)NvM_SetRamBlockStatus( blockId, TRUE );
#endif
        } else {
            ret = E_NOT_OK;
        }
    }
    return ret;

#else
    (void)blockId;
    return E_OK;
#endif
}



static void setEventBlockChanged(Dem_DTCOriginType origin)
{
    uint32 i;
    BufferInfo_t* bufferInfo = NULL;

    for (i=0;i<DEM_MAX_NR_OF_EVENT_DESTINATION;i++) {
        if (EventIsModified[i].origin == origin) {
            bufferInfo = &EventIsModified[i];
            break;
        }
    }

    if (bufferInfo == NULL) {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GLOBAL_ID, DEM_E_UNEXPECTED_EXECUTION);
        return;
    }

    if( E_OK == setNvMBlockChanged(bufferInfo->blockId) ) {
        bufferInfo->dataModified = FALSE;
    } else {
        bufferInfo->dataModified = TRUE;
    }
}

static void setFreezeFrameBlockChanged(Dem_DTCOriginType origin)
{
    uint32 i;
    BufferInfo_t* bufferInfo = NULL;

    for (i=0;i<DEM_MAX_NR_OF_EVENT_DESTINATION;i++) {
        if (FFIsModified[i].origin == origin) {
            bufferInfo = &FFIsModified[i];
            break;
        }
    }

    if (bufferInfo == NULL) {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GLOBAL_ID, DEM_E_UNEXPECTED_EXECUTION);
        return;
    }

    if( E_OK == setNvMBlockChanged(bufferInfo->blockId) ) {
        bufferInfo->dataModified = FALSE;
    } else {
        bufferInfo->dataModified = TRUE;
    }
}

static void setExtendedDataBlockChanged(Dem_DTCOriginType origin)
{

    uint32 i;
    BufferInfo_t* bufferInfo = NULL;

    for (i=0;i<DEM_MAX_NR_OF_EVENT_DESTINATION;i++) {
        if (ExtendedDataIsModified[i].origin == origin) {
            bufferInfo = &ExtendedDataIsModified[i];
            break;
        }
    }

    if (bufferInfo == NULL) {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GLOBAL_ID, DEM_E_UNEXPECTED_EXECUTION);
        return;
    }

    if( E_OK == setNvMBlockChanged(bufferInfo->blockId) ) {
        bufferInfo->dataModified = FALSE;
    } else {
        bufferInfo->dataModified = TRUE;
    }

}

#if !defined(CFG_DEM_AGING_PROCESSING_EXTENSION)
static void setAgingBlockChanged(Dem_DTCOriginType origin)
{
    uint32 i;
    BufferInfo_t* bufferInfo = NULL;

    for (i=0;i<DEM_MAX_NR_OF_EVENT_DESTINATION;i++) {
        if (AgingIsModified[i].origin == origin) {
            bufferInfo = &AgingIsModified[i];
            break;
        }
    }

    if (bufferInfo == NULL) {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GLOBAL_ID, DEM_E_UNEXPECTED_EXECUTION);
        return;
    }

    if( E_OK == setNvMBlockChanged(bufferInfo->blockId) ) {
        bufferInfo->dataModified = FALSE;
    } else {
        bufferInfo->dataModified = TRUE;
    }
}
#endif
#endif /* DEM_USE_MEMORY_FUNCTIONS */

#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
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

#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON)
Std_ReturnType lookupExtDataForDisplacement(const Dem_EventParameterType *eventParam, ExtDataRecType *extDataBufPtr, uint16 bufferSize, ExtDataRecType **extData )
{
    const Dem_EventParameterType *eventToRemoveParam = NULL;
    Std_ReturnType ret = E_NOT_OK;
    /* @req DEM400 */
    Dem_EventIdType eventToRemove = DEM_EVENT_ID_NULL;

#if defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
    Dem_Extension_GetExtDataEventForDisplacement(eventParam, extDataBufPtr, bufferSize, &eventToRemove);
#else
    if( E_OK != getExtDataEventForDisplacement(eventParam, extDataBufPtr, bufferSize, &eventToRemove) ) {
        eventToRemove = DEM_EVENT_ID_NULL;
    }
#endif
    if( DEM_EVENT_ID_NULL != eventToRemove ) {
        /* Extended data for a less significant event was found.
         * Find the entry in ext data buffer. */
        for (uint16 index = 0; (index < bufferSize) && (E_OK != ret); index++) {
            if( extDataBufPtr[index].eventId == eventToRemove ) {
                memset(&extDataBufPtr[index], 0, sizeof(ExtDataRecType));
                *extData = &extDataBufPtr[index];
                ret = E_OK;
#if defined(USE_DEM_EXTENSION)
                Dem_Extension_EventExtendedDataDisplaced(eventToRemove);
#endif
                lookupEventIdParameter(eventToRemove, &eventToRemoveParam);
                /* @req DEM475 */
                notifyEventDataChanged(eventToRemoveParam);
            }
        }
    }
    return ret;
}
#endif





static boolean StoreExtDataInMem( const Dem_EventParameterType *eventParam, ExtDataRecType *extDataMem, uint16 bufferSize, Dem_DTCOriginType origin, boolean overrideOldData) {

    Std_ReturnType callbackReturnCode;
    uint16 i;
    uint16 storeIndex = 0;
    uint16 recordSize;
    const Dem_ExtendedDataRecordClassType *extendedDataRecord;
    ExtDataRecType *extData;
    boolean eventIdFound = FALSE;
    boolean bStoredData = FALSE;

    // Check if already stored
    for (i = 0; (i < bufferSize) && (!eventIdFound); i++){
        eventIdFound = (extDataMem[i].eventId == eventParam->EventID);
        extData = &extDataMem[i];
    }

    if( FALSE == eventIdFound ) {
        extData = NULL;
        for (i = 0; (i < bufferSize) && (NULL == extData); i++){
            if( extDataMem[i].eventId == DEM_EVENT_ID_NULL ) {
                extData = &extDataMem[i];
            }
        }
        if( NULL == extData ) {
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON)
            /* @req DEM400 */
            if( E_OK != lookupExtDataForDisplacement(eventParam, extDataMem, bufferSize, &extData) ) {
                return FALSE;
            }
#else
            /* @req DEM402*//* Displacement supported disabled */
            DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_STORE_EXT_DATA_MEM_ID, DEM_E_MEM_EXT_DATA_BUFF_FULL);
            return FALSE;
#endif /* DEM_EVENT_DISPLACEMENT_SUPPORT */
        }
    }

    // Check if any pointer to extended data class
    if (eventParam->ExtendedDataClassRef != NULL) {
        // Request extended data and copy it to the buffer
        for (i = 0; (i < DEM_MAX_NR_OF_RECORDS_IN_EXTENDED_DATA) && (eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i] != NULL); i++) {
            extendedDataRecord = eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i];
            if( DEM_UPDATE_RECORD_VOLATILE != extendedDataRecord->UpdateRule ) {
                recordSize = extendedDataRecord->DataSize;
                if ((storeIndex + recordSize) <= DEM_MAX_SIZE_EXT_DATA) {
                    if( DEM_UPDATE_RECORD_YES == extendedDataRecord->UpdateRule ||
                            (DEM_UPDATE_RECORD_NO == extendedDataRecord->UpdateRule && (extData->eventId != eventParam->EventID)) ||
                            overrideOldData) {
                        /* Either update rule YES, or update rule is NO and extended data was not previously stored for this event */
                        if( NULL != extendedDataRecord->CallbackGetExtDataRecord ) {
                            callbackReturnCode = extendedDataRecord->CallbackGetExtDataRecord(&extData->data[storeIndex]); /** @req DEM282 */
                            if (callbackReturnCode != E_OK) {
                                // Callback data currently not available, clear space.
                                memset(&extData->data[storeIndex], 0xFF, recordSize);
                            }
                            bStoredData = TRUE;
                        } else if( DEM_NO_ELEMENT != extendedDataRecord->InternalDataElement ) {
                            getInternalElement( eventParam, extendedDataRecord->InternalDataElement, &extData->data[storeIndex] );
                            bStoredData = TRUE;
                        }
                    }
                    storeIndex += recordSize;
                } else {
                    // Error: Size of extended data record is bigger than reserved space.
                    DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GET_EXTENDED_DATA_ID, DEM_E_EXT_DATA_TOO_BIG);
                    break;  // Break the loop
                }
            }
        }
    }

    // Check if any data has been stored
    if (TRUE == bStoredData) {
        extData->eventId = eventParam->EventID;
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
        setExtDataTimeStamp(extData);
#endif

#ifdef DEM_USE_MEMORY_FUNCTIONS
        if( DEM_PREINITIALIZED != demState ) {
            setExtendedDataBlockChanged(origin);
        }
#endif
    }
    return bStoredData;
}


/*
 * Procedure:   getExtendedData
 * Description: Collects the extended data according to "eventParam" and return it in "extData",
 *              if not found eventId is set to DEM_EVENT_ID_NULL.
 */
static boolean storeExtendedData(const Dem_EventParameterType *eventParam, boolean overrideOldData)
{
    boolean ret = FALSE;
    if( DEM_PREINITIALIZED == demState ) {
        (void)StoreExtDataInMem(eventParam, preInitExtDataBuffer, DEM_MAX_NUMBER_EXT_DATA_PRE_INIT, DEM_DTC_ORIGIN_NOT_USED, overrideOldData);
    } else {
        switch (eventParam->EventClass->EventDestination) {
            case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
                ret = StoreExtDataInMem(eventParam, priMemExtDataBuffer, DEM_MAX_NUMBER_EXT_DATA_PRI_MEM, DEM_DTC_ORIGIN_PRIMARY_MEMORY, overrideOldData);
#endif
                break;
            case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
                ret = StoreExtDataInMem(eventParam, secMemExtDataBuffer, DEM_MAX_NUMBER_EXT_DATA_SEC_MEM, DEM_DTC_ORIGIN_SECONDARY_MEMORY, overrideOldData);
#endif
                break;
            case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
            case DEM_DTC_ORIGIN_MIRROR_MEMORY:
                // Not yet supported
                DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GLOBAL_ID, DEM_E_NOT_IMPLEMENTED_YET);
                break;
            default:
                break;
        }
    }
    return ret;
}

#ifdef DEM_USE_MEMORY_FUNCTIONS
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON)
Std_ReturnType lookupEventForDisplacement(const Dem_EventParameterType *eventParam, EventRecType *eventBuffer, uint16 bufferSize,
        EventRecType **priMemEventStatusRec, Dem_DTCOriginType origin)
{
    Std_ReturnType ret = E_NOT_OK;
    Dem_EventIdType eventToRemove = DEM_EVENT_ID_NULL;
    /* No free position found. See if any of the stored events may be removed */
#if defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
    Dem_Extension_GetEventForDisplacement(eventParam, eventBuffer, bufferSize, &eventToRemove);
#else
    if( E_OK != getEventForDisplacement(eventParam, eventBuffer, bufferSize, &eventToRemove) ) {
        eventToRemove = DEM_EVENT_ID_NULL;
    }
#endif
    if( DEM_EVENT_ID_NULL != eventToRemove ) {
        /* a less significant event was found.
         * Find the entry in buffer. */
        const Dem_EventParameterType *removeEventParam = NULL;
        for (uint16 i = 0; (i < bufferSize) && (E_OK != ret); i++) {
            if( eventBuffer[i].eventId == eventToRemove ) {
                memset(&eventBuffer[i], 0, sizeof(EventRecType));
                *priMemEventStatusRec = &eventBuffer[i];
                ret = E_OK;
#if defined(USE_DEM_EXTENSION)
                Dem_Extension_EventDataDisplaced(eventToRemove);
#endif
            }
        }
        EventStatusRecType *eventStatusRecPtr;
        lookupEventStatusRec(eventToRemove, &eventStatusRecPtr);
        if( NULL != eventStatusRecPtr ) {
            Dem_EventStatusExtendedType oldStatus = eventStatusRecPtr->eventStatusExtended;
            /* @req DEM409 */
            eventStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)(~DEM_CONFIRMED_DTC);
#if !defined(CFG_DEM_AGING_PROCESSING_EXTENSION)
            eventStatusRecPtr->confirmationCounter = 0;
            eventStatusRecPtr->confCntrUpdatedThisConfCycle = FALSE;
#endif
            removeEventParam = eventStatusRecPtr->eventParamRef;
            if( oldStatus != eventStatusRecPtr->eventStatusExtended ) {
                /* @req DEM016 */
                notifyEventStatusChange(removeEventParam, oldStatus, eventStatusRecPtr->eventStatusExtended);
            }
        } else {
            lookupEventIdParameter(eventToRemove, &removeEventParam);
        }
        if( NULL != removeEventParam ) {
            /* Remove all event related data */
            /* @req DEM408 */

#if !defined(CFG_DEM_AGING_PROCESSING_EXTENSION)
            deleteAgingRecMem(removeEventParam, origin);
#endif

            if( NULL != removeEventParam->FreezeFrameClassRef ) {
                (void)deleteFreezeFrameDataMem(removeEventParam, origin);
            }
            if( NULL!= removeEventParam->ExtendedDataClassRef ) {
                (void)deleteExtendedDataMem(removeEventParam, origin);
            }
            /* @req DEM475 */
            notifyEventDataChanged(removeEventParam);
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
        EventRecType* buffer, uint32 bufferSize, Dem_DTCOriginType origin)
{
    boolean positionFound = FALSE;
    EventRecType *eventStatusRec = NULL;
    Std_ReturnType ret = E_OK;

    // Lookup event ID
    for (uint16 i = 0; (i < bufferSize) && (!positionFound); i++){
        if( buffer[i].eventId == eventStatus->eventId ) {
            eventStatusRec = &buffer[i];
            positionFound = TRUE;
        }
    }

    if( !positionFound ) {
        /* Event is not already stored, Search for free position */
        for (uint16 i  = 0; (i < bufferSize) && (!positionFound); i++){
            if( buffer[i].eventId == DEM_EVENT_ID_NULL ) {
                eventStatusRec = &buffer[i];
                positionFound = TRUE;
            }
        }

#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON)
        /* @req DEM400 */
        if( !positionFound ) {
            if( E_OK == lookupEventForDisplacement(eventParam, buffer, bufferSize, &eventStatusRec, origin) ) {
                positionFound = TRUE;
            }
        }
#else
        /* @req DEM402*/ /* No displacement should be done */
        (void)*eventParam;  /* Do this to avoid warning */
#endif /* DEM_EVENT_DISPLACEMENT_SUPPORT */
    }

    if ((positionFound) && (NULL != eventStatusRec)) {
        // Update event found
        eventStatusRec->eventId = eventStatus->eventId;
        eventStatusRec->occurrence = eventStatus->occurrence;
        eventStatusRec->eventStatusExtended = eventStatus->eventStatusExtended;
#if !defined(CFG_DEM_CONFIRMATION_PROCESSING_EXTENSION)
        eventStatusRec->confirmationCounter = eventStatus->confirmationCounter;
#endif
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
        eventStatusRec->timeStamp = eventStatus->timeStamp;
#endif
        setEventBlockChanged(origin);
    } else {
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
        /* Error: mem event buffer full */
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_STORE_EVENT_MEM_ID, DEM_E_MEM_EVENT_BUFF_FULL);
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
    uint16 i;

    for (i = 0; (i < eventMemorySize) && (!eventIdFound); i++){
        eventIdFound = (eventMemory[i].eventId == eventParam->EventID);
    }

    if (eventIdFound) {
        memset(&eventMemory[i-1], 0, sizeof(EventRecType));
        setEventBlockChanged(origin);
    }
    return eventIdFound;
}
#endif /* DEM_USE_MEMORY_FUNCTIONS */
/*
 * Procedure:   storeEventEvtMem
 * Description: Store the event data of "eventStatus->eventId" in event memory according to
 *              "eventParam" destination option.
 */
static Std_ReturnType storeEventEvtMem(const Dem_EventParameterType *eventParam, const EventStatusRecType *eventStatus)
{
    Std_ReturnType ret = E_NOT_OK;

    switch (eventParam->EventClass->EventDestination) {
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
            ret = storeEventMem(eventParam, eventStatus, priMemEventBuffer, DEM_MAX_NUMBER_EVENT_ENTRY_PRI, DEM_DTC_ORIGIN_PRIMARY_MEMORY); /** @req DEM010 */
#endif
            break;
        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
            ret = storeEventMem(eventParam, eventStatus, secMemEventBuffer, DEM_MAX_NUMBER_EVENT_ENTRY_SEC, DEM_DTC_ORIGIN_SECONDARY_MEMORY);    /** @req DEM010 */
#endif
            break;
        case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
        case DEM_DTC_ORIGIN_MIRROR_MEMORY:
            // Not yet supported
            DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GLOBAL_ID, DEM_E_NOT_IMPLEMENTED_YET);
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
static void getExtendedDataMem(const Dem_EventParameterType *eventParam, ExtDataRecType ** const extendedData,
                                  ExtDataRecType* extendedDataBuffer, uint32 extendedDataBufferSize) /** @req DEM041 */
{
    boolean eventIdFound = FALSE;
    boolean eventIdFreePositionFound=FALSE;
    uint16 i;

    // Check if already stored
    for (i = 0; (i<extendedDataBufferSize) && (!eventIdFound); i++){
        if( extendedDataBuffer[i].eventId == eventParam->EventID ) {
            *extendedData = &extendedDataBuffer[i];
            eventIdFound = TRUE;
        }
    }

    if (!eventIdFound) {
        // No, lookup first free position
        for (i = 0; (i < extendedDataBufferSize) && (!eventIdFreePositionFound); i++){
            eventIdFreePositionFound =  (extendedDataBuffer[i].eventId == DEM_EVENT_ID_NULL);
        }
        if (eventIdFreePositionFound) {
            *extendedData = &extendedDataBuffer[i-1];
        } else {
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON)
            if(E_OK != lookupExtDataForDisplacement(eventParam, extendedDataBuffer, extendedDataBufferSize, extendedData)) {
                *extendedData = NULL;
            }
#else
                /* Displacement supported disabled */
                /* Error: mem extended data buffer full */
                DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_STORE_EXT_DATA_MEM_ID, DEM_E_MEM_EXT_DATA_BUFF_FULL);

#endif /* DEM_EVENT_DISPLACEMENT_SUPPORT */
        }
    }
}

/*
 * Procedure:   deleteExtendedDataMem
 * Description: Delete the extended data of "eventParam->eventId" from "priMemExtDataBuffer".
 */
static boolean deleteExtendedDataMem(const Dem_EventParameterType *eventParam, Dem_DTCOriginType origin)
{
    boolean eventIdFound = FALSE;
    uint16 i;

    ExtDataRecType* extBuffer = NULL;
    uint32 bufferSize = 0;

    switch (origin) {
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
            extBuffer = priMemExtDataBuffer;
            bufferSize = DEM_MAX_NUMBER_EXT_DATA_PRI_MEM;
#endif
            break;
        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
            extBuffer = secMemExtDataBuffer;
            bufferSize = DEM_MAX_NUMBER_EXT_DATA_SEC_MEM;
#endif
            break;
        case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
        case DEM_DTC_ORIGIN_MIRROR_MEMORY:
            // Not yet supported
            DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GLOBAL_ID, DEM_E_NOT_IMPLEMENTED_YET);
            break;
        default:
            break;
    }

    // Check if already stored
    for (i = 0;(i<bufferSize) && (!eventIdFound); i++){
        //lint -e613
        eventIdFound = (extBuffer[i].eventId == eventParam->EventID);
    }

    if (eventIdFound) {
        // Yes, clear record
        memset(&extBuffer[i-1], 0, sizeof(ExtDataRecType));
        setExtendedDataBlockChanged(origin);
    }
    return eventIdFound;
}

/*
 * Procedure:   storeExtendedDataEvtMem
 * Description: Store the extended data in event memory according to
 *              "eventParam" destination option
 */
static boolean mergeExtendedDataEvtMem(const Dem_EventParameterType *eventParam, const ExtDataRecType *extendedData, ExtDataRecType* extendedDataBuffer,
                                    uint32 extendedDataBufferSize, Dem_DTCOriginType origin, boolean updateAllExtData)
{
    uint16 i;
    const Dem_ExtendedDataRecordClassType *extendedDataRecordClass;
    ExtDataRecType *memExtDataRec = NULL;
    uint16 storeIndex = 0;
    boolean bCopiedData = FALSE;

    if( eventParam->EventClass->EventDestination ==  origin ) {
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
                        if( eventParam->EventID != memExtDataRec->eventId  || updateAllExtData) {
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
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
                memExtDataRec->timeStamp = extendedData->timeStamp;
#endif
                setExtendedDataBlockChanged(origin);
            }
        }
        else {
            /* DET FEL */
        }
    }
    return bCopiedData;
}
#endif /* DEM_USE_MEMORY_FUNCTIONS */


/*
 * Procedure:   lookupExtendedDataRecNumParam
 * Description: Returns TRUE if the requested extended data number was found among the configured records for the event.
 *              "extDataRecClassPtr" returns a pointer to the record class, "posInExtData" returns the position in stored extended data.
 */
static boolean lookupExtendedDataRecNumParam(uint8 extendedDataNumber, const Dem_EventParameterType *eventParam, Dem_ExtendedDataRecordClassType const **extDataRecClassPtr, uint16 *posInExtData)
{
    boolean recNumFound = FALSE;

    if (eventParam->ExtendedDataClassRef != NULL) {
        uint16  byteCnt = 0;
        uint16 i;

        // Request extended data and copy it to the buffer
        for (i = 0; (i < DEM_MAX_NR_OF_RECORDS_IN_EXTENDED_DATA) && (eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i] != NULL) && (!recNumFound); i++) {
            if (eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i]->RecordNumber == extendedDataNumber) {
                *extDataRecClassPtr =  eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i];
                *posInExtData = byteCnt;
                recNumFound = TRUE;
            }
            if(DEM_UPDATE_RECORD_VOLATILE != eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i]->UpdateRule) {
                byteCnt += eventParam->ExtendedDataClassRef->ExtendedDataRecordClassRef[i]->DataSize;
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
    uint16 i;
    ExtDataRecType* extBuffer = NULL;
    uint32 extBufferSize = 0;

    switch (origin) {
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
            extBuffer = priMemExtDataBuffer;
            extBufferSize = DEM_MAX_NUMBER_EXT_DATA_PRI_MEM;
#endif
            break;
        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
            extBuffer = secMemExtDataBuffer;
            extBufferSize = DEM_MAX_NUMBER_EXT_DATA_SEC_MEM;
#endif
            break;
        default:
            break;
    }

    // Lookup corresponding extended data
    for (i = 0; (i < extBufferSize) && (!eventIdFound); i++) {
        eventIdFound = (extBuffer[i].eventId == eventId);
    }

    if (eventIdFound) {
        // Yes, return pointer
        *extData = &extBuffer[i-1];
    }

    return eventIdFound;
}

/*
 * Procedure:   deleteAgingRecMem
 * Description: delete aging record in memory
 */
#if !defined(CFG_DEM_AGING_PROCESSING_EXTENSION)
static void deleteAgingRecMem(const Dem_EventParameterType *eventParam, Dem_DTCOriginType origin)
{
    uint16 i;
    boolean agingRecDeleted = FALSE;
    HealingRecType*  agingBuffer = NULL;
    uint32 agingBufferSize = 0;

    switch (origin) {
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
            agingBuffer = priMemAgingBuffer;
            agingBufferSize = DEM_MAX_NUMBER_AGING_PRI_MEM;
#endif
            break;
        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
            agingBuffer = secMemAgingBuffer;
            agingBufferSize = DEM_MAX_NUMBER_AGING_SEC_MEM;
#endif
            break;
        default:
            break;
    }

    for (i = 0; i<agingBufferSize; i++){
        if (agingBuffer[i].eventId == eventParam->EventID){
            memset(&agingBuffer[i], 0, sizeof(HealingRecType));
            agingRecDeleted = TRUE;
        }
    }

    if( agingRecDeleted ) {
        setAgingBlockChanged(origin);
    }
}
#endif

/*
 * Procedure:   storeFreezeFrameDataMem
 * Description: store FreezeFrame data record in primary memory
 */
#ifdef DEM_USE_MEMORY_FUNCTIONS
static boolean storeFreezeFrameDataMem(const Dem_EventParameterType *eventParam, const FreezeFrameRecType *freezeFrame,
                                       FreezeFrameRecType* freezeFrameBuffer, uint32 freezeFrameBufferSize,
                                       Dem_DTCOriginType  origin)
{
    boolean eventIdFound = FALSE;
    boolean eventIdFreePositionFound=FALSE;
    boolean ffUpdated = FALSE;
    uint16 i;

    /* Check if already stored */
    for (i = 0; (i<freezeFrameBufferSize) && (!eventIdFound); i++){
        eventIdFound = ((freezeFrameBuffer[i].eventId == eventParam->EventID) && (freezeFrameBuffer[i].recordNumber == freezeFrame->recordNumber));
    }

    if (eventIdFound) {
        memcpy(&freezeFrameBuffer[i-1], freezeFrame, sizeof(FreezeFrameRecType));
        ffUpdated = TRUE;
    }
    else {
        for (i = 0; (i < freezeFrameBufferSize) && (!eventIdFreePositionFound); i++){
            eventIdFreePositionFound =  (freezeFrameBuffer[i].eventId == DEM_EVENT_ID_NULL);
        }
        if (eventIdFreePositionFound) {
            memcpy(&freezeFrameBuffer[i-1], freezeFrame, sizeof(FreezeFrameRecType));
            ffUpdated = TRUE;
        } else {
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON)
            /* @req DEM400 */
            FreezeFrameRecType *freezeFrameLocal;
            if( lookupFreezeFrameForDisplacement(eventParam, &freezeFrameLocal, freezeFrameBuffer, freezeFrameBufferSize) ){
                memcpy(freezeFrameLocal, freezeFrame, sizeof(FreezeFrameRecType));
                ffUpdated = TRUE;
            }
#else
            /* @req DEM402*/ /* Req is not the Det-error.. */
            DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_STORE_FF_DATA_PRI_MEM_ID, DEM_E_PRI_MEM_FF_DATA_BUFF_FULL);
#endif
        }
    }

    if( ffUpdated ) {
        setFreezeFrameBlockChanged(origin);
    }
    return ffUpdated;
}

// PC-Lint (715 etc): Remove errors until function is filled.
//lint -e{715}      Symbol not referenced
static boolean deleteFreezeFrameDataMem(const Dem_EventParameterType *eventParam, Dem_DTCOriginType origin)
{
    uint16 i;
    boolean ffDeleted = FALSE;
    FreezeFrameRecType* freezeFrameBuffer = NULL;
    uint32 bufferSize = 0;

    switch (origin) {
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
            freezeFrameBuffer = priMemFreezeFrameBuffer;
            bufferSize = DEM_MAX_NUMBER_FF_DATA_PRI_MEM;
            break;
#endif
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
            freezeFrameBuffer = secMemFreezeFrameBuffer;
            bufferSize = DEM_MAX_NUMBER_FF_DATA_SEC_MEM;
            break;
#endif
        default:
            break;
    }

    for (i = 0; i<bufferSize; i++){
        if (freezeFrameBuffer[i].eventId == eventParam->EventID){
            memset(&freezeFrameBuffer[i], 0, sizeof(FreezeFrameRecType));
            ffDeleted = TRUE;
        }
    }

    if( ffDeleted ) {
        setFreezeFrameBlockChanged(origin);
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
                                       Dem_EventStatusType eventStatus, EventStatusRecType *eventStatusRec,
                                       Dem_FreezeFrameKindType ffKind)
{
    boolean ret = FALSE;
    switch (eventParam->EventClass->EventDestination) {
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
            getFreezeFrameData(eventParam, freezeFrame, eventStatus, eventStatusRec, ffKind, DEM_DTC_ORIGIN_PRIMARY_MEMORY);
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
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
            getFreezeFrameData(eventParam, freezeFrame, eventStatus, eventStatusRec, ffKind, DEM_DTC_ORIGIN_SECONDARY_MEMORY);
            if (freezeFrame->eventId != DEM_EVENT_ID_NULL) {
                if(freezeFrame->kind == DEM_FREEZE_FRAME_OBD){
                    DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GLOBAL_ID, DEM_E_OBD_NOT_ALLOWED_IN_SEC_MEM);
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
            DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GLOBAL_ID, DEM_E_NOT_IMPLEMENTED_YET);
            break;
        default:
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

    if ( (NULL != eventParam->FreezeFrameClassRef) && (NULL != eventParam->FreezeFrameRecNumClassRef)) {
        for( uint8 i = 0; (i < eventParam->MaxNumberFreezeFrameRecords) && !recNumFound; i++ ) {
            if( eventParam->FreezeFrameRecNumClassRef->FreezeFrameRecordNumber[i] == recordNumber ) {
                recNumFound = TRUE;
                *freezeFrameClassPtr =  eventParam->FreezeFrameClassRef;
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
static boolean lookupFreezeFrameDataSize(uint8 recordNumber, Dem_FreezeFrameClassType const  **freezeFrameClassPtr, uint16 *dataSize)
{
    Std_ReturnType callbackReturnCode;
    boolean dataSizeFound = TRUE;
    uint16 dataSizeLocal = 0;
    uint16 i;

    (void)recordNumber; /* Avoid compiler warning - can this be removed */

    if (*freezeFrameClassPtr != NULL) {
        for (i = 0; (i < DEM_MAX_NR_OF_DIDS_IN_FREEZEFRAME_DATA) && ((*freezeFrameClassPtr)->FFIdClassRef[i]->Arc_EOL != TRUE); i++) {
            if((*freezeFrameClassPtr)->FFIdClassRef[i]->DidReadDataLengthFnc != NULL){
                callbackReturnCode = (*freezeFrameClassPtr)->FFIdClassRef[i]->DidReadDataLengthFnc(&dataSizeLocal);
                if(callbackReturnCode != E_OK){
                    return FALSE;
                }
            }
            else{
                dataSizeLocal = (*freezeFrameClassPtr)->FFIdClassRef[i]->PidOrDidSize;
            }

            *dataSize += dataSizeLocal + DEM_DID_IDENTIFIER_SIZE_OF_BYTES;
        }

    }

    return dataSizeFound;
}

/*
 * Procedure:   lookupFreezeFrameDataPriMem
 * Description: Returns TRUE if the requested event id is found, "freezeFrame" points to the found data.
 */
static boolean getFreezeFrameRecord(Dem_EventIdType eventId,uint8 recordNumber, FreezeFrameRecType **freezeFrame, Dem_FreezeFrameClassType const *FFDataRecordClass,
                                    Dem_DTCOriginType dtcOrigin, uint8* destBuffer, uint16*  bufSize, uint16 *FFDataSize)
{

    boolean eventIdFound = FALSE;
    uint16 i;

    FreezeFrameRecType* freezeFrameBuffer = NULL;
    uint32 freezeFrameBufferSize = 0;

    switch (dtcOrigin) {
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
            freezeFrameBuffer = priMemFreezeFrameBuffer;
            freezeFrameBufferSize = DEM_MAX_NUMBER_FF_DATA_PRI_MEM;
#endif
            break;
        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
            freezeFrameBuffer = secMemFreezeFrameBuffer;
            freezeFrameBufferSize = DEM_MAX_NUMBER_FF_DATA_SEC_MEM;
#endif
            break;
        default:
            DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETFREEZEFRAMEDATARECORDBYDTC_ID, DEM_E_NOT_IMPLEMENTED_YET);
            break;
    }

    if (freezeFrameBuffer == NULL) {
        return FALSE;
    }

    for (i = 0; (i < freezeFrameBufferSize) && (!eventIdFound); i++) {
        eventIdFound = ((freezeFrameBuffer[i].eventId == eventId) && (freezeFrameBuffer[i].recordNumber == recordNumber));
    }

    if (!eventIdFound) {
        return FALSE;
    }


    *freezeFrame = &freezeFrameBuffer[i-1];

    destBuffer[0] = recordNumber;
    destBuffer[1] = 0;
    for (uint8 FFIdNumber = 0; FFDataRecordClass->FFIdClassRef[FFIdNumber]->Arc_EOL == FALSE; FFIdNumber++) {
        destBuffer[1]++;
    }

    memcpy(&destBuffer[2], (*freezeFrame)->data, *FFDataSize); /** @req DEM071 */
    *bufSize = *FFDataSize + 2;

    return TRUE;

}

/*
 * Procedure:   handlePreInitEvent
 * Description: Handle the updating of event status and storing of
 *              event related data in preInit buffers.
 */
static void handlePreInitEvent(Dem_EventIdType eventId, Dem_EventStatusType eventStatus)
{
    const Dem_EventParameterType *eventParam;
    EventStatusRecType *eventStatusBeforeUpdate;
    EventStatusRecType eventStatusLocal;
    FreezeFrameRecType freezeFrameLocal;

    lookupEventIdParameter(eventId, &eventParam);
    if (eventParam != NULL) {
        if (eventParam->EventClass->OperationCycleRef < DEM_OPERATION_CYCLE_ID_ENDMARK) {
            if (operationCycleStateList[eventParam->EventClass->OperationCycleRef] == DEM_CYCLE_STATE_START) {
                lookupEventStatusRec(eventParam->EventID, &eventStatusBeforeUpdate);
                if (eventStatus == DEM_EVENT_STATUS_FAILED) {
                    updateEventStatusRec(eventParam, eventStatus, TRUE, &eventStatusLocal);
                }
                else {
                    updateEventStatusRec(eventParam, eventStatus, FALSE, &eventStatusLocal);
                }

                if (eventStatusLocal.errorStatusChanged || eventStatusLocal.extensionDataChanged) {
                    boolean storeExtData = FALSE;
                    boolean storeFFData = FALSE;
                    boolean overrideOldExtData = FALSE;
#if defined(DEM_EXTENDED_DATA_CAPTURE_EVENT_MEMORY_STORAGE)
                    storeExtData = TRUE;
#elif defined(DEM_EXTENDED_DATA_CAPTURE_TESTFAILED)
                    storeExtData = eventStatusLocal.errorStatusChanged && (0 != (eventStatusLocal.eventStatusExtended & DEM_TEST_FAILED));
#elif defined(DEM_EXTENDED_DATA_CAPTURE_EXTENSION)
                    storeExtData = (0 != (eventStatusLocal.extensionDataStoreBitfield & DEM_EXT_STORE_EXT_DATA_BIT));
                    overrideOldExtData = (0 != (eventStatusLocal.extensionDataStoreBitfield & DEM_EXT_CLEAR_BEFORE_STORE_EXT_DATA_BIT));
#endif

#if defined(DEM_FREEZE_FRAME_CAPTURE_EVENT_MEMORY_STORAGE)
                    storeFFData = TRUE;
#elif defined(DEM_FREEZE_FRAME_CAPTURE_TESTFAILED)
                    storeFFData = eventStatusLocal.errorStatusChanged && (0 != (eventStatusLocal.eventStatusExtended & DEM_TEST_FAILED));
#elif defined(DEM_FREEZE_FRAME_CAPTURE_EXTENSION)
                    storeFFData = (0 != (eventStatusLocal.extensionDataStoreBitfield & DEM_EXT_STORE_FF_BIT));
#endif

                    if( storeExtData && (NULL != eventParam->ExtendedDataClassRef) ) {
                        (void)storeExtendedData(eventParam, overrideOldExtData);
                    }

                    if( storeFFData) {
                        if(NULL != eventParam->FreezeFrameClassRef ) {
#if defined(DEM_FREEZE_FRAME_CAPTURE_EXTENSION)
                            /* Allow extension to decide if ffs should be deleted before storing */
                            if( 0 != (eventStatusLocal.extensionDataStoreBitfield & DEM_EXT_CLEAR_BEFORE_STORE_FF_BIT) ) {
                                deleteFreezeFrameDataPreInit(eventParam);
                            }
#endif
                            getFreezeFrameData(eventParam, &freezeFrameLocal, eventStatus, &eventStatusLocal, DEM_FREEZE_FRAME_NON_OBD, DEM_DTC_ORIGIN_NOT_USED);
                            if (freezeFrameLocal.eventId != DEM_EVENT_ID_NULL) {
                                storeFreezeFrameDataPreInit(eventParam, &eventStatusLocal, &freezeFrameLocal);
                            }
                        }
                        if( (NULL != eventParam->DTCClassRef) && (DEM_DTC_KIND_EMISSION_REL_DTCS == eventParam->DTCClassRef->DTCKind) ) {
                            getFreezeFrameData(eventParam, &freezeFrameLocal, eventStatus, &eventStatusLocal, DEM_FREEZE_FRAME_OBD, DEM_DTC_ORIGIN_NOT_USED);
                            if (freezeFrameLocal.eventId != DEM_EVENT_ID_NULL) {
                                storeFreezeFrameDataPreInit(eventParam, &eventStatusLocal, &freezeFrameLocal);
                            }
                        }
                    }
                }
            }
            else {
                // Operation cycle not started
                // TODO: Report error?
            }
        }
        else {
            // Operation cycle not set
            // TODO: Report error?
        }
    }
    else {
        // Event ID not configured
        // TODO: Report error?
    }
}

static boolean enableConditionsSet(const Dem_EventClassType *eventClass)
{
    /* @req DEM449 */
    /* @req DEM450 */
    boolean conditionsSet = TRUE;
#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
    if( NULL != eventClass->EnableConditionGroupRef ) {
        /* Each group must reference at least one enable condition. Or this won't work.. */
        const Dem_EnableConditionGroupType *enableConditionGroupPtr = eventClass->EnableConditionGroupRef;
        for( uint8 i = 0; i < enableConditionGroupPtr->nofEnableConditions; i++ ) {
            if( !DemEnableConditions[enableConditionGroupPtr->EnableCondition[i]->EnableConditionID] ) {
                conditionsSet = FALSE;
            }
        }
    }
#endif
    return conditionsSet;
}

/*
 * Procedure:   handleEvent
 * Description: Handle the updating of event status and storing of
 *              event related data in event memory.
 */
static Std_ReturnType handleEvent(Dem_EventIdType eventId, Dem_EventStatusType eventStatus)
{
    Std_ReturnType returnCode = E_OK;
    const Dem_EventParameterType *eventParam;
    EventStatusRecType *eventStatusBeforeUpdate;
    EventStatusRecType eventStatusLocal;
    FreezeFrameRecType freezeFrameLocal;
    Dem_EventStatusExtendedType oldStatus = DEM_DEFAULT_EVENT_STATUS;

    lookupEventIdParameter(eventId, &eventParam);
    if (eventParam != NULL) {
        if (eventParam->EventClass->OperationCycleRef < DEM_OPERATION_CYCLE_ID_ENDMARK) {
            if (operationCycleStateList[eventParam->EventClass->OperationCycleRef] == DEM_CYCLE_STATE_START) {
                if ((!((disableDtcSetting.settingDisabled) && (checkDtcGroup(disableDtcSetting.dtcGroup, eventParam)) && (checkDtcKind(disableDtcSetting.dtcKind, eventParam))))
                        && (enableConditionsSet(eventParam->EventClass)))  {/* @req DEM447 */
                    lookupEventStatusRec(eventParam->EventID, &eventStatusBeforeUpdate);
                    if(NULL != eventStatusBeforeUpdate) {
                        oldStatus = eventStatusBeforeUpdate->eventStatusExtended;
                    }
                    updateEventStatusRec(eventParam, eventStatus, TRUE, &eventStatusLocal);
                    if((DEM_EVENT_ID_NULL != eventStatusLocal.eventId) && (oldStatus != eventStatusLocal.eventStatusExtended) ) {
                        /* @req DEM016 */
                        notifyEventStatusChange(eventStatusLocal.eventParamRef, oldStatus, eventStatusLocal.eventStatusExtended);
                    }
                    if (eventStatusLocal.errorStatusChanged || eventStatusLocal.extensionDataChanged) {
                        Std_ReturnType eventStoreStatus;
                        eventStoreStatus = storeEventEvtMem(eventParam, &eventStatusLocal); /** @req DEM184 */
                        boolean storeExtData = FALSE;
                        boolean storeFFData = FALSE;
                        boolean overrideOldExtData = FALSE;
                        boolean eventDataUpdated = (E_OK == eventStoreStatus);
#if defined(DEM_EXTENDED_DATA_CAPTURE_EVENT_MEMORY_STORAGE)
                        storeExtData = (E_OK == eventStoreStatus);
#elif defined(DEM_EXTENDED_DATA_CAPTURE_TESTFAILED)
                        storeExtData = eventStatusLocal.errorStatusChanged && (0 != (eventStatusLocal.eventStatusExtended & DEM_TEST_FAILED));
#elif defined(DEM_EXTENDED_DATA_CAPTURE_EXTENSION)
                        overrideOldExtData = (0 != (eventStatusLocal.extensionDataStoreBitfield & DEM_EXT_CLEAR_BEFORE_STORE_EXT_DATA_BIT));
                        storeExtData = (0 != (eventStatusLocal.extensionDataStoreBitfield & DEM_EXT_STORE_EXT_DATA_BIT));
#endif

#if defined(DEM_FREEZE_FRAME_CAPTURE_EVENT_MEMORY_STORAGE)
                        storeFFData = (E_OK == eventStoreStatus);
#elif defined(DEM_FREEZE_FRAME_CAPTURE_TESTFAILED)
                        storeFFData = eventStatusLocal.errorStatusChanged && (0 != (eventStatusLocal.eventStatusExtended & DEM_TEST_FAILED));
#elif defined(DEM_FREEZE_FRAME_CAPTURE_EXTENSION)
                        storeFFData = (0 != (eventStatusLocal.extensionDataStoreBitfield & DEM_EXT_STORE_FF_BIT));
#endif

                        if (storeExtData && (NULL != eventParam->ExtendedDataClassRef)) {
                            if( storeExtendedData(eventParam, overrideOldExtData) ) {
                                eventDataUpdated = TRUE;
                            }
                        }
                        if (storeFFData) {
                            if(NULL != eventParam->FreezeFrameClassRef){
#ifdef DEM_USE_MEMORY_FUNCTIONS
#if defined(DEM_FREEZE_FRAME_CAPTURE_EXTENSION)
                                /* Allow extension to decide if ffs should be deleted before storing */
                                if( 0 != (eventStatusLocal.extensionDataStoreBitfield & DEM_EXT_CLEAR_BEFORE_STORE_FF_BIT) ) {
                                    if( deleteFreezeFrameDataMem(eventParam, eventParam->EventClass->EventDestination) ) {
                                        eventDataUpdated = TRUE;
                                    }
                                }
#endif
#endif

                                if( storeFreezeFrameDataEvtMem(eventParam, &freezeFrameLocal, eventStatus, &eventStatusLocal, DEM_FREEZE_FRAME_NON_OBD) ) { /** @req DEM190 */
                                    eventDataUpdated = TRUE;
                                }
                            }
                            if( (NULL != eventParam->DTCClassRef) && (DEM_DTC_KIND_EMISSION_REL_DTCS == eventParam->DTCClassRef->DTCKind) ) {
                                if( storeFreezeFrameDataEvtMem(eventParam, &freezeFrameLocal, eventStatus, &eventStatusLocal, DEM_FREEZE_FRAME_OBD) ) { /** @req DEM190 */
                                    eventDataUpdated = TRUE;
                                }
                            }
                        } else{
                            // do nothing
                        }
                        if( eventDataUpdated ) {
                            /* @req DEM475 */
                            notifyEventDataChanged(eventParam);
                        }
                    }
                }
            } else {
                // Operation cycle not started
                returnCode = E_NOT_OK;
            }
        } else {
            // Operation cycle not set
            returnCode = E_NOT_OK;
        }
    } else {
        // Event ID not configured
        returnCode = E_NOT_OK;
    }

    return returnCode;
}


/*
 * Procedure:   resetEventStatus
 * Description: Resets the events status of eventId.
 */
static void resetEventStatus(Dem_EventIdType eventId)
{
    EventStatusRecType *eventStatusRecPtr;

    lookupEventStatusRec(eventId, &eventStatusRecPtr);
    if (eventStatusRecPtr != NULL) {
        Dem_EventStatusExtendedType oldStatus = eventStatusRecPtr->eventStatusExtended;
        eventStatusRecPtr->eventStatusExtended &= (Dem_EventStatusExtendedType)~DEM_TEST_FAILED; /** @req DEM187 */
        if( oldStatus != eventStatusRecPtr->eventStatusExtended ) {
            /* @req DEM016 */
            notifyEventStatusChange(eventStatusRecPtr->eventParamRef, oldStatus, eventStatusRecPtr->eventStatusExtended);
        }
        /* TODO: Should we store in "event destination" if DEM_TEST_FAILED_STORAGE == STD_ON) */
#if 0
#if DEM_TEST_FAILED_STORAGE == STD_ON
        if((0 != (oldStatus & DEM_TEST_FAILED)) && (E_OK == storeEventEvtMem(eventStatusRecPtr->eventParamRef, eventStatusRecPtr))) {
            notifyEventDataChanged(eventStatusRecPtr->eventParamRef);
        }
#endif
#endif
    }

}


/*
 * Procedure:   getEventStatus
 * Description: Returns the extended event status bitmask of eventId in "eventStatusExtended".
 */
static void getEventStatus(Dem_EventIdType eventId, Dem_EventStatusExtendedType *eventStatusExtended)
{
    EventStatusRecType eventStatusLocal;

    // Get recorded status
    getEventStatusRec(eventId, &eventStatusLocal);
    if (eventStatusLocal.eventId == eventId) {
        *eventStatusExtended = eventStatusLocal.eventStatusExtended; /** @req DEM051 */
    }
    else {
        // Event Id not found, no report received.
        *eventStatusExtended = DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE | DEM_TEST_NOT_COMPLETED_SINCE_LAST_CLEAR;
    }
}

/*
 * Procedure:   getEventFailed
 * Description: Returns the TRUE or FALSE of "eventId" in "eventFailed" depending on current status.
 */
static void getEventFailed(Dem_EventIdType eventId, boolean *eventFailed)
{
    EventStatusRecType eventStatusLocal;

    // Get recorded status
    getEventStatusRec(eventId, &eventStatusLocal);
    if (eventStatusLocal.eventId == eventId) {
        if (eventStatusLocal.eventStatusExtended & DEM_TEST_FAILED) { /** @req DEM052 */
            *eventFailed = TRUE;
        }
        else {
            *eventFailed = FALSE;
        }
    }
    else {
        // Event Id not found, assume ok.
        *eventFailed = FALSE;
    }
}

/*
 * Procedure:   getEventTested
 * Description: Returns the TRUE or FALSE of "eventId" in "eventTested" depending on
 *              current status the "test not completed this operation cycle" bit.
 */
static void getEventTested(Dem_EventIdType eventId, boolean *eventTested)
{
    EventStatusRecType eventStatusLocal;

    // Get recorded status
    getEventStatusRec(eventId, &eventStatusLocal);
    if (eventStatusLocal.eventId == eventId) {
        if ( !(eventStatusLocal.eventStatusExtended & DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE)) { /** @req DEM053 */
            *eventTested = TRUE;
        }
        else {
            *eventTested = FALSE;
        }
    }
    else {
        // Event Id not found, not tested.
        *eventTested = FALSE;
    }
}


/*
 * Procedure:   getFaultDetectionCounter
 * Description: Returns pre debounce counter of "eventId" in "counter" and return value E_OK if
 *              the counter was available else E_NOT_OK.
 */
static Std_ReturnType getFaultDetectionCounter(Dem_EventIdType eventId, sint8 *counter)
{
    Std_ReturnType returnCode = E_NOT_OK;
    const Dem_EventParameterType *eventParam;

    lookupEventIdParameter(eventId, &eventParam);
    if (eventParam != NULL) {
        if (eventParam->EventClass->PreDebounceAlgorithmClass != NULL) {
            switch (eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceName)
            {
            case DEM_NO_PRE_DEBOUNCE:
                if (eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceAlgorithm.PreDebounceMonitorInternal != NULL) {
                    if (eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceAlgorithm.PreDebounceMonitorInternal->CallbackGetFDCntFnc != NULL) {
                        returnCode = eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceAlgorithm.PreDebounceMonitorInternal->CallbackGetFDCntFnc(counter); /** @req DEM204.None */ /** @req DEM264 */ /** @req DEM265 */
                    }
                }
                break;

            case DEM_PRE_DEBOUNCE_COUNTER_BASED:
                {
                    EventStatusRecType *eventStatusRec;

                    lookupEventStatusRec(eventId, &eventStatusRec);
                    if (eventStatusRec != NULL) {
                        *counter = eventStatusRec->UDSFdc; /** @req DEM204.Counter */
                    } else {
                        *counter = 0;
                    }
                    returnCode = E_OK;
                }
                break;

            case DEM_PRE_DEBOUNCE_FREQUENCY_BASED:
            case DEM_PRE_DEBOUNCE_TIME_BASED:
                DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETFAULTDETECTIONCOUNTER_ID, DEM_E_NOT_IMPLEMENTED_YET);
                break;

            default:
                DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETFAULTDETECTIONCOUNTER_ID, DEM_E_PARAM_DATA);
                break;
            }
        }
    }

    return returnCode;
}
/*
 * Procedure:   lookupAgingRecMem
 * Description: Returns the pointer to event id parameters of "eventId" in "*priMemAgingBuffer",
 *              if not found NULL is returned.
 */
#if !defined(CFG_DEM_AGING_PROCESSING_EXTENSION)
static boolean lookupAgingRecMem(Dem_EventIdType eventId, const HealingRecType **agingRec, Dem_DTCOriginType origin)
{
    uint16 i;
    boolean agingRecFound = FALSE;

    HealingRecType*  agingBuffer = NULL;
    uint32 agingBufferSize = 0;

    switch (origin) {
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
            agingBuffer = priMemAgingBuffer;
            agingBufferSize = DEM_MAX_NUMBER_AGING_PRI_MEM;
#endif
            break;
        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
            agingBuffer = secMemAgingBuffer;
            agingBufferSize = DEM_MAX_NUMBER_AGING_SEC_MEM;
#endif
            break;
        default:
            DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_CLEARDTC_ID, DEM_E_NOT_IMPLEMENTED_YET);
            break;
    }

    for (i = 0; i < agingBufferSize && (!agingRecFound); i++) {
        if(agingBuffer[i].eventId == eventId){
            agingRecFound = TRUE;
        }

    }

    if(agingRecFound){
        *agingRec = &agingBuffer[i-1];
    }
    else{
        *agingRec = NULL;
    }

    return agingRecFound;

}




static boolean ageEvent(EventStatusRecType* evtStatusBuffer,  Dem_DTCOriginType origin) {

    HealingRecType *agingRecLocal = NULL;
    boolean updatedMemory = FALSE;
    boolean agingRecFound = FALSE;


    if((evtStatusBuffer->eventStatusExtended & DEM_CONFIRMED_DTC) &&
       (!(evtStatusBuffer->eventStatusExtended & DEM_TEST_FAILED_THIS_OPERATION_CYCLE)) &&
       (!(evtStatusBuffer->eventStatusExtended & DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE))) {

        agingRecFound = lookupAgingRecMem(evtStatusBuffer->eventId, (const HealingRecType **)(&agingRecLocal), origin);

        if (agingRecFound) {
            agingRecLocal->agingCounter++; /** @req Dem489 */
            /* TODO: Should it be >= instead of > in the expression below? */
            if(agingRecLocal->agingCounter > evtStatusBuffer->eventParamRef->EventClass->HealingCycleCounter) {
                Dem_EventStatusExtendedType oldStatus = evtStatusBuffer->eventStatusExtended;
                deleteAgingRecMem(evtStatusBuffer->eventParamRef, origin);
                evtStatusBuffer->eventStatusExtended &= (Dem_EventStatusExtendedType)(~DEM_CONFIRMED_DTC);
                evtStatusBuffer->eventStatusExtended &= (Dem_EventStatusExtendedType)(~DEM_PENDING_DTC);
                evtStatusBuffer->eventStatusExtended &= (Dem_EventStatusExtendedType)(~DEM_WARNING_INDICATOR_REQUESTED);
#if !defined(CFG_DEM_CONFIRMATION_PROCESSING_EXTENSION)
                evtStatusBuffer->confirmationCounter = 0;
#endif
                if( oldStatus != evtStatusBuffer->eventStatusExtended ) {
                    /* @req DEM016 */
                    notifyEventStatusChange(evtStatusBuffer->eventParamRef, oldStatus, evtStatusBuffer->eventStatusExtended);
                }
                if(E_OK == storeEventEvtMem(evtStatusBuffer->eventParamRef, evtStatusBuffer) ) {
                    /* @req DEM475 */
                    notifyEventDataChanged(evtStatusBuffer->eventParamRef);
                }
#if defined(USE_DEM_EXTENSION)
                Dem_Extension_HealedEvent(evtStatusBuffer->eventId);
#endif
            }
            /* Set the flag,start up the storage of NVRam in main function. */
            updatedMemory = TRUE;

        }
        else{
            /* If it does exist,establish a new record for the corresponding event */
            agingRecFound = lookupAgingRecMem(DEM_EVENT_ID_NULL, (const HealingRecType **)(&agingRecLocal), origin);
            if(agingRecFound){
                agingRecLocal->eventId = evtStatusBuffer->eventId;
                agingRecLocal->agingCounter++;

                updatedMemory = TRUE;
            }
            else{
                /* memory of aging records is full. */
            }
        }
    }
    else {
        /* If the status bit testFailed (bit 0) is set during the operation cycle, the counter shall be reset. */
        if(evtStatusBuffer->eventStatusExtended & DEM_TEST_FAILED_THIS_OPERATION_CYCLE){
            agingRecFound = lookupAgingRecMem(evtStatusBuffer->eventId, (const HealingRecType **)(&agingRecLocal), origin);
            if(agingRecFound){
                if(agingRecLocal->agingCounter){
                    agingRecLocal->agingCounter = 0;
                    updatedMemory = TRUE;
                }
            }
        }
    }

    return updatedMemory;
}


/*
 * Procedure:   handleAging
 * Description: according to the operation state of "operationCycleId" to "cycleState" , handle the aging relatived data
 *              Returns E_OK if operation was successful else E_NOT_OK.
 */
static Std_ReturnType handleAging(Dem_OperationCycleIdType operationCycleId, Dem_OperationCycleStateType cycleState)
{
    uint16 i;
    Std_ReturnType returnCode = E_OK;
    boolean agingUpdatedSecondaryMemory = FALSE;
    boolean agingUpdatedPrimaryMemory = FALSE;

    if (operationCycleId < DEM_OPERATION_CYCLE_ID_ENDMARK) {
        switch (cycleState)
        {
        case DEM_CYCLE_STATE_START:
            break;

        case DEM_CYCLE_STATE_END:/** @req Dem490 */
            for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
                if(eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL){
                    if(eventStatusBuffer[i].eventParamRef != NULL){
                        if(eventStatusBuffer[i].eventParamRef->EventClass != NULL){
                            if((eventStatusBuffer[i].eventParamRef->EventClass->HealingAllowed == TRUE)\
                                && (eventStatusBuffer[i].eventParamRef->EventClass->HealingCycleRef == operationCycleId)){
                                /* Loop all destination memories e.g. primary and secondary */
                                Dem_DTCOriginType origin = eventStatusBuffer[i].eventParamRef->EventClass->EventDestination;
                                if (origin == DEM_DTC_ORIGIN_SECONDARY_MEMORY) {
                                    agingUpdatedSecondaryMemory = ageEvent(&eventStatusBuffer[i], origin);
                                } else if (origin == DEM_DTC_ORIGIN_PRIMARY_MEMORY) {
                                    agingUpdatedPrimaryMemory = ageEvent(&eventStatusBuffer[i], origin);
                                }
                            }
                        }
                    }
                }
            }
            break;
        default:
            DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_SETOPERATIONCYCLESTATE_ID, DEM_E_PARAM_DATA);
            returnCode = E_NOT_OK;
            break;
        }
    }
    else {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_SETOPERATIONCYCLESTATE_ID, DEM_E_PARAM_DATA);
        returnCode = E_NOT_OK;
    }

    if( agingUpdatedPrimaryMemory ) {
        setAgingBlockChanged(DEM_DTC_ORIGIN_PRIMARY_MEMORY);
    }
    if( agingUpdatedSecondaryMemory ) {
        setAgingBlockChanged(DEM_DTC_ORIGIN_SECONDARY_MEMORY);
    }

    return returnCode;

}
#endif
/*
 * Procedure:   setOperationCycleState
 * Description: Change the operation state of "operationCycleId" to "cycleState" and updates stored
 *              event connected to this cycle id.
 *              Returns E_OK if operation was successful else E_NOT_OK.
 */
static Std_ReturnType setOperationCycleState(Dem_OperationCycleIdType operationCycleId, Dem_OperationCycleStateType cycleState) /** @req DEM338 */
{
    uint16 i;
    Dem_EventStatusExtendedType oldStatus;
    Std_ReturnType returnCode = E_OK;

    if (operationCycleId < DEM_OPERATION_CYCLE_ID_ENDMARK) {
        switch (cycleState)
        {
        case DEM_CYCLE_STATE_START:
            operationCycleStateList[operationCycleId] = cycleState;
            // Lookup event ID
            for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
                if( eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL ) {
                    if( eventStatusBuffer[i].eventParamRef->EventClass->OperationCycleRef == operationCycleId ) {
                    oldStatus = eventStatusBuffer[i].eventStatusExtended;
                    eventStatusBuffer[i].eventStatusExtended &= (Dem_EventStatusExtendedType)~DEM_TEST_FAILED_THIS_OPERATION_CYCLE;
                    eventStatusBuffer[i].eventStatusExtended |= DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE;
                    sint8 startFDC = getDefaultUDSFdc(eventStatusBuffer[i].eventId);
                    eventStatusBuffer[i].UDSFdc = startFDC;/* @req DEM344 */
                    eventStatusBuffer[i].maxUDSFdc = startFDC;
                    eventStatusBuffer[i].fdcInternal = 0;

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
#if !defined(CFG_DEM_CONFIRMATION_PROCESSING_EXTENSION)
                if( (eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL) && (eventStatusBuffer[i].eventParamRef->EventClass->ConfirmationCycleRef == operationCycleId) ) {
                    eventStatusBuffer[i].confCntrUpdatedThisConfCycle = FALSE;
                }
#endif
            }
            break;

        case DEM_CYCLE_STATE_END:
            operationCycleStateList[operationCycleId] = cycleState;
            // Lookup event ID
            for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
                boolean storeEvtMem = FALSE;
                if (eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL) {
                    oldStatus = eventStatusBuffer[i].eventStatusExtended;
                    if ((!(eventStatusBuffer[i].eventStatusExtended & DEM_TEST_FAILED_THIS_OPERATION_CYCLE)) && (!(eventStatusBuffer[i].eventStatusExtended & DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE))) {
                        if( eventStatusBuffer[i].eventParamRef->EventClass->OperationCycleRef == operationCycleId ) {
                            eventStatusBuffer[i].eventStatusExtended &= (Dem_EventStatusExtendedType)~DEM_PENDING_DTC;      // Clear pendingDTC bit /** @req DEM379.PendingClear
                            if( oldStatus != eventStatusBuffer[i].eventStatusExtended ) {
                                storeEvtMem = TRUE;
                            }
                        }
#if !defined(CFG_DEM_CONFIRMATION_PROCESSING_EXTENSION)
                        if( eventStatusBuffer[i].eventParamRef->EventClass->ConfirmationCycleRef == operationCycleId ) {
                            /* @dev DEM: Spec. does not say when this counter should be cleared */
                            if( 0 != eventStatusBuffer[i].confirmationCounter ) {
                                eventStatusBuffer[i].confirmationCounter = 0;
                                storeEvtMem = TRUE;
                            }
                        }
#endif
                    }
#if defined(USE_DEM_EXTENSION)
                    Dem_Extension_OperationCycleEnd(operationCycleId, &eventStatusBuffer[i]);
#endif
                    if( oldStatus != eventStatusBuffer[i].eventStatusExtended ) {
                        /* @req DEM016 */
                        notifyEventStatusChange(eventStatusBuffer[i].eventParamRef, oldStatus, eventStatusBuffer[i].eventStatusExtended);
                    }
                    if( storeEvtMem ) {
                        /* Transfer to event memory.  */
                        if( E_OK == storeEventEvtMem(eventStatusBuffer[i].eventParamRef, &eventStatusBuffer[i]) ) {
                            notifyEventDataChanged(eventStatusBuffer[i].eventParamRef);
                        }
                    }
                }
            }
            break;
        default:
            DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_SETOPERATIONCYCLESTATE_ID, DEM_E_PARAM_DATA);
            returnCode = E_NOT_OK;
            break;
        }
    }
    else {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_SETOPERATIONCYCLESTATE_ID, DEM_E_PARAM_DATA);
        returnCode = E_NOT_OK;
        }

    return returnCode;
}


static inline void initEventStatusBuffer(const Dem_EventParameterType *eventIdParamList)
{
    // Insert all supported events into event status buffer
    uint16 index = 0;
    EventStatusRecType *eventStatusRecPtr;
    eventIdParamList = configSet->EventParameter;
    while( !eventIdParamList[index].Arc_EOL ) {
        // Find next free position in event status buffer
        lookupEventStatusRec(DEM_EVENT_ID_NULL, &eventStatusRecPtr);
        if(NULL != eventStatusRecPtr) {
            eventStatusRecPtr->eventId = eventIdParamList[index].EventID;
            eventStatusRecPtr->eventParamRef = &eventIdParamList[index];
            sint8 startUdsFdc = getDefaultUDSFdc(eventIdParamList[index].EventID);
            eventStatusRecPtr->UDSFdc = startUdsFdc;
            eventStatusRecPtr->maxUDSFdc = startUdsFdc;
            eventStatusRecPtr->fdcInternal = 0;
        } else {
            // event status buffer is full
            DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_PREINIT_ID, DEM_E_EVENT_STATUS_BUFF_FULL);
        }
        index++;
    }
}

static inline void initPreInitFreezeFrameBuffer(void)
{
    //lint -save
    //lint -e568 -e685 //PC-Lint exception.
    //lint -e681 //PC-Lint exception to MISRA 14.1: Loop is not entered. This only happens when config variable is zero. Keep as it is for less complex code.
    // Initialize the pre init buffers
    for (uint16 i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRE_INIT; i++) {
        preInitFreezeFrameBuffer[i].eventId = DEM_EVENT_ID_NULL;
        preInitFreezeFrameBuffer[i].dataSize = 0;
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
        preInitFreezeFrameBuffer[i].timeStamp = 0;
#endif
        for (uint16 j = 0; j < DEM_MAX_SIZE_FF_DATA;j++){
            preInitFreezeFrameBuffer[i].data[j] = 0;
        }
    }
    //lint -restore
}

static inline void initPreInitExtDataBuffer(void)
{
    for (uint16 i = 0; i < DEM_MAX_NUMBER_EXT_DATA_PRE_INIT; i++) {
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
        preInitExtDataBuffer[i].timeStamp = 0;
#endif
        preInitExtDataBuffer[i].eventId = DEM_EVENT_ID_NULL;
        for (uint16 j = 0; j < DEM_MAX_SIZE_EXT_DATA;j++){
            preInitExtDataBuffer[i].data[j] = 0;
        }
    }
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
#if !defined(CFG_DEM_AGING_PROCESSING_EXTENSION)
static boolean validateAgingBuffer(HealingRecType* agingBuffer, uint32 agingBufferSize, Dem_DTCOriginType origin)
{
    boolean agingBlockChanged = FALSE;
    // Validate aging records stored in primary memory
    for (uint16 i = 0; i < agingBufferSize; i++){
        if ((agingBuffer[i].eventId == DEM_EVENT_ID_NULL) || (FALSE == checkEntryValid(agingBuffer[i].eventId, origin))) {
            // Unlegal record, clear the record
            memset(&agingBuffer[i], 0, sizeof(HealingRecType));
            agingBlockChanged = TRUE;
        }
    }
    return agingBlockChanged;
}
#endif




static boolean validateFreezeFrames(FreezeFrameRecType* freezeFrameBuffer, uint32 freezeFrameBufferSize, Dem_DTCOriginType origin)
{
    /* TODO: Delete OBD freeze frames if the event is not emission related
     *  */
    boolean freezeFrameBlockChanged = FALSE;
    //lint -save
    //lint -e568 //PC-Lint exception.
    //lint -e685 //PC-Lint exception.
    //lint -e681 //PC-Lint exception to MISRA 14.1: Loop is not entered. This only happens when DEM_MAX_NUMBER_FF_DATA_PRE_INIT is zero. Keep as it is for less complex code.
    // Validate freeze frame records stored in primary memory
    for (uint16 i = 0; i < freezeFrameBufferSize; i++) {
        if ((freezeFrameBuffer[i].eventId == DEM_EVENT_ID_NULL) || (FALSE == checkEntryValid(freezeFrameBuffer[i].eventId, origin))) {
            // Unlegal record, clear the record
            memset(&freezeFrameBuffer[i], 0, sizeof(FreezeFrameRecType));
            freezeFrameBlockChanged = TRUE;
        }
    }
    //lint -restore
    return freezeFrameBlockChanged;
}

static boolean validateExtendedData(ExtDataRecType* extendedDataBuffer, uint32 extendedDataBufferSize, Dem_DTCOriginType origin)
{
    boolean extendedDataBlockChanged = FALSE;
    for (uint16 i = 0; i < extendedDataBufferSize; i++) {
        if ((extendedDataBuffer[i].eventId == DEM_EVENT_ID_NULL) || (FALSE == checkEntryValid(extendedDataBuffer[i].eventId, origin))) {
            // Unlegal record, clear the record
            memset(&extendedDataBuffer[i], 0, sizeof(ExtDataRecType));
            extendedDataBlockChanged = TRUE;
        }
    }
    return extendedDataBlockChanged;
}
#endif /* DEM_USE_MEMORY_FUNCTIONS */
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


static void InitialModifyBuffers(void) {


    uint32 indx = 0;
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
    EventIsModified[indx].blockId = DEM_EVENT_PRIMARY_NVM_BLOCK_HANDLE;
    EventIsModified[indx].origin = DEM_DTC_ORIGIN_PRIMARY_MEMORY;
    EventIsModified[indx].dataModified = FALSE;
    indx++;
#endif
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
    EventIsModified[indx].blockId = DEM_EVENT_SECONDARY_NVM_BLOCK_HANDLE;
    EventIsModified[indx].origin = DEM_DTC_ORIGIN_SECONDARY_MEMORY;
    EventIsModified[indx].dataModified = FALSE;
    indx++;
#endif

    indx = 0;
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
    FFIsModified[indx].blockId = DEM_FREEZE_FRAME_PRIMARY_NVM_BLOCK_HANDLE;
    FFIsModified[indx].origin = DEM_DTC_ORIGIN_PRIMARY_MEMORY;
    FFIsModified[indx].dataModified = FALSE;
    indx++;
#endif
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
    FFIsModified[indx].blockId = DEM_FREEZE_FRAME_SECONDARY_NVM_BLOCK_HANDLE;
    FFIsModified[indx].origin = DEM_DTC_ORIGIN_SECONDARY_MEMORY;
    FFIsModified[indx].dataModified = FALSE;
    indx++;
#endif


    indx = 0;
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
    ExtendedDataIsModified[indx].blockId = DEM_EXTENDED_DATA_PRIMARY_NVM_BLOCK_HANDLE;
    ExtendedDataIsModified[indx].origin = DEM_DTC_ORIGIN_PRIMARY_MEMORY;
    ExtendedDataIsModified[indx].dataModified = FALSE;
    indx++;
#endif
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
    ExtendedDataIsModified[indx].blockId = DEM_EXTENDED_DATA_SECONDARY_NVM_BLOCK_HANDLE;
    ExtendedDataIsModified[indx].origin = DEM_DTC_ORIGIN_SECONDARY_MEMORY;
    ExtendedDataIsModified[indx].dataModified = FALSE;
    indx++;
#endif

#if !defined(CFG_DEM_AGING_PROCESSING_EXTENSION)
    indx = 0;

#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
    AgingIsModified[indx].blockId = DEM_AGING_PRIMARY_NVM_BLOCK_HANDLE;
    AgingIsModified[indx].origin = DEM_DTC_ORIGIN_PRIMARY_MEMORY;
    AgingIsModified[indx].dataModified = FALSE;
    indx++;
#endif
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
    AgingIsModified[indx].blockId = DEM_AGING_SECONDARY_NVM_BLOCK_HANDLE;
    AgingIsModified[indx].origin = DEM_DTC_ORIGIN_SECONDARY_MEMORY;
    AgingIsModified[indx].dataModified = FALSE;
#endif

#endif

}



/*
 * Procedure:   Dem_PreInit
 * Reentrant:   No
 */
void Dem_PreInit(const Dem_ConfigType *ConfigPtr)
{
    /** @req DEM180 */
    uint16 i;
    imask_t state;

    VALIDATE_NO_RV(ConfigPtr != NULL, DEM_PREINIT_ID, DEM_E_CONFIG_PTR_INVALID);
    VALIDATE_NO_RV(ConfigPtr->ConfigSet != NULL, DEM_PREINIT_ID, DEM_E_CONFIG_PTR_INVALID);

    Irq_Save(state);

#if defined(USE_NVM) && (DEM_USE_NVM == STD_ON)
    /* Check sizes of used NvM blocks */
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
    DEM_ASSERT( (0 == DEM_EVENT_PRIMARY_NVM_BLOCK_HANDLE) ||
                (DEM_EVENT_PRIMARY_NVM_BLOCK_SIZE == sizeof(priMemEventBuffer)));

    DEM_ASSERT( (0 == DEM_FREEZE_FRAME_PRIMARY_NVM_BLOCK_HANDLE) ||
                (DEM_FREEZE_FRAME_PRIMARY_NVM_BLOCK_SIZE == sizeof(priMemFreezeFrameBuffer)));

    DEM_ASSERT( (0 == DEM_EXTENDED_DATA_PRIMARY_NVM_BLOCK_HANDLE) ||
                (DEM_EXTENDED_DATA_PRIMARY_NVM_BLOCK_SIZE == sizeof(priMemExtDataBuffer)));

#endif

#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
    DEM_ASSERT( (0 == DEM_EVENT_SECONDARY_NVM_BLOCK_HANDLE) ||
                (DEM_EVENT_SECONDARY_NVM_BLOCK_SIZE == sizeof(secMemEventBuffer)));

    DEM_ASSERT( (0 == DEM_FREEZE_FRAME_SECONDARY_NVM_BLOCK_HANDLE) ||
                (DEM_FREEZE_FRAME_SECONDARY_NVM_BLOCK_SIZE == sizeof(secMemFreezeFrameBuffer)));

    DEM_ASSERT( (0 == DEM_EXTENDED_DATA_SECONDARY_NVM_BLOCK_HANDLE ) ||
                (DEM_EXTENDED_DATA_SECONDARY_NVM_BLOCK_SIZE == sizeof(secMemExtDataBuffer)));
#endif

#if !defined(CFG_DEM_AGING_PROCESSING_EXTENSION)

#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
    DEM_ASSERT( (0 == DEM_AGING_PRIMARY_NVM_BLOCK_HANDLE ) ||
                (DEM_AGING_PRIMARY_NVM_BLOCK_SIZE == sizeof(priMemAgingBuffer)));
#endif

#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
    DEM_ASSERT( (0 == DEM_AGING_SECONDARY_NVM_BLOCK_HANDLE ) ||
                (DEM_AGING_SECONDARY_NVM_BLOCK_HANDLE == sizeof(secMemAgingBuffer)));
#endif

#endif
#endif

    InitialModifyBuffers();

    configSet = ConfigPtr->ConfigSet;

    // Initializion of operation cycle states.
    for (i = 0; i < DEM_OPERATION_CYCLE_ID_ENDMARK; i++) {
        operationCycleStateList[i] = DEM_CYCLE_STATE_END;
    }

    // Initialize the event status buffer
    for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
        setDefaultEventStatus(&eventStatusBuffer[i]);
    }

    // Initialize the eventstatus buffer (Insert all supported events into event status buffer)
    initEventStatusBuffer(configSet->EventParameter);

    /* Initialize the preInit freeze frame buffer */
    initPreInitFreezeFrameBuffer();

    /* Initialize the preInit extended data buffer */
    initPreInitExtDataBuffer();

#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
    /* Initialize the enable conditions */
    initEnableConditions();
#endif

#if defined(USE_DEM_EXTENSION)
    Dem_Extension_PreInit(ConfigPtr);
#endif

#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
    /* Reset freze frame time stamp */
    FF_TimeStamp = 0;

    /* Reset event time stamp */
    Event_TimeStamp = 0;

    /* Reset extended data timestamp */
    ExtData_TimeStamp = 0;
#endif

    disableDtcSetting.settingDisabled = FALSE;

    (void)setOperationCycleState(DEM_ACTIVE, DEM_CYCLE_STATE_START); /** @req DEM047 */

    demState = DEM_PREINITIALIZED;

    Irq_Restore(state);
}

#ifdef DEM_USE_MEMORY_FUNCTIONS
static boolean ValidateAndMergeEventRecords(EventRecType* eventBuffer, uint32 eventBufferSize, boolean* eventEntryChanged,
                                    uint32* Evt_TimeStamp, Dem_DTCOriginType origin ) {


    boolean eventBlockChanged = FALSE;
    uint32 i;

    // Validate event records stored in memory
    for (i = 0; i < eventBufferSize; i++) {
        eventEntryChanged[i] = FALSE;

        if ((eventBuffer[i].eventId == DEM_EVENT_ID_NULL) || (!checkEntryValid(eventBuffer[i].eventId, origin) )) {
            // Unlegal record, clear the record
            memset(&eventBuffer[i], 0, sizeof(EventRecType));
            eventBlockChanged = TRUE;
        }
    #if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_OFF)
        else {
            // Valid, update current status
            eventEntryChanged[i] = mergeEventStatusRec(&eventBuffer[i]);
        }
    #endif
    }

#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON)
#if !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
    /* initialize the current timestamp and update the timestamp in pre init */
    initCurrentEventTimeStamp(Evt_TimeStamp);
#else
    (void)Evt_TimeStamp;/* Avoid compiler warning */
#endif
    /* Merge events read from NvRam */
    for (i = 0; i < eventBufferSize; i++) {
        eventEntryChanged[i] = FALSE;
        if( DEM_EVENT_ID_NULL != eventBuffer[i].eventId ) {
            eventEntryChanged[i] = mergeEventStatusRec(&eventBuffer[i]);
        }
    }
#else
    (void)Evt_TimeStamp;/* Avoid compiler warning */
#endif

    return eventBlockChanged;

}

static void MergeBuffer(Dem_DTCOriginType origin) {

    uint16 i;
    boolean eventBlockChanged = FALSE;
    boolean extendedDataBlockChanged = FALSE;
    boolean freezeFrameBlockChanged = FALSE;
#if !defined(CFG_DEM_AGING_PROCESSING_EXTENSION)
    boolean agingBlockChanged = FALSE;
#endif
    /*lint -e506 */
    boolean eventEntryChanged[DEM_MAX_NUMBER_EVENT_ENTRY] = {FALSE};
    const Dem_EventParameterType *eventParam;
#if !defined(CFG_DEM_AGING_PROCESSING_EXTENSION)
    HealingRecType* agingBuffer = NULL;
    uint32 agingBufferSize = 0;
#endif
    EventRecType* eventBuffer = NULL;
    uint32 eventBufferSize = 0;
    FreezeFrameRecType* freezeFrameBuffer = NULL;
    uint32 freezeFrameBufferSize = 0;
    ExtDataRecType* extendedDataBuffer = NULL;
    uint32 extendedDataBufferSize = 0;

    /* Setup variables for merging */
    switch (origin) {
        case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
#if !defined(CFG_DEM_AGING_PROCESSING_EXTENSION)
            agingBuffer = priMemAgingBuffer;
            agingBufferSize = DEM_MAX_NUMBER_AGING_PRI_MEM;
#endif
            eventBuffer = priMemEventBuffer;
            eventBufferSize = DEM_MAX_NUMBER_EVENT_PRI_MEM;
            freezeFrameBuffer = priMemFreezeFrameBuffer;
            freezeFrameBufferSize = DEM_MAX_NUMBER_FF_DATA_PRI_MEM;
            extendedDataBuffer = priMemExtDataBuffer;
            extendedDataBufferSize = DEM_MAX_NUMBER_EXT_DATA_PRI_MEM;
#endif
            break;

        case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
#if !defined(CFG_DEM_AGING_PROCESSING_EXTENSION)
            agingBuffer = secMemAgingBuffer;
            agingBufferSize = DEM_MAX_NUMBER_AGING_SEC_MEM;
#endif
            eventBuffer = secMemEventBuffer;
            eventBufferSize = DEM_MAX_NUMBER_EVENT_SEC_MEM;
            freezeFrameBuffer = secMemFreezeFrameBuffer;
            freezeFrameBufferSize = DEM_MAX_NUMBER_FF_DATA_SEC_MEM;
            extendedDataBuffer = secMemExtDataBuffer;
            extendedDataBufferSize = DEM_MAX_NUMBER_EXT_DATA_SEC_MEM;
#endif
            break;
        default:
            DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GLOBAL_ID, DEM_E_NOT_IMPLEMENTED_YET);
            break;

    }

#if !defined(CFG_DEM_AGING_PROCESSING_EXTENSION)
        /* Validate aging records stored in memory*/
        agingBlockChanged = validateAgingBuffer(agingBuffer, agingBufferSize, origin);
#endif
#if !((DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION))
        /* The timestamp isn't actually used. This just to make it compile.. */
        uint32 Event_TimeStamp = 0;
#endif
        eventBlockChanged = ValidateAndMergeEventRecords(eventBuffer, eventBufferSize, eventEntryChanged, &Event_TimeStamp, origin);

#if defined(USE_DEM_EXTENSION)
        Dem_Extension_Init_PostEventMerge(origin);
#endif

#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
        //initialize the current timestamp and update the timestamp in pre init
        initCurrentFreezeFrameTimeStamp(&FF_TimeStamp);
#endif

        /* Validate freeze frames stored in memory */
        freezeFrameBlockChanged = validateFreezeFrames(freezeFrameBuffer, freezeFrameBufferSize, origin);

        /* Transfer updated event data to event memory */
        for (i = 0; i < eventBufferSize; i++) {
            if ( (eventBuffer[i].eventId != DEM_EVENT_ID_NULL) && eventEntryChanged[i] ) {
                EventStatusRecType *eventStatusRecPtr = NULL;
                eventParam = NULL;
                lookupEventIdParameter(eventBuffer[i].eventId, &eventParam);
                 /* Transfer to event memory. */
                lookupEventStatusRec(eventBuffer[i].eventId, &eventStatusRecPtr);
                if( (NULL != eventStatusRecPtr) && (NULL != eventParam) ) {
                    if( E_OK == storeEventEvtMem(eventParam, eventStatusRecPtr) ) {
                        /* Use errorStatusChanged in eventsStatusBuffer to signal that the event data was updated */
                        eventStatusRecPtr->errorStatusChanged = TRUE;
                    }
                }
            }
        }

        /* Now we need to store events that was reported during preInit.
         * That is, events not already stored in eventBuffer.  */
        for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
            if( (DEM_EVENT_ID_NULL != eventStatusBuffer[i].eventId) &&
                 !(eventStatusBuffer[i].eventStatusExtended & DEM_TEST_NOT_COMPLETED_THIS_OPERATION_CYCLE) &&
                 !eventIsStoredInMem(eventStatusBuffer[i].eventId, eventBuffer, eventBufferSize) ) {

                lookupEventIdParameter(eventStatusBuffer[i].eventId, &eventParam);
                if( (NULL != eventParam) && (eventParam->EventClass->EventDestination == origin)) {
                    /* Destination check is needed two avoid notifying status change twice */
                    notifyEventStatusChange(eventParam, DEM_DEFAULT_EVENT_STATUS, eventStatusBuffer[i].eventStatusExtended);
                }
                if( E_OK == storeEventEvtMem(eventParam, &eventStatusBuffer[i]) ) {
                    /* Use errorStatusChanged in eventsStatusBuffer to signal that the event data was updated */
                    eventStatusBuffer[i].errorStatusChanged = TRUE;
                }
            }
        }

        // Validate extended data records stored in primary memory
        extendedDataBlockChanged = validateExtendedData(extendedDataBuffer, extendedDataBufferSize, origin);

#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
        //initialize the current timestamp and update the timestamp in pre init
        initCurrentExtDataTimeStamp(&ExtData_TimeStamp);
#endif

        /* Transfer extended data to event memory if necessary */
        for (i = 0; i < DEM_MAX_NUMBER_EXT_DATA_PRE_INIT; i++) {
            if (preInitExtDataBuffer[i].eventId !=  DEM_EVENT_ID_NULL) {
                boolean updateAllExtData = FALSE;
                lookupEventIdParameter(preInitExtDataBuffer[i].eventId, &eventParam);
#if defined(USE_DEM_EXTENSION)
                Dem_Extension_PreMergeExtendedData(preInitExtDataBuffer[i].eventId, &updateAllExtData);
#endif
                if(mergeExtendedDataEvtMem(eventParam, &preInitExtDataBuffer[i], extendedDataBuffer, extendedDataBufferSize, origin, updateAllExtData)) {
                    /* Use errorStatusChanged in eventsStatusBuffer to signal that the event data was updated */
                    EventStatusRecType *eventStatusRecPtr;
                    lookupEventStatusRec(eventParam->EventID, &eventStatusRecPtr);
                    if(NULL != eventStatusRecPtr) {
                        eventStatusRecPtr->errorStatusChanged = TRUE;
                    }
                }
            }
        }

        /* Transfer freeze frames stored during preInit to event memory */
        if( transferPreInitFreezeFramesEvtMem(freezeFrameBuffer, freezeFrameBufferSize, origin) ){
            freezeFrameBlockChanged = TRUE;
        }
        if( eventBlockChanged ) {
            setEventBlockChanged(origin);
        }
        if( extendedDataBlockChanged ) {
            setExtendedDataBlockChanged(origin);
        }
        if( freezeFrameBlockChanged ) {
            setFreezeFrameBlockChanged(origin);
        }
    #if !defined(CFG_DEM_AGING_PROCESSING_EXTENSION)
        if( agingBlockChanged ) {
            setAgingBlockChanged(origin);
        }
    #endif
}
#endif /* DEM_USE_MEMORY_FUNCTIONS */


/*
 * Procedure:   Dem_Init
 * Reentrant:   No
 */
void Dem_Init(void)
{

    imask_t state;
    Irq_Save(state);
    for(uint16 i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
        eventStatusBuffer[i].errorStatusChanged = FALSE;
    }
    if(DEM_PREINITIALIZED != demState){
        /*
         * Dem_PreInit was has not been called since last time Dem_Shutdown was called.
         * This suggests that we are resuming from sleep. According to section 5.7 in
         * EcuM specification, RAM content is assumed to be still valid from the previous cycle.
         * Do not read from saved error log since buffers already contains this data.
         */
        (void)setOperationCycleState(DEM_ACTIVE, DEM_CYCLE_STATE_START);

    } else {
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
        MergeBuffer(DEM_DTC_ORIGIN_PRIMARY_MEMORY);
#endif
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
        MergeBuffer(DEM_DTC_ORIGIN_SECONDARY_MEMORY);
#endif
    }
    /* Notify application if event data was updated */
    for(uint16 i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
        /* @req DEM475 */
        if( eventStatusBuffer[i].errorStatusChanged ) {
            notifyEventDataChanged(eventStatusBuffer[i].eventParamRef);
            eventStatusBuffer[i].errorStatusChanged = FALSE;
        }
    }
#if defined(USE_DEM_EXTENSION)
    Dem_Extension_Init_Complete();
#endif

    // Init the dtc filter
    dtcFilter.dtcStatusMask = DEM_DTC_STATUS_MASK_ALL;                  // All allowed
    dtcFilter.dtcKind = DEM_DTC_KIND_ALL_DTCS;                          // All kinds of DTCs
    dtcFilter.dtcOrigin = DEM_DTC_ORIGIN_PRIMARY_MEMORY;                // Primary memory
    dtcFilter.filterWithSeverity = DEM_FILTER_WITH_SEVERITY_NO;         // No Severity filtering
    dtcFilter.dtcSeverityMask = DEM_SEVERITY_NO_SEVERITY;               // Not used when filterWithSeverity is FALSE
    dtcFilter.filterForFaultDetectionCounter = DEM_FILTER_FOR_FDC_NO;   // No fault detection counter filtering

    dtcFilter.faultIndex = DEM_MAX_NUMBER_EVENT;

    disableDtcSetting.settingDisabled = FALSE;

    ffRecordFilter.ffIndex = DEM_MAX_NUMBER_FF_DATA_PRI_MEM;
    ffRecordFilter.dtcFormat = 0xff;

    demState = DEM_INITIALIZED;

    Irq_Restore(state);
}


/*
 * Procedure:   Dem_shutdown
 * Reentrant:   No
 */
void Dem_Shutdown(void)
{

    imask_t state;
    Irq_Save(state);

    (void)setOperationCycleState(DEM_ACTIVE, DEM_CYCLE_STATE_END); /** @req DEM047 */

    /* handleAging() should be called behind setOperationCycleState() */
#if !defined(CFG_DEM_AGING_PROCESSING_EXTENSION)
    (void)handleAging(DEM_ACTIVE, DEM_CYCLE_STATE_END);
#endif
#if defined(USE_DEM_EXTENSION)
    Dem_Extension_Shutdown();
#endif
    demState = DEM_UNINITIALIZED; /** @req DEM368 */

    Irq_Restore(state);
}


/*
 * Interface for basic software scheduler
 */

void Dem_MainFunction(void)/** @req DEM125 */
{
#ifdef DEM_USE_MEMORY_FUNCTIONS
    for (uint32 i=0;i<DEM_MAX_NR_OF_EVENT_DESTINATION;i++) {
        if( EventIsModified[i].dataModified ) {
            setEventBlockChanged(EventIsModified[i].origin);
        }

        if( ExtendedDataIsModified[i].dataModified ) {
            setExtendedDataBlockChanged(ExtendedDataIsModified[i].origin);
        }

        if ( FFIsModified[i].dataModified ) {
            setFreezeFrameBlockChanged(FFIsModified[i].origin);
        }
    #if !defined(CFG_DEM_AGING_PROCESSING_EXTENSION)
        if ( AgingIsModified[i].dataModified ) {
            setAgingBlockChanged(AgingIsModified[i].origin);
        }
    #endif

    }
#endif /* DEM_USE_MEMORY_FUNCTIONS */
#if defined(USE_DEM_EXTENSION)
    Dem_Extension_MainFunction();
#endif
}


/***************************************************
 * Interface SW-Components via RTE <-> DEM (8.3.3) *
 ***************************************************/

/*
 * Procedure:   Dem_SetEventStatus
 * Reentrant:   Yes
 */
Std_ReturnType Dem_SetEventStatus(Dem_EventIdType eventId, Dem_EventStatusType eventStatus) /** @req DEM330 */
{
    imask_t state;

    VALIDATE_RV(DEM_INITIALIZED == demState, DEM_SETEVENTSTATUS_ID, DEM_E_UNINIT, E_NOT_OK);
    VALIDATE_RV(IS_VALID_EVENT_STATUS(eventStatus), DEM_SETEVENTSTATUS_ID, DEM_E_PARAM_DATA, E_NOT_OK);
    Irq_Save(state);

    Std_ReturnType returnCode = handleEvent(eventId, eventStatus);

    Irq_Restore(state);

    return returnCode;
}


/*
 * Procedure:   Dem_ResetEventStatus
 * Reentrant:   Yes
 */
Std_ReturnType Dem_ResetEventStatus(Dem_EventIdType eventId) /** @req DEM331 */
{
    Std_ReturnType returnCode = E_OK;

    imask_t state;
    Irq_Save(state);


    if (demState == DEM_INITIALIZED) // No action is taken if the module is not started
    {
        /* Function resetEventStatus will notify application if there is a change in the status bits */
        resetEventStatus(eventId); /** @req DEM186 */
    }
    else
    {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_RESETEVENTSTATUS_ID, DEM_E_UNINIT);
        returnCode = E_NOT_OK;
    }

    Irq_Restore(state);
    return returnCode;
}


/*
 * Procedure:   Dem_GetEventStatus
 * Reentrant:   Yes
 */
Std_ReturnType Dem_GetEventStatus(Dem_EventIdType eventId, Dem_EventStatusExtendedType *eventStatusExtended) /** @req DEM332 */
{
    imask_t state;
    Irq_Save(state);

    Std_ReturnType returnCode = E_OK;

    if (demState == DEM_INITIALIZED) // No action is taken if the module is not started
    {
        getEventStatus(eventId, eventStatusExtended);
    }
    else
    {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETEVENTSTATUS_ID, DEM_E_UNINIT);
        returnCode = E_NOT_OK;
    }

    Irq_Restore(state);

    return returnCode;
}


/*
 * Procedure:   Dem_GetEventFailed
 * Reentrant:   Yes
 */
Std_ReturnType Dem_GetEventFailed(Dem_EventIdType eventId, boolean *eventFailed) /** @req DEM333 */
{
    Std_ReturnType returnCode = E_OK;
    imask_t state;
    Irq_Save(state);

    if (demState == DEM_INITIALIZED) // No action is taken if the module is not started
    {
        getEventFailed(eventId, eventFailed);
    }
    else
    {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETEVENTFAILED_ID, DEM_E_UNINIT);
        returnCode = E_NOT_OK;
    }

    Irq_Restore(state);

    return returnCode;
}


/*
 * Procedure:   Dem_GetEventTested
 * Reentrant:   Yes
 */
Std_ReturnType Dem_GetEventTested(Dem_EventIdType eventId, boolean *eventTested)
{
    Std_ReturnType returnCode = E_OK;
    imask_t state;
    Irq_Save(state);

    if (demState == DEM_INITIALIZED) // No action is taken if the module is not started
    {
        getEventTested(eventId, eventTested);
    }
    else
    {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETEVENTTESTED_ID, DEM_E_UNINIT);
        returnCode = E_NOT_OK;
    }

    Irq_Restore(state);

    return returnCode;
}


/*
 * Procedure:   Dem_GetFaultDetectionCounter
 * Reentrant:   No
 */
Std_ReturnType Dem_GetFaultDetectionCounter(Dem_EventIdType eventId, sint8 *counter)
{
    Std_ReturnType returnCode = E_OK;
    imask_t state;
    Irq_Save(state);


    if (demState == DEM_INITIALIZED) // No action is taken if the module is not started
    {
        returnCode = getFaultDetectionCounter(eventId, counter);
    }
    else
    {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETFAULTDETECTIONCOUNTER_ID, DEM_E_UNINIT);
        returnCode = E_NOT_OK;
    }

    Irq_Restore(state);
    return returnCode;
}


/*
 * Procedure:   Dem_SetOperationCycleState
 * Reentrant:   No
 */
Std_ReturnType Dem_SetOperationCycleState(Dem_OperationCycleIdType operationCycleId, Dem_OperationCycleStateType cycleState)
{
    Std_ReturnType returnCode = E_OK;
    imask_t state;
    Irq_Save(state);

    if (demState == DEM_INITIALIZED) // No action is taken if the module is not started
    {
        returnCode = setOperationCycleState(operationCycleId, cycleState);
#if !defined(CFG_DEM_AGING_PROCESSING_EXTENSION)
        (void)handleAging(operationCycleId, cycleState);
#endif

    }
    else
    {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_SETOPERATIONCYCLESTATE_ID, DEM_E_UNINIT);
        returnCode = E_NOT_OK;
    }

    Irq_Restore(state);
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

    imask_t state;
    Irq_Save(state);


    (void)dtcFormat; /* avoid compiler warning - different DTC formats not supported */

    if (demState == DEM_INITIALIZED) // No action is taken if the module is not started
    {
        lookupEventIdParameter(eventId, &eventParam);

        if (eventParam != NULL) {
            // TODO: DTCFormat issue (UDS, OBD)
            if (eventParam->DTCClassRef != NULL) {
                *dtcOfEvent = eventParam->DTCClassRef->DTC; /** @req DEM269 */
                returnCode = E_OK;
            }
        }
        else {
            // Event Id not found
            returnCode = E_NOT_OK;
        }
    }
    else
    {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETDTCOFEVENT_ID, DEM_UNINITIALIZED);
        returnCode = E_NOT_OK;
    }

    Irq_Restore(state);
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
    imask_t state;
    VALIDATE_NO_RV(DEM_UNINITIALIZED != demState, DEM_REPORTERRORSTATUS_ID, DEM_E_UNINIT);
    VALIDATE_NO_RV(IS_VALID_EVENT_STATUS(eventStatus), DEM_REPORTERRORSTATUS_ID, DEM_E_PARAM_DATA);

    Irq_Save(state);

    switch (demState) {
        case DEM_PREINITIALIZED:
            // Update status and check if is to be stored
            if ((eventStatus == DEM_EVENT_STATUS_PASSED) || (eventStatus == DEM_EVENT_STATUS_FAILED)) {
                handlePreInitEvent(eventId, eventStatus); /** @req DEM168 */
            }
            break;

        case DEM_INITIALIZED:
            (void)handleEvent(eventId, eventStatus);    /** @req DEM167 */
            break;

        default:
            // Uninitialized can not do anything
            break;

    } // switch (demState)

    Irq_Restore(state);
}

/*********************************
 * Interface DCM <-> DEM (8.3.5) *
 *********************************/
/*
 * Procedure:   Dem_GetDTCStatusAvailabilityMask
 * Reentrant:   No
 */
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

    if (demState == DEM_INITIALIZED) {
        // Check dtcKind parameter
        VALIDATE_RV((dtcKind == DEM_DTC_KIND_ALL_DTCS) || (dtcKind == DEM_DTC_KIND_EMISSION_REL_DTCS), DEM_SETDTCFILTER_ID, DEM_E_PARAM_DATA, DEM_WRONG_FILTER);

        // Check dtcOrigin parameter
        VALIDATE_RV((dtcOrigin == DEM_DTC_ORIGIN_SECONDARY_MEMORY) || (dtcOrigin == DEM_DTC_ORIGIN_PRIMARY_MEMORY), DEM_SETDTCFILTER_ID, DEM_E_PARAM_DATA, DEM_WRONG_FILTER);

        // Check filterWithSeverity and dtcSeverityMask parameter
        VALIDATE_RV(((filterWithSeverity == DEM_FILTER_WITH_SEVERITY_NO)
                    || ((filterWithSeverity == DEM_FILTER_WITH_SEVERITY_YES)
                        && (!(dtcSeverityMask & (Dem_DTCSeverityType)~(DEM_SEVERITY_MAINTENANCE_ONLY | DEM_SEVERITY_CHECK_AT_NEXT_FALT | DEM_SEVERITY_CHECK_IMMEDIATELY))))), DEM_SETDTCFILTER_ID, DEM_E_PARAM_DATA, DEM_WRONG_FILTER);

        // Check filterForFaultDetectionCounter parameter
        VALIDATE_RV((filterForFaultDetectionCounter == DEM_FILTER_FOR_FDC_YES) || (filterForFaultDetectionCounter ==  DEM_FILTER_FOR_FDC_NO), DEM_SETDTCFILTER_ID, DEM_E_PARAM_DATA, DEM_WRONG_FILTER);

        VALIDATE_RV( ( DEM_DTC_FORMAT_OBD == dtcFormat) || (DEM_DTC_FORMAT_UDS == dtcFormat), DEM_SETDTCFILTER_ID, DEM_E_PARAM_DATA, DEM_WRONG_FILTER);

        (void)Dem_GetDTCStatusAvailabilityMask(&dtcStatusAvailabilityMask);

        // Yes all parameters correct, set the new filters.  /** @req DEM057 */
        dtcFilter.dtcStatusMask = dtcStatusMask & dtcStatusAvailabilityMask;
        dtcFilter.dtcKind = dtcKind;
        dtcFilter.dtcOrigin = dtcOrigin;
        dtcFilter.filterWithSeverity = filterWithSeverity;
        dtcFilter.dtcSeverityMask = dtcSeverityMask;
        dtcFilter.filterForFaultDetectionCounter = filterForFaultDetectionCounter;
        dtcFilter.faultIndex = DEM_MAX_NUMBER_EVENT;
        dtcFilter.dtcFormat = dtcFormat;
    } else {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_SETDTCFILTER_ID, DEM_E_UNINIT);
        returnCode = DEM_WRONG_FILTER;
    }

    return returnCode;
}


/*
 * Procedure:   Dem_GetStatusOfDTC
 * Reentrant:   No
 */
Dem_ReturnGetStatusOfDTCType Dem_GetStatusOfDTC(uint32 dtc, Dem_DTCOriginType dtcOrigin, Dem_EventStatusExtendedType* status) {
    Dem_ReturnGetStatusOfDTCType returnCode = DEM_STATUS_FAILED;
    EventStatusRecType *eventRec;
    imask_t state;
    Irq_Save(state);

    if (demState == DEM_INITIALIZED) {
        if (lookupDtcEvent(dtc, &eventRec)) {
            if (checkDtcOrigin(dtcOrigin,eventRec->eventParamRef)) {
                *status = eventRec->eventStatusExtended; /** @req DEM059 */
                returnCode = DEM_STATUS_OK;
            }
            else {
                returnCode = DEM_STATUS_WRONG_DTCORIGIN; /** @req DEM171 */
            }
        }
        else {
            returnCode = DEM_STATUS_WRONG_DTC; /** @req DEM172 */
        }
    } else {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETSTATUSOFDTC_ID, DEM_E_UNINIT);
        returnCode = DEM_STATUS_FAILED;
    }

    Irq_Restore(state);

    return returnCode;
}


/*
 * Procedure:   Dem_GetNumberOfFilteredDtc
 * Reentrant:   No
 */
Dem_ReturnGetNumberOfFilteredDTCType Dem_GetNumberOfFilteredDtc(uint16 *numberOfFilteredDTC) {
    uint16 i;
    uint16 numberOfFaults = 0;
    Dem_ReturnGetNumberOfFilteredDTCType returnCode = DEM_NUMBER_OK;

    imask_t state;
    Irq_Save(state);

    if (demState == DEM_INITIALIZED) {
        //Dem_DisableEventStatusUpdate();

        for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
            if (eventStatusBuffer[i].eventId != DEM_EVENT_ID_NULL) {
                if (matchEventWithDtcFilter(&eventStatusBuffer[i])) {
                    if (eventStatusBuffer[i].eventParamRef->DTCClassRef != NULL) {
                        numberOfFaults++;
                    }
                }
            }
        }

        //Dem_EnableEventStatusUpdate();

        *numberOfFilteredDTC = numberOfFaults; /** @req DEM061 */
    } else {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETNUMBEROFFILTEREDDTC_ID, DEM_E_UNINIT);
        returnCode = DEM_NUMBER_FAILED;
    }

    Irq_Restore(state);
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

    imask_t state;
    Irq_Save(state);

    if (demState == DEM_INITIALIZED) {
        // TODO: This job should be done in an more advanced way according to Dem217
        while ((!dtcFound) && (dtcFilter.faultIndex != 0)) {
            dtcFilter.faultIndex--;
            if (eventStatusBuffer[dtcFilter.faultIndex].eventId != DEM_EVENT_ID_NULL) {
                if (matchEventWithDtcFilter(&eventStatusBuffer[dtcFilter.faultIndex])) {
                    if (eventStatusBuffer[dtcFilter.faultIndex].eventParamRef->DTCClassRef != NULL) {
                        *dtc = eventStatusBuffer[dtcFilter.faultIndex].eventParamRef->DTCClassRef->DTC; /** @req DEM216 */
                        *dtcStatus = eventStatusBuffer[dtcFilter.faultIndex].eventStatusExtended;
                        dtcFound = TRUE;
                    }
                }
            }
        }

        if (!dtcFound) {
            dtcFilter.faultIndex = DEM_MAX_NUMBER_EVENT;
            returnCode = DEM_FILTERED_NO_MATCHING_DTC;
        }
    } else {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETNEXTFILTEREDDTC_ID, DEM_E_UNINIT);
        returnCode = DEM_FILTERED_NO_MATCHING_DTC;
    }

    Irq_Restore(state);
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
    Dem_EventIdType eventId;
    const Dem_EventParameterType *eventParam;
    uint16 i;
    Dem_EventStatusExtendedType oldStatus;
    boolean dataDeleted;
    (void)dtcFormat;
    (void)dtcOrigin;

    imask_t state;
    Irq_Save(state);


    if (demState == DEM_INITIALIZED) {
        for (i = 0; i < DEM_MAX_NUMBER_EVENT; i++) {
            dataDeleted = FALSE;
            eventId = eventStatusBuffer[i].eventId;
            if (eventId != DEM_EVENT_ID_NULL) {
                eventParam = eventStatusBuffer[i].eventParamRef;
                if (eventParam != NULL) {
                    //lint --e(506) PC-Lint exception Misra 13.7, 14.1 Allow configuration variables in boolean expression
                    //lint --e(774) PC-Lint exception   Related to MISRA 13.7
                    boolean clearAllowed = TRUE;
                    if ((DEM_CLEAR_ALL_EVENTS == STD_ON) || (eventParam->DTCClassRef != NULL)) {
                        if (checkDtcGroup(dtc, eventParam)) {
                            if( eventParam->EventClass->EventDestination == dtcOrigin ) {
                                if( NULL != eventParam->CallbackClearEventAllowed ) {
                                    /* @req DEM514 */
                                    /* @req DEM515 */
                                    if(E_OK != eventParam->CallbackClearEventAllowed(&clearAllowed)) {
                                        /* @req DEM516 */
                                        clearAllowed = TRUE;
                                    }
                                }
                                if( clearAllowed ) {
                                    boolean dtcOriginFound = FALSE;
                                    oldStatus = eventStatusBuffer[i].eventStatusExtended;
                                    switch (eventParam->EventClass->EventDestination)
                                    {
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
                                    case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
                                        /** @req DEM077 */
                                        if( deleteEventMem(eventParam, priMemEventBuffer, DEM_MAX_NUMBER_EVENT_PRI_MEM, DEM_DTC_ORIGIN_PRIMARY_MEMORY) ) {
                                            dataDeleted = TRUE;
                                        }
                                        if( deleteFreezeFrameDataMem(eventParam, DEM_DTC_ORIGIN_PRIMARY_MEMORY) ) {
                                            dataDeleted = TRUE;
                                        }
                                        if( deleteExtendedDataMem(eventParam, DEM_DTC_ORIGIN_PRIMARY_MEMORY) ) {
                                            dataDeleted = TRUE;
                                        }
                                        resetEventStatusRec(eventParam);
                                        dtcOriginFound = TRUE;
                                        break;
#endif
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
                                    case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
                                        if( deleteEventMem(eventParam, secMemEventBuffer, DEM_MAX_NUMBER_EVENT_SEC_MEM, DEM_DTC_ORIGIN_SECONDARY_MEMORY) ) {
                                            dataDeleted = TRUE;
                                        }
                                        if( deleteFreezeFrameDataMem(eventParam, DEM_DTC_ORIGIN_SECONDARY_MEMORY) ) {
                                            dataDeleted = TRUE;
                                        }
                                        if( deleteExtendedDataMem(eventParam, DEM_DTC_ORIGIN_SECONDARY_MEMORY) ) {
                                            dataDeleted = TRUE;
                                        }
                                        resetEventStatusRec(eventParam);
                                        dtcOriginFound = TRUE;
                                        break;
#endif
                                    case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
                                    case DEM_DTC_ORIGIN_MIRROR_MEMORY:
                                        // Not yet supported
                                        returnCode = DEM_CLEAR_WRONG_DTCORIGIN;
                                        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_CLEARDTC_ID, DEM_E_NOT_IMPLEMENTED_YET);
                                        break;
                                    default:
                                        break;
                                    }
                                    if (dtcOriginFound) {
#if defined(USE_DEM_EXTENSION)
                                        Dem_Extension_ClearEvent(eventParam);
#endif
                                        if( dataDeleted ) {
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
                                }
                            }
                        }
                    }
                }
                else {
                    // Fatal error, no event parameters found for the stored event!
                    DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_CLEARDTC_ID, DEM_E_UNEXPECTED_EXECUTION);
                }
            }
        }
    } else {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_CLEARDTC_ID, DEM_E_UNINIT);
        returnCode = DEM_CLEAR_FAILED;
    }

    Irq_Restore(state);

    return returnCode;
}


/*
 * Procedure:   Dem_DisableDTCStorage
 * Reentrant:   No
 */
Dem_ReturnControlDTCStorageType Dem_DisableDTCSetting(Dem_DTCGroupType dtcGroup, Dem_DTCKindType dtcKind) /** @req DEM035 */
{
    Dem_ReturnControlDTCStorageType returnCode = DEM_CONTROL_DTC_STORAGE_OK;

    if (demState == DEM_INITIALIZED) {
        // Check dtcGroup parameter
        if (dtcGroup == DEM_DTC_GROUP_ALL_DTCS) {
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
    } else {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_DISABLEDTCSTORAGE_ID, DEM_E_UNINIT);
        returnCode = DEM_CONTROL_DTC_STORAGE_N_OK;
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

    if (demState == DEM_INITIALIZED) {
        // TODO: Behavior is not defined if group or kind do not match active settings, therefore the filter is just switched off.
        (void)dtcGroup; (void)dtcKind;  // Just to make get rid of PC-Lint warnings
        disableDtcSetting.settingDisabled = FALSE; /** @req DEM080 */
    } else {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_ENABLEDTCSTORAGE_ID, DEM_E_UNINIT);
        returnCode = DEM_CONTROL_DTC_STORAGE_N_OK;
    }

    return returnCode;
}



/*
 * Procedure:   Dem_GetExtendedDataRecordByDTC
 * Reentrant:   No
 */
Dem_ReturnGetExtendedDataRecordByDTCType Dem_GetExtendedDataRecordByDTC(uint32 dtc, Dem_DTCOriginType dtcOrigin, uint8 extendedDataNumber, uint8 *destBuffer, uint16 *bufSize)
{
    Dem_ReturnGetExtendedDataRecordByDTCType returnCode = DEM_RECORD_WRONG_DTC;
    EventStatusRecType *eventRec;
    Dem_ExtendedDataRecordClassType const *extendedDataRecordClass = NULL;
    ExtDataRecType *extData;
    uint16 posInExtData = 0;

    imask_t state;
    Irq_Save(state);

    if (demState == DEM_INITIALIZED) {
        if( extendedDataNumber <= DEM_HIGHEST_EXT_DATA_REC_NUM ) {
            if (lookupDtcEvent(dtc, &eventRec)) {
                if (checkDtcOrigin(dtcOrigin, eventRec->eventParamRef)) {
                    if (lookupExtendedDataRecNumParam(extendedDataNumber, eventRec->eventParamRef, &extendedDataRecordClass, &posInExtData)) {
                        if (*bufSize >= extendedDataRecordClass->DataSize) {
                            if( extendedDataRecordClass->UpdateRule != DEM_UPDATE_RECORD_VOLATILE )
                            {
                                switch (dtcOrigin)
                                {
                                case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
                                case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
                                    if (lookupExtendedDataMem(eventRec->eventId, &extData, dtcOrigin)) {
                                        // Yes all conditions met, copy the extended data record to destination buffer.
                                        memcpy(destBuffer, &extData->data[posInExtData], extendedDataRecordClass->DataSize); /** @req DEM075 */
                                        *bufSize = extendedDataRecordClass->DataSize;
                                        returnCode = DEM_RECORD_OK;
                                    }
                                    else {
                                        // The record number is legal but no record was found for the DTC
                                        *bufSize = 0;
                                        returnCode = DEM_RECORD_OK;
                                    }
                                    break;

                                case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
                                case DEM_DTC_ORIGIN_MIRROR_MEMORY:
                                    // Not yet supported
                                    returnCode = DEM_RECORD_WRONG_DTCORIGIN;
                                    DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETEXTENDEDDATARECORDBYDTC_ID, DEM_E_NOT_IMPLEMENTED_YET);
                                    break;
                                default:
                                    returnCode = DEM_RECORD_WRONG_DTCORIGIN;
                                    break;
                                }
                            }
                            else {
                                if( NULL != extendedDataRecordClass->CallbackGetExtDataRecord ) {
                                    /* TODO: Handle return value? */
                                    (void)extendedDataRecordClass->CallbackGetExtDataRecord(destBuffer);
                                    *bufSize = extendedDataRecordClass->DataSize;
                                    returnCode = DEM_RECORD_OK;
                                }
                                else if (DEM_NO_ELEMENT != extendedDataRecordClass->InternalDataElement ) {
                                    getInternalElement( eventRec->eventParamRef, extendedDataRecordClass->InternalDataElement, destBuffer );
                                    *bufSize = extendedDataRecordClass->DataSize;
                                    returnCode = DEM_RECORD_OK;
                                }
                                else {
                                    returnCode = DEM_RECORD_WRONG_DTC;
                                }

                            }
                        }
                        else {
                            returnCode = DEM_RECORD_BUFFERSIZE;
                        }
                    }
                    else {
                        returnCode = DEM_RECORD_NUMBER;
                    }
                }
                else {
                    returnCode = DEM_RECORD_WRONG_DTCORIGIN;
                }
            } else {
                returnCode = DEM_RECORD_WRONG_DTC;
            }
        } else {
            returnCode = DEM_RECORD_NUMBER;
        }
    } else {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETEXTENDEDDATARECORDBYDTC_ID, DEM_E_UNINIT);
        returnCode = DEM_RECORD_WRONG_DTC;
    }

    Irq_Restore(state);
    return returnCode;
}


/*
 * Procedure:   Dem_GetSizeOfExtendedDataRecordByDTC
 * Reentrant:   No
 */
Dem_ReturnGetSizeOfExtendedDataRecordByDTCType Dem_GetSizeOfExtendedDataRecordByDTC(uint32 dtc, Dem_DTCOriginType dtcOrigin, uint8 extendedDataNumber, uint16 *sizeOfExtendedDataRecord)
{
    Dem_ReturnGetExtendedDataRecordByDTCType returnCode = DEM_GET_SIZEOFEDRBYDTC_W_DTC;
    EventStatusRecType *eventRec;
    Dem_ExtendedDataRecordClassType const *extendedDataRecordClass = NULL;
    uint16 posInExtData;

    imask_t state;
    Irq_Save(state);

    if (demState == DEM_INITIALIZED) {
        if (lookupDtcEvent(dtc, &eventRec)) {
            if (checkDtcOrigin(dtcOrigin, eventRec->eventParamRef)) {
                if (lookupExtendedDataRecNumParam(extendedDataNumber, eventRec->eventParamRef, &extendedDataRecordClass, &posInExtData)) {
                    *sizeOfExtendedDataRecord = extendedDataRecordClass->DataSize; /** @req DEM076 */
                    returnCode = DEM_GET_SIZEOFEDRBYDTC_OK;
                }
                else {
                    returnCode = DEM_GET_SIZEOFEDRBYDTC_W_RNUM;
                }
            }
            else {
                returnCode = DEM_GET_SIZEOFEDRBYDTC_W_DTCOR;
            }
        }
    } else {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETSIZEOFEXTENDEDDATARECORDBYDTC_ID, DEM_E_UNINIT);
        returnCode = DEM_GET_SIZEOFEDRBYDTC_W_DTC;
    }

    Irq_Restore(state);
    return returnCode;
}

/*
 * Procedure:   Dem_GetFreezeFrameDataByDTC
 * Reentrant:   No
 */
/** @req DEM236 */
Dem_ReturnGetFreezeFrameDataByDTCType Dem_GetFreezeFrameDataByDTC(uint32 dtc, Dem_DTCOriginType dtcOrigin, uint8 recordNumber, uint8* destBuffer, uint16*  bufSize)
{
    Dem_ReturnGetFreezeFrameDataByDTCType returnCode = DEM_GET_FFDATABYDTC_WRONG_DTC;
    EventStatusRecType *eventRec;
    Dem_FreezeFrameClassType const *FFDataRecordClass = NULL;
    FreezeFrameRecType *freezeframe;
    uint16 FFDataSize = 0;
    imask_t state;
    Irq_Save(state);


    if (demState == DEM_INITIALIZED) {
        if( recordNumber <= DEM_HIGHEST_FF_REC_NUM ) {
            if (lookupDtcEvent(dtc, &eventRec)) {
                if (checkDtcOrigin(dtcOrigin, eventRec->eventParamRef)) {
                    if (lookupFreezeFrameDataRecNumParam(recordNumber, eventRec->eventParamRef, &FFDataRecordClass)) {
                        /* TODO: Handle return value? */
                        (void)lookupFreezeFrameDataSize(recordNumber, &FFDataRecordClass, &FFDataSize);
                        if (*bufSize >= FFDataSize) {
                            switch (dtcOrigin)
                            {
                            case DEM_DTC_ORIGIN_PRIMARY_MEMORY:
                            case DEM_DTC_ORIGIN_SECONDARY_MEMORY:
                                if (getFreezeFrameRecord(eventRec->eventId, recordNumber, &freezeframe, FFDataRecordClass, dtcOrigin, destBuffer, bufSize, &FFDataSize)) {
                                    returnCode = DEM_GET_FFDATABYDTC_OK;
                                }
                                else {
                                    *bufSize = 0;
                                    returnCode = DEM_GET_FFDATABYDTC_OK;
                                }
                                break;
                            case DEM_DTC_ORIGIN_PERMANENT_MEMORY:
                            case DEM_DTC_ORIGIN_MIRROR_MEMORY:
                                // Not yet supported
                                returnCode = DEM_GET_FFDATABYDTC_WRONG_DTCORIGIN;
                                DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETFREEZEFRAMEDATARECORDBYDTC_ID, DEM_E_NOT_IMPLEMENTED_YET);
                                break;
                            default:
                                returnCode = DEM_GET_FFDATABYDTC_WRONG_DTCORIGIN;
                                break;
                            }
                        }
                        else{
                            returnCode = DEM_GET_FFDATABYDTC_BUFFERSIZE;
                        }
                    }
                    else {
                        returnCode = DEM_GET_FFDATABYDTC_RECORDNUMBER;
                    }
                }
                else {
                    returnCode = DEM_GET_FFDATABYDTC_WRONG_DTCORIGIN;
                }
            }
            else{
                returnCode = DEM_GET_FFDATABYDTC_WRONG_DTC;

            }
        } else {
            returnCode = DEM_GET_FFDATABYDTC_RECORDNUMBER;
        }
    } else {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETFREEZEFRAMEDATARECORDBYDTC_ID, DEM_E_UNINIT);
        returnCode = DEM_GET_ID_PENDING;
    }

    Irq_Restore(state);

    return returnCode;


}

/*
 * Procedure:   Dem_GetSizeOfFreezeFrame
 * Reentrant:   No
 */
 /** @req DEM238 */
Dem_ReturnGetSizeOfFreezeFrameType Dem_GetSizeOfFreezeFrameByDTC(uint32 dtc, Dem_DTCOriginType dtcOrigin, uint8 recordNumber, uint16* sizeOfFreezeFrame)
{
    Dem_ReturnGetSizeOfFreezeFrameType returnCode = DEM_GET_SIZEOFFF_PENDING;
    Dem_FreezeFrameClassType const *FFDataRecordClass = NULL;
    Std_ReturnType callbackReturnCode;
    EventStatusRecType *eventRec;
    uint16 dataSize = 0;
    uint16 i = 0;

    imask_t state;
    Irq_Save(state);


    if (demState == DEM_INITIALIZED) {
        if (lookupDtcEvent(dtc, &eventRec)) {
            if (checkDtcOrigin(dtcOrigin, eventRec->eventParamRef)) {
                if (lookupFreezeFrameDataRecNumParam(recordNumber, eventRec->eventParamRef, &FFDataRecordClass)) {
                    if(FFDataRecordClass->FFIdClassRef != NULL){
                        /* Note - there is a function called lookupFreezeFrameDataSize that can be used here */
                        for(i = 0; (i < DEM_MAX_NR_OF_DIDS_IN_FREEZEFRAME_DATA) && (!(FFDataRecordClass->FFIdClassRef[i]->Arc_EOL)); i++){
                            /* read out the did size */
                            if(FFDataRecordClass->FFIdClassRef[i]->DidReadDataLengthFnc != NULL){
                                callbackReturnCode = FFDataRecordClass->FFIdClassRef[i]->DidReadDataLengthFnc(&dataSize);
                                if(callbackReturnCode != E_OK){
                                    Irq_Restore(state);
                                    return DEM_GET_SIZEOFFF_PENDING;
                                }
                            }
                            else{
                                dataSize = FFDataRecordClass->FFIdClassRef[i]->PidOrDidSize;
                            }
                            *sizeOfFreezeFrame += dataSize+DEM_DID_IDENTIFIER_SIZE_OF_BYTES;/** @req DEM074 */
                            returnCode = DEM_GET_SIZEOFFF_OK;
                        }
                    }
                    else {
                        returnCode = DEM_GET_SIZEOFFF_WRONG_RNUM;
                    }
                }
                else{
                    returnCode = DEM_GET_SIZEOFFF_WRONG_RNUM;
                }
            }
            else{
                returnCode = DEM_GET_SIZEOFFF_WRONG_DTCOR;
            }
        }
        else{
            returnCode = DEM_GET_SIZEOFFF_WRONG_DTC;
        }

    }
    else{
            DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETFREEZEFRAMEDATAIDENTIFIERBYDTC_ID, DEM_E_UNINIT);
            returnCode = DEM_GET_SIZEOFFF_PENDING;
        }

    Irq_Restore(state);
    return returnCode;


}


Dem_ReturnSetFilterType Dem_SetFreezeFrameRecordFilter(Dem_DTCFormatType DTCFormat, uint16 *NumberOfFilteredRecords)
{
    Dem_ReturnSetFilterType ret = DEM_WRONG_FILTER;
    imask_t state;
    Irq_Save(state);

    if (demState != DEM_INITIALIZED) {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_SETFREEZEFRAMERECORDFILTER_ID, DEM_E_UNINIT);
    } else if( FALSE == ((DEM_DTC_FORMAT_OBD == DTCFormat) || (DEM_DTC_FORMAT_UDS == DTCFormat)) ) {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_SETFREEZEFRAMERECORDFILTER_ID, DEM_E_PARAM_DATA);
    } else {
        uint16 nofRecords = 0;
        /* @req DEM210 Only applies to primary memory */
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
        for( uint16 i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM; i++ ) {
            if( DEM_EVENT_ID_NULL != priMemFreezeFrameBuffer[i].eventId ) {
                EventStatusRecType *eventStatusRecPtr = NULL;
                lookupEventStatusRec(priMemFreezeFrameBuffer[i].eventId, &eventStatusRecPtr);
                if( (NULL != eventStatusRecPtr) && (NULL != eventStatusRecPtr->eventParamRef->DTCClassRef) ) {
                    nofRecords++;
                }
            }
        }
#endif
        *NumberOfFilteredRecords = nofRecords;
        /* @req DEM595 */
        ffRecordFilter.dtcFormat = DTCFormat;
        ffRecordFilter.ffIndex = 0;
        ret = DEM_FILTER_ACCEPTED;
    }
    Irq_Restore(state);
    return ret;
}

Dem_ReturnGetNextFilteredDTCType Dem_GetNextFilteredRecord(uint32 *DTC, uint8 *RecordNumber)
{
    Dem_ReturnGetNextFilteredDTCType ret = DEM_FILTERED_NO_MATCHING_DTC;
    imask_t state;
    Irq_Save(state);


    EventStatusRecType *eventStatusRecPtr = NULL;
    if (demState == DEM_INITIALIZED) {
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
        /* Find the next record which has a DTC */
        boolean found = FALSE;
        for( uint16 i = ffRecordFilter.ffIndex; (i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM) && !found; i++  ) {
            if( DEM_EVENT_ID_NULL != priMemFreezeFrameBuffer[i].eventId ) {
                lookupEventStatusRec(priMemFreezeFrameBuffer[i].eventId, &eventStatusRecPtr);
               if( (NULL != eventStatusRecPtr) && (NULL != eventStatusRecPtr->eventParamRef->DTCClassRef)) {
                   /* Found one! */
                   /* @req DEM225 */
                   *RecordNumber = priMemFreezeFrameBuffer[i].recordNumber;
                   *DTC = eventStatusRecPtr->eventParamRef->DTCClassRef->DTC;
                   /* @req DEM226 */
                   ffRecordFilter.ffIndex = i + 1;
                   found = TRUE;
                   ret = DEM_FILTERED_OK;
               }
            }
        }
#else
        (void)eventStatusRecPtr; /* Just to remove warning */
#endif
    } else {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETNEXTFILTEREDRECORD_ID, DEM_E_UNINIT);
    }

    Irq_Restore(state);
    return ret;
}

#if (DEM_ENABLE_CONDITION_SUPPORT == STD_ON)
/* @req DEM202 */
Std_ReturnType Dem_SetEnableCondition(uint8 EnableConditionID, boolean ConditionFulfilled)
{
    Std_ReturnType ret = E_NOT_OK;
    if( DEM_INITIALIZED == demState  ) {
        if( EnableConditionID < DEM_NUM_ENABLECONDITIONS ) {
            DemEnableConditions[EnableConditionID] = ConditionFulfilled;
            ret = E_OK;
        } else {
            DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_SETENABLECONDITION_ID, DEM_E_PARAM_DATA);
        }
    } else {
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_SETENABLECONDITION_ID, DEM_E_UNINIT);
    }
    return ret;
}
#endif

#if (DEM_UNIT_TEST == STD_ON)
void getFFDataPreInit(FreezeFrameRecType **buf)
{
    *buf = &preInitFreezeFrameBuffer[0];
    return;
}
void getPriMemFFBufPtr(FreezeFrameRecType **buf)
{
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
    *buf = &priMemFreezeFrameBuffer[0];
#endif
    return;
}
void getSecMemFFBufPtr(FreezeFrameRecType **buf)
{
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
    *buf = &secMemFreezeFrameBuffer[0];
#endif
    return;
}

#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
uint32 getCurTimeStamp()
{
    return FF_TimeStamp;
}
#endif
void getEventStatusBufPtr(EventStatusRecType **buf)
{
    *buf = &eventStatusBuffer[0];
    return;
}
#if !defined(CFG_DEM_AGING_PROCESSING_EXTENSION)
void getPriMemAgingBufPtr(HealingRecType **buf)
{
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
    *buf = &priMemAgingBuffer[0];
#endif
    return;
}

void getSecMemAgingBufPtr(HealingRecType **buf)
{
#if (DEM_USE_SECONDARY_MEMORY_SUPPORT == STD_ON)
    *buf = &secMemAgingBuffer[0];
#endif
    return;
}

#endif
#endif





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
    FreezeFrameRecType *freezeFrame = NULL;
    const Dem_EventParameterType *eventParameter = NULL;
    Std_ReturnType returnCode = E_NOT_OK;
    (void)FrameNumber;

    if (demState == DEM_INITIALIZED){
        /* find the corresponding FF in FF buffer */
        /* @req OBD_DEM_REQ_1 */
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
            for(uint16 i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM; i++){
                if((priMemFreezeFrameBuffer[i].eventId != DEM_EVENT_ID_NULL) \
                    && (DEM_FREEZE_FRAME_OBD == priMemFreezeFrameBuffer[i].kind)){
                    freezeFrame = &priMemFreezeFrameBuffer[i];
                    break;
                }
            }
#endif
        /*if FF found,find the corresponding eventParameter*/
        if(freezeFrame != NULL){
            lookupEventIdParameter(freezeFrame->eventId, &eventParameter);
            if(eventParameter != NULL){
                /* if DTCClass configured,get DTC value */
                if(eventParameter->DTCClassRef != NULL){
                    *DTC = eventParameter->DTCClassRef->DTC;
                    returnCode = E_OK;
                }
            }

        }

    }
    else{
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETDTCOFOBDFREEZEFRAME_ID, DEM_E_UNINIT);
    }

    return returnCode;

}

/*
 * Procedure:   Dem_GetOBDFreezeFrameData
 * Reentrant:   No
 */
 /* @req OBD_DEM_REQ_2 */
Std_ReturnType Dem_GetOBDFreezeFrameData(uint8 PID, uint8 DataElementIndexOfPid, uint8* DestBuffer, uint8* BufSize)
{
    /* TODO: Validate parameters */
    FreezeFrameRecType *freezeFrame = NULL;
    const Dem_FreezeFrameClassType *freezeFrameClass = NULL;
    Std_ReturnType returnCode = E_NOT_OK;
    boolean pidFound = FALSE;
    uint16 offset = 0;
    uint8 pidDataSize = 0;
    uint16 i = 0;
    imask_t state;
    Irq_Save(state);


    /* TODO: DataElementIndexOfPid should be used to get the data of the Pid. But we only support 1 data element
     * per Pid.. */
    (void)DataElementIndexOfPid;

    if (demState == DEM_INITIALIZED){
        freezeFrameClass = configSet->GlobalOBDFreezeFrameClassRef;
#if (DEM_USE_PRIMARY_MEMORY_SUPPORT == STD_ON)
        /*find the corresponding FF in FF buffer*/
        for(i = 0; i < DEM_MAX_NUMBER_FF_DATA_PRI_MEM; i++){
            /* @req OBD_DEM_REQ_1 */
            if((priMemFreezeFrameBuffer[i].eventId != DEM_EVENT_ID_NULL) \
                && (DEM_FREEZE_FRAME_OBD == priMemFreezeFrameBuffer[i].kind)){
                freezeFrame = &priMemFreezeFrameBuffer[i];
                break;
            }
        }
#endif
        /*if FF class found,find the corresponding PID*/
        if(NULL != freezeFrame){
            if(freezeFrameClass->FFKind == DEM_FREEZE_FRAME_OBD){
                if(freezeFrameClass->FFIdClassRef != NULL){
                    for(i = 0; (i < DEM_MAX_NR_OF_PIDS_IN_FREEZEFRAME_DATA) && ((freezeFrameClass->FFIdClassRef[i]->Arc_EOL) == FALSE); i++){
                        offset += DEM_PID_IDENTIFIER_SIZE_OF_BYTES;
                        if(freezeFrameClass->FFIdClassRef[i]->PidIdentifier == PID){
                            pidDataSize = freezeFrameClass->FFIdClassRef[i]->PidOrDidSize;
                            pidFound = TRUE;
                            break;
                        }
                        else{
                            offset += freezeFrameClass->FFIdClassRef[i]->PidOrDidSize;
                        }
                    }
                }

            }
        }

        if((pidFound == TRUE) && (NULL != freezeFrame) && (offset >= DEM_PID_IDENTIFIER_SIZE_OF_BYTES)){
            if(((*BufSize) >= pidDataSize) && (PID == (freezeFrame->data[offset - DEM_PID_IDENTIFIER_SIZE_OF_BYTES])) \
                && ((offset + pidDataSize) <= (freezeFrame->dataSize)) && ((offset + pidDataSize) <= DEM_MAX_SIZE_FF_DATA)){
                (void)memcpy(DestBuffer, freezeFrame->data + offset, pidDataSize);
                *BufSize = pidDataSize;
                returnCode = E_OK;
            }

        }
    }
    else{
        DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GETOBDFREEZEFRAMEDATA_ID, DEM_E_UNINIT);
    }

    Irq_Restore(state);
    return returnCode;
}


/*
 * Procedure:   getPidData
 * Description: get OBD FF data,only called by getFreezeFrameData()
 */
static void getPidData(const Dem_PidOrDidType ** const* pidClassPtr, FreezeFrameRecType **freezeFrame, uint16 *storeIndexPtr)
{
    const Dem_PidOrDidType **FFIdClassRef = NULL;
    Std_ReturnType callbackReturnCode;
    uint16 i = 0;
    uint16 storeIndex = 0;
    uint16 recordSize = 0;
    FFIdClassRef = *pidClassPtr;

    //get all pids
    for (i = 0; ((i < DEM_MAX_NR_OF_PIDS_IN_FREEZEFRAME_DATA) && (!FFIdClassRef[i]->Arc_EOL)); i++) {
        if(FFIdClassRef[i]->PidOrDidUsePort == FALSE){
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
                        memset(&(*freezeFrame)->data[storeIndex], DEM_FREEZEFRAME_DEFAULT_VALUE, recordSize);
                    }
                    storeIndex += recordSize;

                }
                else{
                    memset(&(*freezeFrame)->data[storeIndex], DEM_FREEZEFRAME_DEFAULT_VALUE, recordSize);
                    storeIndex += recordSize;
                }

            }
            else{
                DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GET_FREEZEFRAME_ID, DEM_E_FF_TOO_BIG);
                break;
            }
        }
        else{
            //TODO:RTE should provide the port
            DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GET_FREEZEFRAME_ID, DEM_DSP_DID_USE_PORT_IS_TRUE);
            break;
        }
    }
    //store storeIndex,it will be used for judge whether FF contains valid data.
    *storeIndexPtr = storeIndex;

}
/*
 * Procedure:   getDidData
 * Description: get UDS FF data,only called by getFreezeFrameData()
 */
 static void getDidData(const Dem_PidOrDidType **const *didClassPtr, FreezeFrameRecType **freezeFrame, uint16 *storeIndexPtr)
{
    const Dem_PidOrDidType **FFIdClassRef = NULL;
    Std_ReturnType callbackReturnCode;
    uint16 i = 0;
    uint16 storeIndex = 0;
    uint16 recordSize = 0;
    Dcm_NegativeResponseCodeType errorCode;//should include Dcm_Lcfg.h

    FFIdClassRef = *didClassPtr;
    //get all dids
    for (i = 0; ((i < DEM_MAX_NR_OF_DIDS_IN_FREEZEFRAME_DATA) && (!FFIdClassRef[i]->Arc_EOL)); i++) {
        if(FFIdClassRef[i]->PidOrDidUsePort == FALSE){
            if(FFIdClassRef[i]->DidReadDataLengthFnc != NULL){
                callbackReturnCode = FFIdClassRef[i]->DidReadDataLengthFnc(&recordSize);
                if(callbackReturnCode != E_OK){
                    //if fail to read data length,discard the storage of FF
                    (*freezeFrame)->eventId = DEM_EVENT_ID_NULL;
                    DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GET_FREEZEFRAME_ID, DEM_READ_DATA_LENGTH_FAILED);
                    return;
                }
            }
            /* if no readDidDataLengthFunction, then try the PidOrDidSize. */
            else{
                recordSize = FFIdClassRef[i]->PidOrDidSize;
            }
            /* read out the did data */
            if ((storeIndex + recordSize + DEM_DID_IDENTIFIER_SIZE_OF_BYTES) <= DEM_MAX_SIZE_FF_DATA) {
                /* store DID */
                (*freezeFrame)->data[storeIndex] = (FFIdClassRef[i]->DidIdentifier>> 8) & 0xFFu;
                storeIndex++;
                (*freezeFrame)->data[storeIndex] = FFIdClassRef[i]->DidIdentifier & 0xFFu;
                storeIndex++;
                /* store data */
                if(FFIdClassRef[i]->DidConditionCheckReadFnc != NULL){
                    callbackReturnCode = FFIdClassRef[i]->DidConditionCheckReadFnc(&errorCode);
                    if ((callbackReturnCode == E_OK) && (errorCode == DCM_E_POSITIVERESPONSE)) {
                        if(FFIdClassRef[i]->DidReadFnc!= NULL){
                            callbackReturnCode = FFIdClassRef[i]->DidReadFnc(&(*freezeFrame)->data[storeIndex]);
                            if (callbackReturnCode != E_OK) {
                                memset(&(*freezeFrame)->data[storeIndex], DEM_FREEZEFRAME_DEFAULT_VALUE, recordSize);
                            }
                            storeIndex += recordSize;

                        }
                        else{
                            memset(&(*freezeFrame)->data[storeIndex], DEM_FREEZEFRAME_DEFAULT_VALUE, recordSize);
                            storeIndex += recordSize;
                        }

                    }
                    else{
                        memset(&(*freezeFrame)->data[storeIndex], DEM_FREEZEFRAME_DEFAULT_VALUE, recordSize);
                        storeIndex += recordSize;
                    }
                }
                else{
                    memset(&(*freezeFrame)->data[storeIndex], DEM_FREEZEFRAME_DEFAULT_VALUE, recordSize);
                    storeIndex += recordSize;
                }

            }
            else{
                DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GET_FREEZEFRAME_ID, DEM_E_FF_TOO_BIG);
                break;
            }
        }
        else{
            //TODO:RTE should provide the port
            DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_GET_FREEZEFRAME_ID, DEM_DSP_DID_USE_PORT_IS_TRUE);
            break;
        }
    }

    //store storeIndex,it will be used for judge whether FF contains valid data.
    *storeIndexPtr = storeIndex;
}

/*
 * Procedure:   storeOBDFreezeFrameDataMem
 * Description: store OBD FreezeFrame data record in primary memory
 */
#ifdef DEM_USE_MEMORY_FUNCTIONS
static boolean storeOBDFreezeFrameDataMem(const Dem_EventParameterType *eventParam, const FreezeFrameRecType *freezeFrame,
                                       FreezeFrameRecType* freezeFrameBuffer, uint32 freezeFrameBufferSize,
                                       Dem_DTCOriginType origin)
{
    boolean eventIdFound = FALSE;
    boolean eventIdFreePositionFound=FALSE;
    uint16 i;
    boolean dataStored = FALSE;
    (void)origin;

    /* Check if already stored */
    for (i = 0; (i<freezeFrameBufferSize) && (!eventIdFound); i++){
        eventIdFound = ((freezeFrameBuffer[i].eventId != DEM_EVENT_ID_NULL) \
            && (freezeFrameBuffer[i].kind == DEM_FREEZE_FRAME_OBD));
    }

    if (eventIdFound == FALSE) {
        /* find the first free position */
        for (i = 0; (i < freezeFrameBufferSize) && (!eventIdFreePositionFound); i++){
            eventIdFreePositionFound =  (freezeFrameBuffer[i].eventId == DEM_EVENT_ID_NULL);
        }
        /* if found,copy it to this position */
        if (eventIdFreePositionFound) {
            memcpy(&freezeFrameBuffer[i-1], freezeFrame, sizeof(FreezeFrameRecType));
        } else {
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON)
            /* if not found,do displacement */
            FreezeFrameRecType *freezeFrameLocal = NULL;
            if( lookupFreezeFrameForDisplacement(eventParam, &freezeFrameLocal, freezeFrameBuffer, freezeFrameBufferSize) ) {
                if(freezeFrameLocal != NULL){
                    memcpy(freezeFrameLocal, freezeFrame, sizeof(FreezeFrameRecType));
                    dataStored = TRUE;
                }
            }
#else
            DET_REPORTERROR(MODULE_ID_DEM, 0, DEM_STORE_FF_DATA_PRI_MEM_ID, DEM_E_PRI_MEM_FF_DATA_BUFF_FULL);
#endif
        }
    }
    return dataStored;
}
#endif
