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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.2 */
/** @tagSettings DEFAULT_ARCHITECTURE=GENERIC */


#ifndef PDUR_SECOC_H_
#define PDUR_SECOC_H_

#include "PduR.h"
#if (PDUR_ZERO_COST_OPERATION == STD_OFF) && (PDUR_SECOC_SUPPORT == STD_ON)
void PduR_SecOCRxIndication(PduIdType pduId, const PduInfoType* pduInfoPtr);
void PduR_SecOCTxConfirmation(PduIdType pduId);
Std_ReturnType PduR_SecOCTransmit(PduIdType pduId, const PduInfoType* pduInfoPtr);
Std_ReturnType PduR_SecOCCancelTransmit(PduIdType pduId);

#endif

#endif /* PDUR_SECOC_H_ */
