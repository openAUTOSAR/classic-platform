#include "J1939Tp.h" /** @req J1939TP0003*/
#include "CanIf.h" /** @req J1939TP0172 */
#include "J1939Tp_Internal.h"
#include "PduR.h"
#include <string.h>

/* Globally fulfilled requirements */
/** @req J1939TP0123 */
/** @req J1939TP0165 */
/** @req J1939TP0097 */
/** @req J1939TP0019 */
static J1939Tp_Internal_GlobalStateInfoType globalState = {
		.State = J1939TP_OFF,
};
static const J1939Tp_ConfigType* J1939Tp_ConfigPtr;
static J1939Tp_Internal_PgStateInfoType PgStates[J1939TP_PG_COUNT];
static J1939Tp_Internal_TxPgInfo txInfos[J1939TP_TX_PG_COUNT];
static J1939Tp_Internal_RxPgInfo rxInfos[J1939TP_RX_PG_COUNT];


/** @req J1939TP0087 */
void J1939Tp_Init(const J1939Tp_ConfigType* ConfigPtr) {
	#if (J1939TP_DEV_ERROR_DETECT == STD_ON)
	if (globalState.State == J1939TP_ON) {
		/** @req J1939TP0026 */
		J1939Tp_Internal_ReportError(J1939TP_INIT_ID, J1939TP_E_REINIT);
		return;
	}
	#endif
	for (int i = 0; i < J1939TP_PG_COUNT; i++) {
		int rxCount = 0;
		int txCount = 0;
		if (ConfigPtr->Pgs[i].Channel->Direction == J1939TP_TX) {
			PgStates[i].Tx = &(txInfos[txCount]);
			txCount++;
		} else if (ConfigPtr->Pgs[i].Channel->Direction == J1939TP_RX) {
			PgStates[i].Rx = &(rxInfos[rxCount]);
			rxCount++;
		} else {
			return; // unexpected
		}
		PgStates[i].PgConfPtr = &(ConfigPtr->Pgs[i]);
	}
	J1939Tp_ConfigPtr = ConfigPtr;
	globalState.State = J1939TP_ON; /** @req J1939TP0022 */
}

void J1939Tp_RxIndication(PduIdType RxPduId, PduInfoType* PduInfoPtr) {
	J1939Tp_Internal_PgStateInfoType* PgState = J1939Tp_Internal_GetPg(RxPduId);
	switch (PgState->PgConfPtr->Channel->Direction) {
		case J1939TP_TX:
			J1939Tp_Internal_RxIndication_TxChannel(RxPduId,PduInfoPtr);
			break;
		case J1939TP_RX:
			J1939Tp_Internal_RxIndication_RxChannel(RxPduId,PduInfoPtr);
			break;
		default:
			break;
	}
}
static inline boolean J1939Tp_Internal_IsDtPacketAlreadySent(uint8 nextPacket, uint8 totalPacketsSent) {
	return nextPacket < totalPacketsSent;
}
static void inline J1939Tp_Internal_RxIndication_TxChannel(PduIdType RxPduId, PduInfoType* PduInfoPtr) {
	J1939Tp_Internal_PgStateInfoType* PgState = J1939Tp_Internal_GetPg(RxPduId);
	uint8 NumPacketsToSend = 0;
	uint8 NextPacket = 0;

	switch (PgState->Tx->State) {
		case J1939TP_TX_WAITING_FOR_CTS:
			if (J1939Tp_Internal_CheckValidCts(PduInfoPtr,&NumPacketsToSend,&NextPacket)) {
				if (NumPacketsToSend == 0) {
					// Receiver wants to keep the connection open but cant receive packets
					/** @req J1939TP0195 */
					J1939Tp_Internal_TxSessionStartTimer(PgState->Tx,J1939TP_T4_TIMEOUT_MS);
				} else if(J1939Tp_Internal_IsDtPacketAlreadySent(NextPacket,PgState->Tx->TotalSentDtCount)) {
					/** @req J1939TP0194 */
					PduR_J1939TpTxConfirmation(PgState->PgConfPtr->NSdu,NTFRSLT_E_NOT_OK);
					J1939Tp_Internal_SendConnectionAbort(PgState->PgConfPtr->Channel->CmNPdu,PgState->PgConfPtr->Pgn);
					PgState->Tx->State = J1939TP_TX_IDLE;
				} else {
					PgState->Tx->DtToSendBeforeCtsCount = NumPacketsToSend;
					PgState->Tx->SentDtCount = 0;
					PgState->Tx->State = J1939TP_TX_WAIT_DT_CANIF_CONFIRM;
					J1939Tp_Internal_SendDt(PgState);
					J1939Tp_Internal_TxSessionStartTimer(PgState->Tx,J1939TP_TX_CONF_TIMEOUT);
				}
			}
			break;
		case J1939TP_TX_WAITING_FOR_END_OF_MSG_ACK:
			if (J1939Tp_Internal_CheckValidEndOfMsgAck(PgState,PduInfoPtr)) {
				PduR_J1939TpTxConfirmation(PgState->PgConfPtr->NSdu,NTFRSLT_OK);
				PgState->Tx->State = J1939TP_TX_IDLE;
			}
			break;
		default:
			break;
	}
}
static void inline J1939Tp_Internal_RxIndication_RxChannel(PduIdType NSduId, PduInfoType* PduInfoPtr) {
	J1939Tp_Internal_PgStateInfoType* PgState = J1939Tp_Internal_GetPg(NSduId);
	switch (PgState->Rx->State) {
		case J1939TP_RX_IDLE:
			if (J1939Tp_Internal_ReceiveRts(NSduId, PduInfoPtr) == E_OK) {
				PgState->Rx->State = J1939TP_RX_WAIT_CTS_CANIF_CONFIRM;
				J1939Tp_Internal_SendCts(PgState,PduInfoPtr);
				J1939Tp_Internal_RxSessionStartTimer(PgState->Rx,J1939TP_TX_CONF_TIMEOUT);
			}
			break;
		case J1939TP_RX_RECEIVING_DT:
			J1939Tp_Internal_ReceiveDt(NSduId,PduInfoPtr);
		default:
			break;
	}
}

static boolean inline J1939Tp_Internal_IsLastDt(J1939Tp_Internal_RxPgInfo* rxPgInfo) {
	return (rxPgInfo->TotalDtToReceiveCount == rxPgInfo->TotalReceivedDtCount);
}
static Std_ReturnType inline J1939Tp_Internal_ReceiveDt(PduIdType NSduId, PduInfoType* PduInfoPtr) {
	PduInfoType* rxPduInfo;
	J1939Tp_Internal_PgStateInfoType* PgState = J1939Tp_Internal_GetPg(NSduId);
	PduIdType PduRSdu = PgState->PgConfPtr->NSdu;
	uint8 requestSize = DT_DATA_SIZE;
	boolean lastMessage = false;
	if (J1939Tp_Internal_IsLastDt(PgState->Rx)) {
		lastMessage = true;
		J1939Tp_Internal_DtPayloadSizeType receivedBytes = PgState->Rx->TotalReceivedDtCount*DT_DATA_SIZE;
		requestSize = PgState->Rx->TotalMessageSize - receivedBytes;
		PgState->Rx->State = J1939TP_RX_IDLE;
		J1939Tp_Internal_SendEndOfMsgAck(PgState);
	}
	BufReq_ReturnType r = PduR_J1939TpProvideRxBuffer(PduRSdu, requestSize, &rxPduInfo);
	if (r == BUFREQ_OK) {
		memcpy(rxPduInfo->SduDataPtr,&(PduInfoPtr[DT_BYTE_DATA_1]),requestSize);
	}
	if (lastMessage == true) {
		PduR_J1939TpRxIndication(PduRSdu,NTFRSLT_OK);
	} else {
		J1939Tp_Internal_RxSessionStartTimer(PgState->Rx,J1939TP_T1_TIMEOUT_MS);
	}
	return E_OK;
}
static Std_ReturnType inline J1939Tp_Internal_ReceiveRts(PduIdType NSduId, PduInfoType* PduInfoPtr) {
	J1939Tp_Internal_ControlByteType controlByte;
	Std_ReturnType r = J1939Tp_Internal_GetCmControlByte(PduInfoPtr,&controlByte);
	if (r == E_NOT_OK) {
		return r;
	}
	PduLengthType* bufferSizePtr = 0;
	/** @req J1939TP0043**/
	J1939Tp_Internal_DtPayloadSizeType messageSize = 0;
	if (controlByte == RTS_CONTROL_VALUE) {
		messageSize = J1939Tp_Internal_GetRtsMessageSize(PduInfoPtr);

		/** @req J1939TP0039**/
		BufReq_ReturnType BufferRes = PduR_J1939TpStartOfReception(NSduId,messageSize,bufferSizePtr);
		if (BufferRes != BUFREQ_OK) {
			return E_OK;
		}
	}
	J1939Tp_Internal_PgStateInfoType* PgState = J1939Tp_Internal_GetPg(NSduId);
	PgState->Rx->TotalReceivedDtCount = 0;
	PgState->Rx->TotalDtToReceiveCount = PduInfoPtr->SduDataPtr[RTS_BYTE_NUM_PACKETS];
	PgState->Rx->TotalMessageSize = messageSize;
	return E_OK;
}


void J1939Tp_MainFunction(void) {
	for (int i = 0; i < J1939TP_PG_COUNT; i++) {
		J1939Tp_Internal_PgStateInfoType* PgState = J1939Tp_Internal_GetPg(i);
		J1939Tp_Internal_TimerStatusType timer = J1939TP_NOT_EXPIRED;
		switch (PgState->PgConfPtr->Channel->Direction) {
			case J1939TP_TX:
				switch (PgState->Tx->State) {
					case J1939TP_TX_WAIT_DIRECT_SEND_CANIF_CONFIRM:
					case J1939TP_TX_WAITING_FOR_CTS:
					case J1939TP_TX_WAITING_FOR_END_OF_MSG_ACK:
						timer = J1939Tp_Internal_IncAndCheckTimer(&(PgState->Tx->TimerInfo));
						break;
					default:
						break;
				}
				break;
				if (timer == J1939TP_EXPIRED) {
					PgState->Tx->State = J1939TP_TX_IDLE;
					J1939Tp_Internal_SendConnectionAbort(PgState->PgConfPtr->Channel->CmNPdu,PgState->PgConfPtr->Pgn);
					PduR_J1939TpTxConfirmation(PgState->PgConfPtr->NSdu,NTFRSLT_E_NOT_OK);
				}
				return;
			case J1939TP_RX:
				switch (PgState->Rx->State) {
					case J1939TP_RX_WAIT_CTS_CANIF_CONFIRM:
					case J1939TP_RX_RECEIVING_DT:
					case J1939TP_RX_WAIT_ENDOFMSGACK_CANIF_CONFIRM:
					timer = J1939Tp_Internal_IncAndCheckTimer(&(PgState->Rx->TimerInfo));
					default:
						break;
				}
			default:
				break;
				if (timer == J1939TP_EXPIRED) {
					PgState->Rx->State = J1939TP_TX_IDLE;
					J1939Tp_Internal_SendConnectionAbort(PgState->PgConfPtr->Channel->FcNPdu,PgState->PgConfPtr->Pgn);
					PduR_J1939TpRxIndication(PgState->PgConfPtr->NSdu,NTFRSLT_E_NOT_OK);
				}
				return;
		}
	}
}
void J1939Tp_TxConfirmation(PduIdType NSduId) {
	J1939Tp_Internal_PgStateInfoType* pgState = J1939Tp_Internal_GetPg(NSduId);
	switch (pgState->PgConfPtr->Channel->Direction) {
		case J1939TP_TX:
			J1939Tp_Internal_TxConfirmation_TxChannel(NSduId);
			break;
		case J1939TP_RX:
			J1939Tp_Internal_TxConfirmation_RxChannel(NSduId);
			break;
		default:
			break;
	}

}
static inline void J1939Tp_Internal_TxConfirmation_RxChannel(PduIdType NSduId) {
	J1939Tp_Internal_PgStateInfoType* pgState = J1939Tp_Internal_GetPg(NSduId);
	switch (pgState->Tx->State) {
		case J1939TP_RX_WAIT_CTS_CANIF_CONFIRM:
			J1939Tp_Internal_TxSessionStartTimer(pgState->Tx,J1939TP_T2_TIMEOUT_MS);
			pgState->Tx->State = J1939TP_RX_RECEIVING_DT;
			break;
		default:
			break;
	}
}
static inline void J1939Tp_Internal_TxConfirmation_TxChannel(PduIdType NSduId) {
	J1939Tp_Internal_PgStateInfoType* pgState = J1939Tp_Internal_GetPg(NSduId);
	switch (pgState->Tx->State) {
		case J1939TP_TX_WAIT_DT_CANIF_CONFIRM:
			if (J1939Tp_Internal_WaitForEndOfMsgAck(pgState->Tx)) {
				J1939Tp_Internal_TxSessionStartTimer(pgState->Tx,J1939TP_T3_TIMEOUT_MS);
				pgState->Tx->State = J1939TP_TX_WAITING_FOR_END_OF_MSG_ACK;
			} else if (J1939Tp_Internal_WaitForCts(pgState->Tx)) {
				J1939Tp_Internal_TxSessionStartTimer(pgState->Tx,J1939TP_T3_TIMEOUT_MS);
				pgState->Tx->State = J1939TP_TX_WAITING_FOR_CTS;
			} else {
				J1939Tp_Internal_TxSessionStartTimer(pgState->Tx,J1939TP_TX_CONF_TIMEOUT);
				J1939Tp_Internal_SendDt(pgState);
			}
			break;
		case J1939TP_TX_WAIT_RTS_CANIF_CONFIRM:
			pgState->Tx->State = J1939TP_TX_WAITING_FOR_CTS;
			break;
		case J1939TP_TX_WAIT_DIRECT_SEND_CANIF_CONFIRM:
			pgState->Tx->State = J1939TP_TX_IDLE;
			PduR_J1939TpTxConfirmation(pgState->PgConfPtr->NSdu, NTFRSLT_OK);

		default:
			break;
	}
}
/** @req J1939TP0180 */
Std_ReturnType J1939Tp_ChangeParameterRequest(PduIdType SduId, TPParameterType Parameter, uint16 value) {
	return E_NOT_OK; /** @req J1939TP0181 */
}

/** @req J1939TP0096 */
Std_ReturnType J1939Tp_Transmit(PduIdType TxSduId, const PduInfoType* TxInfoPtr) {
	#if J1939TP_DEV_ERROR_DETECT
	if (globalState.State == J1939TP_OFF) {
		J1939Tp_Internal_ReportError(J1939TP_TRANSMIT_ID,J1939TP_E_UNINIT);
	}
	#endif
	J1939Tp_Internal_PgStateInfoType* pgState = J1939Tp_Internal_GetPg(TxSduId);
	if (TxInfoPtr->SduLength <= 8) { // direct transmit
		PduInfoType* ToSendTxInfoPtr;
		PduR_J1939TpProvideTxBuffer(TxSduId, &ToSendTxInfoPtr, TxInfoPtr->SduLength);
		PduIdType CanIfPdu = pgState->PgConfPtr->DirectNPdu;
		pgState->Tx->State = J1939TP_TX_WAIT_DIRECT_SEND_CANIF_CONFIRM;
		CanIf_Transmit(CanIfPdu,ToSendTxInfoPtr);
	} else {
		J1939Tp_PgnType pgn = J1939Tp_Internal_ConfGetPg(TxSduId)->Pgn;
		uint8 pf = J1939Tp_Internal_GetPf(pgn);
		J1939Tp_ProtocolType protocol = J1939Tp_Internal_GetProtocol(pf);
		switch (protocol) { /** @req J1939TP0039*/
			case J1939TP_PROTOCOL_BAM:
				break;
			case J1939TP_PROTOCOL_CMDT:
				pgState->Tx->State = J1939TP_TX_WAIT_RTS_CANIF_CONFIRM;
				J1939Tp_Internal_SendRts(pgState,TxInfoPtr);
				J1939Tp_Internal_TxSessionStartTimer(pgState->Tx,J1939TP_TX_CONF_TIMEOUT);
				J1939Tp_Internal_PgStateInfoType* txPgState = J1939Tp_Internal_GetPg(TxSduId);
				J1939Tp_Internal_ResetTimer(txPgState);
				txPgState->Tx->TotalMessageSize = TxInfoPtr->SduLength;
				txPgState->Tx->TotalBytesSent = 0;
				txPgState->Tx->TotalSentDtCount = 0;
				txPgState->Tx->PduRPdu = TxSduId;
				break;
		}
	}
	return E_OK;
}
static inline uint16 J1939Tp_Internal_GetRtsMessageSize(PduInfoType* pduInfo) {
	return (((uint16)pduInfo->SduDataPtr[RTS_BYTE_LENGTH_1]) << 8) | pduInfo->SduDataPtr[RTS_BYTE_LENGTH_1];
}
static inline Std_ReturnType J1939Tp_Internal_GetCmControlByte(PduInfoType* pduInfo, J1939Tp_Internal_ControlByteType* controlByte) {
	if (pduInfo->SduLength != 8) {
		return E_NOT_OK;
	}
	else {
		*controlByte = pduInfo->SduDataPtr[0];
		return E_OK;
	}
}
static inline const J1939Tp_PgType* J1939Tp_Internal_ConfGetPg(PduIdType NSduId) {
	return &(J1939Tp_ConfigPtr->Pgs[NSduId]);
}
static inline boolean J1939Tp_Internal_WaitForCts(J1939Tp_Internal_TxPgInfo* TxPgState) {
	return TxPgState->SentDtCount == TxPgState->DtToSendBeforeCtsCount;
}

static inline boolean J1939Tp_Internal_WaitForEndOfMsgAck(J1939Tp_Internal_TxPgInfo* TxPgState) {
	return TxPgState->TotalMessageSize == TxPgState->TotalBytesSent;
}


static inline J1939Tp_Internal_PgStateInfoType* J1939Tp_Internal_GetPg(PduIdType NSduId) {
	return &(PgStates[NSduId]);
}
static inline const J1939Tp_ChannelType* J1939Tp_Internal_ConfGetTxChannel(PduIdType NSduId) {
	return J1939Tp_ConfigPtr->Pgs[NSduId].Channel;
}



static inline boolean J1939Tp_Internal_CheckValidEndOfMsgAck(J1939Tp_Internal_PgStateInfoType* pgState, PduInfoType* PduInfoPtr) {
	if (PduInfoPtr->SduLength != ENDOFMSGACK_SIZE) {
		return false;
	}
	if (PduInfoPtr->SduDataPtr[ENDOFMSGACK_BYTE_CONTROL] != ENDOFMSGACK_CONTROL_VALUE) {
		return false;
	}
	return true;
}
static inline J1939Tp_Internal_TimerStatusType J1939Tp_Internal_IncAndCheckTimer(J1939Tp_Internal_TimerType* TimerInfo) {
	TimerInfo->Timer += J1939TP_MAIN_FUNCTION_PERIOD;
	if (TimerInfo->Timer >= TimerInfo->TimerExpire) {
		return J1939TP_EXPIRED;
	}
	return J1939TP_NOT_EXPIRED;
}
static uint8 J1939Tp_Internal_GetPf(J1939Tp_PgnType pgn) {
	return (pgn && 0x0000FF00) >> 8;
}
static J1939Tp_ProtocolType J1939Tp_Internal_GetProtocol(uint8 pf) {
	if (pf < 240) {
		return J1939TP_PROTOCOL_CMDT;
	} else {
		return J1939TP_PROTOCOL_BAM;
	}
}
static inline boolean J1939Tp_Internal_CheckValidCts(PduInfoType* PduInfoPtr,uint8* NumPackets, uint8* NextPacket) {
	/*
	if (PduInfoPtr->SduLength != CTS_SIZE) {
		return false;
	}
	*/
	if (PduInfoPtr->SduDataPtr[CTS_BYTE_CONTROL] != CTS_CONTROL_VALUE) {
		return false;
	}

	*NumPackets = PduInfoPtr->SduDataPtr[CTS_BYTE_NUM_PACKETS];
	*NextPacket = PduInfoPtr->SduDataPtr[CTS_BYTE_NEXT_PACKET];
	return true;
}

static void inline J1939Tp_Internal_SendDt(J1939Tp_Internal_PgStateInfoType* PgState) {
	uint8 requestLength = DT_DATA_SIZE;
	uint8 bytesLeftToSend = PgState->Tx->TotalMessageSize - PgState->Tx->TotalSentDtCount * DT_DATA_SIZE;
	if (bytesLeftToSend < DT_DATA_SIZE){
		requestLength = bytesLeftToSend;
	}
	// prepare dt message
	uint8 dtBuffer[DT_SIZE] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	PduInfoType dtPduInfoBuffer;
	dtPduInfoBuffer.SduLength = DT_SIZE;
	dtPduInfoBuffer.SduDataPtr = dtBuffer;

	BufReq_ReturnType allocateBufferRes;
	PduInfoType* dataPduInfoBuffer;
	PduIdType Pdur_NSdu = PgState->PgConfPtr->NSdu;
	allocateBufferRes = PduR_J1939TpProvideTxBuffer(Pdur_NSdu, &dataPduInfoBuffer, requestLength);
	if (allocateBufferRes == BUFREQ_OK) {
		dtPduInfoBuffer.SduDataPtr[DT_BYTE_SEQ_NUM] = PgState->Tx->TotalSentDtCount;
		memcpy(&(dtPduInfoBuffer.SduDataPtr[DT_BYTE_DATA_1]), dataPduInfoBuffer, requestLength);
		PduIdType CanIf_NSdu = PgState->PgConfPtr->Channel->DtNPdu;
		CanIf_Transmit(CanIf_NSdu, &dtPduInfoBuffer);
		PgState->Tx->TotalBytesSent += requestLength;
		PgState->Tx->TotalSentDtCount++;
		PgState->Tx->SentDtCount++;
	} else {
		/* Todo: check for error */
	}

}

static void J1939Tp_Internal_SendRts(J1939Tp_Internal_PgStateInfoType* PgState, const PduInfoType* TxInfoPtr) {
	uint8 cmRtsData[RTS_SIZE];
	cmRtsData[RTS_BYTE_CONTROL] = 16;
	cmRtsData[RTS_BYTE_LENGTH_1] = (uint8)(TxInfoPtr->SduLength & 0x00FF);
	cmRtsData[RTS_BYTE_LENGTH_2] = (uint8)(TxInfoPtr->SduLength & 0xFF00);
	uint8 reminder = 0;
	if (TxInfoPtr->SduLength % J1939TP_PACKET_SIZE != 0) {
		reminder = 1;
	}
	J1939Tp_PgnType pgn = PgState->PgConfPtr->Pgn;
	cmRtsData[RTS_BYTE_NUM_PACKETS] = (TxInfoPtr->SduLength / J1939TP_PACKET_SIZE) + reminder;
	cmRtsData[RTS_BYTE_SAE_ASSIGN] = 0xFF;
	/* PGN is 18 bits, RTS message have 24 bits PGN payload, therefore we squeeze in pdu id */
	J1939Tp_Internal_SetPgn(&(cmRtsData[RTS_BYTE_PGN_1]),pgn);
	//cmRtsData[RTS_BYTE_PGN_3] = cmRtsData[RTS_BYTE_PGN_3] & ((TxSduId << 2) | 0x03); /* add the sdu id to the remaining 4 bits*/

	PduInfoType cmRtsPdu;
	cmRtsPdu.SduLength = RTS_SIZE;
	cmRtsPdu.SduDataPtr = cmRtsData;

	CanIf_Transmit(PgState->PgConfPtr->Channel->CmNPdu,&cmRtsPdu);
}

static void inline J1939Tp_Internal_SendEndOfMsgAck(J1939Tp_Internal_PgStateInfoType* PgState) {
	PduInfoType endofmsgInfo;
	uint8 endofmsgData[ENDOFMSGACK_SIZE];
	endofmsgInfo.SduLength = ENDOFMSGACK_SIZE;
	endofmsgInfo.SduDataPtr = endofmsgData;
	endofmsgData[ENDOFMSGACK_BYTE_CONTROL] = ENDOFMSGACK_CONTROL_VALUE;
	endofmsgData[ENDOFMSGACK_BYTE_TOTAL_MSG_SIZE_1] = ((uint8)(PgState->Rx->TotalMessageSize)) << 8;
	endofmsgData[ENDOFMSGACK_BYTE_TOTAL_MSG_SIZE_2] = ((uint8)(PgState->Rx->TotalMessageSize));
	endofmsgData[ENDOFMSGACK_BYTE_NUM_PACKETS] = PgState->Rx->TotalReceivedDtCount;
	endofmsgData[ENDOFMSGACK_BYTE_SAE_ASSIGN] = 0xFF;
	J1939Tp_Internal_SetPgn(&(endofmsgData[ENDOFMSGACK_BYTE_PGN_1]),PgState->PgConfPtr->Pgn);
	PduIdType CmNPdu = PgState->PgConfPtr->Channel->CmNPdu;

	CanIf_Transmit(CmNPdu,&endofmsgInfo);
}

/**
 * Send a response to the incoming RTS
 * @param NSduId
 * @param RtsPduInfoPtr needs to be a valid RTS message
 */
static inline void J1939Tp_Internal_SendCts(J1939Tp_Internal_PgStateInfoType* PgState, PduInfoType* RtsPduInfoPtr) {
	PduInfoType ctsInfo;
	uint8 ctsData[CTS_SIZE];
	ctsInfo.SduLength = CTS_SIZE;
	ctsInfo.SduDataPtr = ctsData;
	ctsData[CTS_BYTE_CONTROL] = CTS_CONTROL_VALUE;
	ctsData[CTS_BYTE_NUM_PACKETS] = J1939TP_PACKETS_PER_BLOCK;
	ctsData[CTS_BYTE_NEXT_PACKET] = CTS_START_SEQ_NUM;
	ctsData[CTS_BYTE_SAE_ASSIGN_1] = 0xFF;
	ctsData[CTS_BYTE_SAE_ASSIGN_2] = 0xFF;
	ctsData[CTS_BYTE_PGN_1] = RtsPduInfoPtr->SduDataPtr[RTS_BYTE_PGN_1];
	ctsData[CTS_BYTE_PGN_2] = RtsPduInfoPtr->SduDataPtr[RTS_BYTE_PGN_2];
	ctsData[CTS_BYTE_PGN_3] = RtsPduInfoPtr->SduDataPtr[RTS_BYTE_PGN_3];
	PduIdType CmNPdu = PgState->PgConfPtr->Channel->CmNPdu;

	CanIf_Transmit(CmNPdu,&ctsInfo);

}
static inline void J1939Tp_Internal_SendConnectionAbort(PduIdType CmNPdu, J1939Tp_PgnType Pgn) {
	PduInfoType connAbortInfo;
	uint8 connAbortData[CONNABORT_SIZE];
	connAbortInfo.SduLength = CONNABORT_SIZE;
	connAbortInfo.SduDataPtr = connAbortData;
	connAbortData[CONNABORT_BYTE_CONTROL] = CTS_CONTROL_VALUE;
	connAbortData[CONNABORT_BYTE_SAE_ASSIGN_1] = 0xFF;
	connAbortData[CONNABORT_BYTE_SAE_ASSIGN_2] = 0xFF;
	connAbortData[CONNABORT_BYTE_SAE_ASSIGN_3] = 0xFF;
	connAbortData[CONNABORT_BYTE_SAE_ASSIGN_4] = 0xFF;
	J1939Tp_Internal_SetPgn(&(connAbortData[CONNABORT_BYTE_PGN_1]),Pgn);
	CanIf_Transmit(CmNPdu,&connAbortInfo);
}
static void inline J1939Tp_Internal_TxSessionStartTimer(J1939Tp_Internal_TxPgInfo* Tx,uint16 TimerExpire) {
	Tx->TimerInfo.Timer = 0;
	Tx->TimerInfo.TimerExpire = TimerExpire;
}
static void inline J1939Tp_Internal_RxSessionStartTimer(J1939Tp_Internal_RxPgInfo* Rx,uint16 TimerExpire) {
	Rx->TimerInfo.Timer = 0;
	Rx->TimerInfo.TimerExpire = TimerExpire;
}
static inline void J1939Tp_Internal_SetPgn(uint8* PgnBytes,J1939Tp_PgnType pgn ) {
	PgnBytes[RTS_BYTE_PGN_1] = pgn; /* get first byte */
	PgnBytes[RTS_BYTE_PGN_2] = pgn >> 8; /* get next byte */
	PgnBytes[RTS_BYTE_PGN_3] = (pgn >> 16) & 0x3; /* get next two bits */
}
static inline void J1939Tp_Internal_ResetTimer(J1939Tp_Internal_PgStateInfoType* pgState) {
	pgState->Tx->TimerInfo.Timer = 0;
}

static inline void J1939Tp_Internal_ReportError(uint8 ApiId, uint8 ErrorId) {
#if (CANSM_DEV_ERROR_DETECT == STD_ON)
	Det_ReportError(MODULE_ID_J1939TP, 0, ApiId, ApiId);
#endif
}
