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
#if defined(USE_DET)
#include "Det.h"
#endif
#include "debug.h"

#if (PDUR_ZERO_COST_OPERATION == STD_OFF) && (PDUR_J1939TP_SUPPORT == STD_ON)

void PduR_J1939TpRxIndication(PduIdType id, NotifResultType Result) {
#if PDUR_COM_SUPPORT == STD_OFF
	return BUFREQ_NOT_OK;
#else
	const PduRRoutingPath_type *route = PduRConfig->RoutingPaths[id];
	Com_TpRxIndication(route->PduRDestPdus[0]->DestPduId, Result);
#endif
}

void PduR_J1939TpTxConfirmation(PduIdType J1939TpTxId, NotifResultType Result) {
#if PDUR_COM_SUPPORT == STD_OFF
	return;
#else
	const PduRRoutingPath_type *route = PduRConfig->RoutingPaths[J1939TpTxId];
	Com_TpTxConfirmation(route->SrcPduId, Result);
#endif
}

/* autosar 4 api */
BufReq_ReturnType PduR_J1939TpCopyTxData(PduIdType id, PduInfoType* info, RetryInfoType* retry, PduLengthType* availableDataPtr) {
#if PDUR_COM_SUPPORT == STD_OFF
	return BUFREQ_NOT_OK;
#else
	const PduRRoutingPath_type *route = PduRConfig->RoutingPaths[id];
	return Com_CopyTxData(route->SrcPduId, info,0 ,availableDataPtr);
#endif
}
BufReq_ReturnType PduR_J1939TpCopyRxData(PduIdType id, PduInfoType* info, PduLengthType* bufferSizePtr) {
#if PDUR_COM_SUPPORT == STD_OFF
	return BUFREQ_NOT_OK;
#else
	const PduRRoutingPath_type *route = PduRConfig->RoutingPaths[id];
	// Limitation, Just sends to the first destination
	return Com_CopyRxData(route->PduRDestPdus[0]->DestPduId, info,bufferSizePtr);
#endif
}

BufReq_ReturnType PduR_J1939TpStartOfReception(PduIdType id, PduLengthType TpSduLength, PduLengthType* bufferSizePtr) {
#if PDUR_COM_SUPPORT == STD_OFF
	return BUFREQ_NOT_OK;
#else
	const PduRRoutingPath_type *route = PduRConfig->RoutingPaths[id];
	// Limitation, Just sends to the first destination
	return Com_StartOfReception(route->PduRDestPdus[0]->DestPduId, TpSduLength, bufferSizePtr);
#endif
}

#endif
