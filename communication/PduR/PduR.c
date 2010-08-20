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
#include "Mcu.h"
#include "debug.h"

/*
 * The state of the PDU router.
 */
PduR_StateType PduRState = PDUR_UNINIT;

const PduR_PBConfigType * PduRConfig;


/* ############### Zero Cost Operation Mode ############# */
/* Only define the following functions if zero cost operation
 * mode is not used! They are defined as macros in PduR.h otherwise. */
#if PDUR_ZERO_COST_OPERATION == STD_OFF

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
	int bufferNr = 0;
	int i = 0;
	PduRRoutingPath_type *path;
	PduRConfig->PduRRoutingTable->NRoutingPaths = 0;
	for (i = 0; !PduRConfig->PduRRoutingTable->PduRRoutingPath[i].PduR_Arc_EOL && !failed; i++) {
		PduRConfig->PduRRoutingTable->NRoutingPaths++;
		path = &PduRConfig->PduRRoutingTable->PduRRoutingPath[i];

		if (path->PduRDestPdu.DataProvision != PDUR_NO_PROVISION) {
			// Allocate memory for new buffer.
			PduRTxBuffer_type *buffer = path->PduRDestPdu.TxBufferRef;

			if (bufferNr >= PDUR_MAX_TX_BUFFER_NUMBER) {
				DEBUG(DEBUG_LOW,"PduR_Init: Initialization of buffer failed due to erroneous configuration.\nThe number of buffer exceeded the maximum number of allowed buffers.\n");
				failed = 1;
				break;
			}
			if	((buffer->Buffer = (uint8 *)malloc(buffer->Depth * sizeof(uint8) * path->SduLength)) == 0) {
				DEBUG(DEBUG_LOW,"PduR_Init: Initialization of buffer failed. Buffer space could not be allocated for buffer number %d\n", bufferNr);
				failed = 1;
				break;
			}

			buffer->First = buffer->Buffer;
			buffer->Last = buffer->Buffer;


			// Initialize the new buffer.
			buffer->BufferId = i; // Set buffer id.
			buffer->BufferType = path->PduRDestPdu.DataProvision; // Set buffer data provision mode.
			buffer->Length = path->SduLength; // Set buffer sdu length.

			if (path->PduRDestPdu.DataProvision == PDUR_TRIGGER_TRANSMIT) {
				//memcpy(buffer->First, path->PduRDefaultValue.PduRDefaultValueElement->DefaultValueElement,path->SduLength);
				PduR_BufferQueue(buffer, path->PduRDefaultValue.PduRDefaultValueElement->DefaultValueElement);
			}

			// Save pointer to the new buffer.
			//PduR_RTable_LoIf.RoutingTable[i].TxBufferRef = &PduRBuffers[bufferNr];

			DEBUG(DEBUG_LOW,"Initialized buffer %d. Id: %d, Type: %d, Depth: %d\n", bufferNr, buffer->BufferId, buffer->BufferType, buffer->Depth);
			bufferNr++;
		}
	}

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

void PduR_BufferInc(PduRTxBuffer_type *Buffer, uint8 **ptr) {
	(*ptr) = (*ptr) + Buffer->Length;

	// TODO make more efficient without multiplication.
	if (*ptr >= Buffer->Buffer + Buffer->Depth * Buffer->Length) {
		*ptr = Buffer->Buffer;
	}
	//*val = (*val + 1) % Buffer->Depth;
}

void PduR_BufferQueue(PduRTxBuffer_type *Buffer, const uint8 * SduPtr) {
	imask_t state = McuE_EnterCriticalSection();

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
	McuE_ExitCriticalSection(state);
}

void PduR_BufferDeQueue(PduRTxBuffer_type *Buffer, uint8 *SduPtr) {
	imask_t state = McuE_EnterCriticalSection();

	if (Buffer->NrItems > 0) {
		memcpy(SduPtr, Buffer->First, sizeof(uint8) * Buffer->Length);
		PduR_BufferInc(Buffer, &Buffer->First);
		Buffer->NrItems--;
		DEBUG(DEBUG_LOW,"\tBuffer %d: DeQueueing data. Status: NrItems %d, First %d, Last %d\n", Buffer->BufferId, Buffer->NrItems, *Buffer->First, *Buffer->Last);
	} else {
		DEBUG(DEBUG_LOW,"\tBuffer %d: Buffer is empty, nothing to dequeue!\n", Buffer->BufferId);
	}
	McuE_ExitCriticalSection(state);
}

void PduR_BufferFlush(PduRTxBuffer_type *Buffer) {
	DEBUG(DEBUG_LOW,"\tBuffer %d: Flushing!\n", Buffer->BufferId);
	imask_t state = McuE_EnterCriticalSection();
	Buffer->NrItems = 0;
	Buffer->First = Buffer->Buffer;
	Buffer->Last = Buffer->Buffer;
	Buffer->TxConfP = 0;
	McuE_ExitCriticalSection(state);
}

uint8 PduR_BufferIsFull(PduRTxBuffer_type *Buffer) {
	imask_t state = McuE_EnterCriticalSection();
	if (Buffer->NrItems < Buffer->Depth) {
		return 0;
	} else {
		return 1;
	}
	McuE_ExitCriticalSection(state);
}


#if PDUR_VERSION_INFO_API == STD_ON
void PduR_GetVersionInfo (Std_VersionInfoType* versionInfo){
	versionInfo->moduleID = (uint16)MODULE_ID_PDUR;
	versionInfo->vendorID = (uint16)1;

	// TODO Add vendor specific version numbers.
}
#endif

uint32 PduR_GetConfigurationId () {
	//PduR_DevCheck(0,1,0x18,E_NOT_OK);
	return PduRConfig->PduRConfigurationId;
}
#endif // End of not Zero Cost Operation Mode

Std_ReturnType PduR_CancelTransmitRequest(PduR_CancelReasonType PduCancelReason, PduIdType PduId) {
	// TODO Implement!
	return E_NOT_OK;
}

void PduR_ChangeParameterRequest(PduR_ParameterValueType PduParameterValue, PduIdType PduId) {
	// TODO Implement!

}
