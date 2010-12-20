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


#ifndef CANNM_CONFIGTYPES_H_
#define CANNM_CONFIGTYPES_H_

typedef enum {
	CANNM_PDU_BYTE_0 = 0x00,
	CANNM_PDU_BYTE_1 = 0x01,
	CANNM_PDU_OFF = 0xFF
} CanNm_PduBytePositionType;

/** @req CANNM202  @req CANNM203 */
typedef struct {
	const boolean					Active;
	const NetworkHandleType			NmNetworkHandle;
	const uint8						NodeId;
	const uint32					MainFunctionPeriod;
	const uint32					TimeoutTime;
	const uint32					RepeatMessageTime;      /**< @req CANNM148.1 */
	const uint32					WaitBusSleepTime;       /**< @req CANNM148.2 */
	const uint32					MessageCycleTime;       /**< @req CANNM148.3 */
	const uint32					MessageCycleOffsetTime;
	const PduIdType					CanIfPduId;
	const uint8						PduLength;              /**< @req CANNM076 */
	const CanNm_PduBytePositionType	NidPosition;            /**< @req CANNM074 */
	const CanNm_PduBytePositionType	CbvPosition;            /**< @req CANNM075 */
} CanNm_ChannelType;

typedef struct {
	const CanNm_ChannelType* 			Channels;
} CanNm_ConfigType;

#endif /* CANNM_CONFIGTYPES_H_ */
