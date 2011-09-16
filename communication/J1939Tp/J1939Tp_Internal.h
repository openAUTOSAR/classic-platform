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
#include "J1939Tp_Internal_Packets.h"
#define PGN_BYTE_COUNT			3
/** Service Ids */
#define J1939TP_TRANSMIT_ID 	0x05
#define J1939TP_INIT_ID 		0x01

/** Error Codes */
#define J1939TP_E_PARAM_ID		0x01
#define J1939TP_E_UNINIT		0x20
#define J1939TP_E_REINIT		0x21

#define J1939TP_PACKET_SIZE 7



/** @req J1939TP0019 */

typedef enum {
	J1939TP_ON,
	J1939TP_OFF
} J1939Tp_Internal_GlobalStateType;
typedef enum {
	J1939TP_EXPIRED,
	J1939TP_NOT_EXPIRED
} J1939Tp_Internal_TimerStatusType;
typedef uint8 J1939Tp_Internal_ControlByteType;
typedef uint32 J1939Tp_Internal_PgnType;
typedef uint32 J1939Tp_Internal_DtPayloadSizeType;

typedef struct {
	J1939Tp_Internal_GlobalStateType State;
} J1939Tp_Internal_GlobalStateInfoType;

typedef enum {
	J1939TP_TX_IDLE,
	J1939TP_TX_WAIT_RTS_CANIF_CONFIRM,
	J1939TP_TX_WAIT_DIRECT_SEND_CANIF_CONFIRM,
	J1939TP_TX_WAITING_FOR_CTS,
	J1939TP_TX_WAIT_DT_CANIF_CONFIRM,
	J1939TP_TX_WAITING_FOR_END_OF_MSG_ACK
} J1939Tp_Internal_TxPgStateType;
typedef enum {
	J1939TP_RX_IDLE,
	J1939TP_RX_WAIT_CTS_CANIF_CONFIRM,
	J1939TP_RX_RECEIVING_DT,
	J1939TP_RX_WAIT_ENDOFMSGACK_CANIF_CONFIRM,
} J1939Tp_Internal_RxPgStateType;
typedef struct {
	uint32 Timer;
	uint32 TimerExpire;
} J1939Tp_Internal_TimerType;

typedef struct {
	J1939Tp_Internal_RxPgStateType State;
	J1939Tp_Internal_TimerType TimerInfo;
	uint8 TotalReceivedDtCount;
	uint8 TotalDtToReceiveCount;
	J1939Tp_Internal_DtPayloadSizeType TotalMessageSize;
}J1939Tp_Internal_RxPgInfo;

typedef struct {
	J1939Tp_Internal_TxPgStateType State;
	J1939Tp_Internal_TimerType TimerInfo;
	uint8 SentDtCount;
	uint8 DtToSendBeforeCtsCount;
	J1939Tp_Internal_DtPayloadSizeType TotalMessageSize;
	uint8 TotalSentDtCount;
	uint16 TotalBytesSent;
	PduIdType PduRPdu;
}J1939Tp_Internal_TxPgInfo;

typedef struct {
	J1939Tp_Internal_TxPgInfo* Tx;
	J1939Tp_Internal_RxPgInfo* Rx;
	const J1939Tp_PgType* PgConfPtr;
} J1939Tp_Internal_PgStateInfoType;

static void J1939Tp_Internal_SendRts(J1939Tp_Internal_PgStateInfoType* TxPgState, const PduInfoType* TxInfoPtr);
static void inline J1939Tp_Internal_SendDt(J1939Tp_Internal_PgStateInfoType* PgState);
static inline void J1939Tp_Internal_SendCts(J1939Tp_Internal_PgStateInfoType* TxPgState, PduInfoType* RtsPduInfoPtr);
static void inline J1939Tp_Internal_SendEndOfMsgAck(J1939Tp_Internal_PgStateInfoType* PgState);
static inline void J1939Tp_Internal_SendConnectionAbort(PduIdType CmPdu, J1939Tp_PgnType Pgn);
static void inline J1939Tp_Internal_TxSessionStartTimer(J1939Tp_Internal_TxPgInfo* Tx,uint16 TimerExpire);
static void inline J1939Tp_Internal_RxSessionStartTimer(J1939Tp_Internal_RxPgInfo* Rx,uint16 TimerExpire);
static inline const J1939Tp_PgType* J1939Tp_Internal_ConfGetPg(PduIdType pduId);
static inline boolean J1939Tp_Internal_WaitForCts(J1939Tp_Internal_TxPgInfo* TxPgState);
static inline boolean J1939Tp_Internal_WaitForEndOfMsgAck(J1939Tp_Internal_TxPgInfo* TxPgState);

static inline J1939Tp_Internal_PgStateInfoType* J1939Tp_Internal_GetPg(PduIdType txPduId);
static inline const J1939Tp_ChannelType* J1939Tp_Internal_ConfGetTxChannel(PduIdType txPduId);
static inline boolean J1939Tp_Internal_CheckValidEndOfMsgAck(J1939Tp_Internal_PgStateInfoType* pgState, PduInfoType* PduInfoPtr);
static inline J1939Tp_Internal_TimerStatusType J1939Tp_Internal_IncAndCheckTimer(J1939Tp_Internal_TimerType* Timer);
static uint8 J1939Tp_Internal_GetPf(uint32 pgn);
static J1939Tp_ProtocolType J1939Tp_Internal_GetProtocol(uint8 pf);
static inline boolean J1939Tp_Internal_CheckValidCts(PduInfoType* PduInfoPtr,uint8* NumPackets,uint8* NextPacket);

static inline void J1939Tp_Internal_ResetTimer(J1939Tp_Internal_PgStateInfoType* pgState);
static inline void J1939Tp_Internal_ReportError(uint8 ApiId, uint8 ErrorId);
static void inline J1939Tp_Internal_RxIndication_TxChannel(PduIdType RxPduId, PduInfoType* PduInfoPtr);
static void inline J1939Tp_Internal_RxIndication_RxChannel(PduIdType RxPduId, PduInfoType* PduInfoPtr);
static Std_ReturnType inline J1939Tp_Internal_ReceiveRts(PduIdType NSduId, PduInfoType* PduInfoPtr);

static Std_ReturnType inline J1939Tp_Internal_ReceiveDt(PduIdType NSduId, PduInfoType* PduInfoPtr);
static inline void J1939Tp_Internal_SetPgn(uint8* PgnBytes,J1939Tp_PgnType pgn );
static inline uint16 J1939Tp_Internal_GetRtsMessageSize(PduInfoType* pduInfo);
static inline void J1939Tp_Internal_TxConfirmation_TxChannel(PduIdType NSduId);
static inline Std_ReturnType J1939Tp_Internal_GetCmControlByte(PduInfoType* pduInfo, J1939Tp_Internal_ControlByteType* controlByte);
static inline void J1939Tp_Internal_TxConfirmation_RxChannel(PduIdType NSduId);
#endif
