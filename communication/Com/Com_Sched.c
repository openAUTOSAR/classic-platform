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






#include "Com_Arc_Types.h"
#include "Com.h"
#include "Com_Internal.h"
#include "Com_misc.h"
#include <string.h>
#include "debug.h"

#define timerDec(timer) \
	if (timer > 0) { \
		timer = timer - 1; \
	} \

extern Com_Arc_Config_type Com_Arc_Config;

void Com_MainFunctionRx() {
	//DEBUG(DEBUG_MEDIUM, "Com_MainFunctionRx() excecuting\n");
	const ComSignal_type *signal;
	for (int i = 0; !ComConfig->ComSignal[i].Com_Arc_EOL; i++) {
		signal = &ComConfig->ComSignal[i];
		GET_ArcSignal(signal->ComHandleId);
		GET_ArcIPdu(Arc_Signal->ComIPduHandleId);

		// Monitor signal reception deadline
		if (Arc_IPdu->Com_Arc_IpduStarted && Arc_Signal->ComTimeoutFactor > 0) {

			// Decrease deadline monitoring timer.
			timerDec(Arc_Signal->Com_Arc_DeadlineCounter);

			// Check if a timeout has occurred.
			if (Arc_Signal->Com_Arc_DeadlineCounter == 0) {
				if (signal->ComRxDataTimeoutAction == COM_TIMEOUT_DATA_ACTION_REPLACE) {
					// Replace signal data.
					Com_WriteSignalDataToPdu(signal->ComHandleId, signal->ComSignalInitValue);

				}

				// A timeout has occurred.
				if (signal->ComTimeoutNotification != NULL) {
					signal->ComTimeoutNotification();
				}

				// Restart timer
				Arc_Signal->Com_Arc_DeadlineCounter = Arc_Signal->ComTimeoutFactor;
			}
		}

		if (Arc_Signal->ComSignalUpdated) {
			if (ComConfig->ComSignal[i].ComNotification != NULL) {
				ComConfig->ComSignal[i].ComNotification();
			}
			Arc_Signal->ComSignalUpdated = 0;
		}
	}
}


void Com_MainFunctionTx() {
	//DEBUG(DEBUG_MEDIUM, "Com_MainFunctionTx() excecuting\n");
	// Decrease timers.
	const ComIPdu_type *IPdu;
	for (int i = 0; !ComConfig->ComIPdu[i].Com_Arc_EOL; i++) {
		IPdu = &ComConfig->ComIPdu[i];
		GET_ArcIPdu(i);

		// Is this a IPdu that should be transmitted?
		if (IPdu->ComIPduDirection == SEND && Arc_IPdu->Com_Arc_IpduStarted) {
			// Decrease minimum delay timer
			timerDec(Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduMinimumDelayTimer);

			// If IPDU has periodic or mixed transmission mode.
			if (IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeMode == PERIODIC
				|| IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeMode == MIXED) {

				timerDec(Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeTimePeriodTimer);

				// Is it time for a direct transmission?
				if (IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeMode == MIXED
					&& Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft > 0) {

					timerDec(Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeRepetitionPeriodTimer);

					// Is it time for a transmission?
					if (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeRepetitionPeriodTimer == 0
						&& Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduMinimumDelayTimer == 0) {

						Com_TriggerIPduSend(i);

						// Reset periodic timer
						Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeRepetitionPeriodTimer = IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeRepetitionPeriodFactor;

						// Register this nth-transmission.
						Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft--;
					}
				}

				// Is it time for a cyclic transmission?
				if (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeTimePeriodTimer == 0 && Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduMinimumDelayTimer == 0) {

					Com_TriggerIPduSend(i);

					// Reset periodic timer.
					Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeTimePeriodTimer = IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeTimePeriodFactor;
				}

			// If IPDU has direct transmission mode.
			} else if (IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeMode == DIRECT) {
				// Do we need to transmit anything?
				if (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft > 0) {
					timerDec(Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeRepetitionPeriodTimer);

					// Is it time for a transmission?
					if (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeRepetitionPeriodTimer == 0 && Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduMinimumDelayTimer == 0) {
						Com_TriggerIPduSend(i);

						// Reset periodic timer
						Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeRepetitionPeriodTimer = IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeRepetitionPeriodFactor;

						// Register this nth-transmission.
						Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft--;
					}
				}

			// The IDPU has NONE transmission mode.
			} else {
				// Don't send!
			}
		}
	}

	// Send scheduled packages.
	// Cyclic
	for (int i = 0; !ComConfig->ComIPdu[i].Com_Arc_EOL; i++) {
		if (ComConfig->ComIPdu[i].ComIPduDirection == SEND) {

		}
	}
}
