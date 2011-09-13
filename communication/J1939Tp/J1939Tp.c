#include "J1939Tp.h" /** @req J1939TP0003*/
#include "CanIf.h" /** @req J1939TP0172 */
#include "J1939Tp_Internal.h"
#include "PduR.h"
#include <string.h>

/** @req J1939TP0019 */
static J1939Tp_Internal_GlobalStateInfoType globalState = {
		.State = J1939TP_OFF,
};
static const J1939Tp_ConfigType* J1939Tp_ConfigPtr;
static J1939Tp_Internal_TxPgStateInfoType txPgState[J1939TP_PG_COUNT];

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
		J1939Tp_Internal_SetStatePg(i,J1939TP_IDLE);
	}
	J1939Tp_ConfigPtr = ConfigPtr;
	globalState.State = J1939TP_ON; /** @req J1939TP0022 */
}

void J1939Tp_RxIndication(PduIdType RxPduId, PduInfoType* PduInfoPtr) {
	J1939Tp_Internal_TxPgStateInfoType* txPgState = J1939Tp_Internal_GetPg(RxPduId);
	uint8 NumPacketsToSend = 0;
	switch (txPgState->State) {
		case J1939TP_WAITING_FOR_CTS:
			if (J1939Tp_Internal_CheckValidCts(PduInfoPtr,&NumPacketsToSend)) {
				txPgState->DtToSendBeforeCtsCount = NumPacketsToSend;
				txPgState->SentDtCount = 0;
				J1939Tp_Internal_SetStatePg(RxPduId, J1939TP_SENDING_DT);
				J1939Tp_Internal_SendDt(RxPduId, txPgState);
			}
			break;
		case J1939TP_WAITING_FOR_END_OF_MSG_ACK:
			J1939Tp_Internal_CheckValidEndOfMsgAck(txPgState,PduInfoPtr);
		default:
			break;
	}
}

void J1939Tp_MainFunction(void) {
	for (int i = 0; i < J1939TP_PG_COUNT; i++) {
		J1939Tp_Internal_TxPgStateInfoType* txPgState = J1939Tp_Internal_GetPg(i);
		switch (txPgState->State) {
			case J1939TP_WAITING_FOR_CTS:
				J1939Tp_Internal_IncAndCheckT3Timer(i,txPgState);
				break;
			case J1939TP_WAITING_FOR_END_OF_MSG_ACK:
				J1939Tp_Internal_IncAndCheckT3Timer(i,txPgState);
				break;
			default:
				break;
		}
	}
}
void J1939Tp_TxConfirmation(PduIdType TxPduId) {
	J1939Tp_Internal_TxPgStateInfoType* pgState = J1939Tp_Internal_GetPg(TxPduId);
	switch (pgState->State) {
		case J1939TP_SENDING_DT:
			if (J1939Tp_Internal_WaitForEndOfMsgAck(pgState)) {
				J1939Tp_Internal_ResetT3(pgState);
				J1939Tp_Internal_SetStatePg(TxPduId, J1939TP_WAITING_FOR_END_OF_MSG_ACK);
			} else if (J1939Tp_Internal_WaitForCts(pgState)) {
				J1939Tp_Internal_SetStatePg(TxPduId, J1939TP_WAITING_FOR_CTS);
			} else {
				J1939Tp_Internal_SendDt(TxPduId,pgState);
			}
			break;
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
	if (TxInfoPtr->SduLength <= 8) { // direct transmit
		PduInfoType* ToSendTxInfoPtr;
		PduR_J1939TpProvideTxBuffer(TxSduId, &ToSendTxInfoPtr, TxInfoPtr->SduLength);
		if (CanIf_Transmit(TxSduId,ToSendTxInfoPtr) == E_OK) {
			PduR_J1939TpTxConfirmation(TxSduId, NTFRSLT_OK);
		} else {
			PduR_J1939TpTxConfirmation(TxSduId, NTFRSLT_E_NOT_OK);
		}
	} else {
		uint32 pgn = J1939Tp_Internal_ConfGetPg(TxSduId)->Pgn;
		uint8 pf = J1939Tp_Internal_GetPf(pgn);
		J1939Tp_ProtocolType protocol = J1939Tp_Internal_GetProtocol(pf);
		switch (protocol) { /** @req J1939TP0039*/
			case J1939TP_PROTOCOL_BAM:
				break;
			case J1939TP_PROTOCOL_CMDT:
				J1939Tp_Internal_SendRts(TxSduId,TxInfoPtr);
				J1939Tp_Internal_TxPgStateInfoType* txPgState = J1939Tp_Internal_GetPg(TxSduId);
				J1939Tp_Internal_ResetT3(txPgState);
				txPgState->TotalMessageSize = TxInfoPtr->SduLength;
				txPgState->TotalBytesSent = 0;
				txPgState->TotalSentDtCount = 0;
				txPgState->PduRPdu = TxSduId;
				J1939Tp_Internal_SetStatePg(TxSduId,J1939TP_WAITING_FOR_CTS);
				break;
		}
	}
	return E_OK;
}

static inline const J1939Tp_PgType* J1939Tp_Internal_ConfGetPg(PduIdType pduId) {
	return &(J1939Tp_ConfigPtr->Pgs[pduId]);
}
static inline boolean J1939Tp_Internal_WaitForCts(J1939Tp_Internal_TxPgStateInfoType* pgState) {
	return pgState->SentDtCount == pgState->DtToSendBeforeCtsCount;
}

static inline boolean J1939Tp_Internal_WaitForEndOfMsgAck(J1939Tp_Internal_TxPgStateInfoType* pgState) {
	return pgState->TotalMessageSize == pgState->TotalBytesSent;
}

static inline void J1939Tp_Internal_SetStatePg(uint32 txPduId,J1939Tp_Internal_TxPgStateType state) {
	txPgState[txPduId].State = state;
}
static inline J1939Tp_Internal_TxPgStateInfoType* J1939Tp_Internal_GetPg(uint32 txPduId) {
	return &(txPgState[txPduId]);
}
static inline const J1939Tp_ChannelType* J1939Tp_Internal_ConfGetTxChannel(uint32 txPduId) {
	return J1939Tp_ConfigPtr->Pgs[txPduId].Channel;
}



static inline boolean J1939Tp_Internal_CheckValidEndOfMsgAck(J1939Tp_Internal_TxPgStateInfoType* pgState, PduInfoType* PduInfoPtr) {
	if (PduInfoPtr->SduLength != ENDOFMSGACK_SIZE) {
		return false;
	}
	if (PduInfoPtr->SduDataPtr[ENDOFMSGACK_BYTE_CONTROL] != ENDOFMSGACK_CONTROL_VALUE) {
		return false;
	}
	return true;
}
static inline void J1939Tp_Internal_IncAndCheckT3Timer(PduIdType pduId, J1939Tp_Internal_TxPgStateInfoType* pgState) {
	txPgState->T3 += (J1939TP_MAIN_FUNCTION_PERIOD*1000);
	if (txPgState->T3 > J1939TP_T3_TIMEOUT_MS) {
		txPgState->State = J1939TP_IDLE;
		PduR_J1939TpTxConfirmation(J1939Tp_Internal_ConfGetPg(pduId)->NSdu,NTFRSLT_E_NOT_OK);
		/* TODO: Call det here */
	}
}
static uint8 J1939Tp_Internal_GetPf(uint32 pgn) {
	return (pgn && 0x0000FF00) >> 8;
}
static J1939Tp_ProtocolType J1939Tp_Internal_GetProtocol(uint8 pf) {
	if (pf < 240) {
		return J1939TP_PROTOCOL_CMDT;
	} else {
		return J1939TP_PROTOCOL_BAM;
	}
}
static inline boolean J1939Tp_Internal_CheckValidCts(PduInfoType* PduInfoPtr,uint8* NumPackets) {
	/*
	if (PduInfoPtr->SduLength != CTS_SIZE) {
		return false;
	}
	*/
	if (PduInfoPtr->SduDataPtr[CTS_BYTE_CONTROL] != CTS_CONTROL_VALUE) {
		return false;
	}
	if (PduInfoPtr->SduDataPtr[CTS_BYTE_NUM_PACKETS] < 1) {
		return false;
	}
	*NumPackets = PduInfoPtr->SduDataPtr[CTS_BYTE_NUM_PACKETS];
	return true;
}

static void inline J1939Tp_Internal_SendDt(PduIdType TxPduId, J1939Tp_Internal_TxPgStateInfoType* PgState) {
	uint8 requestLength = DT_DATA_SIZE;
	uint8 bytesLeftToSend = PgState->TotalMessageSize - PgState->TotalSentDtCount * DT_DATA_SIZE;
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
	PduIdType Pdur_NSdu = J1939Tp_Internal_ConfGetPg(TxPduId)->NSdu;
	allocateBufferRes = PduR_J1939TpProvideTxBuffer(Pdur_NSdu, &dataPduInfoBuffer, requestLength);
	if (allocateBufferRes == BUFREQ_OK) {
		dtPduInfoBuffer.SduDataPtr[DT_BYTE_SEQ_NUM] = PgState->TotalSentDtCount;
		memcpy(&(dtPduInfoBuffer.SduDataPtr[DT_BYTE_DATA_1]), dataPduInfoBuffer, requestLength);
		PduIdType CanIf_NSdu = J1939Tp_Internal_ConfGetPg(TxPduId)->Channel->DtNPdu;
		CanIf_Transmit(CanIf_NSdu, &dtPduInfoBuffer);
		PgState->TotalBytesSent += requestLength;
		PgState->TotalSentDtCount++;
		PgState->SentDtCount++;
	} else {
		/* Todo: check for error */
	}

}
static void J1939Tp_Internal_SendRts(PduIdType TxSduId, const PduInfoType* TxInfoPtr) {
	uint8 cmRtsData[RTS_SIZE];
	cmRtsData[RTS_BYTE_CONTROL] = 16;
	cmRtsData[RTS_BYTE_LENGTH_1] = (uint8)(TxInfoPtr->SduLength & 0x00FF);
	cmRtsData[RTS_BYTE_LENGTH_2] = (uint8)(TxInfoPtr->SduLength & 0xFF00);
	uint8 reminder = 0;
	if (TxInfoPtr->SduLength % J1939TP_PACKET_SIZE != 0) {
		reminder = 1;
	}
	const J1939Tp_PgType* pg = J1939Tp_Internal_ConfGetPg(TxSduId);
	uint32 pgn = pg->Pgn;
	cmRtsData[RTS_BYTE_NUM_PACKETS] = (TxInfoPtr->SduLength / J1939TP_PACKET_SIZE) + reminder;
	cmRtsData[RTS_BYTE_SAE_ASSIGN] = 0xFF;
	/* PGN is 18 bits, RTS message have 24 bits PGN payload, therefore we squeeze in pdu id */
	cmRtsData[RTS_BYTE_PGN_1] = pgn; /* get first byte */
	cmRtsData[RTS_BYTE_PGN_2] = pgn >> 8; /* get next byte */
	cmRtsData[RTS_BYTE_PGN_3] = (pgn >> 16) & 0x3; /* get next two bits */
	//cmRtsData[RTS_BYTE_PGN_3] = cmRtsData[RTS_BYTE_PGN_3] & ((TxSduId << 2) | 0x03); /* add the sdu id to the remaining 4 bits*/

	PduInfoType cmRtsPdu;
	cmRtsPdu.SduLength = RTS_SIZE;
	cmRtsPdu.SduDataPtr = cmRtsData;
	CanIf_Transmit(J1939Tp_Internal_ConfGetTxChannel(TxSduId)->CmNPdu,&cmRtsPdu);
}
static inline void J1939Tp_Internal_ResetT3(J1939Tp_Internal_TxPgStateInfoType* pgState) {
	pgState->T3 = 0;
}

static inline void J1939Tp_Internal_ReportError(uint8 ApiId, uint8 ErrorId) {
#if (CANSM_DEV_ERROR_DETECT == STD_ON)
	Det_ReportError(MODULE_ID_J1939TP, 0, ApiId, ApiId);
#endif
}
