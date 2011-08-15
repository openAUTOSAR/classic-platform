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


#include "PduR.h"
#include "Det.h"
#include "debug.h"

#if (PDUR_ZERO_COST_OPERATION == STD_OFF) && (PDUR_CANTP_SUPPORT == STD_ON)

BufReq_ReturnType PduR_CanTpProvideRxBuffer(PduIdType CanTpRxPduId, PduLengthType TpSduLength, PduInfoType** PduInfoPtr) {
	return PduR_ARC_ProvideRxBuffer(CanTpRxPduId, TpSduLength, PduInfoPtr, 0x03);
}


void PduR_CanTpRxIndication(PduIdType CanTpRxPduId, NotifResultType Result) {
	PduInfoType PduInfo = {
		.SduDataPtr = &Result,
		.SduLength = 0 // To fix PC-Lint 785
	};
	PduR_ARC_RxIndication(CanTpRxPduId, &PduInfo, 0x04);
}

BufReq_ReturnType PduR_CanTpProvideTxBuffer(PduIdType CanTpTxPduId, PduInfoType** PduInfoPtr, uint16 Length) {
	return PduR_ARC_ProvideTxBuffer(CanTpTxPduId, PduInfoPtr, Length, 0x03);
}


void PduR_CanTpTxConfirmation(PduIdType CanTpTxPduId, NotifResultType Result) {
	PduR_ARC_TxConfirmation(CanTpTxPduId, Result, 0x0f);
}

#endif
