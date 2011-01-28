/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/








#ifndef DCM_CBK_H_
#define DCM_CBK_H_

//lint -e451 //451 PC-Lint OK. Slå av regel helt?
#include "ComStack_Types.h"

/*
 * Interfaces for callback notifications from PduR and ComM (8.4)
 */
BufReq_ReturnType Dcm_ProvideRxBuffer(PduIdType dcmRxPduId, PduLengthType tpSduLength, PduInfoType **pduInfoPtr); /** @req DCM094 */
void Dcm_RxIndication(PduIdType dcmRxPduId, NotifResultType result); /** @req DCM093 */
BufReq_ReturnType Dcm_ProvideTxBuffer(PduIdType dcmTxPduId, PduInfoType **pduInfoPtr, PduLengthType length); /** @req DCM092 */
void Dcm_TxConfirmation(PduIdType dcmTxPduId, NotifResultType result); /** @req DCM351 */
void Dcm_ComM_NoComModeEntered(void); /** @req DCM356 */
void Dcm_ComM_SilentComModeEntered(void); /** @req DCM358 */
void Dcm_ComM_FullComModeEntered(void); /** @req DCM360 */

#endif /*DCM_CBK_H_*/
