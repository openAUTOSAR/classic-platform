/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/






#include "Com_Arc_Types.h"
#include "Com.h"
#include "Com_Internal.h"
#include "Com_misc.h"
#include <string.h>
#include "debug.h"
#include "Cpu.h"

#define timerDec(timer) \
	if (timer > 0) { \
		timer = timer - 1; \
	} \

/* Declared in Com_Cfg.c */
extern const ComNotificationCalloutType ComNotificationCallouts[];

void Com_MainFunctionRx(void) {
	/* !req COM513 */
	/* !req COM053 */
	/* !req COM352 */

	/* @req COM664 */
	if( COM_INIT != Com_GetStatus() ) {
		DET_REPORTERROR(COM_MAINFUNCTIONRX_ID, COM_E_UNINIT);
		return;
	}
	//DEBUG(DEBUG_MEDIUM, "Com_MainFunctionRx() excecuting\n");
	for (uint16 pduId = 0; !ComConfig->ComIPdu[pduId].Com_Arc_EOL; pduId++) {
		boolean pduUpdated = false;
		const ComIPdu_type *IPdu = GET_IPdu(pduId);
		Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(pduId);
		imask_t irq_state;
		Irq_Save(irq_state);
		for (uint16 sri = 0; (IPdu->ComIPduSignalRef != NULL) && (IPdu->ComIPduSignalRef[sri] != NULL); sri++) {
			const ComSignal_type *signal = IPdu->ComIPduSignalRef[sri];
			Com_Arc_Signal_type * Arc_Signal = GET_ArcSignal(signal->ComHandleId);
			// Monitor signal reception deadline
			/* @req COM685 */
			/* @req COM292 */
			/* @req COM290 */
			/* @req COM716 */
			if ( (Arc_IPdu->Com_Arc_IpduStarted) && (signal->ComTimeoutFactor > 0) ) {

				// Decrease deadline monitoring timer.
				timerDec(Arc_Signal->Com_Arc_DeadlineCounter);

				// Check if a timeout has occurred.
				if (Arc_Signal->Com_Arc_DeadlineCounter == 0) {
					/* @req COM500 */
					/* @req COM744 */
				    boolean signalChanged = FALSE;
					if (signal->Com_Arc_IsSignalGroup != FALSE) {
						if (signal->ComRxDataTimeoutAction == COM_TIMEOUT_DATA_ACTION_REPLACE) {

							for (uint32 i=0;signal->ComGroupSignal[i]!=NULL; i++)
							{
								Com_WriteSignalDataToPduBuffer(
										signal->ComGroupSignal[i]->ComHandleId,
										TRUE,
										signal->ComGroupSignal[i]->ComSignalInitValue,
										Arc_IPdu->ComIPduDataPtr,
										&signalChanged);
							}
							Arc_Signal->ComSignalUpdated = TRUE;
						}
					}
					else if (signal->ComRxDataTimeoutAction == COM_TIMEOUT_DATA_ACTION_REPLACE) {
						// Replace signal data.
						/* @req COM470 */
						Arc_Signal->ComSignalUpdated = TRUE;
						Com_WriteSignalDataToPdu(signal->ComHandleId, signal->ComSignalInitValue, &signalChanged);

					}

					// A timeout has occurred.
					/* @req COM556 */
					if ((signal->ComTimeoutNotification != COM_NO_FUNCTION_CALLOUT) && (ComNotificationCallouts[signal->ComTimeoutNotification] != NULL) ) {
						ComNotificationCallouts[signal->ComTimeoutNotification]();
					}

					// Restart timer
					/* !req COM291 */
					/* TODO: For signals and signal groups with update bit within the same
					 * I-PDU the smallest configured timeout parameter (ComSignalFirstTimeout,
					 * ComSignalGroupTimeout, ComSignalTimeout, ComSignalGroupTimeout)
					 * of the associated signals and signal groups should be used as reception
					 * timeout parameter. Handle this in generator?*/
					Arc_Signal->Com_Arc_DeadlineCounter = signal->ComTimeoutFactor;
				}
			}

			if (Arc_Signal->ComSignalUpdated) {
				pduUpdated = true;
			}
		}

		if (pduUpdated && IPdu->ComIPduSignalProcessing == DEFERRED && IPdu->ComIPduDirection == RECEIVE) {
			UnlockTpBuffer(getPduId(IPdu));
			memcpy(Arc_IPdu->ComIPduDeferredDataPtr,Arc_IPdu->ComIPduDataPtr,IPdu->ComIPduSize);
			for (uint16 i = 0; (IPdu->ComIPduSignalRef != NULL) && (IPdu->ComIPduSignalRef[i] != NULL); i++) {
				const ComSignal_type *signal = IPdu->ComIPduSignalRef[i];
				Com_Arc_Signal_type * Arc_Signal = GET_ArcSignal(signal->ComHandleId);
				if (Arc_Signal->ComSignalUpdated) {

					if ((signal->ComNotification != COM_NO_FUNCTION_CALLOUT) && (ComNotificationCallouts[signal->ComNotification] != NULL) ) {
						ComNotificationCallouts[signal->ComNotification]();
					}
					Arc_Signal->ComSignalUpdated = FALSE;
				}
			}
		}
		Irq_Restore(irq_state);
	}
}


void Com_MainFunctionTx(void) {
	/* !req	COM789 */
	/* !req COM305 */
	imask_t irq_state;
	/* @req COM665 */
	if( COM_INIT != Com_GetStatus() ) {
		DET_REPORTERROR(COM_MAINFUNCTIONTX_ID, COM_E_UNINIT);
		return;
	}
	//DEBUG(DEBUG_MEDIUM, "Com_MainFunctionTx() excecuting\n");
	// Decrease timers.
	const ComIPdu_type *IPdu;
	for (uint16 i = 0; !ComConfig->ComIPdu[i].Com_Arc_EOL; i++) {
		IPdu = &ComConfig->ComIPdu[i];
		Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(i);

		Irq_Save(irq_state);
		/* !req COM471 */
		// Is this a IPdu that should be transmitted?
		if ( (IPdu->ComIPduDirection == SEND) && (Arc_IPdu->Com_Arc_IpduStarted) ) {
			// Decrease minimum delay timer
			timerDec(Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduMinimumDelayTimer);

			// If IPDU has periodic or mixed transmission mode.
			if ( (IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeMode == PERIODIC)
				|| (IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeMode == MIXED) ) {

				timerDec(Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeTimePeriodTimer);

				// Is it time for a direct transmission?
				if ( (IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeMode == MIXED)
					&& (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft > 0) ) {

					timerDec(Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeRepetitionPeriodTimer);

					// Is it time for a transmission?
					if ( (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeRepetitionPeriodTimer == 0)
						&& (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduMinimumDelayTimer == 0) ) {

					    Com_TriggerIPDUSend(i);

						// Reset periodic timer
						Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeRepetitionPeriodTimer = IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeRepetitionPeriodFactor;

						// Register this nth-transmission.
						/* @req COM494 */
						Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft--;
					}
				}

				// Is it time for a cyclic transmission?
				if ( (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeTimePeriodTimer == 0) && (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduMinimumDelayTimer == 0) ) {

				    Com_TriggerIPDUSend(i);

					// Reset periodic timer.
					Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeTimePeriodTimer = IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeTimePeriodFactor;
				}

			// If IPDU has direct transmission mode.
			} else if (IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeMode == DIRECT) {
				// Do we need to transmit anything?
				if (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft > 0) {
					timerDec(Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeRepetitionPeriodTimer);

					// Is it time for a transmission?
					if ( (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeRepetitionPeriodTimer == 0) && (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduMinimumDelayTimer == 0) ) {
					    Com_TriggerIPDUSend(i);

						// Reset periodic timer
						Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeRepetitionPeriodTimer = IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeRepetitionPeriodFactor;

						// Register this nth-transmission.
						Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft--;
					}
				}

			// The IDPU has NONE transmission mode.
				/* @req COM135 */
			} else {
				// Don't send!
			}

		}

		Irq_Restore(irq_state);

        /* Check notifications */
        boolean confirmationStatus = GetTxConfirmationStatus(IPdu);
        SetTxConfirmationStatus(IPdu, FALSE);

        for (uint8 signalIndex = 0; confirmationStatus && (IPdu->ComIPduSignalRef[signalIndex] != NULL); signalIndex++) {
            const ComSignal_type *signal = IPdu->ComIPduSignalRef[signalIndex];
            if ((signal->ComNotification != COM_NO_FUNCTION_CALLOUT) &&
                (ComNotificationCallouts[signal->ComNotification] != NULL) ) {
                ComNotificationCallouts[signal->ComNotification]();
            }
        }
	}
}
