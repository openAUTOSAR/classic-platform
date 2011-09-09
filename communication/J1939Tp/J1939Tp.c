#include "J1939Tp.h" /** @req J1939TP0003*/
#include "CanIf.h" /** @req J1939TP0172 */
#include "J1939Tp_Internal.h"
#include "PdurR_J1939Tp.h"

/** @req J1939TP0019 */
static J1939Tp_GlobalStateType globalState = J1939TP_OFF;
static const J1939Tp_ConfigType* J1939Tp_ConfigPtr;

/** @req J1939TP0087 */
void J1939Tp_Init(const J1939Tp_ConfigType* ConfigPtr) {
	#if (J1939TP_DEV_ERROR_DETECT == STD_ON)
	if (globalState == J1939TP_ON) {
		/** @req J1939TP0026 */
		J1939Tp_Internal_ReportError(J1939TP_INIT_ID, J1939TP_E_REINIT);
		return;
	}
	#endif
	J1939Tp_ConfigPtr = ConfigPtr;
	globalState = J1939TP_ON; /** @req J1939TP0022 */
}
void J1939Tp_RxIndication(PduIdType RxPduId, PduInfoType* PduInfoPtr) {

}

/** @req J1939TP0180 */
Std_ReturnType J1939Tp_ChangeParameterRequest(PduIdType SduId, TPParameterType Parameter, uint16 value) {
	return E_NOT_OK; /** @req J1939TP0181 */
}

/** @req J1939TP0096 */
Std_ReturnType J1939Tp_Transmit(PduIdType TxSduId, const PduInfoType* TxInfoPtr) {
	#if J1939TP_DEV_ERROR_DETECT
	if (globalState == J1939TP_OFF) {
		J1939Tp_Internal_ReportError(J1939TP_TRANSMIT_ID,J1939TP_E_UNINIT);
	}
	#endif
	if (TxInfoPtr->SduLength <= 8) {
		PduR_J1939TpProvideTxBuffer(TxSduId, &TxInfoPtr,0);
		if (CanIf_Transmit(TxSduId,TxInfoPtr) == E_OK) {
			PduR_J1939TpTxConfirmation(TxSduId, NTFRSLT_OK);
		} else {
			PduR_J1939TpTxConfirmation(TxSduId, NTFRSLT_E_NOT_OK);
		}
	} else {
		uint32 pgn = J1939Tp_Internal_ConfGetTxPg(TxSduId)->Pgn;
		uint8 pf = J1939Tp_Internal_GetPf(pgn);
		J1939Tp_ProtocolType protocol = J1939Tp_Internal_GetProtocol(pf);
		switch (protocol) { /** @req J1939TP0039*/
			case J1939TP_PROTOCOL_BAM:
				break;
			case J1939TP_PROTOCOL_CMDT:
				J1939Tp_Internal_SendRts(TxSduId,TxInfoPtr);
				break;
		}
	}
	return E_OK;
}


static inline const J1939Tp_PgType* J1939Tp_Internal_ConfGetTxPg(uint32 txPduId) {
	return &(J1939Tp_ConfigPtr->TxPgs[txPduId]);
}
static inline const J1939Tp_ChannelType* J1939Tp_Internal_ConfGetTxChannel(uint32 txPduId) {
	return J1939Tp_ConfigPtr->TxPgs[txPduId].Channel;
}

static void J1939Tp_Internal_SendRts(PduIdType TxSduId, const PduInfoType* TxInfoPtr) {
	uint8 cmRtsData[RTS_SIZE];
	cmRtsData[RTS_BYTE_CONTROL] = 16;
	cmRtsData[RTS_BYTE_LENGTH_1] = TxInfoPtr->SduLength >> 8;
	cmRtsData[RTS_BYTE_LENGTH_2] = TxInfoPtr->SduLength << 8;
	uint8 reminder = 0;
	if (TxInfoPtr->SduLength % J1939TP_PACKET_SIZE == 0) {
		reminder = 1;
	}
	cmRtsData[RTS_BYTE_NUM_PACKETS] = (TxInfoPtr->SduLength / J1939TP_PACKET_SIZE) + reminder;
	cmRtsData[RTS_BYTE_SAE_ASSIGN] = 0xFF;
	cmRtsData[RTS_BYTE_PGN_1] = RTS_PGN_VALUE_1;
	cmRtsData[RTS_BYTE_PGN_2] = RTS_PGN_VALUE_2;
	cmRtsData[RTS_BYTE_PGN_3] = RTS_PGN_VALUE_3;

	PduInfoType cmRtsPdu;
	cmRtsPdu.SduLength = RTS_SIZE;
	cmRtsPdu.SduDataPtr = cmRtsData;
	CanIf_Transmit(J1939Tp_Internal_ConfGetTxChannel(TxSduId)->CmNPdu,&cmRtsPdu);
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
static inline void J1939Tp_Internal_ReportError(uint8 ApiId, uint8 ErrorId) {
#if (CANSM_DEV_ERROR_DETECT == STD_ON)
	Det_ReportError(MODULE_ID_J1939TP, 0, ApiId, ApiId);
#endif
}
