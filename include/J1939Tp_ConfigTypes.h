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


#ifndef J1939TP_CONFIGTYPES_H_
#define J1939TP_CONFIGTYPES_H_

#include "ComStack_Types.h"

typedef enum {
	J1939TP_PROTOCOL_BAM,
	J1939TP_PROTOCOL_CMDT
} J1939Tp_ProtocolType;

typedef enum {
	J1939TP_RX,
	J1939TP_TX
} J1939Tp_DirectionType;

typedef enum {
	J1939TP_TX_IDLE,
	J1939TP_TX_WAIT_BAM_CANIF_CONFIRM,
	J1939TP_TX_WAIT_RTS_CANIF_CONFIRM,
	J1939TP_TX_WAITING_FOR_CTS,
	J1939TP_TX_WAIT_DT_CANIF_CONFIRM,
	J1939TP_TX_WAIT_DT_BAM_CANIF_CONFIRM,
	J1939TP_TX_WAITING_FOR_BAM_DT_SEND_TIMEOUT,
	J1939TP_TX_WAITING_FOR_END_OF_MSG_ACK
} J1939Tp_Internal_TxChannelStateType;

typedef enum {
	J1939TP_RX_IDLE,
	J1939TP_RX_WAIT_CTS_CANIF_CONFIRM,
	J1939TP_RX_RECEIVING_DT,
	J1939TP_RX_WAIT_ENDOFMSGACK_CANIF_CONFIRM,
} J1939Tp_Internal_RxChannelStateType;

typedef struct {
	uint32 Timer;
	uint32 TimerExpire;
} J1939Tp_Internal_TimerType;
typedef uint32 J1939Tp_Internal_PgnType;
typedef uint32 J1939Tp_Internal_DtPayloadSizeType;


typedef struct J1939Tp_ChannelType_ J1939Tp_ChannelType;
typedef struct J1939Tp_PgType_ J1939Tp_PgType;
typedef uint32 J1939Tp_PgnType;

struct J1939Tp_PgType_ {
	const boolean 					DynLength;
	const J1939Tp_PgnType			Pgn;
	const PduIdType 				DirectNPdu; /** only set when DynLength = true */
	const PduIdType					NSdu;
	const J1939Tp_ChannelType*		Channel;
};


struct J1939Tp_ChannelType_ {
	const J1939Tp_ProtocolType 				Protocol;
	const PduIdType 						DtNPdu;
	const PduIdType 						CmNPdu;
	const PduIdType	 						FcNPdu; /** only set when Protocol == J1939TP_PROTOCOL_CMDT */
	const J1939Tp_DirectionType 			Direction;
	const uint16							PgCount;
	const J1939Tp_PgType**					Pgs;
} ;

typedef enum {
	J1939TP_REVERSE_CM,
	J1939TP_CM,
	J1939TP_DT,
	J1939TP_DIRECT
} J1939Tp_RxPduType;


typedef struct {
	const J1939Tp_RxPduType		PacketType;
	const uint8					ChannelIndex;
	const J1939Tp_PgType* PgPtr;
} J1939Tp_RxPduInfoType;

typedef struct {
	const J1939Tp_RxPduInfoType** RxPdus;
	const PduIdType RxPduCount;
} J1939Tp_RxPduInfoRelationsType;

/** @req J1939TP0175 */
typedef struct {
	const J1939Tp_RxPduInfoRelationsType* RxPduRelations;
	const J1939Tp_ChannelType* Channels;
	const J1939Tp_PgType* Pgs;
} J1939Tp_ConfigType;

#endif
