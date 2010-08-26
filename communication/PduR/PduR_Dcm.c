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

#include "debug.h"

#if (PDUR_DCM_SUPPORT == STD_ON)

Std_ReturnType PduR_DcmTransmit(PduIdType DcmTxPduId, const PduInfoType* PduInfoPtr) {
	BufReq_ReturnType retVal = BUFREQ_NOT_OK;
#if (PDUR_DCM_SUPPORT == STD_ON)
	PduR_DevCheck(DcmTxPduId,PduInfoPtr,0x15, E_NOT_OK);

	//DEBUG(DEBUG_LOW,"PduR_ComTransmit: received transmit request with id %d and data %d\n", ComTxPduId, *PduInfoPtr->SduDataPtr);


	PduRRoutingPath_type *route = &PduRConfig->PduRRoutingTable->PduRRoutingPath[DcmTxPduId];
	retVal = route->FctPtrs.TargetTransmitFctPtr(route->PduRDestPdu.DestPduId, PduInfoPtr);
#endif
	return retVal;
}

#endif

#endif
