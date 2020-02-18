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

#ifndef PDUR_DOIP_H_
#define PDUR_DOIP_H_


void PduR_DoIPTpRxIndication(PduIdType id, NotifResultType result);
BufReq_ReturnType PduR_DoIPTpCopyRxData(PduIdType id, PduInfoType* info, PduLengthType* bufferSizePtr);
BufReq_ReturnType PduR_DoIPTpStartOfReception(PduIdType id,const PduInfoType* info, PduLengthType TpSduLength, PduLengthType* bufferSizePtr);
BufReq_ReturnType PduR_DoIPTpCopyTxData(PduIdType id, PduInfoType* info, RetryInfoType* retry, PduLengthType* availableDataPtr);

void PduR_DoIpIfRxIndication(PduIdType pduId, PduInfoType* pduInfoPtr);
void PduR_DoIPTpTxConfirmation(PduIdType id, Std_ReturnType result);
void PduR_DoIPIfTxConfirmation(PduIdType id, Std_ReturnType result);

#endif /* PDUR_DOIP_H_ */
