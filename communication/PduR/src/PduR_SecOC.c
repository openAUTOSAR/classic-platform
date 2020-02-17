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

#if (PDUR_ZERO_COST_OPERATION == STD_OFF) && (PDUR_SECOC_SUPPORT == STD_ON)


// Autosar4 API

void PduR_SecOCRxIndication(PduIdType pduId, const PduInfoType* pduInfoPtr) {
    /*lint -e{9005,929} exception due api parameters mismatch between calls*/
    PduR_LoIfRxIndication(pduId, (PduInfoType *)pduInfoPtr, 0x71);

}

void PduR_SecOCTxConfirmation(PduIdType pduId) {
    PduR_LoIfTxConfirmation(pduId, 0x72);
}

Std_ReturnType PduR_SecOCTransmit(PduIdType pduId, const PduInfoType* pduInfoPtr) {
    return PduR_UpTransmit(pduId, pduInfoPtr, 0xc9);
}

Std_ReturnType PduR_SecOCCancelTransmit(PduIdType pduId) {
    return PduR_UpCancelTransmit(pduId, 0xca);
}
#endif
