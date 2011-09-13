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

#ifndef J1939TP_INTERNAL_H_
#define J1939TP_INTERNAL_H_

/** Service Ids */
#define J1939TP_TRANSMIT_ID 	0x05
#define J1939TP_INIT_ID 		0x01

/** Error Codes */
#define J1939TP_E_PARAM_ID		0x01
#define J1939TP_E_UNINIT		0x20
#define J1939TP_E_REINIT		0x21

#define J1939TP_PACKET_SIZE 7

#define CM_PGN_VALUE_1			0x00
#define CM_PGN_VALUE_2			0xCE
#define CM_PGN_VALUE_3			0x00


/* Rts message */
#define RTS_SIZE				8
#define RTS_BYTE_CONTROL		0
#define RTS_BYTE_LENGTH_1		1
#define RTS_BYTE_LENGTH_2		2
#define RTS_BYTE_NUM_PACKETS	3
#define RTS_BYTE_SAE_ASSIGN		4
#define RTS_BYTE_PGN_1			5
#define RTS_BYTE_PGN_2			6
#define RTS_BYTE_PGN_3			7


/* Cts message */
#define CTS_SIZE 				8
#define CTS_BYTE_CONTROL		0
#define CTS_BYTE_NUM_PACKETS	1
#define CTS_BYTE_NEXT_PACKET	2
#define CTS_BYTE_SAE_ASSIGN_1	3
#define CTS_BYTE_SAE_ASSIGN_2	4
#define CTS_BYTE_PGN_1			5
#define CTS_BYTE_PGN_2			6
#define CTS_BYTE_PGN_3			7

#define CTS_CONTROL_VALUE		17

/* Dt message */
#define DT_SIZE 				8
#define DT_DATA_SIZE 			7
#define DT_BYTE_SEQ_NUM			0
#define DT_BYTE_DATA_1			1
#define DT_BYTE_DATA_2			2
#define DT_BYTE_DATA_3			3
#define DT_BYTE_DATA_4			4
#define DT_BYTE_DATA_5			5
#define DT_BYTE_DATA_6			6
#define DT_BYTE_DATA_7			7

#define DT_PGN_VALUE_1			0x00
#define DT_PGN_VALUE_2			0xEB
#define DT_PGN_VALUE_3			0x00

/* EndOfMsgAck message */
#define ENDOFMSGACK_SIZE					8
#define ENDOFMSGACK_BYTE_CONTROL			0
#define ENDOFMSGACK_BYTE_TOTAL_MSG_SIZE_1	1
#define ENDOFMSGACK_BYTE_TOTAL_MSG_SIZE_2	2
#define ENDOFMSGACK_BYTE_NUM_PACKETS		3
#define ENDOFMSGACK_BYTE_SAE_ASSIGN			4
#define ENDOFMSGACK_BYTE_PGN_1				5
#define ENDOFMSGACK_BYTE_PGN_2				6
#define ENDOFMSGACK_BYTE_PGN_3				7
#define ENDOFMSGACK_CONTROL_VALUE			19


#define J1939TP_T4_TIMEOUT_MS	1050
#define J1939TP_T3_TIMEOUT_MS	1250
#define J1939TP_T2_TIMEOUT_MS	1250
#define J1939TP_T1_TIMEOUT_MS	750

/** @req J1939TP0019 */

typedef enum {
	J1939TP_ON,
	J1939TP_OFF
} J1939Tp_Internal_GlobalStateType;

typedef struct {
	J1939Tp_Internal_GlobalStateType State;
} J1939Tp_Internal_GlobalStateInfoType;

typedef enum {
	J1939TP_IDLE,
	J1939TP_WAITING_FOR_CTS,
	J1939TP_SENDING_DT,
	J1939TP_WAITING_FOR_END_OF_MSG_ACK
} J1939Tp_Internal_TxPgStateType;


typedef struct {
	J1939Tp_Internal_TxPgStateType State;
	uint32 T3;
	uint8 SentDtCount;
	uint8 DtToSendBeforeCtsCount;
	uint16 TotalMessageSize;
	uint8 TotalSentDtCount;
	uint16 TotalBytesSent;
	PduIdType PduRPdu;

} J1939Tp_Internal_TxPgStateInfoType;

static inline const J1939Tp_PgType* J1939Tp_Internal_ConfGetPg(PduIdType pduId);
static inline boolean J1939Tp_Internal_WaitForCts(J1939Tp_Internal_TxPgStateInfoType* pgState);
static inline boolean J1939Tp_Internal_WaitForEndOfMsgAck(J1939Tp_Internal_TxPgStateInfoType* pgState);
static inline void J1939Tp_Internal_SetStatePg(uint32 txPduId,J1939Tp_Internal_TxPgStateType state);
static inline J1939Tp_Internal_TxPgStateInfoType* J1939Tp_Internal_GetPg(uint32 txPduId);
static inline const J1939Tp_ChannelType* J1939Tp_Internal_ConfGetTxChannel(uint32 txPduId);
static inline boolean J1939Tp_Internal_CheckValidEndOfMsgAck(J1939Tp_Internal_TxPgStateInfoType* pgState, PduInfoType* PduInfoPtr);
static inline void J1939Tp_Internal_IncAndCheckT3Timer(PduIdType pduId,J1939Tp_Internal_TxPgStateInfoType* pgState);
static uint8 J1939Tp_Internal_GetPf(uint32 pgn);
static J1939Tp_ProtocolType J1939Tp_Internal_GetProtocol(uint8 pf);
static inline boolean J1939Tp_Internal_CheckValidCts(PduInfoType* PduInfoPtr,uint8* NumPackets);
static void inline J1939Tp_Internal_SendDt(PduIdType RxPduId,J1939Tp_Internal_TxPgStateInfoType* PgState);
static void J1939Tp_Internal_SendRts(PduIdType TxSduId, const PduInfoType* TxInfoPtr);
static inline void J1939Tp_Internal_ResetT3(J1939Tp_Internal_TxPgStateInfoType* pgState);
static inline void J1939Tp_Internal_ReportError(uint8 ApiId, uint8 ErrorId);

#endif
