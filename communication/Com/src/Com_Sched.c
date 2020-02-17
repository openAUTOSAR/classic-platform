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
#include "SchM_Com.h"
#if (COM_OSEKNM_SUPPORT == STD_ON)
#include "OsekNm.h"
#endif

#define timerDec(timer) \
    if (timer > 0) { \
        timer = timer - 1; \
    } \

/* Declared in Com_Cfg.c */
extern const ComNotificationCalloutType ComNotificationCallouts[];

/**
 * Tx sub main function- routed from Com_MainFunctionTx
 * Processes mixed or periodic tx pdus
 * handles the Tx timing functionalities/timers as well
 * @param IPduId
 * @param dmTimeOut
 * @return none
 */
static void Com_ProcessMixedOrPeriodicTxMode(uint16 IPduId,boolean dmTimeOut){
    const ComIPdu_type *IPdu;
    IPdu = &ComConfig->ComIPdu[IPduId];
    Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(IPduId);
    const ComTxMode_type *txModePtr;
    if( TRUE == Arc_IPdu->Com_Arc_IpduTxMode ) {
        txModePtr = &IPdu->ComTxIPdu.ComTxModeTrue;
    }
    else {
        txModePtr = &IPdu->ComTxIPdu.ComTxModeFalse;
    }
    timerDec(Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeTimePeriodTimer);
    /* Is it time for a direct transmission?*/
    if ( (txModePtr->ComTxModeMode == COM_MIXED)
        && (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft > 0) ) {

        timerDec(Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeRepetitionPeriodTimer);

        if(FALSE == dmTimeOut) {
            /* @req COM305.2 */
            /* Is it time for a transmission?*/
            if ( (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeRepetitionPeriodTimer == 0)
                && (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduMinimumDelayTimer == 0) ) {

                Com_TriggerIPDUSend(IPduId);

                /* Reset periodic timer*/
                Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeRepetitionPeriodTimer = txModePtr->ComTxModeRepetitionPeriodFactor;

                /* Register this nth-transmission.*/
                /* @req COM494 */
                Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft--;
            }

        } else {

            /* @req COM392 */
            /* @req COM305.2 */
            /* Cancel outstanding N time transmission due to DM timeout */
            Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft = 0;
            /* Cancel the wait for outstanding Tx confirmations as well */
            Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfTxConfirmations = 0;

        }
    }

    /* Is it time for a cyclic transmission?*/
    if ( (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeTimePeriodTimer == 0) && (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduMinimumDelayTimer == 0) ) {

        Com_TriggerIPDUSend(IPduId);

        /* Reset periodic timer.*/
        Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeTimePeriodTimer = txModePtr->ComTxModeTimePeriodFactor;

        /* start/reset the DM timer for cyclic part (Mixed or periodic) only if not running or cancelled  */
        if((0 < Arc_IPdu->Com_Arc_TxDeadlineCounter) &&
           (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxDMTimer == 0)){
            Arc_IPdu->Com_Arc_TxIPduTimers.ComTxDMTimer = Arc_IPdu->Com_Arc_TxDeadlineCounter;
        }

    }
}

/**
 * Tx sub main function- routed from Com_MainFunctionTx
 * Processes direct tx pdus
 * handles the Tx timing functionalities/timers as well
 * @param IPduId
 * @param dmTimeOut
 * @return none
 */
static void Com_ProcessDirectTxMode(uint16 IPduId,boolean dmTimeOut){
    const ComIPdu_type *IPdu;
    IPdu = &ComConfig->ComIPdu[IPduId];
    Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(IPduId);
    const ComTxMode_type *txModePtr;
    if( TRUE == Arc_IPdu->Com_Arc_IpduTxMode ) {
        txModePtr = &IPdu->ComTxIPdu.ComTxModeTrue;
    }
    else {
        txModePtr = &IPdu->ComTxIPdu.ComTxModeFalse;
    }
    /* Do we need to transmit anything?*/
    if (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft > 0) {
        timerDec(Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeRepetitionPeriodTimer);

        if(FALSE == dmTimeOut) {
            /* @req COM305.2 */
            /* Is it time for a transmission?*/
            if ( (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeRepetitionPeriodTimer == 0) && (Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduMinimumDelayTimer == 0) ) {
                Com_TriggerIPDUSend(IPduId);

                /* Reset periodic timer*/
                Arc_IPdu->Com_Arc_TxIPduTimers.ComTxModeRepetitionPeriodTimer = txModePtr->ComTxModeRepetitionPeriodFactor;

                /* Register this nth-transmission.*/
                Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft--;

            }
        }
        else {
            /* @req COM392 */
            /* Cancel outstanding N time transmission due to DM timeout */
            Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfRepetitionsLeft = 0;
            /* Cancel the wait for outstanding Tx confirmations as well */
            Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduNumberOfTxConfirmations = 0;
        }
    }
}

/**
 * Rx Main function- called from the task context
 * Processes the incoming signals or signal groups,
 * updates responsibles of their signals
 * handles the rx timing functionalities/timers
 * @param void
 * @return none
 */
void Com_MainFunctionRx(void) {
    /* !req COM513 */
    /* !req COM053 */
    /* !req COM352 */
    /* @req COM664 */
    if( COM_INIT != Com_GetStatus() ) {
        DET_REPORTERROR(COM_MAINFUNCTIONRX_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }
    for (uint16 pduId = 0; 0 == ComConfig->ComIPdu[pduId].Com_Arc_EOL; pduId++) {
        const ComIPdu_type *IPdu = GET_IPdu(pduId);
        const Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(pduId);
        if((IPdu->ComIPduDirection == COM_RECEIVE) && (TRUE == Arc_IPdu->Com_Arc_IpduStarted)){
            boolean pduUpdated = false;
            SchM_Enter_Com_EA_0();
            for (uint16 sri = 0; (IPdu->ComIPduSignalRef != NULL) && (IPdu->ComIPduSignalRef[sri] != NULL); sri++) {
                const ComSignal_type *signal = IPdu->ComIPduSignalRef[sri];
                Com_Arc_Signal_type * Arc_Signal = GET_ArcSignal(signal->ComHandleId);
                /* Monitor signal reception deadline*/
                /* @req COM685 */
                /* @req COM292 */
                /* @req COM290 */
                /* @req COM716 */
                /* @req COM617 */
                if ( (TRUE == Arc_IPdu->Com_Arc_IpduRxDMControl)&&
                     (signal->ComTimeoutFactor > 0) ) {
                   /* @req COM716 */
                   /* deadline timer can be zero for some signals as well hence the check for valid counter */
                    if(Arc_Signal->Com_Arc_DeadlineCounter != 0){
                        /* Decrease deadline monitoring timer.*/
                        timerDec(Arc_Signal->Com_Arc_DeadlineCounter);
                        /* Check if a timeout has occurred.*/
                        if (Arc_Signal->Com_Arc_DeadlineCounter == 0) {
                            /* @req COM500 */
                            /* @req COM744 */
                            boolean signalChanged = FALSE;
                            if (signal->Com_Arc_IsSignalGroup != FALSE) {
                                if (signal->ComRxDataTimeoutAction == COM_TIMEOUT_DATA_ACTION_REPLACE) {

                                    for (uint32 i=0;signal->ComGroupSignal[i]!=NULL; i++)
                                    {
                                        Com_Misc_WriteSignalDataToPdu(
                                            signal->ComGroupSignal[i]->ComSignalInitValue,
                                            signal->ComGroupSignal[i]->ComSignalType,
                                            Arc_IPdu->ComIPduDataPtr,
                                            signal->ComGroupSignal[i]->ComBitPosition,
                                            signal->ComGroupSignal[i]->ComBitSize,
                                            signal->ComGroupSignal[i]->ComSignalEndianess,
                                            &signalChanged);
                                    }
                                    Arc_Signal->ComSignalUpdated = TRUE;
                                }
                            }
                            else if (signal->ComRxDataTimeoutAction == COM_TIMEOUT_DATA_ACTION_REPLACE) {
                                /* Replace signal data.*/
                                /* @req COM470 */
                                Arc_Signal->ComSignalUpdated = TRUE;
                                Com_Misc_WriteSignalDataToPdu(
                                    signal->ComSignalInitValue,
                                    signal->ComSignalType,
                                    Arc_IPdu->ComIPduDataPtr,
                                    signal->ComBitPosition,
                                    signal->ComBitSize,
                                    signal->ComSignalEndianess,
                                    &signalChanged);
                            }
                            else {
                                /*intentionally left blank, else part is for lint exception*/
                            }
                            /* A timeout has occurred.*/
                            /* @req COM556 */
                            /* take out this out of resource protection mechanism ?*/
                            if ((signal->ComTimeoutNotification != COM_NO_FUNCTION_CALLOUT) && (ComNotificationCallouts[signal->ComTimeoutNotification] != NULL) ) {
                                ComNotificationCallouts[signal->ComTimeoutNotification]();
                            }
#if (COM_OSEKNM_SUPPORT == STD_ON)
                            if (signal->ComOsekNmNetId != COM_OSEKNM_INVALID_NET_ID) {
                                OsekNm_TimeoutNotification(signal->ComOsekNmNetId, signal->ComOsekNmNodeId);
                            }
#endif
                            /* Restart timer*/
                            Arc_Signal->Com_Arc_DeadlineCounter = signal->ComTimeoutFactor;
                        }
                    }
                }
                if (TRUE == Arc_Signal->ComSignalUpdated) {
                    pduUpdated = true;
                }
    #if (COM_SIG_GATEWAY_ENABLE == STD_ON)

                /* Use ComSignalRoutingReq variable for updated signal indication to avoid data consistency problems */
                Arc_Signal->ComSignalRoutingReq =  Arc_Signal->ComSignalUpdatedGwRouting;
                Arc_Signal->ComSignalUpdatedGwRouting = FALSE;
    #endif
            }
            if ((TRUE == pduUpdated) && (IPdu->ComIPduSignalProcessing == COM_DEFERRED) ) {
                Com_Misc_UnlockTpBuffer(getPduId(IPdu));
                memcpy(Arc_IPdu->ComIPduDeferredDataPtr,Arc_IPdu->ComIPduDataPtr,IPdu->ComIPduSize);
                for (uint16 i = 0; (IPdu->ComIPduSignalRef != NULL) && (IPdu->ComIPduSignalRef[i] != NULL); i++) {
                    const ComSignal_type *signal = IPdu->ComIPduSignalRef[i];
                    Com_Arc_Signal_type * Arc_Signal = GET_ArcSignal(signal->ComHandleId);
                    if (TRUE == Arc_Signal->ComSignalUpdated) {
                        /* take out this out of resource protection mechanism ?*/
                        if ((signal->ComNotification != COM_NO_FUNCTION_CALLOUT) && (ComNotificationCallouts[signal->ComNotification] != NULL) ) {
                            ComNotificationCallouts[signal->ComNotification]();
                        }
                        Arc_Signal->ComSignalUpdated = FALSE;
                    }
                }
            }
#if (COM_SIG_GATEWAY_ENABLE == STD_ON)
            Com_Arc_GwSrcDesc_type * gwSrcPtr;
            uint16 srcSigDescHandle;
            uint8 j;
            /* Indicate new source description value */
            if ((TRUE == IPdu->ComIPduGwRoutingReq) && (NULL != IPdu->ComIPduGwMapSigDescHandle)) {

                for (j=0; IPdu->ComIPduGwMapSigDescHandle[j] != INVALID_GWSIGNAL_DESCRIPTION_HANDLE; j++){
                    srcSigDescHandle = IPdu->ComIPduGwMapSigDescHandle[j];
                    gwSrcPtr = GET_ArcGwSrcSigDesc(srcSigDescHandle);
                    /* Use ComSignalRoutingReq variable for updated signal indication to avoid data consistency problems */
                    gwSrcPtr->ComSignalRoutingReq= gwSrcPtr->ComSignalUpdatedGwRouting;
                    gwSrcPtr->ComSignalUpdatedGwRouting = FALSE;
                }
            }
#endif
            SchM_Exit_Com_EA_0();
        }
    }
}

/**
 * Tx Main function- called from the task context
 * Processes the PDUS of all the tx modes, handles the timing functionalities/timers
 * @param void
 * @return none
 */
void Com_MainFunctionTx(void) {
    /* !req	COM789 */
    boolean dmTimeOut;
    /* @req COM665 */
    if( COM_INIT != Com_GetStatus() ) {
        DET_REPORTERROR(COM_MAINFUNCTIONTX_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }
    const ComIPdu_type *IPdu;
    for (uint16 i = 0; 0 == ComConfig->ComIPdu[i].Com_Arc_EOL; i++) {
        IPdu = &ComConfig->ComIPdu[i];
        Com_Arc_IPdu_type *Arc_IPdu = GET_ArcIPdu(i);
        dmTimeOut  = FALSE;
        /* Is this a IPdu that should be transmitted?*/
        if ( (IPdu->ComIPduDirection == COM_SEND) && (TRUE == Arc_IPdu->Com_Arc_IpduStarted) ) {
            const ComTxMode_type *txModePtr;
            if( TRUE == Arc_IPdu->Com_Arc_IpduTxMode ) {
                txModePtr = &IPdu->ComTxIPdu.ComTxModeTrue;
            }
            else {
                txModePtr = &IPdu->ComTxIPdu.ComTxModeFalse;
            }
            SchM_Enter_Com_EA_0();
            /* Decrease minimum delay timer*/
            timerDec(Arc_IPdu->Com_Arc_TxIPduTimers.ComTxIPduMinimumDelayTimer);

             if(0 < Arc_IPdu->Com_Arc_TxDeadlineCounter) /* if DM configured for this TX pdu */
             {
               /* Decrement Tx deadline monitoring timer.*/
                 if(0 < Arc_IPdu->Com_Arc_TxIPduTimers.ComTxDMTimer){ /* if DM timer is running */
                     timerDec(Arc_IPdu->Com_Arc_TxIPduTimers.ComTxDMTimer);
                     if(Arc_IPdu->Com_Arc_TxIPduTimers.ComTxDMTimer == 0){/* if timed out */
                         dmTimeOut = TRUE;
                         /*NOTE : it could happen that a new request arrive via Com_SendSignalGroup() or Com_SendSignal() or
                          * Com_SendDynamicSignal() during wait for last request(at this time) hence DM timer would be restarted there
                          * */
                     }
                 }
             }

            /* If IPDU has periodic or mixed transmission mode.*/
            if ( (txModePtr->ComTxModeMode == COM_PERIODIC)
                || (txModePtr->ComTxModeMode == COM_MIXED) ) {
                Com_ProcessMixedOrPeriodicTxMode(i,dmTimeOut);

            /* If IPDU has direct transmission mode.*/
            } else if (txModePtr->ComTxModeMode == COM_DIRECT) {
                Com_ProcessDirectTxMode(i,dmTimeOut);
            }
            /* The IDPU has NONE transmission mode.*/
            /* @req COM135 */
            /* !req  COM835 */
            else {
                /* Don't send!*/
            }

            SchM_Exit_Com_EA_0();

            /* Check notifications */
            boolean confirmationStatus = Com_Misc_GetTxConfirmationStatus(IPdu);
            Com_Misc_SetTxConfirmationStatus(IPdu, FALSE);

            /* !req  COM708 */
            for (uint16 signalIndex = 0;  (IPdu->ComIPduSignalRef != NULL) &&
                (IPdu->ComIPduSignalRef[signalIndex] != NULL); signalIndex++)
            {
                const ComSignal_type *signal = IPdu->ComIPduSignalRef[signalIndex];
                if(TRUE == confirmationStatus)
                {
                    if ((signal->ComNotification != COM_NO_FUNCTION_CALLOUT) &&
                        (ComNotificationCallouts[signal->ComNotification] != NULL) ) {
                         ComNotificationCallouts[signal->ComNotification]();
                    }
                }
                /* notify with the DM callbacks registered */
                /* @req  COM554 */
                /* @req  COM304 */
                if(TRUE == dmTimeOut){
                    if ((signal->ComTimeoutNotification != COM_NO_FUNCTION_CALLOUT) &&
                        (ComNotificationCallouts[signal->ComTimeoutNotification] != NULL) ) {
                         ComNotificationCallouts[signal->ComTimeoutNotification]();
                    }
#if (COM_OSEKNM_SUPPORT == STD_ON)
                    if (signal->ComOsekNmNetId != COM_OSEKNM_INVALID_NET_ID) {
                        OsekNm_TimeoutNotification(signal->ComOsekNmNetId, signal->ComOsekNmNodeId);
                    }
#endif
                }
            }
        }
    }
}




/**
 * Main function gateway route - called from the task context
 * Checks for all received signals and signal groups and routes
 * @param void
 * @return none
 */
/* @req COM400 */
/* @req COM667 */
/* @req COM668 */
void Com_MainFunctionRouteSignals( void ) {
    /* @req COM666 */
    if( COM_INIT != Com_GetStatus() ) {
        DET_REPORTERROR(COM_MAINFUNCTIONROUTESIGNALS_ID, COM_E_UNINIT);
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }

#if (COM_SIG_GATEWAY_ENABLE == STD_ON)

    uint16 iPduHandle,sigHandle, bitSize,srcByteNo, destByteNo,noOfBytes,byteCnt;
    const uint8 *srcDataPtr;
    uint8 *destDataPtr;
    const void *comSignalSrc;
    const void * arcSignalSrc;
    uint8 Data[8],j,i;
    ComGwSignalRef_type sigRefType;
    boolean isUpdated;
    const Com_Arc_Signal_type *arcSignalDest;
    Com_SignalType sigType;
    const ComSignal_type* comSigGrp;
    Com_Arc_ExtractPduInfo_Type pduInfo;
    const ComSignal_type *comSignalDest;

    /* @req COM359 */
    /* @req COM370 */ /*Configs for gateway are empty if gateway mapping are absent*/
    for (i=0;ComConfig->ComGwMappingRef[i].Com_Arc_EOL != 1; i++) {
        isUpdated = FALSE;
        sigRefType = ComConfig->ComGwMappingRef[i].ComGwSourceSignalRef;
        sigHandle = ComConfig->ComGwMappingRef[i].ComGwSourceSignalHandle;

        /*lint -save -e644 -e645 */
        /* Signal group mapping */
        if (COM_SIGNAL_GROUP_REFERENCE == sigRefType) {
            /* @req COM361 */
            comSignalSrc = GET_Signal(sigHandle);
            arcSignalSrc   = GET_ArcSignal(sigHandle);
            isUpdated       = ((const Com_Arc_Signal_type *)arcSignalSrc)->ComSignalRoutingReq;

            if (TRUE == isUpdated) {

                SchM_Enter_Com_EA_0();
                Com_Misc_CopySignalGroupDataFromPduToShadowBuffer(sigHandle);
                SchM_Exit_Com_EA_0();

                for (j=0;j < ComConfig->ComGwMappingRef[i].ComGwNoOfDesitnationRoutes;j++) {

                    sigHandle   = ComConfig->ComGwMappingRef[i].ComGwDestinationRef[j].ComGwDestinationSignalHandle;
                    comSignalDest = GET_Signal(sigHandle);
                    arcSignalDest = GET_ArcSignal(sigHandle);
                    srcByteNo = ((const ComSignal_type *)comSignalSrc)->ComBitPosition/8; /*Starting bit position*/
                    noOfBytes   = ((((const ComSignal_type *)comSignalSrc)->ComEndBitPosition/8) - srcByteNo) + 1; /*No of data bytes*/
                    destByteNo = ((comSignalDest)->ComBitPosition/8);
                    srcDataPtr =  (const uint8 *)(((const Com_Arc_Signal_type *)arcSignalSrc)->Com_Arc_ShadowBuffer);
                    /*lint -e{9005} GET_ArcSignal(sigHandle) value is modified by destDataPtr*/
                    destDataPtr = (uint8 *)((arcSignalDest)->Com_Arc_ShadowBuffer);
                    /* Manage Signal group routing as one consistent block by utilizing shadow buffer */
                    /* @req COM383 */
                    SchM_Enter_Com_EA_0();

                    for (byteCnt=0; byteCnt<noOfBytes; byteCnt++){
                        destDataPtr[destByteNo] = srcDataPtr[srcByteNo];
                        destByteNo++;
                        srcByteNo++;
                    }
                    SchM_Exit_Com_EA_0();
                    (void)Com_SendSignalGroup(sigHandle); /*Update destination IPdu ram buffer*/
                }
            }


        } else {
            switch(sigRefType) {
                /* Signal mapping */
                case COM_SIGNAL_REFERENCE:
                    /* @req COM357 */
                    comSignalSrc    = GET_Signal(sigHandle);
                    arcSignalSrc   = GET_ArcSignal(((const ComSignal_type *)comSignalSrc)->ComHandleId);
                    isUpdated       = ((const Com_Arc_Signal_type *)arcSignalSrc)->ComSignalRoutingReq;
                    iPduHandle      = ((const ComSignal_type *)comSignalSrc)->ComIPduHandleId;
                    bitSize         =((const ComSignal_type *)comSignalSrc)->ComBitSize;
                    sigType         =((const ComSignal_type *)comSignalSrc)->ComSignalType;
                    pduInfo.ComSignalType         =   sigType;
                    pduInfo.ComBitPosition        =   ((const ComSignal_type *)comSignalSrc)->ComBitPosition;
                    pduInfo.ComBitSize            =   bitSize;
                    pduInfo.ComSignalEndianess    =   ((const ComSignal_type *)comSignalSrc)->ComSignalEndianess;
                    break;

                    /* Group Signal mapping */
                case COM_GROUP_SIGNAL_REFERENCE:

                    comSignalSrc    = GET_GroupSignal(sigHandle);
                    comSigGrp       = &ComConfig->ComSignal[((const ComGroupSignal_type *)comSignalSrc)->ComSigGrpHandleId];
                    arcSignalSrc   = GET_ArcSignal(comSigGrp->ComHandleId);
                    isUpdated       = ((const Com_Arc_Signal_type *)arcSignalSrc)->ComSignalRoutingReq;
                    iPduHandle      = comSigGrp->ComIPduHandleId;
                    bitSize         =((const ComGroupSignal_type *)comSignalSrc)->ComBitSize;
                    sigType         =((const ComGroupSignal_type *)comSignalSrc)->ComSignalType;
                    pduInfo.ComSignalType         =   sigType;
                    pduInfo.ComBitPosition        =   ((const ComGroupSignal_type *)comSignalSrc)->ComBitPosition;
                    pduInfo.ComBitSize            =   bitSize;
                    pduInfo.ComSignalEndianess    =   ((const ComGroupSignal_type *)comSignalSrc)->ComSignalEndianess;
                    break;

                    /* Gateway source signal mapping */
                case GATEWAY_SIGNAL_DESCRIPTION:

                    comSignalSrc    = GET_GwSrcSigDesc(sigHandle);
                    arcSignalSrc   = GET_ArcGwSrcSigDesc(sigHandle);
                    isUpdated       = ((const Com_Arc_GwSrcDesc_type *)arcSignalSrc)->ComSignalRoutingReq;
                    iPduHandle      = ((const ComGwSrcDesc_type *)comSignalSrc)->ComIPduHandleId;
                    bitSize         =((const ComGwSrcDesc_type *)comSignalSrc)->ComBitSize;
                    sigType         =((const ComGwSrcDesc_type *)comSignalSrc)->ComSignalType;
                    pduInfo.ComSignalType         =   sigType;
                    pduInfo.ComBitPosition        =   ((const ComGwSrcDesc_type *)comSignalSrc)->ComBitPosition;
                    pduInfo.ComBitSize            =   bitSize;
                    pduInfo.ComSignalEndianess    =   ((const ComGwSrcDesc_type *)comSignalSrc)->ComSignalEndianess;
                    break;

                case COM_SIGNAL_GROUP_REFERENCE:
                    break;
                default : break;

            }
            /* Route for new receptions */
            if (TRUE == isUpdated) {

                Com_Misc_ExtractGwSrcSigData(comSignalSrc,iPduHandle,Data,&pduInfo);
                Com_Misc_RouteGwDestnSignals(i,Data,sigType,bitSize);
            }
        }

    }
    /*lint -restore */
#endif

}
