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
static J1939Tp_Internal_ChannelInfoType channelInfos[J1939TP_CHANNEL_COUNT];
static J1939Tp_Internal_TxChannelInfoType txChannelInfos[J1939TP_TX_CHANNEL_COUNT];
static J1939Tp_Internal_RxChannelInfoType rxChannelInfos[J1939TP_RX_CHANNEL_COUNT];
static J1939Tp_Internal_PgInfoType pgInfos[J1939TP_PG_COUNT];

/** @req J1939TP0087 */
void J1939Tp_Init(const J1939Tp_ConfigType* ConfigPtr) {
	#if (J1939TP_DEV_ERROR_DETECT == STD_ON)
	if (globalState.State == J1939TP_ON) {
		/** @req J1939TP0026 */
		J1939Tp_Internal_ReportError(J1939TP_INIT_ID, J1939TP_E_REINIT);
		return;
	}
	#endif
	for (int i = 0; i < J1939TP_CHANNEL_COUNT; i++) {
		int rxCount = 0;
		int txCount = 0;
		channelInfos[i].ChannelConfPtr = &(ConfigPtr->Channels[i]);
		if (ConfigPtr->Channels[i].Direction == J1939TP_TX) {
			channelInfos[i].TxState = &(txChannelInfos[txCount]);
			channelInfos[i].RxState = 0;
			txCount++;
		} else if (ConfigPtr->Channels[i].Direction == J1939TP_RX) {
			channelInfos[i].TxState = 0;
			channelInfos[i].RxState = &(rxChannelInfos[rxCount]);
			rxCount++;
		} else {
			return; // unexpected
		}
	}
	for (int i = 0; i < J1939TP_PG_COUNT; i++) {
		pgInfos[i].PgConfPtr = &(ConfigPtr->Pgs[i]);
		uint8 channelIndex = ConfigPtr->Pgs[i].Channel - ConfigPtr->Channels;
		pgInfos[i].ChannelInfoPtr = &(channelInfos[channelIndex]);

	}
	J1939Tp_ConfigPtr = ConfigPtr;
	globalState.State = J1939TP_ON; /** @req J1939TP0022 */
}

void J1939Tp_RxIndication(PduIdType RxPduId, PduInfoType* PduInfoPtr) {
	const J1939Tp_RxPduInfoType* RxPduInfo = 0;
	if (J1939Tp_Internal_GetRxPduInfo(RxPduId, &RxPduInfo) != E_OK) {
		return;
	}
	/*if (J1939Tp_Internal_ValidatePacketType(RxPduInfo) == E_NOT_OK) {
		return;
	}*/
	J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr = J1939Tp_Internal_GetChannelState(RxPduInfo);
	switch (RxPduInfo->PacketType ) {
		case J1939TP_REVERSE_CM:
			J1939Tp_Internal_RxIndication_ReverseCm(PduInfoPtr,ChannelInfoPtr);
			break;
		case J1939TP_DT:
			J1939Tp_Internal_RxIndication_Dt(PduInfoPtr,ChannelInfoPtr);
			break;
		case J1939TP_CM:
			J1939Tp_Internal_RxIndication_Cm(PduInfoPtr,ChannelInfoPtr);
			break;
		default:
			return;
	}
}


void J1939Tp_MainFunction(void) {
	for (int i = 0; i < J1939TP_CHANNEL_COUNT; i++) {
		J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr = &(channelInfos[i]);
		const J1939Tp_ChannelType* Channel = ChannelInfoPtr->ChannelConfPtr;
		J1939Tp_Internal_TimerStatusType timer = J1939TP_NOT_EXPIRED;
		switch (Channel->Direction) {
			case J1939TP_TX:
				switch (ChannelInfoPtr->TxState->State) {
					case J1939TP_TX_WAIT_DIRECT_SEND_CANIF_CONFIRM:
					case J1939TP_TX_WAITING_FOR_CTS:
					case J1939TP_TX_WAITING_FOR_END_OF_MSG_ACK:
						timer = J1939Tp_Internal_IncAndCheckTimer(&(ChannelInfoPtr->TxState->TimerInfo));
						break;
					default:
						break;
				}
				break;
				if (timer == J1939TP_EXPIRED) {
					ChannelInfoPtr->TxState->State = J1939TP_TX_IDLE;
					if (ChannelInfoPtr->TxState->CurrentPgPtr != 0) {
						J1939Tp_Internal_SendConnectionAbort(Channel->CmNPdu,ChannelInfoPtr->TxState->CurrentPgPtr->Pgn);
						PduR_J1939TpTxConfirmation(ChannelInfoPtr->TxState->CurrentPgPtr->NSdu,NTFRSLT_E_NOT_OK);
					}
				}
				return;
			case J1939TP_RX:
				switch (ChannelInfoPtr->RxState->State) {
					case J1939TP_RX_WAIT_CTS_CANIF_CONFIRM:
					case J1939TP_RX_RECEIVING_DT:
					case J1939TP_RX_WAIT_ENDOFMSGACK_CANIF_CONFIRM:
					timer = J1939Tp_Internal_IncAndCheckTimer(&(ChannelInfoPtr->RxState->TimerInfo));
					default:
						break;
				}
			default:
				break;
				if (timer == J1939TP_EXPIRED) {
					ChannelInfoPtr->RxState->State = J1939TP_TX_IDLE;
					J1939Tp_Internal_SendConnectionAbort(Channel->FcNPdu,ChannelInfoPtr->TxState->CurrentPgPtr->Pgn);
					PduR_J1939TpRxIndication(ChannelInfoPtr->RxState->CurrentPgPtr->NSdu,NTFRSLT_E_NOT_OK);
				}
				return;
		}
	}
}
void J1939Tp_TxConfirmation(PduIdType RxPdu) {
	const J1939Tp_RxPduInfoType* RxPduInfo = 0;
	if (J1939Tp_Internal_GetRxPduInfo(RxPdu, &RxPduInfo) != E_OK) {
		return;
	}

	const J1939Tp_ChannelType* Channel = J1939Tp_Internal_GetChannel(RxPduInfo);
	J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr = J1939Tp_Internal_GetChannelState(RxPduInfo);
	switch (Channel->Direction) {
		case J1939TP_TX:
			J1939Tp_Internal_TxConfirmation_TxChannel(ChannelInfoPtr, RxPduInfo);
			break;
		case J1939TP_RX:
			J1939Tp_Internal_TxConfirmation_RxChannel(ChannelInfoPtr, RxPduInfo);
			break;
		default:
			break;
	}

}
static inline const J1939Tp_ChannelType* J1939Tp_Internal_GetChannel(const J1939Tp_RxPduInfoType* RxPduInfo) {
	return &(J1939Tp_ConfigPtr->Channels[RxPduInfo->ChannelIndex]);
}
static inline J1939Tp_Internal_ChannelInfoType* J1939Tp_Internal_GetChannelState(const J1939Tp_RxPduInfoType* RxPduInfo) {
	return &(channelInfos[RxPduInfo->ChannelIndex]);
}
static inline Std_ReturnType J1939Tp_Internal_ValidatePacketType(const J1939Tp_RxPduInfoType* RxPduInfo) {
	const J1939Tp_ChannelType* ChannelPtr = J1939Tp_Internal_GetChannel(RxPduInfo);
	switch (RxPduInfo->PacketType) {
		case J1939TP_REVERSE_CM:
			if (ChannelPtr->Direction != J1939TP_TX ) {
				return E_NOT_OK;
			}
			break;
		case J1939TP_DT:
			if (ChannelPtr->Direction != J1939TP_RX) {
				return E_NOT_OK;
			}
			break;
		case J1939TP_CM:
			if (ChannelPtr->Direction != J1939TP_RX) {
				return E_NOT_OK;
			}
			break;
		default:
			return E_NOT_OK;
	}
	return E_OK;
}

static inline void J1939Tp_Internal_RxIndication_Dt(PduInfoType* PduInfoPtr, J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr) {
	PduInfoType* rxPduInfo;
	PduIdType PduRSdu = ChannelInfoPtr->RxState->CurrentPgPtr->NSdu;
	uint8 requestSize = DT_DATA_SIZE;
	boolean lastMessage = false;
	if (J1939Tp_Internal_IsLastDt(ChannelInfoPtr->RxState)) {
		lastMessage = true;
		J1939Tp_Internal_DtPayloadSizeType receivedBytes = ChannelInfoPtr->RxState->TotalReceivedDtCount*DT_DATA_SIZE;
		requestSize = ChannelInfoPtr->RxState->TotalMessageSize - receivedBytes;
		ChannelInfoPtr->RxState->State = J1939TP_RX_IDLE;
		J1939Tp_Internal_SendEndOfMsgAck(ChannelInfoPtr);
	}
	BufReq_ReturnType r = PduR_J1939TpProvideRxBuffer(PduRSdu, requestSize, &rxPduInfo);
	if (r == BUFREQ_OK) {
		memcpy(rxPduInfo->SduDataPtr,&(PduInfoPtr[DT_BYTE_DATA_1]),requestSize);
	}
	if (lastMessage == true) {
		PduR_J1939TpRxIndication(PduRSdu,NTFRSLT_OK);
	} else {
		J1939Tp_Internal_RxSessionStartTimer(ChannelInfoPtr->RxState,J1939TP_T1_TIMEOUT_MS);
	}
}
static inline void J1939Tp_Internal_RxIndication_Cm(PduInfoType* PduInfoPtr, J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr) {
	const J1939Tp_PgType* pg = 0;
	J1939Tp_PgnType pgn = J1939Tp_Internal_GetPgn(&(PduInfoPtr->SduDataPtr[CM_PGN_BYTE_1]));
	if (J1939Tp_Internal_GetPgFromPgn(ChannelInfoPtr->ChannelConfPtr,pgn,&pg) != E_OK) {
		return;
	}
	uint8 Command = PduInfoPtr->SduDataPtr[CM_BYTE_CONTROL];
	switch (Command) {
		case RTS_CONTROL_VALUE:
			if (ChannelInfoPtr->RxState->State == J1939TP_RX_IDLE) {
				/** @req J1939TP0043**/
				J1939Tp_Internal_DtPayloadSizeType messageSize = 0;
				messageSize = J1939Tp_Internal_GetRtsMessageSize(PduInfoPtr);
					/** @req J1939TP0039**/
				PduLengthType* bufferSizePtr = 0;
				BufReq_ReturnType BufferRes = PduR_J1939TpStartOfReception(pg->NSdu,messageSize,bufferSizePtr);
				if (BufferRes != BUFREQ_OK) {
					return;
				}
				ChannelInfoPtr->RxState->TotalReceivedDtCount = 0;
				ChannelInfoPtr->RxState->TotalDtToReceiveCount = PduInfoPtr->SduDataPtr[RTS_BYTE_NUM_PACKETS];
				ChannelInfoPtr->RxState->TotalMessageSize = messageSize;
				ChannelInfoPtr->RxState->State = J1939TP_RX_WAIT_CTS_CANIF_CONFIRM;
				ChannelInfoPtr->RxState->CurrentPgPtr = pg;
				J1939Tp_Internal_SendCts(ChannelInfoPtr,PduInfoPtr);
				J1939Tp_Internal_RxSessionStartTimer(ChannelInfoPtr->RxState,J1939TP_TX_CONF_TIMEOUT);
			}
			break;
		default:
			break;
	}
}
static inline void J1939Tp_Internal_RxIndication_ReverseCm(PduInfoType* PduInfoPtr, J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr) {
	const J1939Tp_PgType* pg = 0;
	J1939Tp_PgnType pgn = J1939Tp_Internal_GetPgn(&(PduInfoPtr->SduDataPtr[CM_PGN_BYTE_1]));
	if (J1939Tp_Internal_GetPgFromPgn(ChannelInfoPtr->ChannelConfPtr,pgn,&pg) != E_OK) {
		return;
	}
	uint8 Command = PduInfoPtr->SduDataPtr[CM_BYTE_CONTROL];
	switch (Command) {
		case CTS_CONTROL_VALUE:
			if (ChannelInfoPtr->TxState->State == J1939TP_TX_WAITING_FOR_CTS) {
				uint8 NumPacketsToSend = PduInfoPtr->SduDataPtr[CTS_BYTE_NUM_PACKETS];
				uint8 NextPacket = PduInfoPtr->SduDataPtr[CTS_BYTE_NEXT_PACKET];
				if (NumPacketsToSend == 0) {
					// Receiver wants to keep the connection open but cant receive packets
					/** @req J1939TP0195 */
					J1939Tp_Internal_TxSessionStartTimer(ChannelInfoPtr->TxState,J1939TP_T4_TIMEOUT_MS);
				} else if(J1939Tp_Internal_IsDtPacketAlreadySent(NextPacket,ChannelInfoPtr->TxState->TotalSentDtCount)) {
					/** @req J1939TP0194 */
					PduR_J1939TpTxConfirmation(pg->NSdu,NTFRSLT_E_NOT_OK);
					J1939Tp_Internal_SendConnectionAbort(ChannelInfoPtr->ChannelConfPtr->CmNPdu,pgn);
					ChannelInfoPtr->TxState->State = J1939TP_TX_IDLE;
				} else {
					ChannelInfoPtr->TxState->DtToSendBeforeCtsCount = NumPacketsToSend;
					ChannelInfoPtr->TxState->SentDtCount = 0;
					ChannelInfoPtr->TxState->State = J1939TP_TX_WAIT_DT_CANIF_CONFIRM;
					J1939Tp_Internal_SendDt(ChannelInfoPtr);
					J1939Tp_Internal_TxSessionStartTimer(ChannelInfoPtr->TxState,J1939TP_TX_CONF_TIMEOUT);
				}
			}
			break;
		case ENDOFMSGACK_CONTROL_VALUE:
			if (ChannelInfoPtr->TxState->State == J1939TP_TX_WAITING_FOR_END_OF_MSG_ACK) {
				PduR_J1939TpTxConfirmation(pg->NSdu,NTFRSLT_OK);
				ChannelInfoPtr->TxState->State = J1939TP_TX_IDLE;
			}
			break;
		case CONNABORT_CONTROL_VALUE:
			PduR_J1939TpTxConfirmation(pg->NSdu,NTFRSLT_E_NOT_OK);
			ChannelInfoPtr->TxState->State = J1939TP_TX_IDLE;
			break;
		default:
			break;
	}
}

/** @req J1939TP0180 */
Std_ReturnType J1939Tp_ChangeParameterRequest(PduIdType SduId, TPParameterType Parameter, uint16 value) {
	return E_NOT_OK; /** @req J1939TP0181 */
}
static inline void J1939Tp_Internal_TxConfirmation_RxChannel(J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr, const J1939Tp_RxPduInfoType* RxPduInfo) {
	switch (RxPduInfo->PacketType ) {
		case J1939TP_REVERSE_CM:
			if (ChannelInfoPtr->RxState->State == J1939TP_RX_WAIT_CTS_CANIF_CONFIRM) {
				J1939Tp_Internal_RxSessionStartTimer(ChannelInfoPtr->RxState,J1939TP_T2_TIMEOUT_MS);
				ChannelInfoPtr->RxState->State = J1939TP_RX_RECEIVING_DT;
			}
			break;
		default:
			break;
	}
}
static inline boolean J1939Tp_Internal_IsDtPacketAlreadySent(uint8 nextPacket, uint8 totalPacketsSent) {
	return nextPacket < totalPacketsSent;
}
static inline Std_ReturnType J1939Tp_Internal_GetRxPduInfo(PduIdType RxPdu,const const J1939Tp_RxPduInfoType** RxPduInfo) {
	if (RxPdu < J1939TP_RX_PDU_COUNT) {
		*RxPduInfo = &(J1939Tp_ConfigPtr->RxPdus[RxPdu]);
		return E_OK;
	} else {
		return E_NOT_OK;
	}

}
static inline Std_ReturnType J1939Tp_Internal_GetPgFromPgn(const J1939Tp_ChannelType* channel, J1939Tp_Internal_PgnType Pgn, const J1939Tp_PgType** Pg) {
	for (int i = 0; i < channel->PgCount; i++) {
		if (channel->Pgs[i]->Pgn == Pgn) {
			*Pg = channel->Pgs[i];
			return E_OK;
		}
	}
	return E_NOT_OK;
}



static inline boolean J1939Tp_Internal_IsLastDt(J1939Tp_Internal_RxChannelInfoType* rxChannelInfo) {
	return (rxChannelInfo->TotalDtToReceiveCount == rxChannelInfo->TotalReceivedDtCount);
}
static inline void J1939Tp_Internal_TxConfirmation_TxChannel(J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr, const J1939Tp_RxPduInfoType* RxPduInfo) {
	switch (RxPduInfo->PacketType ) {
		case J1939TP_REVERSE_CM:
			break;
		case J1939TP_CM:
			if (ChannelInfoPtr->TxState->State == J1939TP_TX_WAIT_RTS_CANIF_CONFIRM) {
				ChannelInfoPtr->TxState->State = J1939TP_TX_WAITING_FOR_CTS;
			}
			break;
		case J1939TP_DT:
			if (ChannelInfoPtr->TxState->State == J1939TP_TX_WAIT_DT_CANIF_CONFIRM) {
				ChannelInfoPtr->TxState->TotalSentDtCount++;
				ChannelInfoPtr->TxState->SentDtCount++;
				if (J1939Tp_Internal_WaitForEndOfMsgAck(ChannelInfoPtr->TxState)) {
					J1939Tp_Internal_TxSessionStartTimer(ChannelInfoPtr->TxState,J1939TP_T3_TIMEOUT_MS);
					ChannelInfoPtr->TxState->State = J1939TP_TX_WAITING_FOR_END_OF_MSG_ACK;
				} else if (J1939Tp_Internal_WaitForCts(ChannelInfoPtr->TxState)) {
					J1939Tp_Internal_TxSessionStartTimer(ChannelInfoPtr->TxState,J1939TP_T3_TIMEOUT_MS);
					ChannelInfoPtr->TxState->SentDtCount = 0;
					ChannelInfoPtr->TxState->State = J1939TP_TX_WAITING_FOR_CTS;
				} else {
					J1939Tp_Internal_TxSessionStartTimer(ChannelInfoPtr->TxState,J1939TP_TX_CONF_TIMEOUT);
					J1939Tp_Internal_SendDt(ChannelInfoPtr);
				}
			}
			break;
		case J1939TP_DIRECT:
			if (ChannelInfoPtr->TxState->State == J1939TP_TX_WAIT_DIRECT_SEND_CANIF_CONFIRM) {
				ChannelInfoPtr->TxState->State = J1939TP_TX_IDLE;
				PduR_J1939TpTxConfirmation(ChannelInfoPtr->TxState->CurrentPgPtr->NSdu, NTFRSLT_OK);
			}
			break;
		default:
			break;
	}
}
static inline Std_ReturnType J1939Tp_Internal_GetPg(PduIdType TxSduId,J1939Tp_Internal_PgInfoType** PgInfo) {
	if (TxSduId < J1939TP_PG_COUNT) {
		*PgInfo = &(pgInfos[TxSduId]);
		return E_OK;
	}
	return E_NOT_OK;
}

/** @req J1939TP0096 */
Std_ReturnType J1939Tp_Transmit(PduIdType TxSduId, const PduInfoType* TxInfoPtr) {
	#if J1939TP_DEV_ERROR_DETECT
	if (globalState.State == J1939TP_OFF) {
		J1939Tp_Internal_ReportError(J1939TP_TRANSMIT_ID,J1939TP_E_UNINIT);
	}
	#endif
	J1939Tp_Internal_PgInfoType* PgInfo;
	//PgInfo = pgInfos;
	if (J1939Tp_Internal_GetPg(TxSduId,&PgInfo) == E_NOT_OK) {
		return E_NOT_OK;
	}
	const J1939Tp_PgType* Pg = PgInfo->PgConfPtr;
	J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr = PgInfo->ChannelInfoPtr;
	if (TxInfoPtr->SduLength <= 8) { // direct transmit
		PduInfoType* ToSendTxInfoPtr;
		PduR_J1939TpProvideTxBuffer(Pg->NSdu, &ToSendTxInfoPtr, TxInfoPtr->SduLength);
		PduIdType CanIfPdu = Pg->DirectNPdu;
		ChannelInfoPtr->TxState->State = J1939TP_TX_WAIT_DIRECT_SEND_CANIF_CONFIRM;
		ChannelInfoPtr->TxState->CurrentPgPtr = Pg;
		CanIf_Transmit(CanIfPdu,ToSendTxInfoPtr);
	} else {
		uint8 pf = J1939Tp_Internal_GetPf(Pg->Pgn);
		J1939Tp_ProtocolType protocol = J1939Tp_Internal_GetProtocol(pf);
		switch (protocol) { /** @req J1939TP0039*/
			case J1939TP_PROTOCOL_BAM:
				break;
			case J1939TP_PROTOCOL_CMDT:
				ChannelInfoPtr->TxState->State = J1939TP_TX_WAIT_RTS_CANIF_CONFIRM;
				ChannelInfoPtr->TxState->TotalMessageSize = TxInfoPtr->SduLength;
				ChannelInfoPtr->TxState->TotalSentDtCount = 0;
				ChannelInfoPtr->TxState->PduRPdu = TxSduId;
				ChannelInfoPtr->TxState->CurrentPgPtr = Pg;
				J1939Tp_Internal_SendRts(ChannelInfoPtr,TxInfoPtr);
				J1939Tp_Internal_TxSessionStartTimer(ChannelInfoPtr->TxState,J1939TP_TX_CONF_TIMEOUT);
				break;
		}
	}
	return E_OK;
}
static inline uint16 J1939Tp_Internal_GetRtsMessageSize(PduInfoType* pduInfo) {
	return (((uint16)pduInfo->SduDataPtr[RTS_BYTE_LENGTH_1]) << 8) | pduInfo->SduDataPtr[RTS_BYTE_LENGTH_1];
}

static inline boolean J1939Tp_Internal_WaitForCts(J1939Tp_Internal_TxChannelInfoType* TxChannelState) {
	return TxChannelState->SentDtCount == TxChannelState->DtToSendBeforeCtsCount;
}

static inline boolean J1939Tp_Internal_WaitForEndOfMsgAck(J1939Tp_Internal_TxChannelInfoType* TxChannelState) {
	uint8 packetsToSend = 0;
	packetsToSend = (TxChannelState->TotalMessageSize)/DT_DATA_SIZE;
	if (TxChannelState->TotalMessageSize % DT_DATA_SIZE != 0) {
		packetsToSend = packetsToSend + 1;
	}
	return packetsToSend == TxChannelState->TotalSentDtCount;
}

static inline Std_ReturnType J1939Tp_Internal_ConfGetPg(PduIdType NSduId, const J1939Tp_PgType* Pg) {
	if (NSduId < J1939TP_PG_COUNT) {
		Pg = &(J1939Tp_ConfigPtr->Pgs[NSduId]);
		return E_OK;
	} else {
		return E_NOT_OK;
	}
}

static inline J1939Tp_Internal_TimerStatusType J1939Tp_Internal_IncAndCheckTimer(J1939Tp_Internal_TimerType* TimerInfo) {
	TimerInfo->Timer += J1939TP_MAIN_FUNCTION_PERIOD;
	if (TimerInfo->Timer >= TimerInfo->TimerExpire) {
		return J1939TP_EXPIRED;
	}
	return J1939TP_NOT_EXPIRED;
}
static inline uint8 J1939Tp_Internal_GetPf(J1939Tp_PgnType pgn) {
	return (pgn && 0x0000FF00) >> 8;
}
static J1939Tp_ProtocolType J1939Tp_Internal_GetProtocol(uint8 pf) {
	if (pf < 240) {
		return J1939TP_PROTOCOL_CMDT;
	} else {
		return J1939TP_PROTOCOL_BAM;
	}
}


static inline void J1939Tp_Internal_SendDt(J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr) {
	uint8 requestLength = DT_DATA_SIZE;
	uint8 bytesLeftToSend = ChannelInfoPtr->TxState->TotalMessageSize - ChannelInfoPtr->TxState->TotalSentDtCount * DT_DATA_SIZE;
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
	PduIdType Pdur_NSdu = ChannelInfoPtr->TxState->CurrentPgPtr->NSdu;
	allocateBufferRes = PduR_J1939TpProvideTxBuffer(Pdur_NSdu, &dataPduInfoBuffer, requestLength);
	if (allocateBufferRes == BUFREQ_OK) {
		dtPduInfoBuffer.SduDataPtr[DT_BYTE_SEQ_NUM] = ChannelInfoPtr->TxState->TotalSentDtCount;
		memcpy(&(dtPduInfoBuffer.SduDataPtr[DT_BYTE_DATA_1]), dataPduInfoBuffer, requestLength);
		PduIdType CanIf_NSdu = ChannelInfoPtr->ChannelConfPtr->DtNPdu;
		CanIf_Transmit(CanIf_NSdu, &dtPduInfoBuffer);

	} else {
		/* Todo: check for error */
	}

}

static inline void J1939Tp_Internal_SendRts(J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr, const PduInfoType* TxInfoPtr) {
	uint8 cmRtsData[RTS_SIZE];
	cmRtsData[RTS_BYTE_CONTROL] = 16;
	cmRtsData[RTS_BYTE_LENGTH_1] = (uint8)(TxInfoPtr->SduLength & 0x00FF);
	cmRtsData[RTS_BYTE_LENGTH_2] = (uint8)(TxInfoPtr->SduLength & 0xFF00);
	uint8 reminder = 0;
	if (TxInfoPtr->SduLength % J1939TP_PACKET_SIZE != 0) {
		reminder = 1;
	}
	J1939Tp_PgnType pgn = ChannelInfoPtr->TxState->CurrentPgPtr->Pgn;
	cmRtsData[RTS_BYTE_NUM_PACKETS] = (TxInfoPtr->SduLength / J1939TP_PACKET_SIZE) + reminder;
	cmRtsData[RTS_BYTE_SAE_ASSIGN] = 0xFF;
	J1939Tp_Internal_SetPgn(&(cmRtsData[RTS_BYTE_PGN_1]),pgn);

	PduInfoType cmRtsPdu;
	cmRtsPdu.SduLength = RTS_SIZE;
	cmRtsPdu.SduDataPtr = cmRtsData;

	CanIf_Transmit(ChannelInfoPtr->ChannelConfPtr->CmNPdu,&cmRtsPdu);
}

static inline void J1939Tp_Internal_SendEndOfMsgAck(J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr) {
	PduInfoType endofmsgInfo;
	uint8 endofmsgData[ENDOFMSGACK_SIZE];
	endofmsgInfo.SduLength = ENDOFMSGACK_SIZE;
	endofmsgInfo.SduDataPtr = endofmsgData;
	endofmsgData[ENDOFMSGACK_BYTE_CONTROL] = ENDOFMSGACK_CONTROL_VALUE;
	endofmsgData[ENDOFMSGACK_BYTE_TOTAL_MSG_SIZE_1] = ((uint8)(ChannelInfoPtr->RxState->TotalMessageSize)) << 8;
	endofmsgData[ENDOFMSGACK_BYTE_TOTAL_MSG_SIZE_2] = ((uint8)(ChannelInfoPtr->RxState->TotalMessageSize));
	endofmsgData[ENDOFMSGACK_BYTE_NUM_PACKETS] = ChannelInfoPtr->RxState->TotalReceivedDtCount;
	endofmsgData[ENDOFMSGACK_BYTE_SAE_ASSIGN] = 0xFF;
	J1939Tp_Internal_SetPgn(&(endofmsgData[ENDOFMSGACK_BYTE_PGN_1]),ChannelInfoPtr->RxState->CurrentPgPtr->Pgn);
	PduIdType CmNPdu = ChannelInfoPtr->ChannelConfPtr->FcNPdu;

	CanIf_Transmit(CmNPdu,&endofmsgInfo);
}

/**
 * Send a response to the incoming RTS
 * @param NSduId
 * @param RtsPduInfoPtr needs to be a valid RTS message
 */
static inline void J1939Tp_Internal_SendCts(J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr, PduInfoType* RtsPduInfoPtr) {
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

	CanIf_Transmit(ChannelInfoPtr->ChannelConfPtr->FcNPdu,&ctsInfo);

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
static inline void J1939Tp_Internal_TxSessionStartTimer(J1939Tp_Internal_TxChannelInfoType* Tx,uint16 TimerExpire) {
	Tx->TimerInfo.Timer = 0;
	Tx->TimerInfo.TimerExpire = TimerExpire;
}
static inline void J1939Tp_Internal_RxSessionStartTimer(J1939Tp_Internal_RxChannelInfoType* Rx,uint16 TimerExpire) {
	Rx->TimerInfo.Timer = 0;
	Rx->TimerInfo.TimerExpire = TimerExpire;
}
/**
 * set three bytes to a 18 bit pgn value
 * @param PgnBytes must be three uint8 bytes
 * @param pgn
 */
static inline void J1939Tp_Internal_SetPgn(uint8* PgnBytes,J1939Tp_PgnType pgn ) {
	PgnBytes[2] = pgn; /* get first byte */
	PgnBytes[1] = pgn >> 8; /* get next byte */
	PgnBytes[0] = (pgn >> 16) & 0x3; /* get next two bits */
}
/**
 * return a 18 bit pgn value from three bytes
 * @param PgnBytes must be three uint8 bytes
 * @return
 */
static inline J1939Tp_PgnType J1939Tp_Internal_GetPgn(uint8* PgnBytes) {
	J1939Tp_PgnType pgn = 0;
	pgn = ((uint32)PgnBytes[0]) << 16;
	pgn = pgn | (((uint32)PgnBytes[1]) << 8);
	pgn = pgn | ((uint32)PgnBytes[2]);
	return pgn;
}

static inline void J1939Tp_Internal_ReportError(uint8 ApiId, uint8 ErrorId) {
#if (CANSM_DEV_ERROR_DETECT == STD_ON)
	Det_ReportError(MODULE_ID_J1939TP, 0, ApiId, ApiId);
#endif
}
