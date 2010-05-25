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

#if (PDUR_ZERO_COST_OPERATION == STD_OFF)

#include <string.h>
#include "debug.h"
#if defined(USE_DEM)
#include "Dem.h"
#endif

/**
 * Helper function for the PduR_<LO>IfRxIndication functions. This helper performs the actions specified by PDUR255 and PDUR258.
 * @param LinRxPduId - The id of the PDU to be routed.
 * @param LinSduPtr - The payload of the PDU.
 * @param LoIf_Transmit
 */
void PduR_LoIfRxIndication(PduIdType PduId, const uint8* SduPtr) {

	// Perform routing lookup.
	PduRRoutingPath_type *route = &PduRConfig->PduRRoutingTable->PduRRoutingPath[PduId];

	if (route->PduR_GatewayMode == 0) {
		// This is an ordinary request.
		route->FctPtrs.TargetIndicationFctPtr(route->PduRDestPdu.DestPduId, SduPtr); // Send PDU to next receptor.


	} else if (route->PduR_GatewayMode == 1 && route->PduRDestPdu.DataProvision == PDUR_NO_PROVISION) {
		// This is a gateway request, but without any data provision (buffer usage).
		PduInfoType PduInfo = {
			.SduDataPtr = (uint8 *)SduPtr,
			.SduLength = route->SduLength
		};
		route->FctPtrs.TargetTransmitFctPtr(route->PduRDestPdu.DestPduId, &PduInfo); // Send PDU to next receptor.


	} else if (route->PduR_GatewayMode == 1 && route->PduRDestPdu.DataProvision == PDUR_TRIGGER_TRANSMIT) {
		// Ths is a gateway request which uses trigger transmit data provision. PDUR255
		DEBUG(DEBUG_LOW,"\tUsing gateway mode with trigger transmit provision\n");

		if (route->PduRDestPdu.TxBufferRef->TxConfP) { // Transfer confirmation pending.
			// Enqueue the new I-PDU. This will flush the buffer if it is full according to the buffer specification.
			PduR_BufferQueue(route->PduRDestPdu.TxBufferRef, SduPtr);
			// TODO report PDUR_E_PDU_INSTANCE_LOST to DEM if needed.
		}

		if (!route->PduRDestPdu.TxBufferRef->TxConfP) { // No transfer confirmation pending (anymore).
			uint8 val[route->SduLength];
			PduInfoType PduInfo = {
				.SduDataPtr = val,
				.SduLength = route->SduLength
			};
			PduR_BufferDeQueue(route->PduRDestPdu.TxBufferRef, val);
			PduR_BufferQueue(route->PduRDestPdu.TxBufferRef, SduPtr);
			if (route->FctPtrs.TargetTransmitFctPtr(route->PduRDestPdu.DestPduId, &PduInfo) == E_OK) {
				setTxConfP(route);
			}
		}

	} else if (route->PduR_GatewayMode == 1 && route->PduRDestPdu.DataProvision == PDUR_DIRECT) {
		// This is a gateway request using a direct data provision fifo. PDUR258
		DEBUG(DEBUG_LOW,"\tUsing gateway mode with direct provision\n");

		if (route->PduRDestPdu.TxBufferRef->TxConfP) { // Transfer confirmation pending.
			DEBUG(DEBUG_LOW,"\tTransfer confirmation pending.\n");
			PduR_BufferQueue(route->PduRDestPdu.TxBufferRef, SduPtr);
			// TODO report PDUR_E_PDU_INSTANCE_LOST to DEM if needed.

		}

		if (!route->PduRDestPdu.TxBufferRef->TxConfP) { // No transfer confirmation pending (anymore)
			// TODO: Shall this function create a new I-PDU from LinSduPtr?

			// Make new PduInfoPackage
			DEBUG(DEBUG_LOW,"\tNo transfer confirmation pending. Forwarding packet.\n");
			PduInfoType PduInfoPtr = {
					.SduDataPtr = (uint8 *)SduPtr,
					.SduLength = route->SduLength
			};
			if (route->FctPtrs.TargetTransmitFctPtr(route->PduRDestPdu.DestPduId, &PduInfoPtr) == E_OK) {
				setTxConfP(route);


			} else {
#if defined(USE_DEM)
				Dem_ReportErrorStatus(PDUR_E_PDU_INSTANCE_LOST, DEM_EVENT_STATUS_FAILED);
#endif
				DEBUG(DEBUG_LOW,"\tTransmission failed. PDUR_E_PDU_INSTANCE_LOST\n");
			}
		}
	}
}

// void PduR_LoIfTxConfirmation(PduIdType PduId, Std_ReturnType (*LoIf_Transmit)(PduIdType, const PduInfoType*)) {

void PduR_LoIfTxConfirmation(PduIdType PduId) {

	// Perform routing lookup.
	PduRRoutingPath_type *route = &PduRConfig->PduRRoutingTable->PduRRoutingPath[PduId];

	// Find out if this is a gateway or ordinary confirmation.

	if (route->PduR_GatewayMode == 0) {
		// This is an ordinary request.
		route->FctPtrs.TargetConfirmationFctPtr(route->PduRDestPdu.DestPduId); // Forward confirmation

	} else if (route->PduR_GatewayMode == 1 && route->PduRDestPdu.DataProvision == PDUR_NO_PROVISION) {
		// A gateway request without provision. Just forward confirmation.
		route->FctPtrs.TargetConfirmationFctPtr(route->PduRDestPdu.DestPduId); // Forward confirmation


	} else if (route->PduR_GatewayMode == 1 && route->PduRDestPdu.DataProvision == PDUR_TRIGGER_TRANSMIT) {
		// The route is using gateway mode and trigger transmit data provision. PDUR256
		DEBUG(DEBUG_LOW,"\tUsing gateway mode with trigger transmit data provision.\n", PduId);

		while (1) {

			// If TxConfP is set process confirmation, ignore otherwise.
			if (!route->PduRDestPdu.TxBufferRef->TxConfP) { // TxConfP not set, ignore!
				DEBUG(DEBUG_LOW,"\tTxConfP was not set, ignoring confirmation.\n");
				break;


			} else { // TxConfP is set
				if (route->PduRDestPdu.TxBufferRef->NrItems == 1) { // Only one entry in buffer.
					clearTxConfP(route); // Clear TxConfP
					DEBUG(DEBUG_LOW,"\tOnly one entry in buffer and TxConfP set, cleared TxConfP.\n");
					break;


				} else { // Buffer is not empty
					// Dequeue the first fifo item.
					DEBUG(DEBUG_LOW,"\tMore than one entry in buffer and TxConfP set, transmitting the next fifo entry.\n");
					uint8 val[route->SduLength];
					PduR_BufferDeQueue(route->PduRDestPdu.TxBufferRef, val);

					// TODO Does this need to be static?
					PduInfoType PduInfo = {
						.SduDataPtr = val,
						.SduLength = route->SduLength
					};
					// Transmit this item.
					if (route->FctPtrs.TargetTransmitFctPtr(route->PduRDestPdu.DestPduId, &PduInfo) == E_OK) {
						DEBUG(DEBUG_LOW,"\tTransmit succeeded.\n");
						break;


					} else {
						DEBUG(DEBUG_LOW,"\tTransmit failed. Retrying with the next fifo entry.\n");
					}
				}
			}
		}


	} else if (route->PduR_GatewayMode == 1 && route->PduRDestPdu.DataProvision == PDUR_DIRECT) {
		// The route is using a direct data provision fifo. PDUR259
		DEBUG(DEBUG_LOW,"\tUsing gateway mode with direct data provision.\n", PduId);

		while (1) {

			// If TxConfP is set process confirmation, ignore otherwise.
			if (!route->PduRDestPdu.TxBufferRef->TxConfP) { // TxConfP not set, ignore!
				DEBUG(DEBUG_LOW,"\tTxConfP was not set, ignoring confirmation.\n");
				break;


			} else { // TxConfP is set
				if (route->PduRDestPdu.TxBufferRef->NrItems == 0) { // Buffer is empty
					clearTxConfP(route); // Clear TxConfP
					DEBUG(DEBUG_LOW,"\tBuffer was empty and TxConfP set, cleared TxConfP.\n");
					break;


				} else { // Buffer is not empty
					// Dequeue the first fifo item.
					DEBUG(DEBUG_LOW,"\tBuffer was not empty and TxConfP set, transmitting the next fifo entry.\n");
					uint8 val[route->SduLength];
					PduR_BufferDeQueue(route->PduRDestPdu.TxBufferRef, val);

					// TODO Does this need to be static?
					PduInfoType PduInfo = {
						.SduDataPtr = val,
						.SduLength = route->SduLength
					};
					// Transmit this item.
					if (route->FctPtrs.TargetTransmitFctPtr(route->PduRDestPdu.DestPduId, &PduInfo) == E_OK) {
						DEBUG(DEBUG_LOW,"\tTransmit succeeded.\n");
						break;


					} else {
						DEBUG(DEBUG_LOW,"\tTransmit failed. Retrying with the next fifo entry.\n");
					}
				}
			}
		}
	}
}

void PduR_LoIfTriggerTransmit(PduIdType PduId, uint8* SduPtr) {
	// Perform routing lookup.
	PduRRoutingPath_type *route = &PduRConfig->PduRRoutingTable->PduRRoutingPath[PduId];

	// Find out if this is a gateway or ordinary trigger.
	//if (route->PduRDestPdu.DataProvision == PDUR_NO_PROVISION) { // This is an ordinary trigger.
	if (route->PduR_GatewayMode == 0) { // This is an ordinary trigger.
		route->FctPtrs.TargetTriggerTransmitFctPtr(route->PduRDestPdu.DestPduId, SduPtr);

	} else if (route->PduR_GatewayMode == 1 && route->PduRDestPdu.DataProvision == PDUR_TRIGGER_TRANSMIT) { // The route is using a trigger transmit fifo. PDUR256
		DEBUG(DEBUG_LOW,"\tUsing gateway mode with trigger transmit data provision.\n", PduId);
		memcpy((void *)SduPtr, (void *)route->PduRDestPdu.TxBufferRef->First, sizeof(uint8) * route->SduLength);

	}
}

#endif
