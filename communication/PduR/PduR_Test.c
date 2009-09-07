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

PduIdType LOIf_ReceivedPduId;
uint8 LOIf_ReceivedData[] = {9,9,9,9,9,9,9,9};
uint8 LOIf_TriggerMode = 0; // 1 => TriggerTransmit mode. 0 => Ordinary mode.
uint8 LOIf_SendConfirmation = 1; // 0 => No confirmation.
Std_ReturnType LOIf_Transmit_TEST(PduIdType LOTxPduId,const PduInfoType* PduInfoPtr)
{
	//debug("LinIf_Transmit_TEST: Received data with id %d\n", LinTxPduId);
	LOIf_ReceivedPduId = LOTxPduId;

	if (LOIf_TriggerMode) {
		PduR_LinIfTriggerTransmit(LOTxPduId, LOIf_ReceivedData);
	} else {
		memcpy(&LOIf_ReceivedData, PduInfoPtr->SduDataPtr, PduInfoPtr->SduLength * (sizeof (uint8)));
	}

	if (LOIf_SendConfirmation) {
		PduR_LinIfTxConfirmation(LOTxPduId);
	}

	return E_OK;
}

Std_ReturnType LOIf_ErroneousTransmit_TEST(PduIdType LOTxPduId,const PduInfoType* PduInfoPtr) {
	return E_NOT_OK;
}

PduIdType UP_ReceivedPduId;
const uint8 UP_ReceivedData[] = {9,9,9,9,9,9,9,9};
Std_ReturnType UP_RxIndication_TEST(PduIdType UPRxPduId, const uint8* PduInfoPtr)
{
	//debug("Com_RxIndication_TEST: received indication with id %d and data %d\n", ComRxPduId, *PduInfoPtr);
	UP_ReceivedPduId = UPRxPduId;

	memcpy(&UP_ReceivedData, PduInfoPtr, PduR_RTable_LoIf.RoutingTable[UPRxPduId].SduLength * (sizeof (uint8)));
	return E_OK;
}

Std_ReturnType UP_TxConfirmation_TEST(PduIdType PduId) {
	//debug("Com_TxConfirmation_TEST: Received confirmation with id %d\n", PduId);
	UP_ReceivedPduId = PduId;
	return E_OK;
}

Std_ReturnType UP_TriggerTransmit_TEST(PduIdType PduId, uint8 *PduInfoPtr) {
	//debug("Com_TriggerTransmit: Received trigger with id %d and data %d\n", PduId, *PduInfoPtr);
	UP_ReceivedPduId = PduId;
	memcpy(PduInfoPtr, &CanSduPtr, PduR_RTable_LoIf.RoutingTable[PduId].SduLength * (sizeof (uint8)));
	return E_OK;
}

PduR_FctPtrType PduR_Callbacks_TEST = {
	.TargetIndicationFctPtr = UP_RxIndication_TEST,
	.TargetTransmitFctPtr = LOIf_Transmit_TEST,
	.TargetConfirmationFctPtr = UP_TxConfirmation_TEST,
	.TargetTriggerTransmitFctPtr = UP_TriggerTransmit_TEST,
};

PduR_FctPtrType PduR_ErroneousCallbacks_TEST = {
	//.TargetIndicationFctPtr = UP_RxIndication_TEST,
	.TargetTransmitFctPtr = LOIf_ErroneousTransmit_TEST,
	//.TargetConfirmationFctPtr = UP_TxConfirmation_TEST,
	//.TargetTriggerTransmitFctPtr = UP_TriggerTransmit_TEST,
};


// DET_REPORTERROR(PDUR_MODULE_ID, PDUR_INSTANCE_ID, 0x00, PDUR_E_CONFIG_PTR_INVALID);

void PduRclearError(void)
{
  error.ApiId=0;
  error.ErrorId=0;
  error.InstanceId=0;
  error.ModuleId=0;

  UP_ReceivedPduId = -1;
  memset(&UP_ReceivedData, 9, sizeof(uint8) * 8);

  LOIf_ReceivedPduId = -1;
  memset(&LOIf_ReceivedData, 9, sizeof(uint8) * 8);
}
