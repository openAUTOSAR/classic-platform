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








#include <stdlib.h>
#include <string.h>
#include "Com_Arc_Types.h"
#include "Com.h"
#include "Com_Internal.h"
#include "Com_misc.h"
#include "debug.h"
#include "PduR.h"
#include "Det.h"


uint8 Com_SendSignal(Com_SignalIdType SignalId, const void *SignalDataPtr) {
	VALIDATE_SIGNAL(SignalId, 0x0a, E_NOT_OK);
	// Store pointer to signal for easier coding.
	const ComSignal_type * Signal = GET_Signal(SignalId);
	Com_Arc_Signal_type * Arc_Signal = GET_ArcSignal(SignalId);
	const ComIPdu_type *IPdu = GET_IPdu(Arc_Signal->ComIPduHandleId);
	Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(Arc_Signal->ComIPduHandleId);

	//DEBUG(DEBUG_LOW, "Com_SendSignal: id %d, nBytes %d, BitPosition %d, intVal %d\n", SignalId, nBytes, signal->ComBitPosition, (uint32)*(uint8 *)SignalDataPtr);

	// TODO: CopyData
	// Com_CopyData(Arc_IPdu->ComIPduDataPtr, dataPtr, Signal->ComBitSize, Signal->ComBitPosition, 0);
	Com_WriteSignalDataToPdu(Signal->ComHandleId, SignalDataPtr);

	// If the signal has an update bit. Set it!
	if (Signal->ComSignalArcUseUpdateBit) {
		SETBIT(Arc_IPdu->ComIPduDataPtr, Signal->ComUpdateBitPosition);
	}

	/*
	 * If signal has triggered transmit property, trigger a transmission!
	 */
	if (Signal->ComTransferProperty == TRIGGERED) {
		Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft = IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeNumberOfRepetitions + 1;
	}
	return E_OK;
}

uint8 Com_ReceiveSignal(Com_SignalIdType SignalId, void* SignalDataPtr) {
	VALIDATE_SIGNAL(SignalId, 0x0b, E_NOT_OK);
	DEBUG(DEBUG_LOW, "Com_ReceiveSignal: SignalId %d\n", SignalId);

	// Com_CopyFromSignal(&ComConfig->ComSignal[SignalId], SignalDataPtr);
	Com_ReadSignalDataFromPdu(SignalId, SignalDataPtr);

	//uint16 val = *(uint16 *)SignalDataPtr;
	//val = bswap16(val);
	// Sign extend!
	return E_OK;
}

Std_ReturnType Com_TriggerTransmit(PduIdType ComTxPduId, uint8 *SduPtr) {
	PDU_ID_CHECK(ComTxPduId, 0x13, E_NOT_OK);
	/*
	 * COM260: This function must not check the transmission mode of the I-PDU
	 * since it should be possible to use it regardless of the transmission mode.
	 */

	/*
	 * COM395: This function must override the IPdu callouts used in Com_TriggerIPduTransmit();
	 */
	const ComIPdu_type *IPdu = GET_IPdu(ComTxPduId);
	Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(ComTxPduId);

	memcpy(SduPtr, Arc_IPdu->ComIPduDataPtr, IPdu->ComIPduSize);
	return E_OK;
}


//lint -esym(904, Com_TriggerIPduSend) //PC-Lint Exception of rule 14.7
void Com_TriggerIPduSend(PduIdType ComTxPduId) {
	PDU_ID_CHECK(ComTxPduId, 0x17);

	//DEBUG(DEBUG_MEDIUM, "Com_TriggerIPduSend sending IPdu %d... ", ComTxPduId);
	const ComIPdu_type *IPdu = GET_IPdu(ComTxPduId);
	Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(ComTxPduId);

	// Is the IPdu ready for transmission?
	if (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduMinimumDelayTimer == 0) {
		//DEBUG(DEBUG_MEDIUM, "success!\n", ComTxPduId);

		/*
		PduInfoType PduInfoPackage = {
			.SduDataPtr = malloc(IPdu->ComIPduSize),
			.SduLength = ComConfig->ComIPdu[ComTxPduId].ComIPduSize
		};
		memcpy((void *)PduInfoPackage.SduDataPtr, Arc_IPdu->ComIPduDataPtr, IPdu->ComIPduSize);
		*/

		Com_Arc_Config.OutgoingPdu.SduLength = ComConfig->ComIPdu[ComTxPduId].ComIPduSize;
		memcpy((void *)Com_Arc_Config.OutgoingPdu.SduDataPtr, Arc_IPdu->ComIPduDataPtr, IPdu->ComIPduSize);
		// Check callout status
		if (IPdu->ComIPduCallout != NULL) {
			if (!IPdu->ComIPduCallout(ComTxPduId, Arc_IPdu->ComIPduDataPtr)) {
				// TODO Report error to DET.
				// Det_ReportError();
				return;
			}
		}

		// Send IPdu!
		if (PduR_ComTransmit(IPdu->ArcIPduOutgoingId, &Com_Arc_Config.OutgoingPdu) == E_OK) {
			// Clear all update bits for the contained signals
			for (uint8 i = 0; (IPdu->ComIPduSignalRef != NULL) && (IPdu->ComIPduSignalRef[i] != NULL); i++) {
			//for (int i = 0; i < Arc_IPdu->NComIPduSignalRef; i++) {
				if (IPdu->ComIPduSignalRef[i]->ComSignalArcUseUpdateBit) {
					CLEARBIT(Arc_IPdu->ComIPduDataPtr, IPdu->ComIPduSignalRef[i]->ComUpdateBitPosition);
				}
			}
		}
		// Free allocted memory.
		// TODO: Is this the best way to solve this memory problem?
		//free(PduInfoPackage.SduDataPtr);

		// Reset miminum delay timer.
		Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduMinimumDelayTimer = IPdu->ComTxIPdu.ComTxIPduMinimumDelayFactor;

	} else {
		//DEBUG(DEBUG_MEDIUM, "failed (MDT)!\n", ComTxPduId);
	}
}

//lint -esym(904, Com_RxIndication) //PC-Lint Exception of rule 14.7
void Com_RxIndication(PduIdType ComRxPduId, const uint8* SduPtr) {
	PDU_ID_CHECK(ComRxPduId, 0x14);

	const ComIPdu_type *IPdu = GET_IPdu(ComRxPduId);
	Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(ComRxPduId);

	// If Ipdu is stopped
	if (!Arc_IPdu->Com_Arc_IpduStarted) {
		return;
	}

	// Check callout status
	if (IPdu->ComIPduCallout != NULL) {
		if (!IPdu->ComIPduCallout(ComRxPduId, SduPtr)) {
			// TODO Report error to DET.
			// Det_ReportError();
			return;
		}
	}

	// Copy IPDU data
	memcpy(Arc_IPdu->ComIPduDataPtr, SduPtr, IPdu->ComIPduSize);

	// For each signal.
	const ComSignal_type *comSignal;
	for (uint8 i = 0; IPdu->ComIPduSignalRef[i] != NULL; i++) {
		comSignal = IPdu->ComIPduSignalRef[i];
		Com_Arc_Signal_type * Arc_Signal = GET_ArcSignal(comSignal->ComHandleId);

		// If this signal uses an update bit, then it is only considered if this bit is set.
		if ( (!comSignal->ComSignalArcUseUpdateBit) ||
			( (comSignal->ComSignalArcUseUpdateBit) && (TESTBIT(Arc_IPdu->ComIPduDataPtr, comSignal->ComUpdateBitPosition)) ) ) {

			if (comSignal->ComTimeoutFactor > 0) { // If reception deadline monitoring is used.
				// Reset the deadline monitoring timer.
				Arc_Signal->Com_Arc_DeadlineCounter = comSignal->ComTimeoutFactor;
			}

			// Check the signal processing mode.
			if (IPdu->ComIPduSignalProcessing == IMMEDIATE) {
				// If signal processing mode is IMMEDIATE, notify the signal callback.
				if (IPdu->ComIPduSignalRef[i]->ComNotification != NULL) {
					IPdu->ComIPduSignalRef[i]->ComNotification();
				}

			} else {
				// Signal processing mode is DEFERRED, mark the signal as updated.
				Arc_Signal->ComSignalUpdated = 1;
			}

		} else {
			DEBUG(DEBUG_LOW, "Com_RxIndication: Ignored signal %d of I-PD %d since its update bit was not set\n", comSignal->ComHandleId, ComRxPduId);
		}
	}

	return;
}

void Com_TxConfirmation(PduIdType ComTxPduId) {
	PDU_ID_CHECK(ComTxPduId, 0x15);
	(void)ComTxPduId; // Nothing to be done. This is just to avoid Lint warning.
}


Std_ReturnType Com_SendSignalGroup(Com_SignalGroupIdType SignalGroupId) {
//#warning Com_SendSignalGroup should be performed atomically. Should we disable interrupts here?
	const ComSignal_type * Signal = GET_Signal(SignalGroupId);
	Com_Arc_Signal_type * Arc_Signal = GET_ArcSignal(SignalGroupId);
	Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(Arc_Signal->ComIPduHandleId);
	const ComIPdu_type *IPdu = GET_IPdu(Arc_Signal->ComIPduHandleId);


	// Copy shadow buffer to Ipdu data space
	const ComGroupSignal_type *groupSignal;
	for (uint8 i = 0; Signal->ComGroupSignal[i] != NULL; i++) {
		groupSignal = Signal->ComGroupSignal[i];
		// TODO CopyData
		// Com_CopyData(Arc_IPdu->ComIPduDataPtr, Arc_Signal->Com_Arc_ShadowBuffer,  groupSignal->ComBitSize, groupSignal->ComBitPosition, groupSignal->ComBitPosition);
		Com_WriteGroupSignalDataToPdu(Signal->ComHandleId, groupSignal->ComHandleId, Arc_Signal->Com_Arc_ShadowBuffer);
	}

	// If the signal has an update bit. Set it!
	if (Signal->ComSignalArcUseUpdateBit) {
		SETBIT(Arc_IPdu->ComIPduDataPtr, Signal->ComUpdateBitPosition);
	}

	/*
	 * If signal has triggered transmit property, trigger a transmission!
	 */
	if (Signal->ComTransferProperty == TRIGGERED) {
		Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft = IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeNumberOfRepetitions + 1;
	}

	return E_OK;
}


Std_ReturnType Com_ReceiveSignalGroup(Com_SignalGroupIdType SignalGroupId) {
//#warning Com_ReceiveSignalGroup should be performed atomically. Should we disable interrupts here?
	const ComSignal_type * Signal = GET_Signal(SignalGroupId);
	Com_Arc_Signal_type * Arc_Signal = GET_ArcSignal(SignalGroupId);

	// Copy Ipdu data buffer to shadow buffer.
	const ComGroupSignal_type *groupSignal;
	for (uint8 i = 0; Signal->ComGroupSignal[i] != NULL; i++) {
		groupSignal = Signal->ComGroupSignal[i];
		// TODO: CopyData
		// Com_CopyData(Arc_Signal->Com_Arc_ShadowBuffer, Arc_IPdu->ComIPduDataPtr, groupSignal->ComBitSize, groupSignal->ComBitPosition, groupSignal->ComBitPosition);
		Com_ReadSignalDataFromPdu(groupSignal->ComHandleId, (void *)Arc_Signal->Com_Arc_ShadowBuffer);
	}


	return E_OK;
}

void Com_UpdateShadowSignal(Com_SignalIdType SignalId, const void *SignalDataPtr) {
	Com_Arc_GroupSignal_type *Arc_GroupSignal = GET_ArcGroupSignal(SignalId);
	// TODO: CopyData
	// Com_CopyData(Arc_GroupSignal->Com_Arc_ShadowBuffer, SignalDataPtr, GroupSignal->ComBitSize, GroupSignal->ComBitPosition, 0);
	Com_WriteSignalDataToPduBuffer(SignalId, TRUE, SignalDataPtr, (void *)Arc_GroupSignal->Com_Arc_ShadowBuffer);
}

void Com_ReceiveShadowSignal(Com_SignalIdType SignalId, void *SignalDataPtr) {
	Com_Arc_GroupSignal_type *Arc_GroupSignal = GET_ArcGroupSignal(SignalId);
	// TODO: CopyData
	// Com_CopyData(SignalDataPtr, Arc_GroupSignal->Com_Arc_ShadowBuffer, GroupSignal->ComBitSize, 0, GroupSignal->ComBitPosition);
	Com_ReadSignalDataFromPduBuffer(SignalId, TRUE, SignalDataPtr, (void *)Arc_GroupSignal->Com_Arc_ShadowBuffer);
}
