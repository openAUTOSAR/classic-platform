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








#ifndef PDUR_SOAD_H_
#define PDUR_SOAD_H_

#include "PduR.h"

#if PDUR_ZERO_COST_OPERATION == STD_OFF

/* SoAd acts as transport protocol module */
BufReq_ReturnType PduR_SoAdTpProvideRxBuffer(PduIdType dcmRxPduId, PduLengthType sduLength, PduInfoType **pduInfoPtr);
void PduR_SoAdTpRxIndication(PduIdType dcmRxPduId, NotifResultType result);
BufReq_ReturnType PduR_SoAdTpProvideTxBuffer(PduIdType dcmTxPduId, PduInfoType **pduInfoPtr, PduLengthType length);
void PduR_SoAdTpTxConfirmation(PduIdType dcmTxPduId, NotifResultType result);

/* SoAd acts as interface module */
void PduR_SoAdIfRxIndication(PduIdType RxPduId, const uint8* SduPtr);
void PduR_SoAdIfTxConfirmation(PduIdType ComTxPduId);
void PduR_SoAdIfTriggerTransmit(PduIdType TxPduId, uint8 *SduPtr);

#endif

#endif /* PDUR_SOAD_H_ */
