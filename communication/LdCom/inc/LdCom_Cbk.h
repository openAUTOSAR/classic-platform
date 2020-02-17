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

#ifndef LDCOM_CBK_H_
#define LDCOM_CBK_H_

#include "ComStack_Types.h"
#include "LdCom_Types.h"

 /* @req 4.2.2/SWS_LDCOM_00027*/
BufReq_ReturnType LdCom_CopyTxData( PduIdType id, const PduInfoType* info, RetryInfoType* retry, PduLengthType* availableDataPtr );
 /* @req 4.2.2/SWS_LDCOM_00028*/
void LdCom_TpTxConfirmation( PduIdType id, Std_ReturnType result );
 /* @req 4.2.2/SWS_LDCOM_00029*/
BufReq_ReturnType LdCom_StartOfReception( PduIdType id, const PduInfoType* info, PduLengthType TpSduLength, PduLengthType* bufferSizePtr );
 /* @req 4.2.2/SWS_LDCOM_00030*/
BufReq_ReturnType LdCom_CopyRxData( PduIdType id, const PduInfoType* info, PduLengthType* bufferSizePtr );
 /* @req 4.2.2/SWS_LDCOM_00031*/
void LdCom_TpRxIndication( PduIdType id, Std_ReturnType result );
 /* @req 4.2.2/SWS_LDCOM_00032*/
void LdCom_RxIndication( PduIdType RxPduId, const PduInfoType* PduInfoPtr );
 /* @req 4.2.2/SWS_LDCOM_00056*/
void LdCom_TxConfirmation( PduIdType TxPduId );
 /* @req 4.2.2/SWS_LDCOM_00033*/
Std_ReturnType LdCom_TriggerTransmit( PduIdType TxPduId, PduInfoType* PduInfoPtr );

#endif /* LDCOM_CBK_H_ */
