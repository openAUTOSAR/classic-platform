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

#ifndef PDUR_SOAD_H_
#define PDUR_SOAD_H_

#include "PduR.h"

#if (PDUR_ZERO_COST_OPERATION == STD_OFF) && (PDUR_SOAD_SUPPORT == STD_ON)

/* SoAd acts as interface module */
void PduR_SoAdIfRxIndication(PduIdType pduId, PduInfoType* pduInfoPtr);
void PduR_SoAdIfTxConfirmation(PduIdType pduId);
Std_ReturnType PduR_SoAdIfTriggerTransmit(PduIdType pduId, PduInfoType* pduInfoPtr);

/* SoAd acts as transport protocol module */
BufReq_ReturnType PduR_SoAdTpCopyRxData(PduIdType id, PduInfoType* info, PduLengthType* bufferSizePtr);
void PduR_SoAdTpRxIndication(PduIdType id, NotifResultType result);
BufReq_ReturnType PduR_SoAdTpStartOfReception(PduIdType id, PduLengthType TpSduLength, PduLengthType* bufferSizePtr);
BufReq_ReturnType PduR_SoAdTpCopyTxData(PduIdType id, PduInfoType* info, RetryInfoType* retry, PduLengthType* availableDataPtr );
void PduR_SoAdTpTxConfirmation(PduIdType id, NotifResultType result);

#endif

#endif /* PDUR_SOAD_H_ */
