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








#ifndef COM_TYPES_H_
#define COM_TYPES_H_

#include "ComStack_Types.h"
#include "Com_Cfg.h"

typedef uint8 Com_PduGroupIdType;
typedef uint16 Com_SignalIdType;
typedef uint8 Com_SignalGroupIdType;

typedef enum {
	IMMEDIATE,
	DEFERRED,
} Com_IPduSignalProcessingMode;

typedef enum {
	RECEIVE,
	SEND
} Com_IPduDirection;

typedef enum {
	BOOLEAN,
	UINT8,
	UINT16,
	UINT32,
	UINT8_N,
	SINT8,
	SINT16,
	SINT32
} Com_SignalType;

typedef enum {
	PENDING,
	TRIGGERED,
} ComTransferProperty_type;

typedef enum {
	DIRECT,
	MIXED,
	NONE,
	PERIODIC,
} ComTxModeMode_type;


typedef enum {
	ALWAYS,
	MASKED_NEW_DIFFERS_MASKED_OLD,
	MASKED_NEW_DIFFERS_X,
	MASKED_NEW_EQUALS_X,
	NEVER,
	NEW_IS_OUTSIDE,
	NEW_IS_WITHIN,
	ONE_EVERY_N,
} ComFilterAlgorithm_type;

typedef enum {
	BIG_ENDIAN,
	LITTLE_ENDIAN,
	OPAQUE,
} ComSignalEndianess_type;

typedef enum {
	COM_TIMEOUT_DATA_ACTION_NONE,
	COM_TIMEOUT_DATA_ACTION_REPLACE
} ComRxDataTimeoutAction_type;

/*
typedef enum {

} ComTransmissionMode_type;
*/

// Shortcut macros
#define M_BOOLEAN boolean
#define M_UINT8 uint8
#define M_UINT16 uint16
#define M_UINT32 uint32
#define M_UINT8_N uint8
#define M_SINT8 sint8
#define M_SINT16 sint16
#define M_SINT32 sint32

#define SignalTypeToSize(type,length) \
	(type == UINT8   ? sizeof(uint8) : \
	type == UINT16  ? sizeof(uint16) : \
	type == UINT32  ? sizeof(uint32) : \
	type == UINT8_N  ? sizeof(uint8) * length : \
	type == SINT8   ? sizeof(sint8) : \
	type == SINT16  ? sizeof(sint16) : \
	type == SINT32  ? sizeof(sint32) : sizeof(boolean)) \


typedef struct {
	ComFilterAlgorithm_type ComFilterAlgorithm;
	uint32 ComFilterMask;
	uint32 ComFilterMax;
	uint32 ComFilterMin;
	uint32 ComFilterOffset;
	uint32 ComFilterPeriodFactor;
	uint32 ComFilterX;


	uint32 ComFilterArcN;
	uint32 ComFilterArcNewValue;
	uint32 ComFilterArcOldValue;

} ComFilter_type;


typedef struct {
	/* Starting position (bit) of the signal within the IPDU.
	 *
	 * Context:
	 *   - Send and receive.
	 *   - Required.
	 *
	 * Comment: Range 0 to 63.
	 */
	const uint8 ComBitPosition;

	/* The size of the signal in bits.
	 *
	 * Context:
	 *   - Send and receive.
	 *   - Required.
	 *
	 * Comment: Range 0 to 64.
	 */
	const uint8 ComBitSize;

	/* Identifier for the signal.
	 *
	 * Context:
	 *   - Send and receive.
	 *   - Required.
	 *
	 * Comment: Should be the same value as the index in the COM signal array.
	 */
	const uint8 ComHandleId;

	/* Callback function used when an invalid signal is received.
	 *
	 * Context:
	 *   - Receive.
	 *   - Not required.
	 */
	// ComInvalidNotification;

	/*
	 *
	 * Context:
	 *   - Send and receive.
	 *   - Not required.
	 */
	//uint8 ComSignalDataInvalidValue;

	/* Defines the endianess of the signal's network representation.
	 *
	 * Context:
	 *   - Send and receive.
	 *   - Required.
	 */
	const ComSignalEndianess_type ComSignalEndianess;

	/*
	 * Value used to initialized this signal.
	 *
	 * Context:
	 *   - Send
	 *   - Required
	 */
	const uint32 ComSignalInitValue;

	/* The number of bytes if the signal has type UINT8_N;
	 *
	 * Context:
	 *   - Send and receive.
	 *   - Required if type of signal is UINT8_N
	 *
	 * Comment: Range 1 to 8.
	 */
	const uint8 ComSignalLength;

	/* Defines the type of the signal
	 *
	 * Context:
	 *   - Send and receive.
	 *   - Required.
	 */
	const Com_SignalType ComSignalType;


	/* Filter for this signal
	 *
	 * Context:
	 *   - Send.
	 *   - Required.
	 */
	const ComFilter_type ComFilter;

	/* Pointer to the shadow buffer of the signal group that this group signal is contained in.
	 *
	 * Comment: This is initialized by Com_Init() and should not be configured.
	 */
	//void *Com_Arc_ShadowBuffer;


	/* IPDU id of the IPDU that this signal belongs to.
	 *
	 * Comment: This is initialized by Com_Init() and should not be configured.
	 */

	//const uint8 ComIPduHandleId;
	//const uint8 ComSignalUpdated;

	const uint8 Com_Arc_EOL;
} ComGroupSignal_type;

typedef struct {

	/* Starting position (bit) of the signal within the IPDU.
	 *
	 * Context:
	 *   - Send and receive.
	 *   - Required.
	 *
	 * Comment: Range 0 to 63.
	 */
	const uint8 ComBitPosition;

	/* The size of the signal in bits.
	 *
	 * Context:
	 *   - Send and receive.
	 *   - Required.
	 *
	 * Comment: Range 0 to 64.
	 */
	const uint8 ComBitSize;


	/* Action to be taken if an invalid signal is received.
	 *
	 * Context:
	 *   -
	 */
	// ComDataInvalidAction;

	/* Notification function for error notification.
	 *
	 * Context:
	 *   - Send.
	 *   - Not required.
	 *
	 */
	void (*ComErrorNotification) (void);

	/* First timeout period for deadline monitoring.
	 *
	 * Context:
	 *   - Receive
	 *   - Not required.
	 */
	const uint32 ComFirstTimeoutFactor;

	/* Identifier for the signal.
	 *
	 * Context:
	 *   - Send and receive.
	 *   - Required.
	 *
	 * Comment: Should be the same value as the index in the COM signal array.
	 */
	const uint8 ComHandleId;

	/* Callback function used when an invalid signal is received.
	 *
	 * Context:
	 *   - Receive.
	 *   - Not required.
	 */
	// ComInvalidNotification;

	/* Tx and Rx notification function.
	 *
	 * Context:
	 *   - Send and receive.
	 *   - Not required.
	 */
	void (*ComNotification) (void);

	/* Action to be performed when a reception timeout occurs.
	 *
	 * Context:
	 *   - Receive.
	 *   - Required.
	 */
	const ComRxDataTimeoutAction_type ComRxDataTimeoutAction;

	/*
	 *
	 * Context:
	 *   - Send and receive.
	 *   - Not required.
	 */
	//uint8 ComSignalDataInvalidValue;

	/* Defines the endianess of the signal's network representation.
	 *
	 * Context:
	 *   - Send and receive.
	 *   - Required.
	 */
	const ComSignalEndianess_type ComSignalEndianess;

	/*
	 * Value used to initialized this signal.
	 *
	 * Context:
	 *   - Send
	 *   - Required
	 */
	const uint32 ComSignalInitValue;

	/* The number of bytes if the signal has type UINT8_N;
	 *
	 * Context:
	 *   - Send and receive.
	 *   - Required if type of signal is UINT8_N
	 *
	 * Comment: Range 1 to 8.
	 */
	const uint8 ComSignalLength;

	/* Defines the type of the signal
	 *
	 * Context:
	 *   - Send and receive.
	 *   - Required.
	 */
	const Com_SignalType ComSignalType;

	/* Timeout period for deadline monitoring.
	 *
	 * Context:
	 *   - Receive
	 *   - Not required.
	 */
	//const uint32 Com_Arc_DeadlineCounter;
	const uint32 ComTimeoutFactor;

	/* Timeout notification function
	 *
	 * Context:
	 *   - Receive and send
	 *   - Not required.
	 */
	void (*ComTimeoutNotification) (void);

	const ComTransferProperty_type ComTransferProperty;

	/*
	 * The bit position in the PDU for this signals update bit.
	 *
	 * Context:
	 *   - Send and receive.
	 *   - Not required.
	 *
	 * Comment: Range 0 to 63. If update bit is used for this signal, then the corresponding parameter ComSignalArcUseUpdateBit
	 *          needs to be set to one.
	 */
	const uint8 ComUpdateBitPosition;
	const uint8 ComSignalArcUseUpdateBit;

	/* Filter for this signal
	 *
	 * Context:
	 *   - Send.
	 *   - Required.
	 */
	const ComFilter_type ComFilter;


	/**** SIGNAL GROUP DATA ****/
	const uint8 Com_Arc_IsSignalGroup;
	const ComGroupSignal_type *ComGroupSignal[COM_MAX_NR_SIGNALS_PER_SIGNAL_GROUP];
	//void *Com_Arc_ShadowBuffer;
	//void *Com_Arc_IPduDataPtr;


	/* Pointer to the data storage of this signals IPDU.
	 *
	 * Comment: This is initialized by Com_Init() and should not be configured.
	 */
	//const void *ComIPduDataPtr;

	/* IPDU id of the IPDU that this signal belongs to.
	 *
	 * Comment: This is initialized by Com_Init() and should not be configured.
	 */

	//const uint8 ComIPduHandleId;
	//const uint8 ComSignalUpdated;


	const uint8 Com_Arc_EOL;
} ComSignal_type;


typedef struct {
	/* Transmission mode for this IPdu.
	 *
	 * Context:
	 *   - Send.
	 *   - Required.
	 */
	const ComTxModeMode_type ComTxModeMode;

	/* Defines the number of times this IPdu will be sent in each IPdu cycle.
	 *
	 * Context:
	 *   - Send.
	 *   - Required for transmission modes DIRECT/N-times and MIXED.
	 *
	 * Comment: Should be set to 0 for DIRECT transmission mode and >0 for DIRECT/N-times mode.
	 */
	const uint8 ComTxModeNumberOfRepetitions;

	/* Defines the period of the transmissions in DIRECT/N-times and MIXED
	 * transmission modes.
	 *
	 * Context:
	 *   - Send.
	 *   - Required for DIRECT/N-times and MIXED transmission modes.
	 */
	const uint32 ComTxModeRepetitionPeriodFactor;

	/* Time before first transmission of this IPDU. (i.e. between the ipdu group start
	 * and this IPDU is sent for the first time.
	 *
	 * Context:
	 *   - Send.
	 *   - Required for all transmission modes except NONE.
	 */
	const uint32 ComTxModeTimeOffsetFactor;

	/* Period of cyclic transmission.
	 *
	 * Context:
	 *   - Send.
	 *   - Required for CYCLIC and MIXED transmission mode.
	 */
	const uint32 ComTxModeTimePeriodFactor;
} ComTxMode_type;


typedef struct {

	/* Minimum delay between successive transmissions of the IPdu.
	 *
	 * Context:
	 *   - Send.
	 *   - Not required.
	 */
	const uint32 ComTxIPduMinimumDelayFactor;

	/* COM will fill unused areas within an IPdu with this bit patter.
	 *
	 * Context:
	 *   - Send.
	 *   - Required.
	 */
	const uint8 ComTxIPduUnusedAreasDefault;

	/* Transmission modes for this IPdu.
	 *
	 * Context:
	 *   - Send.
	 *   - Not required.
	 *
	 * Comment: TMS is not implemented so only one static transmission
	 *          mode is supported.
	 */
	const ComTxMode_type ComTxModeTrue;
	//ComTxMode_type ComTxModeFalse;

} ComTxIPdu_type;

/*
typedef struct {
	uint8  ComTxIPduNumberOfRepetitionsLeft;
	uint32 ComTxModeRepetitionPeriodTimer;
	uint32 ComTxIPduMinimumDelayTimer;
	uint32 ComTxModeTimePeriodTimer;
} ComTxIPduTimer_type;
*/

typedef struct ComIPduGroup_type {
	// ID of this group. 0-31.
	const uint8 ComIPduGroupHandleId;

	// reference to the group that this group possibly belongs to.
	//struct ComIPduGroup_type *ComIPduGroupRef;

	const uint8 Com_Arc_EOL;
} ComIPduGroup_type;


typedef struct {

	/* Callout function of this IPDU.
	 * The callout function is an optional function used both on sender and receiver side.
	 * If configured, it determines whether an IPdu is considered for further processing. If
	 * the callout return false the IPdu will not be received/sent.
	 *
	 * Context:
	 *   - Send and receive.
	 *   - Not required.
	 */
	boolean (*ComIPduCallout)(PduIdType PduId, const uint8 *IPduData);


	/* The ID of this IPDU.
	 *
	 * Context:
	 *   - Send and receive.
	 *   - Required.
	 *
	 * Comment:
	 */
	const uint8 ComIPduRxHandleId;

	/* Signal processing mode for this IPDU.
	 *
	 * Context:
	 *   - Receive.
	 *   - Required.
	 */
	const Com_IPduSignalProcessingMode ComIPduSignalProcessing;

	/* Size of the IPDU in bytes. 0-8 for CAN and LIN and 0-256 for FlexRay.
	 *
	 * Context:
	 *   - Send and receive.
	 *   - Required.
	 */
	const uint8 ComIPduSize;

	/* The direction of the IPDU. Receive or Send.
	 *
	 * Context:
	 *   - Receive or send.
	 *   - Required.
	 */
	const Com_IPduDirection ComIPduDirection;

	/* Reference to the IPDU group that this IPDU belongs to.
	 *
	 * Context:
	 *   - Send and receive.
	 *   - Required.
	 */
	const uint8 ComIPduGroupRef;

	/* Reference to global PDU structure. ???
	 *
	 * No global PDU structure defined so this variable is left out.
	 */
	// PduIdRef

	/* Container of transmission related parameters.
	 *
	 * Context:
	 * 	 - Send
	 *   - Required.
	 */
	const ComTxIPdu_type ComTxIPdu;

	/* Transmission related timers and parameters.
	 *
	 * Context:
	 *   - Send
	 *   - Not required.
	 *   - Not part of the AUTOSAR standard.
	 *
	 * Comment: These are internal variables and should not be configured.
	 */
	//ComTxIPduTimer_type Com_Arc_TxIPduTimers;

	/* Pointer to data storage of this IPDU.
	 *
	 * Context:
	 *   - Send and receive.
	 *
	 * Comment: this memory will be initialized dynamically in Com_Init();
	 */
	//void *ComIPduDataPtr;

	/* References to all signals contained in this IPDU.
	 *
	 * Context:
	 *   - Send and receive.
	 *   - Not required.
	 *
	 * Comment: It probably makes little sense not to define at least one signal for each IPDU.
	 */
	//const uint8 Com_Arc_NIPduSignalGroupRef;
	const ComSignal_type *ComIPduSignalGroupRef[COM_MAX_NR_SIGNALS_PER_IPDU];


	/* References to all signals contained in this IPDU.
	 *
	 * Context:
	 *   - Send and receive.
	 *   - Not required.
	 *
	 * Comment: It probably makes little sense not to define at least one signal for each IPDU.
	 */
	//const uint8 NComIPduSignalRef;
	const ComSignal_type *ComIPduSignalRef[COM_MAX_NR_SIGNALS_PER_IPDU];

	/*
	 * The following two variables are used to control the per I-PDU based Rx/Tx-deadline monitoring.
	 */
	//const uint32 Com_Arc_DeadlineCounter;
	//const uint32 Com_Arc_TimeoutFactor;

	const uint8 Com_Arc_EOL;

} ComIPdu_type;


// Contains configuration specific configuration parameters. Exists once per configuration.
typedef struct {

	// The ID of this configuration. This is returned by Com_GetConfigurationId();
	const uint8 ComConfigurationId;

	/*
	 * Signal Gateway mappings.
	 * Not Implemented yet.
	ComGwMapping_type ComGwMapping[];
	 */

	// IPDU definitions. At least one
	const ComIPdu_type *ComIPdu;

	//uint16 Com_Arc_NIPdu;

	// IPDU group definitions
	const ComIPduGroup_type *ComIPduGroup;

	// Signal definitions
	const ComSignal_type *ComSignal;

	// Signal group definitions
	//ComSignalGroup_type *ComSignalGroup;

	// Group signal definitions
	const ComGroupSignal_type *ComGroupSignal;

} Com_ConfigType;

#endif /*COM_TYPES_H_*/
