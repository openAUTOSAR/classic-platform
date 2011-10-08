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

#if (PDUR_ZERO_COST_OPERATION == STD_OFF) && (PDUR_J1939TP_SUPPORT == STD_ON)

BufReq_ReturnType PduR_J1939TpProvideRxBuffer(PduIdType J1939TpTxId, PduLengthType sduLength, PduInfoType **pduInfoPtr) {
	return PduR_ARC_ProvideRxBuffer(J1939TpTxId, sduLength, pduInfoPtr, 0x03);
}


void PduR_J1939TpRxIndication(PduIdType id, NotifResultType Result) {
	PduInfoType PduInfo = {
		.SduDataPtr = &Result,
		.SduLength = 0 // To fix PC-Lint 785
	};
	PduR_ARC_RxIndication(id, &PduInfo, 0x04);
}

BufReq_ReturnType PduR_J1939TpProvideTxBuffer(PduIdType J1939TpTxId,PduInfoType** PduInfoPtr, uint16 Length) {
	return PduR_ARC_ProvideTxBuffer(J1939TpTxId, PduInfoPtr, Length, 0x03);
}


void PduR_J1939TpTxConfirmation(PduIdType J1939TpTxId, NotifResultType Result) {
	PduR_ARC_TxConfirmation(J1939TpTxId, Result, 0x0f);
}

/* autosar 4 api */
BufReq_ReturnType PduR_J1939TpCopyTxData(PduIdType id, PduInfoType* info, RetryInfoType* retry, PduLengthType* availableDataPtr) {
#if PDUR_COM_SUPPORT == STD_OFF
	return BUFREQ_NOT_OK;
#endif
	const PduRRoutingPath_type *route = PduRConfig->RoutingPaths[id];
	return Com_CopyTxData(route->SrcModule, info,0 ,availableDataPtr);
}

#endif
