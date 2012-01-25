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








#ifndef COM_H_
#define COM_H_

#include "Std_Types.h"
#include "ComStack_Types.h"


#define COM_SW_MAJOR_VERSION   1
#define COM_SW_MINOR_VERSION   2
#define COM_SW_PATCH_VERSION   0

#include "Com_Cfg.h"
#include "Com_Types.h"
#include "Com_PbCfg.h"
//#include "Com_Internal.h" //TODO: Remove completely?
#include "Com_Com.h"
#include "Com_Sched.h"

//-------------------------------------------------------------------
// From OSEK_VDX spec...
//
//typedef uint32 MessageIdentifier;
// TODO: Have no idea here..
//typedef void * ApplicationDataRef;






//-------------------------------------------------------------------

// From OSEK_VDX

/* The service SendMessage updates the message object identified by
 * <Message> with the application message referenced by the
 * <DataRef> parameter.
 *
 * Internal communication:
 * The message <Message> is routed to the receiving part of the IL.
  */
// Update 2008-10-30, SendMessage and ReceiveMessage should not be required. ensured by RTE. COM013
//StatusType SendMessage(MessageIdentifier , ApplicationDataRef );

// The service  ReceiveMessage updates the application message
// referenced by <DataRef> with the data in the message object
// identified by <Message>. It resets all flags (Notification classes 1 and
// 3) associated with <Message>.
//StatusType ReceiveMessage ( MessageIdentifier , ApplicationDataRef );


// From Autosar
void Com_Init(const Com_ConfigType * config);
void Com_DeInit(void);

void Com_IpduGroupStart(Com_PduGroupIdType IpduGroupId, boolean Initialize);
void Com_IpduGroupStop(Com_PduGroupIdType IpduGroupId);

/* Autosar 4 api */
BufReq_ReturnType Com_CopyTxData(PduIdType PduId, PduInfoType* PduInfoPtr, RetryInfoType* RetryInfoPtr, PduLengthType* TxDataCntPtr);
BufReq_ReturnType Com_CopyRxData(PduIdType PduId, const PduInfoType* PduInfoPtr, PduLengthType* RxBufferSizePtr);
BufReq_ReturnType Com_StartOfReception(PduIdType ComRxPduId, PduLengthType TpSduLength, PduLengthType* RxBufferSizePtr);
void Com_TpRxIndication(PduIdType PduId, NotifResultType Result);
void Com_TpTxConfirmation(PduIdType PduId, NotifResultType Result);


extern ComSignalEndianess_type Com_SystemEndianness;

#define COM_BUSY 0x81
#define COM_SERVICE_NOT_AVAILABLE 0x80
#endif /*COM_H_*/
