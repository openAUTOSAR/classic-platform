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


/*
 *  General requirements
 */
/** @req CANTP133 */
/** @req CANTP289 */
/** @req CANTP288 */
/** @req CANTP287 */
/** @req CANTP286 */
/** @req CANTP285 */
/** @req CANTP327 */
/** @req CANTP326 */
/** @req CANTP307 */
/** @req CANTP296 */
/** @req CANTP265 */
/** @req CANTP249 */
/** @req CANTP250 */
/** @req CANTP251 */
/** @req CANTP252 */
/** @req CANTP248 */
/** @req CANTP001 */
/** @req CANTP002 */
/** @req CANTP008 */
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

/** @req CANTP132 */ /** @req CANTP021 */ /** @req CANTP294 */
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

BufReq_ReturnType PduR_CanTpCopyRxData(PduIdType CanTpRxId, PduInfoType *PduInfoPtr, PduLengthType *length) {
	;
}

BufReq_ReturnType PduR_CanTpCopyTxData(PduIdType id, PduInfoType* info, RetryInfoType *retry, PduLengthType *availableDataPtr) {
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

/** @req CANTP033 */

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

#define CANTP_TX_CHANNEL			0
#define CANTP_RX_CHANNEL			1
#define CANTP_NO_SIMPLEX_CHANNEL    2
#define INVALID_PDU_ID				0xFFFF

/*
 *
 */
typedef enum {
	UNINITIALIZED, IDLE, SF_OR_FF_RECEIVED_WAITING_PDUR_BUFFER, /** @req CANTP079 */
	RX_WAIT_CONSECUTIVE_FRAME, RX_WAIT_SF_SDU_BUFFER, RX_WAIT_CF_SDU_BUFFER,

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
	PduLengthType transferTotal; // Total length of the PDU.
	PduLengthType transferCount; // Counter ongoing transfer.
	PduLengthType sizeBuffer;
	CanIfSduType canFrameBuffer; // Temp storage of SDU data.
	CanTp_TransferInstanceMode mode; // CanTp030.
	uint16 CanTpWftMaxCounter;
	PduIdType pduId;
} CanTp_SimplexChannelPrivateType;

typedef struct {
	CanTp_SimplexChannelPrivateType SimplexChnlList[CANTP_NO_SIMPLEX_CHANNEL]; //one for RX and one for TX
	CanTp_SimplexChannelPrivateType functionalChnl; //For functional frame reception
}CanTp_ChannelPrivateType;
// - - - - - - - - - - - - - -

typedef struct {
	boolean initRun;
	CanTp_StateType internalState; /** @req CANTP027 */
	CanTp_ChannelPrivateType runtimeDataList[CANTP_MAX_NO_CHANNELS];
} CanTp_RunTimeDataType;

// - - - - - - - - - - - - - -

CanTp_RunTimeDataType CanTpRunTimeData = { .initRun = FALSE,
		.internalState = CANTP_OFF }; /** @req CANTP168 */

/* Global configure */
static const CanTp_ConfigType *CanTp_ConfigPtr = NULL;


// - - - - - - - - - - - - - -


static uint32 ConvertMsToMainCycles(uint32 ms) {
    return (ms/CanTp_ConfigPtr->CanTpGeneral->main_function_period);
}


ISO15765FrameType getFrameType(
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
		default :
			/* Invalid FC recognized and transmision is aborted.This is handled in handleFlowControlFrame() */
			res = FLOW_CONTROL_CTS_FRAME;
			break;
		}
	}
	return res;
}

// - - - - - - - - - - - - - -

PduLengthType getPduLength(
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

static void initRx15765RuntimeData(CanTp_SimplexChannelPrivateType *rxRuntimeParams) {

	rxRuntimeParams->iso15765.state = IDLE;
	rxRuntimeParams->iso15765.NasNarPending = FALSE;
	rxRuntimeParams->iso15765.framesHandledCount = 0;
	rxRuntimeParams->iso15765.nextFlowControlCount = 0;
	rxRuntimeParams->transferTotal = 0;
	rxRuntimeParams->transferCount = 0;
	rxRuntimeParams->sizeBuffer = 0;
	rxRuntimeParams->CanTpWftMaxCounter = 0;
	rxRuntimeParams->mode = CANTP_RX_WAIT; /** @req CANTP030 */
	rxRuntimeParams->pduId = INVALID_PDU_ID;
}

// - - - - - - - - - - - - - -

static void initTx15765RuntimeData(CanTp_SimplexChannelPrivateType *txRuntimeParams) {

	txRuntimeParams->iso15765.state = IDLE;
	txRuntimeParams->iso15765.NasNarPending = FALSE;
	txRuntimeParams->iso15765.framesHandledCount = 0;
	txRuntimeParams->iso15765.nextFlowControlCount = 0;
	txRuntimeParams->transferTotal = 0;
	txRuntimeParams->transferCount = 0;
	txRuntimeParams->sizeBuffer = 0;
	txRuntimeParams->mode = CANTP_TX_WAIT; /** @req CANTP030 */
	txRuntimeParams->pduId = INVALID_PDU_ID;

}

// - - - - - - - - - - - - - -

BufReq_ReturnType copySegmentToPduRRxBuffer(const CanTp_RxNSduType *rxConfig,
		CanTp_SimplexChannelPrivateType *rxRuntime, uint8 *segment,
		PduLengthType segmentSize, PduLengthType *bytesWrittenSuccessfully) {

	BufReq_ReturnType ret;
	static PduInfoType tempPdu;

	*bytesWrittenSuccessfully = 0;

	/* when not a consectuive frame we have to begin a new buffer */
	if (rxRuntime->iso15765.state != RX_WAIT_CONSECUTIVE_FRAME)
	{
		ret = PduR_CanTpStartOfReception(rxConfig->PduR_PduId, rxRuntime->transferTotal, &rxRuntime->sizeBuffer); /** @req CANTP079 */
		/* ok buffer is now locked for us */
		/* until PduR_CanTpTxConfirmation or PduR_CanTpRxIndication arises */
	}
	else
	{
		/** @req CANTP270 */
		/* ask how many free space is left */
		tempPdu.SduLength = 0;
		tempPdu.SduDataPtr = NULL_PTR;
		ret = PduR_CanTpCopyRxData(rxConfig->PduR_PduId, &tempPdu, &rxRuntime->sizeBuffer);
	}

	if (ret == BUFREQ_OK)
	{
		tempPdu.SduDataPtr = segment;

		if (segmentSize <= rxRuntime->sizeBuffer) /** @req CANTP080 */
		{
			/* everything is ok - we can go on */
			tempPdu.SduLength = segmentSize;
			ret = PduR_CanTpCopyRxData(rxConfig->PduR_PduId, &tempPdu, &rxRuntime->sizeBuffer);
		}
		else
		{
			/* currently there is not enough space available */
			ret = BUFREQ_E_BUSY;
		}

		/* when everything is fine we can adjust our "global" variables and end */
		if (ret == BUFREQ_OK)
		{
			*bytesWrittenSuccessfully = tempPdu.SduLength;
			rxRuntime->transferCount += tempPdu.SduLength;
		}
	}

	return ret;
}

// - - - - - - - - - - - - - -

boolean copySegmentToLocalRxBuffer /*writeDataSegmentToLocalBuffer*/(
		CanTp_SimplexChannelPrivateType *rxRuntime, uint8 *segment,
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

Std_ReturnType canReceivePaddingHelper(
		const CanTp_RxNSduType *rxConfig, CanTp_SimplexChannelPrivateType *rxRuntime,
		PduInfoType *PduInfoPtr) {
	if (rxConfig->CanTpRxPaddingActivation == CANTP_ON) {
		for (int i = PduInfoPtr->SduLength; i < MAX_SEGMENT_DATA_SIZE; i++) {
			PduInfoPtr->SduDataPtr[i] = CanTp_ConfigPtr->CanTpGeneral->padding;
		}
		PduInfoPtr->SduLength = MAX_SEGMENT_DATA_SIZE;
	}
	rxRuntime->iso15765.NasNarTimeoutCount = rxConfig->CanTpNar; /** @req CANTP075 */
	rxRuntime->iso15765.NasNarPending = TRUE;
	return CanIf_Transmit(rxConfig->CanIf_FcPduId, PduInfoPtr);
}

// - - - - - - - - - - - - - -

Std_ReturnType canTansmitPaddingHelper(
		const CanTp_TxNSduType *txConfig, CanTp_SimplexChannelPrivateType *txRuntime,
		PduInfoType *PduInfoPtr) {

	/** @req CANTP114 */
	/** @req CANTP040 */
	/** @req CANTP098 */
	/** @req CANTP116 */
	/** @req CANTP059 */

	if (txConfig->CanTpTxPaddingActivation == CANTP_ON) { /** @req CANTP225 */
		for (int i = PduInfoPtr->SduLength; i < MAX_SEGMENT_DATA_SIZE; i++) {
			PduInfoPtr->SduDataPtr[i] = CanTp_ConfigPtr->CanTpGeneral->padding;
		}
		PduInfoPtr->SduLength = MAX_SEGMENT_DATA_SIZE;
	}
	txRuntime->iso15765.NasNarTimeoutCount = txConfig->CanTpNas; /** @req CANTP075 */
	txRuntime->iso15765.NasNarPending = TRUE;
	return CanIf_Transmit(txConfig->CanIf_PduId, PduInfoPtr);
}

// - - - - - - - - - - - - - -

void sendFlowControlFrame(const CanTp_RxNSduType *rxConfig, CanTp_SimplexChannelPrivateType *rxRuntime, BufReq_ReturnType flowStatus) {
	uint8 indexCount = 0;
	Std_ReturnType ret = E_NOT_OK;
	PduInfoType pduInfo;
	uint8 sduData[8]; // Note that buffer in declared on the stack.
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
		if (rxConfig->CanTpAddressingFormant == CANTP_EXTENDED) { /** @req CANTP094 *//** @req CANTP095 */
			computedBs = (rxRuntime->sizeBuffer / MAX_PAYLOAD_SF_EXT_ADDR) + 1;  // + 1 is for local buffer.
		} else {
			computedBs = (rxRuntime->sizeBuffer / MAX_PAYLOAD_SF_STD_ADDR) + 1;  // + 1 is for local buffer.
		}
		if (computedBs > rxConfig->CanTpBs) { // /** @req CANTP091 *//** @req CANTP084 */
			computedBs = rxConfig->CanTpBs;
		}
		rxRuntime->iso15765.BS = rxConfig->CanTpBs;

		DEBUG( DEBUG_MEDIUM, "computedBs:%d\n", computedBs);
		sduData[indexCount++] = computedBs; // 734 PC-lint: Okej att casta till uint8?
		sduData[indexCount++] = (uint8) rxConfig->CanTpSTmin;
		rxRuntime->iso15765.nextFlowControlCount = computedBs;
		pduInfo.SduLength = indexCount;
		rxRuntime->CanTpWftMaxCounter = 0;
		break;
	}
	case BUFREQ_E_NOT_OK:
		break;
	case BUFREQ_E_BUSY:
		sduData[indexCount++] = ISO15765_TPCI_FC | ISO15765_FLOW_CONTROL_STATUS_WAIT;
		indexCount +=2;
		pduInfo.SduLength = indexCount;
		rxRuntime->CanTpWftMaxCounter++;
		break;
	case BUFREQ_E_OVFL: /** @req CANTP318 Assuming 1-byte frame length */
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
		rxRuntime->pduId = INVALID_PDU_ID;
		rxRuntime->mode = CANTP_RX_WAIT;
	}
}

// - - - - - - - - - - - - - -


void handleConsecutiveFrame(const CanTp_RxNSduType *rxConfig,
		CanTp_SimplexChannelPrivateType *rxRuntime, const PduInfoType *rxPduData) {
	uint8 indexCount = 0;
	uint8 segmentNumber = 0;
	PduLengthType bytesLeftToCopy = 0;
	PduLengthType bytesLeftToTransfer = 0;
	PduLengthType currentSegmentSize = 0;
	PduLengthType currentSegmentMaxSize = 0;
	PduLengthType bytesCopiedToPdurRxBuffer = 0;
	BufReq_ReturnType ret = BUFREQ_E_NOT_OK;

	if (rxConfig->CanTpAddressingFormant == CANTP_EXTENDED)
	{
		uint8 extendedAddress = 0;
		extendedAddress = rxPduData->SduDataPtr[indexCount++];
		//Verfify the target address
		if (extendedAddress != rxConfig->CanTpNTa->CanTpNTa)
			return;
	}
	if (rxRuntime->iso15765.state == RX_WAIT_CONSECUTIVE_FRAME) {

		segmentNumber = rxPduData->SduDataPtr[indexCount++] & SEGMENT_NUMBER_MASK;
		if (segmentNumber != (rxRuntime->iso15765.framesHandledCount & SEGMENT_NUMBER_MASK)) { /** @req CANTP314 */
			DEBUG(DEBUG_MEDIUM,"Segmentation number error detected - is the sending"
					"unit too fast? Increase STmin (cofig) to slow it down!\n");
			PduR_CanTpRxIndication(rxConfig->PduR_PduId, NTFRSLT_E_WRONG_SN); /** @req CANTP084 */
			rxRuntime->iso15765.state = IDLE;
			rxRuntime->pduId = INVALID_PDU_ID;
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
					&bytesCopiedToPdurRxBuffer); /** @req CANTP269 */

			if (ret == BUFREQ_E_NOT_OK) {
				PduR_CanTpRxIndication(rxConfig->PduR_PduId, NTFRSLT_E_NOT_OK); /** @req CANTP084 */
				rxRuntime->iso15765.state = IDLE; /** @req CANTP271 */
				rxRuntime->pduId = INVALID_PDU_ID;
				rxRuntime->mode = CANTP_RX_WAIT;
			} else if (ret == BUFREQ_E_BUSY) {

				boolean status = FALSE;
				status = (rxConfig->CanTpAddressingFormant == CANTP_STANDARD) ? (bytesLeftToCopy > 7 ) : (bytesLeftToCopy > 6 );
				// Check whether this is the last CF in the block and a FC is to be sent
				if ((rxRuntime->iso15765.nextFlowControlCount == 1) && (status))
				{
					boolean dataCopyFailure = FALSE;
					PduLengthType bytesNotCopiedToPdurRxBuffer = currentSegmentSize - bytesCopiedToPdurRxBuffer;
					if (rxConfig->CanTpAddressingFormant == CANTP_STANDARD) {
						if ( copySegmentToLocalRxBuffer(rxRuntime,
								&rxPduData->SduDataPtr[1 + bytesCopiedToPdurRxBuffer],
								bytesNotCopiedToPdurRxBuffer ) != TRUE ) {
							rxRuntime->iso15765.state = IDLE; /** @req CANTP271 */ /* this can be quite dangerous! imagine a concurrent reception (ok sender should wait some time..) - we set first to idle and then a few steps later to RX_WAIT_SDU_BUFFER => in the meantime a new firstframe could come in?!?! */
							/* by the way - requirement 271 isn't correct!! when receiving busy we still have wait until WFT > the configured value */
							rxRuntime->pduId = INVALID_PDU_ID;
							rxRuntime->mode = CANTP_RX_WAIT;
							dataCopyFailure = TRUE;
							DEBUG( DEBUG_MEDIUM, "Unexpected error, could not copy 'unaligned leftover' " "data to local buffer!\n");
						}
					} else {
						if ( copySegmentToLocalRxBuffer(rxRuntime,
								&rxPduData->SduDataPtr[2 + bytesCopiedToPdurRxBuffer],
								bytesNotCopiedToPdurRxBuffer) != TRUE ) {
							rxRuntime->iso15765.state = IDLE; /** @req CANTP271 */ /* this can be quite dangerous! imagine a concurrent reception (ok sender should wait some time..) - we set first to idle and then a few steps later to RX_WAIT_SDU_BUFFER => in the meantime a new firstframe could come in?!?! */
							/* by the way - requirement 271 isn't correct!! when receiving busy we still have wait until WFT > the configured value */
							rxRuntime->pduId = INVALID_PDU_ID;
							rxRuntime->mode = CANTP_RX_WAIT;
							dataCopyFailure = TRUE;
							DEBUG( DEBUG_MEDIUM, "Unexpected error, could not copy 'unaligned leftover' " "data to local buffer!\n");
						}
					}
					if ( !dataCopyFailure ) {
						rxRuntime->iso15765.framesHandledCount++;
						rxRuntime->iso15765.stateTimeoutCount = (rxConfig->CanTpNbr) + 1;
						rxRuntime->iso15765.state = RX_WAIT_CF_SDU_BUFFER;
						rxRuntime->mode = CANTP_RX_PROCESSING;
						sendFlowControlFrame(rxConfig, rxRuntime, ret);  /** @req CANTP268 */
					}
				}

				else
				{
					// Abort connection /** @req CANTP271 */
					PduR_CanTpRxIndication(rxConfig->PduR_PduId, NTFRSLT_E_NOT_OK); /** @req CANTP084 */
					rxRuntime->iso15765.state = IDLE; /** @req CANTP271 */
					rxRuntime->pduId = INVALID_PDU_ID;
					rxRuntime->mode = CANTP_RX_WAIT;

				}
			} else if (ret == BUFREQ_OK) {
				bytesLeftToTransfer = rxRuntime->transferTotal - rxRuntime->transferCount;
				if (bytesLeftToTransfer > 0) {
					rxRuntime->iso15765.framesHandledCount++;
					COUNT_DECREMENT(rxRuntime->iso15765.nextFlowControlCount);
					if (rxRuntime->iso15765.nextFlowControlCount == 0 && rxRuntime->iso15765.BS > 0) {
						sendFlowControlFrame(rxConfig, rxRuntime, BUFREQ_OK);
					} else {
						rxRuntime->iso15765.stateTimeoutCount = (rxConfig->CanTpNcr) + 1;  //UH /** @req CANTP312 */
					}
				} else {
					DEBUG( DEBUG_MEDIUM,"ISO15765-Rx session finished, going back to IDLE!\n");
					rxRuntime->iso15765.state = IDLE;
					rxRuntime->pduId = INVALID_PDU_ID;
					rxRuntime->mode = CANTP_RX_WAIT;
					PduR_CanTpRxIndication(rxConfig->PduR_PduId, NTFRSLT_OK); /** @req CANTP084 */
				}
			}
		}
		}
}

// - - - - - - - - - - - - - -

BufReq_ReturnType sendNextTxFrame(
		const CanTp_TxNSduType *txConfig, CanTp_SimplexChannelPrivateType *txRuntime) {
	BufReq_ReturnType ret = BUFREQ_OK;

	PduInfoType pduInfo;

	uint8 offset = txRuntime->canFrameBuffer.byteCount;

	pduInfo.SduDataPtr = &txRuntime->canFrameBuffer.data[offset];
	if( (txRuntime->transferTotal - txRuntime->transferCount) >  (MAX_SEGMENT_DATA_SIZE - offset) ) {
		pduInfo.SduLength = MAX_SEGMENT_DATA_SIZE - offset;
	} else {
		pduInfo.SduLength = txRuntime->transferTotal - txRuntime->transferCount;
	}

	ret = PduR_CanTpCopyTxData(txConfig->PduR_PduId, &pduInfo, NULL_PTR, &txRuntime->sizeBuffer); /** @req CANTP272 */ /** @req CANTP226 */ /** @req CANTP086 */

	txRuntime->canFrameBuffer.byteCount += pduInfo.SduLength; /* SduLength could have changed during transmit (when frame is smaller than MAX_SEGMENT_DATA_SIZE) */
	txRuntime->transferCount += pduInfo.SduLength; /* SduLength could have changed during transmit (when frame is smaller than MAX_SEGMENT_DATA_SIZE) */

	if(ret == BUFREQ_OK) {
		Std_ReturnType resp;

		pduInfo.SduDataPtr = &txRuntime->canFrameBuffer.data[0];
		pduInfo.SduLength += offset;

		// change state to verify tx confirm within timeout
		txRuntime->iso15765.stateTimeoutCount = (txConfig->CanTpNas) + 1;
		txRuntime->iso15765.state = TX_WAIT_TX_CONFIRMATION;
		resp = canTansmitPaddingHelper(txConfig, txRuntime, &pduInfo);
		if(resp == E_OK) {
			// sending done
		} else {
			// failed to send
			ret = BUFREQ_E_NOT_OK;
		}
	}
	return ret;
}

// - - - - - - - - - - - - - -

void handleNextTxFrameSent(
		const CanTp_TxNSduType *txConfig, CanTp_SimplexChannelPrivateType *txRuntime) {

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
		PduR_CanTpTxConfirmation(txConfig->PduR_PduId, NTFRSLT_OK); /** @req CANTP090 *//** @req CANTP204 */
		txRuntime->iso15765.state = IDLE;
		txRuntime->pduId = INVALID_PDU_ID;
		txRuntime->mode = CANTP_TX_WAIT;
	} else if (txRuntime->iso15765.nextFlowControlCount == 0 && txRuntime->iso15765.BS) {
		// receiver expects flow control.
		txRuntime->iso15765.stateTimeoutCount = (txConfig->CanTpNbs) + 1; /** @req CANTP315.partially */
		txRuntime->iso15765.state = TX_WAIT_FLOW_CONTROL;
	} else if (txRuntime->iso15765.STmin == 0) {
		// Send next consecutive frame!
		Std_ReturnType resp;
		resp = sendNextTxFrame(txConfig, txRuntime);
		if (resp == BUFREQ_OK ) {
			// successfully sent frame, wait for tx confirm
		} else if(BUFREQ_E_BUSY == resp) { /** @req CANTP184 */ /** @req CANTP279 */
			// change state and setup timeout
			txRuntime->iso15765.stateTimeoutCount = (txConfig->CanTpNcs) + 1;
			txRuntime->iso15765.state = TX_WAIT_TRANSMIT;
		} else {
			PduR_CanTpTxConfirmation(txConfig->PduR_PduId, NTFRSLT_E_NOT_OK);  /** @req CANTP177 */ /** @req CANTP084 */ /** @req CANTP087 */
			txRuntime->iso15765.state = IDLE;
			txRuntime->pduId = INVALID_PDU_ID;
			txRuntime->mode = CANTP_TX_WAIT;
		}
	} else {
		// Send next consecutive frame after stmin!
	    //ST MIN error handling ISO 15765-2 sec 7.6
	    if (txRuntime->iso15765.STmin < 0x80) {
		txRuntime->iso15765.stateTimeoutCount = ConvertMsToMainCycles(txRuntime->iso15765.STmin) + 1;
	    } else if (txRuntime->iso15765.STmin > 0xF0 && txRuntime->iso15765.STmin < 0xFA) {
	        txRuntime->iso15765.stateTimeoutCount = ConvertMsToMainCycles((txRuntime->iso15765.STmin - 0xF0)/10) + 1;
	    }
	    else {
	        txRuntime->iso15765.stateTimeoutCount = ConvertMsToMainCycles(0x7F) + 1;
	    }
	    txRuntime->iso15765.state = TX_WAIT_STMIN;
	}
}

// - - - - - - - - - - - - - -

void handleFlowControlFrame(const CanTp_TxNSduType *txConfig,
		CanTp_SimplexChannelPrivateType *txRuntime, const PduInfoType *txPduData) {
	int indexCount = 0;
	if (txConfig->CanTpAddressingMode == CANTP_EXTENDED)
	{
	   		/** @req CANTP094 *//** @req CANTP095 */
		uint8 extendedAddress = 0;
	    extendedAddress = txPduData->SduDataPtr[indexCount++];
	    //Verfify the target address
	    if (extendedAddress != txConfig->CanTpNSa->CanTpNSa)
	    	return;
	}
	if ( txRuntime->iso15765.state == TX_WAIT_FLOW_CONTROL )
	{
        switch (txPduData->SduDataPtr[indexCount++] & ISO15765_TPCI_FS_MASK)
		{

			case ISO15765_FLOW_CONTROL_STATUS_CTS:
	#if 1
			{	// This construction is added to make the hcs12 compiler happy.
				const uint8 bs = txPduData->SduDataPtr[indexCount++];
				txRuntime->iso15765.BS = bs;
				txRuntime->iso15765.nextFlowControlCount = bs;
			}
			txRuntime->iso15765.STmin = txPduData->SduDataPtr[indexCount++]; /** @req CANTP282 */
	#else
			txRuntime->iso15765.BS = txPduData->SduDataPtr[indexCount++];
			txRuntime->iso15765.nextFlowControlCount = txRuntime->iso15765.BS;
			txRuntime->iso15765.STmin = txPduData->SduDataPtr[indexCount++];
	#endif
			DEBUG( DEBUG_MEDIUM, "txRuntime->iso15765.STmin = %d\n", txRuntime->iso15765.STmin);
			// change state and setup timout
			txRuntime->iso15765.stateTimeoutCount = (txConfig->CanTpNcs) + 1;
			txRuntime->iso15765.state = TX_WAIT_TRANSMIT;
			break;
			case ISO15765_FLOW_CONTROL_STATUS_WAIT:
				txRuntime->iso15765.stateTimeoutCount = (txConfig->CanTpNbs) + 1; /** @req CANTP315.partially */
				txRuntime->iso15765.state = TX_WAIT_FLOW_CONTROL;
				break;
			case ISO15765_FLOW_CONTROL_STATUS_OVFLW:
				PduR_CanTpTxConfirmation(txConfig->PduR_PduId, NTFRSLT_E_NO_BUFFER); /* @req ISO/FDIS 15765-2:2004(E) 7.5.5.2*/ /** @req CANTP309 */
				txRuntime->iso15765.state = IDLE;
				txRuntime->pduId = INVALID_PDU_ID;
				txRuntime->mode = CANTP_TX_WAIT;
				break;

			default:
				/* Abort transmission if invalid FS */
				PduR_CanTpTxConfirmation(txConfig->PduR_PduId,NTFRSLT_E_INVALID_FS); /* @req ISO/FDIS 15765-2:2004(E) 7.5.5.3*/ /** @req CANTP317 */
				txRuntime->iso15765.state = IDLE;
				txRuntime->pduId = INVALID_PDU_ID;
				txRuntime->mode = CANTP_TX_WAIT;
				break;
			}
	} else {
		DEBUG( DEBUG_MEDIUM, "Ignoring flow control, we do not expect it!");
	}
}


// - - - - - - - - - - - - - -

void handleSingleFrame(const CanTp_RxNSduType *rxConfig,
		CanTp_SimplexChannelPrivateType *rxRuntime, const PduInfoType *rxPduData, PduIdType CanTpRxNSduId) {
	BufReq_ReturnType ret;
	PduLengthType pduLength;
	uint8 *data = NULL;
	PduLengthType bytesWrittenToSduRBuffer;

	if ((rxRuntime->iso15765.state != IDLE) && (rxRuntime->iso15765.state != RX_WAIT_CF_SDU_BUFFER) &&
            (rxRuntime->iso15765.state != RX_WAIT_SF_SDU_BUFFER)){
		DEBUG( DEBUG_MEDIUM, "Single frame received and channel not IDLE!\n");
		PduIdType pdurPduOngoing=CanTp_ConfigPtr->CanTpNSduList[rxRuntime->pduId].configData.CanTpRxNSdu.PduR_PduId;

		PduR_CanTpRxIndication(pdurPduOngoing, NTFRSLT_E_NOT_OK); /** @req CANTP084 */ // Abort current reception, we need to tell the current receiver it has been aborted.
	}
	if (rxConfig->CanTpAddressingFormant == CANTP_EXTENDED)
	{
		uint8 extendedAddress = 0;
		extendedAddress = rxPduData->SduDataPtr[0];
		//Verfify the target address
		if (extendedAddress != rxConfig->CanTpNTa->CanTpNTa && extendedAddress != 255)
			return;
	}
	initRx15765RuntimeData(rxRuntime); /** @req CANTP124 */
	pduLength = getPduLength(&rxConfig->CanTpAddressingFormant, SINGLE_FRAME, rxPduData);

	if (rxConfig->CanTpAddressingFormant == CANTP_STANDARD) { /** @req CANTP094 *//** @req CANTP095 */
		data = &rxPduData->SduDataPtr[1];
	} else {
		data = &rxPduData->SduDataPtr[2];
	}
	rxRuntime->transferTotal = pduLength;
	rxRuntime->iso15765.state = SF_OR_FF_RECEIVED_WAITING_PDUR_BUFFER;
	rxRuntime->mode = CANTP_RX_PROCESSING;
	rxRuntime->iso15765.stateTimeoutCount = (rxConfig->CanTpNbr) + 1;  /** @req CANTP166 */
	rxRuntime->pduId = CanTpRxNSduId;

	ret = copySegmentToPduRRxBuffer(rxConfig, rxRuntime, data, pduLength, &bytesWrittenToSduRBuffer); /** @req CANTP277 */

	if (ret == BUFREQ_OK) {
		PduR_CanTpRxIndication(rxConfig->PduR_PduId, NTFRSLT_OK); /** @req CANTP084 */
		rxRuntime->iso15765.state = IDLE;
		rxRuntime->pduId = INVALID_PDU_ID;
		rxRuntime->mode = CANTP_RX_WAIT;
	} else if (ret == BUFREQ_E_BUSY) {
		if (rxConfig->CanTpAddressingFormant == CANTP_STANDARD) {
			data = &rxPduData->SduDataPtr[1];
		} else {
			data = &rxPduData->SduDataPtr[2];
		}
		(void)copySegmentToLocalRxBuffer(rxRuntime, data, pduLength );
		rxRuntime->iso15765.state = RX_WAIT_SF_SDU_BUFFER;
		rxRuntime->mode = CANTP_RX_PROCESSING;
	} else {
		rxRuntime->iso15765.state = IDLE;
		rxRuntime->pduId = INVALID_PDU_ID;
		rxRuntime->mode = CANTP_RX_WAIT;
	}
}

// - - - - - - - - - - - - - -


void handleFirstFrame(const CanTp_RxNSduType *rxConfig,
		CanTp_SimplexChannelPrivateType *rxRuntime, const PduInfoType *rxPduData, PduIdType CanTpRxNSduId) {
	BufReq_ReturnType ret;
	PduLengthType pduLength = 0;
	PduLengthType bytesWrittenToSduRBuffer;
	uint8 extendedAddress = 0;

	if (rxConfig->CanTpAddressingFormant == CANTP_EXTENDED)
	{
		extendedAddress = rxPduData->SduDataPtr[0];
		//Verfify the target address
		if (extendedAddress != rxConfig->CanTpNTa->CanTpNTa)
			return;
	}
	if ((rxRuntime->iso15765.state != IDLE) && (rxRuntime->iso15765.state != RX_WAIT_CF_SDU_BUFFER) &&
            (rxRuntime->iso15765.state != RX_WAIT_SF_SDU_BUFFER)) {
        DEBUG( DEBUG_MEDIUM, "First frame received during Rx-session!\n" );
        PduIdType pdurPduOngoing=CanTp_ConfigPtr->CanTpNSduList[rxRuntime->pduId].configData.CanTpRxNSdu.PduR_PduId;

        PduR_CanTpRxIndication(pdurPduOngoing, NTFRSLT_E_NOT_OK);  /** @req CANTP084 */ // Abort current reception, we need to tell the current receiver it has been aborted.
	}
	initRx15765RuntimeData(rxRuntime); /** @req CANTP124 */
	pduLength = getPduLength(&rxConfig->CanTpAddressingFormant, FIRST_FRAME,
			rxPduData);
	rxRuntime->transferTotal = pduLength;
	rxRuntime->pduId = CanTpRxNSduId;

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
	rxRuntime->iso15765.stateTimeoutCount = rxConfig->CanTpNbr; /** @req CANTP166 */

	if (rxConfig->CanTpAddressingFormant == CANTP_STANDARD) {
		ret = copySegmentToPduRRxBuffer(rxConfig, rxRuntime,
				&rxPduData->SduDataPtr[2],
				MAX_PAYLOAD_FF_STD_ADDR,
				&bytesWrittenToSduRBuffer);
	} else {
		rxRuntime->iso15765.extendedAddress = rxConfig->CanTpNSa->CanTpNSa;
		ret = copySegmentToPduRRxBuffer(rxConfig, rxRuntime,
				&rxPduData->SduDataPtr[3],
				MAX_PAYLOAD_FF_EXT_ADDR,
				&bytesWrittenToSduRBuffer);
	}

	if (ret == BUFREQ_OK) {
		rxRuntime->iso15765.stateTimeoutCount = (rxConfig->CanTpNcr) + 1; /** @req CANTP312 */
		rxRuntime->iso15765.state = RX_WAIT_CONSECUTIVE_FRAME;
		rxRuntime->mode = CANTP_RX_PROCESSING;
		sendFlowControlFrame(rxConfig, rxRuntime, ret); /** @req CANTP064 */
	} else if (ret == BUFREQ_E_BUSY) {
		/** @req CANTP222 */
		if (rxConfig->CanTpAddressingFormant == CANTP_STANDARD) {
			(void)copySegmentToLocalRxBuffer(rxRuntime, &rxPduData->SduDataPtr[2], MAX_PAYLOAD_FF_STD_ADDR );
		} else {
			(void)copySegmentToLocalRxBuffer(rxRuntime, &rxPduData->SduDataPtr[3], MAX_PAYLOAD_FF_EXT_ADDR );
		}
		rxRuntime->iso15765.stateTimeoutCount = (rxConfig->CanTpNbr) + 1;
		rxRuntime->iso15765.state = RX_WAIT_CF_SDU_BUFFER;
		rxRuntime->mode = CANTP_RX_PROCESSING;
		sendFlowControlFrame(rxConfig, rxRuntime, ret);  /** @req CANTP082 */ /** @req CANTP064 */
	} else if (ret == BUFREQ_E_OVFL) {
		sendFlowControlFrame(rxConfig, rxRuntime, ret); /** @req CANTP318 */ /** @req CANTP064 */
		rxRuntime->iso15765.state = IDLE;
		rxRuntime->pduId = INVALID_PDU_ID;
		rxRuntime->mode = CANTP_RX_WAIT;
	}
	else if (ret == BUFREQ_E_NOT_OK) {
        rxRuntime->iso15765.state = IDLE;
        rxRuntime->pduId = INVALID_PDU_ID;
        rxRuntime->mode = CANTP_RX_WAIT;
	}
	/** @req CANTP081 */
}

// - - - - - - - - - - - - - -

ISO15765FrameType calcRequiredProtocolFrameType(
		const CanTp_TxNSduType *txConfig, CanTp_SimplexChannelPrivateType *txRuntime) {

	ISO15765FrameType ret = INVALID_FRAME;
	if (txConfig->CanTpAddressingMode == CANTP_EXTENDED) {
		if (txRuntime->transferTotal <= MAX_PAYLOAD_CF_EXT_ADDR) {
			ret = SINGLE_FRAME;
		} else {
			if (txConfig->CanTpTxTaType == CANTP_PHYSICAL) {
				ret = FIRST_FRAME;
			} else {
				DET_REPORTERROR( MODULE_ID_CANTP, 0, SERVICE_ID_CANTP_TRANSMIT, CANTP_E_INVALID_TATYPE ); /** @req CANTP093 */
			}
		}
	} else {	// CANTP_STANDARD
		if (txRuntime->transferTotal <= MAX_PAYLOAD_CF_STD_ADDR) {
			ret = SINGLE_FRAME;
		} else {
			if (txConfig->CanTpTxTaType == CANTP_PHYSICAL) {
				ret = FIRST_FRAME;
			} else {
				DET_REPORTERROR( MODULE_ID_CANTP, 0, SERVICE_ID_CANTP_TRANSMIT, CANTP_E_INVALID_TATYPE ); /** @req CANTP093 */
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
	CanTp_SimplexChannelPrivateType *txRuntime = NULL;
	Std_ReturnType ret = 0;
	PduIdType CanTp_InternalTxNSduId;

	DEBUG( DEBUG_MEDIUM, "CanTp_Transmit called in polite index: %d!\n", CanTpTxSduId);

	VALIDATE( CanTpTxInfoPtr != NULL,
			SERVICE_ID_CANTP_TRANSMIT, CANTP_E_PARAM_ADDRESS ); /** @req CANTP321 */
	VALIDATE( CanTpRunTimeData.internalState == CANTP_ON, /** @req CANTP238 */
			SERVICE_ID_CANTP_TRANSMIT, CANTP_E_UNINIT ); /** @req CANTP031 */
	VALIDATE( CanTpTxSduId < CanTp_ConfigPtr->CanTpGeneral->number_of_sdus, SERVICE_ID_CANTP_TRANSMIT, CANTP_E_INVALID_TX_ID );

	if( CanTp_ConfigPtr->CanTpRxIdList[CanTpTxSduId].CanTpNSduIndex != 0xFFFF ) {
		CanTp_InternalTxNSduId = CanTp_ConfigPtr->CanTpRxIdList[CanTpTxSduId].CanTpNSduIndex;

		txConfig =&CanTp_ConfigPtr->CanTpNSduList[CanTp_InternalTxNSduId].configData.CanTpTxNSdu;

		txRuntime = &CanTpRunTimeData.runtimeDataList[txConfig->CanTpTxChannel].SimplexChnlList[CANTP_TX_CHANNEL]; // Runtime data.
		if (txRuntime->iso15765.state == IDLE) {
			ISO15765FrameType iso15765Frame;
			txRuntime->canFrameBuffer.byteCount = 0;
			txRuntime->transferCount = 0;
			txRuntime->iso15765.framesHandledCount = 0;
			txRuntime->transferTotal = CanTpTxInfoPtr->SduLength; /** @req CANTP225 */
			txRuntime->iso15765.stateTimeoutCount = (txConfig->CanTpNcs) + 1; /** @req CANTP167 */
			txRuntime->mode = CANTP_TX_PROCESSING;
			txRuntime->pduId = CanTp_InternalTxNSduId;
			iso15765Frame = calcRequiredProtocolFrameType(txConfig, txRuntime); /** @req CANTP231 */ /** @req CANTP232 */
			if (txConfig->CanTpAddressingMode == CANTP_EXTENDED) { /** @req CANTP094 *//** @req CANTP095 */
				txRuntime->canFrameBuffer.data[txRuntime->canFrameBuffer.byteCount++] =
						(uint8) txConfig->CanTpNTa->CanTpNTa; // Target address.
			}
			switch(iso15765Frame) {
			case SINGLE_FRAME:
				txRuntime->canFrameBuffer.data[txRuntime->canFrameBuffer.byteCount++] =
						ISO15765_TPCI_SF | (uint8)(txRuntime->transferTotal);
				ret = E_OK;
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
				break;
			default:
				ret = E_NOT_OK;
			}
			txRuntime->iso15765.state = TX_WAIT_TRANSMIT;
		} else {
			DEBUG( DEBUG_MEDIUM, "CanTp can't transmit, it is already occupied!\n", CanTpTxSduId);
			ret = E_NOT_OK;  /** @req CANTP123 *//** @req CANTP206 */
		}
	}

	return ret; // CAN level error code.
}


/* @req CANTP273 */
void CanTp_Init( const CanTp_ConfigType* CfgPtr)
{

    VALIDATE_NO_RV(CfgPtr != NULL, SERVICE_ID_CANTP_INIT,CANTP_E_PARAM_POINTER);    /* @req CANTP320 */
    CanTp_ConfigPtr = CfgPtr;

	for (uint8 i=0; i < CANTP_MAX_NO_CHANNELS; i++) {

			initTx15765RuntimeData(&CanTpRunTimeData.runtimeDataList[i].SimplexChnlList[CANTP_TX_CHANNEL]);
			initRx15765RuntimeData(&CanTpRunTimeData.runtimeDataList[i].SimplexChnlList[CANTP_RX_CHANNEL]);
			initRx15765RuntimeData(&CanTpRunTimeData.runtimeDataList[i].functionalChnl);
	}
	CanTpRunTimeData.internalState = CANTP_ON; /** @req CANTP170 */
}

// - - - - - - - - - - - - - -
/** @req CANTP214 */
void CanTp_RxIndication(PduIdType CanTpRxPduId, /** @req CANTP078 */ /** @req CANTP035 */
		PduInfoType *CanTpRxPduPtr)
{

	const CanTp_RxNSduType *rxConfigParams; // Params reside in ROM.
	const CanTp_TxNSduType *txConfigParams;
	const CanTp_AddressingFormantType *addressingFormat; // Configured
	CanTp_SimplexChannelPrivateType *runtimeParams = 0; // Params reside in RAM.
	ISO15765FrameType frameType;
	PduIdType CanTpTxNSduId, CanTpRxNSduId;
	CanTpRxNSduId = INVALID_PDU_ID;

	//Check if PduId is valid
	if (CanTpRxPduId >= CanTp_ConfigPtr->CanTpGeneral->number_of_pdus )
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

	addressingFormat = &CanTp_ConfigPtr->CanTpRxIdList[CanTpRxPduId].CanTpAddressingMode;

	/* TODO John - Use a different indication of not set than 0xFFFF? */
	frameType = getFrameType(addressingFormat, CanTpRxPduPtr); /** @req CANTP094 *//** @req CANTP095 */ /** @req CANTP284 */
	if( frameType == FLOW_CONTROL_CTS_FRAME ) {
		if( CanTp_ConfigPtr->CanTpRxIdList[CanTpRxPduId].CanTpReferringTxIndex != 0xFFFF ) {
			CanTpTxNSduId = CanTp_ConfigPtr->CanTpRxIdList[CanTpRxPduId].CanTpReferringTxIndex;
			txConfigParams = &CanTp_ConfigPtr->CanTpNSduList[CanTpTxNSduId].configData.CanTpTxNSdu;
			runtimeParams = &CanTpRunTimeData.runtimeDataList[txConfigParams->CanTpTxChannel].SimplexChnlList[CANTP_TX_CHANNEL];
		}
		else {
		    //Invalid FC received
			return;
		}
		rxConfigParams = NULL;
	}
	else {
		if( CanTp_ConfigPtr->CanTpRxIdList[CanTpRxPduId].CanTpNSduIndex != 0xFFFF ) {
			CanTpRxNSduId = CanTp_ConfigPtr->CanTpRxIdList[CanTpRxPduId].CanTpNSduIndex;
			rxConfigParams = &CanTp_ConfigPtr->CanTpNSduList[CanTpRxNSduId].configData.CanTpRxNSdu;  /** @req CANTP120 */
			if( CANTP_FUNCTIONAL == rxConfigParams->CanTpRxTaType) {// Find if the current Pdu received is on a functional channel
			    runtimeParams = &CanTpRunTimeData.runtimeDataList[rxConfigParams->CanTpRxChannel].functionalChnl;
			}
			else {
			    runtimeParams = &CanTpRunTimeData.runtimeDataList[rxConfigParams->CanTpRxChannel].SimplexChnlList[CANTP_RX_CHANNEL];  /** @req CANTP096 *//** @req CANTP121 *//** @req CANTP122 *//** @req CANTP190 */
			}
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
			handleSingleFrame(rxConfigParams, runtimeParams, CanTpRxPduPtr, CanTpRxNSduId);  /** @req CANTP096 *//** @req CANTP121 *//** @req CANTP122 *//** @req CANTP190 */
		}
		else{
			DEBUG( DEBUG_MEDIUM, "Single frame received on ISO15765-Tx - is ignored!\n");
		}
		break;
	}
	case FIRST_FRAME: {
		if (rxConfigParams != NULL) {
			DEBUG( DEBUG_MEDIUM, "calling handleFirstFrame!\n");
			handleFirstFrame(rxConfigParams, runtimeParams, CanTpRxPduPtr,CanTpRxNSduId);
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
/** @req CANTP215 */
void CanTp_TxConfirmation(PduIdType CanTpTxPduId) /** @req CANTP076 */
{
	PduIdType CanTpNSduId;
	const CanTp_RxNSduType *rxConfigParams = NULL;
	const CanTp_TxNSduType *txConfigParams = NULL;

	DEBUG( DEBUG_MEDIUM, "CanTp_TxConfirmation called.\n" );

	VALIDATE_NO_RV( CanTpRunTimeData.internalState == CANTP_ON,
			SERVICE_ID_CANTP_TX_CONFIRMATION, CANTP_E_UNINIT ); /** @req CANTP031 */
	VALIDATE_NO_RV( CanTpTxPduId < CanTp_ConfigPtr->CanTpGeneral->number_of_pdus,
			SERVICE_ID_CANTP_TX_CONFIRMATION, CANTP_E_INVALID_TX_ID ); /** @req CANTP158 */

	/** @req CANTP236 */
	if( CanTp_ConfigPtr->CanTpRxIdList[CanTpTxPduId].CanTpNSduIndex != 0xFFFF ) {
		CanTpNSduId = CanTp_ConfigPtr->CanTpRxIdList[CanTpTxPduId].CanTpNSduIndex;
		if ( CanTp_ConfigPtr->CanTpNSduList[CanTpNSduId].direction == IS015765_TRANSMIT ) {
			txConfigParams = (CanTp_TxNSduType*)&CanTp_ConfigPtr->CanTpNSduList[CanTpNSduId].configData.CanTpTxNSdu;
			CanTp_SimplexChannelPrivateType *txRuntime = &CanTpRunTimeData.runtimeDataList[txConfigParams->CanTpTxChannel].SimplexChnlList[CANTP_TX_CHANNEL];
			if(txRuntime->iso15765.state == TX_WAIT_TX_CONFIRMATION) {
				handleNextTxFrameSent(txConfigParams, txRuntime);
			}
		} else {
			rxConfigParams = (CanTp_RxNSduType*)&CanTp_ConfigPtr->CanTpNSduList[CanTpNSduId].configData.CanTpRxNSdu;
			CanTpRunTimeData.runtimeDataList[rxConfigParams->CanTpRxChannel].SimplexChnlList[CANTP_RX_CHANNEL].iso15765.NasNarPending = FALSE;
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


#if 0
/* TODO: This needs to be fixed. Ticket created #2234.*/

boolean checkNasNarTimeout(PduIdType CanTpTxPduId, CanTp_SimplexChannelPrivateType *runtimeData) { /* this function is NEVER CALLED */
	boolean ret = FALSE;
	if (runtimeData->iso15765.NasNarPending) {
		TIMER_DECREMENT(runtimeData->iso15765.NasNarTimeoutCount);
		if (runtimeData->iso15765.NasNarTimeoutCount == 0) {
			DEBUG( DEBUG_MEDIUM, "NAS timed out.\n" );
			runtimeData->iso15765.state = IDLE;
			rxRuntime->pduId = INVALID_PDU_ID;
			runtimeData->iso15765.NasNarPending = FALSE;
			ret = TRUE;
			PduR_CanTpTxConfirmation(CanTpTxPduId, NTFRSLT_E_TIMEOUT_A); /** @req CANTP310 */ /** @req CANTP311 */
		}
	}
	return ret;
}

#endif


//This function tries to obtain a buffer in every MainFunction for a SF which is denied buffer by higher layers. This is not an ASR req.
void getBuffSingleFrame(const CanTp_RxNSduType *rxConfigListItem, CanTp_SimplexChannelPrivateType *rxRuntimeListItem) {
    /* We end up here if we have requested a buffer from the
     * PDUR but the response have been BUSY.
     */

    BufReq_ReturnType ret;
    PduLengthType bytesWrittenToSduRBuffer;

    TIMER_DECREMENT (rxRuntimeListItem->iso15765.stateTimeoutCount);
    /* first try to copy the the old data to the buffer */

    /** @req CANTP222 */
    ret = copySegmentToPduRRxBuffer(rxConfigListItem, rxRuntimeListItem, rxRuntimeListItem->canFrameBuffer.data, rxRuntimeListItem->canFrameBuffer.byteCount,   &bytesWrittenToSduRBuffer);

    if (ret == BUFREQ_OK)
    {
        PduR_CanTpRxIndication(rxConfigListItem->PduR_PduId, NTFRSLT_OK); /** @req CANTP084 */
        /** @req CANTP204 */
        rxRuntimeListItem->iso15765.state = IDLE;
        rxRuntimeListItem->pduId = INVALID_PDU_ID;
        rxRuntimeListItem->mode = CANTP_RX_WAIT;

    }
    else if (((ret == BUFREQ_E_BUSY) && (rxRuntimeListItem->iso15765.stateTimeoutCount == 0)) || (ret == BUFREQ_E_NOT_OK ))
    {
        /** @req CANTP204 */
        rxRuntimeListItem->iso15765.state = IDLE;
        rxRuntimeListItem->pduId = INVALID_PDU_ID;
        rxRuntimeListItem->mode = CANTP_RX_WAIT;
    }

    return;
}

void txSduStateMachine(const CanTp_TxNSduType *txConfigListItem, CanTp_SimplexChannelPrivateType *txRuntimeListItem) {

    BufReq_ReturnType ret;

    switch (txRuntimeListItem->iso15765.state) {
        case TX_WAIT_STMIN:
            TIMER_DECREMENT(txRuntimeListItem->iso15765.stateTimeoutCount); // Make sure that STmin timer has expired.
            if (txRuntimeListItem->iso15765.stateTimeoutCount != 0) {
                break;
            }
            txRuntimeListItem->iso15765.state = TX_WAIT_TRANSMIT;
            txRuntimeListItem->iso15765.stateTimeoutCount = (txConfigListItem->CanTpNcs) + 1;
            /* lint -restore */
            // no break, continue
        case TX_WAIT_TRANSMIT: {
            ret = sendNextTxFrame(txConfigListItem, txRuntimeListItem); /** @req CANTP184 */ /** @req CANTP089 */
            if ( ret == BUFREQ_OK ) {
                // successfully sent frame, wait for tx confirm
            } else if(BUFREQ_E_BUSY == ret) { /** @req CANTP184 */
                // check N_Cs timeout
                TIMER_DECREMENT(txRuntimeListItem->iso15765.stateTimeoutCount);
                if (txRuntimeListItem->iso15765.stateTimeoutCount == 0) {
                    /* Cs timeout */
                    DEBUG( DEBUG_MEDIUM, "ERROR: N_Cs timeout when sending consecutive frame!\n");
                    txRuntimeListItem->iso15765.state = IDLE;
                    txRuntimeListItem->pduId = INVALID_PDU_ID;
                    txRuntimeListItem->mode = CANTP_TX_WAIT;
                    PduR_CanTpTxConfirmation(txConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK); /** @req CANTP204 */ /** @req CANTP280 */ /** @req CANTP185 */
                } else {
                            DEBUG( DEBUG_MEDIUM, "Waiting for STmin timer to expire!\n");
                        }
                    } else {
                        DEBUG( DEBUG_MEDIUM, "ERROR: Consecutive frame could not be sent!\n");
                        txRuntimeListItem->iso15765.state = IDLE;
                        txRuntimeListItem->pduId = INVALID_PDU_ID;
                        txRuntimeListItem->mode = CANTP_TX_WAIT;
                        PduR_CanTpTxConfirmation(txConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK); /** @req CANTP204 */ /** @req CANTP087 */
                    }
                    break;
                }
        case TX_WAIT_FLOW_CONTROL:
            //DEBUG( DEBUG_MEDIUM, "Waiting for flow control!\n");
            TIMER_DECREMENT(txRuntimeListItem->iso15765.stateTimeoutCount);
            if (txRuntimeListItem->iso15765.stateTimeoutCount == 0) {
                /* N_Bs timeout */
                DEBUG( DEBUG_MEDIUM, "State TX_WAIT_FLOW_CONTROL timed out!\n");
                txRuntimeListItem->iso15765.state = IDLE;
                txRuntimeListItem->pduId = INVALID_PDU_ID;
                txRuntimeListItem->mode = CANTP_TX_WAIT;
                PduR_CanTpTxConfirmation(txConfigListItem->PduR_PduId, NTFRSLT_E_TIMEOUT_BS ); /** @req CANTP204 */ /** @req CANTP316 */
            }
            break;
        case TX_WAIT_TX_CONFIRMATION:
            TIMER_DECREMENT(txRuntimeListItem->iso15765.stateTimeoutCount);
            if (txRuntimeListItem->iso15765.stateTimeoutCount == 0) { /** @req CANTP075 */
                /* N_As timeout */
                txRuntimeListItem->iso15765.state = IDLE;
                txRuntimeListItem->pduId = INVALID_PDU_ID;
                txRuntimeListItem->mode = CANTP_TX_WAIT;
            }
            break;
        default:
            break;
        }
}

void rxPhySduStateMachine(const CanTp_RxNSduType *rxConfigListItem, CanTp_SimplexChannelPrivateType *rxRuntimeListItem) {

    BufReq_ReturnType ret;
    PduLengthType bytesWrittenToSduRBuffer;

    switch (rxRuntimeListItem->iso15765.state) {
        case RX_WAIT_CONSECUTIVE_FRAME: {
            TIMER_DECREMENT (rxRuntimeListItem->iso15765.stateTimeoutCount);
            if (rxRuntimeListItem->iso15765.stateTimeoutCount == 0) {
                /* N_Cr timeout */
                DEBUG( DEBUG_MEDIUM, "TIMEOUT!\n");
                PduR_CanTpRxIndication(rxConfigListItem->PduR_PduId, NTFRSLT_E_TIMEOUT_CR); /** @req CANTP084 */ /** @req CANTP313 */
                rxRuntimeListItem->iso15765.state = IDLE;
                rxRuntimeListItem->pduId = INVALID_PDU_ID;
                rxRuntimeListItem->mode = CANTP_RX_WAIT;
            }
            break;
        }
        case RX_WAIT_CF_SDU_BUFFER: {
            /* We end up here if we have requested a buffer from the
             * PDUR but the response have been BUSY. We assume
             * we have data in our local buffer and we are expected
             * to send a flow-control clear to send (CTS).
             */

            TIMER_DECREMENT (rxRuntimeListItem->iso15765.stateTimeoutCount);

            /* first try to copy the the old data to the buffer */

            /** @req CANTP222 */
            PduLengthType bytesRemaining = 0;
            ret = copySegmentToPduRRxBuffer(rxConfigListItem, rxRuntimeListItem, rxRuntimeListItem->canFrameBuffer.data, rxRuntimeListItem->canFrameBuffer.byteCount,   &bytesWrittenToSduRBuffer);
            bytesRemaining = rxRuntimeListItem->transferTotal - rxRuntimeListItem->transferCount;

            if (ret == BUFREQ_OK)
            {
                if (( bytesRemaining > 0))
                {
                    /** @req CANTP224 */
                    sendFlowControlFrame( rxConfigListItem, rxRuntimeListItem, ret );
                     /** @req CANTP312 */
                    rxRuntimeListItem->iso15765.stateTimeoutCount = rxConfigListItem->CanTpNcr;
                    rxRuntimeListItem->iso15765.state = RX_WAIT_CONSECUTIVE_FRAME;
                }
                else
                {
                    /** @req CANTP204 */
                    rxRuntimeListItem->iso15765.state = IDLE;
                    rxRuntimeListItem->pduId = INVALID_PDU_ID;
                    rxRuntimeListItem->mode = CANTP_RX_WAIT;
                    PduR_CanTpRxIndication(rxConfigListItem->PduR_PduId, NTFRSLT_OK); /** @req CANTP084 */
                }
            }
            else if (ret == BUFREQ_E_BUSY)
            {
                if (rxRuntimeListItem->iso15765.stateTimeoutCount == 0)
                {
                    /* N_Br timeout */
                    if ((rxRuntimeListItem->CanTpWftMaxCounter < rxConfigListItem->CanTpWftMax))
                    {
                        /** @req CANTP082 */
                        sendFlowControlFrame( rxConfigListItem, rxRuntimeListItem, BUFREQ_E_BUSY);
                        rxRuntimeListItem->iso15765.stateTimeoutCount = rxConfigListItem->CanTpNbr;
                    }
                    else
                    {
                        /** @req CANTP205 */
                        /** @req CANTP223 */
                        rxRuntimeListItem->iso15765.state = IDLE;
                        rxRuntimeListItem->pduId = INVALID_PDU_ID;
                        rxRuntimeListItem->mode = CANTP_RX_WAIT;
                        rxRuntimeListItem->CanTpWftMaxCounter = 0;
                        /* If FF has already acquired a buffer and last CF in a block fails indicate failure to PduR */
                        if (rxRuntimeListItem->transferCount != 0)
                        {
                            PduR_CanTpRxIndication(rxConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK); /** @req CANTP084 */
                        }
                    }
                }
                else
                {
                    if (rxRuntimeListItem->CanTpWftMaxCounter >= rxConfigListItem->CanTpWftMax)
                    {
                        /** @req CANTP205 */
                        /** @req CANTP223 */
                        rxRuntimeListItem->iso15765.state = IDLE;
                        rxRuntimeListItem->pduId = INVALID_PDU_ID;
                        rxRuntimeListItem->mode = CANTP_RX_WAIT;
                        rxRuntimeListItem->CanTpWftMaxCounter = 0;
                        /* If FF has already acquired a buffer and last CF in a block fails indicate failure to PduR */
                        if (rxRuntimeListItem->transferCount != 0)
                        {
                            PduR_CanTpRxIndication(rxConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK); /** @req CANTP084 */
                        }
                    }
                }
            }
            else if (ret == BUFREQ_E_NOT_OK )
            {
                rxRuntimeListItem->iso15765.state = IDLE;
                rxRuntimeListItem->pduId = INVALID_PDU_ID;
                rxRuntimeListItem->mode = CANTP_RX_WAIT;
                rxRuntimeListItem->CanTpWftMaxCounter = 0;
                /* If FF has already acquired a buffer and last CF in a block fails indicate failure to PduR */
                if (rxRuntimeListItem->transferCount != 0)
                {
                    PduR_CanTpRxIndication(rxConfigListItem->PduR_PduId, NTFRSLT_E_NOT_OK); /** @req CANTP084 */
                }

            }
            break;
        }
        case RX_WAIT_SF_SDU_BUFFER: {
            getBuffSingleFrame(rxConfigListItem, rxRuntimeListItem);
            break;
        }
        default:
            break;
        }

}

void rxFncSduStateMachine(const CanTp_RxNSduType *rxConfigListItem, CanTp_SimplexChannelPrivateType *rxRuntimeListItem) {

    switch (rxRuntimeListItem->iso15765.state) {
        case RX_WAIT_SF_SDU_BUFFER: {
            getBuffSingleFrame(rxConfigListItem, rxRuntimeListItem);
            break;
        }

        default:
        break;

    }
}
// - - - - - - - - - - - - - -


void CanTp_MainFunction(void)
{
    PduIdType pduTxId, pduRxId,pduRxFuncId;

    CanTp_SimplexChannelPrivateType *txRuntimeListItem = NULL;
    CanTp_SimplexChannelPrivateType *rxRuntimeListItem = NULL;

    const CanTp_TxNSduType *txConfigListItem = NULL;
    const CanTp_RxNSduType *rxConfigListItem = NULL;

    if( CanTpRunTimeData.internalState != CANTP_ON ) {
        DET_REPORTERROR(MODULE_ID_CANTP, 0, SERVICE_ID_CANTP_MAIN_FUNCTION, CANTP_E_UNINIT ); /** @req CANTP031 */
        return;
    }

    for( uint32 i=0; i < CANTP_MAX_NO_CHANNELS; i++ ) {
        pduTxId = CanTpRunTimeData.runtimeDataList[i].SimplexChnlList[CANTP_TX_CHANNEL].pduId;
        pduRxId = CanTpRunTimeData.runtimeDataList[i].SimplexChnlList[CANTP_RX_CHANNEL].pduId;
        pduRxFuncId = CanTpRunTimeData.runtimeDataList[i].functionalChnl.pduId;

        if (pduTxId != INVALID_PDU_ID)
        {
            txConfigListItem = (CanTp_TxNSduType*)&CanTp_ConfigPtr->CanTpNSduList[pduTxId].configData.CanTpTxNSdu;
            txRuntimeListItem = &CanTpRunTimeData.runtimeDataList[i].SimplexChnlList[CANTP_TX_CHANNEL];
            txSduStateMachine(txConfigListItem, txRuntimeListItem);
        }

        if (pduRxId != INVALID_PDU_ID)
        {

            rxConfigListItem = (CanTp_RxNSduType*)&CanTp_ConfigPtr->CanTpNSduList[pduRxId].configData.CanTpRxNSdu;
            rxRuntimeListItem = &CanTpRunTimeData.runtimeDataList[i].SimplexChnlList[CANTP_RX_CHANNEL];
            rxPhySduStateMachine(rxConfigListItem, rxRuntimeListItem);
        }

        if (pduRxFuncId != INVALID_PDU_ID)
        {
            rxConfigListItem = (CanTp_RxNSduType*)&CanTp_ConfigPtr->CanTpNSduList[pduRxFuncId].configData.CanTpRxNSdu;
            rxRuntimeListItem = &CanTpRunTimeData.runtimeDataList[i].functionalChnl;
            rxFncSduStateMachine(rxConfigListItem, rxRuntimeListItem);
         }
    }
}
