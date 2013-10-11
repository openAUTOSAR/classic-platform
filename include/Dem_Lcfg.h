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

// InitMonitorForEvent
typedef Std_ReturnType (*Dem_CallbackInitMonitorForEventFncType)(Dem_InitMonitorKindType InitMonitorKind); /** @req DEM256 */

// InitMonitorForFunction
typedef Std_ReturnType (*Dem_CallbackInitMonitorForFunctionFncType)(void); /** @req DEM258 */

// EventStatusChanged
typedef Std_ReturnType (*Dem_CallbackEventStatusChangedFncType)(Dem_EventStatusExtendedType EventStatusOld, Dem_EventStatusExtendedType EventStatusNew); /** @req DEM259 */

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

typedef enum{
	FF_STORAGE_CONDITION_WRONG = 0,	//FF storage condition is wrong
	PREFAILED = 1,						//store FF when the event status is prefailed
	FAILED = 2							//store FF when the event status is failed
}Dem_FreezeFrameStorageConditonType; /** @req Dem001_private */

typedef enum {
	DEM_NO_ELEMENT,
	DEM_OCCCTR,
	DEM_FAULTDETCTR,
	DEM_MAXFAULTDETCTR,
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
	// uint16	EnableConditionID;		// Optional
} Dem_EnableConditionType;

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
	const uint8								PidIndentifier;				// (0..1)
	uint8									PidOrDidSize;				// (1)
	boolean									PidOrDidUsePort;			// (1) Not used in current implementation	
	Dem_CallbackGetPIDValueFncType			PidReadFnc;					// (0..1)
	boolean									Arc_EOL;
} Dem_PidOrDidType; /** @req DEM136 */

// 10.2.18 DemFreezeFrameClass
typedef struct {
	Dem_FreezeFrameKindType FFKind;			// (1)
	uint8					FFRecordNumber;	// (1)/** @req DEM040 */
	Dem_FreezeFrameStorageConditonType FFStorageCondition;/** @req Dem001_private */
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
} Dem_CallbackEventStatusChangedType; /** @req DEM140 */

// 10.2.27 DemCallbackInitMForE
typedef struct {
	Dem_CallbackInitMonitorForEventFncType	CallbackInitMForEFnc;	// (0..1)
} Dem_CallbackInitMforEType; /** @req DEM130 */

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
	uint8	CountInStepSize;
	uint8	CountOutStepSize;
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
	const Dem_DTCOriginType 		EventDestination[DEM_MAX_NR_OF_EVENT_DESTINATION+1];// (0..4)
	uint8						EventPriority;										// (1)
	boolean						FFPrestorageSupported;								// (1)
	boolean						HealingAllowed;										// (1)
	Dem_OperationCycleIdType	OperationCycleRef;									// (1)
	Dem_OperationCycleIdType	HealingCycleRef;									// (1)
	uint8						HealingCycleCounter;									// (0..1) Optional
	uint8						ConfirmationCycleCounterThreshold;
//	const Dem_EnableConditionType	*EnableConditionRef;							// (0..*) Optional
	const Dem_PreDebounceAlgorithmClassType	*PreDebounceAlgorithmClass;				// (0..255) (Only 0..1 supported)
	const Dem_IndicatorAttributeType		*IndicatorAttribute;					// (0..255)
//	Dem_OEMSPecific
} Dem_EventClassType; /** @req DEM131 */

// 10.2.12 DemEventParameter
typedef struct {
	uint16										EventID;					// (1)
	Dem_EventKindType							EventKind;					// (1)
	const Dem_EventClassType					*EventClass;				// (1)
	const Dem_ExtendedDataClassType				*ExtendedDataClassRef;		// (0..1)
	const Dem_FreezeFrameClassType				**FreezeFrameClassRef; //[DEM_MAX_NR_OF_CLASSES_IN_FREEZEFRAME_DATA+1];		// (0..255) (Only 0..1 supported)/** @req DEM021 */
	const Dem_CallbackInitMforEType				*CallbackInitMforE;			// (0..1)
	const Dem_CallbackEventStatusChangedType	*CallbackEventStatusChanged;// (0..*)
	const Dem_DTCClassType						*DTCClassRef;				// (0..1)
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
} Dem_ConfigSetType; /** @req DEM130 */

// 10.2.2 Dem
typedef struct {
	const Dem_ConfigSetType *ConfigSet;	//	(1)
} Dem_ConfigType;


typedef uint16 ChecksumType;
typedef struct {
	Dem_EventIdType         eventId;
	Dem_FreezeFrameKindType kind;
	uint16                  occurrence;
	uint16                  dataSize;
	uint8                   recordNumber;
	uint32                  timeStamp;
	uint8                   data[DEM_MAX_SIZE_FF_DATA];
	ChecksumType            checksum;
} FreezeFrameRecType;

// Types for storing different event aging counter
typedef struct {
	Dem_EventIdType		eventId;
	uint8				agingCounter;/** @req Dem019 */
	ChecksumType		checksum;
} HealingRecType;

/*
 * Make the DEM_Config visible for others.
 */
extern const Dem_ConfigType DEM_Config;

#endif /*DEM_LCFG_H_*/
