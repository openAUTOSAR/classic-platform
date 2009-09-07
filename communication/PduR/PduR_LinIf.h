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








#ifndef PDUR_LINIF_H_
#define PDUR_LINIF_H_

#include "PduR.h"


#ifdef PDUR_LINIF_SUPPORT
#ifndef PDUR_ZERO_COST_OPERATION

void PduR_LinIfRxIndication(PduIdType LinRxPduId,const uint8* LinSduPtr);
void PduR_LinIfTxConfirmation(PduIdType LinTxPduId);
void PduR_LinIfTriggerTransmit(PduIdType LinTxPduId,uint8* LinSduPtr);

#else

#define PduR_LinIfRxIndication Com_RxIndication
#define PduR_LinIfTxConfirmation Com_TxConfirmation
#define PduR_LinIfTriggerTransmit Com_TriggerTransmit

#endif
#endif

#endif /*PDUR_LINIF_H_*/
