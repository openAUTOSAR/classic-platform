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
//#include <stdio.h>
#include <string.h>
#include "Com.h"
#include "Com_Types.h"
#include "Trace.h"

#ifdef COM_DEV_ERROR_DETECT
#include "Det.h"
#endif

const Com_ConfigType * ComConfig;

ComEcoreIPdu_type ComEcoreIPdu[COM_MAX_NR_IPDU];
ComEcoreSignal_type ComEcoreSignal[COM_MAX_NR_SIGNAL];
ComEcoreGroupSignal_type ComEcoreGroupSignal[COM_MAX_NR_GROUPSIGNAL];

ComEcoreConfig_type ComEcoreConfig = {
	.ComIPdu = ComEcoreIPdu,
	.ComSignal = ComEcoreSignal,
	.ComGroupSignal = ComEcoreGroupSignal
};


void Com_Init(const Com_ConfigType *config ) {
	DEBUG(DEBUG_LOW, "--Initialization of COM--\n");

	ComConfig = config;

	uint8 failure = 0;

	uint32 earliestDeadline;
	uint32 firstTimeout;

	ComEcoreConfig.OutgoingPdu.SduDataPtr = malloc(8);

	// Initialize each IPdu
	//ComIPdu_type *IPdu;
	//ComEcoreIPdu_type *EcoreIPdu;
	const ComSignal_type *Signal;
	const ComGroupSignal_type *GroupSignal;
	for (int i = 0; !ComConfig->ComIPdu[i].ComEcoreEOL; i++) {
		ComEcoreConfig.ComNIPdu++;

		ComGetIPdu(i);
		ComGetEcoreIPdu(i);

		if (i >= COM_MAX_NR_IPDU) {
			DET_REPORTERROR(COM_MODULE_ID, COM_INSTANCE_ID, 0x01, COM_E_TOO_MANY_IPDU);
			failure = 1;
			break;
		}

		// If this is a TX and cyclic IPdu, configure the first deadline.
		if (IPdu->ComIPduDirection == SEND &&
				(IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeMode == PERIODIC || IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeMode == MIXED)) {
			//IPdu->ComEcoreTxIPduTimers.ComTxModeTimePeriodTimer = IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeTimeOffsetFactor;
			EcoreIPdu->ComEcoreTxIPduTimers.ComTxModeTimePeriodTimer = IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeTimeOffsetFactor;
		}


		// Reset earliest deadline.
		earliestDeadline = -1; // Gives the max value of uint32 due to overflow.
		firstTimeout = -1;

		// Reserve memory for all defined signals.
		EcoreIPdu->ComIPduDataPtr = malloc(IPdu->ComIPduSize);
		if (EcoreIPdu->ComIPduDataPtr == NULL) {
			failure = 1;
		}

		// Initialize the memory with the default value.
		if (IPdu->ComIPduDirection == SEND) {
			memset(EcoreIPdu->ComIPduDataPtr, IPdu->ComTxIPdu.ComTxIPduUnusedAreasDefault, IPdu->ComIPduSize);
		}

		// For each signal in this PDU.
		for (int j = 0; IPdu->ComIPduSignalRef[j] != NULL; j++) {
			Signal = IPdu->ComIPduSignalRef[j];
			ComGetEcoreSignal(Signal->ComHandleId);

			// If this signal already has been configured this is most likely an error.
			if (EcoreSignal->ComIPduDataPtr != NULL) {
				DET_REPORTERROR(COM_MODULE_ID, COM_INSTANCE_ID, 0x01, COM_E_INVALID_SIGNAL_CONFIGURATION);
				failure = 1;
			}

			// Configure signal deadline monitoring if used.
			if (Signal->ComTimeoutFactor > 0) {

				if (Signal->ComSignalEcoreUseUpdateBit) {
					// This signal uses an update bit, and hence has its own deadline monitoring.
					EcoreSignal->ComEcoreDeadlineCounter = Signal->ComFirstTimeoutFactor; // Configure the deadline counter
					EcoreSignal->ComTimeoutFactor = Signal->ComTimeoutFactor;

				} else {
					// This signal does not use an update bit, and should therefore use per I-PDU deadline monitoring.
					// Find the earliest deadline for this I-PDU and setup the deadline later.
					if (earliestDeadline > Signal->ComTimeoutFactor) {
						earliestDeadline = Signal->ComTimeoutFactor;
					}
					if (firstTimeout > Signal->ComFirstTimeoutFactor) {
						firstTimeout = Signal->ComFirstTimeoutFactor;
					}
				}
			}

			// Increment helper counters
		    EcoreIPdu->NComIPduSignalRef = j + 1;

			//IPdu->ComEcoreNIPduSignalGroupRef = j + 1;

			EcoreSignal->ComIPduDataPtr = EcoreIPdu->ComIPduDataPtr;
			EcoreSignal->ComIPduHandleId = i;

			// Clear update bits
			if (Signal->ComSignalEcoreUseUpdateBit) {
				clearBit(EcoreIPdu->ComIPduDataPtr, Signal->ComUpdateBitPosition);
			}

			// If this signal is a signal group
			if (Signal->ComEcoreIsSignalGroup) {
				EcoreSignal->ComEcoreShadowBuffer = malloc(IPdu->ComIPduSize);

				if (EcoreSignal->ComEcoreShadowBuffer == NULL) {
					failure = 1;
				}

				// For each group signal of this signal group.
				for(int h = 0; Signal->ComGroupSignal[h] != NULL; h++) {
					GroupSignal = Signal->ComGroupSignal[h];
					ComGetEcoreGroupSignal(GroupSignal->ComHandleId);
					// Set pointer to shadow buffer
					EcoreGroupSignal->ComEcoreShadowBuffer = EcoreSignal->ComEcoreShadowBuffer;
					// Initialize group signal data.
					Com_CopyData(EcoreIPdu->ComIPduDataPtr, &GroupSignal->ComSignalInitValue, GroupSignal->ComBitSize, GroupSignal->ComBitPosition, 0);
				}

			} else {
				// Initialize signal data.
				Com_CopyData(EcoreIPdu->ComIPduDataPtr, &Signal->ComSignalInitValue, Signal->ComBitSize, Signal->ComBitPosition, 0);
			}

			// Check filter configuration
			if (IPdu->ComIPduDirection == RECEIVE) {

				// This represents an invalid configuration of the UINT8_N datatype
				if ((Signal->ComSignalType == UINT8_N
					&&
					(Signal->ComFilter.ComFilterAlgorithm == MASKED_NEW_EQUALS_X
					|| Signal->ComFilter.ComFilterAlgorithm == MASKED_NEW_DIFFERS_X
					|| Signal->ComFilter.ComFilterAlgorithm == MASKED_NEW_DIFFERS_MASKED_OLD
					|| Signal->ComFilter.ComFilterAlgorithm == NEW_IS_WITHIN
					|| Signal->ComFilter.ComFilterAlgorithm == NEW_IS_OUTSIDE
					|| Signal->ComFilter.ComFilterAlgorithm == ONE_EVERY_N))) {

					DET_REPORTERROR(COM_MODULE_ID, COM_INSTANCE_ID, 0x01, COM_E_INVALID_FILTER_CONFIGURATION);
					failure = 1;
				}

				// This represens an invalid configuration of the BOOLEAN datatype
				if ((Signal->ComSignalType == BOOLEAN
					&&
					(Signal->ComFilter.ComFilterAlgorithm == NEW_IS_WITHIN
					|| Signal->ComFilter.ComFilterAlgorithm == NEW_IS_OUTSIDE))) {


					DET_REPORTERROR(COM_MODULE_ID, COM_INSTANCE_ID, 0x01, COM_E_INVALID_FILTER_CONFIGURATION);
					failure = 1;
				}
			// Initialize filter values. COM230
			//signal.ComFilter.ComFilterNewValue = ComConfig->ComIPdu[i].ComTxIPdu.ComTxIPduUnusedAreasDefault;
			//signal.ComFilter.ComFilterOldValue = ComConfig->ComIPdu[i].ComTxIPdu.ComTxIPduUnusedAreasDefault;
			}
		}

		// Configure per I-PDU based deadline monitoring.
		for (int j = 0; IPdu->ComIPduSignalRef[j] != NULL; j++) {
			Signal = IPdu->ComIPduSignalRef[j];
			ComGetEcoreSignal(Signal->ComHandleId);

			if (Signal->ComTimeoutFactor > 0 && !Signal->ComSignalEcoreUseUpdateBit) {
				EcoreSignal->ComTimeoutFactor = earliestDeadline;
				EcoreSignal->ComEcoreDeadlineCounter = firstTimeout;
			}
		}
	}


	// An error occurred.
	if (failure) {
		// Free allocated memory
		for (int i = 0; !ComConfig->ComIPdu[i].ComEcoreEOL; i++) {
			// Release memory for all defined signals.
			//free(ComConfig->ComIPdu[i].ComIPduDataPtr);
		}
		DEBUG(DEBUG_LOW, "--Initialization of COM failed--\n");
		//DET_REPORTERROR(COM_MODULE_ID, COM_INSTANCE_ID, 0x01, COM_E_INVALID_FILTER_CONFIGURATION);
	} else {
		DEBUG(DEBUG_LOW, "--Initialization of COM completed--\n");
	}
}


void Com_DeInit( void ) {

}

void Com_IpduGroupStart(Com_PduGroupIdType IpduGroupId,boolean Initialize) {
	for (int i = 0; !ComConfig->ComIPdu[i].ComEcoreEOL; i++) {
		if (ComConfig->ComIPdu[i].ComIPduGroupRef == IpduGroupId) {
			ComEcoreConfig.ComIPdu[i].ComEcoreIpduStarted = 1;
		}
	}
}

void Com_IpduGroupStop(Com_PduGroupIdType IpduGroupId) {
	for (int i = 0; !ComConfig->ComIPdu[i].ComEcoreEOL; i++) {
		if (ComConfig->ComIPdu[i].ComIPduGroupRef == IpduGroupId) {
			ComEcoreConfig.ComIPdu[i].ComEcoreIpduStarted = 0;
		}
	}
}
