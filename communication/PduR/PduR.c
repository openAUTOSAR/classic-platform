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
#include "PduR.h"
#include "PduR_Com.h"
#include "PduR_CanIf.h"
#include "PduR_LinIf.h"
#include "PduR_Ipdum.h"
#include "Mcu.h"
#include "Trace.h"

/*
 * The state of the PDU router.
 */
PduR_StateType PduRState = PDUR_UNINIT;

const PduR_PBConfigType * PduRConfig;


/* ############### Zero Cost Operation Mode ############# */
/* Only define the following functions if zero cost operation
 * mode is not used! They are defined as macros in PduR.h otherwise. */
#ifndef PDUR_ZERO_COST_OPERATION

/*
 * Initializes the PDU Router.
 */
void PduR_Init (const PduR_PBConfigType* ConfigPtr) {

	//Enter(0);

	// Make sure the PDU Router is uninitialized.
	// Otherwise raise an error.
	if (PduRState != PDUR_UNINIT) {
		// Raise error and return.
		DET_REPORTERROR(PDUR_MODULE_ID, PDUR_INSTANCE_ID, 0x00, PDUR_E_INVALID_REQUEST);
		return;
	}

	if (ConfigPtr == NULL) {
		DET_REPORTERROR(PDUR_MODULE_ID, PDUR_INSTANCE_ID, 0x00, PDUR_E_CONFIG_PTR_INVALID);
		return;
	} else {
		PduRConfig = ConfigPtr;
	}

	// Start initialization!
	DEBUG(DEBUG_LOW,"--Initialization of PDU router--\n");

	uint8 failed = 0;

	// TODO Initialize DestPduIds!!!!!

	// TODO Initialize NRoutingPaths.

	// Initialize buffers.
	int bufferNr = 0;
	int i = 0;
	PduRRoutingPath_type *path;
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
		DET_REPORTERROR(PDUR_MODULE_ID, PDUR_INSTANCE_ID, 0x00, PDUR_E_PDU_INSTANCE_LOST);


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


#ifdef PDUR_VERSION_INFO_API
void PduR_GetVersionInfo (Std_VersionInfoType* versionInfo){
	versionInfo->moduleID = (uint16)PDUR_MODULE_ID;
	versionInfo->vendorID = (uint16)1;

	// TODO Add vendor specific version numbers.
}
#endif

uint32 PduR_GetConfigurationId () {
	//DevCheck(0,1,0x18,E_NOT_OK);
	return PduRConfig->PduRConfigurationId;
}
#endif // End of not Zero Cost Operation Mode

Std_ReturnType PduR_CancelTransmitRequest(PduR_CancelReasonType PduCancelReason, PduIdType PduId) {
	Enter(PduId,E_NOT_OK);
	// TODO Implement!

	Exit();
	return E_NOT_OK;
}

void PduR_ChangeParameterRequest(PduR_ParameterValueType PduParameterValue, PduIdType PduId) {
	Enter(0);
	// TODO Implement!

}


// If we are using the simulator CANIF and LINIF are not available.
// Therefore the functions needed by the functions pointer tables below needs to be stubbed.
#if !defined(USE_CANIF)
Std_ReturnType CanIf_Transmit(PduIdType CanTxPduId, const PduInfoType *PduInfoPtr) {
	// Just a stub
	return E_OK;
}

// If CAN are available we include these functions directly
#else
#include "CanIf.h"
#endif


#if !defined(USE_LINIF)
Std_ReturnType LinIf_Transmit(PduIdType LinTxPduId,const PduInfoType* PduInfoPtr) {
	// Just a stub
	return E_OK;
}

// If LIN are available we include these functions directly
#else
#include "LinIf.h"
#endif



PduR_FctPtrType PduR_StdLinFctPtrs = {
	.TargetIndicationFctPtr = Com_RxIndication,
	.TargetTransmitFctPtr = LinIf_Transmit,
	.TargetConfirmationFctPtr = Com_TxConfirmation,
	.TargetTriggerTransmitFctPtr = Com_TriggerTransmit,
};

PduR_FctPtrType PduR_StdCanFctPtrs = {
	.TargetIndicationFctPtr = Com_RxIndication,
	.TargetTransmitFctPtr = CanIf_Transmit,
	.TargetConfirmationFctPtr = Com_TxConfirmation,
	.TargetTriggerTransmitFctPtr = Com_TriggerTransmit,
};

