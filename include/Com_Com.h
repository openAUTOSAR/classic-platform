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








#ifndef COM_COM_H_
#define COM_COM_H_

#include <Com.h>
#include <PduR.h>

uint8 Com_SendSignal(Com_SignalIdType SignalId, const void *SignalDataPtr);
uint8 Com_ReceiveSignal(Com_SignalIdType SignalId, void* SignalDataPtr);

Std_ReturnType Com_TriggerTransmit(PduIdType ComTxPduId, uint8 *SduPtr);

void Com_TriggerIPduSend(PduIdType ComTxPduId);

void Com_RxIndication(PduIdType ComRxPduId, const uint8* SduPtr); // TODO: Parameter SduPtr should be const PduInfoType* PduInfoPtr
void Com_TxConfirmation(PduIdType ComTxPduId);


/* Signal Groups */

Std_ReturnType Com_SendSignalGroup(Com_SignalGroupIdType SignalGroupId);
Std_ReturnType Com_ReceiveSignalGroup(Com_SignalGroupIdType SignalGroupId);

void Com_UpdateShadowSignal(Com_SignalIdType SignalId, const void *SignalDataPtr);
void Com_ReceiveShadowSignal(Com_SignalIdType SignalId, void *SignalDataPtr);


#endif /* COM_COM_H_ */
