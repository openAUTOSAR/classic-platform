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

#define USE_DEBUG

#include "Det.h"
#include "CanIf.h"
#include "CanTp_Cfg.h" /** req: CanTp156.1 **/
#include "CanTp_Cbk.h" /** req: CanTp156.2 **/
#include "CanTp.h" /** req: CanTp156.3 **/
#include "SchM_CanTp.h" /** req: CanTp156.4 **/
//#include "MemMap.h" /** req: CanTp156.5 **/
#include "Trace.h"

#if  ( CANTP_DEV_ERROR_DETECT == STD_ON )

#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_CANTP, 0, _api, _err); \
          return E_NOT_OK; \
        }

#define VALIDATE_NO_RV(_exp,_api,_err ) \
  if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_CANTP, 0, _api, _err); \
          return; \
        }
#undef DET_REPORTERROR
#define DET_REPORTERROR(_x,_y,_z,_q) Det_ReportError(_x, _y, _z, _q)

#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_NO_RV(_exp,_api,_err )
#define DET_REPORTERROR(_x,_y,_z,_q)
#endif

/****************
 *
 * Dummy implementation.
 *
 ***************/

#define NTFRSLT_E_NO_BUFFER 123
#define NTFRSLT_E_WRONG_SN 234
#define NTFRSLT_E_NOT_OK 987

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

//#define INLINE inline
#define INLINE

#define TIMER_DECREMENT(timer) \
	if (timer > 0) { \
		timer = timer - 1; \
	} \

#define CANTP_ERR       							-1
#define ISO15765_FLOW_CONTROL_STATUS_CTS   		0
#define ISO15765_FLOW_CONTROL_STATUS_WAIT  		1
#define ISO15765_FLOW_CONTROL_STATUS_OVFLW 		2

// - - - - - - - - - - - - - -

#define ISO15765_TPCI_MASK      0x30
#define ISO15765_TPCI_SF        0x00  /* Single Frame */
#define ISO15765_TPCI_FF        0x10  /* First Frame */
#define ISO15765_TPCI_CF        0x20  /* Consecutive Frame */
#define ISO15765_TPCI_FC        0x30  /* Flow Control */
#define ISO15765_TPCI_DL        0x7   /* Single frame data length mask */
#define ISO15765_TPCI_FS_MASK   0x0F  /* Flowcontrol status mask*/

// - - - - - - - - - - - - - -

#define MAX_PAYLOAD_SF_STD_ADDR		7
#define MAX_PAYLOAD_SF_EXT_ADDR		6

#define MAX_PAYLOAD_FF_STD_ADDR		6
#define MAX_PAYLOAD_FF_EXT_ADDR		5

#define MAX_PAYLOAD_CF_STD_ADDR		7
#define MAX_PAYLOAD_CF_EXT_ADDR		6

#define SEGMENT_NUMBER_MASK			0x0f

/*
 *
 */
typedef enum {
	UNINITIALIZED, IDLE, SF_OR_FF_RECEIVED_WAITING_PDUR_BUFFER, /** Req: CanTp079. */
	RX_WAIT_CONSECUTIVE_FRAME, RX_WAIT_SDU_BUFFER,

	TX_WAIT_CAN_TP_TRANSMIT_CAN_TP_PROVIDE_TX_BUFFER, /** req: CanTp 226. */
	TX_WAIT_CAN_TP_TRANSMIT_PENDING, /* CanTP_Transmit was called but no buffer was received (BUSY). */
	TX_WAIT_SEND_CONSECUTIVE_FRAME, TX_WAIT_FLOW_CONTROL
} ISO15765TransferStateTypes;

typedef enum {
	INVALID_FRAME, /* Not specified by ISO15765 - used as error return type when decoding frame. */
	SINGLE_FRAME, FIRST_FRAME, CONSECUTIVE_FRAME, FLOW_CONTROL_CTS_FRAME, /* Clear to send */
	FLOW_CONTROL_WAIT_FRAME, FLOW_CONTROL_OVERFLOW_FRAME
} ISO15765FrameType;

typedef enum {
	SEND_NEXT_CONSECUTIVE_FRAME, WAIT_FLOW_CONTROL, TRANSFER_FINISHED
} ISO15765TxStateTypes;

/*
 *  In case no buffer is available at some cases the data needs to be
 *  temporarly stored away.
 */
typedef struct {
	uint8 data[8];
	PduLengthType byteCount;
} CanIfSduType;

/*
 *  Structure that is keeping track on the run-time variables for the ongoing
 *  transfer.
 */
typedef struct {
	uint16 nextFlowControlCount; // Count down to next Flow Control.
	uint16 framesHandledCount; // Counter keeping track total frames handled.
	uint8 extendedAddress; // Not always used but need to be available.
	uint32 stateTimeoutCount; // Counter for timeout.
	uint8 STmin; // In case we are transmitters the remote node can configure this value (only valid for TX).
	uint8 BS; // Blocksize (only valid for TX).
	boolean NasNarPending;
	uint32 NasNarTimeoutCount; // CanTpNas, CanTpNar.
	ISO15765TransferStateTypes state; // Transfer state machine. qqq: Can this be initialized here?
} ISO15765TransferControlType;

/*
 *	Container for TX or RX runtime paramters (TX/RX are identical?)
 */
typedef struct {
	ISO15765TransferControlType iso15765;
	PduInfoType *bufferPduRouter; // The PDUR make an instance of this.
	PduLengthType pdurBufferCount; // Number of bytes in PDUR buffer.
	PduLengthType pduLenghtTotalBytes; // Total length of the PDU.
	PduLengthType pduTransferedBytesCount; // Counter ongoing transfer.
	CanIfSduType canFrameBuffer; // Temp storage of SDU data.
	CanTp_TransferInstanceMode mode; // CanTp030.
} CanTp_ChannelPrivateType;


#define CANIF_PDU_MAX_LENGTH					0x08  // Max length is 8 (it is CAN dlc=8).

typedef struct {
	PduIdType PduId;
	uint8 SduData[CANIF_PDU_MAX_LENGTH];
	PduLengthType SduLength;
} CanTpFifoQueueItem;

#define FIFO_QUEUE_DEPTH	10

typedef struct {
	int fifoQueueReadIndex;
	int fifoQueueWriteIndex;
	CanTpFifoQueueItem queueList[FIFO_QUEUE_DEPTH];
} CanTpFifo;


/*
 *	Run-time CANTP/ISO15765 parameters 1:1 mapped to configuration generated by tool,
 *	see file CanTp_PBcfg.c.
 */
typedef struct {
	boolean initRun;
	CanTpFifo fifo;
	CanTp_StateType internalState; /** req: CanTp 027 */
	CanTp_ChannelPrivateType txNSduData[CANTP_TX_NSDU_CONFIG_LIST_CNT];
	CanTp_ChannelPrivateType rxNSduData[CANTP_RX_NSDU_CONFIG_LIST_CNT];
} CanTp_RunTimeDataType;



/*
 *	Container for all runtime parameters for 15765 implementation.
 */
CanTp_RunTimeDataType CanTpRunTimeData = { .initRun = FALSE,
		.internalState = CANTP_OFF };

void fifoQueueInit( CanTpFifo *fifoQueue ) {
	fifoQueue->fifoQueueReadIndex = 0;
	fifoQueue->fifoQueueWriteIndex = 0;
}

/*
 *
 */
boolean fifoQueueRead( CanTpFifo *fifoQueue, CanTpFifoQueueItem *item ) {
	boolean ret = FALSE;
	int readIndex = 0;
	readIndex = fifoQueue->fifoQueueReadIndex;
	if ( fifoQueue->fifoQueueReadIndex !=
			fifoQueue->fifoQueueWriteIndex ) {

		if (++readIndex == FIFO_QUEUE_DEPTH) {
			readIndex = 0;
		}
		item->PduId = fifoQueue->queueList[fifoQueue->fifoQueueReadIndex].PduId;
		item->SduLength = fifoQueue->queueList[fifoQueue->fifoQueueReadIndex].SduLength;
		for (int i=0; i<item->SduLength; i++) {
			item->SduData[i] = fifoQueue->queueList[fifoQueue->fifoQueueReadIndex].SduData[i];
		}
		fifoQueue->fifoQueueReadIndex = readIndex;
		ret = TRUE;
	} else {
		ret = FALSE;
	}
	return ret;
}

/*
 *
 */
boolean fifoQueueWrite( CanTpFifo *fifoQueue, CanTpFifoQueueItem *item ) {
	boolean ret = FALSE;
	int writeIndex;
	writeIndex = fifoQueue->fifoQueueWriteIndex;
	if (++writeIndex == FIFO_QUEUE_DEPTH) {
		writeIndex = 0;
	}
	if ( writeIndex != fifoQueue->fifoQueueReadIndex ) { // Check if space left.
		fifoQueue->queueList[fifoQueue->fifoQueueWriteIndex].PduId = item->PduId;
		fifoQueue->queueList[fifoQueue->fifoQueueWriteIndex].SduLength = item->SduLength;
		for (int i=0; i<item->SduLength;i++) {
			fifoQueue->queueList[fifoQueue->fifoQueueWriteIndex].SduData[i] = item->SduData[i];
		}
		fifoQueue->fifoQueueWriteIndex = writeIndex;
		ret = TRUE;
	}
	return ret;
}


/*
 * 	Helper for finding runtime parameter index for given PDU-Id.
 */
static inline int getCanTpRxNSduConfigListIndex(CanTp_ConfigType *config,
		PduIdType PduId) {
	for (int i = 0; i < CANTP_RX_NSDU_CONFIG_LIST_CNT; i++) {
		if (config->CanTpRxNSduList[i].CanTpRxPduId == PduId) {
			return i;
		} else if (config->CanTpRxNSduList[i].CanTpListItemType
				== CANTP_END_OF_LIST) {
			break;
		}
	}
	return CANTP_ERR;
}

/*
 * 	Helper for finding runtime parameter index for given PDU-Id.
 */
static INLINE int getCanTpTxNSduConfigListIndex(CanTp_ConfigType *config,
		PduIdType PduId) {
	for (int i = 0; i < CANTP_RX_NSDU_CONFIG_LIST_CNT; i++) {
		if (config->CanTpTxNSduList[i].CanTpTxPduId == PduId) {
			return i;
		} else if (config->CanTpTxNSduList[i].CanTpListItemType
				== CANTP_END_OF_LIST) {
			break;
		}
	}
	return CANTP_ERR;
}

/*
 * 	Helper for decoding the frame type received.
 */
static inline ISO15765FrameType getFrameType(
		const CanTp_AddressingFormantType *formatType,
		const PduInfoType *CanTpRxPduPtr) {
	ISO15765FrameType res = INVALID_FRAME;
	uint8 tpci = 0;

	switch (*formatType) {
	case CANTP_STANDARD:
		tpci = CanTpRxPduPtr->SduDataPtr[0];
		break;
	case CANTP_EXTENDED:
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

/*
 * 	Helper for decoding the PDU length.
 */
static inline PduLengthType getPduLength(
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
		res = CanTpRxPduPtr->SduDataPtr[tpci_offset + 1] + (PduLengthType)(
				(CanTpRxPduPtr->SduDataPtr[tpci_offset]) & 0xf << 8);
		break;
	default:
		res = 0; // qqq maybe we should have an error code here.
		break;
	}
	return res;
}

///----------------------------------------------------------------------------------------------------------------------


void initRx15765RuntimeData(const CanTp_RxNSduType *rxConfigParams,
		CanTp_ChannelPrivateType *rxRuntimeParams) {

	rxRuntimeParams->iso15765.state = IDLE;
	rxRuntimeParams->pdurBufferCount = 0;
	rxRuntimeParams->pduLenghtTotalBytes = 0;
	rxRuntimeParams->pduTransferedBytesCount = 0;
	rxRuntimeParams->mode = CANTP_RX_WAIT; /* req: CanTp030. */
	rxRuntimeParams->bufferPduRouter = NULL;
}

///----------------------------------------------------------------------------------------------------------------------


void initTx15765RuntimeData(const CanTp_TxNSduType *txConfigParams,
		CanTp_ChannelPrivateType *txRuntimeParams) {

	txRuntimeParams->iso15765.state = IDLE;
	txRuntimeParams->pdurBufferCount = 0;
	txRuntimeParams->pduLenghtTotalBytes = 0;
	txRuntimeParams->pduTransferedBytesCount = 0;
	txRuntimeParams->mode = CANTP_TX_WAIT; /* req: CanTp030. */
	txRuntimeParams->bufferPduRouter = NULL;

}

// - - - - - - - - - - - - - -

BufReq_ReturnType saveSduPayloadData(const CanTp_RxNSduType *rxConfig,
		CanTp_ChannelPrivateType *rxRuntime, uint8 *data,
		PduLengthType dataLength) {

	BufReq_ReturnType ret;
	PduLengthType copyCount = 0;
	boolean error = FALSE;

	while ((copyCount < dataLength) && error == FALSE) {
		// Copy the data that resides in the buffer.
		if (rxRuntime->bufferPduRouter != NULL) {
			while (copyCount < dataLength ) {
				rxRuntime->bufferPduRouter->SduDataPtr[rxRuntime->pdurBufferCount++]
						= data[copyCount++];
			}
		}
		if (copyCount < dataLength) {
			// We need to request a new buffer from the SDUR.
			// qqq: TODO: We should do a timeout here.
			ret = PduR_CanTpProvideRxBuffer(rxConfig->CanTpRxPduId,
							rxRuntime->pduLenghtTotalBytes,
							&rxRuntime->bufferPduRouter);
			if (ret == BUFREQ_OK) {
				VALIDATE( rxRuntime->bufferPduRouter->SduDataPtr != NULL,
						SERVICE_ID_CANTP_TRANSMIT, CANTP_E_INVALID_RX_BUFFER );
				rxRuntime->pdurBufferCount = 0; // The buffer is emptied.
			} else {
				error = TRUE;
				break;
			}
		} else {
			rxRuntime->pduTransferedBytesCount += dataLength;
			ret = BUFREQ_OK;
			break;
		}
	}
	return ret;
}

// - - - - - - - - - - - - - -

static inline Std_ReturnType canReceivePaddingHelper(
		const CanTp_RxNSduType *rxConfig, CanTp_ChannelPrivateType *rxRuntime,
		PduInfoType *PduInfoPtr) {
	if (rxConfig->CanTpRxPaddingActivation == CANTP_ON) {
		for (int i = PduInfoPtr->SduLength; i < CANIF_PDU_MAX_LENGTH; i++) {
			PduInfoPtr->SduDataPtr[i] = 0x0; // qqq: Does it have to be padded with zeroes?
		}
		PduInfoPtr->SduLength = CANIF_PDU_MAX_LENGTH;
	}
	rxRuntime->iso15765.NasNarTimeoutCount =
			CONVERT_MS_TO_MAIN_CYCLES(rxConfig->CanTpNar); // req: CanTp075.
	rxRuntime->iso15765.NasNarPending = TRUE;
	return CanIf_Transmit(rxConfig->CanTpRxPduId, PduInfoPtr);
}

// - - - - - - - - - - - - - -

static inline Std_ReturnType canTansmitPaddingHelper(
		const CanTp_TxNSduType *txConfig, CanTp_ChannelPrivateType *txRuntime,
		PduInfoType *PduInfoPtr) {
	if (txConfig->CanTpTxPaddingActivation == CANTP_ON) {
		for (int i = PduInfoPtr->SduLength; i < CANIF_PDU_MAX_LENGTH; i++) {
			PduInfoPtr->SduDataPtr[i] = 0x0; // qqq: Does it have to be padded with zeroes?
		}
		PduInfoPtr->SduLength = CANIF_PDU_MAX_LENGTH;
	}
	txRuntime->iso15765.NasNarTimeoutCount =
			CONVERT_MS_TO_MAIN_CYCLES(txConfig->CanTpNas); // req: CanTp075.
	txRuntime->iso15765.NasNarPending = TRUE;
	return CanIf_Transmit(txConfig->CanTpTxPduId, PduInfoPtr);
}

// - - - - - - - - - - - - - -

static inline void sendFlowControlFrame(const CanTp_RxNSduType *rxConfig,
		CanTp_ChannelPrivateType *rxRuntime, BufReq_ReturnType flowStatus) {
	int indexCount = 0;
	Std_ReturnType ret;
	PduInfoType pduInfo;
	uint8 sduData[8]; // qqq: Note that buffer in declared on the stack.

	DEBUG( DEBUG_MEDIUM, "sendFlowControlFrame called!\n");

	pduInfo.SduDataPtr = &sduData[0];
	if (rxConfig->CanTpAddressingFormant == CANTP_EXTENDED) {
		sduData[indexCount++] = rxRuntime->iso15765.extendedAddress;
	}

	switch (flowStatus) {
	case BUFREQ_OK:
		sduData[indexCount++] = ISO15765_TPCI_FC
				| ISO15765_FLOW_CONTROL_STATUS_CTS;
		sduData[indexCount++] = (uint8) rxConfig->CanTpBs;
		sduData[indexCount++] = (uint8) rxConfig->CanTpSTmin;
		pduInfo.SduLength = indexCount;
		break;
	case BUFREQ_NOT_OK:
		break;
	case BUFREQ_BUSY: //* qqq req: CanTp 222 */
		sduData[indexCount++] = ISO15765_TPCI_FC
				| ISO15765_FLOW_CONTROL_STATUS_WAIT;
		pduInfo.SduLength = indexCount;
		break;
	case BUFREQ_OVFL: //* qqq req: CanTp 081 */
		sduData[indexCount++] = ISO15765_TPCI_FC
				| ISO15765_FLOW_CONTROL_STATUS_OVFLW;
		pduInfo.SduLength = indexCount;
		break;
	default:
		break;
	}
	ret = canReceivePaddingHelper(rxConfig, rxRuntime, &pduInfo);
	if (ret != E_OK) {
		PduR_CanTpRxIndication(rxConfig->CanTpRxPduId,
				(NotifResultType) NTFRSLT_E_NOT_OK);
		rxRuntime->iso15765.state = IDLE;
		rxRuntime->mode = CANTP_RX_WAIT;
	}
}

// - - - - - - - - - - - - - -


static inline void handleConsecutiveFrame(const CanTp_RxNSduType *rxConfig,
		CanTp_ChannelPrivateType *rxRuntime, const PduInfoType *rxPduData) {
	int indexCount = 0;
	uint8 segmentNumber = 0;
	uint8 extendedAddress = 0;
	PduLengthType bytesRemainingTotalPdu = 0;
	PduLengthType payloadLen = 0;
	PduLengthType maxPayloadDataSdu = 0;

	DEBUG( DEBUG_MEDIUM,"handleConsecutiveFrame called!\n");

	if (rxConfig->CanTpAddressingFormant == CANTP_EXTENDED) {
		extendedAddress = rxPduData->SduDataPtr[indexCount++];
		// qqq: TODO: Should we validate the extended address.
	}

	segmentNumber = rxPduData->SduDataPtr[indexCount++] & SEGMENT_NUMBER_MASK;
	if (segmentNumber != (rxRuntime->iso15765.framesHandledCount
			& SEGMENT_NUMBER_MASK)) {
		DEBUG( DEBUG_MEDIUM,"Segmentation number error detected!\n");
		PduR_CanTpRxIndication(rxConfig->CanTpRxPduId, NTFRSLT_E_WRONG_SN);
		rxRuntime->iso15765.state = IDLE;
		rxRuntime->mode = CANTP_RX_WAIT;
	} else {
		// Calculate the maximal payload for Pdu ( depends on addressing mode ).
		maxPayloadDataSdu = CANIF_PDU_MAX_LENGTH - indexCount;

		// Calculate the number of valid bytes in received frame.
		bytesRemainingTotalPdu = rxRuntime->pduLenghtTotalBytes
				- rxRuntime->pduTransferedBytesCount;

		if (bytesRemainingTotalPdu < maxPayloadDataSdu) {
			payloadLen = bytesRemainingTotalPdu; // 1-5.
		} else {
			payloadLen = maxPayloadDataSdu; // 6 or 7, depends on addressing format used.
		}
		// Copy received data to buffer provided by SDUR.
		if (saveSduPayloadData(rxConfig, rxRuntime,
				&rxPduData->SduDataPtr[indexCount], payloadLen) != E_OK) {
			// qqq: TODO:  handle this error.
			DEBUG( DEBUG_MEDIUM,"saveSduPayloadData failed!\n");
			PduR_CanTpRxIndication(rxConfig->CanTpRxPduId, NTFRSLT_E_NO_BUFFER);
			rxRuntime->iso15765.state = IDLE;
			rxRuntime->mode = CANTP_RX_WAIT;
		} else {
			if ((rxRuntime->pduLenghtTotalBytes
					- rxRuntime->pduTransferedBytesCount) > 0) {
				DEBUG( DEBUG_MEDIUM,"bytesRemainingTotalPdu:%d\n", bytesRemainingTotalPdu);
				rxRuntime->iso15765.framesHandledCount++;
				rxRuntime->iso15765.nextFlowControlCount--;
				if (rxRuntime->iso15765.nextFlowControlCount == 0) {
					sendFlowControlFrame(rxConfig, rxRuntime, BUFREQ_OK);
					rxRuntime->iso15765.framesHandledCount = rxConfig->CanTpBs;
				}
			} else {
				// The transfer succeeded successfully.
				DEBUG( DEBUG_MEDIUM,"RX FINISHED!\n");
				rxRuntime->iso15765.state = IDLE;
				rxRuntime->mode = CANTP_RX_WAIT;
				PduR_CanTpRxIndication(rxConfig->CanTpRxPduId, NTFRSLT_OK);
			}
		}
	}
}

// - - - - - - - - - - - - - -

static inline Std_ReturnType sendConsecutiveFrame(
		const CanTp_TxNSduType *txConfig, CanTp_ChannelPrivateType *txRuntime) {
	Std_ReturnType ret;
	uint8 sduData[CANIF_PDU_MAX_LENGTH];
	PduLengthType maxPayloadDataSdu = 0;
	PduLengthType payloadLen = 0;
	PduLengthType bytesRemainingTotalPdu = 0;
	PduInfoType pduInfo;
	int copyCount = 0;
	int indexCount = 0;

	DEBUG( DEBUG_MEDIUM, "sendConsecutiveFrame entered!\n");

	if (txConfig->CanTpAddressingMode == CANTP_EXTENDED) {
		sduData[indexCount++] = (uint8) txConfig->CanTpNTa->CanTpNTa; // Target address.
	}
	sduData[indexCount++] = ISO15765_TPCI_CF | (uint8)(
			txRuntime->iso15765.framesHandledCount & ISO15765_TPCI_FS_MASK);

	// Always copy from the PDUR buffer data to the canFrameBuffer because if
	// we are unlucky the application give us very small buffers.

	maxPayloadDataSdu = CANIF_PDU_MAX_LENGTH - indexCount;
	bytesRemainingTotalPdu = txRuntime->pduLenghtTotalBytes
			- txRuntime->pduTransferedBytesCount;

	if ( bytesRemainingTotalPdu < maxPayloadDataSdu ) {
		payloadLen = bytesRemainingTotalPdu; // Last frame.
	} else {
		payloadLen = maxPayloadDataSdu;
	}

	copyCount = txRuntime->canFrameBuffer.byteCount; // maybe som bytes already reside in the buffer.
	while (copyCount < payloadLen) {
		if ( txRuntime->bufferPduRouter->SduLength > txRuntime->pdurBufferCount ) {
			txRuntime->canFrameBuffer.data[copyCount] =
					txRuntime->bufferPduRouter->SduDataPtr[txRuntime->pdurBufferCount++];
			copyCount++;
			txRuntime->canFrameBuffer.byteCount++;
		} else {
			// More buffering is required.
			BufReq_ReturnType pdurResp;
			pdurResp = PduR_CanTpProvideTxBuffer(txConfig->CanTpTxPduId,
					&txRuntime->bufferPduRouter, 0);
			if (pdurResp == BUFREQ_OK) {
				txRuntime->pdurBufferCount = 0;
				continue;
			} else if (pdurResp == BUFREQ_BUSY) {
				ret = E_OK; // We will remain in this state, called again later, not data lost/destoryed?
			} else {
				ret = E_NOT_OK; // Serious malfunction, function caller should cancel this transfer.
			}
		}
	}
	if (copyCount == payloadLen) {
		for (int i=0; i<txRuntime->canFrameBuffer.byteCount; i++) {
			sduData[indexCount++] = txRuntime->canFrameBuffer.data[i];
		}
		pduInfo.SduDataPtr = sduData;
		pduInfo.SduLength = indexCount; // also includes consecutive frame header.
		ret = canTansmitPaddingHelper(txConfig, txRuntime, &pduInfo);
		if (ret == E_OK) {
			// Now we consider this frame sent and we can not handle
			// the scenario where the CAN queue is full.
			txRuntime->iso15765.framesHandledCount++;
			txRuntime->iso15765.nextFlowControlCount--;  // qqq: use decremt macro.
			txRuntime->pduTransferedBytesCount += txRuntime->canFrameBuffer.byteCount;
			txRuntime->canFrameBuffer.byteCount = 0;
			DEBUG( DEBUG_MEDIUM, "pduTransferedBytesCount:%d\n", txRuntime->pduTransferedBytesCount);

		}
	}
	DEBUG( DEBUG_MEDIUM, "sendConsecutiveFrame exit!\n");

	return ret;
}

// - - - - - - - - - - - - - -

static inline void handleConsecutiveFrameSent(
		const CanTp_TxNSduType *txConfig, CanTp_ChannelPrivateType *txRuntime) {

	if (txRuntime->pduLenghtTotalBytes <= txRuntime->pduTransferedBytesCount) {
		// Transfer finished.
		DEBUG( DEBUG_MEDIUM, "TRANSFER FINISHED!\n");
		txRuntime->iso15765.state = IDLE;
		txRuntime->mode = CANTP_TX_WAIT;
		PduR_CanTpTxConfirmation(txConfig->CanTpTxPduId,
				(NotifResultType) NTFRSLT_OK);
	} else if (txRuntime->iso15765.nextFlowControlCount == 0) {
		// It is time to receive a flow control.
		DEBUG( DEBUG_MEDIUM, "Sender require flow control!\n");
		txRuntime->iso15765.stateTimeoutCount = txConfig->CanTpNbs
				* MAIN_FUNCTION_PERIOD_TIME_MS; /*CanTp: 264*/
		txRuntime->iso15765.state = TX_WAIT_FLOW_CONTROL;
	} else if (txRuntime->iso15765.nextFlowControlCount != 0) {
		// Send next consecutive frame.
		DEBUG( DEBUG_MEDIUM, "nextFlowControlCount:%d \n",
				txRuntime->iso15765.nextFlowControlCount);
		txRuntime->iso15765.stateTimeoutCount
				= txRuntime->iso15765.STmin
						* MAIN_FUNCTION_PERIOD_TIME_MS;
		txRuntime->iso15765.state = TX_WAIT_SEND_CONSECUTIVE_FRAME;
	} else {
		DEBUG( DEBUG_MEDIUM, "Sender failed!\n" );
		PduR_CanTpTxConfirmation(txConfig->CanTpTxPduId,
				(NotifResultType) NTFRSLT_E_NOT_OK);
		txRuntime->iso15765.state = IDLE;
		txRuntime->mode = CANTP_TX_WAIT;
	}
}

// - - - - - - - - - - - - - -

static inline void handleFlowControlFrame(const CanTp_TxNSduType *txConfig,
		CanTp_ChannelPrivateType *txRuntime, const PduInfoType *txPduData) {
	int indexCount = 0;
	uint8 extendedAddress = 0;
	Std_ReturnType ret;

	DEBUG( DEBUG_MEDIUM, "handleFlowControlFrame called!\n");

	if (txConfig->CanTpAddressingMode == CANTP_EXTENDED) {
		extendedAddress = txPduData->SduDataPtr[indexCount++];
		// qqq: TODO: Should we validate the extended address ?.
	}

	switch (txPduData->SduDataPtr[indexCount++] & ISO15765_TPCI_FS_MASK) {
	case ISO15765_FLOW_CONTROL_STATUS_CTS:
		/* qqq: TODO: Sometimes this value can be forced by config, but not in AutoSAR CANTP ? */
		txRuntime->iso15765.BS = txPduData->SduDataPtr[indexCount++];
		txRuntime->iso15765.nextFlowControlCount = txRuntime->iso15765.BS; // qqq do we need to store the .BS ?
		txRuntime->iso15765.STmin = txPduData->SduDataPtr[indexCount++];
		ret = sendConsecutiveFrame(txConfig, txRuntime);
		if (ret == E_OK) {
			// qqq: TODO: Move to TX-acknowledge callback.
			handleConsecutiveFrameSent(txConfig, txRuntime);
		} else {
			/* qqq: TODO: We have a CAN error, put in log-file? */
			PduR_CanTpRxIndication(txConfig->CanTpTxPduId,
					(NotifResultType) NTFRSLT_E_NOT_OK);
			txRuntime->iso15765.state = IDLE;
			txRuntime->mode = CANTP_TX_WAIT;
		}
		break;
	case ISO15765_FLOW_CONTROL_STATUS_WAIT:
		txRuntime->iso15765.stateTimeoutCount = txConfig->CanTpNbs
				* MAIN_FUNCTION_PERIOD_TIME_MS; /*CanTp: 264*/
		txRuntime->iso15765.state = TX_WAIT_FLOW_CONTROL;
		break;
	case ISO15765_FLOW_CONTROL_STATUS_OVFLW:
		/* qqq: TODO: We have a CAN error, put in log-file? */
		PduR_CanTpRxIndication(txConfig->CanTpTxPduId,
				(NotifResultType) NTFRSLT_E_NOT_OK);
		txRuntime->iso15765.state = IDLE;
		txRuntime->mode = CANTP_TX_WAIT;
		break;
	}
}

// - - - - - - - - - - - - - -

static inline void handleSingleFrame(const CanTp_RxNSduType *rxConfig,
		CanTp_ChannelPrivateType *rxRuntime, const PduInfoType *rxPduData) {
	BufReq_ReturnType ret;
	PduLengthType pduLength;
	uint8 *data = NULL;

	DEBUG( DEBUG_MEDIUM, "handleSingleFrame called!\n");


	if (rxRuntime->iso15765.state != IDLE) {
		// qqq: TODO: Log this maybe?
	}

	(void) initRx15765RuntimeData(rxConfig, rxRuntime);
	pduLength = getPduLength(&rxConfig->CanTpAddressingFormant, SINGLE_FRAME,
			rxPduData);

	VALIDATE( rxRuntime->bufferPduRouter->SduDataPtr != NULL,
			SERVICE_ID_CANTP_RX_INDICATION, CANTP_E_INVALID_RX_LENGTH );

	if (rxConfig->CanTpAddressingFormant == CANTP_STANDARD) {
		data = &rxPduData->SduDataPtr[1];
	} else {
		data = &rxPduData->SduDataPtr[2];
	}
	rxRuntime->pduLenghtTotalBytes = pduLength;
	rxRuntime->iso15765.state = SF_OR_FF_RECEIVED_WAITING_PDUR_BUFFER;
	rxRuntime->mode = CANTP_RX_PROCESSING;
	rxRuntime->iso15765.stateTimeoutCount = rxConfig->CanTpNbr
			* MAIN_FUNCTION_PERIOD_TIME_MS; /* CanTP 166. */

	ret = saveSduPayloadData(rxConfig, rxRuntime, data, pduLength);
	if (ret == BUFREQ_OK && rxRuntime->iso15765.stateTimeoutCount != 0) {
		PduR_CanTpRxIndication(rxConfig->CanTpRxPduId, NTFRSLT_OK);
	} else {
		PduR_CanTpRxIndication(rxConfig->CanTpRxPduId, NTFRSLT_E_NO_BUFFER);
	}
	rxRuntime->iso15765.state = IDLE;
	rxRuntime->mode = CANTP_RX_WAIT;
}

// - - - - - - - - - - - - - -


static inline void handleFirstFrame(const CanTp_RxNSduType *rxConfig,
		CanTp_ChannelPrivateType *rxRuntime, const PduInfoType *rxPduData) {
	BufReq_ReturnType ret;
	PduLengthType pduLength;
	int i;

	if (rxRuntime->iso15765.state != IDLE) {
		// qqq: TODO: Log this maybe?
	}

	DEBUG( DEBUG_MEDIUM, "handleFirstFrame called!\n");

	(void) initRx15765RuntimeData(rxConfig, rxRuntime);
	pduLength = getPduLength(&rxConfig->CanTpAddressingFormant, FIRST_FRAME,
			rxPduData);
	rxRuntime->pduLenghtTotalBytes = pduLength;

	VALIDATE( rxRuntime->pduLenghtTotalBytes != 0,
			SERVICE_ID_CANTP_RX_INDICATION, CANTP_E_INVALID_RX_LENGTH );

	// Validate that that there is a reason for using the segmented transfers and
	// if not simply skip (single frame should have been used).
	if (rxConfig->CanTpAddressingFormant == CANTP_STANDARD) {
		if (pduLength <= MAX_PAYLOAD_SF_STD_ADDR)
			return;
	} else {
		if (pduLength <= MAX_PAYLOAD_SF_EXT_ADDR)
			return;
	}
	// Validate that the SDU is full length in this first frame.
	if (rxPduData->SduLength < CANIF_PDU_MAX_LENGTH) {
		return;
	}

	rxRuntime->iso15765.state = SF_OR_FF_RECEIVED_WAITING_PDUR_BUFFER;
	rxRuntime->mode = CANTP_RX_PROCESSING;
	rxRuntime->iso15765.stateTimeoutCount = rxConfig->CanTpNbr
			* MAIN_FUNCTION_PERIOD_TIME_MS; /* CanTP 166. */

	if (rxConfig->CanTpAddressingFormant == CANTP_STANDARD) {
		ret = saveSduPayloadData(rxConfig, rxRuntime,
				&rxPduData->SduDataPtr[2],
				MAX_PAYLOAD_FF_STD_ADDR);
	} else {
		ret = saveSduPayloadData(rxConfig, rxRuntime,
				&rxPduData->SduDataPtr[3],
				MAX_PAYLOAD_FF_EXT_ADDR);
	}
	if (ret == BUFREQ_OK) {
		rxRuntime->iso15765.stateTimeoutCount = rxConfig->CanTpNcr
				* MAIN_FUNCTION_PERIOD_TIME_MS;
		rxRuntime->iso15765.state = RX_WAIT_CONSECUTIVE_FRAME;
		rxRuntime->mode = CANTP_RX_PROCESSING;
		sendFlowControlFrame(rxConfig, rxRuntime, ret);
	} else if (ret == BUFREQ_BUSY) {
		/* req: CanTp: 222. */
		for (i = 0; i < rxPduData->SduLength; i++) {
			rxRuntime->canFrameBuffer.data[i] = rxPduData->SduDataPtr[i];
		}
		rxRuntime->canFrameBuffer.byteCount = rxPduData->SduLength;
		rxRuntime->iso15765.stateTimeoutCount = rxConfig->CanTpNbr
				* MAIN_FUNCTION_PERIOD_TIME_MS;
		rxRuntime->iso15765.state = RX_WAIT_SDU_BUFFER;
		rxRuntime->mode = CANTP_RX_PROCESSING;
		sendFlowControlFrame(rxConfig, rxRuntime, ret);
	} else if (ret == BUFREQ_OVFL) {
		sendFlowControlFrame(rxConfig, rxRuntime, ret);
		rxRuntime->iso15765.state = IDLE;
		rxRuntime->mode = CANTP_RX_WAIT;
	}
}

// - - - - - - - - - - - - - -

/*
 * This function validates if the data to be sent needs to be segmented or
 * could fit into a single frame.
 */
static inline Std_ReturnType calcRequiredProtocolFrameType(
		const CanTp_TxNSduType *txConfig, CanTp_ChannelPrivateType *txRuntime,
		ISO15765FrameType *iso15765Frame) {

	Std_ReturnType ret;
	if (txConfig->CanTpAddressingMode == CANTP_EXTENDED) {
		if ( txRuntime->pduLenghtTotalBytes > MAX_PAYLOAD_CF_EXT_ADDR ) {
			VALIDATE( txConfig->CanTpTxTaType == CANTP_FUNCTIONAL,
					SERVICE_ID_CANTP_TRANSMIT, CANTP_E_INVALID_TATYPE );
		}
		if (txRuntime->pduLenghtTotalBytes > MAX_PAYLOAD_CF_EXT_ADDR) {
			if (txConfig->CanTpTxTaType == CANTP_PHYSICAL) {
				*iso15765Frame = FIRST_FRAME;
				ret = E_OK;
			} else {
				*iso15765Frame = NONE;
				ret = E_NOT_OK;
			}
		} else {
			*iso15765Frame = SINGLE_FRAME;
			ret = E_OK;
		}
	} else {
		if ( txRuntime->pduLenghtTotalBytes > MAX_PAYLOAD_CF_EXT_ADDR ) {
			VALIDATE( txConfig->CanTpTxTaType == CANTP_FUNCTIONAL,
					SERVICE_ID_CANTP_TRANSMIT, CANTP_E_INVALID_TATYPE );
		}
		if (txRuntime->pduLenghtTotalBytes > MAX_PAYLOAD_CF_STD_ADDR) {
			if (txConfig->CanTpTxTaType == CANTP_PHYSICAL) {
				*iso15765Frame = FIRST_FRAME;
				ret = E_OK;
			} else {
				*iso15765Frame = NONE;
				ret = E_NOT_OK;
			}
		} else {
			*iso15765Frame = SINGLE_FRAME;
			ret = E_OK;
		}
	}
	return ret;
}

// - - - - - - - - - - - - - -


static inline Std_ReturnType sendSingleFrame(const CanTp_TxNSduType *txConfig,
		CanTp_ChannelPrivateType *txRuntime) {
	Std_ReturnType ret;
	int indexCount = 0;
	PduInfoType pduInfo;
	uint8 sduData[CANIF_PDU_MAX_LENGTH]; // qqq: Note that buffer in declared on the stack.

	DEBUG( DEBUG_MEDIUM, "sendSingleFrame called!\n");


	if (txConfig->CanTpAddressingMode == CANTP_EXTENDED) {
		sduData[indexCount++] = (uint8) txConfig->CanTpNTa->CanTpNTa; // Target address.
	}
	sduData[indexCount++] = ISO15765_TPCI_SF
			| txRuntime->bufferPduRouter->SduLength;
	for (int i = 0; i < txRuntime->pduLenghtTotalBytes; i++) {
		sduData[indexCount++] = txRuntime->bufferPduRouter->SduDataPtr[i];
	}

	pduInfo.SduDataPtr = sduData;
	pduInfo.SduLength = indexCount;
	ret = canTansmitPaddingHelper(txConfig, txRuntime, &pduInfo);
	return ret;
}

// - - - - - - - - - - - - - -

static inline Std_ReturnType sendFirstFrame(const CanTp_TxNSduType *txConfig,
		CanTp_ChannelPrivateType *txRuntime) {
	Std_ReturnType ret;
	int indexCount = 0;
	int i = 0;
	PduInfoType pduInfo;
	uint8 sduData[CANIF_PDU_MAX_LENGTH];

	DEBUG( DEBUG_MEDIUM, "sendFirstFrame called!\n");

	if (txConfig->CanTpAddressingMode == CANTP_EXTENDED) {
		sduData[indexCount++] = (uint8) txConfig->CanTpNTa->CanTpNTa; // Target address.
	}
	sduData[indexCount++] = ISO15765_TPCI_FF | (uint8)(
			(txRuntime->pduLenghtTotalBytes & 0xf00) >> 8);
	sduData[indexCount++] = (uint8)(txRuntime->pduLenghtTotalBytes & 0xff);
	for (i = 0; indexCount < CANIF_PDU_MAX_LENGTH; i++) {
		sduData[indexCount++] = txRuntime->bufferPduRouter->SduDataPtr[i];
		txRuntime->pdurBufferCount++;
		if (txRuntime->pdurBufferCount > txRuntime->bufferPduRouter->SduLength) {
			// qqq: TODO: Report failure - this is unexpected.
		}
		txRuntime->pduTransferedBytesCount++;
	}
	pduInfo.SduDataPtr = sduData;
	pduInfo.SduLength = indexCount;
	ret = canTansmitPaddingHelper(txConfig, txRuntime, &pduInfo);
	return ret;
}

// - - - - - - - - - - - - - -

BufReq_ReturnType canTpTransmitHelper(const CanTp_TxNSduType *txConfig,
		CanTp_ChannelPrivateType *txRuntime) {

	BufReq_ReturnType pdurResp;
	Std_ReturnType res;
	ISO15765FrameType iso15765Frame;

	pdurResp = PduR_CanTpProvideTxBuffer(txConfig->CanTpTxPduId,
			&txRuntime->bufferPduRouter, 0);  // Req: CanTp 186.
	if (txRuntime->iso15765.stateTimeoutCount != 0) {
		VALIDATE( txRuntime->bufferPduRouter->SduDataPtr != NULL,
				SERVICE_ID_CANTP_TRANSMIT, CANTP_E_INVALID_TX_BUFFER );
		if (pdurResp == BUFREQ_OK) {
			res = calcRequiredProtocolFrameType(txConfig, txRuntime, &iso15765Frame);
			switch (iso15765Frame) {
			case SINGLE_FRAME:
				res = sendSingleFrame(txConfig, txRuntime); /* req: CanTp 231  */
				if (res == E_OK) {
					PduR_CanTpTxConfirmation(txConfig->CanTpTxPduId, NTFRSLT_OK);
				} else {
					PduR_CanTpTxConfirmation(txConfig->CanTpTxPduId,
							NTFRSLT_NOT_OK);
				}
				txRuntime->iso15765.state = IDLE;
				txRuntime->mode = CANTP_TX_WAIT;
				break;
			case FIRST_FRAME:
				txRuntime->iso15765.state = TX_WAIT_FLOW_CONTROL;
				res = sendFirstFrame(txConfig, txRuntime); /* req: CanTp 232 */
				if (res == E_OK) {
					txRuntime->mode = CANTP_TX_PROCESSING;
				}
				break;
			case INVALID_FRAME:
			default:
				PduR_CanTpTxConfirmation(txConfig->CanTpTxPduId, NTFRSLT_NOT_OK);
				txRuntime->iso15765.state = IDLE;
				txRuntime->mode = CANTP_TX_WAIT;
				break;
			}
		} else if (pdurResp == BUFREQ_NOT_OK) {
			PduR_CanTpTxConfirmation(txConfig->CanTpTxPduId, NTFRSLT_NOT_OK);
			txRuntime->iso15765.state = IDLE;
			txRuntime->mode = CANTP_TX_WAIT;
		} else if (pdurResp == BUFREQ_BUSY) {
			txRuntime->iso15765.state = TX_WAIT_CAN_TP_TRANSMIT_PENDING; // We have to issue this request later from main until timeout.
			txRuntime->mode = CANTP_TX_PROCESSING;
		}
	} else {
		// qqq: TDDO: Put this in a logfile? Error response should have been
		// sent from main.
	}
	return pdurResp;
}

// - - - - - - - - - - - - - -


Std_ReturnType CanTp_Transmit(PduIdType CanTpTxSduId,
		const PduInfoType *CanTpTxInfoPtr) /** req : CanTp225. **/
{
	const CanTp_TxNSduType *txConfig;
	CanTp_ChannelPrivateType *txRuntime;
	BufReq_ReturnType res;
	Std_ReturnType ret;
	int index;

	DEBUG( DEBUG_MEDIUM, "CanTp_Transmit called!\n");

	VALIDATE( CanTpTxInfoPtr != NULL,
			SERVICE_ID_CANTP_TRANSMIT, CANTP_E_PARAM_ADDRESS ); /* req: CanTp031 */
	VALIDATE( CanTpRunTimeData.internalState == CANTP_ON,
			SERVICE_ID_CANTP_TRANSMIT, CANTP_E_UNINIT ); /* req: CanTp031 */

	index = getCanTpTxNSduConfigListIndex(&CanTpConfig, CanTpTxSduId);
	//VALIDATE( index != CANTP_ERR, SERVICE_ID_CANTP_TRANSMIT, CANTP_E_PARAM_ID );
	VALIDATE( index != CANTP_ERR, SERVICE_ID_CANTP_TRANSMIT, CANTP_E_INVALID_TX_ID );
	if (index != CANTP_ERR) {
		//VALIDATE( index != CANTP_ERR, SERVICE_ID_CANTP_TRANSMIT, CANTP_E_INVALID_TX_LENGHT );
		txConfig = &CanTpTxNSduConfigList[index];
		txRuntime = &CanTpRunTimeData.txNSduData[index];
		txRuntime->pduLenghtTotalBytes = CanTpTxInfoPtr->SduLength;
		txRuntime->iso15765.stateTimeoutCount = txConfig->CanTpNcs
				* MAIN_FUNCTION_PERIOD_TIME_MS;
		txRuntime->mode = CANTP_TX_PROCESSING;
		txRuntime->iso15765.state
				= TX_WAIT_CAN_TP_TRANSMIT_CAN_TP_PROVIDE_TX_BUFFER;
		res = canTpTransmitHelper(txConfig, txRuntime);
		switch (res) {
		case BUFREQ_OK:
		case BUFREQ_BUSY:
			ret = E_OK;
			break;
		case BUFREQ_NOT_OK:
			ret = E_NOT_OK;
			break;
		default:
			ret = E_NOT_OK;
			break;
		}
	} else {

		ret = E_NOT_OK;
	}
	return ret; // CAN level error code.
}

// - - - - - - - - - - - - - -

#if FRTP_CANCEL_TRANSMIT_REQUEST
Std_ReturnType FrTp_CancelTransmitRequest(PduIdType FrTpTxPduId,
		FrTp_CancelReasonType FrTpCancelReason) /** req : CanTp246 **/
{
	;
}
#endif

// - - - - - - - - - - - - - -


void CanTp_Init() /** req : CanTp208. **/
{
	const CanTp_RxNSduType *rxConfigListItem = CanTpRxNSduConfigList;
	const CanTp_TxNSduType *txConfigListItem = CanTpTxNSduConfigList;

	CanTp_ChannelPrivateType *rxRuntimeListItem = CanTpRunTimeData.rxNSduData;
	CanTp_ChannelPrivateType *txRuntimeListItem = CanTpRunTimeData.txNSduData;

	for(int i=0; i < CANTP_TX_NSDU_CONFIG_LIST_CNT; i++) {
		initTx15765RuntimeData(txConfigListItem, txRuntimeListItem);
		if (txConfigListItem->CanTpListItemType != CANTP_END_OF_LIST) {
			txConfigListItem++;
			txRuntimeListItem++;
		} else {
			break;
		}
	}
	for(int i=0; i < CANTP_RX_NSDU_CONFIG_LIST_CNT; i++) {
		initRx15765RuntimeData(rxConfigListItem, rxRuntimeListItem);
		if (rxConfigListItem->CanTpListItemType != CANTP_END_OF_LIST) {
			rxConfigListItem++;
			rxRuntimeListItem++;
		} else {
			break;
		}
	}
	fifoQueueInit( &CanTpRunTimeData.fifo );
	CanTpRunTimeData.internalState = CANTP_ON;
}

// - - - - - - - - - - - - - -

void CanTp_RxIndication(PduIdType CanTpRxPduId,
		const PduInfoType *CanTpRxPduPtr) /** req : CanTp214. **/
{
	CanTpFifoQueueItem item;
	VALIDATE( CanTpRunTimeData.internalState == CANTP_ON,
			SERVICE_ID_CANTP_RX_INDICATION, CANTP_E_UNINIT ); /* req: CanTp031 */

	item.PduId = CanTpRxPduId;
	item.SduLength = CanTpRxPduPtr->SduLength;
	for (int i=0; i<item.SduLength; i++) {
		item.SduData[i] = CanTpRxPduPtr->SduDataPtr[i];
	}
	if ( fifoQueueWrite( &CanTpRunTimeData.fifo, &item ) == FALSE ) {
		; //qqq: TODO: Report overrun.
	}
}

// - - - - - - - - - - - - - -

void CanTp_RxIndication_Main(PduIdType CanTpRxPduId,
		const PduInfoType *CanTpRxPduPtr) /** req : CanTp214. **/
{
	const CanTp_RxNSduType *rxConfigParams; // Params reside in ROM.
	const CanTp_TxNSduType *txConfigParams;
	const CanTp_AddressingFormantType *addressingFormat; // Configured
	CanTp_ChannelPrivateType *runtimeParams; // Params reside in RAM.
	ISO15765FrameType frameType;
	int configListIndex;

	DEBUG( DEBUG_MEDIUM, "CanTp_RxIndication_Main entered!\n");

	VALIDATE( CanTpRunTimeData.internalState == CANTP_ON,
			SERVICE_ID_CANTP_RX_INDICATION, CANTP_E_UNINIT ); /* req: CanTp031 */

	// Find transfer instance, try Rx and then Tx.
	configListIndex = getCanTpRxNSduConfigListIndex(&CanTpConfig, CanTpRxPduId);
	if (configListIndex == CANTP_ERR) {
		configListIndex = getCanTpTxNSduConfigListIndex(&CanTpConfig,
				CanTpRxPduId);
		if (configListIndex == CANTP_ERR)
			return;
		txConfigParams = &CanTpConfig.CanTpTxNSduList[configListIndex];
		runtimeParams = &CanTpRunTimeData.txNSduData[configListIndex];
		addressingFormat = &txConfigParams->CanTpAddressingMode;
		rxConfigParams = NULL;
	} else {
		rxConfigParams = &CanTpConfig.CanTpRxNSduList[configListIndex];
		runtimeParams = &CanTpRunTimeData.rxNSduData[configListIndex];
		addressingFormat = &rxConfigParams->CanTpAddressingFormant;
		txConfigParams = NULL;
	}

	frameType = getFrameType(addressingFormat, CanTpRxPduPtr);
	switch (frameType) {
	case SINGLE_FRAME: {
		if (rxConfigParams != NULL)
			handleSingleFrame(rxConfigParams, runtimeParams, CanTpRxPduPtr);
		break;
	}
	case FIRST_FRAME: {
		if (rxConfigParams != NULL)
			handleFirstFrame(rxConfigParams, runtimeParams, CanTpRxPduPtr);
		break;
	}
	case CONSECUTIVE_FRAME: {
		if (rxConfigParams != NULL)
			handleConsecutiveFrame(rxConfigParams, runtimeParams, CanTpRxPduPtr);
		break;
	}
	case FLOW_CONTROL_CTS_FRAME: {
		if (txConfigParams != NULL)
			handleFlowControlFrame(txConfigParams, runtimeParams, CanTpRxPduPtr);
		break;
	}
	case INVALID_FRAME: {
		return; // qqq: Failure, should go to log-file.
		break;
	}
	default:
		break;
	}

	DEBUG( DEBUG_MEDIUM, "CanTp_RxIndication_Main exit!\n");

}

// - - - - - - - - - - - - - -

void CanTp_GetVersionInfo(Std_VersionInfoType* versioninfo) /** req : CanTp210 **/
{
	;
}

// - - - - - - - - - - - - - -

void CanTp_TxConfirmation(PduIdType PduId) /** req: CanTp215 **/
{
	VALIDATE( CanTpRunTimeData.internalState != CANTP_ON,
			SERVICE_ID_CANTP_TX_CONFIRMATION, CANTP_E_UNINIT ); /* req: CanTp031 */

	int configListIndex = 0;
	configListIndex = getCanTpRxNSduConfigListIndex(&CanTpConfig, PduId);
	if (configListIndex == CANTP_ERR) {
		configListIndex = getCanTpTxNSduConfigListIndex(&CanTpConfig, PduId);
		if (configListIndex != CANTP_ERR) {
			CanTpRunTimeData.txNSduData[configListIndex].iso15765.NasNarPending = FALSE;
		}
	} else {
		CanTpRunTimeData.rxNSduData[configListIndex].iso15765.NasNarPending = FALSE;
	}

}

// - - - - - - - - - - - - - -

void CanTp_Shutdown() /** req : CanTp211 **/
{
	VALIDATE( CanTpRunTimeData.internalState == CANTP_ON,
			SERVICE_ID_CANTP_SHUTDOWN, CANTP_E_UNINIT ); /* req: CanTp031 */

	CanTpRunTimeData.internalState = CANTP_OFF;
}

// - - - - - - - - - - - - - -


static inline boolean checkNasNarTimeout(CanTp_ChannelPrivateType *runtimeData) {
	boolean ret = FALSE;
	if (runtimeData->iso15765.NasNarPending) {
		TIMER_DECREMENT(runtimeData->iso15765.NasNarTimeoutCount);
		if (runtimeData->iso15765.NasNarTimeoutCount == 0) {
			runtimeData->iso15765.state = IDLE;
			runtimeData->iso15765.NasNarPending = FALSE;
			ret = TRUE;
		}
	}
	return ret;
}

// - - - - - - - - - - - - - -


void CanTp_MainFunction() /** req : CanTp213 **/
{
	BufReq_ReturnType ret;
	CanTpFifoQueueItem item;

	const CanTp_RxNSduType *rxConfigListItem = CanTpRxNSduConfigList;
	const CanTp_TxNSduType *txConfigListItem = CanTpTxNSduConfigList;

	CanTp_ChannelPrivateType *rxRuntimeListItem = CanTpRunTimeData.rxNSduData;
	CanTp_ChannelPrivateType *txRuntimeListItem = CanTpRunTimeData.txNSduData;

	DEBUG( DEBUG_MEDIUM, "CanTp_MainFunction called.\n" );

	VALIDATE( CanTpRunTimeData.internalState == CANTP_ON,
			SERVICE_ID_CANTP_MAIN_FUNCTION, CANTP_E_UNINIT ); /* req: CanTp031 */

	// Dispatch the messages that resides in the FIFO to CanTp_RxIndication_Main.

/*
	while ( fifoQueueRead( &CanTpRunTimeData.fifo, &item ) == TRUE  ) {
		PduInfoType *pduInfo;
		pduInfo->SduDataPtr = item.SduData;
		pduInfo->SduLength = item.SduLength;
		CanTp_RxIndication_Main( item.PduId, pduInfo );
	}
*/
	PduInfoType pduInfo;
	if ( fifoQueueRead( &CanTpRunTimeData.fifo, &item ) == TRUE  ) {
		pduInfo.SduDataPtr = item.SduData;
		pduInfo.SduLength = item.SduLength;
		CanTp_RxIndication_Main( item.PduId, &pduInfo );
	}

	for(int i=0; i < CANTP_TX_NSDU_CONFIG_LIST_CNT; i++) {
		if ( checkNasNarTimeout( txRuntimeListItem ) ) { // CanTp075.
			PduR_CanTpTxConfirmation(txConfigListItem->CanTpTxPduId,
					NTFRSLT_NOT_OK); /* qqq: req: CanTp: 185. */
			continue;
		}
		switch (txRuntimeListItem->iso15765.state) {
		case TX_WAIT_CAN_TP_TRANSMIT_CAN_TP_PROVIDE_TX_BUFFER:
			TIMER_DECREMENT (rxRuntimeListItem->iso15765.stateTimeoutCount);
			if (txRuntimeListItem->iso15765.stateTimeoutCount == 0)
				PduR_CanTpTxConfirmation(txConfigListItem->CanTpTxPduId,
						NTFRSLT_NOT_OK); /* qqq: req: CanTp: 185. */
			txRuntimeListItem->iso15765.state = IDLE;
			txRuntimeListItem->mode = CANTP_TX_WAIT;
			break;
		case TX_WAIT_CAN_TP_TRANSMIT_PENDING:
			(void) canTpTransmitHelper(txConfigListItem, txRuntimeListItem);
			break;
		case TX_WAIT_SEND_CONSECUTIVE_FRAME:
			ret = sendConsecutiveFrame(txConfigListItem, txRuntimeListItem);
			/* qqq: TODO: We have a CAN error, put in log-file? */
			if ( ret == E_OK ) {
				handleConsecutiveFrameSent(txConfigListItem, txRuntimeListItem);
			} else {
				PduR_CanTpRxIndication(txConfigListItem->CanTpTxPduId,
						(NotifResultType) NTFRSLT_E_NOT_OK);
				txRuntimeListItem->iso15765.state = IDLE;
				txRuntimeListItem->mode = CANTP_TX_WAIT;
			}
			break;
		case TX_WAIT_FLOW_CONTROL:
			if (txRuntimeListItem->iso15765.stateTimeoutCount == 0)
				PduR_CanTpTxConfirmation(txConfigListItem->CanTpTxPduId,
						NTFRSLT_NOT_OK); /* qqq: req: CanTp: 185. */
			break;
		default:
			break;
		}
		// qqq: TODO:
		if (txConfigListItem->CanTpListItemType != CANTP_END_OF_LIST) {
			txConfigListItem++;
			txRuntimeListItem++;
		} else {
			break;
		}
	}

	for(int i=0; i < CANTP_RX_NSDU_CONFIG_LIST_CNT; i++) {
		if ( checkNasNarTimeout( rxRuntimeListItem ) ) {  // CanTp075.
			PduR_CanTpTxConfirmation(rxConfigListItem->CanTpRxPduId,
					NTFRSLT_NOT_OK); /* qqq: req: CanTp: 185. */
			continue;
		}
		switch (rxRuntimeListItem->iso15765.state) {
		case RX_WAIT_CONSECUTIVE_FRAME: {
			TIMER_DECREMENT (rxRuntimeListItem->iso15765.stateTimeoutCount);
			if (rxRuntimeListItem->iso15765.stateTimeoutCount == 0) {
				rxRuntimeListItem->iso15765.state = IDLE;
				rxRuntimeListItem->mode = CANTP_RX_WAIT;
			}
			break;
		}
		case RX_WAIT_SDU_BUFFER: {
			if (rxRuntimeListItem->iso15765.stateTimeoutCount == 0) {
				rxRuntimeListItem->iso15765.state = IDLE;
				rxRuntimeListItem->mode = CANTP_RX_WAIT;
			} else {
				if (rxConfigListItem->CanTpAddressingFormant == CANTP_STANDARD) {
					ret = saveSduPayloadData(rxConfigListItem,
							rxRuntimeListItem,
							&rxRuntimeListItem->canFrameBuffer.data[2],
							MAX_PAYLOAD_FF_STD_ADDR);
				} else {
					ret = saveSduPayloadData(rxConfigListItem,
							rxRuntimeListItem,
							&rxRuntimeListItem->canFrameBuffer.data[3],
							MAX_PAYLOAD_FF_EXT_ADDR);
				}
			}
			break;
		}
		default:
			break;
		}

		if (rxConfigListItem->CanTpListItemType != CANTP_END_OF_LIST) {
			rxConfigListItem++;
			rxRuntimeListItem++;
		} else {
			break;
		}
	}
}

