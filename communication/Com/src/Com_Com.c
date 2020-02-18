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

#include <string.h>
#include "PduR.h"
#include "Com_Arc_Types.h"
#include "Com.h"
#include "Com_Internal.h"
#include "Com_misc.h"
#include "debug.h"
#include "Det.h"
#include "Cpu.h"
#include "SchM_Com.h"
#include "arc_assert.h"

#if defined(USE_LINUXOS)
#include "linos_logger.h" /* Logger functions */
#endif

/* Declared in Com_Cfg.c */
/*lint -esym(9003, ComRxIPduCallouts)*/ /* ComRxIPduCallouts is defined in com_cfg.c */
extern const ComRxIPduCalloutType ComRxIPduCallouts[];
/*lint -e9003 ComNotificationCallouts cannot be defined at block scope because it is used in Com_RxIndication  and Com_TxConfirmation */
extern const ComNotificationCalloutType ComNotificationCallouts[];
/*lint -esym(9003, ComTriggerTransmitIPduCallouts)*/ /* ComTriggerTransmitIPduCallouts is defined in com_cfg.c */
extern const ComTxIPduCalloutType ComTriggerTransmitIPduCallouts[];



uint8 Com_SendSignal(Com_SignalIdType SignalId, const void *SignalDataPtr) {
    /* @req COM334 */ /* Shall update buffer if pdu stopped, should not store trigger */
    uint8 ret = E_OK;
    boolean dataChanged = FALSE;
    if( COM_INIT != Com_GetStatus() ) {
        DET_REPORTERROR(COM_SENDSIGNAL_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return COM_SERVICE_NOT_AVAILABLE;
    }

    if(SignalId >= ComConfig->ComNofSignals) {
        DET_REPORTERROR(COM_SENDSIGNAL_ID, COM_E_PARAM);
        /*lint -e{904} ARGUMENT CHECK */
        return COM_SERVICE_NOT_AVAILABLE;
    }

    /* Store pointer to signal for easier coding.*/
    const ComSignal_type * Signal = GET_Signal(SignalId);
#if defined(USE_LINUXOS)
    logger(LOG_INFO, "Com_SendSignal SignalDataPtr [%s] ComBitSize [%d]", 
            logger_format_hex((char*)SignalDataPtr, (Signal->ComBitSize / 8)),
            Signal->ComBitSize
    );
#endif

    if (Signal->ComIPduHandleId == NO_PDU_REFERENCE) {
        /* Return error if signal is not connected to an IPdu*/
        ret = COM_SERVICE_NOT_AVAILABLE;
    } else if (TRUE == isPduBufferLocked(Signal->ComIPduHandleId)) {
        ret = COM_BUSY;
    } else {

    	if (Signal->ComBitSize != 0) {
            //DEBUG(DEBUG_LOW, "Com_SendSignal: id %d, nBytes %d, BitPosition %d, intVal %d\n", SignalId, nBytes, signal->ComBitPosition, (uint32)*(uint8 *)SignalDataPtr);
            const Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(Signal->ComIPduHandleId);

            /* @req COM624 */
            uint8 *comIPduDataPtr = Arc_IPdu->ComIPduDataPtr;
            SchM_Enter_Com_EA_0();
            Com_Misc_WriteSignalDataToPdu(
                    (const uint8 *)SignalDataPtr,
                    Signal->ComSignalType,
                    comIPduDataPtr,
                    Signal->ComBitPosition,
                    Signal->ComBitSize,
                    Signal->ComSignalEndianess,
                    &dataChanged);
            /* If the signal has an update bit. Set it!*/
            /* @req COM061 */
            if (TRUE == Signal->ComSignalArcUseUpdateBit) {
                /*lint -e{926} pointer cast is essential since SETBIT parameters are of different pointer data type*/
                /*lint -e{9016} Array indexing couldn't be implemented, as parameters are of different data types */
                SETBIT(comIPduDataPtr, Signal->ComUpdateBitPosition);
            }
    	}

        /* Assign the number of repetitions based on Transfer property */
        if( FALSE == Com_Misc_TriggerTxOnConditions(Signal->ComIPduHandleId, dataChanged, Signal->ComTransferProperty) ) {
            ret = COM_SERVICE_NOT_AVAILABLE;
        }
        SchM_Exit_Com_EA_0();
    }
    return ret;
}

uint8 Com_ReceiveSignal(Com_SignalIdType SignalId, void* SignalDataPtr) {

    uint8 ret;
    ret = E_OK;
    if( COM_INIT != Com_GetStatus() ) {
        DET_REPORTERROR(COM_RECEIVESIGNAL_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return COM_SERVICE_NOT_AVAILABLE;
    }
    if(SignalId >= ComConfig->ComNofSignals) {
        DET_REPORTERROR(COM_RECEIVESIGNAL_ID, COM_E_PARAM);
        /*lint -e{904} ARGUMENT CHECK */
        return COM_SERVICE_NOT_AVAILABLE;
    }
    DEBUG(DEBUG_LOW, "Com_ReceiveSignal: SignalId %d\n", SignalId);
    const ComSignal_type * Signal = GET_Signal(SignalId);

    if (Signal->ComIPduHandleId == NO_PDU_REFERENCE) {
        /* Return error init value signal if signal is not connected to an IPdu*/
        memcpy(SignalDataPtr, Signal->ComSignalInitValue, SignalTypeToSize(Signal->ComSignalType, Signal->ComBitSize/8));
        ret = E_OK;
    } else {
        const ComIPdu_type *IPdu = GET_IPdu(Signal->ComIPduHandleId);
        const Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(Signal->ComIPduHandleId);

        const void* pduBuffer = NULL;
        if ((IPdu->ComIPduSignalProcessing == COM_DEFERRED) && (IPdu->ComIPduDirection == COM_RECEIVE)) {
            pduBuffer = Arc_IPdu->ComIPduDeferredDataPtr;
        } else {
            if (TRUE == isPduBufferLocked(Signal->ComIPduHandleId)) {
                ret = COM_BUSY;
            }
            pduBuffer = Arc_IPdu->ComIPduDataPtr;
        }
        /* @req COM631 */
        Com_Misc_ReadSignalDataFromPdu(
                pduBuffer,
                Signal->ComBitPosition,
                Signal->ComBitSize,
                Signal->ComSignalEndianess,
                Signal->ComSignalType,
                SignalDataPtr);
        if( (FALSE == Arc_IPdu->Com_Arc_IpduStarted) && (E_OK == ret) ) {
            ret = COM_SERVICE_NOT_AVAILABLE;
        }
    }
    return ret;
}

uint8 Com_ReceiveDynSignal(Com_SignalIdType SignalId, void* SignalDataPtr, uint16* Length) {
    /* IMPROVEMENT: Validate signal id?*/

    uint8 ret;
    ret = E_OK;
    if( COM_INIT != Com_GetStatus() ) {
        DET_REPORTERROR(COM_RECEIVEDYNSIGNAL_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return COM_SERVICE_NOT_AVAILABLE;
    }
    const ComSignal_type * Signal = GET_Signal(SignalId);
    const Com_Arc_IPdu_type    *Arc_IPdu   = GET_ArcIPdu(Signal->ComIPduHandleId);
    const ComIPdu_type   *IPdu       = GET_IPdu(Signal->ComIPduHandleId);

    Com_SignalType signalType = Signal->ComSignalType;
    /* @req COM753 */
    if (signalType != COM_UINT8_DYN) {
        ret = COM_SERVICE_NOT_AVAILABLE;
    } else {
        SchM_Enter_Com_EA_0();
        /* @req COM712 */
        if( *Length >= Arc_IPdu->Com_Arc_DynSignalLength ) {
            if (*Length > Arc_IPdu->Com_Arc_DynSignalLength) {
                *Length = Arc_IPdu->Com_Arc_DynSignalLength;
            }
            uint16 startFromPduByte = (Signal->ComBitPosition) / 8;
            const uint8* pduDataPtr = NULL;
            if ((IPdu->ComIPduSignalProcessing == COM_DEFERRED) && (IPdu->ComIPduDirection == COM_RECEIVE)) {
                pduDataPtr = (uint8 *)Arc_IPdu->ComIPduDeferredDataPtr;
            } else {
                if (TRUE == isPduBufferLocked(getPduId(IPdu))) {
                    ret = COM_BUSY;
                }
                pduDataPtr = (uint8 *)Arc_IPdu->ComIPduDataPtr;
            }
            /* @req COM711 */

            memcpy(SignalDataPtr, (&pduDataPtr[startFromPduByte]), *Length);

        } else {
            /* @req COM724 */
            *Length = Arc_IPdu->Com_Arc_DynSignalLength;
            ret = E_NOT_OK;
        }
        SchM_Exit_Com_EA_0();
        if( (FALSE == Arc_IPdu->Com_Arc_IpduStarted) && (E_OK == ret) ) {
            ret = COM_SERVICE_NOT_AVAILABLE;
        }
    }
    return ret;
}


/**
 * Send a dynamic signal (ie ComSignalType=COM_UINT8_DYN)
 *
 * @param SignalId			- The signal ID
 * @param SignalDataPtr		- Pointer to the data.
 * @param Length			- The signal length in bytes
 * @return
 */
uint8 Com_SendDynSignal(Com_SignalIdType SignalId, const void* SignalDataPtr, uint16 Length) {
    /* IMPROVEMENT: validate SignalId */
    /* !req COM629 */
    /* @req COM630 */

    if( COM_INIT != Com_GetStatus() ) {
        DET_REPORTERROR(COM_SENDDYNSIGNAL_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return COM_SERVICE_NOT_AVAILABLE;
    }
    const ComSignal_type * Signal = GET_Signal(SignalId);
    Com_Arc_IPdu_type    *Arc_IPdu   = GET_ArcIPdu(Signal->ComIPduHandleId);
    const ComIPdu_type   *IPdu       = GET_IPdu(Signal->ComIPduHandleId);
    uint8 ret = E_OK;
    Com_SignalType signalType = Signal->ComSignalType;
    boolean dataChanged = FALSE;
    const uint8* Arc_IpduDataPtr;
    uint8* Arc_IpduDataPtr1;

    /* @req COM753 */
    if (signalType != COM_UINT8_DYN) {

        ret = COM_SERVICE_NOT_AVAILABLE;
    } else if (TRUE == isPduBufferLocked(getPduId(IPdu))) {

        ret = COM_BUSY;
    } else {

        /* Get signalLength in bytes,
         * Note!, ComBitSize is set to ComSignalLength in case of COM_UINT8_DYN or COM_UINT8_N */
        uint32 signalLength = Signal->ComBitSize / 8;
        Com_BitPositionType bitPosition = Signal->ComBitPosition;

        /* Check so that we are not trying to send a signal that is
         * bigger that the actual space */
        if (signalLength < Length) {

            ret = E_NOT_OK;
        } else {
            uint16 startFromPduByte = bitPosition / 8;

            SchM_Enter_Com_EA_0();
            Arc_IpduDataPtr = (uint8 *)Arc_IPdu->ComIPduDataPtr;
            /*lint -e{9007} Either one of the condition yielding true is sufficient */
            if( (Arc_IPdu->Com_Arc_DynSignalLength != Length) ||
                    (0 != memcmp((&Arc_IpduDataPtr[startFromPduByte]), SignalDataPtr, Length)) ) {
                dataChanged = TRUE;
            }
            /* @req COM628 */
            Arc_IpduDataPtr1 = (uint8 *)Arc_IPdu->ComIPduDataPtr;
            memcpy((&Arc_IpduDataPtr1[startFromPduByte]), SignalDataPtr, Length);
            /* !req COM757 */ /*Length of I-PDU?*/
            Arc_IPdu->Com_Arc_DynSignalLength = Length;
            /* If the signal has an update bit. Set it!*/
            if (TRUE == Signal->ComSignalArcUseUpdateBit) {
                /*lint -e{9016} Array indexing couldn't be implemented, as parameters are of different data types */
                SETBIT(Arc_IPdu->ComIPduDataPtr, Signal->ComUpdateBitPosition);
            }

            /* Assign the number of repetitions based on Transfer property */
            if(FALSE == Com_Misc_TriggerTxOnConditions(Signal->ComIPduHandleId, dataChanged, Signal->ComTransferProperty) ) {
                ret = COM_SERVICE_NOT_AVAILABLE;
            }
            SchM_Exit_Com_EA_0();

        }
    }
    return ret;
}

Std_ReturnType Com_TriggerTransmit(PduIdType TxPduId, PduInfoType *PduInfoPtr) {

    Std_ReturnType status;
    status = E_OK;
    /* @req COM475 */
    if( COM_INIT != Com_GetStatus() ) {
        DET_REPORTERROR(COM_TRIGGERTRANSMIT_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return E_NOT_OK;
    }
    if( TxPduId >= ComConfig->ComNofIPdus ) {
        DET_REPORTERROR(COM_TRIGGERTRANSMIT_ID, COM_E_PARAM);
        /*lint -e{904} ARGUMENT CHECK */
        return E_NOT_OK;
    }
    /*
     * !req COM260: This function must not check the transmission mode of the I-PDU
     * since it should be possible to use it regardless of the transmission mode.
     * Only for ComIPduType = NORMAL.
     */

    const ComIPdu_type *IPdu = GET_IPdu(TxPduId);
    const Com_Arc_IPdu_type    *Arc_IPdu   = GET_ArcIPdu(TxPduId);

    SchM_Enter_Com_EA_0();

    /* @req COM766 */
    if( (IPdu->ComTriggerTransmitIPduCallout != COM_NO_FUNCTION_CALLOUT) && (ComTriggerTransmitIPduCallouts[IPdu->ComTriggerTransmitIPduCallout] != NULL) ) {
        /* @req COM395 */
        (void)ComTriggerTransmitIPduCallouts[IPdu->ComTriggerTransmitIPduCallout](TxPduId, Arc_IPdu->ComIPduDataPtr);
    }

    /* @req COM647 */
    /* @req COM648 */ /* Interrups disabled */
    memcpy(PduInfoPtr->SduDataPtr, Arc_IPdu->ComIPduDataPtr, IPdu->ComIPduSize);

    PduInfoPtr->SduLength = IPdu->ComIPduSize;
    if( FALSE == Arc_IPdu->Com_Arc_IpduStarted ) {
        status = E_NOT_OK;
    } else {

        if (IPdu->ComTxIPdu.ComTxIPduClearUpdateBit == TRIGGERTRANSMIT) {
            /* Clear all update bits for the contained signals*/
            /* @req COM578 */
            for (uint16 i = 0;(IPdu->ComIPduSignalRef != NULL)&& (IPdu->ComIPduSignalRef[i] != NULL); i++) {
                if (TRUE == IPdu->ComIPduSignalRef[i]->ComSignalArcUseUpdateBit) {
                    /*lint -e{9016} Array indexing couldn't be implemented, as parameters are of different data types */
                    CLEARBIT(Arc_IPdu->ComIPduDataPtr,IPdu->ComIPduSignalRef[i]->ComUpdateBitPosition);
                }
            }
        }

        status = E_OK;
    }

    SchM_Exit_Com_EA_0();

    return status;
}

void Com_TriggerIPDUSend(PduIdType PduId) {
    /* !req	COM789 */
    /* !req	COM662 */ /* May be supported, but when is buffer locked?*/

    if( COM_INIT != Com_GetStatus() ) {
        DET_REPORTERROR(COM_TRIGGERIPDUSEND_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }
    if( PduId >= ComConfig->ComNofIPdus ) {
        DET_REPORTERROR(COM_TRIGGERIPDUSEND_ID, COM_E_PARAM);
        /*lint -e{904} ARGUMENT CHECK */
        return;
    }

    (void)Com_Misc_TriggerIPDUSend(PduId);

}
/*lint --e{818} 'PduInfoPtr' could be declared as pointing to const, this is done in 4.2.2 */
void Com_RxIndication(PduIdType RxPduId, PduInfoType* PduInfoPtr) {

    boolean status;
    status = TRUE;
    if( COM_INIT != Com_GetStatus() ) {
        DET_REPORTERROR(COM_RXINDICATION_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }
    if( RxPduId >= ComConfig->ComNofIPdus ) {
        DET_REPORTERROR(COM_RXINDICATION_ID, COM_E_PARAM);
        /*lint -e{904} ARGUMENT CHECK */
        return;
    }

    const ComIPdu_type *IPdu = GET_IPdu(RxPduId);
    const Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(RxPduId);
    SchM_Enter_Com_EA_0();


    /* @req COM649 */ /* Interrups disabled */
    /* If Ipdu is stopped*/
    /* @req COM684 */
    if (FALSE == Arc_IPdu->Com_Arc_IpduStarted) {

    } else {
        /* Check callout status*/
        /* @req COM555 */
        /* @req COM700 */
        if ((IPdu->ComRxIPduCallout != COM_NO_FUNCTION_CALLOUT) && (ComRxIPduCallouts[IPdu->ComRxIPduCallout] != NULL)) {
            if (FALSE == ComRxIPduCallouts[IPdu->ComRxIPduCallout](RxPduId, PduInfoPtr->SduDataPtr)) {
                // IMPROVMENT: Report error to DET.
                // Det_ReportError();
                /* !req COM738 */
                status = FALSE;
            }
        }
        /* !req COM574 */
        /* !req COM575 */
        if (status == TRUE) {
#if(COM_IPDU_COUNTING_ENABLE ==  STD_ON )
            if (ComConfig->ComIPdu[RxPduId].ComIpduCounterRef != NULL) {
                /* Ignore the IPdu if errors detected*/
                if (FALSE == (Com_Misc_validateIPduCounter(ComConfig->ComIPdu[RxPduId].ComIpduCounterRef,PduInfoPtr->SduDataPtr))){
                    /* @req COM726 *//* @req COM727 */
                    if (ComConfig->ComIPdu[RxPduId].ComIpduCounterRef->ComIPduCntErrNotification != COM_NO_FUNCTION_CALLOUT) {
                        ComNotificationCallouts[ComConfig->ComIPdu[RxPduId].ComIpduCounterRef->ComIPduCntErrNotification](); /*Trigger error callback*/
                    }

                    /* @req COM590 */
                    status = FALSE;
                }
            }
#endif
            /* Copy IPDU data*/
            if (status == TRUE) { /*lint !e774   status value can be update based on configuration    */
                memcpy(Arc_IPdu->ComIPduDataPtr, PduInfoPtr->SduDataPtr, IPdu->ComIPduSize);
                Com_Misc_RxProcessSignals(IPdu,Arc_IPdu);
            }
        }
	}
    SchM_Exit_Com_EA_0();
	return;
}

void Com_TpRxIndication(PduIdType PduId, NotifResultType Result) {
    /* @req COM720 */

    if( COM_INIT != Com_GetStatus() ) {
        DET_REPORTERROR(COM_TPRXINDICATION_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }
    if( PduId >= ComConfig->ComNofIPdus ) {
        DET_REPORTERROR(COM_RXINDICATION_ID, COM_E_PARAM);
        /*lint -e{904} ARGUMENT CHECK */
        return;
    }

    const ComIPdu_type *IPdu = GET_IPdu(PduId);
    const Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(PduId);
    /* @req COM651 */ /* Interrups disabled */
    SchM_Enter_Com_EA_0();

    /* If Ipdu is stopped */
    /* @req COM684 */
    if (FALSE == Arc_IPdu->Com_Arc_IpduStarted) {
        Com_Misc_UnlockTpBuffer(getPduId(IPdu));
        SchM_Exit_Com_EA_0();
    } else {
        if (Result == NTFRSLT_OK) {
            if (IPdu->ComIPduSignalProcessing == COM_IMMEDIATE) {
                /* irqs needs to be disabled until signal notifications have been called*/
                /* Otherwise a new Tp session can start and fill up pdus*/
                Com_Misc_UnlockTpBuffer(getPduId(IPdu));
            }
            /* In deferred mode, buffers are unlocked in mainfunction*/
            Com_Misc_RxProcessSignals(IPdu,Arc_IPdu);
        } else {
            Com_Misc_UnlockTpBuffer(getPduId(IPdu));
        }
        SchM_Exit_Com_EA_0();
    }
}

void Com_TpTxConfirmation(PduIdType PduId, NotifResultType Result) {
    /* !req COM713 */
    /* !req COM662 */ /* Tp buffer unlocked but buffer is never locked */
    if( COM_INIT != Com_GetStatus() ) {
        DET_REPORTERROR(COM_TPTXCONFIRMATION_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }
    if( PduId >= ComConfig->ComNofIPdus ) {
        DET_REPORTERROR(COM_RXINDICATION_ID, COM_E_PARAM);
        /*lint -e{904} ARGUMENT CHECK */
        return;
    }
    (void)Result; /* touch*/

    SchM_Enter_Com_EA_0();
    Com_Misc_UnlockTpBuffer(PduId);
    SchM_Exit_Com_EA_0();
}

void Com_TxConfirmation(PduIdType TxPduId) {
    /* !req COM469 */
    /* !req COM053 */
    /* @req COM652 */ /* Function does nothing.. */

    /* Need to implement COM305 */

    if( COM_INIT != Com_GetStatus() ) {
        DET_REPORTERROR(COM_TXCONFIRMATION_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }
    if( TxPduId >= ComConfig->ComNofIPdus ) {
        DET_REPORTERROR(COM_RXINDICATION_ID, COM_E_PARAM);
        /*lint -e{904} ARGUMENT CHECK */
        return;
    }

    /* @req COM468 */
    /* Call all notifications for the PDU */
    const ComIPdu_type *IPdu = GET_IPdu(TxPduId);
    Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(TxPduId);

    if (IPdu->ComIPduDirection == COM_RECEIVE) {
        DET_REPORTERROR(COM_TXCONFIRMATION_ID, COM_INVALID_PDU_ID);
    }
    else {
        /* take care DM handling */
        if(0 < Arc_IPdu->Com_Arc_TxDeadlineCounter){ /* if DM configured */
            Com_Misc_TxHandleDM(IPdu,Arc_IPdu);
            /* IMPROVEMENT - handle also in Com_TpTxConfirmation */
        }

        /* Clear all update bits for the contained signals*/
        /* @req COM577 */
        if (IPdu->ComTxIPdu.ComTxIPduClearUpdateBit == CONFIRMATION) {
            for (uint16 i = 0; (IPdu->ComIPduSignalRef != NULL) && (IPdu->ComIPduSignalRef[i] != NULL); i++) {
                if (TRUE == IPdu->ComIPduSignalRef[i]->ComSignalArcUseUpdateBit) {
                    /*lint -e{9016} Array indexing couldn't be implemented, as parameters are of different data types */
                    CLEARBIT(Arc_IPdu->ComIPduDataPtr, IPdu->ComIPduSignalRef[i]->ComUpdateBitPosition);
                }
            }
        }

        if (IPdu->ComIPduSignalProcessing == COM_IMMEDIATE) {
        /* If immediate, call the notification functions  */
        for (uint16 i = 0; IPdu->ComIPduSignalRef[i] != NULL; i++) {
            const ComSignal_type *signal = IPdu->ComIPduSignalRef[i];
            if ((signal->ComNotification != COM_NO_FUNCTION_CALLOUT) &&
                (ComNotificationCallouts[signal->ComNotification] != NULL) ) {
                ComNotificationCallouts[signal->ComNotification]();
            }
          }
        }
        else {
            /* If deferred, set status and let the main function call the notification function */
            Com_Misc_SetTxConfirmationStatus(IPdu, TRUE);
        }    	
     }
}


uint8 Com_SendSignalGroup(Com_SignalGroupIdType SignalGroupId) {
    /* Validate signalgroupid?*/
    /* @req COM334 */
    uint8 ret = E_OK;
    boolean dataChanged = FALSE;
    if( COM_INIT != Com_GetStatus() ) {
        DET_REPORTERROR(COM_SENDSIGNALGROUP_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return COM_SERVICE_NOT_AVAILABLE;
    }
    const ComSignal_type * Signal = GET_Signal(SignalGroupId);

    if (Signal->ComIPduHandleId == NO_PDU_REFERENCE) {
        ret = COM_SERVICE_NOT_AVAILABLE;
    } else {
        const Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(Signal->ComIPduHandleId);
        const ComIPdu_type *IPdu = GET_IPdu(Signal->ComIPduHandleId);

        if (TRUE == isPduBufferLocked(getPduId(IPdu))) {
            ret = COM_BUSY;
        } else {
            /* Copy shadow buffer to Ipdu data space*/

            SchM_Enter_Com_EA_0();
            /* @req COM635 */
            /* @req COM050 */
            Com_Misc_CopySignalGroupDataFromShadowBufferToPdu(SignalGroupId,
                                                         (((IPdu->ComIPduSignalProcessing == COM_DEFERRED) &&
                                                         (IPdu->ComIPduDirection == COM_RECEIVE))? TRUE : FALSE),
                                                         &dataChanged);

            /* If the signal has an update bit. Set it!*/
            /* @req COM061 */
            if (TRUE== Signal->ComSignalArcUseUpdateBit) {
                /*lint -e{9016} Array indexing couldn't be implemented, as parameters are of different data types */
                SETBIT(Arc_IPdu->ComIPduDataPtr, Signal->ComUpdateBitPosition);
            }
            /* Assign the number of repetitions based on Transfer property */
            if( FALSE == Com_Misc_TriggerTxOnConditions(Signal->ComIPduHandleId, dataChanged, Signal->ComTransferProperty) ) {
                ret = COM_SERVICE_NOT_AVAILABLE;
            }
            SchM_Exit_Com_EA_0();
        }
    }
    return ret;
}


uint8 Com_ReceiveSignalGroup(Com_SignalGroupIdType SignalGroupId) {

    uint8 status;
    status = E_NOT_OK;
    if( COM_INIT != Com_GetStatus() ) {
        DET_REPORTERROR(COM_RECEIVESIGNALGROUP_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return COM_SERVICE_NOT_AVAILABLE;
    }

    const ComSignal_type * Signal = GET_Signal(SignalGroupId);

    if (Signal->ComIPduHandleId == NO_PDU_REFERENCE) {
        status = COM_SERVICE_NOT_AVAILABLE;
    } else {
        const ComIPdu_type *IPdu = GET_IPdu(Signal->ComIPduHandleId);

        if (TRUE == isPduBufferLocked(getPduId(IPdu))) {
            status = COM_BUSY;
        } else {
            /* Copy Ipdu data buffer to shadow buffer.*/
            /* @req COM638 */
            /* @req COM461 */
            /* @req COM051 */
        	SchM_Enter_Com_EA_0();
            Com_Misc_CopySignalGroupDataFromPduToShadowBuffer(SignalGroupId);
            SchM_Exit_Com_EA_0();

            if( FALSE == GET_ArcIPdu(Signal->ComIPduHandleId)->Com_Arc_IpduStarted ) {
                status = COM_SERVICE_NOT_AVAILABLE;
            } else {
                status = E_OK;
            }
        }
    }
    return status;
}

void Com_UpdateShadowSignal(Com_SignalIdType SignalId, const void *SignalDataPtr) {

    if( COM_INIT != Com_GetStatus() ) {
        DET_REPORTERROR(COM_UPDATESHADOWSIGNAL_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }

    if( SignalId >= ComConfig->ComNofGroupSignals ) {
        DET_REPORTERROR(COM_UPDATESHADOWSIGNAL_ID, COM_E_PARAM);
        /*lint -e{904} ARGUMENT CHECK */
        return;
    }

    const Com_Arc_GroupSignal_type *Arc_GroupSignal = GET_ArcGroupSignal(SignalId);

    if (Arc_GroupSignal->Com_Arc_ShadowBuffer != NULL) {
        Com_Misc_UpdateShadowSignal(SignalId, SignalDataPtr);
    }
}

void Com_ReceiveShadowSignal(Com_SignalIdType SignalId, void *SignalDataPtr) {

    if( COM_INIT != Com_GetStatus() ) {
        DET_REPORTERROR(COM_RECEIVESHADOWSIGNAL_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }

    if( SignalId >= ComConfig->ComNofGroupSignals ) {
        DET_REPORTERROR(COM_RECEIVESHADOWSIGNAL_ID, COM_E_PARAM);
        /*lint -e{904} ARGUMENT CHECK */
        return;
    }

    const ComGroupSignal_type *GroupSignal = GET_GroupSignal(SignalId);
    const Com_Arc_GroupSignal_type *Arc_GroupSignal = GET_ArcGroupSignal(SignalId);

    /* Get default value if unconnected */
    if (Arc_GroupSignal->Com_Arc_ShadowBuffer == NULL) {
        memcpy(SignalDataPtr, GroupSignal->ComSignalInitValue, SignalTypeToSize(GroupSignal->ComSignalType, (GroupSignal->ComBitSize/8)));
    }
    else {
    /* @req COM640 */
        Com_Misc_ReadSignalDataFromPdu(
                Arc_GroupSignal->Com_Arc_ShadowBuffer,
                GroupSignal->ComBitPosition,
                GroupSignal->ComBitSize,
                GroupSignal->ComSignalEndianess,
                GroupSignal->ComSignalType,
                SignalDataPtr);
    }

}



