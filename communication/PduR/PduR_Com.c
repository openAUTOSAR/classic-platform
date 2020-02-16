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
#if (PDUR_ZERO_COST_OPERATION == STD_OFF) && (PDUR_COM_SUPPORT == STD_ON)


// Autosar4 API

Std_ReturnType PduR_ComTransmit(PduIdType pduId, const PduInfoType* pduInfoPtr) {
	return PduR_UpTransmit(pduId, pduInfoPtr, 0x89);
}

Std_ReturnType PduR_ComCancelTransmit(PduIdType pduId) {
    return PduR_UpCancelTransmit(pduId, 0x8a);
}

void PduR_ComChangeParameter(PduIdType pduId, TPParameterType parameter, uint16 value) {
    PduR_UpChangeParameter(pduId, parameter, value, 0x8b);
}

Std_ReturnType PduR_ComCancelReceive(PduIdType pduId) {
    return PduR_UpCancelReceive(pduId, 0x8c);
}

#endif
