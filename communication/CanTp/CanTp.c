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

/** @addtogroup TBD
 *  @{ */

/** @file CanTp.c
 * TBD.
 */

/*
 *  General requirements
 */

/** @req CANTP001 */
/** @req CANTP019 */
/** @req CANTP020 */
/** @req CANTP156.Partially */
/** @req CANTP150 */
/** @req CANTP151 */
/** @req CANTP152 */
/** @req CANTP153 */
/** @req CANTP155 */
/** @req CANTP158 */
/** @req CANTP003 */
/** @req CANTP216 */
/** @req CANTP217 */

/*
 * Environmental requirements
 */
/** @req CANTP164 */
/** @req CANTP199 */


#include "CanTp.h" /** @req CANTP219 */
#include "CanTp_Cbk.h" /** @req CANTP233 */
#include "Det.h"
#include "CanIf.h"
#include "SchM_CanTp.h"
#include "PduR_CanTp.h"
//#include "MemMap.h"
#include <string.h>
//#define USE_DEBUG_PRINTF
#include "debug.h"

#if  ( CANTP_DEV_ERROR_DETECT == STD_ON ) /** @req CANTP006 *//** @req CANTP134 */

/** @req CANTP132 */ /** @req CANTP021 */
#define VALIDATE(_exp,_api,_err ) \
		if( !(_exp) ) { \
			Det_ReportError(MODULE_ID_CANTP, 0, _api, _err); \
			return E_NOT_OK; \
		}
/** @req CANTP132 */ /** @req CANTP021 */
#define VALIDATE_NO_RV(_exp,_api,_err ) \
		if( !(_exp) ) { \
			Det_ReportError(MODULE_ID_CANTP, 0, _api, _err); \
			return; \
		}
#define DET_REPORTERROR(_x,_y,_z,_q) Det_ReportError(_x, _y, _z, _q)

#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_NO_RV(_exp,_api,_err )
#define DET_REPORTERROR(_x,_y,_z,_q)
#endif


#if 0

NotifResultType PduR_CanTpRxIndication(PduIdType CanTpRxPduId,
		NotifResultType Result) {
	;

}

BufReq_ReturnType PduR_CanTpProvideRxBuffer(PduIdType id, PduLengthType length,
		PduInfoType **PduInfoPtr) {
	;
}

BufReq_ReturnType PduR_CanTpProvideTxBuffer(PduIdType CanTpTxId,
		PduInfoType** PduinfoPtr, uint16 Length) {
	;
}

void PduR_CanTpTxConfirmation(PduIdType CanTpTxPduId, NotifResultType Result) {
	;

}

#endif

#define TIMER_DECREMENT(timer) \
		if (timer > 0) { \
			timer = timer - 1; \
		} \

#define COUNT_DECREMENT(timer) \
		if (timer > 0) { \
			timer = timer - 1; \
		} \


#define CANTP_ERR                              -1
#define ISO15765_FLOW_CONTROL_STATUS_CTS        0
#define ISO15765_FLOW_CONTROL_STATUS_WAIT       1
#define ISO15765_FLOW_CONTROL_STATUS_OVFLW      2

// - - - - - - - - - - - - - -

#define ISO15765_TPCI_MASK      0x30
#define ISO15765_TPCI_SF        0x00  /* Single Frame */
#define ISO15765_TPCI_FF        0x10  /* First Frame */
#define ISO15765_TPCI_CF        0x20  /* Consecutive Frame */
#define ISO15765_TPCI_FC        0x30  /* Flow Control */
#define ISO15765_TPCI_DL        0x7   /* Single frame data length mask */
#define ISO15765_TPCI_FS_MASK   0x0F  /* Flowcontrol status mask */

// - - - - - - - - - - - - - -

#define MAX_PAYLOAD_SF_STD_ADDR		7
#define MAX_PAYLOAD_SF_EXT_ADDR		6

#define MAX_PAYLOAD_FF_STD_ADDR		6
#define MAX_PAYLOAD_FF_EXT_ADDR		5

#define MAX_PAYLOAD_CF_STD_ADDR		7
#define MAX_PAYLOAD_CF_EXT_ADDR		6

#define SEGMENT_NUMBER_MASK			0x0f

#define MAX_SEGMENT_DATA_SIZE		8 	// Size of a CAN frame data bytes.

/*
 *
 */
typedef enum {
	UNINITIALIZED, IDLE, SF_OR_FF_RECEIVED_WAITING_PDUR_BUFFER, /** @req CANTP079 */
	RX_WAIT_CONSECUTIVE_FRAME, RX_WAIT_SDU_BUFFER,

	TX_WAIT_STMIN,
	TX_WAIT_TRANSMIT, TX_WAIT_FLOW_CONTROL,
	TX_WAIT_TX_CONFIRMATION
} ISO15765TransferStateTypes;

typedef enum {
	INVALID_FRAME, /* Not specified by ISO15765 - used as error return type when decoding frame. */
	SINGLE_FRAME, FIRST_FRAME, CONSECUTIVE_FRAME, FLOW_CONTROL_CTS_FRAME, /* Clear to send */
	FLOW_CONTROL_WAIT_FRAME, FLOW_CONTROL_OVERFLOW_FRAME
} ISO15765FrameType;

/*
 *  In case no buffer is available at some cases the data needs to be
 *  temporarily stored away.
 */

typedef struct {
	uint8 data[MAX_SEGMENT_DATA_SIZE];
	PduLengthType byteCount;
} CanIfSduType;

/*
 *  Structure that is keeping track on the run-time variables for the ongoing
 *  transfer.
 */
typedef struct {
	uint16 nextFlowControlCount; // Count down to next Flow Control.
	uint16 framesHandledCount; // Counter keeping track total frames handled.
	uint32 stateTimeoutCount; // Counter for timeout.
	uint8 extendedAddress; // Not always used but need to be available.
	uint8 STmin; // In case we are transmitters the remote node can configure this value (only valid for TX).
	uint8 BS; // Blocksize (only valid for TX).
	boolean NasNarPending;
	uint32 NasNarTimeoutCount; // CanTpNas, CanTpNar.
	ISO15765TransferStateTypes state; // Transfer state machine. TODO: Can this be initialized here?
} ISO15765TransferControlType;

/*
 *	Container for TX or RX runtime paramters (TX/RX are identical?)
 */
typedef struct {
	ISO15765TransferControlType iso15765;
	PduInfoType *pdurBuffer; // The PDUR make an instance of this.
	PduLengthType pdurBufferCount; // Number of bytes in PDUR buffer.
	PduLengthType transferTotal; // Total length of the PDU.
	PduLengthType transferCount; // Counter ongoing transfer.
	CanIfSduType canFrameBuffer; // Temp storage of SDU data.
	CanTp_TransferInstanceMode mode; // CanTp030.
} CanTp_ChannelPrivateType;


// - - - - - - - - - - - - - -

typedef struct {
	boolean initRun;
	CanTp_StateType internalState; /** @req CANTP027 */
	CanTp_ChannelPrivateType runtimeDataList[CANTP_NSDU_RUNTIME_LIST_SIZE];
} CanTp_RunTimeDataType;

// - - - - - - - - - - - - - -

CanTp_RunTimeDataType CanTpRunTimeData = { .initRun = FALSE,
		.internalState = CANTP_OFF }; /** @req CANTP168 */

// - - - - - - - - - - - - - -

static ISO15765FrameType getFrameType(
		const CanTp_AddressingFormantType *formatType,
		const PduInfoType *CanTpRxPduPtr) {
	ISO15765FrameType res = INVALID_FRAME;
	uint8 tpci = 0;

	switch (*formatType) {
	case CANTP_STANDARD:
		DEBUG( DEBUG_MEDIUM, "CANTP_STANDARD\n");
		tpci = CanTpRxPduPtr->SduDataPtr[0];
		break;
	case CANTP_EXTENDED:
		DEBUG( DEBUG_MEDIUM, "CANTP_EXTENDED\n");
		tpci = CanTpRxPduPtr->SduDataPtr[1];
		break;
	default:
		break;
	}

	switch (tpci & ISO15765_TPCI_MASK) {
	case ISO15765_TPCI_SF:
		res = SINGLE_FRAME;
		break;
	case ISO15765_TPCI_FF:
		res = FIRST_FRAME;
		break;
	case ISO15765_TPCI_CF:
		res = CONSECUTIVE_FRAME;
		break;
	case ISO15765_TPCI_FC: // Some kind of flow control.
		switch (tpci & ISO15765_TPCI_FS_MASK) {
		case ISO15765_FLOW_CONTROL_STATUS_CTS:
			res = FLOW_CONTROL_CTS_FRAME;
			break;
		case ISO15765_FLOW_CONTROL_STATUS_WAIT:
			res = FLOW_CONTROL_CTS_FRAME;
			break;
		case ISO15765_FLOW_CONTROL_STATUS_OVFLW:
			res = FLOW_CONTROL_CTS_FRAME;
			break;
		}
	}
	return res;
}

// - - - - - - - - - - - - - -

static PduLengthType getPduLength(
		const CanTp_AddressingFormantType *formatType,
		const ISO15765FrameType iso15765Frame, const PduInfoType *CanTpRxPduPtr) {
	PduLengthType res = 0;
	uint8 tpci_offset = 0;

	switch (*formatType) {
	case CANTP_STANDARD:
		tpci_offset = 0;
		break;
	case CANTP_EXTENDED:
		tpci_offset = 1;
		break;
	default:
		return 0;
	}

	switch (iso15765Frame) {
	case SINGLE_FRAME:
		// Parse the data length from the single frame header.
		res = CanTpRxPduPtr->SduDataPtr[tpci_offset] & ISO15765_TPCI_DL;
		break;
	case FIRST_FRAME:
		// Parse the data length form the first frame.
		res = CanTpRxPduPtr->SduDataPtr[tpci_offset + 1] + ((PduLengthType)((CanTpRxPduPtr->SduDataPtr[tpci_offset]) & 0xf) << 8);
		break;
	default:
		res = 0; // TODO: maybe we should have an error code here.
		break;
	}
	return res;
}

// - - - - - - - - - - - - - -

static void initRx15765RuntimeData(const CanTp_RxNSduType *rxConfigParams,
		CanTp_ChannelPrivateType *rxRuntimeParams) {

	rxRuntimeParams->iso15765.state = IDLE;
	rxRuntimeParams->iso15765.NasNarPending = FALSE;
	rxRuntimeParams->iso15765.framesHandledCount = 0;
	rxRuntimeParams->iso15765.nextFlowControlCount = 0;
	rxRuntimeParams->pdurBufferCount = 0;
	rxRuntimeParams->transferTotal = 0;
	rxRuntimeParams->transferCount = 0;
	rxRuntimeParams->mode = CANTP_RX_WAIT; /** @req CANTP030 */
	rxRuntimeParams->pdurBuffer = NULL;
}

// - - - - - - - - - - - - - -

static void initTx15765RuntimeData(const CanTp_TxNSduType *txConfigParams,
		CanTp_ChannelPrivateType *txRuntimeParams) {

	txRuntimeParams->iso15765.state = IDLE;
	txRuntimeParams->iso15765.NasNarPending = FALSE;
	txRuntimeParams->iso15765.framesHandledCount = 0;
	txRuntimeParams->iso15765.nextFlowControlCount = 0;
	txRuntimeParams->pdurBufferCount = 0;
	txRuntimeParams->transferTotal = 0;
	txRuntimeParams->transferCount = 0;
	txRuntimeParams->mode = CANTP_TX_WAIT; /** @req CANTP030 */
	txRuntimeParams->pdurBuffer = NULL;

}

// - - - - - - - - - - - - - -

static BufReq_ReturnType copySegmentToPduRRxBuffer(const CanTp_RxNSduType *rxConfig,
		CanTp_ChannelPrivateType *rxRuntime, uint8 *segment,
		PduLengthType segmentSize, PduLengthType *bytesWrittenSuccessfully) {

	BufReq_ReturnType ret = BUFREQ_NOT_OK;
	boolean endLoop = FALSE;
	*bytesWrittenSuccessfully = 0;

	while ((*bytesWrittenSuccessfully < segmentSize) && (!endLoop)) {
		// Copy the data that resides in the buffer.
		if (rxRuntime->pdurBuffer != NULL) {
			while ((*bytesWrittenSuccessfully < segmentSize ) && (rxRuntime->pdurBuffer->SduLength > rxRuntime->pdurBufferCount)) {
				rxRuntime->pdurBuffer->SduDataPtr[rxRuntime->pdurBufferCount++] = segment[(*bytesWrittenSuccessfully)++];
			}
		}
		if (*bytesWrittenSuccessfully < segmentSize ) {
			// We need to request a new buffer from the SDUR.
			// TODO: We should do a timeout here.
			ret = PduR_CanTpProvideRxBuffer(rxConfig->PduR_PduId, rxRuntime->transferTotal, &rxRuntime->pdurBuffer);  /** @req CANTP079 */ /** @req CANTP080 */ /** @req CANTP064 */
			if (ret == BUFREQ_OK) {
				VALIDATE( rxRuntime->pdurBuffer->SduDataPtr != NULL,
						SERVICE_ID_CANTP_TRANSMIT, CANTP_E_INVALID_RX_BUFFER );
				rxRuntime->pdurBufferCount = 0; // The buffer is emptied.
			} else if (ret == BUFREQ_BUSY) {
				rxRuntime->transferCount += *bytesWrittenSuccessfully;
				endLoop = TRUE;
			} else {
				endLoop = TRUE; // Let calling function handle this error.
			}
		} else {
			rxRuntime->transferCount += segmentSize; //== bytesWrittenSuccessfully
			ret = BUFREQ_OK;
			endLoop = TRUE;
		}
	}
	return ret;
}

// - - - - - - - - - - - - - -

static boolean copySegmentToLocalRxBuffer /*writeDataSegmentToLocalBuffer*/(
		CanTp_ChannelPrivateType *rxRuntime, uint8 *segment,
		PduLengthType segmentSize) {
	boolean ret = FALSE;

	if ( segmentSize < MAX_SEGMENT_DATA_SIZE ) {
		for (int i=0; i < segmentSize; i++) {
			rxRuntime->canFrameBuffer.data[i] = segment[i];
		}
		rxRuntime->canFrameBuffer.byteCount = segmentSize;
		ret = TRUE;
	}
	return ret;
}

// - - - - - - - - - - - - - -

static Std_ReturnType canReceivePaddingHelper(
		const CanTp_RxNSduType *rxConfig, CanTp_ChannelPrivateType *rxRuntime,
		PduInfoType *PduInfoPtr) {
	if (rxConfig->CanTpRxPaddingActivation == CANTP_ON) {
		for (int i = PduInfoPtr->SduLength; i < MAX_SEGMENT_DATA_SIZE; i++) {
			PduInfoPtr->SduDataPtr[i] = 0x0; // TODO: Does it have to be padded with zeroes?
		}
		PduInfoPtr->SduLength = MAX_SEGMENT_DATA_SIZE;
	}
	rxRuntime->iso15765.NasNarTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(rxConfig->CanTpNar); /** @req CANTP075 */
	rxRuntime->iso15765.NasNarPending = TRUE;
	return CanIf_Transmit(rxConfig->CanIf_FcPduId, PduInfoPtr);
}

// - - - - - - - - - - - - - -

static Std_ReturnType canTansmitPaddingHelper(
		const CanTp_TxNSduType *txConfig, CanTp_ChannelPrivateType *txRuntime,
		PduInfoType *PduInfoPtr) {

	/** @req CANTP114 */
	/** @req CANTP040 */
	/** @req CANTP098 */
	/** @req CANTP116 */
	/** @req CANTP059 */

	if (txConfig->CanTpTxPaddingActivation == CANTP_ON) { /** @req CANTP225 */
		for (int i = PduInfoPtr->SduLength; i < MAX_SEGMENT_DATA_SIZE; i++) {
			PduInfoPtr->SduDataPtr[i] = 0x0; // TODO: Does it have to be padded with zeroes?
		}
		PduInfoPtr->SduLength = MAX_SEGMENT_DATA_SIZE;
	}
	txRuntime->iso15765.NasNarTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(txConfig->CanTpNas); /** @req CANTP075 */
	txRuntime->iso15765.NasNarPending = TRUE;
	return CanIf_Transmit(txConfig->CanIf_PduId, PduInfoPtr);
}

// - - - - - - - - - - - - - -

static void sendFlowControlFrame(const CanTp_RxNSduType *rxConfig, CanTp_ChannelPrivateType *rxRuntime, BufReq_ReturnType flowStatus) {
	uint8 indexCount = 0;
	Std_ReturnType ret = E_NOT_OK;
	PduInfoType pduInfo;
	uint8 sduData[8]; // Note that buffer in declared on the stack.
	uint16 spaceFreePduRBuffer = 0;
	uint16 computedBs = 0;

	DEBUG( DEBUG_MEDIUM, "sendFlowControlFrame called!\n");
	pduInfo.SduDataPtr = &sduData[0];
	if (rxConfig->CanTpAddressingFormant == CANTP_EXTENDED) {
		sduData[indexCount++] = rxRuntime->iso15765.extendedAddress;
	}
	switch (flowStatus) {
	case BUFREQ_OK:
	{
		sduData[indexCount++] = ISO15765_TPCI_FC | ISO15765_FLOW_CONTROL_STATUS_CTS;
		spaceFreePduRBuffer = rxRuntime->pdurBuffer->SduLength - rxRuntime->pdurBufferCount;
		if (rxConfig->CanTpAddressingFormant == CANTP_EXTENDED) { /** @req CANTP094 *//** @req CANTP095 */
			computedBs = (spaceFreePduRBuffer / MAX_PAYLOAD_SF_EXT_ADDR) + 1;  // + 1 is for local buffer.
		} else {
			computedBs = (spaceFreePduRBuffer / MAX_PAYLOAD_SF_STD_ADDR) + 1;  // + 1 is for local buffer.
		}
		if (computedBs > rxConfig->CanTpBs) { // /** @req CANTP091 *//** @req CANTP084 */
			computedBs = rxConfig->CanTpBs;
		}
		DEBUG( DEBUG_MEDIUM, "computedBs:%d\n", computedBs);
		sduData[indexCount++] = computedBs; // 734 PC-lint: Okej att casta till uint8?
		sduData[indexCount++] = (uint8) rxConfig->CanTpSTmin;
		rxRuntime->iso15765.nextFlowControlCount = computedBs;
		pduInfo.SduLength = indexCount;
		break;
	}
	case BUFREQ_NOT_OK:
		break;
	case BUFREQ_BUSY:
		sduData[indexCount++] = ISO15765_TPCI_FC | ISO15765_FLOW_CONTROL_STATUS_WAIT;
		indexCount +=2;
		pduInfo.SduLength = indexCount;
		break;
	case BUFREQ_OVFL: /** @req CANTP081 */
		sduData[indexCount++] = ISO15765_TPCI_FC | ISO15765_FLOW_CONTROL_STATUS_OVFLW;
		indexCount +=2;
		pduInfo.SduLength = indexCount;
		break;
	default:
		break;
	}
	ret = canReceivePaddingHelper(rxConfig, rxRuntime, &pduInfo);
	if (ret != E_OK) {
		PduR_CanTpRxIndication(rxConfig->PduR_PduId, NTFRSLT_E_NOT_OK);  /** @req CANTP084 */
		rxRuntime->iso15765.state = IDLE;
		rxRuntime->mode = CANTP_RX_WAIT;
	}
}

// - - - - - - - - - - - - - -


static void handleConsecutiveFrame(const CanTp_RxNSduType *rxConfig,
		CanTp_ChannelPrivateType *rxRuntime, const PduInfoType *rxPduData) {
	uint8 indexCount = 0;
	uint8 segmentNumber = 0;
	uint8 extendedAddress = 0;
	PduLengthType bytesLeftToCopy = 0;
	PduLengthType bytesLeftToTransfer = 0;
	PduLengthType currentSegmentSize = 0;
	PduLengthType currentSegmentMaxSize = 0;
	PduLengthType bytesCopiedToPdurRxBuffer = 0;
	BufReq_ReturnType ret = BUFREQ_NOT_OK;

	if (rxRuntime->iso15765.state == RX_WAIT_CONSECUTIVE_FRAME) {
		if (rxConfig->CanTpAddressingFormant == CANTP_EXTENDED) {
			extendedAddress = rxPduData->SduDataPtr[indexCount++];
		}
		segmentNumber = rxPduData->SduDataPtr[indexCount++] & SEGMENT_NUMBER_MASK;
		if (segmentNumber != (rxRuntime->iso15765.framesHandledCount & SEGMENT_NUMBER_MASK)) {
			DEBUG(DEBUG_MEDIUM,"Segmentation number error detected - is the sending"
					"unit too fast? Increase STmin (cofig) to slow it down!\n");
			PduR_CanTpRxIndication(rxConfig->PduR_PduId, NTFRSLT_E_WRONG_SN); /** @req CANTP084 */
			rxRuntime->iso15765.state = IDLE;
			rxRuntime->mode = CANTP_RX_WAIT;
		} else {
			currentSegmentMaxSize = MAX_SEGMENT_DATA_SIZE - indexCount;
			bytesLeftToCopy = rxRuntime->transferTotal - rxRuntime->transferCount;
			if (bytesLeftToCopy < currentSegmentMaxSize) {
				currentSegmentSize = bytesLeftToCopy; // 1-5.
			} else {
				currentSegmentSize = currentSegmentMaxSize; // 6 or 7, depends on addressing format used.
			}
			// Copy received data to buffer provided by SDUR.
			ret = copySegmentToPduRRxBuffer(rxConfig, rxRuntime,
					&rxPduData->SduDataPtr[indexCount],
					currentSegmentSize,
					&bytesCopiedToPdurRxBuffer);
			if (ret == BUFREQ_NOT_OK) {
				PduR_CanTpRxIndication(rxConfig->PduR_PduId, NTFRSLT_E_NO_BUFFER); /** @req CANTP084 */
				rxRuntime->iso15765.state = IDLE;
				rxRuntime->mode = CANTP_RX_WAIT;
			} else if (ret == BUFREQ_BUSY) {
				boolean dataCopyFailure = FALSE;
				PduLengthType bytesNotCopiedToPdurRxBuffer = currentSegmentSize - bytesCopiedToPdurRxBuffer;
				if (rxConfig->CanTpAddressingFormant == CANTP_STANDARD) {
					if ( copySegmentToLocalRxBuffer(rxRuntime,	/** @req CANTP067 */
							&rxPduData->SduDataPtr[1 + bytesCopiedToPdurRxBuffer],
							bytesNotCopiedToPdurRxBuffer ) != TRUE ) {
						rxRuntime->iso15765.state = IDLE;
						rxRuntime->mode = CANTP_RX_WAIT;
						dataCopyFailure = TRUE;
						DEBUG( DEBUG_MEDIUM, "Unexpected error, could not copy 'unaligned leftover' " "data to local buffer!\n");
					}
				} else {
					if ( copySegmentToLocalRxBuffer(rxRuntime,  /** @req CANTP067 */
							&rxPduData->SduDataPtr[2 + bytesCopiedToPdurRxBuffer],
							bytesNotCopiedToPdurRxBuffer) != TRUE ) {
						rxRuntime->iso15765.state = IDLE;
						rxRuntime->mode = CANTP_RX_WAIT;
						dataCopyFailure = TRUE;
						DEBUG( DEBUG_MEDIUM, "Unexpected error, could not copy 'unaligned leftover' " "data to local buffer!\n");
					}
				}
				if ( !dataCopyFailure ) {
					rxRuntime->iso15765.framesHandledCount++;
					rxRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(rxConfig->CanTpNbr);
					rxRuntime->iso15765.state = RX_WAIT_SDU_BUFFER;
					rxRuntime->mode = CANTP_RX_PROCESSING;
					sendFlowControlFrame(rxConfig, rxRuntime, ret);  /** @req CANTP082 */
				}
			} else if (ret == BUFREQ_OK) {
				bytesLeftToTransfer = rxRuntime->transferTotal - rxRuntime->transferCount;
				if (bytesLeftToTransfer > 0) {
					rxRuntime->iso15765.framesHandledCount++;
					COUNT_DECREMENT(rxRuntime->iso15765.nextFlowControlCount);
					if (rxRuntime->iso15765.nextFlowControlCount == 0  && rxRuntime->iso15765.BS > 0) {
						sendFlowControlFrame(rxConfig, rxRuntime, BUFREQ_OK);
					} else {
						rxRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(rxConfig->CanTpNcr);  //UH
					}
				} else {
					DEBUG( DEBUG_MEDIUM,"ISO15765-Rx session finished, going back to IDLE!\n");
					rxRuntime->iso15765.state = IDLE;
					rxRuntime->mode = CANTP_RX_WAIT;
					PduR_CanTpRxIndication(rxConfig->PduR_PduId, NTFRSLT_OK); /** @req CANTP084 */
				}
			}
		}
	}
} // 438, 550 PC-lint: extendedAdress not accessed. Extended adress needs to be implemented. Ticket #136

// - - - - - - - - - - - - - -

static BufReq_ReturnType sendNextTxFrame(
		const CanTp_TxNSduType *txConfig, CanTp_ChannelPrivateType *txRuntime) {
	BufReq_ReturnType ret = BUFREQ_OK;

	// copy data to temp buffer
	for(; txRuntime->canFrameBuffer.byteCount < MAX_SEGMENT_DATA_SIZE && ret == BUFREQ_OK ;) {
		if(txRuntime->pdurBuffer == 0 || txRuntime->pdurBufferCount == txRuntime->pdurBuffer->SduLength) {
			// data empty, request new data
			ret = PduR_CanTpProvideTxBuffer(txConfig->PduR_PduId, &txRuntime->pdurBuffer, 0);
			txRuntime->pdurBufferCount = 0;
			if(ret == BUFREQ_OK) {
				// new data received
				VALIDATE( txRuntime->pdurBuffer->SduDataPtr != NULL,
						SERVICE_ID_CANTP_TRANSMIT, CANTP_E_INVALID_TX_BUFFER );
			} else {
				// failed to receive new data
				txRuntime->pdurBuffer = 0;
				break;
			}
		}
		txRuntime->canFrameBuffer.data[txRuntime->canFrameBuffer.byteCount++] = txRuntime->pdurBuffer->SduDataPtr[txRuntime->pdurBufferCount++];
		txRuntime->transferCount++;
		if(txRuntime->transferCount == txRuntime->transferTotal) {
			// all bytes, send
			break;
		}
	}
	if(ret == BUFREQ_OK) {
		PduInfoType pduInfo;
		Std_ReturnType resp;
		pduInfo.SduDataPtr = txRuntime->canFrameBuffer.data;
		pduInfo.SduLength = txRuntime->canFrameBuffer.byteCount;
		// change state to verify tx confirm within timeout
		txRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(txConfig->CanTpNas);  /** @req CANTPxxx */
		txRuntime->iso15765.state = TX_WAIT_TX_CONFIRMATION;
		resp = canTansmitPaddingHelper(txConfig, txRuntime, &pduInfo);
		if(resp == E_OK) {
			// sending done
		} else {
			// failed to send
			ret = BUFREQ_NOT_OK;
		}
	}
	return ret;
}

// - - - - - - - - - - - - - -

static void handleNextTxFrameSent(
		const CanTp_TxNSduType *txConfig, CanTp_ChannelPrivateType *txRuntime) {

	txRuntime->iso15765.framesHandledCount++;
	// prepare tx buffer for next frame
	txRuntime->canFrameBuffer.byteCount = 1;
	if (txConfig->CanTpAddressingMode == CANTP_EXTENDED) {
		txRuntime->canFrameBuffer.byteCount++;
	}
	txRuntime->canFrameBuffer.data[txRuntime->canFrameBuffer.byteCount - 1] =
			(txRuntime->iso15765.framesHandledCount & SEGMENT_NUMBER_MASK) + ISO15765_TPCI_CF;
	COUNT_DECREMENT(txRuntime->iso15765.nextFlowControlCount);
	if (txRuntime->transferTotal <= txRuntime->transferCount) {
		// Transfer finished!
		PduR_CanTpTxConfirmation(txConfig->PduR_PduId, NTFRSLT_OK); /** @req CANTP074 *//** @req CANTP09 *//** @req CANTP204 */
		txRuntime->iso15765.state = IDLE;
		txRuntime->mode = CANTP_TX_WAIT;
	} else if (txRuntime->iso15765.nextFlowControlCount == 0 && txRuntime->iso15765.BS) {
		// receiver expects flow control.
		txRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(txConfig->CanTpNbs);  /** @req CANTP264 */
		txRuntime->iso15765.state = TX_WAIT_FLOW_CONTROL;
	} else if (txRuntime->iso15765.STmin == 0) {
		// Send next consecutive frame!
		Std_ReturnType resp;
		resp = sendNextTxFrame(txConfig, txRuntime);
		if (resp == BUFREQ_OK ) {
			// successfully sent frame, wait for tx confirm
		} else if(BUFREQ_BUSY == resp) {
			// change state and setup timeout
			txRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(txConfig->CanTpNcs);
			txRuntime->iso15765.state = TX_WAIT_TRANSMIT;
		} else {
			PduR_CanTpTxConfirmation(txConfig->PduR_PduId, NTFRSLT_E_NOT_OK);  /** @req CANTP177 */ /** @req CANTP084 */
			txRuntime->iso15765.state = IDLE;
			txRuntime->mode = CANTP_TX_WAIT;
		}
	} else {
		// Send next consecutive frame after stmin!
		//ST MIN error handling ISO 15765-2 sec 7.6
		if (txRuntime->iso15765.STmin < 0x80) {
			txRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(txRuntime->iso15765.STmin) + 1;
		} else if (txRuntime->iso15765.STmin > 0xF0 && txRuntime->iso15765.STmin < 0xFA) {
			txRuntime->iso15765.stateTimeoutCount = 1; //0.1 ms resoultion needs a lower task period. So hard coded to 1 main cycle
		}
		else {
			txRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(0x7F) + 1;
		}
		txRuntime->iso15765.state = TX_WAIT_STMIN;
	}
}

// - - - - - - - - - - - - - -

static void handleFlowControlFrame(const CanTp_TxNSduType *txConfig,
		CanTp_ChannelPrivateType *txRuntime, const PduInfoType *txPduData) {
	int indexCount = 0;
	uint8 extendedAddress = 0;

	if ( txRuntime->iso15765.state == TX_WAIT_FLOW_CONTROL ) {
		if (txConfig->CanTpAddressingMode == CANTP_EXTENDED) { /** @req CANTP094 *//** @req CANTP095 */
			extendedAddress = txPduData->SduDataPtr[indexCount++];
		}
		switch (txPduData->SduDataPtr[indexCount++] & ISO15765_TPCI_FS_MASK) {
		case ISO15765_FLOW_CONTROL_STATUS_CTS:
#if 1
		{	// This construction is added to make the hcs12 compiler happy.
			const uint8 bs = txPduData->SduDataPtr[indexCount++];
			txRuntime->iso15765.BS = bs;
			txRuntime->iso15765.nextFlowControlCount = bs;
		}
		txRuntime->iso15765.STmin = txPduData->SduDataPtr[indexCount++];
#else
		txRuntime->iso15765.BS = txPduData->SduDataPtr[indexCount++];
		txRuntime->iso15765.nextFlowControlCount = txRuntime->iso15765.BS;
		txRuntime->iso15765.STmin = txPduData->SduDataPtr[indexCount++];
#endif
		DEBUG( DEBUG_MEDIUM, "txRuntime->iso15765.STmin = %d\n", txRuntime->iso15765.STmin);
		// change state and setup timout
		txRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(txConfig->CanTpNcs);
		txRuntime->iso15765.state = TX_WAIT_TRANSMIT;
		break;
		case ISO15765_FLOW_CONTROL_STATUS_WAIT:
			txRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(txConfig->CanTpNbs);  /*CanTp: 264*/
			txRuntime->iso15765.state = TX_WAIT_FLOW_CONTROL;
			break;
		case ISO15765_FLOW_CONTROL_STATUS_OVFLW:
			PduR_CanTpRxIndication(txConfig->PduR_PduId, NTFRSLT_E_NOT_OK);
			txRuntime->iso15765.state = IDLE;
			txRuntime->mode = CANTP_TX_WAIT;
			break;
		}
	} else {
		DEBUG( DEBUG_MEDIUM, "Ignoring flow control, we do not expect it!");
	}
} // 438, 550 PC-lint: extendAdress används inte. EN BUG? Behöver fixas


// - - - - - - - - - - - - - -

static void handleSingleFrame(const CanTp_RxNSduType *rxConfig,
		CanTp_ChannelPrivateType *rxRuntime, const PduInfoType *rxPduData) {
	BufReq_ReturnType ret;
	PduLengthType pduLength;
	uint8 *data = NULL;
	PduLengthType bytesWrittenToSduRBuffer;


	if (rxRuntime->iso15765.state != IDLE) {
		PduR_CanTpRxIndication(rxConfig->PduR_PduId, NTFRSLT_E_NOT_OK);  // Abort current reception, we need to tell the current receiver it has been aborted.
		DEBUG( DEBUG_MEDIUM, "Single frame received and channel not IDLE!\n");
	}
	(void) initRx15765RuntimeData(rxConfig, rxRuntime); /** @req CANTP124 */
	pduLength = getPduLength(&rxConfig->CanTpAddressingFormant, SINGLE_FRAME, rxPduData);

	if (rxRuntime->pdurBuffer != NULL) { // vad är detta? initieras ju till null
		VALIDATE_NO_RV( rxRuntime->pdurBuffer->SduDataPtr != NULL,
				SERVICE_ID_CANTP_RX_INDICATION, CANTP_E_INVALID_RX_LENGTH );
	}

	if (rxConfig->CanTpAddressingFormant == CANTP_STANDARD) { /** @req CANTP094 *//** @req CANTP095 */
		data = &rxPduData->SduDataPtr[1];
	} else {
		data = &rxPduData->SduDataPtr[2];
	}
	rxRuntime->transferTotal = pduLength;
	rxRuntime->iso15765.state = SF_OR_FF_RECEIVED_WAITING_PDUR_BUFFER;
	rxRuntime->mode = CANTP_RX_PROCESSING;
	rxRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(rxConfig->CanTpNbr);  /** @req CANTP166 */

	ret = copySegmentToPduRRxBuffer(rxConfig, rxRuntime, data, pduLength, &bytesWrittenToSduRBuffer);

	if (ret == BUFREQ_OK) {
		PduR_CanTpRxIndication(rxConfig->PduR_PduId, NTFRSLT_OK);
		rxRuntime->iso15765.state = IDLE;
		rxRuntime->mode = CANTP_RX_WAIT;
	} else if (ret == BUFREQ_BUSY) {
		if (rxConfig->CanTpAddressingFormant == CANTP_STANDARD) {
			data = &rxPduData->SduDataPtr[1];
		} else {
			data = &rxPduData->SduDataPtr[2];
		}
		(void)copySegmentToLocalRxBuffer(rxRuntime, data, pduLength ); /** @req CANTP067 */
		rxRuntime->iso15765.state = RX_WAIT_SDU_BUFFER;
		rxRuntime->mode = CANTP_RX_PROCESSING;
	} else {
		PduR_CanTpRxIndication(rxConfig->PduR_PduId, NTFRSLT_E_NO_BUFFER);
		rxRuntime->iso15765.state = IDLE;
		rxRuntime->mode = CANTP_RX_WAIT;
	}
}

// - - - - - - - - - - - - - -


static void handleFirstFrame(const CanTp_RxNSduType *rxConfig,
		CanTp_ChannelPrivateType *rxRuntime, const PduInfoType *rxPduData) {
	BufReq_ReturnType ret;
	PduLengthType pduLength = 0;
	PduLengthType bytesWrittenToSduRBuffer;

	if (rxRuntime->iso15765.state != IDLE) {
		DEBUG( DEBUG_MEDIUM, "First frame received during Rx-session!\n" );
		PduR_CanTpRxIndication(rxConfig->PduR_PduId, NTFRSLT_E_NOT_OK);  // Abort current reception, we need to tell the current receiver it has been aborted.
	}

	(void) initRx15765RuntimeData(rxConfig, rxRuntime); /** @req CANTP124 */
	pduLength = getPduLength(&rxConfig->CanTpAddressingFormant, FIRST_FRAME,
			rxPduData);
	rxRuntime->transferTotal = pduLength;

	VALIDATE_NO_RV( rxRuntime->transferTotal != 0,
			SERVICE_ID_CANTP_RX_INDICATION, CANTP_E_INVALID_RX_LENGTH );

	// Validate that that there is a reason for using the segmented transfers and
	// if not simply skip (single frame should have been used).
	if (rxConfig->CanTpAddressingFormant == CANTP_STANDARD) { /** @req CANTP094 *//** @req CANTP095 */
		if (pduLength <= MAX_PAYLOAD_SF_STD_ADDR){
			return;
		}
	} else {
		if (pduLength <= MAX_PAYLOAD_SF_EXT_ADDR){
			return;
		}
	}
	// Validate that the SDU is full length in this first frame.
	if (rxPduData->SduLength < MAX_SEGMENT_DATA_SIZE) {
		return;
	}

	rxRuntime->iso15765.framesHandledCount = 1; // Segment count begins with 1 (FirstFrame has the 0).
	rxRuntime->iso15765.state = SF_OR_FF_RECEIVED_WAITING_PDUR_BUFFER;
	rxRuntime->mode = CANTP_RX_PROCESSING;
	rxRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(rxConfig->CanTpNbr); /** @req CANTP166 */

	if (rxConfig->CanTpAddressingFormant == CANTP_STANDARD) {
		ret = copySegmentToPduRRxBuffer(rxConfig, rxRuntime,
				&rxPduData->SduDataPtr[2],
				MAX_PAYLOAD_FF_STD_ADDR,
				&bytesWrittenToSduRBuffer);
	} else {
		ret = copySegmentToPduRRxBuffer(rxConfig, rxRuntime,
				&rxPduData->SduDataPtr[3],
				MAX_PAYLOAD_FF_EXT_ADDR,
				&bytesWrittenToSduRBuffer);
	}
	if (ret == BUFREQ_OK) {
		rxRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(rxConfig->CanTpNcr);
		rxRuntime->iso15765.state = RX_WAIT_CONSECUTIVE_FRAME;
		rxRuntime->mode = CANTP_RX_PROCESSING;
		sendFlowControlFrame(rxConfig, rxRuntime, ret);
	} else if (ret == BUFREQ_BUSY) {
		/** @req CANTP222 */
		if (rxConfig->CanTpAddressingFormant == CANTP_STANDARD) {
			(void)copySegmentToLocalRxBuffer(rxRuntime, &rxPduData->SduDataPtr[2], MAX_PAYLOAD_FF_STD_ADDR );
		} else {
			(void)copySegmentToLocalRxBuffer(rxRuntime, &rxPduData->SduDataPtr[3], MAX_PAYLOAD_FF_EXT_ADDR );
		}
		rxRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(rxConfig->CanTpNbr);
		rxRuntime->iso15765.state = RX_WAIT_SDU_BUFFER;
		rxRuntime->mode = CANTP_RX_PROCESSING;
		sendFlowControlFrame(rxConfig, rxRuntime, ret);  /** @req CANTP082 */
	} else if (ret == BUFREQ_OVFL) {
		sendFlowControlFrame(rxConfig, rxRuntime, ret); /** @req CANTP081 */
		rxRuntime->iso15765.state = IDLE;
		rxRuntime->mode = CANTP_RX_WAIT;
	}
}

// - - - - - - - - - - - - - -

static ISO15765FrameType calcRequiredProtocolFrameType(
		const CanTp_TxNSduType *txConfig, CanTp_ChannelPrivateType *txRuntime) {

	ISO15765FrameType ret = INVALID_FRAME;
	if (txConfig->CanTpAddressingMode == CANTP_EXTENDED) {
		if (txRuntime->transferTotal <= MAX_PAYLOAD_CF_EXT_ADDR) {
			ret = SINGLE_FRAME;
		} else {
			if (txConfig->CanTpTxTaType == CANTP_PHYSICAL) {
				ret = FIRST_FRAME;
			} else {
				DET_REPORTERROR( MODULE_ID_CANTP, 0, SERVICE_ID_CANTP_TRANSMIT, CANTP_E_INVALID_TATYPE );
			}
		}
	} else {	// CANTP_STANDARD
		if (txRuntime->transferTotal <= MAX_PAYLOAD_CF_STD_ADDR) {
			ret = SINGLE_FRAME;
		} else {
			if (txConfig->CanTpTxTaType == CANTP_PHYSICAL) {
				ret = FIRST_FRAME;
			} else {
				DET_REPORTERROR( MODULE_ID_CANTP, 0, SERVICE_ID_CANTP_TRANSMIT, CANTP_E_INVALID_TATYPE );
			}
		}
	}

	return ret;
}

// - - - - - - - - - - - - - -

Std_ReturnType CanTp_Transmit(PduIdType CanTpTxSduId,
		const PduInfoType *CanTpTxInfoPtr)  /** @req CANTP176 */
{
	const CanTp_TxNSduType *txConfig = NULL;
	CanTp_ChannelPrivateType *txRuntime = NULL;
	Std_ReturnType ret = 0;
	PduIdType CanTp_InternalTxNSduId;

	DEBUG( DEBUG_MEDIUM, "CanTp_Transmit called in polite index: %d!\n", CanTpTxSduId);

	VALIDATE( CanTpTxInfoPtr != NULL,
			SERVICE_ID_CANTP_TRANSMIT, CANTP_E_PARAM_ADDRESS ); /** @req CANTP031 */
	VALIDATE( CanTpRunTimeData.internalState == CANTP_ON, /** @req CANTP238 */
			SERVICE_ID_CANTP_TRANSMIT, CANTP_E_UNINIT ); /** @req CANTP031 */
	VALIDATE( CanTpTxSduId < CANTP_RXID_LIST_SIZE, SERVICE_ID_CANTP_TRANSMIT, CANTP_E_INVALID_TX_ID );

	if( CanTpConfig.CanTpRxIdList[CanTpTxSduId].CanTpNSduIndex != 0xFFFF ) {
		CanTp_InternalTxNSduId = CanTpConfig.CanTpRxIdList[CanTpTxSduId].CanTpNSduIndex;

		txConfig =&CanTpConfig.CanTpNSduList[CanTp_InternalTxNSduId].configData.CanTpTxNSdu;

		txRuntime = &CanTpRunTimeData.runtimeDataList[txConfig->CanTpTxChannel]; // Runtime data.
		if (txRuntime->iso15765.state == IDLE) {
			ISO15765FrameType iso15765Frame;
			txRuntime->canFrameBuffer.byteCount = 0;
			txRuntime->pdurBuffer = 0;
			txRuntime->transferCount = 0;
			txRuntime->iso15765.framesHandledCount = 0;
			txRuntime->transferTotal = CanTpTxInfoPtr->SduLength; /** @req CANTP225 */
			txRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(txConfig->CanTpNcs); /** @req CANTP167 */
			txRuntime->mode = CANTP_TX_PROCESSING;
			iso15765Frame = calcRequiredProtocolFrameType(txConfig, txRuntime);
			if (txConfig->CanTpAddressingMode == CANTP_EXTENDED) { /** @req CANTP094 *//** @req CANTP095 */
				txRuntime->canFrameBuffer.data[txRuntime->canFrameBuffer.byteCount++] =
						(uint8) txConfig->CanTpNTa->CanTpNTa; // Target address.
			}
			switch(iso15765Frame) {
			case SINGLE_FRAME:
				txRuntime->canFrameBuffer.data[txRuntime->canFrameBuffer.byteCount++] =
						ISO15765_TPCI_SF | (uint8)(txRuntime->transferTotal);
				ret = E_OK;
				txRuntime->iso15765.state = TX_WAIT_TRANSMIT;
				break;
			case FIRST_FRAME:
				txRuntime->canFrameBuffer.data[txRuntime->canFrameBuffer.byteCount++] =
						ISO15765_TPCI_FF | (uint8)((txRuntime->transferTotal & 0xf00) >> 8);
				txRuntime->canFrameBuffer.data[txRuntime->canFrameBuffer.byteCount++] =
						(uint8)(txRuntime->transferTotal & 0xff);
				// setup block size so that state machine waits for flow control after first frame
				txRuntime->iso15765.nextFlowControlCount = 1;
				txRuntime->iso15765.BS = 1;
				ret = E_OK;
				txRuntime->iso15765.state = TX_WAIT_TRANSMIT;
				break;
			default:
				ret = E_NOT_OK;
			}
		} else {
			DEBUG( DEBUG_MEDIUM, "CanTp can't transmit, it is already occupied!\n", CanTpTxSduId);
			ret = E_NOT_OK;  /** @req CANTP123 *//** @req CANTP206 */
		}
	}

	return ret; // CAN level error code.
}

// - - - - - - - - - - - - - -

#if FRTP_CANCEL_TRANSMIT_REQUEST
Std_ReturnType FrTp_CancelTransmitRequest(PduIdType FrTpTxPduId,
		FrTp_CancelReasonType FrTpCancelReason)
{
	return E_NOT_OK;
}
#endif

// - - - - - - - - - - - - - -


void CanTp_Init(void)
{
	CanTp_ChannelPrivateType *runtimeData;
	const CanTp_TxNSduType *txConfigParams;
	const CanTp_RxNSduType *rxConfigParams;

	for (int i=0; i < CANTP_NSDU_CONFIG_LIST_SIZE; i++) {
		if ( CanTpConfig.CanTpNSduList[i].direction == IS015765_TRANSMIT ) {
			txConfigParams = (CanTp_TxNSduType*)&CanTpConfig.CanTpNSduList[i].configData;
			if (txConfigParams->CanTpTxChannel < CANTP_NSDU_RUNTIME_LIST_SIZE) {
				runtimeData = &CanTpRunTimeData.runtimeDataList[txConfigParams->CanTpTxChannel];
			} else {
				runtimeData = &CanTpRunTimeData.runtimeDataList[CANTP_NSDU_RUNTIME_LIST_SIZE-1];
			}
			initTx15765RuntimeData( txConfigParams, runtimeData);
		} else {
			rxConfigParams = (CanTp_RxNSduType*)&CanTpConfig.CanTpNSduList[i].configData;
			if (rxConfigParams->CanTpRxChannel < CANTP_NSDU_RUNTIME_LIST_SIZE) {
				runtimeData = &CanTpRunTimeData.runtimeDataList[rxConfigParams->CanTpRxChannel];
			} else {
				runtimeData = &CanTpRunTimeData.runtimeDataList[CANTP_NSDU_RUNTIME_LIST_SIZE-1];
			}
			initRx15765RuntimeData( rxConfigParams, runtimeData);
		}
	}
	CanTpRunTimeData.internalState = CANTP_ON; /** @req CANTP170 */
}

// - - - - - - - - - - - - - -

void CanTp_RxIndication(PduIdType CanTpRxPduId, /** @req CANTP078 */ /** @req CANTP035 */
		const PduInfoType *CanTpRxPduPtr)
{

	const CanTp_RxNSduType *rxConfigParams; // Params reside in ROM.
	const CanTp_TxNSduType *txConfigParams;
	const CanTp_AddressingFormantType *addressingFormat; // Configured
	CanTp_ChannelPrivateType *runtimeParams = 0; // Params reside in RAM.
	ISO15765FrameType frameType;
	PduIdType CanTpTxNSduId, CanTpRxNSduId;

	//Check if PduId is valid
	if (CanTpRxPduId >= CANTP_RXID_LIST_SIZE)
	{
		return;
	}
	if( CanTpRunTimeData.internalState != CANTP_ON ) {
		DET_REPORTERROR(MODULE_ID_CANTP, 0, SERVICE_ID_CANTP_RX_INDICATION, CANTP_E_UNINIT );  /** @req CANTP238 */ /** @req CANTP031 */
		return;
	}
	DEBUG( DEBUG_MEDIUM, "CanTp_RxIndication: PduId=%d, [", CanTpRxPduId);
	for (int i=0; i<CanTpRxPduPtr->SduLength; i++) {
		DEBUG( DEBUG_MEDIUM, "%x, ", CanTpRxPduPtr->SduDataPtr[i]);
	}
	DEBUG( DEBUG_MEDIUM, "]");

	VALIDATE_NO_RV( CanTpRunTimeData.internalState == CANTP_ON,
			SERVICE_ID_CANTP_RX_INDICATION, CANTP_E_UNINIT ); /** @req CANTP031 */

	addressingFormat = &CanTpConfig.CanTpRxIdList[CanTpRxPduId].CanTpAddressingMode;

	/* TODO John - Use a different indication of not set than 0xFFFF? */
	frameType = getFrameType(addressingFormat, CanTpRxPduPtr); /** @req CANTP094 *//** @req CANTP095 */
	if( frameType == FLOW_CONTROL_CTS_FRAME ) {
		if( CanTpConfig.CanTpRxIdList[CanTpRxPduId].CanTpReferringTxIndex != 0xFFFF ) {
			CanTpTxNSduId = CanTpConfig.CanTpRxIdList[CanTpRxPduId].CanTpReferringTxIndex;
			txConfigParams = &CanTpConfig.CanTpNSduList[CanTpTxNSduId].configData.CanTpTxNSdu;
			runtimeParams = &CanTpRunTimeData.runtimeDataList[txConfigParams->CanTpTxChannel];
		}
		else {
			//Invalid FC received
			return;
		}
		rxConfigParams = NULL;
	}
	else {
		if( CanTpConfig.CanTpRxIdList[CanTpRxPduId].CanTpNSduIndex != 0xFFFF ) {
			CanTpRxNSduId = CanTpConfig.CanTpRxIdList[CanTpRxPduId].CanTpNSduIndex;
			rxConfigParams = &CanTpConfig.CanTpNSduList[CanTpRxNSduId].configData.CanTpRxNSdu;  /** @req CANTP120 */
			runtimeParams = &CanTpRunTimeData.runtimeDataList[rxConfigParams->CanTpRxChannel];  /** @req CANTP096 *//** @req CANTP121 *//** @req CANTP122 *//** @req CANTP190 */
		}
		else {
			//Invalid Frame received
			return;
		}
		txConfigParams = NULL;
	}



	switch (frameType) {
	case SINGLE_FRAME: {
		if (rxConfigParams != NULL) {
			DEBUG( DEBUG_MEDIUM, "calling handleSingleFrame!\n");
			handleSingleFrame(rxConfigParams, runtimeParams, CanTpRxPduPtr);
		}
		else{
			DEBUG( DEBUG_MEDIUM, "Single frame received on ISO15765-Tx - is ignored!\n");
		}
		break;
	}
	case FIRST_FRAME: {
		if (rxConfigParams != NULL) {
			DEBUG( DEBUG_MEDIUM, "calling handleFirstFrame!\n");
			handleFirstFrame(rxConfigParams, runtimeParams, CanTpRxPduPtr);
		}else{
			DEBUG( DEBUG_MEDIUM, "First frame received on ISO15765-Tx - is ignored!\n");
		}
		break;
	}
	case CONSECUTIVE_FRAME: {
		if (rxConfigParams != NULL) {
			DEBUG( DEBUG_MEDIUM, "calling handleConsecutiveFrame!\n");
			handleConsecutiveFrame(rxConfigParams, runtimeParams, CanTpRxPduPtr);
		} else {
			DEBUG( DEBUG_MEDIUM, "Consecutive frame received on ISO15765-Tx - is ignored!\n");
		}
		break;
	}
	case FLOW_CONTROL_CTS_FRAME: {
		if (txConfigParams != NULL) {
			DEBUG( DEBUG_MEDIUM, "calling handleFlowControlFrame!\n");
			handleFlowControlFrame(txConfigParams, runtimeParams, CanTpRxPduPtr);
		} else {
			DEBUG( DEBUG_MEDIUM, "Flow control frame received on ISO15765-Rx - is ignored!\n");
		}
		break;
	}
	case INVALID_FRAME: {
		DEBUG( DEBUG_MEDIUM, "INVALID_FRAME received - is ignored!\n!\n");
		break;
	}
	default:
		break;
	}
	DEBUG( DEBUG_LOW, "CanTp_RxIndication_Main exit!\n");
}

// - - - - - - - - - - - - - -

void CanTp_TxConfirmation(PduIdType CanTpTxPduId) /** @req CANTP076 */
{
	PduIdType CanTpNSduId;
	const CanTp_RxNSduType *rxConfigParams = NULL;
	const CanTp_TxNSduType *txConfigParams = NULL;

	DEBUG( DEBUG_MEDIUM, "CanTp_TxConfirmation called.\n" );

	VALIDATE_NO_RV( CanTpRunTimeData.internalState == CANTP_ON,
			SERVICE_ID_CANTP_TX_CONFIRMATION, CANTP_E_UNINIT ); /** @req CANTP031 */
	VALIDATE_NO_RV( CanTpTxPduId < CANTP_RXID_LIST_SIZE,
			SERVICE_ID_CANTP_TX_CONFIRMATION, CANTP_E_INVALID_TX_ID ); /** @req CANTP158 */

	/** @req CANTP236 */
	if( CanTpConfig.CanTpRxIdList[CanTpTxPduId].CanTpNSduIndex != 0xFFFF ) {
		CanTpNSduId = CanTpConfig.CanTpRxIdList[CanTpTxPduId].CanTpNSduIndex;
		if ( CanTpConfig.CanTpNSduList[CanTpNSduId].direction == IS015765_TRANSMIT ) {
			txConfigParams = (CanTp_TxNSduType*)&CanTpConfig.CanTpNSduList[CanTpNSduId].configData;
			CanTp_ChannelPrivateType *txRuntime = &CanTpRunTimeData.runtimeDataList[txConfigParams->CanTpTxChannel];
			if(txRuntime->iso15765.state == TX_WAIT_TX_CONFIRMATION) {
				handleNextTxFrameSent(txConfigParams, txRuntime);
			}
		} else {
			rxConfigParams = (CanTp_RxNSduType*)&CanTpConfig.CanTpNSduList[CanTpNSduId].configData;
			CanTpRunTimeData.runtimeDataList[rxConfigParams->CanTpRxChannel].iso15765.NasNarPending = FALSE;
		}
	}


}

// - - - - - - - - - - - - - -

void CanTp_Shutdown(void) /** @req CANTP202 *//** @req CANTP200 *//** @req CANTP010 */
{
	VALIDATE_NO_RV( CanTpRunTimeData.internalState == CANTP_ON,
			SERVICE_ID_CANTP_SHUTDOWN, CANTP_E_UNINIT ); /** @req CANTP031 */

	CanTpRunTimeData.internalState = CANTP_OFF;
}

// - - - - - - - - - - - - - -


static boolean checkNasNarTimeout(CanTp_ChannelPrivateType *runtimeData) {
	boolean ret = FALSE;
	if (runtimeData->iso15765.NasNarPending) {
		TIMER_DECREMENT(runtimeData->iso15765.NasNarTimeoutCount);
		if (runtimeData->iso15765.NasNarTimeoutCount == 0) {
			DEBUG( DEBUG_MEDIUM, "NAS timed out.\n" );
			runtimeData->iso15765.state = IDLE;
			runtimeData->iso15765.NasNarPending = FALSE;
			ret = TRUE;
		}
	}
	return ret;
}

// - - - - - - - - - - - - - -


void CanTp_MainFunction(void)
{
	BufReq_ReturnType ret;
	PduLengthType bytesWrittenToSduRBuffer;

	CanTp_ChannelPrivateType *txRuntimeListItem = NULL;
	CanTp_ChannelPrivateType *rxRuntimeListItem = NULL;

	const CanTp_TxNSduType *txConfigListItem = NULL;
	const CanTp_RxNSduType *rxConfigListItem = NULL;

	if( CanTpRunTimeData.internalState != CANTP_ON ) {
		DET_REPORTERROR(MODULE_ID_CANTP, 0, SERVICE_ID_CANTP_MAIN_FUNCTION, CANTP_E_UNINIT ); /** @req CANTP031 */
		return;
	}

	for( int i=0; i < CANTP_NSDU_CONFIG_LIST_SIZE; i++ ) {
		if ( CanTpConfig.CanTpNSduList[i].direction == IS015765_TRANSMIT ) {
			txConfigListItem = (CanTp_TxNSduType*)&CanTpConfig.CanTpNSduList[i].configData;
			txRuntimeListItem = &CanTpRunTimeData.runtimeDataList[txConfigListItem->CanTpTxChannel];

			switch (txRuntimeListItem->iso15765.state) {
			case TX_WAIT_STMIN:
				TIMER_DECREMENT(txRuntimeListItem->iso15765.stateTimeoutCount); // Make sure that STmin timer has expired.
				if (txRuntimeListItem->iso15765.stateTimeoutCount != 0) {
					break;
				}
				txRuntimeListItem->iso15765.state = TX_WAIT_TRANSMIT;
				txRuntimeListItem->iso15765.stateTimeoutCount =
						CANTP_CONVERT_MS_TO_MAIN_CYCLES(rxConfigListItem->CanTpNcr) -
						CANTP_CONVERT_MS_TO_MAIN_CYCLES(txRuntimeListItem->iso15765.STmin);
				// no break, continue
			case TX_WAIT_TRANSMIT: {
				ret = sendNextTxFrame(txConfigListItem, txRuntimeListItem);
				if ( ret == BUFREQ_OK ) {
					// successfully sent frame, wait for tx confirm
				} else if(BUFREQ_BUSY == ret) {
					// check N_Cs timeout
					TIMER_DECREMENT(txRuntimeListItem->iso15765.stateTimeoutCount);
					if (txRuntimeListItem->iso15765.stateTimeoutCount == 0) {
						DEBUG( DEBUG_MEDIUM, "ERROR: N_Cs timeout when sending consecutive frame!\n");
						txRuntimeListItem->iso15765.state = IDLE;
						txRuntimeListItem->mode = CANTP_TX_WAIT;
						PduR_CanTpTxConfirmation(txConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK); /** @req CANTP204 */
					} else {
						DEBUG( DEBUG_MEDIUM, "Waiting for STmin timer to expire!\n");
					}
				} else {
					DEBUG( DEBUG_MEDIUM, "ERROR: Consecutive frame could not be sent!\n");
					txRuntimeListItem->iso15765.state = IDLE;
					txRuntimeListItem->mode = CANTP_TX_WAIT;
					PduR_CanTpTxConfirmation(txConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK); /** @req CANTP204 */
				}
				break;
			}
			case TX_WAIT_FLOW_CONTROL:
				//DEBUG( DEBUG_MEDIUM, "Waiting for flow control!\n");
				TIMER_DECREMENT(txRuntimeListItem->iso15765.stateTimeoutCount);
				if (txRuntimeListItem->iso15765.stateTimeoutCount == 0) {
					DEBUG( DEBUG_MEDIUM, "State TX_WAIT_FLOW_CONTROL timed out!\n");
					txRuntimeListItem->iso15765.state = IDLE;
					txRuntimeListItem->mode = CANTP_TX_WAIT;
					PduR_CanTpTxConfirmation(txConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK); /** @req CANTP204 */ /** @req CANTP185 */
				}
				break;
			case TX_WAIT_TX_CONFIRMATION:
				TIMER_DECREMENT(txRuntimeListItem->iso15765.stateTimeoutCount);
				if (txRuntimeListItem->iso15765.stateTimeoutCount == 0) { /** @req CANTP075 */
					txRuntimeListItem->iso15765.state = IDLE;
					txRuntimeListItem->mode = CANTP_TX_WAIT;
					PduR_CanTpTxConfirmation(txConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK); /** @req CANTP074 */ /** @req CANTP204 */
				}
				break;
			default:
				break;
			}
		} else {
			rxConfigListItem = (CanTp_RxNSduType*)&CanTpConfig.CanTpNSduList[i].configData;
			rxRuntimeListItem = &CanTpRunTimeData.runtimeDataList[rxConfigListItem->CanTpRxChannel];
			switch (rxRuntimeListItem->iso15765.state) {
			case RX_WAIT_CONSECUTIVE_FRAME: {
				TIMER_DECREMENT (rxRuntimeListItem->iso15765.stateTimeoutCount);
				if (rxRuntimeListItem->iso15765.stateTimeoutCount == 0) {
					DEBUG( DEBUG_MEDIUM, "TIMEOUT!\n");
					PduR_CanTpRxIndication(rxConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK);
					rxRuntimeListItem->iso15765.state = IDLE;
					rxRuntimeListItem->mode = CANTP_RX_WAIT;
				}
				break;
			}
			case RX_WAIT_SDU_BUFFER: {
				TIMER_DECREMENT (rxRuntimeListItem->iso15765.stateTimeoutCount);
				/* We end up here if we have requested a buffer from the
				 * PDUR but the response have been BUSY. We assume
				 * we have data in our local buffer and we are expected
				 * to send a flow-control clear to send (CTS).
				 */
				if (rxRuntimeListItem->iso15765.stateTimeoutCount == 0) { /** @req CANTP223 */
					PduR_CanTpRxIndication(rxConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK); /** @req CANTP204 */
					rxRuntimeListItem->iso15765.state = IDLE;
					rxRuntimeListItem->mode = CANTP_RX_WAIT;
				} else { /** @req CANTP222 */
					PduLengthType bytesRemaining = 0;
					ret = copySegmentToPduRRxBuffer(rxConfigListItem, /** @req CANTP067, copies from local buffer to PDUR buffer. */
							rxRuntimeListItem,
							rxRuntimeListItem->canFrameBuffer.data,
							rxRuntimeListItem->canFrameBuffer.byteCount,
							&bytesWrittenToSduRBuffer);
					bytesRemaining = rxRuntimeListItem->transferTotal -
							rxRuntimeListItem->transferCount;
					if (bytesRemaining > 0) {
						sendFlowControlFrame( rxConfigListItem, rxRuntimeListItem, ret ); /** @req CANTP224 (Busy or CTS) */
					}
					if (ret == BUFREQ_OK) {
						if ( bytesRemaining > 0 ) {
							rxRuntimeListItem->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(rxConfigListItem->CanTpNcr);  //UH
							rxRuntimeListItem->iso15765.state = RX_WAIT_CONSECUTIVE_FRAME;
						} else {
							PduR_CanTpRxIndication(rxConfigListItem->PduR_PduId, NTFRSLT_OK);
							rxRuntimeListItem->iso15765.state = IDLE;
							rxRuntimeListItem->mode = CANTP_RX_WAIT;
						}
					} else if (ret == BUFREQ_NOT_OK ) {
						PduR_CanTpRxIndication(rxConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK); /** @req CANTP205 */
						rxRuntimeListItem->iso15765.state = IDLE;
						rxRuntimeListItem->mode = CANTP_RX_WAIT;
					} else if ( ret == BUFREQ_BUSY ) {
						DEBUG( DEBUG_MEDIUM, "Still busy!\n");
					}
				}
				break;
			}
			default:
				break;
			}
		}
	}
}


