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








#ifndef DEM_LCFG_H_
#define DEM_LCFG_H_

#include "Dem_Types.h"


typedef void *(*FunctionNameDefType)( uint16 x, uint8 y , uint8 z);	// TODO: Define this properly

/*
 * Callback function prototypes
 */
typedef void (*Dem_CallbackGetExtDataRecordFncType)( uint8 *ExtendedDataRecord );


/*
 * DemGeneral types
 */

// 10.2.25 DemEnableCondition
typedef struct {
	boolean EnableConditionStatus;		// (Pre+Post)
	// uint16	EnableConditionID;		// (Pre+Post) Optional
} Dem_EnableConditionType;

// 10.2.30 DemExtendedDataRecordClass
typedef struct {
	// Unique number of the record
	uint16	RecordNumber;						// (Pre+Post)	(1)
	// Size of data
	uint16	DataSize;							// (Pre+Post)	(1)
	// Callback function
	Dem_CallbackGetExtDataRecordFncType	CallbackGetExtDataRecord;// (1)
} Dem_ExtendedDataRecordClassType;

// 10.2.13 DemExtendedDataClass
typedef struct {
	const Dem_ExtendedDataRecordClassType *const ExtendedDataRecordClassRef[DEM_MAX_NR_OF_RECORDS_IN_EXTENDED_DATA+1]; // (--)	(1..253)
} Dem_ExtendedDataClassType;

// 10.2.8 DemPidOrDid
typedef struct {
	boolean				PidOrDidUsePort;		// (--)			(1)
	uint8				PidOrDidSize;			// (Pre+Post)	(1)
	uint16				DidIdentifier;			// (---)		(0..1)
	FunctionNameDefType	DidConditionCheckReadFnc;// (Pre)		(0..1)
	FunctionNameDefType	DidReadDataLengthFnc;	// (Pre)		(0..1)
	FunctionNameDefType	DidReadFnc;				// (Pre)		(0..1)
	uint8				PidIndentifier;			// (Pre+Post)	(0..1)
	FunctionNameDefType	PidReadFnc;				// (Pre)		(0..1)
} Dem_PidOrDidType;

// 10.2.18 DemFreezeFrameClass
typedef struct {
	Dem_FreezeFrameKindType FFKind;			// (Pre+Post)	(1)
//	uint8					FFRecordNumber;	// (Pre+Post)	(1) Optional
//	const Dem_PidOrDidType 	*FFIdClassRef; 	// (Pre+Post)	(1..255) Optional?
} Dem_FreezeFrameClassType;

// 10.2.4 DemIndicator
typedef struct {
	uint16	Indicator;		// (Pre)	(1)
} Dem_IndicatorType;

// 10.2.28 DemNvramBlockId
typedef struct {
	// TODO: Fill out
} Dem_NvramBlockIdType;

// 10.2.11	DemOperationCycleTgt
typedef struct {
	Dem_OperationCycleType	OperationCycle;		// (Pre)	(1)
} Dem_OperationCycleTgtType;

/*
 * DemConfigSetType types
 */

// 10.2.6 DemCallbackDTCStatusChanged
typedef struct {
	FunctionNameDefType CallbackDTCStatusChangedFnc;	// (Pre)	(0..1)
} Dem_CallbackDTCStatusChangedType;

// 10.2.26 DemCallbackInitMForF
typedef struct {
	FunctionNameDefType	CallbackInitMForF;			// (Pre)	(0..1)
} Dem_CallbackInitMForFType;

// 10.2.17 DemDTCClass
typedef struct {
	uint32									DTC;						// (Pre+Post)	(1)
	uint8									DTCFunctionUnit;			// (Pre+Post)	(1)
	Dem_DTCKindType							DTCKind;					// (Pre+Post)	(1)
	const Dem_CallbackDTCStatusChangedType	*CallbackDTCStatusChanged;	// (0..*)
	const Dem_CallbackInitMForFType			*CallbackInitMForF;			// (0..*)
	// Dem_DTCSeverityType					DTCSeverity					// (0..1)  Optional
} Dem_DTCClassType;

// 10.2.5 DemCallbackEventStatusChanged
typedef struct {
	FunctionNameDefType	CallbackEventStatusChangedFnc;	// (Pre)	(0..1)
} Dem_CallbackEventStatusChangedType;

// 10.2.27 DemCallbackInitMForE
typedef struct {
	FunctionNameDefType	CallbackInitMForEFnc;	// (--)	(0..1)
} Dem_CallbackInitMforEType;

// 10.2.14 DemEventClass
typedef struct {
	boolean	ConsiderPtoStatus;								// (--) 		(1)
	const Dem_DTCOriginType EventDestination[DEM_MAX_NR_OF_EVENT_DESTINATION+1];	// (Pre+Post)	(0..4)
	uint8	EventPriority;									// (Pre+Post)	(1)
	boolean	FFPrestorageSupported;							// (Pre+Post)	(1)
	boolean	HealingAllowed;									// (Pre+Post)	(1)
//	uint8	HealingCycleCounter;							// (Pre+Post)	(0..1) Optional
//	const Dem_EnableConditionType	*EnableConditionRef;	// (Pre+Post)	(0..*) Optional
//	const Dem_OperationCycleTgtType *HealingCycleRef;		// (Pre+Post)	(0..1) Optional
	const Dem_OperationCycleTgtType	*OperationCycleRef;		// (Pre+Post)	(0..1)
	/*
	 * TODO: Fill out
	 * Dem_IndicatorAttribute
	 * Dem_OEMSPecific
	 * Dem_PredebounceAlgorithmClass
	 */
} Dem_EventClassType;

// 10.2.12 DemEventParameter
typedef struct {
	uint16										EventID;					// (Pre)(1)
	Dem_EventKindType							EventKind;					// (--)	(1)
	const Dem_EventClassType					*EventClass;				// (--)	(1)
	const Dem_ExtendedDataClassType				*ExtendedDataClassRef;		// (--) (0..1)
	const Dem_FreezeFrameClassType				*FreezeFrameClassRef;		// (--)	(0..255)
	const Dem_CallbackInitMforEType				*CallbackInitMforE;			// (--)	(0..1)
	const Dem_CallbackEventStatusChangedType	*CallbackEventStatusChanged;// (Pre)(0..*)
	const Dem_DTCClassType						*DTCClass;					// (--)	(0..1)
	boolean										Arc_EOL;
} Dem_EventParameterType;

// 10.2.19 DemGroupOfDTC
typedef struct {
	// TODO: Fill out
} Dem_GroupOfDtcType;

// 10.2.10 DemOemIdClass
typedef struct {
	uint8	OemID;	// (Pre+Post)
} Dem_OemIdClassType;

// 10.2.9 DemConfigSet
typedef struct {
	const Dem_EventParameterType	*EventParameter;	// (0..65535)
	const Dem_DTCClassType 			*DTCClass;			// (1..16777214)
	const Dem_GroupOfDtcType		*GroupOfDtc;		// (1.16777214)
	const Dem_OemIdClassType		*OemIdClass;		// (0..*)
} Dem_ConfigSetType;

// 10.2.2 Dem
typedef struct {
	const Dem_ConfigSetType *ConfigSet;	//	(1)
} Dem_ConfigType;


/*
 * Make the DEM_Config visible for others.
 */
extern const Dem_ConfigType DEM_Config;


#endif /*DEM_LCFG_H_*/
