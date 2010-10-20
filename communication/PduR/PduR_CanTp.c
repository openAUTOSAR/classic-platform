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

#if (PDUR_ZERO_COST_OPERATION == STD_OFF)
#include "Det.h"
#include "debug.h"


BufReq_ReturnType PduR_CanTpProvideRxBuffer(PduIdType CanTpRxPduId, PduLengthType TpSduLength, PduInfoType** PduInfoPtr) {
	BufReq_ReturnType retVal = BUFREQ_NOT_OK;
#if (PDUR_CANTP_SUPPORT == STD_ON)
	/* Gateway and multicast modes not supported. */
	PduRRoutingPath_type *route = &PduRConfig->PduRRoutingTable->PduRRoutingPath[CanTpRxPduId];
	retVal = Dcm_ProvideRxBuffer(route->PduRDestPdu.DestPduId, TpSduLength, PduInfoPtr);
#endif
	return retVal;
}


void PduR_CanTpRxIndication(PduIdType CanTpRxPduId, NotifResultType Result) {
#if (PDUR_CANTP_SUPPORT == STD_ON)

	DEBUG(DEBUG_LOW,"----------------------\n");
	DEBUG(DEBUG_LOW,"PduR_CanTpRxIndication: received indication with id %d and data %d\n", CanTpRxPduId);

	/* Note that there is no support for CAN TP and gateway operation mode */
	PduRRoutingPath_type *route = &PduRConfig->PduRRoutingTable->PduRRoutingPath[CanTpRxPduId];
	Dcm_RxIndication(route->PduRDestPdu.DestPduId, Result); // Send PDU to next receptor.

	DEBUG(DEBUG_LOW,"----------------------\n");
#endif
}


BufReq_ReturnType PduR_CanTpProvideTxBuffer(PduIdType CanTpTxPduId, PduInfoType** PduInfoPtr, uint16 Length) {
	BufReq_ReturnType retVal = BUFREQ_NOT_OK;
#if (PDUR_CANTP_SUPPORT == STD_ON)
	/* Gateway and multicast modes not supported. */
	PduRRoutingPath_type *route = &PduRConfig->PduRRoutingTable->PduRRoutingPath[CanTpTxPduId];
	retVal = Dcm_ProvideTxBuffer(route->PduRSrcPdu.SrcPduId, PduInfoPtr, Length);
#endif
	return retVal;
}


void PduR_CanTpTxConfirmation(PduIdType CanTpTxPduId, NotifResultType Result) {
#if (PDUR_CANTP_SUPPORT == STD_ON)
	PduR_DevCheck(CanTpTxPduId,1,0x0f);

	DEBUG(DEBUG_LOW,"----------------------\n");
	DEBUG(DEBUG_LOW,"PduR_CanIfTxConfirmation: received confirmation with id %d\n", CanTxPduId);

	PduRRoutingPath_type *route = &PduRConfig->PduRRoutingTable->PduRRoutingPath[CanTpTxPduId];
	Dcm_TxConfirmation(route->PduRSrcPdu.SrcPduId, Result); // Forward confirmation

	DEBUG(DEBUG_LOW,"----------------------\n");
#endif
}


#endif
