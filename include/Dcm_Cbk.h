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








#ifndef DCM_CBK_H_
#define DCM_CBK_H_

//lint -e451 //451 PC-Lint OK. Sl� av regel helt?
#include "ComStack_Types.h"

/*
 * Interfaces for callback notifications from PduR and ComM (8.4)
 */
BufReq_ReturnType Dcm_CopyRxData(PduIdType dcmRxPduId, PduInfoType *pduInfoPtr, PduLengthType *rxBufferSizePtr); /** @req DCM556 */
BufReq_ReturnType Dcm_StartOfReception(PduIdType dcmRxPduId, PduLengthType tpSduLength, PduLengthType *rxBufferSizePtr); /** @req DCM094 */
void Dcm_TpRxIndication(PduIdType dcmRxPduId, NotifResultType result); /** @req DCM093 */
BufReq_ReturnType Dcm_CopyTxData(PduIdType dcmTxPduId, PduInfoType *pduInfoPtr, RetryInfoType *retryInfoPtr, PduLengthType *txDataCntPtr); /** @req DCM092 */
void Dcm_TpTxConfirmation(PduIdType dcmTxPduId, NotifResultType result); /** @req DCM351 */
void Dcm_ComM_NoComModeEntered( uint8 NetworkId ); /** @req DCM356 */
void Dcm_ComM_SilentComModeEntered( uint8 NetworkId ); /** @req DCM358 */
void Dcm_ComM_FullComModeEntered( uint8 NetworkId ); /** @req DCM360 */


#endif /*DCM_CBK_H_*/
