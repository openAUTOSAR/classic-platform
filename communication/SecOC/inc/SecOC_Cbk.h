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

#ifndef SECOC_CBK_H_
#define SECOC_CBK_H_

/* @req SWS_SecOC_00125 */
void SecOC_TpRxIndication( PduIdType id, Std_ReturnType result );

/* @req SWS_SecOC_00152 */
void SecOC_TpTxConfirmation( PduIdType id, Std_ReturnType result );

/* !req SWS_SecOC_00127 */
Std_ReturnType SecOC_TriggerTransmit( PduIdType TxPduId, PduInfoType* PduInfoPtr );

/* @req SWS_SecOC_00128 */
BufReq_ReturnType SecOC_CopyRxData( PduIdType id, const PduInfoType* info, PduLengthType* bufferSizePtr );

/* @req SWS_SecOC_00129 */
BufReq_ReturnType SecOC_CopyTxData( PduIdType id, const PduInfoType* info, RetryInfoType* retry, PduLengthType* availableDataPtr );

/* @req SWS_SecOC_00130 */
BufReq_ReturnType SecOC_StartOfReception( PduIdType id, const PduInfoType* info, PduLengthType TpSduLength, PduLengthType* bufferSizePtr );



#endif /* SECOC_CBK_H_ */
