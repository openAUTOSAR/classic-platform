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
#include "Com_Com.h"
#include "Trace.h"
#include "CanIf.h"
#include "PduR.h"
#include "PduR_Com.h"
#include "Byteorder.h"

uint8 Com_SendSignal(Com_SignalIdType SignalId, const void *SignalDataPtr) {
	COM_VALIDATE_SIGNAL(SignalId, 0x0a, E_NOT_OK);
	// Store pointer to signal for easier coding.
	ComGetSignal(SignalId);
	ComGetArcSignal(SignalId);
	ComGetIPdu(Arc_Signal->ComIPduHandleId);
	ComGetArcIPdu(Arc_Signal->ComIPduHandleId);

	//DEBUG(DEBUG_LOW, "Com_SendSignal: id %d, nBytes %d, BitPosition %d, intVal %d\n", SignalId, nBytes, signal->ComBitPosition, (uint32)*(uint8 *)SignalDataPtr);

	void *dataPtr = (void *)SignalDataPtr;

	if (Signal->ComSignalEndianess == BIG_ENDIAN) {
		if (Signal->ComSignalType == UINT16) {
			uint16 data;
			memcpy(&data, SignalDataPtr, SignalTypeToSize(Signal->ComSignalType,Signal->ComSignalLength));
			data = bswap16(data);
			dataPtr = &data;

		} else if (Signal->ComSignalType == UINT32) {
			uint32 data;
			memcpy(&data, SignalDataPtr, SignalTypeToSize(Signal->ComSignalType,Signal->ComSignalLength));
			data = bswap32(data);
			dataPtr = &data;

		} else if (Signal->ComSignalType == SINT16) {
			sint16 data;
			memcpy(&data, SignalDataPtr, SignalTypeToSize(Signal->ComSignalType,Signal->ComSignalLength));
			data = bswap16(data);
			dataPtr = &data;

		} else if (Signal->ComSignalType == SINT32) {
			sint32 data;
			memcpy(&data, SignalDataPtr, SignalTypeToSize(Signal->ComSignalType,Signal->ComSignalLength));
			data = bswap32(data);
			dataPtr = &data;
		}

	}

	Com_CopyData(Arc_IPdu->ComIPduDataPtr, dataPtr, Signal->ComBitSize, Signal->ComBitPosition, 0);

	// If the signal has an update bit. Set it!
	if (Signal->ComSignalArcUseUpdateBit) {
		setBit(Arc_IPdu->ComIPduDataPtr, Signal->ComUpdateBitPosition);
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
	ComGetSignal(SignalId);
	COM_VALIDATE_SIGNAL(SignalId, 0x0b, E_NOT_OK);
	DEBUG(DEBUG_LOW, "Com_ReceiveSignal: SignalId %d\n", SignalId);

	Com_CopyFromSignal(&ComConfig->ComSignal[SignalId], SignalDataPtr);

	if (Signal->ComSignalEndianess == BIG_ENDIAN) {
		if (Signal->ComSignalType == UINT16) {
			*(uint16*)SignalDataPtr = bswap16(*(uint16*)SignalDataPtr);
			//memcpy(SignalDataPtr, dataPtr, SignalTypeToSize(Signal->ComSignalType,Signal->ComSignalLength));

		} else if (Signal->ComSignalType == UINT32) {
			*(uint32*)SignalDataPtr = bswap32(*(uint32*)SignalDataPtr);
			//memcpy(SignalDataPtr, dataPtr, SignalTypeToSize(Signal->ComSignalType,Signal->ComSignalLength));

		} else if (Signal->ComSignalType == SINT16) {
			*(sint16*)SignalDataPtr = bswap16(*(sint16*)SignalDataPtr);
			//memcpy(SignalDataPtr, dataPtr, SignalTypeToSize(Signal->ComSignalType,Signal->ComSignalLength));

		} else if (Signal->ComSignalType == SINT32) {
			*(sint32*)SignalDataPtr = bswap32(*(sint32*)SignalDataPtr);
			//memcpy(SignalDataPtr, dataPtr, SignalTypeToSize(Signal->ComSignalType,Signal->ComSignalLength));
		}
	}



	//uint16 val = *(uint16 *)SignalDataPtr;
	//val = bswap16(val);
	// Sign extend!
	return E_OK;
}

Std_ReturnType Com_TriggerTransmit(PduIdType ComTxPduId, uint8 *SduPtr) {
	PduIdCheck(ComTxPduId, 0x13, E_NOT_OK);
	/*
	 * COM260: This function must not check the transmission mode of the I-PDU
	 * since it should be possible to use it regardless of the transmission mode.
	 */

	/*
	 * COM395: This function must override the IPdu callouts used in Com_TriggerIPduTransmit();
	 */
	ComGetIPdu(ComTxPduId);
	ComGetArcIPdu(ComTxPduId);

	memcpy(SduPtr, Arc_IPdu->ComIPduDataPtr, IPdu->ComIPduSize);
	return E_OK;
}


void Com_TriggerIPduSend(PduIdType ComTxPduId) {
	PduIdCheck(ComTxPduId, 0x17);

	//DEBUG(DEBUG_MEDIUM, "Com_TriggerIPduSend sending IPdu %d... ", ComTxPduId);
	ComGetIPdu(ComTxPduId);
	ComGetArcIPdu(ComTxPduId);

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
		if (PduR_ComTransmit(ComTxPduId, &Com_Arc_Config.OutgoingPdu) == E_OK) {
			// Clear all update bits for the contained signals
			for (int i = 0; i < Arc_IPdu->NComIPduSignalRef; i++) {
				if (IPdu->ComIPduSignalRef[i]->ComSignalArcUseUpdateBit) {
					clearBit(Arc_IPdu->ComIPduDataPtr, IPdu->ComIPduSignalRef[i]->ComUpdateBitPosition);
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

Std_ReturnType Com_RxIndication(PduIdType ComRxPduId, const uint8* SduPtr) {
	PduIdCheck(ComRxPduId, 0x14, E_NOT_OK);

	ComGetIPdu(ComRxPduId);
	ComGetArcIPdu(ComRxPduId);

	// If Ipdu is stopped
	if (!Arc_IPdu->Com_Arc_IpduStarted) {
		return E_OK;
	}

	// Check callout status
	if (IPdu->ComIPduCallout != NULL) {
		if (!IPdu->ComIPduCallout(ComRxPduId, SduPtr)) {
			// TODO Report error to DET.
			// Det_ReportError();
			return E_NOT_OK;
		}
	}

	// Copy IPDU data
	memcpy(Arc_IPdu->ComIPduDataPtr, SduPtr, IPdu->ComIPduSize);

	// For each signal.
	const ComSignal_type *signal;
	for (int i = 0; IPdu->ComIPduSignalRef[i] != NULL; i++) {
		signal = IPdu->ComIPduSignalRef[i];
		ComGetArcSignal(signal->ComHandleId);

		// If this signal uses an update bit, then it is only considered if this bit is set.
		if (!signal->ComSignalArcUseUpdateBit ||
			(signal->ComSignalArcUseUpdateBit && testBit(Arc_IPdu->ComIPduDataPtr, signal->ComUpdateBitPosition))) {

			if (signal->ComTimeoutFactor > 0) { // If reception deadline monitoring is used.
				// Reset the deadline monitoring timer.
				Arc_Signal->Com_Arc_DeadlineCounter = signal->ComTimeoutFactor;
			}

			/*
			// Zero new filter value.
			IPdu->ComIPduSignalRef[i]->ComFilter.ComFilterArcNewValue = 0;

			//Fix this!!!
			Com_CopyFromSignal(IPdu->ComIPduSignalRef[i], &IPdu->ComIPduSignalRef[i]->ComFilter.ComFilterArcNewValue);
			*/
			// Perform filtering
			//if (Com_Filter(IPdu->ComIPduSignalRef[i])) {

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
			//}
		} else {
			DEBUG(DEBUG_LOW, "Com_RxIndication: Ignored signal %d of I-PDU %d since its update bit was not set\n", signal->ComHandleId, IPdu->ComIPduRxHandleId);
		}
	}

	return E_OK;
}

void Com_TxConfirmation(PduIdType ComTxPduId) {
	PduIdCheck(ComTxPduId, 0x15);
}


Std_ReturnType Com_SendSignalGroup(Com_SignalGroupIdType SignalGroupId) {
//#warning Com_SendSignalGroup should be performed atomically. Should we disable interrupts here?
	ComGetSignal(SignalGroupId);
	ComGetArcSignal(SignalGroupId);
	ComGetArcIPdu(Arc_Signal->ComIPduHandleId);
	ComGetIPdu(Arc_Signal->ComIPduHandleId);


	// Copy shadow buffer to Ipdu data space
	const ComGroupSignal_type *groupSignal;
	for (int i = 0; Signal->ComGroupSignal[i] != NULL; i++) {
		groupSignal = Signal->ComGroupSignal[i];
		Com_CopyData(Arc_IPdu->ComIPduDataPtr, Arc_Signal->Com_Arc_ShadowBuffer,  groupSignal->ComBitSize, groupSignal->ComBitPosition, groupSignal->ComBitPosition);
	}

	// If the signal has an update bit. Set it!
	if (Signal->ComSignalArcUseUpdateBit) {
		setBit(Arc_IPdu->ComIPduDataPtr, Signal->ComUpdateBitPosition);
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
	ComGetSignal(SignalGroupId);
	ComGetArcSignal(SignalGroupId);
	ComGetArcIPdu(Arc_Signal->ComIPduHandleId);

	// Copy Ipdu data buffer to shadow buffer.
	const ComGroupSignal_type *groupSignal;
	for (int i = 0; Signal->ComGroupSignal[i] != NULL; i++) {
		groupSignal = Signal->ComGroupSignal[i];
		Com_CopyData(Arc_Signal->Com_Arc_ShadowBuffer, Arc_IPdu->ComIPduDataPtr, groupSignal->ComBitSize, groupSignal->ComBitPosition, groupSignal->ComBitPosition);
	}


	return E_OK;
}

void Com_UpdateShadowSignal(Com_SignalIdType SignalId, const void *SignalDataPtr) {
	ComGetGroupSignal(SignalId);
	ComGetArcGroupSignal(SignalId);
	Com_CopyData(Arc_GroupSignal->Com_Arc_ShadowBuffer, SignalDataPtr, GroupSignal->ComBitSize, GroupSignal->ComBitPosition, 0);
}

void Com_ReceiveShadowSignal(Com_SignalIdType SignalId, void *SignalDataPtr) {
	ComGetGroupSignal(SignalId);
	ComGetArcGroupSignal(SignalId);
	Com_CopyData(SignalDataPtr, Arc_GroupSignal->Com_Arc_ShadowBuffer, GroupSignal->ComBitSize, 0, GroupSignal->ComBitPosition);
}
