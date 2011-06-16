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

#if (PDUR_ZERO_COST_OPERATION == STD_OFF) && (PDUR_LINIF_SUPPORT == STD_ON)

void PduR_LinIfRxIndication(PduIdType LinRxPduId, const PduInfoType* PduInfoPtr) {
	PduR_ARC_RxIndication(LinRxPduId, PduInfoPtr, 0x0e);
}

void PduR_LinIfTxConfirmation(PduIdType LinTxPduId) {
	PduR_ARC_TxConfirmation(LinTxPduId, NULL, 0x0f);
}

Std_ReturnType PduR_LinIfTriggerTransmit(PduIdType LinTxPduId, PduInfoType* PduInfoPtr) {
	return PduR_ARC_TriggerTransmit(LinTxPduId, PduInfoPtr, 0x10);
}

#endif
