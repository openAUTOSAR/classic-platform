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

#include "Det.h"
#include "CanIf.h"
#include "CanTp_Cfg.h" /** req: CanTp156.1 **/
#include "CanTp_Cbk.h" /** req: CanTp156.2 **/
#include "CanTp.h" /** req: CanTp156.3 **/
#include "SchM_CanTp.h" /** req: CanTp156.4 **/
#include "PduR.h"
//#include "MemMap.h" /** req: CanTp156.5 **/
#include "String.h"
#define USE_DEBUG_PRINTF
#include "debug.h"

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
#define INLINE inline

#define TIMER_DECREMENT(timer) \
	if (timer > 0) { \
		timer = timer - 1; \
	} \

#define COUNT_DECREMENT(timer) \
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

#define MAX_SEGMENT_DATA_SIZE		8 	// Size of a CAN frame data bytes.

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

// - - - - - - - - - - - - - -

typedef struct {
	boolean initRun;
	CanTpFifo fifo;
	CanTp_StateType internalState; /** req: CanTp 027 */
	CanTp_ChannelPrivateType runtimeDataList[CANTP_NSDU_RUNTIME_LIST_SIZE];
} CanTp_RunTimeDataType;

// - - - - - - - - - - - - - -

#if (CANTP_VERSION_INFO_API == STD_ON)
static Std_VersionInfoType _CanTp_VersionInfo =
{
  .vendorID   = (uint16)1,
  .moduleID   = (uint16)1,
  .instanceID = (uint8)1,
  .sw_major_version = (uint8)CANTP_SW_MAJOR_VERSION,
  .sw_minor_version = (uint8)CANTP_SW_MINOR_VERSION,
  .sw_patch_version = (uint8)CANTP_SW_PATCH_VERSION,
  .ar_major_version = (uint8)CANTP_AR_MAJOR_VERSION,
  .ar_minor_version = (uint8)CANTP_AR_MINOR_VERSION,
  .ar_patch_version = (uint8)CANTP_AR_PATCH_VERSION,
};
#endif /* DEM_VERSION_INFO_API */

// - - - - - - - - - - - - - -

CanTp_RunTimeDataType CanTpRunTimeData = { .initRun = FALSE,
		.internalState = CANTP_OFF };

static INLINE void fifoQueueInit( CanTpFifo *fifoQueue ) {
	fifoQueue->fifoQueueReadIndex = 0;
	fifoQueue->fifoQueueWriteIndex = 0;
}

// - - - - - - - - - - - - - -

static INLINE boolean fifoQueueRead( CanTpFifo *fifoQueue, CanTpFifoQueueItem *item ) {
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

// - - - - - - - - - - - - - -

static INLINE boolean fifoQueueWrite( CanTpFifo *fifoQueue, CanTpFifoQueueItem *item ) {
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

// - - - - - - - - - - - - - -

static inline ISO15765FrameType getFrameType(
		const CanTp_AddressingFormantType *formatType,
		const PduInfoType *CanTpRxPduPtr) {
	ISO15765FrameType res = INVALID_FRAME;
	uint8 tpci = 0;

	switch (*formatType) {
	case CANTP_STANDARD:
		DEBUG( DEBUG_MEDIUM, "CANTP_STANDARD\n")
		tpci = CanTpRxPduPtr->SduDataPtr[0];
		break;
	case CANTP_EXTENDED:
		DEBUG( DEBUG_MEDIUM, "CANTP_EXTENDED\n")
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

// - - - - - - - - - - - - - -

void initRx15765RuntimeData(const CanTp_RxNSduType *rxConfigParams,
		CanTp_ChannelPrivateType *rxRuntimeParams) {

	rxRuntimeParams->iso15765.state = IDLE;
	rxRuntimeParams->iso15765.NasNarPending = FALSE;
	rxRuntimeParams->iso15765.framesHandledCount = 0;
	rxRuntimeParams->iso15765.nextFlowControlCount = 0;
	rxRuntimeParams->pdurBufferCount = 0;
	rxRuntimeParams->pduLenghtTotalBytes = 0;
	rxRuntimeParams->pduTransferedBytesCount = 0;
	rxRuntimeParams->mode = CANTP_RX_WAIT; /* req: CanTp030. */
	rxRuntimeParams->bufferPduRouter = NULL;
}

// - - - - - - - - - - - - - -

void initTx15765RuntimeData(const CanTp_TxNSduType *txConfigParams,
		CanTp_ChannelPrivateType *txRuntimeParams) {

	txRuntimeParams->iso15765.state = IDLE;
	txRuntimeParams->iso15765.NasNarPending = FALSE;
	txRuntimeParams->iso15765.framesHandledCount = 0;
	txRuntimeParams->iso15765.nextFlowControlCount = 0;
	txRuntimeParams->pdurBufferCount = 0;
	txRuntimeParams->pduLenghtTotalBytes = 0;
	txRuntimeParams->pduTransferedBytesCount = 0;
	txRuntimeParams->mode = CANTP_TX_WAIT; /* req: CanTp030. */
	txRuntimeParams->bufferPduRouter = NULL;

}

// - - - - - - - - - - - - - -

static INLINE BufReq_ReturnType copySegmentToPduRRxBuffer(const CanTp_RxNSduType *rxConfig,
		CanTp_ChannelPrivateType *rxRuntime, uint8 *segment,
		PduLengthType segmentSize, PduLengthType *bytesWrittenSuccessfully) {

	BufReq_ReturnType ret = BUFREQ_NOT_OK;
	boolean error = FALSE;
	*bytesWrittenSuccessfully = 0;

	while ((*bytesWrittenSuccessfully < segmentSize) && error == FALSE) {
		// Copy the data that resides in the buffer.
		if (rxRuntime->bufferPduRouter != NULL) {
			while ((*bytesWrittenSuccessfully < segmentSize ) &&
					(rxRuntime->bufferPduRouter->SduLength > rxRuntime->pdurBufferCount)) {
				rxRuntime->bufferPduRouter->SduDataPtr[rxRuntime->pdurBufferCount++]
						= segment[(*bytesWrittenSuccessfully)++];
			}
		}
		if (*bytesWrittenSuccessfully < segmentSize ) {
			// We need to request a new buffer from the SDUR.
			// qqq: TODO: We should do a timeout here.
			ret = PduR_CanTpProvideRxBuffer(rxConfig->PduR_PduId,
							rxRuntime->pduLenghtTotalBytes,
							&rxRuntime->bufferPduRouter);
			if (ret == BUFREQ_OK) {
				VALIDATE( rxRuntime->bufferPduRouter->SduDataPtr != NULL,
						SERVICE_ID_CANTP_TRANSMIT, CANTP_E_INVALID_RX_BUFFER );
				rxRuntime->pdurBufferCount = 0; // The buffer is emptied.
			} else if (ret == BUFREQ_BUSY) {
				rxRuntime->pduTransferedBytesCount += *bytesWrittenSuccessfully;
				error = TRUE;
				break;
			} else {
				error = TRUE; // Let calling function handle this error.
				break;
			}
		} else {
			rxRuntime->pduTransferedBytesCount += segmentSize; //== bytesWrittenSuccessfully
			ret = BUFREQ_OK;
			break;
		}
	}
	return ret;
}

// - - - - - - - - - - - - - -

static INLINE boolean copySegmentToLocalRxBuffer /*writeDataSegmentToLocalBuffer*/(
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

static INLINE Std_ReturnType canReceivePaddingHelper(
		const CanTp_RxNSduType *rxConfig, CanTp_ChannelPrivateType *rxRuntime,
		PduInfoType *PduInfoPtr) {
	if (rxConfig->CanTpRxPaddingActivation == CANTP_ON) {
		for (int i = PduInfoPtr->SduLength; i < CANIF_PDU_MAX_LENGTH; i++) {
			PduInfoPtr->SduDataPtr[i] = 0x0; // qqq: Does it have to be padded with zeroes?
		}
		PduInfoPtr->SduLength = CANIF_PDU_MAX_LENGTH;
	}
	rxRuntime->iso15765.NasNarTimeoutCount =
			CANTP_CONVERT_MS_TO_MAIN_CYCLES(rxConfig->CanTpNar * 1000); // req: CanTp075.
	rxRuntime->iso15765.NasNarPending = TRUE;
	return CanIf_Transmit(rxConfig->CanIf_FcPduId, PduInfoPtr);
}

// - - - - - - - - - - - - - -

static INLINE Std_ReturnType canTansmitPaddingHelper(
		const CanTp_TxNSduType *txConfig, CanTp_ChannelPrivateType *txRuntime,
		PduInfoType *PduInfoPtr) {
	if (txConfig->CanTpTxPaddingActivation == CANTP_ON) {
		for (int i = PduInfoPtr->SduLength; i < CANIF_PDU_MAX_LENGTH; i++) {
			PduInfoPtr->SduDataPtr[i] = 0x0; // qqq: Does it have to be padded with zeroes?
		}
		PduInfoPtr->SduLength = CANIF_PDU_MAX_LENGTH;
	}
	txRuntime->iso15765.NasNarTimeoutCount =
			CANTP_CONVERT_MS_TO_MAIN_CYCLES(txConfig->CanTpNas * 1000); // req: CanTp075.
	txRuntime->iso15765.NasNarPending = TRUE;
	return CanIf_Transmit(txConfig->CanIf_PduId, PduInfoPtr);
}

// - - - - - - - - - - - - - -

static INLINE void sendFlowControlFrame(const CanTp_RxNSduType *rxConfig,
		CanTp_ChannelPrivateType *rxRuntime, BufReq_ReturnType flowStatus) {
	int indexCount = 0;
	Std_ReturnType ret = E_NOT_OK;
	PduInfoType pduInfo;
	uint8 sduData[8]; // Note that buffer in declared on the stack.
	uint16 spaceFreePduRBuffer = 0;
	uint8 computedBs = 0; // req:CanTp064 and example.

	DEBUG( DEBUG_MEDIUM, "sendFlowControlFrame called!\n");
	pduInfo.SduDataPtr = &sduData[0];
	if (rxConfig->CanTpAddressingFormant == CANTP_EXTENDED) {
		sduData[indexCount++] = rxRuntime->iso15765.extendedAddress;
	}
	switch (flowStatus) {
	case BUFREQ_OK:
	{
		sduData[indexCount++] = ISO15765_TPCI_FC
				| ISO15765_FLOW_CONTROL_STATUS_CTS;
		spaceFreePduRBuffer = rxRuntime->bufferPduRouter->SduLength -
				rxRuntime->pdurBufferCount;
		if (rxConfig->CanTpAddressingFormant == CANTP_EXTENDED) {
			computedBs = spaceFreePduRBuffer / MAX_PAYLOAD_SF_EXT_ADDR + 1;  // + 1 is for local buffer.
		} else {
			computedBs = spaceFreePduRBuffer / MAX_PAYLOAD_SF_STD_ADDR + 1;  // + 1 is for local buffer.
		}
		if (computedBs > rxConfig->CanTpBs) { // qqq: CanTp084 + example.
			computedBs = rxConfig->CanTpBs; // Should not exceed the max configured value.
		}
		DEBUG( DEBUG_MEDIUM, "computedBs:%d\n", computedBs);
		sduData[indexCount++] = computedBs;
		sduData[indexCount++] = (uint8) rxConfig->CanTpSTmin;
		rxRuntime->iso15765.nextFlowControlCount = (uint8) computedBs;
		pduInfo.SduLength = indexCount;
		break;
	}
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
		PduR_CanTpRxIndication(rxConfig->PduR_PduId,
				(NotifResultType) NTFRSLT_E_NOT_OK);
		rxRuntime->iso15765.state = IDLE;
		rxRuntime->mode = CANTP_RX_WAIT;
	}
}

// - - - - - - - - - - - - - -


static INLINE void handleConsecutiveFrame(const CanTp_RxNSduType *rxConfig,
		CanTp_ChannelPrivateType *rxRuntime, const PduInfoType *rxPduData) {
	int indexCount = 0;
	uint8 segmentNumber = 0;
	uint8 extendedAddress = 0;
	PduLengthType bytesLeftToCopy = 0;
	PduLengthType bytesLeftToTransfer = 0;
	PduLengthType currentSegmentSize = 0;
	PduLengthType currentSegmentMaxSize = 0;
	PduLengthType bytesCopiedToPdurRxBuffer = 0;
	BufReq_ReturnType ret = BUFREQ_NOT_OK;

	if (rxConfig->CanTpAddressingFormant == CANTP_EXTENDED) {
		extendedAddress = rxPduData->SduDataPtr[indexCount++];
		// TODO: Should we validate the extended address ?
	}
	segmentNumber = rxPduData->SduDataPtr[indexCount++] & SEGMENT_NUMBER_MASK;
	if (segmentNumber != (rxRuntime->iso15765.framesHandledCount
			& SEGMENT_NUMBER_MASK)) {
		DEBUG(DEBUG_MEDIUM,"Segmentation number error detected - is the sending"
				"unit too fast? Increase STmin (cofig) to slow it down!\n");
		PduR_CanTpRxIndication(rxConfig->PduR_PduId, NTFRSLT_E_WRONG_SN);
		rxRuntime->iso15765.state = IDLE;
		rxRuntime->mode = CANTP_RX_WAIT;
	} else {
		currentSegmentMaxSize = CANIF_PDU_MAX_LENGTH - indexCount;
		bytesLeftToCopy = rxRuntime->pduLenghtTotalBytes
				- rxRuntime->pduTransferedBytesCount;
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
			PduR_CanTpRxIndication(rxConfig->PduR_PduId, NTFRSLT_E_NO_BUFFER);
			rxRuntime->iso15765.state = IDLE;
			rxRuntime->mode = CANTP_RX_WAIT;
		} else if (ret == BUFREQ_BUSY) {
			boolean dataCopyFailure = FALSE;
			PduLengthType bytesNotCopiedToPdurRxBuffer =
					currentSegmentSize - bytesCopiedToPdurRxBuffer;
			if (rxConfig->CanTpAddressingFormant == CANTP_STANDARD) {
				if ( copySegmentToLocalRxBuffer(rxRuntime,
						&rxPduData->SduDataPtr[1 + bytesCopiedToPdurRxBuffer],
						bytesNotCopiedToPdurRxBuffer ) != TRUE ) {
					rxRuntime->iso15765.state = IDLE;
					rxRuntime->mode = CANTP_RX_WAIT;
					dataCopyFailure = TRUE;
					DEBUG( DEBUG_MEDIUM, "Unexpected error, could not copy 'unaligned leftover' "
							"data to local buffer!\n");
				}
			} else {
				if ( copySegmentToLocalRxBuffer(rxRuntime,
						&rxPduData->SduDataPtr[2 + bytesCopiedToPdurRxBuffer],
						bytesNotCopiedToPdurRxBuffer) != TRUE ) {
					rxRuntime->iso15765.state = IDLE;
					rxRuntime->mode = CANTP_RX_WAIT;
					dataCopyFailure = TRUE;
					DEBUG( DEBUG_MEDIUM, "Unexpected error, could not copy 'unaligned leftover' "
							"data to local buffer!\n");
				}
			}
			if ( dataCopyFailure == FALSE ) {
				rxRuntime->iso15765.framesHandledCount++;
				rxRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(rxConfig->CanTpNbr);
				rxRuntime->iso15765.state = RX_WAIT_SDU_BUFFER;
				rxRuntime->mode = CANTP_RX_PROCESSING;
				sendFlowControlFrame(rxConfig, rxRuntime, ret);  // Sends "WAIT" to remote node.
			}
		} else if (ret == BUFREQ_OK) {
			bytesLeftToTransfer = rxRuntime->pduLenghtTotalBytes - rxRuntime->pduTransferedBytesCount;
			if (bytesLeftToTransfer > 0) {
				rxRuntime->iso15765.framesHandledCount++;
				COUNT_DECREMENT(rxRuntime->iso15765.nextFlowControlCount);
				if (rxRuntime->iso15765.nextFlowControlCount == 0) {
					sendFlowControlFrame(rxConfig, rxRuntime, BUFREQ_OK);
				}
			} else {
				DEBUG( DEBUG_MEDIUM,"ISO15765-Rx session finished, going back to IDLE!\n");
				rxRuntime->iso15765.state = IDLE;
				rxRuntime->mode = CANTP_RX_WAIT;
				PduR_CanTpRxIndication(rxConfig->PduR_PduId, NTFRSLT_OK);
			}
		}
	}
}

// - - - - - - - - - - - - - -

static INLINE Std_ReturnType sendConsecutiveFrame(
		const CanTp_TxNSduType *txConfig, CanTp_ChannelPrivateType *txRuntime) {
	BufReq_ReturnType ret = BUFREQ_NOT_OK;
	uint8 sduData[CANIF_PDU_MAX_LENGTH];
	PduLengthType consecutiveFrameMaxPayload = 0;
	PduLengthType consecutiveFrameActualPayload = 0;
	PduLengthType remaningSduDataSize = 0;
	PduInfoType pduInfo;
	int copyCount = 0;
	int indexCount = 0;

	if (txConfig->CanTpAddressingMode == CANTP_EXTENDED) {
		sduData[indexCount++] = (uint8) txConfig->CanTpNTa->CanTpNTa; // Target address.
	}
	sduData[indexCount++] = ISO15765_TPCI_CF | (uint8)(
			(txRuntime->iso15765.framesHandledCount + 1) & ISO15765_TPCI_FS_MASK); // + 1 is because the consecutive frame numbering begins with 1 and not 0.

	// Always copy from the PDUR buffer data to the canFrameBuffer because if
	// we are unlucky the application give us very small buffers.
	consecutiveFrameMaxPayload  = CANIF_PDU_MAX_LENGTH - indexCount;
	remaningSduDataSize = txRuntime->pduLenghtTotalBytes
			- txRuntime->pduTransferedBytesCount;

	// Calculate number of valid bytes that reside in this CF.
	if ( remaningSduDataSize < consecutiveFrameMaxPayload  ) {
		consecutiveFrameActualPayload = remaningSduDataSize; // Last frame.
	} else {
		consecutiveFrameActualPayload = consecutiveFrameMaxPayload;
	}
	copyCount = txRuntime->canFrameBuffer.byteCount; // maybe some bytes already reside in the buffer that we need to handle before proceeding with application buffer data.
	while (copyCount < consecutiveFrameActualPayload) {
		if ( txRuntime->bufferPduRouter->SduLength > txRuntime->pdurBufferCount ) {
			txRuntime->canFrameBuffer.data[copyCount] =
					txRuntime->bufferPduRouter->SduDataPtr[txRuntime->pdurBufferCount++];
			copyCount++;
			txRuntime->canFrameBuffer.byteCount++;
		} else {
			BufReq_ReturnType pdurResp = PduR_CanTpProvideTxBuffer(txConfig->PduR_PduId,
					&txRuntime->bufferPduRouter, 0);
			if (pdurResp == BUFREQ_OK) {
				txRuntime->pdurBufferCount = 0;
				continue;
			} else if (pdurResp == BUFREQ_BUSY) {
				ret = E_OK; // We will remain in this state, called again later, not data lost/destoryed?
			} else {
				DEBUG( DEBUG_MEDIUM, "sendConsecutiveFrame failed, no buffer provided!\n");
				ret = E_NOT_OK; // Serious malfunction, function caller should cancel this transfer.
				break;
			}
		}
	}
	if (copyCount == consecutiveFrameActualPayload) {
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
			COUNT_DECREMENT(txRuntime->iso15765.nextFlowControlCount);
			txRuntime->pduTransferedBytesCount += txRuntime->canFrameBuffer.byteCount;
			txRuntime->canFrameBuffer.byteCount = 0;
			DEBUG( DEBUG_MEDIUM, "pduTransferedBytesCount:%d\n", txRuntime->pduTransferedBytesCount);
		}
	} else {
		DEBUG( DEBUG_MEDIUM, "Unexpected error, should not happen!\n");
	}
	DEBUG( DEBUG_MEDIUM, "sendConsecutiveFrame exit!\n");
	return ret;
}

// - - - - - - - - - - - - - -

static INLINE void handleConsecutiveFrameSent(
		const CanTp_TxNSduType *txConfig, CanTp_ChannelPrivateType *txRuntime) {

	if (txRuntime->pduLenghtTotalBytes <= txRuntime->pduTransferedBytesCount) {
		// Transfer finished!
		txRuntime->iso15765.state = IDLE;
		txRuntime->mode = CANTP_TX_WAIT;
		PduR_CanTpTxConfirmation(txConfig->PduR_PduId, (NotifResultType) NTFRSLT_OK);
	} else if (txRuntime->iso15765.nextFlowControlCount == 0) {
		if (txRuntime->iso15765.BS) { // Check if receiver expects flow control.
			// Time to send flow control!
			txRuntime->iso15765.stateTimeoutCount =
					CANTP_CONVERT_MS_TO_MAIN_CYCLES(txConfig->CanTpNbs);  /*CanTp: 264*/
			txRuntime->iso15765.state = TX_WAIT_FLOW_CONTROL;
		} else {
			// Send next consecutive frame!
			txRuntime->iso15765.stateTimeoutCount =
					CANTP_CONVERT_MS_TO_MAIN_CYCLES(txRuntime->iso15765.STmin);
			txRuntime->iso15765.state = TX_WAIT_SEND_CONSECUTIVE_FRAME;
		}
	} else {
		// Send next consecutive frame!
		txRuntime->iso15765.stateTimeoutCount =
				CANTP_CONVERT_MS_TO_MAIN_CYCLES(txRuntime->iso15765.STmin);
		txRuntime->iso15765.state = TX_WAIT_SEND_CONSECUTIVE_FRAME;
	}
}

// - - - - - - - - - - - - - -

static INLINE void handleFlowControlFrame(const CanTp_TxNSduType *txConfig,
		CanTp_ChannelPrivateType *txRuntime, const PduInfoType *txPduData) {
	int indexCount = 0;
	uint8 extendedAddress = 0;
	Std_ReturnType ret;

	if (txConfig->CanTpAddressingMode == CANTP_EXTENDED) {
		extendedAddress = txPduData->SduDataPtr[indexCount++];
	}
	switch (txPduData->SduDataPtr[indexCount++] & ISO15765_TPCI_FS_MASK) {
	case ISO15765_FLOW_CONTROL_STATUS_CTS:
		/* qqq: TODO: Sometimes this value can be forced by config, but not in AutoSAR CANTP ? */
		txRuntime->iso15765.BS = txPduData->SduDataPtr[indexCount++];
		txRuntime->iso15765.nextFlowControlCount = txRuntime->iso15765.BS;
		txRuntime->iso15765.STmin = txPduData->SduDataPtr[indexCount++];
		ret = sendConsecutiveFrame(txConfig, txRuntime);
		if (ret == E_OK) {
			handleConsecutiveFrameSent(txConfig, txRuntime);
		} else {
			DEBUG( DEBUG_MEDIUM, "sendConsecutiveFrame returned error!\n");
			PduR_CanTpRxIndication(txConfig->PduR_PduId,
					(NotifResultType) NTFRSLT_E_NOT_OK);
			txRuntime->iso15765.state = IDLE;
			txRuntime->mode = CANTP_TX_WAIT;
		}
		break;
	case ISO15765_FLOW_CONTROL_STATUS_WAIT:
		DEBUG( DEBUG_MEDIUM, "----------------------->Flow Control: WAIT!\n");
		txRuntime->iso15765.stateTimeoutCount =
				CANTP_CONVERT_MS_TO_MAIN_CYCLES(txConfig->CanTpNbs);  /*CanTp: 264*/
		txRuntime->iso15765.state = TX_WAIT_FLOW_CONTROL;
		break;
	case ISO15765_FLOW_CONTROL_STATUS_OVFLW:
		DEBUG( DEBUG_MEDIUM, "----------------------->Flow Control: OVERFLOW!\n");
		PduR_CanTpRxIndication(txConfig->PduR_PduId,
				(NotifResultType) NTFRSLT_E_NOT_OK);
		txRuntime->iso15765.state = IDLE;
		txRuntime->mode = CANTP_TX_WAIT;
		break;
	}
}

// - - - - - - - - - - - - - -

static INLINE void handleSingleFrame(const CanTp_RxNSduType *rxConfig,
		CanTp_ChannelPrivateType *rxRuntime, const PduInfoType *rxPduData) {
	BufReq_ReturnType ret;
	PduLengthType pduLength;
	uint8 *data = NULL;
	PduLengthType bytesWrittenToSduRBuffer;

	DEBUG( DEBUG_MEDIUM, "handleSingleFrame called!\n");
	if (rxRuntime->iso15765.state != IDLE) {
		DEBUG( DEBUG_MEDIUM, "Single frame received and channel not valid!\n");
	}

	(void) initRx15765RuntimeData(rxConfig, rxRuntime);
	pduLength = getPduLength(&rxConfig->CanTpAddressingFormant, SINGLE_FRAME,
			rxPduData);

	VALIDATE_NO_RV( rxRuntime->bufferPduRouter->SduDataPtr != NULL,
			SERVICE_ID_CANTP_RX_INDICATION, CANTP_E_INVALID_RX_LENGTH );

	if (rxConfig->CanTpAddressingFormant == CANTP_STANDARD) {
		data = &rxPduData->SduDataPtr[1];
	} else {
		data = &rxPduData->SduDataPtr[2];
	}
	rxRuntime->pduLenghtTotalBytes = pduLength;
	rxRuntime->iso15765.state = SF_OR_FF_RECEIVED_WAITING_PDUR_BUFFER;
	rxRuntime->mode = CANTP_RX_PROCESSING;
	rxRuntime->iso15765.stateTimeoutCount =
			CANTP_CONVERT_MS_TO_MAIN_CYCLES(rxConfig->CanTpNbr);


	ret = copySegmentToPduRRxBuffer(rxConfig, rxRuntime, data, pduLength,
			&bytesWrittenToSduRBuffer);
	if (ret == BUFREQ_OK && rxRuntime->iso15765.stateTimeoutCount != 0) {
		PduR_CanTpRxIndication(rxConfig->PduR_PduId, NTFRSLT_OK);
	} else {
		PduR_CanTpRxIndication(rxConfig->PduR_PduId, NTFRSLT_E_NO_BUFFER);
	}
	rxRuntime->iso15765.state = IDLE;
	rxRuntime->mode = CANTP_RX_WAIT;
}

// - - - - - - - - - - - - - -


static INLINE void handleFirstFrame(const CanTp_RxNSduType *rxConfig,
		CanTp_ChannelPrivateType *rxRuntime, const PduInfoType *rxPduData) {
	BufReq_ReturnType ret;
	PduLengthType pduLength = 0;
	PduLengthType bytesWrittenToSduRBuffer;

	if (rxRuntime->iso15765.state != IDLE) {
		DEBUG( DEBUG_MEDIUM, "Unexpected first frame during ongoing reception!\n" );
	}
	(void) initRx15765RuntimeData(rxConfig, rxRuntime);
	pduLength = getPduLength(&rxConfig->CanTpAddressingFormant, FIRST_FRAME,
			rxPduData);
	rxRuntime->pduLenghtTotalBytes = pduLength;

	VALIDATE_NO_RV( rxRuntime->pduLenghtTotalBytes != 0,
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

	rxRuntime->iso15765.framesHandledCount++; // First expected is one (1) according to Movimento Puma.
	rxRuntime->iso15765.state = SF_OR_FF_RECEIVED_WAITING_PDUR_BUFFER;
	rxRuntime->mode = CANTP_RX_PROCESSING;
	rxRuntime->iso15765.stateTimeoutCount =
			CANTP_CONVERT_MS_TO_MAIN_CYCLES(rxConfig->CanTpNbr); /* CanTP 166. */

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
		rxRuntime->iso15765.stateTimeoutCount =
				CANTP_CONVERT_MS_TO_MAIN_CYCLES(rxConfig->CanTpNcr);
		rxRuntime->iso15765.state = RX_WAIT_CONSECUTIVE_FRAME;
		rxRuntime->mode = CANTP_RX_PROCESSING;
		sendFlowControlFrame(rxConfig, rxRuntime, ret);
	} else if (ret == BUFREQ_BUSY) {
		/* req: CanTp: 222. */
		if (rxConfig->CanTpAddressingFormant == CANTP_STANDARD) {
			(void)copySegmentToLocalRxBuffer(rxRuntime,
					&rxPduData->SduDataPtr[2], MAX_PAYLOAD_FF_STD_ADDR );
		} else {
			(void)copySegmentToLocalRxBuffer(rxRuntime,
					&rxPduData->SduDataPtr[3], MAX_PAYLOAD_FF_EXT_ADDR );
		}
		rxRuntime->iso15765.stateTimeoutCount =
				CANTP_CONVERT_MS_TO_MAIN_CYCLES(rxConfig->CanTpNbr);
		rxRuntime->iso15765.state = RX_WAIT_SDU_BUFFER;
		rxRuntime->mode = CANTP_RX_PROCESSING;
		sendFlowControlFrame(rxConfig, rxRuntime, ret);  // Sends "WAIT" to remote node.
	} else if (ret == BUFREQ_OVFL) {
		sendFlowControlFrame(rxConfig, rxRuntime, ret);
		rxRuntime->iso15765.state = IDLE;
		rxRuntime->mode = CANTP_RX_WAIT;
	}
}

// - - - - - - - - - - - - - -

static INLINE Std_ReturnType calcRequiredProtocolFrameType(
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


static INLINE Std_ReturnType sendSingleFrame(const CanTp_TxNSduType *txConfig,
		CanTp_ChannelPrivateType *txRuntime) {
	Std_ReturnType ret;
	int indexCount = 0;
	PduInfoType pduInfo;
	uint8 sduData[CANIF_PDU_MAX_LENGTH]; // qqq: Note that buffer in declared on the stack.

	DEBUG( DEBUG_MEDIUM, "sendSingleFrame called!\n");

	if (txConfig->CanTpAddressingMode == CANTP_EXTENDED) {
		sduData[indexCount++] = (uint8) txConfig->CanTpNTa->CanTpNTa; // Target address.
	}
	sduData[indexCount++] = ISO15765_TPCI_SF | txRuntime->pduLenghtTotalBytes;
	for (int i = 0; i < txRuntime->pduLenghtTotalBytes; i++) {
		sduData[indexCount++] = txRuntime->bufferPduRouter->SduDataPtr[i];
	}

	pduInfo.SduDataPtr = sduData;
	pduInfo.SduLength = indexCount;
	ret = canTansmitPaddingHelper(txConfig, txRuntime, &pduInfo);
	return ret;
}

// - - - - - - - - - - - - - -

static INLINE Std_ReturnType sendFirstFrame(const CanTp_TxNSduType *txConfig,
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

static INLINE BufReq_ReturnType canTpTransmitHelper(const CanTp_TxNSduType *txConfig,
		CanTp_ChannelPrivateType *txRuntime) {

	BufReq_ReturnType pdurResp = BUFREQ_NOT_OK;
	Std_ReturnType res = E_NOT_OK;
	ISO15765FrameType iso15765Frame = INVALID_FRAME;

	pdurResp = PduR_CanTpProvideTxBuffer(txConfig->PduR_PduId,
			&txRuntime->bufferPduRouter, 0);  // Req: CanTp 186.
	//if (txRuntime->iso15765.stateTimeoutCount != 0) {   qqq: WHY WAS THIS DONE?
		VALIDATE( txRuntime->bufferPduRouter->SduDataPtr != NULL,
				SERVICE_ID_CANTP_TRANSMIT, CANTP_E_INVALID_TX_BUFFER );
		if (pdurResp == BUFREQ_OK) {
			res = calcRequiredProtocolFrameType(txConfig, txRuntime, &iso15765Frame);
			switch (iso15765Frame) {
			case SINGLE_FRAME:
				res = sendSingleFrame(txConfig, txRuntime); /* req: CanTp 231  */
				if (res == E_OK) {
					PduR_CanTpTxConfirmation(txConfig->PduR_PduId, NTFRSLT_OK);
				} else {
					PduR_CanTpTxConfirmation(txConfig->PduR_PduId, NTFRSLT_NOT_OK);
				}
				txRuntime->iso15765.state = IDLE;
				txRuntime->mode = CANTP_TX_WAIT;
				break;
			case FIRST_FRAME: {
				txRuntime->iso15765.stateTimeoutCount = CANTP_CONVERT_MS_TO_MAIN_CYCLES(txConfig->CanTpNbs * 1000);  /*CanTp: 264*/
				if ( txRuntime->iso15765.stateTimeoutCount == 0 ) {
					DEBUG( DEBUG_MEDIUM, "WARNING! Too low CanTpNbs timeout!\n" );
				}
				txRuntime->iso15765.state = TX_WAIT_FLOW_CONTROL;
				res = sendFirstFrame(txConfig, txRuntime); /* req: CanTp 232 */
				if (res == E_OK) {
					txRuntime->mode = CANTP_TX_PROCESSING;
				}
				break;
			}
			case INVALID_FRAME:
			default:
				PduR_CanTpTxConfirmation(txConfig->PduR_PduId, NTFRSLT_NOT_OK);
				txRuntime->iso15765.state = IDLE;
				txRuntime->mode = CANTP_TX_WAIT;
				break;
			}
		} else if (pdurResp == BUFREQ_NOT_OK) {
			PduR_CanTpTxConfirmation(txConfig->PduR_PduId, NTFRSLT_NOT_OK);
			txRuntime->iso15765.state = IDLE;
			txRuntime->mode = CANTP_TX_WAIT;
		} else if (pdurResp == BUFREQ_BUSY) {
			txRuntime->iso15765.state = TX_WAIT_CAN_TP_TRANSMIT_PENDING; // We have to issue this request later from main until timeout.
			txRuntime->mode = CANTP_TX_PROCESSING;
		}
	//} else {
		// qqq: TDDO: Put this in a logfile? Error response should have been
		// sent from main.
	//}
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
	//int index;

	DEBUG( DEBUG_MEDIUM, "CanTp_Transmit called in polite index: %d!\n", CanTpTxSduId);

	VALIDATE( CanTpTxInfoPtr != NULL,
			SERVICE_ID_CANTP_TRANSMIT, CANTP_E_PARAM_ADDRESS ); /* req: CanTp031 */
	VALIDATE( CanTpRunTimeData.internalState == CANTP_ON,
			SERVICE_ID_CANTP_TRANSMIT, CANTP_E_UNINIT ); /* req: CanTp031 */
	VALIDATE( CanTpTxSduId < CANTP_NSDU_CONFIG_LIST_SIZE, SERVICE_ID_CANTP_TRANSMIT, CANTP_E_INVALID_TX_ID );

	txConfig = (CanTp_TxNSduType*)&CanTpConfig.CanTpNSduList[CanTpTxSduId].configData;

	txRuntime = &CanTpRunTimeData.runtimeDataList[txConfig->CanTpTxChannel]; // Runtime data.
	txRuntime->pdurBufferCount = 0;
	txRuntime->pdurBufferCount = 0;
	txRuntime->pduTransferedBytesCount = 0;
	txRuntime->iso15765.framesHandledCount = 0;
	//txRuntime->iso15765->framesHandledCount = 0;
	txRuntime->pduLenghtTotalBytes = CanTpTxInfoPtr->SduLength;
	txRuntime->iso15765.stateTimeoutCount =
			CANTP_CONVERT_MS_TO_MAIN_CYCLES(txConfig->CanTpNcs);
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
	return ret; // CAN level error code.
}

// - - - - - - - - - - - - - -

#if FRTP_CANCEL_TRANSMIT_REQUEST
Std_ReturnType FrTp_CancelTransmitRequest(PduIdType FrTpTxPduId,
		FrTp_CancelReasonType FrTpCancelReason) /** req : CanTp246 **/
{
	return E_NOT_OK;
}
#endif

// - - - - - - - - - - - - - -


void CanTp_Init() /** req : CanTp208. **/
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
				DEBUG( DEBUG_MEDIUM, "ERROR: Configuration error detected on index %d!\n", i );
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
	fifoQueueInit( &CanTpRunTimeData.fifo );
	CanTpRunTimeData.internalState = CANTP_ON;
}

// - - - - - - - - - - - - - -

void CanTp_RxIndication(PduIdType CanTpRxPduId,
		const PduInfoType *CanTpRxPduPtr) /** req : CanTp214. **/
{
	CanTpFifoQueueItem item;
	VALIDATE_NO_RV( CanTpRunTimeData.internalState == CANTP_ON,
			SERVICE_ID_CANTP_RX_INDICATION, CANTP_E_UNINIT ); /* req: CanTp031 */

	item.PduId = CanTpRxPduId;
	item.SduLength = CanTpRxPduPtr->SduLength;
	for (int i=0; i<item.SduLength; i++) {
		item.SduData[i] = CanTpRxPduPtr->SduDataPtr[i];
	}
	if ( fifoQueueWrite( &CanTpRunTimeData.fifo, &item ) == FALSE ) {
		DEBUG( DEBUG_MEDIUM, "WARNING!: Frames are lost!\n");
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

	DEBUG( DEBUG_MEDIUM, "CanTp_RxIndication: PduId=%d, [", CanTpRxPduId);
	for (int i=0; i<CanTpRxPduPtr->SduLength; i++) {
		DEBUG( DEBUG_MEDIUM, "%x, ", CanTpRxPduPtr->SduDataPtr[i]);
	}
	DEBUG( DEBUG_MEDIUM, "]");

	VALIDATE_NO_RV( CanTpRunTimeData.internalState == CANTP_ON,
			SERVICE_ID_CANTP_RX_INDICATION, CANTP_E_UNINIT ); /* req: CanTp031 */

	if ( CanTpConfig.CanTpNSduList[CanTpRxPduId].direction == IS015765_TRANSMIT ) {
		txConfigParams =
				(CanTp_TxNSduType*)&CanTpConfig.CanTpNSduList[CanTpRxPduId].configData;
		addressingFormat = &txConfigParams->CanTpAddressingMode;
		runtimeParams = &CanTpRunTimeData.runtimeDataList[txConfigParams->CanTpTxChannel];
		rxConfigParams = NULL;
	} else {
		rxConfigParams =
				(CanTp_RxNSduType*)&CanTpConfig.CanTpNSduList[CanTpRxPduId].configData;
		addressingFormat = &rxConfigParams->CanTpAddressingFormant;
		runtimeParams = &CanTpRunTimeData.runtimeDataList[rxConfigParams->CanTpRxChannel];
		txConfigParams = NULL;
	}

	frameType = getFrameType(addressingFormat, CanTpRxPduPtr);
	switch (frameType) {
	case SINGLE_FRAME: {
		if (rxConfigParams != NULL) {
			DEBUG( DEBUG_MEDIUM, "calling handleSingleFrame!\n");
			handleSingleFrame(rxConfigParams, runtimeParams, CanTpRxPduPtr);
		}
		else
			DEBUG( DEBUG_MEDIUM, "Single frame received on ISO15765-Tx flow control - is ingnored!\n");
		break;
	}
	case FIRST_FRAME: {
		if (rxConfigParams != NULL) {
			DEBUG( DEBUG_MEDIUM, "calling handleFirstFrame!\n");
			handleFirstFrame(rxConfigParams, runtimeParams, CanTpRxPduPtr);
		} else
			DEBUG( DEBUG_MEDIUM, "First frame received on ISO15765-Tx flow control - is ignored!\n");
		break;
	}
	case CONSECUTIVE_FRAME: {
		if (rxConfigParams != NULL) {
			DEBUG( DEBUG_MEDIUM, "calling handleConsecutiveFrame!\n");
			handleConsecutiveFrame(rxConfigParams, runtimeParams, CanTpRxPduPtr);
		} else
			DEBUG( DEBUG_MEDIUM, "Consecutive frame received on ISO15765-Tx flow control - is ignored!\n");
		break;
	}
	case FLOW_CONTROL_CTS_FRAME: {
		if (txConfigParams != NULL) {
			DEBUG( DEBUG_MEDIUM, "calling handleFlowControlFrame!\n");
			handleFlowControlFrame(txConfigParams, runtimeParams, CanTpRxPduPtr);
		} else
			DEBUG( DEBUG_MEDIUM, "Flow control frame received on ISO15765-RX flow control - is ignored!\n");
		break;
	}
	case INVALID_FRAME: {
		DEBUG( DEBUG_MEDIUM, "INVALID_FRAME recived, doing nothing!!\n");
		break;
	}
	default:
		break;
	}
	DEBUG( DEBUG_LOW, "CanTp_RxIndication_Main exit!\n");
}

// - - - - - - - - - - - - - -

#if (CANTP_VERSION_INFO_API == STD_ON)
void CanTp_GetVersionInfo(Std_VersionInfoType* versionInfo) /** req : CanTp210 **/
{
	memcpy(versionInfo, &_CanTp_VersionInfo, sizeof(Std_VersionInfoType));
}
#endif /* DEM_VERSION_INFO_API */

// - - - - - - - - - - - - - -

void CanTp_TxConfirmation(PduIdType PduId) /** req: CanTp215 **/
{
	const CanTp_RxNSduType *rxConfigParams = NULL;
	const CanTp_TxNSduType *txConfigParams = NULL;

	DEBUG( DEBUG_MEDIUM, "CanTp_TxConfirmation called.\n" );

	VALIDATE_NO_RV( CanTpRunTimeData.internalState == CANTP_ON,
			SERVICE_ID_CANTP_TX_CONFIRMATION, CANTP_E_UNINIT ); /* req: CanTp031 */
	VALIDATE_NO_RV( PduId < CANTP_NSDU_CONFIG_LIST_SIZE,
			SERVICE_ID_CANTP_TX_CONFIRMATION, CANTP_E_INVALID_TX_ID ); /* req: CanTp031 */

	if ( CanTpConfig.CanTpNSduList[PduId].direction == IS015765_TRANSMIT ) {
		txConfigParams =
				(CanTp_TxNSduType*)&CanTpConfig.CanTpNSduList[PduId].configData;
		CanTpRunTimeData.runtimeDataList[txConfigParams->CanTpTxChannel]
		                                 .iso15765.NasNarPending = FALSE;
	} else {
		rxConfigParams =
				(CanTp_RxNSduType*)&CanTpConfig.CanTpNSduList[PduId].configData;
		CanTpRunTimeData.runtimeDataList[rxConfigParams->CanTpRxChannel]
		                                 .iso15765.NasNarPending = FALSE;
	}
}

// - - - - - - - - - - - - - -

void CanTp_Shutdown() /** req : CanTp211 **/
{
	VALIDATE_NO_RV( CanTpRunTimeData.internalState == CANTP_ON,
			SERVICE_ID_CANTP_SHUTDOWN, CANTP_E_UNINIT ); /* req: CanTp031 */

	CanTpRunTimeData.internalState = CANTP_OFF;
}

// - - - - - - - - - - - - - -


static inline boolean checkNasNarTimeout(CanTp_ChannelPrivateType *runtimeData) {
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


void CanTp_MainFunction() /** req : CanTp213 **/
{
  BufReq_ReturnType ret;
	CanTpFifoQueueItem item;
	PduLengthType bytesWrittenToSduRBuffer;

	CanTp_ChannelPrivateType *txRuntimeListItem = NULL;
	CanTp_ChannelPrivateType *rxRuntimeListItem = NULL;

	const CanTp_TxNSduType *txConfigListItem = NULL;
	const CanTp_RxNSduType *rxConfigListItem = NULL;

	//DEBUG( DEBUG_MEDIUM, "CanTp_MainFunction called.\n" );
	VALIDATE_NO_RV( CanTpRunTimeData.internalState == CANTP_ON,
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

	for( int i=0; i < CANTP_NSDU_CONFIG_LIST_SIZE; i++ ) {
#if 0	// TODO: Not tested yet
		if (checkNasNarTimeout( txRuntimeListItem )) { // CanTp075.
			PduR_CanTpTxConfirmation(txConfigListItem->PduR_CanTpTxPduId, NTFRSLT_NOT_OK); // qqq: req: CanTp: 185.
			continue;
		}
#endif
		if ( CanTpConfig.CanTpNSduList[i].direction == IS015765_TRANSMIT ) {
			txConfigListItem = (CanTp_TxNSduType*)&CanTpConfig.CanTpNSduList[i].configData;
			txRuntimeListItem = &CanTpRunTimeData.runtimeDataList[txConfigListItem->CanTpTxChannel];

			switch (txRuntimeListItem->iso15765.state) {
			case TX_WAIT_CAN_TP_TRANSMIT_CAN_TP_PROVIDE_TX_BUFFER:
				TIMER_DECREMENT (rxRuntimeListItem->iso15765.stateTimeoutCount);
				if (txRuntimeListItem->iso15765.stateTimeoutCount == 0)
					PduR_CanTpTxConfirmation(txConfigListItem->PduR_PduId,
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
					DEBUG( DEBUG_MEDIUM, "ERROR: Consecutive frame could not be sent!\n");
					PduR_CanTpTxConfirmation(txConfigListItem->PduR_PduId,
							(NotifResultType) NTFRSLT_E_NOT_OK);
					txRuntimeListItem->iso15765.state = IDLE;
					txRuntimeListItem->mode = CANTP_TX_WAIT;
				}
				break;
			case TX_WAIT_FLOW_CONTROL:
				//DEBUG( DEBUG_MEDIUM, "Waiting for flow control!\n");
				if (txRuntimeListItem->iso15765.stateTimeoutCount == 0) {
					DEBUG( DEBUG_MEDIUM, "State TX_WAIT_FLOW_CONTROL timed out!\n");
					PduR_CanTpTxConfirmation(txConfigListItem->PduR_PduId,
							NTFRSLT_NOT_OK); /* qqq: req: CanTp: 185. */
					txRuntimeListItem->iso15765.state = IDLE;
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
					rxRuntimeListItem->iso15765.state = IDLE;
					rxRuntimeListItem->mode = CANTP_RX_WAIT;
					PduR_CanTpRxIndication(rxConfigListItem->PduR_PduId,
							(NotifResultType) NTFRSLT_E_NOT_OK);
				}
				break;
			}
			case RX_WAIT_SDU_BUFFER: {
				TIMER_DECREMENT (rxRuntimeListItem->iso15765.stateTimeoutCount);
				/* We end up here if we have requested a buffer from the
				 * PDUR but the response have been BUSY. We assume
				 * we have data in our local buffer and we are expected
				 * to send a flowcontrol clear to send (CTS).
				 */
				if (rxRuntimeListItem->iso15765.stateTimeoutCount == 0) {
					PduR_CanTpTxConfirmation(rxConfigListItem->PduR_PduId,
							NTFRSLT_NOT_OK);
					rxRuntimeListItem->iso15765.state = IDLE;
					rxRuntimeListItem->mode = CANTP_RX_WAIT;
				} else {
					ret = copySegmentToPduRRxBuffer(rxConfigListItem,
								rxRuntimeListItem,
								rxRuntimeListItem->canFrameBuffer.data,
								rxRuntimeListItem->canFrameBuffer.byteCount,
								&bytesWrittenToSduRBuffer);
					sendFlowControlFrame( rxConfigListItem, rxRuntimeListItem, ret );
					if (ret == BUFREQ_OK ) {
						rxRuntimeListItem->iso15765.state = RX_WAIT_CONSECUTIVE_FRAME;
					} else if (ret == BUFREQ_NOT_OK ) {
						PduR_CanTpTxConfirmation(rxConfigListItem->PduR_PduId,
								NTFRSLT_NOT_OK);
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


