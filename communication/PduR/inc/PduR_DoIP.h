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


/* @req PDUR216 */

#ifndef PDUR_DOIP_H_
#define PDUR_DOIP_H_

#include "PduR.h"

#if (PDUR_ZERO_COST_OPERATION == STD_OFF) && (PDUR_DOIP_SUPPORT == STD_ON)

/* DoIP acts as interface module */
void PduR_DoIPIfRxIndication(PduIdType pduId, PduInfoType* pduInfoPtr);
void PduR_DoIPIfTxConfirmation(PduIdType pduId);
Std_ReturnType PduR_DoIPIfTriggerTransmit(PduIdType pduId, PduInfoType* pduInfoPtr);

/* DoIP acts as transport protocol module */
BufReq_ReturnType PduR_DoIPTpCopyRxData(PduIdType id, PduInfoType* info, PduLengthType* bufferSizePtr);
void PduR_DoIPTpRxIndication(PduIdType id, NotifResultType result);
BufReq_ReturnType PduR_DoIPTpStartOfReception(PduIdType id, const PduInfoType* info,PduLengthType TpSduLength, PduLengthType* bufferSizePtr);
BufReq_ReturnType PduR_DoIPTpCopyTxData(PduIdType id, PduInfoType* info, RetryInfoType* retry, PduLengthType* availableDataPtr );
void PduR_DoIPTpTxConfirmation(PduIdType id, NotifResultType result);

#endif

#endif /* PDUR_DOIP_H_ */
