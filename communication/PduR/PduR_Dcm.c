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


#if PDUR_DCM_SUPPORT == STD_ON && PDUR_ZERO_COST_OPERATION

#include "PduR_Dcm.h"

Std_ReturnType PduR_DcmTransmit(PduIdType DcmTxPduId, const PduInfoType* PduInfoPtr) {
	DevCheck(DcmTxPduId,PduInfoPtr,0x15, E_NOT_OK);

	//DEBUG(DEBUG_LOW,"PduR_ComTransmit: received transmit request with id %d and data %d\n", ComTxPduId, *PduInfoPtr->SduDataPtr);
	PduRRoutingPath_type *route = &PduRConfig->PduRRoutingTable->PduRRoutingPath[ComTxPduId];
	Std_ReturnType retVal = CanTp_Transmit(route->PduRDestPdu.DestPduId, PduInfoPtr);

	return retVal;
}

#endif
