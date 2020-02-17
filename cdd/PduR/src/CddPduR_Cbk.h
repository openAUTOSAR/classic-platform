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

#ifndef CDDPDUR_CBK_H_
#define CDDPDUR_CBK_H_

void CddPduR_TpRxIndication(PduIdType id, Std_ReturnType result);
void CddPduR_TpTxConfirmation(PduIdType id, Std_ReturnType result);

void CddPduR_RxIndication(PduIdType id, PduInfoType* pduInfoPtr);
void CddPduR_TxConfirmation(PduIdType id, Std_ReturnType result);

BufReq_ReturnType CddPduR_CopyTxData(PduIdType id, const PduInfoType* pduInfoPtr, RetryInfoType* retryInfoPtr, PduLengthType* availableDataPtr);
BufReq_ReturnType CddPduR_CopyRxData(PduIdType id, const PduInfoType* pduInfoPtr, PduLengthType* bufferSizePtr );

BufReq_ReturnType CddPduR_StartOfReception(	PduIdType id,
                                            const PduInfoType* infoPtr,
                                            PduLengthType tpSduLength,
                                            PduLengthType* bufferSizePtr);

#endif /* CDDPDUR_CBK_H_ */
