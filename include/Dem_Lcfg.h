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








/*
 *  General requirements
 */
/** @req DEM128 */



#ifndef DEM_LCFG_H_
#define DEM_LCFG_H_
#include "Dem_Types.h"
#if defined(USE_DCM)
#include "Dcm_Types.h"	 /** @req DEM176.Dcm */
#include "Dcm_Lcfg.h"    	 /** @req DEM136.Dcm */
#endif
//#if defined(USE_NVRam)
#include "NvM_ConfigTypes.h"	 /** @req Dem147.Nvm */
//#endif

/*
 * Callback function prototypes
 */

// ClearEventAllowed
typedef Std_ReturnType (*Dem_CallbackClearEventAllowedFncType)(boolean *Allowed); /* @req DEM563 */

// EventDataChanged
typedef void (*Dem_CallbackEventDataChangedFncTypeWithId)(Dem_EventIdType EventId);/* @req DEM562 */
typedef void (*Dem_CallbackEventDataChangedFncTypeWithoutId)(void);

typedef union {
    Dem_CallbackEventDataChangedFncTypeWithId eventDataChangedWithId;
    Dem_CallbackEventDataChangedFncTypeWithoutId eventDataChangedWithoutId;
}Dem_CallbackEventDataChangedFncType;

typedef struct {
    boolean UsePort;
    Dem_CallbackEventDataChangedFncType CallbackEventDataChangedFnc;
}Dem_CallbackEventDataChangedType;

// InitMonitorForEvent
typedef Std_ReturnType (*Dem_CallbackInitMonitorForEventFncType)(Dem_InitMonitorReasonType InitMonitorReason); /** @req DEM256 */

// InitMonitorForFunction
typedef Std_ReturnType (*Dem_CallbackInitMonitorForFunctionFncType)(void); /** @req DEM258 */

// EventStatusChanged
typedef void (*Dem_CallbackEventStatusChangedFncTypeWithId)(Dem_EventIdType EventId, Dem_EventStatusExtendedType EventStatusOld, Dem_EventStatusExtendedType EventStatusNew); /** @req DEM259 */
typedef void (*Dem_CallbackEventStatusChangedFncTypeWithoutId)(Dem_EventStatusExtendedType EventStatusOld, Dem_EventStatusExtendedType EventStatusNew); /** @req DEM259 */

typedef union {
    Dem_CallbackEventStatusChangedFncTypeWithId eventStatusChangedWithId;
    Dem_CallbackEventStatusChangedFncTypeWithoutId eventStatusChangedWithoutId;
}Dem_CallbackEventStatusChangedFncType;

// DTCStatusChanged
typedef Std_ReturnType (*Dem_CallbackDTCStatusChangedFncType)(uint8 DTCStatusOld, uint8 DTCStatusNew); /** @req DEM260 */

// DIDServices /** @req DEM261 */
#if defined(USE_DCM)
typedef Std_ReturnType (*Dem_CallbackConditionCheckReadFncType)(Dcm_NegativeResponseCodeType *Nrc);
typedef Std_ReturnType (*Dem_CallbackReadDataFncType)(uint8 *Data);
typedef Std_ReturnType (*Dem_CallbackReadDataLength)(uint16 *DidLength);
#else
typedef Std_ReturnType (*Dem_CallbackConditionCheckReadFncType)(uint8 *Nrc);
typedef Std_ReturnType (*Dem_CallbackReadDataFncType)(uint8 *Data);
typedef Std_ReturnType (*Dem_CallbackReadDataLength)(uint16 *DidLength);
#endif

// GetExtendedDataRecord
typedef Std_ReturnType (*Dem_CallbackGetExtDataRecordFncType)(uint8 *ExtendedDataRecord); /** @req DEM262 */

// GetFaultDetectionCounter
typedef Std_ReturnType (*Dem_CallbackGetFaultDetectionCounterFncType)(sint8 *EventIdFaultDetectionCounter); /** @req DEM263 */

// GetPIDValue
typedef Std_ReturnType (*Dem_CallbackGetPIDValueFncType)(uint8 *DataValueBuffer); /** @req DEM326 */

typedef enum {
	DEM_NO_ELEMENT,
	DEM_OCCCTR,
	DEM_FAULTDETCTR,
	DEM_MAXFAULTDETCTR,
	DEM_CONFIRMATIONCNTR,
} Dem_InternalDataElementType;

typedef enum {
	DEM_UPDATE_RECORD_NO,
	DEM_UPDATE_RECORD_YES,
	DEM_UPDATE_RECORD_VOLATILE,
} Dem_UpdateRuleType;
/*
 * DemGeneral types
 */
/** @req DEM128 */

// 10.2.25 DemEnableCondition
typedef struct {
	boolean EnableConditionStatus;		//
	uint8	EnableConditionID;		// Optional
} Dem_EnableConditionType;

typedef struct {
    uint8 nofEnableConditions;
    const Dem_EnableConditionType **EnableCondition;
}Dem_EnableConditionGroupType;

// 10.2.30 DemExtendedDataRecordClass
typedef struct {
	uint16	RecordNumber;						// (1)
	uint16	DataSize;							// (1)
	Dem_UpdateRuleType UpdateRule;
	Dem_CallbackGetExtDataRecordFncType	CallbackGetExtDataRecord;// (1)  /** @req DEM139 */
	Dem_InternalDataElementType InternalDataElement;
} Dem_ExtendedDataRecordClassType; /** @req DEM135 */

// 10.2.13 DemExtendedDataClass
typedef struct {
	const Dem_ExtendedDataRecordClassType *const ExtendedDataRecordClassRef[DEM_MAX_NR_OF_RECORDS_IN_EXTENDED_DATA+1]; // (1..253)
} Dem_ExtendedDataClassType; /** @req DEM135 */

// 10.2.8 DemPidOrDid
typedef struct {
	Dem_CallbackConditionCheckReadFncType		DidConditionCheckReadFnc;	// (0..1)
	const uint16								DidIdentifier;				// (0..1)
	Dem_CallbackReadDataLength				DidReadDataLengthFnc;		// (0..1)
	Dem_CallbackReadDataFncType				DidReadFnc;					// (0..1)
	const uint8								PidIdentifier;				// (0..1)
	uint8									PidOrDidSize;				// (1)
	boolean									PidOrDidUsePort;			// (1) Not used in current implementation	
	Dem_CallbackGetPIDValueFncType			PidReadFnc;					// (0..1)
	boolean									Arc_EOL;
} Dem_PidOrDidType; /** @req DEM136 */

// 10.2.18 DemFreezeFrameClass
typedef struct {
	Dem_FreezeFrameKindType FFKind;			// (1)
	const Dem_PidOrDidType 	**FFIdClassRef; 	// (1..255)/** @req DEM039 */
} Dem_FreezeFrameClassType; /** @req DEM136 */


// 10.2.4 DemIndicator
typedef struct {
	uint16	Indicator;		// (1)
} Dem_IndicatorType; /** @req DEM129 */

// 10.2.28 DemNvramBlockId
typedef struct {

//#if defined(USE_NVRam)
	const NvM_BlockDescriptorType *BlockDescriptor;/** @req FIM083 *//** @req Dem697_Conf AutoSAR 4.2.0*/
//#endif

} Dem_NvramBlockIdType;/** @req Dem147 *//** @req Dem696_Conf AutoSAR 4.2.0*/

/*
 * DemConfigSetType types
 */

// 10.2.6 DemCallbackDTCStatusChanged
typedef struct {
	Dem_CallbackDTCStatusChangedFncType CallbackDTCStatusChangedFnc;	// (0..1)
} Dem_CallbackDTCStatusChangedType; /** @req DEM140 */

// 10.2.26 DemCallbackInitMForF
typedef struct {
	Dem_CallbackInitMonitorForFunctionFncType	CallbackInitMForF;		// (0..1)
} Dem_CallbackInitMForFType;

// 10.2.17 DemDTCClass
typedef struct {
	uint32									DTC;						// (1)
	uint8									DTCFunctionalUnit;			// (1)
	Dem_DTCKindType							DTCKind;					// (1)
	const Dem_CallbackDTCStatusChangedType	*CallbackDTCStatusChanged;	// (0..*)
	const Dem_CallbackInitMForFType			*CallbackInitMForF;			// (0..*)
	// Dem_DTCSeverityType					DTCSeverity					// (0..1)  Optional
	boolean									Arc_EOL;
} Dem_DTCClassType; /** @req DEM132 */

// 10.2.5 DemCallbackEventStatusChanged
typedef struct {
	Dem_CallbackEventStatusChangedFncType	CallbackEventStatusChangedFnc;	// (0..1)
	boolean UsePort;
	boolean Arc_EOL;
} Dem_CallbackEventStatusChangedType; /** @req DEM140 */

// 10.2.27 DemCallbackInitMForE
//typedef struct {
//	Dem_CallbackInitMonitorForEventFncType	CallbackInitMForEFnc;	// (0..1)
//} Dem_CallbackInitMforEType; /** @req DEM130 */

// 10.2.15
typedef struct {
	Dem_IndicatorStatusType	IndicatorBehaviour;			// (1)
	Dem_IndicatorType		*LinkedIndicator;			// (1)
} Dem_IndicatorAttributeType; /** @req DEM133 */

// 10.2.23 DemPreDebounceMonitorInternal
typedef struct {
	Dem_CallbackGetFaultDetectionCounterFncType	CallbackGetFDCntFnc;	// (1)
} Dem_PreDebounceMonitorInternalType; /** @req DEM146 */

// 10.2.21 DemPreDebounceCounterBased
typedef struct {
	boolean	JumpUp;
	boolean	JumpDown;
	uint16	IncrementStepSize;
	uint16	DecrementStepSize;
    sint16  JumpDownValue;
    sint16  JumpUpValue;
    sint16  FailedThreshold;
    sint16  PassedThreshold;

} Dem_PreDebounceCounterBasedType; /** @req DEM144 */

// 10.2.22 DemPreDebounceFrequencyBased
//typedef struct {
//	// TODO: Fill out
//} Dem_PreDebounceFrequencyBasedType;
typedef uint8 Dem_PreDebounceFrequencyBasedType;

// 10.2.24 DemPreDebounceTimeBased
//typedef struct {
//	// TODO: Fill out
//} Dem_PreDebounceTimeBasedType;
typedef uint8 Dem_PreDebounceTimeBasedType;

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

// 10.2.14 DemEventClass
typedef struct {
	boolean						ConsiderPtoStatus;									// (1)
	Dem_DTCOriginType 		    EventDestination;                                   // (1 Arcore specific)
	uint8						EventPriority;										// (1)
	boolean						FFPrestorageSupported;								// (1)
	boolean						HealingAllowed;										// (1)
	Dem_OperationCycleIdType	OperationCycleRef;									// (1)
	Dem_OperationCycleIdType	HealingCycleRef;									// (1)
	Dem_OperationCycleIdType    ConfirmationCycleRef;                               // (1)
	uint8                       ConfirmationCycleCounterThreshold;                  // (1)
	uint8						HealingCycleCounter;									// (0..1) Optional
	const Dem_EnableConditionGroupType	*EnableConditionGroupRef;							// (0..*) Optional
	const Dem_PreDebounceAlgorithmClassType	*PreDebounceAlgorithmClass;				// (0..255) (Only 0..1 supported)
	const Dem_IndicatorAttributeType		*IndicatorAttribute;					// (0..255)
//	Dem_OEMSPecific
} Dem_EventClassType; /** @req DEM131 */

typedef struct
{
	uint8	FreezeFrameRecordNumber[DEM_MAX_RECORD_NUMBERS_IN_FF_REC_NUM_CLASS + 1];
}Dem_FreezeFrameRecNumClass;

// 10.2.12 DemEventParameter
typedef struct {
	uint16										EventID;					// (1)
	Dem_EventKindType							EventKind;					// (1)
	uint8										MaxNumberFreezeFrameRecords;// (1)
	const Dem_EventClassType					*EventClass;				// (1)
	const Dem_ExtendedDataClassType				*ExtendedDataClassRef;		// (0..1)
	const Dem_FreezeFrameClassType				*FreezeFrameClassRef; 		// (0..1) (Only 0..1 supported)/** @req DEM021 */
	const Dem_CallbackInitMonitorForEventFncType CallbackInitMforE;			// (0..1)
	const Dem_CallbackEventStatusChangedType	*CallbackEventStatusChanged;// (0..)
	const Dem_CallbackClearEventAllowedFncType  CallbackClearEventAllowed; // (0..1)
	const Dem_CallbackEventDataChangedType      *CallbackEventDataChanged;  // (0..1)
	const Dem_DTCClassType						*DTCClassRef;				// (0..1)
	const Dem_FreezeFrameRecNumClass			*FreezeFrameRecNumClassRef; // (1)
	boolean										Arc_EOL;
} Dem_EventParameterType; /** @req DEM130 */



// 10.2.19 DemGroupOfDTC
//typedef struct {
//	// TODO: Fill out
//} Dem_GroupOfDtcType;
typedef uint8 Dem_GroupOfDtcType;

// 10.2.10 DemOemIdClass
typedef struct {
	uint8	OemID;
} Dem_OemIdClassType; /** @req DEM141 */

// 10.2.9 DemConfigSet
typedef struct {
	const Dem_EventParameterType	*EventParameter;	// (0..65535)
	const Dem_DTCClassType 			*DTCClass;			// (1..16777214)
	const Dem_GroupOfDtcType		*GroupOfDtc;		// (1.16777214)
	const Dem_OemIdClassType		*OemIdClass;		// (0..*)
	const Dem_EnableConditionType   *EnableCondition;
	const Dem_FreezeFrameClassType  *GlobalOBDFreezeFrameClassRef;
} Dem_ConfigSetType; /** @req DEM130 */

// 10.2.2 Dem
typedef struct {
	const Dem_ConfigSetType *ConfigSet;	//	(1)
} Dem_ConfigType;


typedef struct {
	Dem_EventIdType         eventId;
	Dem_FreezeFrameKindType kind;
	uint16                  dataSize;
	uint8                   recordNumber;
#if (DEM_EVENT_DISPLACEMENT_SUPPORT == STD_ON) && !defined(DEM_EVENT_DISPLACEMENT_STRATEGY_EXTENSION)
    uint32                  timeStamp;
#endif
	uint8                   data[DEM_MAX_SIZE_FF_DATA];
} FreezeFrameRecType;

// Types for storing different event aging counter
typedef struct {
	Dem_EventIdType		eventId;
	uint8				agingCounter;/** @req Dem019 */
} HealingRecType;

/*
 * Make the DEM_Config visible for others.
 */
//extern const Dem_ConfigType DEM_Config;

#endif /*DEM_LCFG_H_*/
