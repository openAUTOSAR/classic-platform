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


/** This N-PDU represents the TP.CM frame that is used in reverese direction
 * for a J1939 transport protocol session using CMDT. TP.CM in reverse direction
 * is used for intermediate and final ack of received  data and to abort the connection
 */
typedef struct {
	const PduIdType	Pdu;
} J1939Tp_FcNPduType;

typedef struct {
	const PduIdType	Pdu;
} J1939Tp_CmNPduType;

typedef struct {
	const PduIdType	Pdu;
}J1939Tp_DtNPduType;

typedef struct {
	const PduIdType Pdu;
} J1939Tp_NSduType;

typedef struct {
	const PduIdType	Pdu;
} J1939Tp_DirectNPduType;

typedef struct {
	const boolean 					DynLength;
	const uint32  					Pgn;
	const J1939Tp_DirectNPduType 	DirectNPdu; /** only set when DynLength = true */
	const J1939Tp_NSduType			NSdu;
} J1939Tp_PgType;

typedef struct {
	const J1939Tp_ProtocolType 	Protocol;
	const J1939Tp_DtNPduType 	DtNPdu;
	const J1939Tp_CmNPduType 	CmNPdu;
	const J1939Tp_PgType 		*Pgs;
	const J1939Tp_FcNPduType 	FcNPdu; /** only set when Protocol == J1939TP_PROTOCOL_CMDT */
} J1939Tp_ChannelType;

/** @req J1939TP0175 */
typedef struct {
	const J1939Tp_RxChannelType *RxChannels;
	const J1939Tp_TxChannelType *TxChannels;
} J1939Tp_ConfigType;

#endif
