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
#include "J1939Tp_ConfigTypes.h"
#include "ComStack_Types.h"
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
typedef struct {
	J1939Tp_Internal_TxChannelStateType State;
	J1939Tp_Internal_TimerType TimerInfo;
	uint8 SentDtCount;
	uint8 DtToSendBeforeCtsCount;
	J1939Tp_Internal_DtPayloadSizeType TotalMessageSize;
	PduIdType PduRPdu;
	const J1939Tp_PgType* CurrentPgPtr;
} J1939Tp_Internal_TxChannelInfoType;

typedef struct {
	J1939Tp_Internal_RxChannelStateType State;
	J1939Tp_Internal_TimerType TimerInfo;
	uint8 ReceivedDtCount;
	uint8 DtToReceiveCount;
	J1939Tp_Internal_DtPayloadSizeType TotalMessageSize;
	const J1939Tp_PgType* CurrentPgPtr;
} J1939Tp_Internal_RxChannelInfoType;

typedef struct {
	const J1939Tp_ChannelType* 					ChannelConfPtr;
	J1939Tp_Internal_TxChannelInfoType*		TxState; /* setup in init */
	J1939Tp_Internal_RxChannelInfoType*		RxState; /* setup in init */
} J1939Tp_Internal_ChannelInfoType;

typedef enum {
	J1939TP_PG_TX_IDLE,
	J1939TP_PG_TX_WAIT_DIRECT_SEND_CANIF_CONFIRM,
} J1939Tp_Internal_PgDirectState;

typedef struct {
	J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr;
	const J1939Tp_PgType* PgConfPtr;
	J1939Tp_Internal_PgDirectState TxState;
	J1939Tp_Internal_TimerType TimerInfo;
} J1939Tp_Internal_PgInfoType;

typedef struct {
	J1939Tp_Internal_GlobalStateType State;
} J1939Tp_Internal_GlobalStateInfoType;


typedef uint8 J1939Tp_Internal_ControlByteType;


static inline uint8 J1939Tp_Internal_GetPf(J1939Tp_PgnType pgn);
static inline Std_ReturnType J1939Tp_Internal_ValidatePacketType(const J1939Tp_RxPduInfoType* RxPduInfo);
static inline J1939Tp_Internal_ChannelInfoType* J1939Tp_Internal_GetChannelState(const J1939Tp_RxPduInfoType* RxPduInfo);
static inline const J1939Tp_ChannelType* J1939Tp_Internal_GetChannel(const J1939Tp_RxPduInfoType* RxPduInfo);
static inline void J1939Tp_Internal_RxIndication_Dt(PduInfoType* PduInfoPtr, J1939Tp_Internal_ChannelInfoType* Channel);
static inline void J1939Tp_Internal_RxIndication_Cm(PduInfoType* PduInfoPtr, J1939Tp_Internal_ChannelInfoType* Channel);
static inline void J1939Tp_Internal_RxIndication_ReverseCm(PduInfoType* PduInfoPtr, J1939Tp_Internal_ChannelInfoType* Channel);
static inline void J1939Tp_Internal_RxIndication_Direct(PduInfoType* PduInfoPtr, const J1939Tp_RxPduInfoType* RxPduInfoPtr);
Std_ReturnType J1939Tp_ChangeParameterRequest(PduIdType SduId, TPParameterType Parameter, uint16 value);
static inline void J1939Tp_Internal_TxConfirmation_RxChannel(J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr, const J1939Tp_RxPduInfoType* RxPduInfo);
static inline boolean J1939Tp_Internal_IsDtPacketAlreadySent(uint8 nextPacket, uint8 totalPacketsSent);
static inline Std_ReturnType J1939Tp_Internal_GetRxPduRelationsInfo(PduIdType RxPdu,const J1939Tp_RxPduInfoRelationsType** RxPduInfo);
static inline Std_ReturnType J1939Tp_Internal_GetPgFromPgn(const J1939Tp_ChannelType* channel, J1939Tp_Internal_PgnType Pgn, const J1939Tp_PgType** Pg);
static inline boolean J1939Tp_Internal_IsLastDt(J1939Tp_Internal_RxChannelInfoType* rxPgInfo);
static inline void J1939Tp_Internal_TxConfirmation_TxChannel(J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr,const J1939Tp_RxPduInfoType* RxPduInfo);
static inline boolean J1939Tp_Internal_IsLastDtBeforeNextCts(J1939Tp_Internal_RxChannelInfoType* rxChannelInfo);

/** @req J1939TP0096 */
Std_ReturnType J1939Tp_Transmit(PduIdType TxSduId, const PduInfoType* TxInfoPtr);
static inline uint16 J1939Tp_Internal_GetRtsMessageSize(PduInfoType* pduInfo);
static inline boolean J1939Tp_Internal_WaitForCts(J1939Tp_Internal_TxChannelInfoType* TxChannelState);
static inline boolean J1939Tp_Internal_LastDtSent(J1939Tp_Internal_TxChannelInfoType* TxPgState);
static inline Std_ReturnType J1939Tp_Internal_ConfGetPg(PduIdType NSduId, const J1939Tp_PgType* Pg);
static inline J1939Tp_Internal_TimerStatusType J1939Tp_Internal_IncAndCheckTimer(J1939Tp_Internal_TimerType* TimerInfo);
static inline uint8 J1939Tp_Internal_GetDtDataSize(uint8 currentSeqNum, uint8 totalSize);

static inline Std_ReturnType J1939Tp_Internal_SendBam(J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr,const PduInfoType* TxInfoPtr);
static inline Std_ReturnType J1939Tp_Internal_SendDt(J1939Tp_Internal_ChannelInfoType* Channel);
static inline Std_ReturnType J1939Tp_Internal_SendRts(J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr, const PduInfoType* TxInfoPtr);
static inline void J1939Tp_Internal_SendEndOfMsgAck(J1939Tp_Internal_ChannelInfoType* Channel);
static inline void J1939Tp_Internal_SendCts(J1939Tp_Internal_ChannelInfoType* ChannelInfoPtr, J1939Tp_PgnType Pgn, uint8 NextPacketSeqNum,uint8 NumPackets);
static inline void J1939Tp_Internal_SendConnectionAbort(PduIdType CmNPdu, J1939Tp_PgnType Pgn);
static inline void J1939Tp_Internal_StartTimer(J1939Tp_Internal_TimerType* TimerInfo,uint16 TimerExpire);
static inline void J1939Tp_Internal_StopTimer(J1939Tp_Internal_TimerType* TimerInfo);
static inline void J1939Tp_Internal_SetPgn(uint8* PgnBytes,J1939Tp_PgnType pgn );
static inline J1939Tp_PgnType J1939Tp_Internal_GetPgn(uint8* PgnBytes);
static inline uint8 J1939TP_Internal_GetNumDtPacketsToSend(uint16 messageSize);
static inline Std_ReturnType J1939Tp_Internal_DirectTransmit(const PduInfoType* TxInfoPtr, J1939Tp_Internal_PgInfoType* PgInfo);
static inline J1939Tp_Internal_PgInfoType* J1939Tp_GetPgInfo(const J1939Tp_PgType* Pg);

static inline void J1939Tp_Internal_ReportError(uint8 ApiId, uint8 ErrorId);
#endif
