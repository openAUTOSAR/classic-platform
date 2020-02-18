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

#include "Dem.h"
#include "Dem_Types.h"
#include "Dem_Lcfg.h"
#include "Dem_Internal.h"
#if defined(USE_DEM_EXTENSION)
#include "Dem_Extension.h"
#endif

/* Local defines */
#define DEM_UDS_TEST_FAILED_TRESHOLD  (sint32)127
#define DEM_UDS_TEST_PASSED_TRESHOLD (sint32)(-128)
#define DEM_UDS_FDC_RANGE (sint32)(DEM_UDS_TEST_FAILED_TRESHOLD - (DEM_UDS_TEST_PASSED_TRESHOLD))

/* Local variables */
#if defined(DEM_USE_TIME_BASE_PREDEBOUNCE)
/* Buffer for time based debounce */
static TimeBaseStatusType DemTimeBaseBuffer[DEM_NOF_TIME_BASE_PREDEB];
#endif

/* Local functions */
/**
 * Translates internal FDC to external FDC
 * @param fdcInternal
 * @param pdVars
 * @return FDC
 */
static sint8 fdcInternalToUDSFdc(sint16 fdcInternal, const Dem_PreDebounceCounterBasedType* pdVars)
{
    /* Map the internal counter to the corresponding UDS fdc. I.e. map from [FailedThreshold, PassedThreshold] to [-128, 127]. */
    sint32 pdRange = (sint32)((sint32)pdVars->FailedThreshold - (sint32)pdVars->PassedThreshold);
    sint32 temp = (DEM_UDS_FDC_RANGE*((sint32)((sint32)fdcInternal - (sint32)pdVars->PassedThreshold))) + (DEM_UDS_TEST_PASSED_TRESHOLD*pdRange);
    return (sint8)(temp/pdRange);
}

/*
 * Procedure:   preDebounceNone
 * Description: Returns the result of the debouncing.
 */
static Dem_EventStatusType preDebounceNone(const Dem_EventStatusType reportedStatus) {
    /* @req DEM437 */
    Dem_EventStatusType returnCode;
    switch (reportedStatus) {
    case DEM_EVENT_STATUS_FAILED:
    case DEM_EVENT_STATUS_PASSED:
        // Already debounced, do nothing.
        break;

    default:
        // NOTE: What to do with PREFAIL and PREPASSED on no debouncing?
        DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_PREDEBOUNCE_NONE_ID, DEM_E_PARAM_DATA);
        break;
    }

    returnCode = reportedStatus;
    return returnCode;
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
            if ((pdVars->JumpUp ==TRUE) && (statusRecord->fdcInternal < pdVars->JumpUpValue)) {
                statusRecord->fdcInternal = pdVars->JumpUpValue;/* @req 4.2.2/SWS_DEM_00423 */
            }
            if (((sint32)statusRecord->fdcInternal + (sint32)pdVars->IncrementStepSize) < pdVars->FailedThreshold) {
                statusRecord->fdcInternal += pdVars->IncrementStepSize;/*lint !e734 OK since we check above that it will not overflow*/ /* @req DEM418 */
            } else {
                statusRecord->fdcInternal = pdVars->FailedThreshold;
            }
        }
        break;

    case DEM_EVENT_STATUS_PREPASSED:
        if (statusRecord->fdcInternal > pdVars->PassedThreshold) {
            if ((pdVars->JumpDown==TRUE) && (statusRecord->fdcInternal > pdVars->JumpDownValue)) {
                statusRecord->fdcInternal = pdVars->JumpDownValue;/* @req 4.2.2/SWS_DEM_00425 */
            }
            if (((sint32)statusRecord->fdcInternal - (sint32)pdVars->DecrementStepSize) > pdVars->PassedThreshold) {
                statusRecord->fdcInternal -= pdVars->DecrementStepSize;/*lint !e734 OK since we check above that it will not overflow*/ /* @req DEM419 */
            } else {
                statusRecord->fdcInternal = pdVars->PassedThreshold;
            }
        }
        break;

    case DEM_EVENT_STATUS_FAILED:
        statusRecord->fdcInternal = pdVars->FailedThreshold; /* @req DEM420 */
        break;

    case DEM_EVENT_STATUS_PASSED:
        statusRecord->fdcInternal = pdVars->PassedThreshold; /* @req DEM421 */
        break;

    default:
        DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_PREDEBOUNCE_COUNTER_BASED_ID, DEM_E_PARAM_DATA);
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

#if defined(DEM_USE_TIME_BASE_PREDEBOUNCE)
/**
 * Calculates FDC based on timer
 * @param debounceTimer
 * @param pdVars
 * @return FDC
 */
static sint8 getTimeBasedFDC(const TimeBaseStatusType *debounceTimer, const Dem_PreDebounceTimeBasedType *pdVars)
{
    /* @req DEM427 */
    sint8 FDC = 0;
    sint64 temp;
    if( TRUE == debounceTimer->started ) {
        if( TRUE == debounceTimer->failureCounting ) {
            /* 0 - pdVars->TimeFailedThreshold -> 0 - 127*/
            if( 0UL != pdVars->TimeFailedThreshold ) {
                temp = ((sint64)debounceTimer->debounceTime * DEM_UDS_TEST_FAILED_TRESHOLD) / pdVars->TimeFailedThreshold;
                temp = MIN(temp, DEM_UDS_TEST_FAILED_TRESHOLD);
                FDC = (sint8)temp;
            } else {
                FDC = (sint8)DEM_UDS_TEST_FAILED_TRESHOLD;
            }

        } else {
            /* 0 - pdVars->TimePassedThreshold -> 0 - -128*/
            if(0UL != pdVars->TimePassedThreshold) {
                temp = ((sint64)debounceTimer->debounceTime * DEM_UDS_TEST_PASSED_TRESHOLD) / pdVars->TimePassedThreshold;
                temp = MAX(temp, DEM_UDS_TEST_PASSED_TRESHOLD);
                FDC = (sint8)temp;
            } else {
                FDC = (sint8)DEM_UDS_TEST_PASSED_TRESHOLD;
            }
        }
    }
    return FDC;
}

/**
 * Handles starting time based debouncing
 * @param reportedStatus
 * @param eventParam
 * @return
 */
static Dem_EventStatusType preDebounceTimeBased(Dem_EventStatusType reportedStatus, const Dem_EventParameterType *eventParam)
{
    Dem_EventStatusType ret = reportedStatus;
    const Dem_PreDebounceTimeBasedType* pdVars = eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceAlgorithm.PreDebounceTimeBased;
    DemTimeBaseBuffer[pdVars->Index].EventId = eventParam->EventID;
    switch (reportedStatus) {
        case DEM_EVENT_STATUS_FAILED:
        case DEM_EVENT_STATUS_PREFAILED:
            /* @req DEM428 */
            /* @req DEM429 */
            if( (FALSE == DemTimeBaseBuffer[pdVars->Index].started) || (FALSE == DemTimeBaseBuffer[pdVars->Index].failureCounting) ) {
                DemTimeBaseBuffer[pdVars->Index].started = TRUE;
                DemTimeBaseBuffer[pdVars->Index].failureCounting = TRUE;
                if( (DEM_EVENT_STATUS_FAILED == reportedStatus) || (0UL == pdVars->TimeFailedThreshold) ) {
                    /* @req DEM431 */
                    DemTimeBaseBuffer[pdVars->Index].debounceTime = pdVars->TimeFailedThreshold;
                    DemTimeBaseBuffer[pdVars->Index].errorReported = TRUE;
                    DemTimeBaseBuffer[pdVars->Index].counterReset = FALSE;
                    ret = DEM_EVENT_STATUS_FAILED;
                } else {
                    DemTimeBaseBuffer[pdVars->Index].debounceTime = 0;
                    DemTimeBaseBuffer[pdVars->Index].errorReported = FALSE;
                    DemTimeBaseBuffer[pdVars->Index].counterReset = TRUE;
                }
            }
            break;
        case DEM_EVENT_STATUS_PASSED:
        case DEM_EVENT_STATUS_PREPASSED:
            /* @req DEM432 */
            /* @req DEM433 */
            if( (FALSE == DemTimeBaseBuffer[pdVars->Index].started) || (TRUE == DemTimeBaseBuffer[pdVars->Index].failureCounting) ) {
                DemTimeBaseBuffer[pdVars->Index].started = TRUE;
                DemTimeBaseBuffer[pdVars->Index].failureCounting = FALSE;
                if( (DEM_EVENT_STATUS_PASSED == reportedStatus) || (0UL == pdVars->TimePassedThreshold) ) {
                    /* @req DEM435 */
                    DemTimeBaseBuffer[pdVars->Index].debounceTime = pdVars->TimePassedThreshold;
                    DemTimeBaseBuffer[pdVars->Index].errorReported = TRUE;
                    DemTimeBaseBuffer[pdVars->Index].counterReset = FALSE;
                    ret = DEM_EVENT_STATUS_PASSED;
                } else {
                    DemTimeBaseBuffer[pdVars->Index].debounceTime = 0;
                    DemTimeBaseBuffer[pdVars->Index].errorReported = FALSE;
                    DemTimeBaseBuffer[pdVars->Index].counterReset = TRUE;
                }
            }

            break;
        default:
            DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_PREDEBOUNCE_COUNTER_BASED_ID, DEM_E_PARAM_DATA);
            break;

    }

    return ret;
}
#endif


/* Exported functions */

#if defined(DEM_USE_TIME_BASE_PREDEBOUNCE)
/**
 * Main function for time based predebounce
 */
void TimeBasedDebounceMainFunction(void)
{
    /* Handle time based predebounce */
    /* @req DEM426 */
    const Dem_EventParameterType *eventParam;
    EventStatusRecType *eventStatusRec;
    for( uint16 idx = 0; idx < DEM_NOF_TIME_BASE_PREDEB; idx++ ) {
        eventParam = NULL;
        eventStatusRec = NULL;
        if( DEM_EVENT_ID_NULL != DemTimeBaseBuffer[idx].EventId ) {
            lookupEventIdParameter(DemTimeBaseBuffer[idx].EventId, &eventParam); /*lint !e934 eventParam only used in this function  */
            lookupEventStatusRec(DemTimeBaseBuffer[idx].EventId, &eventStatusRec); /*lint !e934 eventStatusRec only used in this function  */
            if( (NULL != eventParam) && (NULL != eventStatusRec) &&
                    (NULL != eventParam->EventClass->PreDebounceAlgorithmClass) &&
                    (DEM_PRE_DEBOUNCE_TIME_BASED == eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceName)) {
                const Dem_PreDebounceTimeBasedType* pdVars = eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceAlgorithm.PreDebounceTimeBased;

                if( TRUE == operationCycleIsStarted(eventParam->EventClass->OperationCycleRef) ) {
                    if( TRUE == DemTimeBaseBuffer[idx].started ) {
                        if(FALSE == DemTimeBaseBuffer[idx].counterReset) {
                            if( TRUE == DemTimeBaseBuffer[idx].failureCounting ) {
                                if( DemTimeBaseBuffer[idx].debounceTime < pdVars->TimeFailedThreshold ) {
                                    DemTimeBaseBuffer[idx].debounceTime += DEM_TASK_TIME;
                                }
                                /* @req DEM430 */
                                if( DemTimeBaseBuffer[idx].debounceTime >= pdVars->TimeFailedThreshold ) {
                                    /* FAILED! */
                                    if(E_OK == handleEvent(DemTimeBaseBuffer[idx].EventId, DEM_EVENT_STATUS_FAILED) ) {
                                        DemTimeBaseBuffer[idx].errorReported = TRUE;
                                    }
                                }


                            } else {
                                if( DemTimeBaseBuffer[idx].debounceTime < pdVars->TimePassedThreshold ) {
                                    DemTimeBaseBuffer[idx].debounceTime += DEM_TASK_TIME;
                                }
                                /* @req DEM434 */
                                if( DemTimeBaseBuffer[idx].debounceTime >= pdVars->TimePassedThreshold ) {
                                    /* PASSED! */
                                    if( E_OK == handleEvent(DemTimeBaseBuffer[idx].EventId, DEM_EVENT_STATUS_PASSED) ) {
                                        DemTimeBaseBuffer[idx].errorReported = TRUE;
                                    }
                                }
                            }
                        } else {
                            DemTimeBaseBuffer[idx].counterReset = FALSE;
                        }
                        eventStatusRec->UDSFdc = getTimeBasedFDC(&DemTimeBaseBuffer[idx], pdVars);
                        eventStatusRec->maxUDSFdc = MAX(eventStatusRec->UDSFdc, eventStatusRec->maxUDSFdc);
                    }
                } else {
                    /* Operation cycle is not started.
                     * Cancel timer. */
                    DemTimeBaseBuffer[idx].started = FALSE;
                    DemTimeBaseBuffer[idx].errorReported = FALSE;
                }
            }
        }
    }
}
#endif

/**
 * Resets debounce counter of event
 * @param eventStatusRec
 */
void resetDebounceCounter(EventStatusRecType *eventStatusRec)
{
    sint8 startFDC = getDefaultUDSFdc(eventStatusRec->eventId);
    eventStatusRec->UDSFdc = startFDC;/* @req DEM344 */
    eventStatusRec->maxUDSFdc = startFDC;
    eventStatusRec->fdcInternal = 0;
#if defined(DEM_USE_TIME_BASE_PREDEBOUNCE)
    const Dem_EventParameterType *eventParam = NULL;
    lookupEventIdParameter(eventStatusRec->eventId, &eventParam); /*lint !e934 eventParam only used in this function  */
    if( NULL != eventParam ) {
        if( (NULL != eventParam->EventClass->PreDebounceAlgorithmClass) &&
            (DEM_PRE_DEBOUNCE_TIME_BASED == eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceName) ) {
            DemTimeBaseBuffer[eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceAlgorithm.PreDebounceTimeBased->Index].started = FALSE;
            DemTimeBaseBuffer[eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceAlgorithm.PreDebounceTimeBased->Index].errorReported = FALSE;
            DemTimeBaseBuffer[eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceAlgorithm.PreDebounceTimeBased->Index].debounceTime = 0UL;
        }
    }
#endif
}




/*
 * Procedure:   getFaultDetectionCounter
 * Description: Returns pre debounce counter of "eventId" in "counter" and return value E_OK if
 *              the counter was available else E_NOT_OK.
 */
Std_ReturnType getFaultDetectionCounter(Dem_EventIdType eventId, sint8 *counter)
{
    Std_ReturnType returnCode = E_NOT_OK;
    const Dem_EventParameterType *eventParam;
    EventStatusRecType *eventStatusRec = NULL_PTR;
    lookupEventStatusRec(eventId, &eventStatusRec); /*lint !e934 eventStatusRec only used in this function  */
    lookupEventIdParameter(eventId, &eventParam);  /*lint !e934 eventParam only used in this function  */
    if ((eventParam != NULL_PTR) && (NULL_PTR != eventStatusRec) && (TRUE == eventStatusRec->isAvailable)) {
        if (eventParam->EventClass->PreDebounceAlgorithmClass != NULL_PTR) {
            switch (eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceName)
            {
            case DEM_NO_PRE_DEBOUNCE:
                if (eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceAlgorithm.PreDebounceMonitorInternal != NULL_PTR) {
                    if (eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceAlgorithm.PreDebounceMonitorInternal->CallbackGetFDCntFnc != NULL_PTR) {
                        /* @req DEM204 None */
                        /* @req DEM264 */
                        /* @req DEM439 */
                        returnCode = eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceAlgorithm.PreDebounceMonitorInternal->CallbackGetFDCntFnc(counter);
                    }
                }
                break;

            case DEM_PRE_DEBOUNCE_COUNTER_BASED:
                *counter = eventStatusRec->UDSFdc; /* @req DEM204 Counter */
                returnCode = E_OK;
                break;

#if defined(DEM_USE_TIME_BASE_PREDEBOUNCE)
            case DEM_PRE_DEBOUNCE_TIME_BASED:
                /* Map timer to FDC */
                *counter = getTimeBasedFDC(&DemTimeBaseBuffer[eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceAlgorithm.PreDebounceTimeBased->Index],
                        eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceAlgorithm.PreDebounceTimeBased);
                returnCode = E_OK;
                break;
#endif

            default:
                DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_GETFAULTDETECTIONCOUNTER_ID, DEM_E_PARAM_DATA);
                break;
            }
        }
    }

    return returnCode;
}

/**
 * Initializes time based debounce buffer
 */
#if defined(DEM_USE_TIME_BASE_PREDEBOUNCE)
void InitTimeBasedDebounce(void)
{
    for( uint16 idx = 0; idx < DEM_NOF_TIME_BASE_PREDEB; idx++ ) {
        DemTimeBaseBuffer[idx].EventId = DEM_EVENT_ID_NULL;
        DemTimeBaseBuffer[idx].started = FALSE;
    }
}
#endif

/**
 * Gets the default UDS fdc
 * @param eventId
 * @return
 */
sint8 getDefaultUDSFdc(Dem_EventIdType eventId)
{
    sint8 udsFdc = 0;
    const Dem_EventParameterType *eventParam = NULL_PTR;
    lookupEventIdParameter(eventId, &eventParam); /*lint !e934 eventParam only used in this function  */
    if( NULL_PTR != eventParam ) {
        if (eventParam->EventClass->PreDebounceAlgorithmClass != NULL_PTR) {
            switch (eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceName) {
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

/**
 * Runs predebounce for event
 * @param reportedEventStatus
 * @param eventStatusRecPtr
 * @param eventParam
 * @return calculated eventStatus
 */
Dem_EventStatusType RunPredebounce(Dem_EventStatusType reportedEventStatus, EventStatusRecType *eventStatusRecPtr, const Dem_EventParameterType *eventParam)
{
    Dem_EventStatusType eventStatus = reportedEventStatus;
    if (eventParam->EventClass->PreDebounceAlgorithmClass != NULL_PTR) {
        switch (eventParam->EventClass->PreDebounceAlgorithmClass->PreDebounceName) {
            case DEM_NO_PRE_DEBOUNCE:
                eventStatus = preDebounceNone(reportedEventStatus);
                break;
            case DEM_PRE_DEBOUNCE_COUNTER_BASED:
                eventStatus = preDebounceCounterBased(reportedEventStatus, eventStatusRecPtr);
                break;
#if defined(DEM_USE_TIME_BASE_PREDEBOUNCE)
            case DEM_PRE_DEBOUNCE_TIME_BASED:
                eventStatus = preDebounceTimeBased(reportedEventStatus, eventParam);
                break;
#endif
            default:
                DET_REPORTERROR(DEM_MODULE_ID, 0, DEM_UPDATE_EVENT_STATUS_ID, DEM_E_NOT_IMPLEMENTED_YET);
                break;
        }
    }
    return eventStatus;
}
