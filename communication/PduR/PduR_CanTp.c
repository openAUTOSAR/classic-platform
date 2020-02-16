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


#include "PduR.h"

/* @req PDUR764 */
#if (PDUR_ZERO_COST_OPERATION == STD_OFF) && (PDUR_CANTP_SUPPORT == STD_ON)


// Autosar4 API

BufReq_ReturnType PduR_CanTpCopyRxData(PduIdType id, PduInfoType* info, PduLengthType* bufferSizePtr) {
    return PduR_LoTpCopyRxData(id, info, bufferSizePtr, 0x04);
}

void PduR_CanTpRxIndication(PduIdType id, NotifResultType result) {
    PduR_LoTpRxIndication(id, result, 0x05);
}

BufReq_ReturnType PduR_CanTpStartOfReception(PduIdType id, PduLengthType TpSduLength, PduLengthType* bufferSizePtr) {
    return PduR_LoTpStartOfReception(id, TpSduLength, bufferSizePtr, 0x06);
}

BufReq_ReturnType PduR_CanTpCopyTxData(PduIdType id, PduInfoType* info, RetryInfoType* retry, PduLengthType* availableDataPtr ) {
    return PduR_LoTpCopyTxData(id, info, retry, availableDataPtr, 0x07);
}

void PduR_CanTpTxConfirmation(PduIdType id, NotifResultType result) {
    PduR_LoTpTxConfirmation(id, result, 0x08);
}

#endif
