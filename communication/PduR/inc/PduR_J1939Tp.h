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

#ifndef PDUR_J1939TP_H_
#define PDUR_J1939TP_H_

#include "PduR.h"

#if (PDUR_ZERO_COST_OPERATION == STD_OFF) && (PDUR_J1939TP_SUPPORT == STD_ON)

BufReq_ReturnType PduR_J1939TpCopyRxData(PduIdType id, PduInfoType* info, PduLengthType* bufferSizePtr);
void PduR_J1939TpRxIndication(PduIdType id, NotifResultType result);

/* See item SWS_PduR_00507 in version 4.1 of PduR */
BufReq_ReturnType PduR_J1939TpStartOfReception
(
        PduIdType      id,
        PduInfoType*   info,         // new in v4.1 of PduR
        PduLengthType  TpSduLength,
        PduLengthType* bufferSizePtr
);


BufReq_ReturnType PduR_J1939TpCopyTxData(PduIdType id, PduInfoType* info, RetryInfoType* retry, PduLengthType* availableDataPtr );
void PduR_J1939TpTxConfirmation(PduIdType id, NotifResultType result);

#endif

#endif /* PDUR_CANTP_H_ */
