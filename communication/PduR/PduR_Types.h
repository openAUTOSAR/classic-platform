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

/** @addtogroup PduR PDU Router
 *  @{ */

/** @file PduR_Types.h
 * Type definitions for PDU Router.
 */

#ifndef _PDUR_TYPES_H
#define _PDUR_TYPES_H

#include "ComStack_Types.h"

/** PduR_StateType defines the states of which the PDU router can be in */
typedef enum {
	PDUR_UNINIT, /**< PDU Router is not initialized. */
	PDUR_ONLINE, /**< PDU Router initialized successfully. */
	PDUR_REDUCED /**< PDU Router initialization did not succeed. Only minimum routing is provided. */
} PduR_StateType;

typedef enum {
	PDU_CNLDO, /**< Cancel transfer since data is outdated. */
	PDU_CNLNB, /**< Cancel transfer since buffer is full. */
	PDU_CNLOR  /**< Cancel transfer of another reason. */
} PduR_CancelReasonType;

typedef uint8 PduR_ParameterValueType;

typedef enum {
	PDUR_NO_PROVISION, /**< No data provision. */
	PDUR_TRIGGER_TRANSMIT, /**< Trigger transmit type. */
	PDUR_DIRECT /**< Data provision type. */
} PduR_DataProvisionType;


/* ################## EXTERNAL STRUCTURES ##################
 *
 * These structures will be external in final implementation
 */
typedef struct {
	const int foo;
} PduR_LConfigType;




/* ################ NEW DEFINITIONS ################### */
typedef struct {
	Std_ReturnType (*TargetIndicationFctPtr)(PduIdType, const uint8*); /**< Pointer to target function in layer above PDU router. */
	Std_ReturnType (*TargetTransmitFctPtr)(PduIdType, const PduInfoType*); /**< Pointer to target function below PDU router. */


	void (*TargetConfirmationFctPtr)(PduIdType);

	/**
	 * Target function for trigger transmit requests from the interface modules, e.g. Com_TriggerTransmit. Only
	 * needed if gateway mode is not used, that is, if .DataProvision is set to PDUR_NO_PROVISION.
	 */
	Std_ReturnType (*TargetTriggerTransmitFctPtr)(PduIdType, uint8*);


	Std_ReturnType (*TargetGatewayFctPtr)(PduIdType, const PduInfoType*);

} PduR_FctPtrType;

typedef struct {
	/*
	 * Not part of autosar standard. Added by ArcCore.
	 */
	int BufferId;
	PduR_DataProvisionType BufferType;
	//uint8 SduLength;
	uint8 *Last;
	uint8 *First;
	uint8 NrItems;
	uint8 TxConfP;
	// uint8 TxIdx; // This is the same as First, hence left out.
	uint8 *Buffer;

	/**
	 * Depth of buffer
	 */
	uint8 Depth;

	/**
	 * Length of buffer
	 */
	uint8 Length;

} PduRTxBuffer_type;

typedef struct {
	/**
	 * The maximum numbers of Tx buffers.
	 */
	uint16 PduRMaxTxBufferNumber; // ???

	PduRTxBuffer_type PduRTxBuffer[];
} PduRTxBufferTable_type;

typedef struct {
	/**
	 * PDU identifier assigned by the PDU router.
	 */
	uint16 HandleId;

	/**
	 * Reference to unique PDU identifier.
	 */
	// SrcPduRef

} PduRSrcPdu_type;

typedef struct {

	/**
	 * Data provision mode for this PDU.
	 */
	PduR_DataProvisionType DataProvision;

	/**
	 * Reference to unique PDU identifier which shall
	 * be used by the PDU router instead of the source identifier.
	 */
	//DestPduRef
	// For the moment replaced by this
	uint16 DestPduId;

	/**
	 * Reference to the assigned Tx buffer.
	 *
	 * Comment: Only required for non-TP gateway PDUs.
	 */
	PduRTxBuffer_type *TxBufferRef;

} PduRDestPdu_type;

typedef struct {
	uint8 ElementBytePosition;
	uint8 DefaultValueElement[];
} PduRDefaultValueElement_type;


typedef struct {
	uint8 PduR_Arc_Dummy; // Needed in order to compile without errors.
	PduRDefaultValueElement_type *PduRDefaultValueElement;
} PduRDefaultValue_type;

typedef struct {
	/**
	 * Not part of standard
	 */
	PduR_FctPtrType *FctPtrs;
	uint8 PduR_Arc_EOL;
	uint8 PduR_GatewayMode;

	/**
	 * Length of PDU data.
	 *
	 * Comment: Only required if a TX buffer is configured.
	 */
	uint8 SduLength;

	/**
	 * Chunk size for routing on the fly.
	 *
	 * Comment: Only required for TP gateway PDUs.
	 */
	uint16 TpChunkSize;

	/**
	 * Specifies the default value of the PDU.
	 *
	 * Comment: Only require for gateway operation.
	 */
	PduRDefaultValue_type PduRDefaultValue;

	/**
	 * Specifies the source of the PDU to be routed.
	 */
	PduRSrcPdu_type PduRSrcPdu;

	/**
	 * Specifies the destination(s) of the PDU to be routed.
	 *
	 * Comment: Multicast (many destinations) is not supported in this implementation.
	 */
	PduRDestPdu_type PduRDestPdu;

} PduRRoutingPath_type;

typedef struct {
	/*
	 * Non-standards
	 */
	uint16 NRoutingPaths;

	/**
	 * References to the routing paths defined for this configuration.
	 */
	PduRRoutingPath_type PduRRoutingPath[];

} PduRRoutingTable_type;


typedef struct {
	/**
	 * Unique configuration identifier.
	 */
	uint8 PduRConfigurationId;

	/**
	 * The routing table of this PDU router configuration.
	 */
	PduRRoutingTable_type *PduRRoutingTable;


	/**
	 * The buffers used for TP gateway operation.
	 *
	 * Comment: Not implemented in this version.
	 */
	//PduRTpBufferTable_type PduRTpBufferTable;

	/**
	 * The buffers used for non-TP gateway operation.
	 */
	PduRTxBufferTable_type *PduRTxBufferTable;

} PduR_PBConfigType;

#endif
/** @} */
