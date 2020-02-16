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



/** @addtogroup Com COM module
 *  @{ */

/** @file Com_Types.h
 * Definitions of configuration types and parameters for the COM module.
 */



#ifndef COM_TYPES_H_
#define COM_TYPES_H_


#include "ComStack_Types.h"

typedef uint16 Com_IpduGroupIdType;
typedef uint16 Com_SignalIdType;
typedef uint8 Com_SignalGroupIdType;
typedef uint16 Com_BitPositionType;

/* @req COM346 */
typedef boolean (*ComRxIPduCalloutType)(PduIdType PduId, const uint8 *IPduData);

/* @req COM700 */
typedef boolean (*ComTxIPduCalloutType)(PduIdType PduId, uint8 *IPduData);

/* @req COM555 COM554 COM491 COM556*/
typedef void (*ComNotificationCalloutType) (void);

#define COM_NO_FUNCTION_CALLOUT 0xFFFFFFFFu

#define NO_PDU_REFERENCE 0xFFFFu

typedef enum {
	COM_UNINIT = 0,
	COM_INIT
}Com_StatusType;

typedef enum {
	IMMEDIATE,
	DEFERRED
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
	UINT8_DYN,
	SINT8,
	SINT16,
	SINT32
} Com_SignalType;

#define COM_SIGNALTYPE_UNSIGNED  FALSE
#define COM_SIGNALTYPE_SIGNED    TRUE

typedef enum {
	PENDING,
	TRIGGERED,
	TRIGGERED_WITHOUT_REPETITION,
	TRIGGERED_ON_CHANGE_WITHOUT_REPETITION,
	TRIGGERED_ON_CHANGE
} ComTransferProperty_type;

typedef enum {
	DIRECT,
	MIXED,
	NONE,
	PERIODIC
} ComTxModeMode_type;


typedef enum {
	ALWAYS,
	MASKED_NEW_DIFFERS_MASKED_OLD,
	MASKED_NEW_DIFFERS_X,
	MASKED_NEW_EQUALS_X,
	NEVER,
	NEW_IS_OUTSIDE,
	NEW_IS_WITHIN,
	ONE_EVERY_N
} ComFilterAlgorithm_type;

typedef enum {
	COM_BIG_ENDIAN,
	COM_LITTLE_ENDIAN,
	COM_OPAQUE
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

#define SignalTypeSignedness(type) \
		(( (type == SINT8) || (type == SINT16) || (type == SINT32) ) ? \
				COM_SIGNALTYPE_SIGNED : COM_SIGNALTYPE_UNSIGNED)

/** Filter configuration type.
 * NOT SUPPORTED
 */
typedef struct {
	/** The algorithm that this filter uses. */
	ComFilterAlgorithm_type ComFilterAlgorithm;
	/** Filter mask. */
	uint32 ComFilterMask;
	/** Max value for filter. */
	uint32 ComFilterMax;
	/** Min value for filter. */
	uint32 ComFilterMin;
	/** Offset for filter. */
	uint32 ComFilterOffset;
	uint32 ComFilterPeriodFactor;
	uint32 ComFilterX;
	uint32 ComFilterArcN;
	uint32 ComFilterArcNewValue;
	uint32 ComFilterArcOldValue;

} ComFilter_type;

/** Configuration structure for group signals */
typedef struct {
	/** Starting position (bit) of the signal within the IPDU.
	 * Range 0 to 63.
	 */
	const Com_BitPositionType ComBitPosition;

	/** The size of the signal in bits.
	 * Range 0 to 64.
	 */
	const uint8 ComBitSize;

	/** Identifier for the signal.
	 * Should be the same value as the index in the COM signal array.
	 */
	const uint8 ComHandleId;

	/** Defines the endianess of the signal's network representation. */
	const ComSignalEndianess_type ComSignalEndianess;

	/** Value used to initialize this signal. */
	const void *ComSignalInitValue;

	/** Defines the type of the signal. */
	const Com_SignalType ComSignalType;


	/** Filter for this signal.
	 * NOT SUPPORTED
	 */
	//const ComFilter_type ComFilter;

	/* Pointer to the shadow buffer of the signal group that this group signal is contained in.
	 *
	 * This is initialized by Com_Init() and should not be configured.
	 */
	//void *Com_Arc_ShadowBuffer;

	/* Callback function used when an invalid signal is received. */
	// ComInvalidNotification;
	//uint8 ComSignalDataInvalidValue;

	/* IPDU id of the IPDU that this signal belongs to.
	 *
	 * This is initialized by Com_Init() and should not be configured.
	 */

	//const uint8 ComIPduHandleId;
	//const uint8 ComSignalUpdated;

	/** Marks the end of list for the configuration array. */
	const uint8 Com_Arc_EOL;
} ComGroupSignal_type;


/** Configuration structure for signals and signal groups. */
typedef struct {

	/** Starting position (bit) of the signal within the IPDU.
	 * Range 0 to 2031.
	 */
	const Com_BitPositionType ComBitPosition;

	/** The size of the signal in bits.
	 * Range 0 to 63.
	 */
	const uint8 ComBitSize;

	/** Notification function for error notification. */
	const uint32 ComErrorNotification;

	/** First timeout period for deadline monitoring. */
	const uint32 ComFirstTimeoutFactor;

	/** Identifier for the signal.
	 * Should be the same value as the index in the COM signal array.
	 */
	const uint16 ComHandleId;

	/** Tx and Rx notification function. */
	const uint32 ComNotification;

	/** Action to be performed when a reception timeout occurs. */
	const ComRxDataTimeoutAction_type ComRxDataTimeoutAction;

	/** Defines the endianess of the signal's network representation. */
	const ComSignalEndianess_type ComSignalEndianess;

	/** Value used to initialized this signal. */
	const void *ComSignalInitValue;

	/** The number of bytes if the signal has type UINT8_N;
	 * Range 1 to 8.
	 */
	//const uint8 ComSignalLength;

	/** Defines the type of the signal. */
	const Com_SignalType ComSignalType;

	/** Timeout period for deadline monitoring. */
	const uint32 ComTimeoutFactor;

	/** Timeout notification function. */
	const uint32 ComTimeoutNotification;

	const ComTransferProperty_type ComTransferProperty;

	/** The bit position in the PDU for this signal's update bit.
	 * Range 0 to 2031.
	 * Only applicable if an update bit is used. NULL otherwise.
	 */
	const Com_BitPositionType ComUpdateBitPosition;

	/** Marks if this signal uses an update bit.
	 * Should be set to one if an update bit is used.
	 */
	const uint8 ComSignalArcUseUpdateBit;

	/** Filter for this signal.
	 * NOT SUPPORTED.
	 */
	//const ComFilter_type ComFilter;

	/** Marks if this signal is a signal group.
	 * Should be set to 1 if the signal is a signal group.
	 */
	const uint8 Com_Arc_IsSignalGroup;

	/** Array of group signals.
	 * Only applicable if this signal is a signal group.
	 */
	const ComGroupSignal_type * const *ComGroupSignal;


	const uint8 *Com_Arc_ShadowBuffer_Mask;
	//void *Com_Arc_IPduDataPtr;

	/* Pointer to the data storage of this signals IPDU.
	 * This is initialized by Com_Init() and should not be configured.
	 */
	//const void *ComIPduDataPtr;

	/* IPDU id of the IPDU that this signal belongs to.
	 * This is initialized by Com_Init() and should not be configured.
	 */

	const uint16 ComIPduHandleId;
	//const uint8 ComSignalUpdated;

	/* Callback function used when an invalid signal is received.
	 */
	// ComInvalidNotification;

	//uint8 ComSignalDataInvalidValue;

	/* Action to be taken if an invalid signal is received.
	 */
	// ComDataInvalidAction;

	/** Marks the end of list for the signal configuration array. */
	const uint8 Com_Arc_EOL;
} ComSignal_type;



/** Configuration structure for Tx-mode for I-PDUs. */
typedef struct {

	/** Transmission mode for this IPdu. */
	const ComTxModeMode_type ComTxModeMode;

	/** Defines the number of times this IPdu will be sent in each IPdu cycle.
	 * Should be set to 0 for DIRECT transmission mode and >0 for DIRECT/N-times mode.
	 */
	const uint8 ComTxModeNumberOfRepetitions;

	/** Defines the period of the transmissions in DIRECT/N-times and MIXED transmission modes. */
	const uint32 ComTxModeRepetitionPeriodFactor;

	/** Time before first transmission of this IPDU. (i.e. between the ipdu group start and this IPDU is sent for the first time. */
	const uint32 ComTxModeTimeOffsetFactor;

	/** Period of cyclic transmission. */
	const uint32 ComTxModeTimePeriodFactor;
} ComTxMode_type;

/** Extra configuration structure for Tx I-PDUs. */
typedef struct {

	/** Minimum delay between successive transmissions of the IPdu. */
	const uint32 ComTxIPduMinimumDelayFactor;

	/** COM will fill unused areas within an IPdu with this bit patter.
	 */
	const uint8 ComTxIPduUnusedAreasDefault;

	/** Transmission modes for the IPdu.
	 * TMS is not implemented so only one static transmission mode is supported.
	 */
	const ComTxMode_type ComTxModeTrue;

	//ComTxMode_type ComTxModeFalse;
} ComTxIPdu_type;


/** Configuration structure for I-PDU groups */
typedef struct {
	/** ID of this group.
	 */
	const Com_IpduGroupIdType ComIPduGroupHandleId;

	/** Marks the end of list for the I-PDU group configuration array. */
	const uint8 Com_Arc_EOL;
} ComIPduGroup_type;


/** Configuration structure for an I-PDU. */
typedef struct {

	/** Callout function of this IPDU.
	 * The callout function is an optional function used both on sender and receiver side.
	 * If configured, it determines whether an IPdu is considered for further processing. If
	 * the callout return false the IPdu will not be received/sent.
	 */
	uint32 ComRxIPduCallout;
	uint32 ComTxIPduCallout;

	/** The outgoing PDU id. For polite PDU id handling. */
	const uint8 ArcIPduOutgoingId;

	/** Signal processing mode for this IPDU. */
	const Com_IPduSignalProcessingMode ComIPduSignalProcessing;

	/** Size of the IPDU in bytes.
	 * Range 0-8 for CAN and LIN and 0-256 for FlexRay.
	 */
	const uint16 ComIPduSize;

	/** The direction of the IPDU. Receive or Send. */
	const Com_IPduDirection ComIPduDirection;

	/** References to the IPDU groups that this IPDU belongs to. */
	const ComIPduGroup_type *const ComIPduGroupRefs;

	/** Container of transmission related parameters. */
	const ComTxIPdu_type ComTxIPdu;

	/** References to all signals and signal groups contained in this IPDU.
	 * It probably makes little sense not to define at least one signal or signal group for each IPDU.
	 */
	const ComSignal_type * const *ComIPduSignalRef;

	const ComSignal_type * const ComIPduDynSignalRef;

	/*
	 * The following two variables are used to control the per I-PDU based Rx/Tx-deadline monitoring.
	 */
	//const uint32 Com_Arc_DeadlineCounter;
	//const uint32 Com_Arc_TimeoutFactor;

	/* Transmission related timers and parameters.
	 * These are internal variables and should not be configured.
	 */
	//ComTxIPduTimer_type Com_Arc_TxIPduTimers;

	/** Marks the end of list for this configuration array. */
	const uint8 Com_Arc_EOL;

} ComIPdu_type;


/** Top-level configuration container for COM. Exists once per configuration. */
typedef struct {

	/** The ID of this configuration. This is returned by Com_GetConfigurationId(); */
	const uint8 ComConfigurationId;

	/* The number of IPdus */
	const uint16 ComNofIPdus;

	/* The number of signals */
	const uint16 ComNofSignals;

	/* The number of group signals */
	const uint16 ComNofGroupSignals;

	/*
	 * Signal Gateway mappings.
	 * Not Implemented yet.
	ComGwMapping_type ComGwMapping[];
	 */

	/** IPDU definitions */
	const ComIPdu_type * const ComIPdu;

	//uint16 Com_Arc_NIPdu;

	/** IPDU group definitions */
	const ComIPduGroup_type * const ComIPduGroup;

	/** Signal definitions */
	const ComSignal_type *const  ComSignal;

	// Signal group definitions
	//ComSignalGroup_type *ComSignalGroup;

	/** Group signal definitions */
	const ComGroupSignal_type * const ComGroupSignal;

} Com_ConfigType;

#endif /*COM_TYPES_H_*/
/** @} */
