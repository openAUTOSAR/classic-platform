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

#ifndef DEM_INTERNAL_H_
#define DEM_INTERNAL_H_

#if defined(DEM_FREEZE_FRAME_CAPTURE_EXTENSION)
#define DEM_EXT_STORE_FF_BIT                    (uint8)(1u<<0u)
#define DEM_EXT_CLEAR_BEFORE_STORE_FF_BIT       (uint8)(1u<<1u)
#endif
#if defined(DEM_EXTENDED_DATA_CAPTURE_EXTENSION)
#define DEM_EXT_STORE_EXT_DATA_BIT              (uint8)(1u<<2u)
#define DEM_EXT_CLEAR_BEFORE_STORE_EXT_DATA_BIT (uint8)(1u<<3u)
#endif

#include "Dem.h"

#if  ( DEM_DEV_ERROR_DETECT == STD_ON )
#if defined(USE_DET)
#include "Det.h"
#endif
/** @req DEM117 */
#define DET_REPORTERROR(_x,_y,_z,_q) (void)Det_ReportError(_x, _y, _z, _q)

#else
#define DET_REPORTERROR(_x,_y,_z,_q)
#endif

#if (DEM_NOF_TIME_BASE_PREDEB > 0)
#define DEM_USE_TIME_BASE_PREDEBOUNCE
#endif

// For keeping track of the events status
/* NOTE: Do not change EventStatusRecType without also changing generation of measurement tags */
typedef struct {
    const Dem_EventParameterType *eventParamRef;
    uint32                      timeStamp;
    Dem_EventIdType             eventId;
    uint16                      occurrence;             /** @req DEM011 */
    sint16                      fdcInternal;            /** @req DEM414 */
    sint8                       UDSFdc;
    sint8                       maxUDSFdc;
    uint8                       failureCounter;
    uint8                       agingCounter;
    Dem_EventStatusExtendedType eventStatusExtended;    /** @req DEM006 */
    uint8                       extensionDataStoreBitfield;
    boolean                     failedDuringAgingCycle:1;  /*lint !e46 *//*structure must remain the same, field type should be _Bool, unsigned int or signed int [MISRA 2004 6.4, 2012 6.1]*/
    boolean                     passedDuringAgingCycle:1; /*lint !e46 *//*structure must remain the same,field type should be _Bool, unsigned int or signed int [MISRA 2004 6.4, 2012 6.1]*/
    boolean                     failedDuringFailureCycle:1; /*lint !e46 *//*structure must remain the same,field type should be _Bool, unsigned int or signed int [MISRA 2004 6.4, 2012 6.1]*/
    boolean                     passedDuringFailureCycle:1; /*lint !e46 *//*structure must remain the same,field type should be _Bool, unsigned int or signed int [MISRA 2004 6.4, 2012 6.1]*/
    boolean                     errorStatusChanged:1; /*lint !e46 *//*structure must remain the same,field type should be _Bool, unsigned int or signed int [MISRA 2004 6.4, 2012 6.1]*/
    boolean                     extensionDataChanged:1; /*lint !e46 *//*structure must remain the same,field type should be _Bool, unsigned int or signed int [MISRA 2004 6.4, 2012 6.1]*/
    boolean                     indicatorDataChanged:1; /*lint !e46 *//*structure must remain the same,field type should be _Bool, unsigned int or signed int [MISRA 2004 6.4, 2012 6.1]*/
    boolean                     isAvailable:1; /*lint !e46 *//*structure must remain the same,field type should be _Bool, unsigned int or signed int [MISRA 2004 6.4, 2012 6.1]*/
    boolean                     isDisabled:1; /*lint !e46 *//*structure must remain the same,field type should be _Bool, unsigned int or signed int [MISRA 2004 6.4, 2012 6.1]*/
} EventStatusRecType;

// Types for storing different event data on event memory
/* ******************************************************************************************************
 * WARNING: DO NOT CHANGE THESE STRUCTURES WITHOUT UPDATED THE DEM GENERATOR!!
 * ******************************************************************************************************/
typedef union {
    struct {
#if (DEM_USE_TIMESTAMPS == STD_ON)
    uint32                      timeStamp;
#endif
    Dem_EventIdType             eventId;
    uint16                      occurrence;
    Dem_EventStatusExtendedType eventStatusExtended;
#if defined(DEM_FAILURE_PROCESSING_DEM_INTERNAL)
    uint8                       failureCounter;
#endif
#if defined(DEM_AGING_PROCESSING_DEM_INTERNAL)
    uint8                       agingCounter;/* @req DEM492 */
#endif
    }EventData;
    struct {
        /* NOTE: This must be kept smaller than the event data */
        uint16  magic;
        boolean overflow;
    }AdminData;
} EventRecType;

typedef struct {
#if (DEM_USE_TIMESTAMPS == STD_ON)
    uint32              timeStamp;
#endif
    Dem_EventIdType     eventId;
    uint8               data[DEM_MAX_SIZE_EXT_DATA];
} ExtDataRecType;

#if defined(DEM_USE_TIME_BASE_PREDEBOUNCE)
typedef struct {
    uint32 debounceTime;
    Dem_EventIdType EventId;
    boolean started:1;/*lint !e46 *//*structure must remain the same, field type should be _Bool, unsigned int or signed int [MISRA 2004 6.4, 2012 6.1]*/
    boolean failureCounting:1;/*lint !e46 *//*structure must remain the same, field type should be _Bool, unsigned int or signed int [MISRA 2004 6.4, 2012 6.1]*/
    boolean errorReported:1;/*lint !e46 *//*structure must remain the same, field type should be _Bool, unsigned int or signed int [MISRA 2004 6.4, 2012 6.1]*/
    boolean counterReset:1;/*lint !e46 *//*structure must remain the same, field type should be _Bool, unsigned int or signed int [MISRA 2004 6.4, 2012 6.1]*/
}TimeBaseStatusType;
#endif

typedef struct {
//    uint32 UDSDTC;
    uint32 OBDDTC;
}PermanentDTCType;

void lookupEventStatusRec(Dem_EventIdType eventId, EventStatusRecType **const eventStatusRec);
boolean operationCycleIsStarted(Dem_OperationCycleIdType opCycle);

#if (DEM_UNIT_TEST == STD_ON)
void demZeroPriMemBuffers(void);
void demZeroSecMemBuffers(void);
void demZeroPermMemBuffers(void);
void demZeroPreStoreFFMemBuffer(void);
void demZeroIumprBuffer(void);
void demSetIgnitionCounterToMax(void);
void demSetDenominatorToMax(Dem_RatioIdType ratioId);
void demSetNumeratorToMax(Dem_RatioIdType ratioId);
#endif
void lookupEventIdParameter(Dem_EventIdType eventId, const Dem_EventParameterType **const eventIdParam);
Std_ReturnType handleEvent(Dem_EventIdType eventId, Dem_EventStatusType eventStatus);

/* Debouncing functions */
void resetDebounceCounter(EventStatusRecType *eventStatusRec);
Std_ReturnType getFaultDetectionCounter(Dem_EventIdType eventId, sint8 *counter);
sint8 getDefaultUDSFdc(Dem_EventIdType eventId);
Dem_EventStatusType RunPredebounce(Dem_EventStatusType reportedEventStatus, EventStatusRecType *eventStatusRecPtr, const Dem_EventParameterType *eventParam);
#if defined(DEM_USE_TIME_BASE_PREDEBOUNCE)
void InitTimeBasedDebounce(void);
void TimeBasedDebounceMainFunction(void);
#endif

#if defined(USE_DEM_EXTENSION)
boolean Dem_LookupEventOfUdsDTC(uint32 dtc, EventStatusRecType **eventStatusRec);
#endif

#define DEM_TRISTATE int8_t
#define DEM_T_NULL   -1
#define DEM_T_FALSE   0
#define DEM_T_TRUE    1

#endif /* DEM_INTERNAL_H_ */
