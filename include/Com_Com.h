/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/








#ifndef COM_COM_H_
#define COM_COM_H_

#include <Com.h>
#include <PduR.h>

/* @req COM197 */
uint8 Com_SendSignal(Com_SignalIdType SignalId, const void *SignalDataPtr);
/* @req COM198 */
uint8 Com_ReceiveSignal(Com_SignalIdType SignalId, void* SignalDataPtr);
/* @req COM690 */
uint8 Com_ReceiveDynSignal(Com_SignalIdType SignalId, void* SignalDataPtr, uint16* Length);
/* @req COM627 */
uint8 Com_SendDynSignal(Com_SignalIdType SignalId, const void* SignalDataPtr, uint16 Length);
/* @req COM001 */
Std_ReturnType Com_TriggerTransmit(PduIdType TxPduId, PduInfoType *PduInfoPtr);
/* @req COM348 */
void Com_TriggerIPDUSend(PduIdType PduId);
/* @req COM123 */
void Com_RxIndication(PduIdType RxPduId, PduInfoType* PduInfoPtr);
/* @req COM124 */
void Com_TxConfirmation(PduIdType TxPduId);


/* Signal Groups */
/* @req COM200 */
uint8 Com_SendSignalGroup(Com_SignalGroupIdType SignalGroupId);
/* @req COM201 */
uint8 Com_ReceiveSignalGroup(Com_SignalGroupIdType SignalGroupId);
/* @req COM199 */
void Com_UpdateShadowSignal(Com_SignalIdType SignalId, const void *SignalDataPtr);
/* @req COM202 */
void Com_ReceiveShadowSignal(Com_SignalIdType SignalId, void *SignalDataPtr);


#endif /* COM_COM_H_ */
