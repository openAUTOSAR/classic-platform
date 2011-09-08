#include "J1939Tp.h" /** @req J1939TP0003*/
#include "CanIf.h" /** @req J1939TP0172 */
#include "J1939Tp_Internal.h"



#define J1939TP_PACKET_SIZE 7

/** @req J1939TP0019 */
typedef enum {
	J1939TP_ON,
	J1939TP_OFF
} J1939Tp_GlobalStateType;

/** @req J1939TP0180 */
Std_ReturnType J1939Tp_ChangeParameterRequest(PduIdType SduId, TPParameterType Parameter, uint16 value) {
	return E_NOT_OK; /** @req J1939TP0181 */
}

/** @req J1939TP0019 */
static J1939Tp_GlobalStateType globalState = J1939TP_OFF;

/** @req J1939TP0087 */
void J1939Tp_Init(const J1939Tp_ConfigType* ConfigPtr) {
	#if J1939TP_DEV_ERROR_DETECT
	if (globalState == J11939TP_ON) {
		/** @req J1939TP0026 */
		J1939TP_DET_REPORTERROR(J1939_INIT_ID, J1939TP_E_REINIT);
		return;
	}
	#endif
	globalState = J1939TP_ON; /** @req J1939TP0022 */

}

#define RTS_SIZE				8

#define RTS_BYTE_CONTROL		0
#define RTS_BYTE_LENGTH_1		1
#define RTS_BYTE_LENGTH_2		2
#define RTS_BYTE_NUM_PACKETS	3
#define RTS_BYTE_SAE_ASSIGN		4
#define RTS_BYTE_PGN_1			5
#define RTS_BYTE_PGN_2			6
#define RTS_BYTE_PGN_3			7

#define RTS_PGN_VALUE_1			0x00
#define RTS_PGN_VALUE_2			0xCE
#define RTS_PGN_VALUE_3			0x00

/** @req J1939TP0096 */
Std_ReturnType J1939Tp_Transmit(PduIdType TxSduId, const PduInfoType* TxInfoPtr) {
	#if J1939TP_DEV_ERROR_DETECT
	if (globalState == J1939TP_OFF) {
		J1939TP_DET_REPORTERROR(J1939_TRANSMIT_ID,J1939_E_UNINIT);
	}
	#endif
	uint8 cmRtsData[RTS_SIZE];

	cmRtsData[RTS_BYTE_CONTROL] = 16;
	cmRtsData[RTS_BYTE_LENGTH_1] = TxInfoPtr->SduLength >> 8;
	cmRtsData[RTS_BYTE_LENGTH_2] = TxInfoPtr->SduLength << 8;
	cmRtsData[RTS_BYTE_NUM_PACKETS] = TxInfoPtr->SduLength / J1939TP_PACKET_SIZE;
	cmRtsData[RTS_BYTE_SAE_ASSIGN] = 0xFF;
	cmRtsData[RTS_BYTE_PGN_1] = RTS_PGN_VALUE_1;
	cmRtsData[RTS_BYTE_PGN_2] = RTS_PGN_VALUE_2;
	cmRtsData[RTS_BYTE_PGN_3] = RTS_PGN_VALUE_3;

	PduInfoType cmRtsPdu;
	cmRtsPdu.SduLength = RTS_SIZE;
	cmRtsPdu.SduDataPtr = cmRtsData;
	CanIf_Transmit(TxSduId,&cmRtsPdu);

}
