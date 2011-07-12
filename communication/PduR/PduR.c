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


#include "Det.h"
#if defined(USE_DEM)
#include "Dem.h"
#endif
#include "PduR.h"
#include "debug.h"

/*
 * The state of the PDU router.
 */
PduR_StateType PduRState = PDUR_UNINIT; // 960, 31 PC-Lint: Borde åtgärdas


#if PDUR_ZERO_COST_OPERATION == STD_OFF



const PduR_PBConfigType * PduRConfig;


/*
 * Initializes the PDU Router.
 */
void PduR_Init (const PduR_PBConfigType* ConfigPtr) {

	//Enter(0);

	// Make sure the PDU Router is uninitialized.
	// Otherwise raise an error.
	if (PduRState != PDUR_UNINIT) {
		// Raise error and return.
		PDUR_DET_REPORTERROR(MODULE_ID_PDUR, PDUR_INSTANCE_ID, 0x00, PDUR_E_INVALID_REQUEST);
		return;
	}

	if (ConfigPtr == NULL) {
		PDUR_DET_REPORTERROR(MODULE_ID_PDUR, PDUR_INSTANCE_ID, 0x00, PDUR_E_CONFIG_PTR_INVALID);
		return;
	} else {
		PduRConfig = ConfigPtr;
	}

	// Start initialization!
	DEBUG(DEBUG_LOW,"--Initialization of PDU router--\n");

	uint8 failed = 0;

	// Initialize buffers.
	/*
	uint16 bufferNr = 0;
	for (uint16 i = 0; PduRConfig->RoutingPaths[i] != NULL && (!failed); i++) {
		const PduRRoutingPath_type *path = PduRConfig->RoutingPaths[i];

		for (uint8 j = 0; path->PduRDestPdus[j] != NULL; j++) {
			const PduRDestPdu_type *destination = path->PduRDestPdus[j];


			if (destination->DataProvision == PDUR_TRIGGER_TRANSMIT) {
				// Allocate memory for new buffer.


				// Initialize the new buffer.
				buffer->BufferId = i; // Set buffer id.
				buffer->BufferType = destination->DataProvision; // Set buffer data provision mode.
				buffer->Length = path->SduLength; // Set buffer sdu length.

				if (destination->DataProvision == PDUR_TRIGGER_TRANSMIT) {
					//memcpy(buffer->First, path->PduRDefaultValue.PduRDefaultValueElement->DefaultValueElement,path->SduLength);
					PduR_BufferQueue(buffer, path->PduRDefaultValue.PduRDefaultValueElement->DefaultValueElement);
				}

				// Save pointer to the new buffer.
				//PduR_RTable_LoIf.RoutingTable[i].TxBufferRef = &PduRBuffers[bufferNr];

				DEBUG(DEBUG_LOW,"Initialized buffer %d. Id: %d, Type: %d, Depth: %d\n", bufferNr, buffer->BufferId, buffer->BufferType, buffer->Depth);
				bufferNr++;

			}

		}
	}
	*/

	if (failed) {
		// TODO Report PDUR_E_INIT_FAILED to Dem.
		PduRState = PDUR_REDUCED;
		DEBUG(DEBUG_LOW,"--Initialization of PDU router failed--\n");


	} else {
		// The initialization succeeded!
		PduRState = PDUR_ONLINE;
		DEBUG(DEBUG_LOW,"--Initialization of PDU router completed --\n");

	}
}

/*
void PduR_BufferInc(PduRTxBuffer_type *Buffer, uint8 **ptr) {
	(*ptr) = (*ptr) + Buffer->Length;

	// TODO make more efficient without multiplication.
	//lint -e946 //PC-Lint Exception of MISRA rule 17.3
	if ( *ptr >= ( Buffer->Buffer + (Buffer->Depth * Buffer->Length) ) ) {
		*ptr = Buffer->Buffer;
	}
	//  *val = (*val + 1) % Buffer->Depth;
}

void PduR_BufferQueue(PduRTxBuffer_type *Buffer, const uint8 * SduPtr) {
    imask_t state;
    Irq_Save(state);

	if (PduR_BufferIsFull(Buffer)) { // Buffer is full
		PduR_BufferFlush(Buffer);
#if defined(USE_DEM)
		Dem_ReportErrorStatus(PDUR_E_PDU_INSTANCE_LOST, DEM_EVENT_STATUS_FAILED);
#endif

	} else {
		// Copy data to last place in buffer
		memcpy(Buffer->Last, SduPtr, sizeof(uint8) * Buffer->Length);

		PduR_BufferInc(Buffer, &Buffer->Last);
		Buffer->NrItems++;
		DEBUG(DEBUG_LOW,"\tBuffer %d: Queued data %d. Status: NrItems %d, First %d, Last %d\n", Buffer->BufferId, *SduPtr, Buffer->NrItems, *Buffer->First, *Buffer->Last);

	}
    Irq_Restore(state);
}

void PduR_BufferDeQueue(PduRTxBuffer_type *Buffer, uint8 *SduPtr) {
    imask_t state;
    Irq_Save(state);

	if (Buffer->NrItems > 0) {
		memcpy(SduPtr, Buffer->First, sizeof(uint8) * Buffer->Length);
		PduR_BufferInc(Buffer, &Buffer->First);
		Buffer->NrItems--;
		DEBUG(DEBUG_LOW,"\tBuffer %d: DeQueueing data. Status: NrItems %d, First %d, Last %d\n", Buffer->BufferId, Buffer->NrItems, *Buffer->First, *Buffer->Last);
	} else {
		DEBUG(DEBUG_LOW,"\tBuffer %d: Buffer is empty, nothing to dequeue!\n", Buffer->BufferId);
	}
    Irq_Restore(state);
}

void PduR_BufferFlush(PduRTxBuffer_type *Buffer) {
	DEBUG(DEBUG_LOW,"\tBuffer %d: Flushing!\n", Buffer->BufferId);
    imask_t state;
    Irq_Save(state);
	Buffer->NrItems = 0;
	Buffer->First = Buffer->Buffer;
	Buffer->Last = Buffer->Buffer;
	Buffer->TxConfP = 0;
    Irq_Restore(state);
}

uint8 PduR_BufferIsFull(PduRTxBuffer_type *Buffer) {
    imask_t state;
    Irq_Save(state);
	uint8 rv = 0;
	if (Buffer->NrItems < Buffer->Depth) {
		rv = 0;
	} else {
		rv = 1;
	}
    Irq_Restore(state);
	return rv;
}
*/



#if PDUR_VERSION_INFO_API == STD_ON
void PduR_GetVersionInfo (Std_VersionInfoType* versionInfo){
	versionInfo->moduleID = (uint16)MODULE_ID_PDUR;
	versionInfo->vendorID = (uint16)1;

	// TODO Add vendor specific version numbers.
}
#endif

uint32 PduR_GetConfigurationId (void) {
	//PduR_DevCheck(0,1,0x18,E_NOT_OK);
	return PduRConfig->PduRConfigurationId;
}

Std_ReturnType PduR_CancelTransmitRequest(PduR_CancelReasonType PduCancelReason, PduIdType PduId) {
	// TODO Implement!
	return E_NOT_OK;
}

void PduR_ChangeParameterRequest(PduR_ParameterValueType PduParameterValue, PduIdType PduId) {
	// TODO Implement!

}

#endif
