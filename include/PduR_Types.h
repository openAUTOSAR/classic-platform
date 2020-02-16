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

/** @addtogroup PduR PDU Router
 *  @{ */

/** @file PduR_Types.h
 * Type definitions for PDU Router.
 */

#ifndef PDUR_TYPES_H
#define PDUR_TYPES_H

#include "ComStack_Types.h"


/* @req PDUR293 */
typedef enum {
	ARC_PDUR_UP_MODULES = 0,
	ARC_PDUR_COM,
	ARC_PDUR_DCM,
	ARC_PDUR_IPDUM,

	ARC_PDUR_LOIF_MODULES,
	ARC_PDUR_CANIF,
	ARC_PDUR_LINIF,
	ARC_PDUR_SOADIF,
	ARC_PDUR_FRIF,

	ARC_PDUR_LOTP_MODULES,
	ARC_PDUR_CANTP,
	ARC_PDUR_LINTP,
	ARC_PDUR_SOADTP,
	ARC_PDUR_SOAD,
	ARC_PDUR_J1939TP,
	ARC_PDUR_FRTP,

	ARC_PDUR_END_OF_MODULES

} ARC_PduR_ModuleType;

/* @req PDUR654 */
/* Identification of a Routing Table */
typedef uint16 PduR_RoutingPathGroupIdType;

/** PduR_StateType defines the states of which the PDU router can be in */
/* @req PDUR742 */
/* @req PDUR324 */
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
    PDUR_BUFFER_RX_READY,
	PDUR_BUFFER_RX_BUSY,
	PDUR_BUFFER_TX_READY,
	PDUR_BUFFER_TX_BUSY,
	PDUR_BUFFER_NOT_ALLOCATED_FROM_UP_MODULE,
	PDUR_BUFFER_ALLOCATED_FROM_UP_MODULE
} PduRTpBufferStatus_type;

typedef struct {
	PduInfoType *pduInfoPtr;
	PduRTpBufferStatus_type status;
	uint16 bufferSize;
	uint8 rxByteCount;
	uint8 txByteCount;
	uint8 nAcc;
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
	 * Id of the assigned Tx buffer.
	 *
	 * Comment: Only required for non-TP gateway PDUs.
	 */
	const uint16 TxBufferId;

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
	 * PduRTpThreshold for routing on the fly.
	 *
	 * Comment: Only required for TP gateway PDUs.
	 */
	uint16 PduRTpThreshld;

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


/* @req PDUR743 */
/* @req PDUR241 */
typedef struct {
    /**
     * The routing table of this PDU router configuration.
     */
    const PduRRoutingPath_type * const*RoutingPaths;

    /**
     * The nubmer of routing paths in the routing table.
     */
    uint8 NRoutingPaths;

    /**
	 * Unique configuration identifier.
	 */
	uint8 PduRConfigurationId;

	/**
	 * List of default value byte arrays.
	 */
	const uint8 * const * const DefaultValues;

	/**
	 * List of default value byte array lengths.
	 */
	const uint32 * const * const DefaultValueLengths;

} PduR_PBConfigType;

typedef struct {
    PduRTpBufferInfo_type *TpBuffers;
    PduRTpBufferInfo_type **TpRouteBuffers;
    PduRTxBuffer_type * TxBuffers;
    uint8 NTpBuffers;
    uint8 NTpRouteBuffers;
    uint16 NTxBuffers;

} PduR_RamBufCfgType;

#endif
/** @} */
