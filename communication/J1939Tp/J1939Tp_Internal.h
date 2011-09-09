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

/** Rts message */
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

/** @req J1939TP0019 */
typedef enum {
	J1939TP_ON,
	J1939TP_OFF
} J1939Tp_GlobalStateType;

typedef struct {
	boolean waitingForCts;
} J1939Tp_Internal_TxPduStateType;

static inline const J1939Tp_PgType* J1939Tp_Internal_ConfGetTxPg(uint32 txPduId);
static void J1939Tp_Internal_SendRts(PduIdType TxSduId, const PduInfoType* TxInfoPtr);
static uint8 J1939Tp_Internal_GetPf(uint32 pgn);
static J1939Tp_ProtocolType J1939Tp_Internal_GetProtocol(uint8 pf);
static inline void J1939Tp_Internal_ReportError(uint8 ApiId, uint8 ErrorId);
#endif
