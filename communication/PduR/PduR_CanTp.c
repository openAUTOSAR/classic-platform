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


#if (PDUR_CANTP_SUPPORT == STD_ON)

#include "PduR_CanTp.h"


BufReq_ReturnType PduR_CanTpProvideRxBuffer(PduIdType CanTpRxPduId, PduLengthType TpSduLength, PduInfoType** PduInfoPtr) {
	/* Gateway and multicast modes not supported. */
	Dcm_ProvideRxBuffer(CanTpTxPduId, PduInfoPtr, Length);
}


void PduR_CanTpRxIndication(PduIdType CanTpRxPduId, NotifResultType Result) {
	Enter(CanTpRxPduId);

	DEBUG(DEBUG_LOW,"----------------------\n");
	DEBUG(DEBUG_LOW,"PduR_CanTpRxIndication: received indication with id %d and data %d\n", CanTpRxPduId);

	/* Note that there is no support for CAN TP and gateway operation mode */
	Dcm_RxIndication(CanTpRxPduId, Result);

	DEBUG(DEBUG_LOW,"----------------------\n");
	Exit();
}


BufReq_ReturnType PduR_CanTpProvideTxBuffer(PduIdType CanTpTxPduId, PduInfoType** PduInfoPtr, uint16 Length) {
	/* Gateway and multicast modes not supported. */
	Dcm_ProvideTxBuffer(CanTpTxPduId, PduInfoPtr, Length);

}
void PduR_CanTpTxConfirmation(PduIdType CanTpTxPduId, NotifResultType Result) {
	Enter(CanTxPduId);
	DevCheck(CanTxPduId,1,0x0f);

	DEBUG(DEBUG_LOW,"----------------------\n");
	DEBUG(DEBUG_LOW,"PduR_CanIfTxConfirmation: received confirmation with id %d\n", CanTxPduId);

	Dcm_TxConfirmation(CanTpTxPduId, Result);

	DEBUG(DEBUG_LOW,"----------------------\n");
	Exit();
}


#endif
