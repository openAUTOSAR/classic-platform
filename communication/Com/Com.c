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


//lint -esym(960,8.7)	PC-Lint misunderstanding of Misra 8.7 for Com_SystenEndianness and endianess_test





#include <assert.h>
#include <stdlib.h>
//#include <stdio.h>
#include <string.h>
#include "Com.h"
#include "Com_Arc_Types.h"
#include "Com_Internal.h"
#include "Com_misc.h"
#include "debug.h"
#include "Cpu.h"


/* TODO: Better way to get endianness across all compilers? */
static const uint32_t endianness_test = 0xdeadbeefU;
ComSignalEndianess_type Com_SystemEndianness;


const Com_ConfigType * ComConfig;


void Com_Init(const Com_ConfigType *config ) {
	DEBUG(DEBUG_LOW, "--Initialization of COM--\n");

	ComConfig = config;

	uint8 failure = 0;

	uint32 firstTimeout;

	//lint --e(928)	PC-Lint exception Misra 11.4, Must be like this. /tojo
	uint8 endiannessByte = *(const uint8 *)&endianness_test;
	if      ( endiannessByte == 0xef ) { Com_SystemEndianness = COM_LITTLE_ENDIAN; }
	else if ( endiannessByte == 0xde ) { Com_SystemEndianness = COM_BIG_ENDIAN; }
	else {
		// No other endianness supported
		//lint --e(506)	PC-Lint exception Misra 13.7, 14.1, Allow boolean to always be false.
		assert(0);
	}

	// Initialize each IPdu
	//ComIPdu_type *IPdu;
	//Com_Arc_IPdu_type *Arc_IPdu;
	const ComSignal_type *Signal;
	const ComGroupSignal_type *GroupSignal;
	for (uint16 i = 0; !ComConfig->ComIPdu[i].Com_Arc_EOL; i++) {

		const ComIPdu_type *IPdu = GET_IPdu(i);
		Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(i);
		Arc_IPdu->Com_Arc_DynSignalLength = 0;

		if (i >= COM_N_IPDUS) {
			DET_REPORTERROR(COM_MODULE_ID, COM_INSTANCE_ID, 0x01, COM_E_TOO_MANY_IPDU);
			failure = 1;
			break;
		}

		// If this is a TX and cyclic IPdu, configure the first deadline.
		if ( (IPdu->ComIPduDirection == SEND) &&
				( (IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeMode == PERIODIC) || (IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeMode == MIXED) )) {
			//IPdu->Com_Arc_TxIPduTimers.ComTxModeTimePeriodTimer = IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeTimeOffsetFactor;
			Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeTimePeriodTimer = IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeTimeOffsetFactor;
		}


		// Reset firstTimeout.
		firstTimeout = 0xffffffffu;

		// Initialize the memory with the default value.
		if (IPdu->ComIPduDirection == SEND) {
			memset((void *)IPdu->ComIPduDataPtr, IPdu->ComTxIPdu.ComTxIPduUnusedAreasDefault, IPdu->ComIPduSize);
		}

		// For each signal in this PDU.
		//Arc_IPdu->NComIPduSignalRef = 0;
		for (uint16 j = 0; (IPdu->ComIPduSignalRef != NULL) && (IPdu->ComIPduSignalRef[j] != NULL) ; j++) {
			Signal = IPdu->ComIPduSignalRef[j];
			Com_Arc_Signal_type * Arc_Signal = GET_ArcSignal(Signal->ComHandleId);

			// Configure signal deadline monitoring if used.
			if (Signal->ComTimeoutFactor > 0) {

				if (Signal->ComSignalArcUseUpdateBit) {
					// This signal uses an update bit, and hence has its own deadline monitoring.
					Arc_Signal->Com_Arc_DeadlineCounter = Signal->ComFirstTimeoutFactor; // Configure the deadline counter

				} else {
					// This signal does not use an update bit, and should therefore use per I-PDU deadline monitoring.
					if (firstTimeout > Signal->ComFirstTimeoutFactor) {
						firstTimeout = Signal->ComFirstTimeoutFactor;
					}
				}
			}

			// Clear update bits
			if (Signal->ComSignalArcUseUpdateBit) {
				CLEARBIT(IPdu->ComIPduDataPtr, Signal->ComUpdateBitPosition);
			}

			// If this signal is a signal group
			if (Signal->Com_Arc_IsSignalGroup) {

				// For each group signal of this signal group.
				for(uint8 h = 0; Signal->ComGroupSignal[h] != NULL; h++) {
					GroupSignal = Signal->ComGroupSignal[h];
					Com_Arc_GroupSignal_type *Arc_GroupSignal = GET_ArcGroupSignal(GroupSignal->ComHandleId);
					// Set pointer to shadow buffer
					Arc_GroupSignal->Com_Arc_ShadowBuffer = (void *)Signal->Com_Arc_ShadowBuffer;
					// Initialize group signal data.
					Com_WriteGroupSignalDataToPdu(Signal->ComHandleId, GroupSignal->ComHandleId, GroupSignal->ComSignalInitValue);
				}
			} else {
				// Initialize signal data.
				Com_WriteSignalDataToPdu(Signal->ComHandleId, Signal->ComSignalInitValue);
			}
		}
		if (IPdu->ComIPduDirection == RECEIVE && IPdu->ComIPduSignalProcessing == DEFERRED) {
			// Copy the initialized pdu to deferred buffer
			memcpy(IPdu->ComIPduDeferredDataPtr,IPdu->ComIPduDataPtr,IPdu->ComIPduSize);
		}
		// Configure per I-PDU based deadline monitoring.
		for (uint16 j = 0; (IPdu->ComIPduSignalRef != NULL) && (IPdu->ComIPduSignalRef[j] != NULL); j++) {
			Signal = IPdu->ComIPduSignalRef[j];
			Com_Arc_Signal_type * Arc_Signal = GET_ArcSignal(Signal->ComHandleId);

			if ( (Signal->ComTimeoutFactor > 0) && (!Signal->ComSignalArcUseUpdateBit) ) {
				Arc_Signal->Com_Arc_DeadlineCounter = firstTimeout;
			}
		}
	}
	for (uint16 i = 0; i < COM_N_IPDUS; i++) {
		Com_BufferPduState[i].currentPosition = 0;
		Com_BufferPduState[i].locked = false;
	}

	// An error occurred.
	if (failure) {
		DEBUG(DEBUG_LOW, "--Initialization of COM failed--\n");
		//DET_REPORTERROR(COM_MODULE_ID, COM_INSTANCE_ID, 0x01, COM_E_INVALID_FILTER_CONFIGURATION);
	} else {
		DEBUG(DEBUG_LOW, "--Initialization of COM completed--\n");
	}
}


void Com_DeInit( void ) {

}

void Com_IpduGroupStart(Com_PduGroupIdType IpduGroupId,boolean Initialize) {
	(void)Initialize; // Nothing to be done. This is just to avoid Lint warning.
	for (uint16 i = 0; !ComConfig->ComIPdu[i].Com_Arc_EOL; i++) {
		if (ComConfig->ComIPdu[i].ComIPduGroupRef == IpduGroupId) {
			Com_Arc_Config.ComIPdu[i].Com_Arc_IpduStarted = 1;
		}
	}
}

void Com_IpduGroupStop(Com_PduGroupIdType IpduGroupId) {
	for (uint16 i = 0; !ComConfig->ComIPdu[i].Com_Arc_EOL; i++) {
		if (ComConfig->ComIPdu[i].ComIPduGroupRef == IpduGroupId) {
			Com_Arc_Config.ComIPdu[i].Com_Arc_IpduStarted = 0;
		}
	}
}

/**
 *
 * @param PduId
 * @param PduInfoPtr
 * @param RetryInfoPtr not supported
 * @param TxDataCntPtr
 * @return
 */
BufReq_ReturnType Com_CopyTxData(PduIdType PduId, PduInfoType* PduInfoPtr, RetryInfoType* RetryInfoPtr, PduLengthType* TxDataCntPtr) {
	imask_t state;
	BufReq_ReturnType r = BUFREQ_OK;
	const ComIPdu_type *IPdu = GET_IPdu(PduId);
	boolean dirOk = ComConfig->ComIPdu[PduId].ComIPduDirection == SEND;
	boolean sizeOk;
	(void)RetryInfoPtr; // get rid of compiler warning

	Irq_Save(state);
	sizeOk = IPdu->ComIPduSize >= Com_BufferPduState[PduId].currentPosition + PduInfoPtr->SduLength;
	Com_BufferPduState[PduId].locked = true;
	if (dirOk && sizeOk) {
		void* source = (void *)IPdu->ComIPduDataPtr;
		memcpy(PduInfoPtr->SduDataPtr,source + Com_BufferPduState[PduId].currentPosition, PduInfoPtr->SduLength);
		Com_BufferPduState[PduId].currentPosition += PduInfoPtr->SduLength;
		*TxDataCntPtr = IPdu->ComIPduSize - Com_BufferPduState[PduId].currentPosition;
	} else {
		r = BUFREQ_NOT_OK;
	}
	Irq_Restore(state);
	return r;
}
BufReq_ReturnType Com_CopyRxData(PduIdType PduId, const PduInfoType* PduInfoPtr, PduLengthType* RxBufferSizePtr) {
	imask_t state;
	BufReq_ReturnType r = BUFREQ_OK;
	uint8 remainingBytes;
	boolean sizeOk;
	boolean dirOk;
	boolean lockOk;

	Irq_Save(state);

	remainingBytes = GET_IPdu(PduId)->ComIPduSize - Com_BufferPduState[PduId].currentPosition;
	sizeOk = remainingBytes >= PduInfoPtr->SduLength;
    dirOk = GET_IPdu(PduId)->ComIPduDirection == RECEIVE;
	lockOk = isPduBufferLocked(PduId);
	if (dirOk && lockOk && sizeOk) {
		memcpy((void *)(GET_IPdu(PduId)->ComIPduDataPtr+Com_BufferPduState[PduId].currentPosition), PduInfoPtr->SduDataPtr, PduInfoPtr->SduLength);
		Com_BufferPduState[PduId].currentPosition += PduInfoPtr->SduLength;
		*RxBufferSizePtr = GET_IPdu(PduId)->ComIPduSize - Com_BufferPduState[PduId].currentPosition;
	} else {
		r = BUFREQ_NOT_OK;
	}
	Irq_Restore(state);
	return r;
}

static void Com_SetDynSignalLength(PduIdType ComRxPduId,PduLengthType TpSduLength) {
	const ComIPdu_type *IPdu = GET_IPdu(ComRxPduId);
	if (IPdu->ComIPduDynSignalRef == 0) {
		return;
	}
	const ComSignal_type * const dynSignal = IPdu->ComIPduDynSignalRef;
	Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(ComRxPduId);
	Arc_IPdu->Com_Arc_DynSignalLength = TpSduLength - (dynSignal->ComBitPosition/8);
	return;
}

BufReq_ReturnType Com_StartOfReception(PduIdType ComRxPduId, PduLengthType TpSduLength, PduLengthType* RxBufferSizePtr) {
	PduLengthType ComIPduSize;
	imask_t state;
	BufReq_ReturnType r = BUFREQ_OK;
	Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(ComRxPduId);

	Irq_Save(state);
	if (Arc_IPdu->Com_Arc_IpduStarted) {
		if (GET_IPdu(ComRxPduId)->ComIPduDirection == RECEIVE) {
			if (!Com_BufferPduState[ComRxPduId].locked) {
				ComIPduSize = GET_IPdu(ComRxPduId)->ComIPduSize;
				if (ComIPduSize >= TpSduLength) {
					Com_BufferPduState[ComRxPduId].locked = true;
					*RxBufferSizePtr = ComIPduSize;
					Com_SetDynSignalLength(ComRxPduId,TpSduLength);
				} else {
					r = BUFREQ_OVFL;
				}
			} else {
				r = BUFREQ_BUSY;
			}
		}
	} else {
		r = BUFREQ_NOT_OK;
	}
	Irq_Restore(state);
	return r;
}
