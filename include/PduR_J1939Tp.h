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



#ifndef PDUR_J1939TP_H_
#define PDUR_J1939TP_H_

#include "PduR.h"

#if PDUR_ZERO_COST_OPERATION == STD_OFF

BufReq_ReturnType PduR_J1939TpProvideTxBuffer(PduIdType J1939TpTxId,PduInfoType** PduInfoPtr, uint16 Length);
void PduR_J1939TpTxConfirmation(PduIdType CanTpTxPduId, NotifResultType Result);

#endif

#endif /* PDUR_CANTP_H_ */
