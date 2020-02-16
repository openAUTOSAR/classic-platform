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


//lint -esym(960,8.7)	PC-Lint misunderstanding of Misra 8.7 for Com_SystenEndianness and endianess_test





#include <stdlib.h>
#include <string.h>
#include "Com_Arc_Types.h"
#include "Com.h"
#include "Com_Internal.h"
#include "Com_misc.h"
#include "debug.h"
#include "PduR.h"
#include "Det.h"
#include "Cpu.h"
#include "Uart.h"
/* Declared in Com_Cfg.c */
extern const ComRxIPduCalloutType ComRxIPduCallouts[];
extern const ComTxIPduCalloutType ComTxIPduCallouts[];
extern const ComNotificationCalloutType ComNotificationCallouts[];


Com_BufferPduStateType Com_BufferPduState[COM_MAX_N_IPDUS];


uint8 Com_SendSignal(Com_SignalIdType SignalId, const void *SignalDataPtr) {
	/* @req COM334 */ /* Shall update buffer if pdu stopped, should not store trigger */
	/* !req COM055 */
	uint8 ret = E_OK;
	boolean dataChanged = FALSE;

	if( COM_INIT != Com_GetStatus() ) {
		DET_REPORTERROR(COM_SENDSIGNAL_ID, COM_E_UNINIT);
		return COM_SERVICE_NOT_AVAILABLE;
	}

	if(SignalId >= ComConfig->ComNofSignals) {
//		DET_REPORTERROR(COM_SENDSIGNAL_ID, COM_E_PARAM);
		DET_REPORTERROR(COM_SENDSIGNAL_ID, COM_INVALID_SIGNAL_ID);
		return COM_SERVICE_NOT_AVAILABLE;
	}

	// Store pointer to signal for easier coding.
	const ComSignal_type * Signal = GET_Signal(SignalId);

    if (Signal->ComIPduHandleId == NO_PDU_REFERENCE) {
        /* Return error if signal is not connected to an IPdu*/
        return COM_SERVICE_NOT_AVAILABLE;
    }

    const ComIPdu_type *IPdu = GET_IPdu(Signal->ComIPduHandleId);
    Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(Signal->ComIPduHandleId);

    if (isPduBufferLocked(getPduId(IPdu))) {
        return COM_BUSY;
    }
	//DEBUG(DEBUG_LOW, "Com_SendSignal: id %d, nBytes %d, BitPosition %d, intVal %d\n", SignalId, nBytes, signal->ComBitPosition, (uint32)*(uint8 *)SignalDataPtr);

    imask_t irq_state;

    Irq_Save(irq_state);
    /* @req COM624 */
    Com_WriteSignalDataToPdu(Signal->ComHandleId, SignalDataPtr, &dataChanged);

    // If the signal has an update bit. Set it!
    /* @req COM061 */
    if (Signal->ComSignalArcUseUpdateBit) {
        SETBIT(Arc_IPdu->ComIPduDataPtr, Signal->ComUpdateBitPosition);
    }

    if( Arc_IPdu->Com_Arc_IpduStarted ) {
        /*
         * If signal has triggered transmit property, trigger a transmission!
         */
        /* @req COM767 */
        /* @req COM734 */
        /* @req COM768 */
        /* @req COM762 *//* Signal with ComBitSize 0 should never be detected as changed */
        if ( (TRIGGERED == Signal->ComTransferProperty) || ( TRIGGERED_WITHOUT_REPETITION == Signal->ComTransferProperty ) ||
                ( ((TRIGGERED_ON_CHANGE == Signal->ComTransferProperty) || ( TRIGGERED_ON_CHANGE_WITHOUT_REPETITION == Signal->ComTransferProperty )) && dataChanged)) {
            /* !req COM625 */
            /* @req COM279 */
            /* @req COM330 */
            /* @req COM467 */ /* Though RetryFailedTransmitRequests not supported. */
            /* @req COM305.1 */

            uint8 nofReps = 0;
            switch(Signal->ComTransferProperty) {
                case TRIGGERED:
                case TRIGGERED_ON_CHANGE:
                    if( 0 == IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeNumberOfRepetitions ) {
                        nofReps = 1;
                    } else {
                        nofReps = IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeNumberOfRepetitions;
                    }
                    break;
                case TRIGGERED_WITHOUT_REPETITION:
                case TRIGGERED_ON_CHANGE_WITHOUT_REPETITION:
                    nofReps = 1;
                    break;
                default:
                    break;
            }
            /* Do not cancel outstanding repetitions triggered by other signals  */
            if( Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft < nofReps ) {
                Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft = nofReps;
            }
        }
    } else {
        ret = COM_SERVICE_NOT_AVAILABLE;
    }
    Irq_Restore(irq_state);

    return ret;
}

uint8 Com_ReceiveSignal(Com_SignalIdType SignalId, void* SignalDataPtr) {

	if( COM_INIT != Com_GetStatus() ) {
		DET_REPORTERROR(COM_RECEIVESIGNAL_ID, COM_E_UNINIT);
		return COM_SERVICE_NOT_AVAILABLE;
	}
	if(SignalId >= ComConfig->ComNofSignals) {
//		DET_REPORTERROR(COM_RECEIVESIGNAL_ID, COM_E_PARAM);
		DET_REPORTERROR(COM_RECEIVESIGNAL_ID, COM_INVALID_SIGNAL_ID);
		return COM_SERVICE_NOT_AVAILABLE;
	}
	DEBUG(DEBUG_LOW, "Com_ReceiveSignal: SignalId %d\n", SignalId);
	const ComSignal_type * Signal = GET_Signal(SignalId);

    if (Signal->ComIPduHandleId == NO_PDU_REFERENCE) {
        /* Return error init value signal if signal is not connected to an IPdu*/
        memcpy(SignalDataPtr, Signal->ComSignalInitValue, (Signal->ComBitSize/8) + 1);
        return E_OK;
    }

    const ComIPdu_type *IPdu = GET_IPdu(Signal->ComIPduHandleId);
    Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(Signal->ComIPduHandleId);

    uint8 ret = E_OK;
    const void* pduDataPtr = 0;
    if (IPdu->ComIPduSignalProcessing == DEFERRED && IPdu->ComIPduDirection == RECEIVE) {
        pduDataPtr = Arc_IPdu->ComIPduDeferredDataPtr;
    } else {
        if (isPduBufferLocked(getPduId(IPdu))) {
            ret = COM_BUSY;
        }
        pduDataPtr = Arc_IPdu->ComIPduDataPtr;
    }
    /* @req COM631 */
    Com_ReadSignalDataFromPduBuffer(
            SignalId,
            FALSE,
            SignalDataPtr,
            pduDataPtr);


    if( !GET_ArcIPdu(Signal->ComIPduHandleId)->Com_Arc_IpduStarted && (E_OK == ret) ) {
        ret = COM_SERVICE_NOT_AVAILABLE;
    }
    return ret;
}

uint8 Com_ReceiveDynSignal(Com_SignalIdType SignalId, void* SignalDataPtr, uint16* Length) {
    /* TODO: Validate signal id?*/
    if( COM_INIT != Com_GetStatus() ) {
        DET_REPORTERROR(COM_RECEIVEDYNSIGNAL_ID, COM_E_UNINIT);
        return COM_SERVICE_NOT_AVAILABLE;
    }
    const ComSignal_type * Signal = GET_Signal(SignalId);
    Com_Arc_IPdu_type    *Arc_IPdu   = GET_ArcIPdu(Signal->ComIPduHandleId);
    const ComIPdu_type   *IPdu       = GET_IPdu(Signal->ComIPduHandleId);
    imask_t state;

    Com_SignalType signalType = Signal->ComSignalType;
    /* @req COM753 */
    if (signalType != UINT8_DYN) {
        return COM_SERVICE_NOT_AVAILABLE;
    }
    uint8 ret = E_OK;
    Irq_Save(state);
    /* @req COM712 */
    if( *Length >= Arc_IPdu->Com_Arc_DynSignalLength ) {
        if (*Length > Arc_IPdu->Com_Arc_DynSignalLength) {
            *Length = Arc_IPdu->Com_Arc_DynSignalLength;
        }
        uint8 startFromPduByte = (Signal->ComBitPosition) / 8;
        const void* pduDataPtr = 0;
        if (IPdu->ComIPduSignalProcessing == DEFERRED && IPdu->ComIPduDirection == RECEIVE) {
            pduDataPtr = Arc_IPdu->ComIPduDeferredDataPtr;
        } else {
            if (isPduBufferLocked(getPduId(IPdu))) {
                ret = COM_BUSY;
            }
            pduDataPtr = Arc_IPdu->ComIPduDataPtr;
        }
        /* @req COM711 */
        memcpy(SignalDataPtr, (uint8 *)pduDataPtr + startFromPduByte, *Length);

        Irq_Restore(state);
    } else {
        /* @req COM724 */
        *Length = Arc_IPdu->Com_Arc_DynSignalLength;
        ret = E_NOT_OK;
    }
    if( !Arc_IPdu->Com_Arc_IpduStarted && (E_OK == ret) ) {
        ret = COM_SERVICE_NOT_AVAILABLE;
    }
    return ret;
}

uint8 Com_SendDynSignal(Com_SignalIdType SignalId, const void* SignalDataPtr, uint16 Length) {
    /* TODO: validate SignalId */
    /* !req COM629 */
    /* @req COM630 */
    if( COM_INIT != Com_GetStatus() ) {
        DET_REPORTERROR(COM_SENDDYNSIGNAL_ID, COM_E_UNINIT);
        return COM_SERVICE_NOT_AVAILABLE;
    }
    const ComSignal_type * Signal = GET_Signal(SignalId);
    Com_Arc_IPdu_type    *Arc_IPdu   = GET_ArcIPdu(Signal->ComIPduHandleId);
    const ComIPdu_type   *IPdu       = GET_IPdu(Signal->ComIPduHandleId);
    imask_t state;
    uint8 ret = E_OK;
    Com_SignalType signalType = Signal->ComSignalType;
    boolean dataChanged = FALSE;

    /* @req COM753 */
    if (signalType != UINT8_DYN) {
        return COM_SERVICE_NOT_AVAILABLE;
    }
    if (isPduBufferLocked(getPduId(IPdu))) {
        return COM_BUSY;
    }
    uint8 signalLength = Signal->ComBitSize / 8;
    Com_BitPositionType bitPosition = Signal->ComBitPosition;
    if (signalLength < Length) {
        return E_NOT_OK;
    }
    uint8 startFromPduByte = bitPosition / 8;

    Irq_Save(state);
    if( (Arc_IPdu->Com_Arc_DynSignalLength != Length) ||
            (0 != memcmp((void *)((uint8 *)Arc_IPdu->ComIPduDataPtr + startFromPduByte), SignalDataPtr, Length)) ) {
        dataChanged = TRUE;
    }
    /* @req COM628 */
    memcpy((void *)((uint8 *)Arc_IPdu->ComIPduDataPtr + startFromPduByte), SignalDataPtr, Length);
    /* !req COM757 */ //Length of I-PDU?
    Arc_IPdu->Com_Arc_DynSignalLength = Length;
    // If the signal has an update bit. Set it!
    if (Signal->ComSignalArcUseUpdateBit) {
        SETBIT(Arc_IPdu->ComIPduDataPtr, Signal->ComUpdateBitPosition);
    }
    if( Arc_IPdu->Com_Arc_IpduStarted ) {
         // If signal has triggered transmit property, trigger a transmission!
        /* @req COM767 */
        /* @req COM734 */ /* TODO: Check this */
        /* @req COM768 */
        /* @req COM762 *//* Signal with ComBitSize 0 should never be detected as changed */
        if ( (TRIGGERED == Signal->ComTransferProperty) || ( TRIGGERED_WITHOUT_REPETITION == Signal->ComTransferProperty ) ||
                ( ((TRIGGERED_ON_CHANGE == Signal->ComTransferProperty) || ( TRIGGERED_ON_CHANGE_WITHOUT_REPETITION == Signal->ComTransferProperty )) && dataChanged)) {
            /* !req COM625 */
            /* @req COM279 */
            /* @req COM330 */
            /* @req COM467 */ /* Though RetryFailedTransmitRequests not supported. */
            /* @req COM305.1 */
            uint8 nofReps = 0;
            switch(Signal->ComTransferProperty) {
                case TRIGGERED:
                case TRIGGERED_ON_CHANGE:
                    if( 0 == IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeNumberOfRepetitions ) {
                        nofReps = 1;
                    } else {
                        nofReps = IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeNumberOfRepetitions;
                    }
                    break;
                case TRIGGERED_WITHOUT_REPETITION:
                case TRIGGERED_ON_CHANGE_WITHOUT_REPETITION:
                    nofReps = 1;
                    break;
                default:
                    break;
            }
            /* Do not cancel outstanding repetitions triggered by other signals  */
            if( Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft < nofReps ) {
                Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft = nofReps;
            }
        }
    } else {
        ret = COM_SERVICE_NOT_AVAILABLE;
    }
    Irq_Restore(state);

    return ret;
}

Std_ReturnType Com_TriggerTransmit(PduIdType TxPduId, PduInfoType *PduInfoPtr) {
	/* TODO: Return E_NOT_OK if pdu stopped , see table 5 in com-spec*/
	/* @req COM475 */
	/* !req COM578 */
	/* !req COM766 */
	/* !req COM395 */
	if( COM_INIT != Com_GetStatus() ) {
		DET_REPORTERROR(COM_TRIGGERTRANSMIT_ID, COM_E_UNINIT);
		return E_NOT_OK;
	}
	if( TxPduId >= ComConfig->ComNofIPdus ) {
//		DET_REPORTERROR(COM_TRIGGERTRANSMIT_ID, COM_E_PARAM);
		DET_REPORTERROR(COM_TRIGGERTRANSMIT_ID, COM_INVALID_PDU_ID);
		return E_NOT_OK;
	}
	/*
	 * !req COM260: This function must not check the transmission mode of the I-PDU
	 * since it should be possible to use it regardless of the transmission mode.
	 * Only for ComIPduType = NORMAL.
	 */

	/*
	 * !req COM395: This function must override the IPdu callouts used in Com_TriggerIPduTransmit();
	 */
	const ComIPdu_type *IPdu = GET_IPdu(TxPduId);
	Com_Arc_IPdu_type    *Arc_IPdu   = GET_ArcIPdu(TxPduId);
    imask_t state;
    Irq_Save(state);
    /* @req COM647 */
    /* @req COM648 */ /* Interrups disabled */
    memcpy(PduInfoPtr->SduDataPtr, Arc_IPdu->ComIPduDataPtr, IPdu->ComIPduSize);

    Irq_Restore(state);

	PduInfoPtr->SduLength = IPdu->ComIPduSize;
	if( !Arc_IPdu->Com_Arc_IpduStarted ) {
		return E_NOT_OK;
	} else {
		return E_OK;
	}
}


//lint -esym(904, Com_TriggerIPduSend) //PC-Lint Exception of rule 14.7
void Com_TriggerIPDUSend(PduIdType PduId) {
	/* !req	COM789 */
	/* !req	COM662 */ /* May be supported, but when is buffer locked?*/

	if( COM_INIT != Com_GetStatus() ) {
		DET_REPORTERROR(COM_TRIGGERIPDUSEND_ID, COM_E_UNINIT);
		return;
	}
	if( PduId >= ComConfig->ComNofIPdus ) {
//		DET_REPORTERROR(COM_TRIGGERIPDUSEND_ID, COM_E_PARAM);
		DET_REPORTERROR(COM_TRIGGERIPDUSEND_ID, COM_INVALID_PDU_ID);
		return;
	}

	const ComIPdu_type *IPdu = GET_IPdu(PduId);
	Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(PduId);
    imask_t state;
    Irq_Save(state);

	// Is the IPdu ready for transmission?
    /* @req COM388 */
    /* TODO: Do not send if pdu stopped?*/
	if (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduMinimumDelayTimer == 0) {

        //lint --e(725)	Suppress PC-Lint warning "Expected positive indentation...". What means?
		// Check callout status
		/* @req COM492 */
		/* @req COM346 */
		/* !req COM381 */
		/* TODO: No other COM API than Com_TriggerIPDUSend, Com_SendSignal or Com_SendSignalGroup
		 * can be called from an an I-PDU callout.*/
		/* !req COM780 */
		/* !req COM781 */
		/* @req COM719 */
		if ((IPdu->ComTxIPduCallout != COM_NO_FUNCTION_CALLOUT) && (ComTxIPduCallouts[IPdu->ComTxIPduCallout] != NULL) ) {
			if (!ComTxIPduCallouts[IPdu->ComTxIPduCallout](PduId, Arc_IPdu->ComIPduDataPtr)) {
				// TODO Report error to DET.
				// Det_ReportError();
			    Irq_Restore(state);
				return;
			}
		}
		PduInfoType PduInfoPackage;
		PduInfoPackage.SduDataPtr = (uint8 *)Arc_IPdu->ComIPduDataPtr;
		if (IPdu->ComIPduDynSignalRef != 0) {
			/* !req COM757 */ //Length of I-PDU?
			uint16 sizeWithoutDynSignal = IPdu->ComIPduSize - (IPdu->ComIPduDynSignalRef->ComBitSize/8);
			PduInfoPackage.SduLength = sizeWithoutDynSignal + Arc_IPdu->Com_Arc_DynSignalLength;
		} else {
			PduInfoPackage.SduLength = IPdu->ComIPduSize;
		}

		// Send IPdu!
		/* @req COM138 */
		if (PduR_ComTransmit(IPdu->ArcIPduOutgoingId, &PduInfoPackage) == E_OK) {
			// Clear all update bits for the contained signals
			/* @req COM062 */
			/* !req COM577 */
			for (uint8 i = 0; (IPdu->ComIPduSignalRef != NULL) && (IPdu->ComIPduSignalRef[i] != NULL); i++) {
				if (IPdu->ComIPduSignalRef[i]->ComSignalArcUseUpdateBit) {
					CLEARBIT(Arc_IPdu->ComIPduDataPtr, IPdu->ComIPduSignalRef[i]->ComUpdateBitPosition);
				}
			}
		} else {
			UnlockTpBuffer(getPduId(IPdu));
		}

		// Reset miminum delay timer.
		/* @req	COM471 */
		/* @req	COM698 */
		Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduMinimumDelayTimer = IPdu->ComTxIPdu.ComTxIPduMinimumDelayFactor;
	} else {
		//DEBUG(DEBUG_MEDIUM, "failed (MDT)!\n", ComTxPduId);
	}
    Irq_Restore(state);
}

//lint -esym(904, Com_RxIndication) //PC-Lint Exception of rule 14.7
void Com_RxIndication(PduIdType RxPduId, PduInfoType* PduInfoPtr) {
	if( COM_INIT != Com_GetStatus() ) {
		DET_REPORTERROR(COM_RXINDICATION_ID, COM_E_UNINIT);
		return;
	}
	if( RxPduId >= ComConfig->ComNofIPdus ) {
//		DET_REPORTERROR(COM_RXINDICATION_ID, COM_E_PARAM);
		DET_REPORTERROR(COM_RXINDICATION_ID, COM_INVALID_PDU_ID);
		return;
	}

	const ComIPdu_type *IPdu = GET_IPdu(RxPduId);
	Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(RxPduId);
	imask_t state;
	Irq_Save(state);
	/* @req COM649 */ /* Interrups disabled */
	// If Ipdu is stopped
	/* @req COM684 */
	if (!Arc_IPdu->Com_Arc_IpduStarted) {
		Irq_Restore(state);
		return;
	}

	// Check callout status
	/* @req COM555 */
	/* @req COM700 */
	/* @req COM381 */
	/* @req COM780 */
	/* @req COM781 */
	if ((IPdu->ComRxIPduCallout != COM_NO_FUNCTION_CALLOUT) && (ComRxIPduCallouts[IPdu->ComRxIPduCallout] != NULL)) {
		if (!ComRxIPduCallouts[IPdu->ComRxIPduCallout](RxPduId, PduInfoPtr->SduDataPtr)) {
			// TODO Report error to DET.
			// Det_ReportError();
			/* !req COM738 */
			Irq_Restore(state);
			return;
		}
	}
	/* !req COM574 */
	/* !req COM575 */
	// Copy IPDU data
	memcpy(Arc_IPdu->ComIPduDataPtr, PduInfoPtr->SduDataPtr, IPdu->ComIPduSize);

	Com_RxProcessSignals(IPdu,Arc_IPdu);

	Irq_Restore(state);

	return;
}

void Com_TpRxIndication(PduIdType PduId, NotifResultType Result) {
	/* @req COM720 */

	if( COM_INIT != Com_GetStatus() ) {
		DET_REPORTERROR(COM_TPRXINDICATION_ID, COM_E_UNINIT);
		return;
	}
	if( PduId >= ComConfig->ComNofIPdus ) {
//		DET_REPORTERROR(COM_RXINDICATION_ID, COM_E_PARAM);
		DET_REPORTERROR(COM_TPRXINDICATION_ID, COM_INVALID_PDU_ID);
		return;
	}

	const ComIPdu_type *IPdu = GET_IPdu(PduId);
	Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(PduId);
	imask_t state;
	/* @req COM651 */ /* Interrups disabled */
	Irq_Save(state);

	// If Ipdu is stopped
	/* @req COM684 */
	if (!Arc_IPdu->Com_Arc_IpduStarted) {
		UnlockTpBuffer(getPduId(IPdu));
		Irq_Restore(state);
		return;
	}
	if (Result == NTFRSLT_OK) {
		if (IPdu->ComIPduSignalProcessing == IMMEDIATE) {
			// irqs needs to be disabled until signal notifications have been called
			// Otherwise a new Tp session can start and fill up pdus
			UnlockTpBuffer(getPduId(IPdu));
		}
		// In deferred mode, buffers are unlocked in mainfunction
		Com_RxProcessSignals(IPdu,Arc_IPdu);
	} else {
		UnlockTpBuffer(getPduId(IPdu));
	}
	Irq_Restore(state);

}

void Com_TpTxConfirmation(PduIdType PduId, NotifResultType Result) {
	/* !req COM713 */
	/* @req COM662 */ /* Tp buffer unlocked */
	if( COM_INIT != Com_GetStatus() ) {
		DET_REPORTERROR(COM_TPTXCONFIRMATION_ID, COM_E_UNINIT);
		return;
	}
	if( PduId >= ComConfig->ComNofIPdus ) {
//		DET_REPORTERROR(COM_RXINDICATION_ID, COM_E_PARAM);
		DET_REPORTERROR(COM_TPTXCONFIRMATION_ID, COM_INVALID_PDU_ID);
		return;
	}
	(void)Result; // touch

	imask_t state;
	Irq_Save(state);
	UnlockTpBuffer(PduId);
	Irq_Restore(state);
}

void Com_TxConfirmation(PduIdType TxPduId) {
	/* !req COM469 */
	/* !req COM053 */
	/* !req COM577 */
	/* !req COM305 */
	/* @req COM652 */ /* Function does nothing.. */
	if( COM_INIT != Com_GetStatus() ) {
		DET_REPORTERROR(COM_TXCONFIRMATION_ID, COM_E_UNINIT);
		return;
	}
	if( TxPduId >= ComConfig->ComNofIPdus ) {
//		DET_REPORTERROR(COM_RXINDICATION_ID, COM_E_PARAM);
		DET_REPORTERROR(COM_TXCONFIRMATION_ID, COM_INVALID_PDU_ID);
		return;
	}

    /* @req COM468 */
    /* Call all notifications for the PDU */
    const ComIPdu_type *IPdu = GET_IPdu(TxPduId);

    if (IPdu->ComIPduDirection == RECEIVE) {
        DET_REPORTERROR(COM_TXCONFIRMATION_ID, COM_INVALID_PDU_ID);
    }
    else if (IPdu->ComIPduSignalProcessing == IMMEDIATE) {
        /* If immediate, call the notification functions  */
        for (uint8 i = 0; IPdu->ComIPduSignalRef[i] != NULL; i++) {
            const ComSignal_type *signal = IPdu->ComIPduSignalRef[i];
            if ((signal->ComNotification != COM_NO_FUNCTION_CALLOUT) &&
                (ComNotificationCallouts[signal->ComNotification] != NULL) ) {
                ComNotificationCallouts[signal->ComNotification]();
            }
        }
    }
    else {
        /* If deferred, set status and let the main function call the notification function */
        SetTxConfirmationStatus(IPdu, TRUE);
    }
}


uint8 Com_SendSignalGroup(Com_SignalGroupIdType SignalGroupId) {
	/* Validate signalgroupid?*/
	/* @req COM334 */
	/* !req COM055 */
//#warning Com_SendSignalGroup should be performed atomically. Should we disable interrupts here?
	/* @req COM050 */
	uint8 ret = E_OK;
	boolean dataChanged = FALSE;
	if( COM_INIT != Com_GetStatus() ) {
		DET_REPORTERROR(COM_SENDSIGNALGROUP_ID, COM_E_UNINIT);
		return COM_SERVICE_NOT_AVAILABLE;
	}
	const ComSignal_type * Signal = GET_Signal(SignalGroupId);

    if (Signal->ComIPduHandleId == NO_PDU_REFERENCE) {
        return COM_SERVICE_NOT_AVAILABLE;
    }

    Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(Signal->ComIPduHandleId);
    const ComIPdu_type *IPdu = GET_IPdu(Signal->ComIPduHandleId);

    if (isPduBufferLocked(getPduId(IPdu))) {
        return COM_BUSY;
    }

    // Copy shadow buffer to Ipdu data space
    imask_t irq_state;

    Irq_Save(irq_state);
    /* @req COM635 */

    Com_CopySignalGroupDataFromShadowBufferToPdu(SignalGroupId,
                                                 (IPdu->ComIPduSignalProcessing == DEFERRED) &&
                                                 (IPdu->ComIPduDirection == RECEIVE),
                                                 &dataChanged);

    // If the signal has an update bit. Set it!
    /* @req COM061 */
    if (Signal->ComSignalArcUseUpdateBit) {
        SETBIT(Arc_IPdu->ComIPduDataPtr, Signal->ComUpdateBitPosition);
    }

    if( Arc_IPdu->Com_Arc_IpduStarted ) {
        // If signal has triggered transmit property, trigger a transmission!
        /* @req COM769 */
        /* @req COM742 */ /* TODO: Check this */
        /* !req COM743 */
        /* @req COM770 */
        if ( (TRIGGERED == Signal->ComTransferProperty) || ( TRIGGERED_WITHOUT_REPETITION == Signal->ComTransferProperty ) ||
                ( ((TRIGGERED_ON_CHANGE == Signal->ComTransferProperty) || ( TRIGGERED_ON_CHANGE_WITHOUT_REPETITION == Signal->ComTransferProperty )) && dataChanged)) {
            /* @req COM279 */
            /* @req COM741 */

            uint8 nofReps = 0;
            switch(Signal->ComTransferProperty) {
                case TRIGGERED:
                case TRIGGERED_ON_CHANGE:
                    if( 0 == IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeNumberOfRepetitions ) {
                        nofReps = 1;
                    } else {
                        nofReps = IPdu->ComTxIPdu.ComTxModeTrue.ComTxModeNumberOfRepetitions;
                    }
                    break;
                case TRIGGERED_WITHOUT_REPETITION:
                case TRIGGERED_ON_CHANGE_WITHOUT_REPETITION:
                    nofReps = 1;
                    break;
                default:
                    break;
            }
            /* Do not cancel outstanding repetitions triggered by other signalss */
            if( Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft < nofReps ) {
                Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft = nofReps;
            }
        }
    } else {
        ret = COM_SERVICE_NOT_AVAILABLE;
    }
    Irq_Restore(irq_state);

    return ret;
}


uint8 Com_ReceiveSignalGroup(Com_SignalGroupIdType SignalGroupId) {

	if( COM_INIT != Com_GetStatus() ) {
		DET_REPORTERROR(COM_RECEIVESIGNALGROUP_ID, COM_E_UNINIT);
		return COM_SERVICE_NOT_AVAILABLE;
	}

	const ComSignal_type * Signal = GET_Signal(SignalGroupId);

    if (Signal->ComIPduHandleId == NO_PDU_REFERENCE) {
        return COM_SERVICE_NOT_AVAILABLE;
    }

    const ComIPdu_type *IPdu = GET_IPdu(Signal->ComIPduHandleId);

    if (isPduBufferLocked(getPduId(IPdu))) {
        return COM_BUSY;
    }
    // Copy Ipdu data buffer to shadow buffer.
    /* @req COM638 */
    /* @req COM461 */
    Com_CopySignalGroupDataFromPduToShadowBuffer(SignalGroupId);

    if( !GET_ArcIPdu(Signal->ComIPduHandleId)->Com_Arc_IpduStarted ) {
        return COM_SERVICE_NOT_AVAILABLE;
    } else {
        return E_OK;
    }
}

void Com_UpdateShadowSignal(Com_SignalIdType SignalId, const void *SignalDataPtr) {

	if( COM_INIT != Com_GetStatus() ) {
		DET_REPORTERROR(COM_UPDATESHADOWSIGNAL_ID, COM_E_UNINIT);
		return;
	}

    if( SignalId >= ComConfig->ComNofGroupSignals ) {
        DET_REPORTERROR(COM_UPDATESHADOWSIGNAL_ID, COM_E_PARAM);
        return;
    }

    Com_Arc_GroupSignal_type *Arc_GroupSignal = GET_ArcGroupSignal(SignalId);

    if (Arc_GroupSignal->Com_Arc_ShadowBuffer != NULL) {
        Com_Internal_UpdateShadowSignal(SignalId, SignalDataPtr);
    }
}

void Com_ReceiveShadowSignal(Com_SignalIdType SignalId, void *SignalDataPtr) {

	if( COM_INIT != Com_GetStatus() ) {
		DET_REPORTERROR(COM_RECEIVESHADOWSIGNAL_ID, COM_E_UNINIT);
		return;
	}

    if( SignalId >= ComConfig->ComNofGroupSignals ) {
        DET_REPORTERROR(COM_RECEIVESHADOWSIGNAL_ID, COM_E_PARAM);
        return;
    }

    const ComGroupSignal_type *GroupSignal = GET_GroupSignal(SignalId);
	Com_Arc_GroupSignal_type *Arc_GroupSignal = GET_ArcGroupSignal(SignalId);

    /* Get default value if unconnected */
    if (Arc_GroupSignal->Com_Arc_ShadowBuffer == NULL) {
        memcpy(SignalDataPtr, GroupSignal->ComSignalInitValue, (GroupSignal->ComBitSize/8) + 1);
    }
    else {
	/* @req COM640 */
	Com_ReadSignalDataFromPduBuffer(SignalId, TRUE, SignalDataPtr, (void *)Arc_GroupSignal->Com_Arc_ShadowBuffer);
    }

}



