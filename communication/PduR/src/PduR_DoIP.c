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

#if (PDUR_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif
#include "debug.h"

/* @req PDUR0764 */
#if (PDUR_ZERO_COST_OPERATION == STD_OFF) && (PDUR_DOIP_SUPPORT == STD_ON)

// Autosar4 API

void PduR_DoIPIfRxIndication(PduIdType pduId, PduInfoType* pduInfoPtr) {
    PduR_LoIfRxIndication(pduId, pduInfoPtr, 0x42);
}

void PduR_DoIPIfTxConfirmation(PduIdType pduId) {
    PduR_LoIfTxConfirmation(pduId, 0x40);
}

BufReq_ReturnType PduR_DoIPTpCopyRxData(PduIdType id, PduInfoType* info, PduLengthType* bufferSizePtr) {
    return PduR_LoTpCopyRxData(id, info, bufferSizePtr, 0x44);
}

void PduR_DoIPTpRxIndication(PduIdType id, NotifResultType result) {
    PduR_LoTpRxIndication(id, result, 0x45);
}

BufReq_ReturnType PduR_DoIPTpStartOfReception(PduIdType id, const PduInfoType* info, PduLengthType TpSduLength, PduLengthType* bufferSizePtr) {
    return PduR_LoTpStartOfReception(id, TpSduLength, bufferSizePtr, 0x46);
    (void)info;
}

BufReq_ReturnType PduR_DoIPTpCopyTxData(PduIdType id, PduInfoType* info, RetryInfoType* retry, PduLengthType* availableDataPtr ) {
    return PduR_LoTpCopyTxData(id, info, retry, availableDataPtr, 0x43);
}

void PduR_DoIPTpTxConfirmation(PduIdType id, NotifResultType result) {
    PduR_ARC_TpTxConfirmation(id, result, 0x48);
}


#endif
