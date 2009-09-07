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








#include "PduR_Ipdum.h"

#ifdef PDUR_IPDUM_SUPPORT

Std_ReturnType PduR_IpdumTransmit(PduIdType IpdumTxPduId, const PduInfoType* PduInfoPtr) {

#ifdef PDUR_DEV_ERROR_DETECT
	DevCheck(IpdumTxPduId,PduInfoPtr,0x19);
#endif

}

void PduR_IpdumTxConfirmation(PduIdType IpdumLoTxPduId) {

#ifdef PDUR_DEV_ERROR_DETECT
	DevCheck(IpdumLoTxPduId,0,0x1a);
#endif

}

void PduR_IpdumRxIndication(PduIdType IpdumLoRxPduId, const uint8* IpdumSduPtr) {

#ifdef PDUR_DEV_ERROR_DETECT
	DevCheck(IpdumLoRxPduId,IpdumSduPtr,0x1b);
#endif

}

#endif
