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

#if (PDUR_ZERO_COST_OPERATION == STD_OFF) && (PDUR_CANIF_SUPPORT == STD_ON)

void PduR_CanIfRxIndication(PduIdType CanRxPduId,const PduInfoType* PduInfoPtr) {
	PduR_ARC_RxIndication(CanRxPduId, PduInfoPtr, 0x01);
}

void PduR_CanIfTxConfirmation(PduIdType CanTxPduId) {
	uint8 dummy = 0;
	PduR_ARC_TxConfirmation(CanTxPduId,dummy,0x02);
}

#endif
