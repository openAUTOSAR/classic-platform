/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/


#ifndef PDUR_IF_H_
#define PDUR_IF_H_

#include "PduR.h"
#if defined(USE_DET)
#include "Det.h"
#endif

#if PDUR_ZERO_COST_OPERATION == STD_OFF


// PduR Interface used by Upper modules

Std_ReturnType PduR_UpTransmit(PduIdType pduId, const PduInfoType* pduInfoPtr, uint8 serviceId);
Std_ReturnType PduR_UpCancelTransmit(PduIdType pduId, uint8 serviceId);
void PduR_UpChangeParameter(PduIdType pduId, TPParameterType parameter, uint16 value, uint8 serviceId);
Std_ReturnType PduR_UpCancelReceive(PduIdType pduId, uint8 serviceId);

// PduR Interface used by Lower If modules

void PduR_LoIfRxIndication(PduIdType pduId, PduInfoType* pduInfoPtr, uint8 serviceId);
void PduR_LoIfTxConfirmation(PduIdType pduId, uint8 serviceId);
Std_ReturnType PduR_LoIfTriggerTransmit(PduIdType pduId, PduInfoType* pduInfoPtr, uint8 serviceId);

// PduR Interface used by Lower Tp modules

BufReq_ReturnType PduR_LoTpCopyRxData(PduIdType id, PduInfoType* info, PduLengthType* bufferSizePtr, uint8 serviceId);
void PduR_LoTpRxIndication(PduIdType id, NotifResultType result, uint8 serviceId);
BufReq_ReturnType PduR_LoTpStartOfReception(PduIdType id, PduLengthType TpSduLength, PduLengthType* bufferSizePtr, uint8 serviceId);
BufReq_ReturnType PduR_LoTpCopyTxData(PduIdType id, PduInfoType* info, RetryInfoType* retry, PduLengthType* availableDataPtr, uint8 serviceId);
void PduR_LoTpTxConfirmation(PduIdType id, NotifResultType result, uint8 serviceId);


// ARC internal route API

Std_ReturnType PduR_ARC_RouteTransmit(const PduRDestPdu_type * destination, const PduInfoType * pduInfo);
void PduR_ARC_RouteRxIndication(const PduRDestPdu_type * destination, const PduInfoType *PduInfo);
void PduR_ARC_RouteTxConfirmation(const PduRRoutingPath_type *route, uint8 result);
Std_ReturnType PduR_ARC_RouteTriggerTransmit(const PduRRoutingPath_type *route, PduInfoType * pduInfo);
BufReq_ReturnType PduR_ARC_RouteCopyRxData(const PduRDestPdu_type * destination, PduInfoType* info, PduLengthType* bufferSizePtr);
BufReq_ReturnType PduR_ARC_RouteCopyTxData(const PduRRoutingPath_type *route, PduInfoType* info, RetryInfoType* retry, PduLengthType* availableDataPtr);
BufReq_ReturnType PduR_ARC_RouteStartOfReception(const PduRDestPdu_type * destination, PduLengthType TpSduLength, PduLengthType* bufferSizePtr);

// ARC internal TP API

Std_ReturnType PduR_ARC_Transmit(PduIdType PduId, const PduInfoType* PduInfo, uint8 serviceId);
void PduR_ARC_TpRxIndication(PduIdType PduId, NotifResultType Result, uint8 serviceId);
void PduR_ARC_RxIndication(PduIdType PduId, const PduInfoType* PduInfo, uint8 serviceId);
void PduR_ARC_TxConfirmation(PduIdType PduId, uint8 result, uint8 serviceId);
Std_ReturnType PduR_ARC_TriggerTransmit(PduIdType PduId, PduInfoType* PduInfo, uint8 serviceId);

#endif

#endif /* PDUR_IF_H_ */
