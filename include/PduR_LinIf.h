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

#ifndef PDUR_LINIF_H_
#define PDUR_LINIF_H_

#include "PduR.h"

#if (PDUR_ZERO_COST_OPERATION == STD_OFF) && (PDUR_LINIF_SUPPORT == STD_ON)

void PduR_LinIfRxIndication(PduIdType pduId, PduInfoType* pduInfoPtr);
void PduR_LinIfTxConfirmation(PduIdType pduId);
Std_ReturnType PduR_LinIfTriggerTransmit(PduIdType pduId, PduInfoType* pduInfoPtr);

#endif

#endif /*PDUR_LINIF_H_*/
