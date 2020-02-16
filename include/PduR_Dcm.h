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

#ifndef PDUR_DCM_H_
#define PDUR_DCM_H_

#include "PduR.h"

#if (PDUR_ZERO_COST_OPERATION == STD_OFF) && (PDUR_DCM_SUPPORT == STD_ON)

Std_ReturnType PduR_DcmTransmit(PduIdType pduId, const PduInfoType* pduInfoPtr);
Std_ReturnType PduR_DcmCancelTransmit(PduIdType pduId);
void PduR_DcmChangeParameter(PduIdType pduId, TPParameterType parameter, uint16 value);
Std_ReturnType PduR_DcmCancelReceive(PduIdType pduId);

#endif

#endif /*PDUR_DCM_H_*/



