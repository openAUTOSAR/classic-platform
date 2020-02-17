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
#include "PduR_FrNm.h"

#if (PDUR_ZERO_COST_OPERATION == STD_OFF)

// Autosar4 API
/* Need to Modify Service IDs in this function*/

void PduR_FrNmRxIndication(PduIdType pduId, PduInfoType* pduInfoPtr) {
    PduR_LoIfRxIndication(pduId, pduInfoPtr, 0x41);
}

void PduR_FrNmTxConfirmation(PduIdType pduId) {
    PduR_LoIfTxConfirmation(pduId, 0x42);
}

Std_ReturnType PduR_FrNmTriggerTransmit(PduIdType pduId, PduInfoType* pduInfoPtr) {
    return PduR_LoIfTriggerTransmit(pduId, pduInfoPtr, 0x43);
}

#endif
