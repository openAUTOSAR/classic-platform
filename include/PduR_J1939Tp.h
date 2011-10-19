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
void PduR_J1939TpTxConfirmation(PduIdType CanTpTxPduId, NotifResultType Result);
void PduR_J1939TpRxIndication(PduIdType id, NotifResultType Result);

/* autosar 4 api */
BufReq_ReturnType PduR_J1939TpCopyTxData(PduIdType id, PduInfoType* info, RetryInfoType* retry, PduLengthType* availableDataPtr);
BufReq_ReturnType PduR_J1939TpCopyRxData(PduIdType id, PduInfoType* info, PduLengthType* bufferSizePtr);
BufReq_ReturnType PduR_J1939TpStartOfReception(PduIdType id, PduLengthType TpSduLength, PduLengthType* bufferSizePtr);
#endif

#endif /* PDUR_CANTP_H_ */
