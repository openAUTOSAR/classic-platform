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

#include "ComStack_Types.h"

/*
 * Interfaces for callback notifications from PduR and ComM (8.4)
 */
BufReq_ReturnType Dcm_ProvideRxBuffer(PduIdType dcmRxPduId, PduLengthType tpSduLength, PduInfoType **pduInfoPtr);
void Dcm_RxIndication(PduIdType dcmRxPduId, NotifResultType result);
BufReq_ReturnType Dcm_ProvideTxBuffer(PduIdType dcmTxPduId, PduInfoType **pduInfoPtr, PduLengthType length);
void Dcm_TxConfirmation(PduIdType dcmTxPduId, NotifResultType result);
void Dcm_ComM_NoComModeEntered(void);
void Dcm_ComM_SilentComModeEntered(void);
void Dcm_ComM_FullComModeEntered(void);

#endif /*DCM_CBK_H_*/
