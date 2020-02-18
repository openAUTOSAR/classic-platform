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

#ifndef DEM_LCFG_H_
#define DEM_LCFG_H_
#include "Dem_Types.h"
#if defined(USE_DCM)
#include "Dcm_Types.h"	 /** @req DEM176.Dcm */
#endif

/*
 * Callback function prototypes
 */

// ClearEventAllowed
typedef Std_ReturnType (*Dem_CallbackClearEventAllowedFncType)(boolean *Allowed); /* @req DEM563 */

// EventDataChanged
typedef Std_ReturnType (*Dem_CallbackEventDataChangedFncTypeWithId)(Dem_EventIdType EventId);/* @req DEM562 */
typedef Std_ReturnType (*Dem_CallbackEventDataChangedFncTypeWithoutId)(void);


typedef union {
    Dem_CallbackEventDataChangedFncTypeWithId eventDataChangedWithId;
    Dem_CallbackEventDataChangedFncTypeWithoutId eventDataChangedWithoutId;
}Dem_CallbackEventDataChangedFncType;

typedef struct {
    boolean UsePort;
    Dem_CallbackEventDataChangedFncType CallbackEventDataChangedFnc;
}Dem_CallbackEventDataChangedType;

// InitMonitorForEvent
typedef Std_ReturnType (*Dem_CallbackInitMonitorForEventFncType)(Dem_InitMonitorReasonType InitMonitorReason); /** @req DEM256 *//** @req DEM376 *//** @req DEM003 */

// InitMonitorForFunction
typedef Std_ReturnType (*Dem_CallbackInitMonitorForFunctionFncType)(void); /** !req DEM258 */

// EventStatusChanged
typedef Std_ReturnType (*Dem_CallbackEventStatusChangedFncTypeWithId)(Dem_EventIdType EventId, Dem_EventStatusExtendedType EventStatusOld, Dem_EventStatusExtendedType EventStatusNew); /** @req DEM259 */
typedef Std_ReturnType (*Dem_CallbackEventStatusChangedFncTypeWithoutId)(Dem_EventStatusExtendedType EventStatusOld, Dem_EventStatusExtendedType EventStatusNew); /** @req DEM259 */

typedef union {
    Dem_CallbackEventStatusChangedFncTypeWithId eventStatusChangedWithId;
    Dem_CallbackEventStatusChangedFncTypeWithoutId eventStatusChangedWithoutId;
}Dem_CallbackEventStatusChangedFncType;

// DTCStatusChanged
typedef Std_ReturnType (*Dem_CallbackDTCStatusChangedFncType)(uint8 DTCStatusOld, uint8 DTCStatusNew); /** !req DEM260 */

// DIDServices /** @req DEM261 *//*  */
typedef Std_ReturnType (*Dem_CallbackReadDataFncType)(uint8 *Data);/* @req DEM564 */

// GetFaultDetectionCounter
typedef Std_ReturnType (*Dem_CallbackGetFaultDetectionCounterFncType)(sint8 *EventIdFaultDetectionCounter); /** @req DEM263 */

typedef uint8 Dem_InternalDataElementType;
#define DEM_NO_ELEMENT          0u
#define DEM_OCCCTR              1u
#define DEM_FAULTDETCTR         2u
#define DEM_MAXFAULTDETCTR      3u
#define DEM_CONFIRMATIONCNTR    4u
#define DEM_AGINGCTR            5u
#define DEM_OVFLIND             6u

typedef enum {
    DEM_UPDATE_RECORD_NO,
    DEM_UPDATE_RECORD_YES,
    DEM_UPDATE_RECORD_VOLATILE,
} Dem_UpdateRuleType;


#define DEM_NON_EMISSION_RELATED            0u/* Used for events without DTC and events with non-emission related DTC */
#define DEM_EMISSION_RELATED                1u
#define DEM_EMISSION_RELATED_MIL_ACTIVATING 2u

typedef uint32 Dem_Arc_EventDTCKindType;

/*
 * DemGeneral types
 */

// 10.2.25 DemEnableCondition
typedef struct {
    boolean EnableConditionStatus;		//
    uint8	EnableConditionID;		// Optional
} Dem_EnableConditionType;

typedef struct {
    uint8 nofEnableConditions;
    const Dem_EnableConditionType * const *EnableCondition;
}Dem_EnableConditionGroupType;

// 10.2.30 DemExtendedDataRecordClass
typedef struct {
    uint8	                            RecordNumber;				// (1)
    uint16	                            DataSize;					// (1)
    Dem_UpdateRuleType                  UpdateRule;                 /* @req DEM466 */
    Dem_CallbackReadDataFncType         CallbackGetExtDataRecord;   // (1)
    Dem_InternalDataElementType         InternalDataElement;        /* @req DEM469 */
} Dem_ExtendedDataRecordClassType;

// 10.2.13 DemExtendedDataClass
typedef struct {
    const Dem_ExtendedDataRecordClassType *const ExtendedDataRecordClassRef[DEM_MAX_NR_OF_RECORDS_IN_EXTENDED_DATA+1]; // (1..253)
} Dem_ExtendedDataClassType;

// 10.2.8 DemPidOrDid
typedef struct {
    const uint16							DidIdentifier;				// (0..1)
    Dem_CallbackReadDataFncType				DidReadFnc;					// (0..1)
    const uint8								PidIdentifier;				// (0..1)
    uint8									PidOrDidSize;				// (1)
    Dem_CallbackReadDataFncType 			PidReadFnc;					// (0..1)
    boolean									Arc_EOL;
} Dem_PidOrDidType;

// 10.2.18 DemFreezeFrameClass
typedef struct {
    const Dem_PidOrDidType  * const * FFIdClassRef;     // (1..255)/** @req DEM039 *//** @req DEM040 */
    uint16 NofXids;
    Dem_FreezeFrameKindType FFKind;			// (1)
} Dem_FreezeFrameClassType;

/*
 * DemConfigSetType types
 */

// 10.2.6 DemCallbackDTCStatusChanged
typedef struct {
    Dem_CallbackDTCStatusChangedFncType CallbackDTCStatusChangedFnc;	// (0..1)
} Dem_CallbackDTCStatusChangedType;

// 10.2.26 DemCallbackInitMForF
typedef struct {
    Dem_CallbackInitMonitorForFunctionFncType	CallbackInitMForF;		// (0..1)
} Dem_CallbackInitMForFType;

// 10.2.17 DemDTCClass
typedef struct {
#if defined(HOST_TEST)
    Arc_Dem_DTC						        *DTCRef;					// (1)
#else
    const Arc_Dem_DTC                       *DTCRef;                    // (1)
#endif
    const Dem_EventIdType                   *Events;                    // List of events referencing DTC
    uint16                                  DTCIndex;                   // Index of the DTC
    uint16                                  NofEvents;                  // Number of events referencing DTC
//	const Dem_CallbackDTCStatusChangedType	*CallbackDTCStatusChanged;	// (0..*)
//	const Dem_CallbackInitMForFType			*CallbackInitMForF;			// (0..*)
    Dem_DTCSeverityType					    DTCSeverity;				// (0..1)  Optional /* @req DEM033 */
    boolean									Arc_EOL;
} Dem_DTCClassType;

// 10.2.5 DemCallbackEventStatusChanged
typedef struct {
    Dem_CallbackEventStatusChangedFncType	CallbackEventStatusChangedFnc;	// (0..1)
    boolean UsePort;
    boolean Arc_EOL;
} Dem_CallbackEventStatusChangedType;


typedef enum {
    DEM_FAILURE_CYCLE_EVENT = 0,
    DEM_FAILURE_CYCLE_INDICATOR
}DemIndicatorFailureSourceType;

typedef struct {
    uint16                          IndicatorBufferIndex;
    uint8                           IndicatorId;
    Dem_IndicatorStatusType	        IndicatorBehaviour;/* @req DEM511 */
    uint8                           IndicatorFailureCycleThreshold;/* @req DEM500 */
    Dem_OperationCycleStateType     *IndicatorFailureCycle;/* @req DEM504 */
    uint8                           IndicatorHealingCycleThreshold;
    Dem_OperationCycleStateType     IndicatorHealingCycle;
    DemIndicatorFailureSourceType   IndicatorFailureCycleSource;
    boolean							*IndicatorValid;
    boolean                         Arc_EOL;
} Dem_IndicatorAttributeType;

// 10.2.23 DemPreDebounceMonitorInternal
typedef struct {
    Dem_CallbackGetFaultDetectionCounterFncType	CallbackGetFDCntFnc;	// (1)
} Dem_PreDebounceMonitorInternalType;

// 10.2.22 DemPreDebounceFrequencyBased
typedef uint8 Dem_PreDebounceFrequencyBasedType;

// 10.2.24 DemPreDebounceTimeBased
typedef struct{
    uint32 TimeFailedThreshold;
    uint32 TimePassedThreshold;
    uint16 Index;
}Dem_PreDebounceTimeBasedType;

// 10.2.20
typedef struct {
    Dem_PreDebounceNameType						PreDebounceName;				// (1)
    union {
    const Dem_PreDebounceMonitorInternalType	*PreDebounceMonitorInternal;	// (0..1)
    const Dem_PreDebounceCounterBasedType		*PreDebounceCounterBased;		// (0..1)
    const Dem_PreDebounceFrequencyBasedType		*PreDebounceFrequencyBased;		// (0..1)
    const Dem_PreDebounceTimeBasedType			*PreDebounceTimeBased;			// (0..1)
    } PreDebounceAlgorithm;
} Dem_PreDebounceAlgorithmClassType;

typedef struct {
    uint8 Threshold;
} Arc_FailureCycleCounterThreshold;

// 10.2.14 DemEventClass
typedef struct {
    const Arc_FailureCycleCounterThreshold  *FailureCycleCounterThresholdRef;       // (1) /* @req DEM529 */
    const Dem_EnableConditionGroupType      *EnableConditionGroupRef;               // (0..*) Optional /* @req DEM446 */
    const Dem_PreDebounceAlgorithmClassType *PreDebounceAlgorithmClass;             // (0..255) (Only 0..1 supported) /* @req DEM413 */
    const Dem_IndicatorAttributeType        *IndicatorAttribute;                    // (0..255)
    const boolean                           *EventAvailableByCalibration;
    const uint8						        *AgingCycleCounterThresholdPtr;				// (0..1) Optional /* @req DEM493 */
    const Dem_OperationCycleIdType          *FailureCycleRef;                       // (1) /* @req DEM528 */
    boolean						            ConsiderPtoStatus;						// (1)
    Dem_DTCOriginType 		                EventDestination;                       // (1 Arccore specific)
    uint8						            EventPriority;							// (1) /* @req DEM382 */
    boolean						            FFPrestorageSupported;					// (1) /* @req DEM002 */
    boolean						            AgingAllowed;							// (1)
    Dem_OperationCycleIdType	            OperationCycleRef;						// (1)
    Dem_OperationCycleIdType	            AgingCycleRef;	    					// (1) /* @req DEM494 */
    Dem_EventOBDReadinessGroup				OBDReadinessGroup;						// (0..1) Optional
//	Dem_OEMSPecific
} Dem_EventClassType;

typedef struct
{
    uint8	FreezeFrameRecordNumber[DEM_MAX_RECORD_NUMBERS_IN_FF_REC_NUM_CLASS + 1];
}Dem_FreezeFrameRecNumClass;

typedef uint8 Dem_FreezeFrameClassTypeRefIndex;

#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
typedef struct {
    Dem_FreezeFrameClassTypeRefIndex FreezeFrameClassIdx;
}Dem_CombinedDTCCalibType;

typedef struct {
    const Dem_FreezeFrameRecNumClass                *FreezeFrameRecNumClassRef;
    const Dem_ExtendedDataClassType                 *ExtendedDataClassRef;
    const Dem_CombinedDTCCalibType                  *Calib;
    const Dem_DTCClassType                          *DTCClassRef;
    Dem_EventIdType                                 CombinedEventId;
    uint8                                           MaxNumberFreezeFrameRecords;
    uint8                                           Priority;
    Dem_DTCOriginType                               MemoryDestination;
}Dem_CombinedDTCCfgType;
#endif

// 10.2.12 DemEventParameter
typedef struct {
    const Dem_EventClassType					    *EventClass;				    // (1)
    const Dem_ExtendedDataClassType				    *ExtendedDataClassRef;		    // (0..1) /* @req DEM460 */
    //const Dem_FreezeFrameClassType				    *FreezeFrameClassRef; 		// (0..1) /* @req DEM460 */
    const Dem_FreezeFrameClassTypeRefIndex          *FreezeFrameClassRefIdx;        // (0..1) /* @req DEM460 */
    const Dem_CallbackInitMonitorForEventFncType    CallbackInitMforE;			    // (0..1)
    const Dem_CallbackEventStatusChangedType	    *CallbackEventStatusChanged;    // (0..)
    const Dem_CallbackClearEventAllowedFncType      CallbackClearEventAllowed;      // (0..1)
    const Dem_CallbackEventDataChangedType          *CallbackEventDataChanged;      // (0..1)
    const Dem_DTCClassType						    *DTCClassRef;				    // (0..1)
    const Dem_FreezeFrameRecNumClass			    *FreezeFrameRecNumClassRef;     // (1)
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
    uint16                                          CombinedDTCCID;
#endif
    uint16                                          EventID;                        // (1)
    Dem_EventKindType                               EventKind;                      // (1)
    uint8                                           MaxNumberFreezeFrameRecords;    // (1) /* @req DEM337 *//* @req DEM582 */
    Dem_Arc_EventDTCKindType                        *EventDTCKind;
    boolean										    Arc_EOL;
} Dem_EventParameterType;


typedef struct {
    uint8       EventListSize;
    const Dem_EventIdType *EventList;
}Dem_IndicatorType;

// 10.2.19 DemGroupOfDTC
typedef struct {
    uint32  DemGroupDTCs;
    boolean Arc_EOL;
}Dem_GroupOfDtcType;

// 10.2.9 DemConfigSet
typedef struct {
    const Dem_EventParameterType	*EventParameter;	// (0..65535)
    const Dem_DTCClassType 			*DTCClass;			// (1..16777214)
	const Dem_GroupOfDtcType		*GroupOfDtc;
    const Dem_EnableConditionType   *EnableCondition;
    const Dem_FreezeFrameClassType  *GlobalOBDFreezeFrameClassRef;/* @req DEM291 */
    const Dem_FreezeFrameClassType  *GlobalFreezeFrameClassRef;
    const Dem_IndicatorType         *Indicators;
#if defined(DEM_EVENT_COMBINATION_DEM_EVCOMB_TYPE1)
    const Dem_CombinedDTCCfgType    *CombinedDTCConfig;
#endif
} Dem_ConfigSetType;

// 10.2.2 Dem
typedef struct {
    const Dem_ConfigSetType *ConfigSet;	//	(1)
} Dem_ConfigType;

/* ******************************************************************************************************
 * WARNING: DO NOT CHANGE THESE STRUCTURES WITHOUT UPDATED THE DEM GENERATOR!!
 * ******************************************************************************************************/
typedef struct {
#if (DEM_USE_TIMESTAMPS == STD_ON)
    uint32                  timeStamp;
#endif
    uint16                  dataSize;
    Dem_EventIdType         eventId;
    Dem_FreezeFrameKindType kind;
    uint8                   recordNumber;
    uint8                   data[DEM_MAX_SIZE_FF_DATA];
} FreezeFrameRecType;

// Types for storing different event aging counter
typedef struct {
    Dem_EventIdType		eventId;
    uint8				agingCounter;/** @req Dem019 */
} HealingRecType;
/* ******************************************************************************************************
 *
 * ******************************************************************************************************/
#if (DEM_OBD_SUPPORT == STD_ON)
typedef struct {
	Dem_RatioIdType RatioId;
	Dem_IUMPRGroup IumprGroup;
	const Dem_EventParameterType *DiagnosticEventRef;
	Dem_RatioKindType RatioKind;
} Dem_RatioType;

typedef struct {
	uint16 value;
	boolean incrementedThisDrivingCycle;
} Dem_RatioNumeratorType;

typedef struct {
	uint16 value;
	boolean incrementedThisDrivingCycle;
	boolean isLocked;
} Dem_RatioDenominatorType;

typedef struct {
	uint16 value;
	boolean incrementedThisDrivingCycle;
} Dem_RatioGeneralDenominatorType;

typedef struct {
	Dem_RatioNumeratorType numerator;
	Dem_RatioDenominatorType denominator;
} Dem_RatioStatusType;

typedef struct {
	uint16 numerator;
	uint16 denominator;
} Dem_RatioStatusNvmType;

#if defined(DEM_USE_IUMPR)
#ifndef DEM_IUMPR_REGISTERED_COUNT
#define DEM_IUMPR_REGISTERED_COUNT 0
#endif

typedef struct {
	Dem_RatioStatusNvmType ratios[DEM_IUMPR_REGISTERED_COUNT];
	uint16 generalDenominatorCount;
	uint16 ignitionCycleCount;
} Dem_RatiosNvm;

extern const Dem_RatioType Dem_RatiosList[DEM_IUMPR_REGISTERED_COUNT];
#endif
typedef uint8 Dem_IumprDenomCondIdType;
typedef uint8 Dem_IumprDenomCondStatusType;

typedef struct {
	Dem_IumprDenomCondIdType condition;
	Dem_IumprDenomCondStatusType status;
} Dem_IumprDenomCondType;

#define DEM_IUMPR_DEN_COND_COLDSTART             (Dem_IumprDenomCondIdType) 0x02
#define DEM_IUMPR_DEN_COND_EVAP                  (Dem_IumprDenomCondIdType) 0x03
#define DEM_IUMPR_DEN_COND_500MI                 (Dem_IumprDenomCondIdType) 0x04
#define DEM_IUMPR_GENERAL_INDIVIDUAL_DENOMINATOR (Dem_IumprDenomCondIdType) 0x05
#define DEM_IUMPR_GENERAL_OBDCOND                (Dem_IumprDenomCondIdType) 0x06

#define DEM_IUMPR_DEN_STATUS_NOT_REACHED         (Dem_IumprDenomCondStatusType) 0x00
#define DEM_IUMPR_DEN_STATUS_REACHED             (Dem_IumprDenomCondStatusType) 0x01
#define DEM_IUMPR_DEN_STATUS_INHIBITED           (Dem_IumprDenomCondStatusType) 0x02
#endif

/*
 * Make the DEM_Config visible for others.
 */
extern const Dem_ConfigType DEM_Config;

#endif /*DEM_LCFG_H_*/
