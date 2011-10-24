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

#ifndef PDUR_TYPES_H
#define PDUR_TYPES_H

#include "ComStack_Types.h"

typedef enum {
	ARC_PDUR_UP_MODULES = 0,
	ARC_PDUR_COM,
	ARC_PDUR_DCM,

	ARC_PDUR_LOIF_MODULES,
	ARC_PDUR_CANIF,
	ARC_PDUR_LINIF,
	ARC_PDUR_SOADIF,

	ARC_PDUR_LOTP_MODULES,
	ARC_PDUR_CANTP,
	ARC_PDUR_LINTP,
	ARC_PDUR_SOADTP,
	ARC_PDUR_SOAD,
	ARC_PDUR_J1939TP,

	ARC_PDUR_END_OF_MODULES

} ARC_PduR_ModuleType;

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

/*
typedef struct {

	uint16 BufferId;
	PduR_DataProvisionType BufferType;
	//uint8 SduLength;
	uint8 *Last;
	uint8 *First;
	uint8 NrItems;
	uint8 TxConfP;
	// uint8 TxIdx; // This is the same as First, hence left out.
	uint8 *Buffer;


	uint8 Depth;

	uint8 Length;

} PduRTxBuffer_type;
*/
typedef uint8 *PduRTxBuffer_type;

typedef enum {
	PDUR_BUFFER_FREE = 0,
	PDUR_BUFFER_RX_BUSY,
	PDUR_BUFFER_TX_READY,
	PDUR_BUFFER_TX_BUSY
} PduRTpBufferStatus_type;

typedef struct {
	PduInfoType *pduInfoPtr;
	PduRTpBufferStatus_type status;
	const uint16 bufferSize;
} PduRTpBufferInfo_type;

typedef struct {

	/**
	 * Data provision mode for this PDU.
	 */
	const PduR_DataProvisionType DataProvision;

	/**
	 * Reference to unique PDU identifier which shall
	 * be used by the PDU router instead of the source identifier.
	 */
	const uint16 DestPduId;

	/**
	 * Reference to the assigned Tx buffer.
	 *
	 * Comment: Only required for non-TP gateway PDUs.
	 */
	PduRTxBuffer_type * const TxBufferRef;

	const ARC_PduR_ModuleType DestModule;

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
	 * Length of PDU data.
	 *
	 * Comment: Only required if a TX buffer is configured.
	 */
	const uint16 SduLength;

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
	 * Specifies the source ID of the PDU to be routed.
	 */
	const uint16 SrcPduId;

	/**
	 * Specifies the source module for this route.
	 */
	const ARC_PduR_ModuleType SrcModule;

	/**
	 * Specifies the destination(s) of the PDU to be routed.
	 */
	const PduRDestPdu_type * const *PduRDestPdus;

} PduRRoutingPath_type;

typedef struct {
	/**
	 * Unique configuration identifier.
	 */
	uint8 PduRConfigurationId;

	uint8 NRoutingPaths;

	/**
	 * The routing table of this PDU router configuration.
	 */
	const PduRRoutingPath_type * const*RoutingPaths;

	PduRTpBufferInfo_type *TpBuffers;
	PduRTpBufferInfo_type **TpRouteBuffers;

} PduR_PBConfigType;

#endif
/** @} */
