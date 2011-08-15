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




#ifndef PDUR_IF_H_
#define PDUR_IF_H_

#include "PduR.h"
#include "Det.h"

#if PDUR_ZERO_COST_OPERATION == STD_OFF

Std_ReturnType PduR_ARC_RouteTransmit(const PduRDestPdu_type * destination, const PduInfoType * pduInfo);
void PduR_ARC_RouteRxIndication(const PduRDestPdu_type * destination, const PduInfoType *PduInfo);
void PduR_ARC_RouteTxConfirmation(const PduRRoutingPath_type *route, uint8 result);
Std_ReturnType PduR_ARC_RouteTriggerTransmit(const PduRRoutingPath_type *route, PduInfoType * pduInfo);
BufReq_ReturnType PduR_ARC_RouteProvideRxBuffer(const PduRDestPdu_type * destination, PduLengthType TpSduLength, PduInfoType** PduInfoPtr);
BufReq_ReturnType PduR_ARC_RouteProvideTxBuffer(const PduRRoutingPath_type *route, PduLengthType TpSduLength, PduInfoType** PduInfoPtr);


Std_ReturnType PduR_ARC_Transmit(PduIdType PduId, const PduInfoType* PduInfo, uint8 serviceId);
void PduR_ARC_RxIndication(PduIdType PduId, const PduInfoType* PduInfo, uint8 serviceId);
void PduR_ARC_TxConfirmation(PduIdType PduId, uint8 result, uint8 serviceId);
Std_ReturnType PduR_ARC_TriggerTransmit(PduIdType PduId, PduInfoType* PduInfo, uint8 serviceId);
BufReq_ReturnType PduR_ARC_ProvideRxBuffer(PduIdType PduId, PduLengthType TpSduLength, PduInfoType** PduInfoPtr, uint8 serviceId);
BufReq_ReturnType PduR_ARC_ProvideTxBuffer(PduIdType PduId, PduInfoType** PduInfoPtr, uint16 Length, uint8 serviceId);

// Prototypes for functions used locally in file PduR_Logic.c (prototypes needed to remove lint errors)
BufReq_ReturnType PduR_ARC_AllocateRxBuffer(PduIdType PduId, PduLengthType TpSduLength);
BufReq_ReturnType PduR_ARC_AllocateTxBuffer(PduIdType PduId, uint16 length);
BufReq_ReturnType PduR_ARC_ReleaseRxBuffer(PduIdType PduId);
BufReq_ReturnType PduR_ARC_ReleaseTxBuffer(PduIdType PduId);
void PduR_ARC_RxIndicationDirect(const PduRDestPdu_type * destination, const PduInfoType *PduInfo);
void PduR_ARC_RxIndicationTT(const PduRDestPdu_type * destination, const PduInfoType *PduInfo, uint16 BufferLength);

#endif

#endif /* PDUR_IF_H_ */
