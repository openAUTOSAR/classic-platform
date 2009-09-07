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








#include "Det.h"
#include "PduR_CanIf.h"
#include "PduR_If.h"
#include "Trace.h"

#ifndef PDUR_ZERO_COST_OPERATION
#ifdef PDUR_CANIF_SUPPORT


void PduR_CanIfRxIndication(PduIdType CanRxPduId,const uint8* CanSduPtr) {
	Enter(CanRxPduId);
	DevCheck(CanRxPduId,CanSduPtr,0x0e);

	DEBUG(DEBUG_LOW,"----------------------\n");
	DEBUG(DEBUG_LOW,"PduR_CanIfRxIndication: received indication with id %d and data %d\n", CanRxPduId, *CanSduPtr);

	PduR_LoIfRxIndication(CanRxPduId, CanSduPtr);

	DEBUG(DEBUG_LOW,"----------------------\n");
	Exit();
}

void PduR_CanIfTxConfirmation(PduIdType CanTxPduId) {
	Enter(CanTxPduId);
	DevCheck(CanTxPduId,1,0x0f);

	DEBUG(DEBUG_LOW,"----------------------\n");
	DEBUG(DEBUG_LOW,"PduR_CanIfTxConfirmation: received confirmation with id %d\n", CanTxPduId);

	PduR_LoIfTxConfirmation(CanTxPduId);

	DEBUG(DEBUG_LOW,"----------------------\n");
	Exit();
}

#endif
#endif
