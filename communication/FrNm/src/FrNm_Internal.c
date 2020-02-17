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
#include "FrNm.h"
#include "FrNm_Internal.h"
#include "PduR_FrNm.h"
#include "SchM_FrNm.h"
/*lint -esym(9003, FrNm_ConfigPtr) MISRA:OTHER:Readability:[MISRA 2012 Rule 8.9, advisory] */
extern const FrNm_ConfigType* FrNm_ConfigPtr ;
/*lint -esym(9003, FrNm_Internal) MISRA:OTHER:Readability:[MISRA 2012 Rule 8.9, advisory] */
extern FrNm_InternalType FrNm_Internal;

#define FRNM_LSBIT_MASK                                    0x1u


static inline void FrNm_Internal_ReadySleep_to_BusSleep( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal );
static inline void FrNm_Internal_RepeatMessage_to_ReadySleep( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal );
#if (FRNM_PASSIVE_MODE_ENABLED == STD_OFF)
static inline void FrNm_Internal_ReadySleep_to_NormalOperation( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal );
static inline void FrNm_Internal_RepeatMessage_to_NormalOperation( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal );
#endif /* FRNM_PASSIVE_MODE_ENABLED */



/**
 * @brief Transition from Ready sleep state to Bus sleep state
 * @param ChannelConf - Channel configuration data ptr
 * @param ChannelInternal - Channel Internal runtime data ptr
 * @return void
 */
static inline void FrNm_Internal_ReadySleep_to_BusSleep( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal ) {
    /* @req FRNM129 */
    ChannelInternal->Mode = NM_MODE_BUS_SLEEP;
    ChannelInternal->State = NM_STATE_BUS_SLEEP;
    ChannelInternal->MessageTimeoutTimeLeft = 0;
    Nm_BusSleepMode( ChannelConf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef); /* @req FRNM135 */
    ChannelInternal->FrNm_RepeatMessage = FALSE; /* @req FRNM320 */
#if (FRNM_STATE_CHANGE_INDICATION_ENABLED == STD_ON) /* @req FRNM106 */
    Nm_StateChangeNotification(ChannelConf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef,NM_STATE_READY_SLEEP, NM_STATE_BUS_SLEEP);
#endif
}

/**
 * @brief transit from Repeat message state to Ready sleep state in Network mode
 * @param ChannelConf - Channel configuration data ptr
 * @param ChannelInternal - Channel Internal runtime data ptr
 * @return void
 */
static inline void FrNm_Internal_RepeatMessage_to_ReadySleep( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal ) {
    ChannelInternal->Mode = NM_MODE_NETWORK;
    ChannelInternal->State = NM_STATE_READY_SLEEP;
    FrNm_Internal_Resetvotedata(ChannelInternal);
    /* @req FRNM127 */
    ChannelInternal->readySleepCntLeft = ChannelConf->FrNmTimingConfig->FrNmReadySleepCnt;
    ChannelInternal->MessageTimeoutTimeLeft = 0 ;
#if (FRNM_STATE_CHANGE_INDICATION_ENABLED == STD_ON) /* @req FRNM106 */
    Nm_StateChangeNotification(ChannelConf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef, NM_STATE_REPEAT_MESSAGE, NM_STATE_READY_SLEEP);
#endif
#if (FRNM_CONTROL_BIT_VECTOR_ENABLED == STD_OFF)/* @req FRNM324 */
            ChannelInternal->FrNmTxDataPdu[CBV_INDEX] = 0x00; /* @req FRNM3829 */
#else
#if(FRNM_REPEAT_MESSAGE_BIT_ENABLED == STD_ON)
            ChannelInternal->FrNmTxDataPdu[CBV_INDEX] &=(uint8)~FRNM_CBV_REPEAT_MESSAGE_REQUEST;
#endif
#endif
}


#if (FRNM_PASSIVE_MODE_ENABLED == STD_OFF)


/**
 * @brief Transition from Ready sleep state to Normal operation in Network mode
 * @param ChannelConf - Channel configuration data ptr
 * @param ChannelInternal - Channel Internal runtime data ptr
 * @return void
 */
static inline void FrNm_Internal_ReadySleep_to_NormalOperation( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal ) {
    ChannelInternal->Mode = NM_MODE_NETWORK;
    ChannelInternal->State = NM_STATE_NORMAL_OPERATION;
    ChannelInternal->MessageTimeoutTimeLeft = ChannelConf->FrNmTimingConfig->FrNmMsgTimeoutTime ;
#if (FRNM_STATE_CHANGE_INDICATION_ENABLED == STD_ON) /* @req FRNM106 */
    Nm_StateChangeNotification(ChannelConf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef, NM_STATE_READY_SLEEP, NM_STATE_NORMAL_OPERATION);
#endif
}


/**
 * @brief Transit from Repeat message state to Normal operation state in Network mode
 * @param ChannelConf - Channel configuration data ptr
 * @param ChannelInternal - Channel Internal runtime data ptr
 * @return void
 */
static inline void FrNm_Internal_RepeatMessage_to_NormalOperation( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal ) {
    ChannelInternal->Mode = NM_MODE_NETWORK;
    ChannelInternal->State = NM_STATE_NORMAL_OPERATION;
#if (FRNM_STATE_CHANGE_INDICATION_ENABLED == STD_ON) /* @req FRNM106 */
    Nm_StateChangeNotification(ChannelConf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef, NM_STATE_REPEAT_MESSAGE, NM_STATE_NORMAL_OPERATION);
#else
    (void)*ChannelConf;
#endif
#if (FRNM_CONTROL_BIT_VECTOR_ENABLED == STD_OFF)/* @req FRNM324 */
            ChannelInternal->FrNmTxDataPdu[CBV_INDEX] = 0x00; /* @req FRNM3829 */
#else
#if(FRNM_REPEAT_MESSAGE_BIT_ENABLED == STD_ON)
            ChannelInternal->FrNmTxDataPdu[CBV_INDEX] &=(uint8)~FRNM_CBV_REPEAT_MESSAGE_REQUEST;
#endif
#endif
}
#endif /* FRNM_PASSIVE_MODE_ENABLED */


/**
 * @brief Transit from Synchronize state to Normal operation state.
 * @param ChannelConf - Channel configuration data ptr
 * @param ChannelInternal - Channel Internal runtime data ptr
 * @return void
 */
void FrNm_Internal_Synchronize_to_RepeatMessage( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal ) {

    ChannelInternal->Mode = NM_MODE_NETWORK;   /* @req FRNM143 */
    ChannelInternal->State = NM_STATE_REPEAT_MESSAGE; /* @req FRNM108 */
    /* @req FRNM119 */  /* @req FRNM117*/
    ChannelInternal->RepeatMessageTimeLeft = ChannelConf->FrNmTimingConfig->FrNmRepeatMessageTime+ChannelConf->FrNmTimingConfig->FrNmMainFunctionPeriod;
    /* @req FRNM109 */
    ChannelInternal->FrNm_RepeatMessage = TRUE;
    /* Notify 'Network Mode' */
    Nm_NetworkMode(ChannelConf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef); /* @req FRNM110 */
#if( FRNM_PASSIVE_MODE_ENABLED == STD_ON)
    ChannelInternal->MessageTimeoutTimeLeft = 0; /* Disable timeout timer in passive mode*/
#else
    ChannelInternal->MessageTimeoutTimeLeft = ChannelConf->FrNmTimingConfig->FrNmMsgTimeoutTime;
#endif

#if (FRNM_STATE_CHANGE_INDICATION_ENABLED == STD_ON) /* @req FRNM106 */
    Nm_StateChangeNotification(ChannelConf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef, NM_STATE_SYNCHRONIZE, NM_STATE_REPEAT_MESSAGE);
#endif


#if (FRNM_CONTROL_BIT_VECTOR_ENABLED == STD_OFF)/* @req FRNM324 */
    ChannelInternal->FrNmTxDataPdu[CBV_INDEX] = 0x00; /* @req FRNM3829 */
#else
    /* @req FRNM405 */
    if (ChannelConf->FrNmChannelIdentifiersConfig->FrNmPnEnabled == STD_ON) {
        ChannelInternal->FrNmTxDataPdu[CBV_INDEX] |= FRNM_CBV_PNI; /* @req FRNM404 */ /* @req FRNM409 */
    } else  {
        ChannelInternal->FrNmTxDataPdu[CBV_INDEX] = 0;
    }
#endif
    if(ChannelInternal->activeWakeup == TRUE) {
#if (FRNM_CONTROL_BIT_VECTOR_ENABLED == STD_ON)
        ChannelInternal->FrNmTxDataPdu[CBV_INDEX] |= (FRNM_CBV_ACTIVE_WAKEUP); /* @req FRNM297 */
#endif
        /* Due to startup error transition to synchronize state flag is cleared */
        ChannelInternal->activeWakeup = FALSE;
    }

}
#if ((FRNM_PNC_EIRA_CALC_ENABLED == STD_ON) && (FRNM_PNC_COUNT > 0))

/**
 * @brief restart PN timers
 * @param pnIndex - Index of PNC
 * @return void
 */
static inline void restartPnEiraTimer(uint8 pnIndex) {

    uint8 timerIndex;

    timerIndex = FrNm_ConfigPtr->FrNmPnInfo->FrNmPnIndexToTimerMap[pnIndex];
    FrNm_Internal.pnEIRATimers[timerIndex].timerRunning = TRUE;
    FrNm_Internal.pnEIRATimers[timerIndex].resetTimer = FRNM_PN_RESET_TIME;

}

/**
 * @brief Identify the PN timer that needs to be restarted
 * @param calcEIRA - EIRA of the new received/transmitted and filtered PDU
 * @return void
 */
static void restartTimerForInternalExternalRequests(FrNm_Internal_RAType *calcEIRA) {

    uint8 byteNo;
    uint8 bit;
    uint8 byteEIRA;

    for (byteNo = 0; byteNo < FrNm_ConfigPtr->FrNmPnInfo->FrNmPnInfoLen; byteNo++) {

        byteEIRA = calcEIRA->bytes[byteNo];

        if ( byteEIRA > 0) {

            for (bit = 0; bit < 8; bit++) {

                if ((byteEIRA >> bit) & FRNM_LSBIT_MASK) {
                    restartPnEiraTimer((byteNo * 8) + bit);
                } else {
                    /* Do nothing */
                }
            }
        } else {
            /* Do nothing */
        }
    }
}

/**
 * @brief NM filtering process done for each reception
 * @param ChannelConf - Channel configuration
 * @param ChannelInternal - Channel internal runtime data
 * @return reception valid or not
 *
 */
void FrNm_Internal_RxProcess(FrNm_Internal_ChannelType* ChannelInternal ) {

    FrNm_Internal_RAType calculatedEIRA = {0};
    FrNm_Internal_RAType EIRAOld = {0};
    PduInfoType pdu;
    uint8 i;
    uint8 offset;
    boolean changed;

    /* @req FRNM408 */
    offset = FrNm_ConfigPtr->FrNmPnInfo->FrNmPnInfoOffset;

    for (i = 0; i < FrNm_ConfigPtr->FrNmPnInfo->FrNmPnInfoLen; i++) {
        /* Accumulate external requests *//* @req FRNM418 */ /* @req FRNM416 */ /* @req FRNM420 */ /* @req FRNM422 */
        calculatedEIRA.bytes[i] = ChannelInternal->FrNmRxDataPdu[i + offset] & FrNm_ConfigPtr->FrNmPnInfo->FrNmPnInfoMask[i];
    }

    if (calculatedEIRA.data != 0){

        SchM_Enter_FrNm_EA_0();
        /* @req FRNM426 */
        restartTimerForInternalExternalRequests(&calculatedEIRA);

        EIRAOld.data = FrNm_Internal.pnEIRA.data;
        FrNm_Internal.pnEIRA.data |= calculatedEIRA.data; /* @req FRNM423 */
        changed = (EIRAOld.data != FrNm_Internal.pnEIRA.data) ? TRUE: FALSE;

        SchM_Exit_FrNm_EA_0();
        if (changed) {
            pdu.SduDataPtr = &FrNm_Internal.pnEIRA.bytes[0];
            pdu.SduLength = FrNm_ConfigPtr->FrNmPnInfo->FrNmPnInfoLen;
            /* @req FRNM429 */
            PduR_FrNmRxIndication(FrNm_ConfigPtr->FrNmPnInfo->FrNmEIRARxNSduId,&pdu);
        }
    }

}




/**
 * @brief Determine internal requests from ComM and identify which PN timer has to be started
 * @param pnInfo - Pn Info range of the transmitted PDU
 * @return void
 */

void FrNm_Internal_ProcessTxPdu(uint8 *pnInfo) {

    PduInfoType pdu;
    FrNm_Internal_RAType calculatedEIRA = {0};
    FrNm_Internal_RAType EIRAOld = {0};
    uint8 byteNo;
    boolean changed;
    SchM_Enter_FrNm_EA_0();

    for (byteNo = 0; byteNo < FrNm_ConfigPtr->FrNmPnInfo->FrNmPnInfoLen; byteNo++) {
        /* @req FRNM418 FRNM416 FRNM420 FRNM422 */
        calculatedEIRA.bytes[byteNo] = *(pnInfo + byteNo) & FrNm_ConfigPtr->FrNmPnInfo->FrNmPnInfoMask[byteNo]; /* Accumulate internal requests */
    }

    if (calculatedEIRA.data != 0) {
        /* @req FRNM427 */
        restartTimerForInternalExternalRequests(&calculatedEIRA);
        EIRAOld.data = FrNm_Internal.pnEIRA.data;
        FrNm_Internal.pnEIRA.data |= calculatedEIRA.data; /* @req FRNM424 */
        changed = (EIRAOld.data != FrNm_Internal.pnEIRA.data) ? TRUE: FALSE;

        if (changed) {

            pdu.SduDataPtr = &FrNm_Internal.pnEIRA.bytes[0];
            pdu.SduLength = FrNm_ConfigPtr->FrNmPnInfo->FrNmPnInfoLen;
            /* @req FRNM429 */
            PduR_FrNmRxIndication(FrNm_ConfigPtr->FrNmPnInfo->FrNmEIRARxNSduId,&pdu);
        }
    }
    SchM_Exit_FrNm_EA_0();
}


/**
 * @brief reset EIRA bits in case of timeout
 * @param pnIndex - Indices of PNC which have timed out
 * @param indexCount - No of PNC that have timed out
 * @return void
 */
void FrNm_Internal_resetEIRAPNbits(uint8 *pnIndex, uint8 indexCount)
{
    PduInfoType pdu;
    uint8 byteNo;
    uint8 bit;
    uint8 idx;

    for (idx=0;idx<indexCount;idx++)
    {
        byteNo  = (*(pnIndex+idx))/8;
        bit     = (*(pnIndex+idx))%8;
        FrNm_Internal.pnEIRA.bytes[byteNo] &=  ~(1u<<bit); /* Reset PN bit */
    }
    pdu.SduDataPtr = &FrNm_Internal.pnEIRA.bytes[0];
    pdu.SduLength = FrNm_ConfigPtr->FrNmPnInfo->FrNmPnInfoLen;
    /* @req FRNM429 */
    PduR_FrNmRxIndication(FrNm_ConfigPtr->FrNmPnInfo->FrNmEIRARxNSduId,&pdu);

}


/**
 * @brief Run PN reset timers every main function cycle
 * @param void
 * @return void
 */
void FrNm_Internal_TickPnEIRAResetTime(uint32 mainFuncPeriod)
{
    uint8 pncIndexReset[FRNM_PNC_COUNT]= {0};
    uint8 len;
    len = 0;
    uint8 timerIdx;
    SchM_Enter_FrNm_EA_0();

    for (timerIdx=0; timerIdx<FRNM_PNC_COUNT;timerIdx++) {
        if (FrNm_Internal.pnEIRATimers[timerIdx].timerRunning)
        {
            if (mainFuncPeriod >= FrNm_Internal.pnEIRATimers[timerIdx].resetTimer)
            {
                FrNm_Internal.pnEIRATimers[timerIdx].timerRunning = FALSE;
                pncIndexReset[len] = FrNm_ConfigPtr->FrNmPnInfo->FrNmTimerIndexToPnMap[timerIdx];
                len++;
            } else {
                FrNm_Internal.pnEIRATimers[timerIdx].resetTimer -= mainFuncPeriod;
            }
        }
    }
    if (len> 0)
    {
        /* @req FRNM428 */
        FrNm_Internal_resetEIRAPNbits(pncIndexReset,len);
    }
    SchM_Exit_FrNm_EA_0();
}

#endif

/**
 * @brief To handle cycles and time out of RepetitionCycles
 * @param ChannelConf - Channel configuration data ptr
 * @param ChannelInternal - Channel Internal runtime data ptr
 * @param CurrentFlexrayCycle - Current flexray cycle received from FrIf and FrDrv
 */
void FrNm_internal_CheckRepetitionCycles(const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal, uint8 CurrentFlexrayCycle ) {

    #if (FRNM_MAIN_ACROSS_FRCYCLE == STD_ON)

        if((CurrentFlexrayCycle%(uint8)ChannelConf->FrNmTimingConfig->FrNmRepetitionCycle) == 0) {
            ChannelInternal->repetitionCycleCompleted = TRUE;
        } else {
            ChannelInternal->repetitionCycleCompleted = FALSE;
        }
    #else
        if (((CurrentFlexrayCycle+1)%FRNM_MAX_FLEXRAY_CYCLE_COUNT)%ChannelConf->FrNmTimingConfig->FrNmRepetitionCycle == 0) {
            ChannelInternal->repetitionCycleCompleted= TRUE;
        } else {
            ChannelInternal->repetitionCycleCompleted = FALSE;
        }
    #endif

}

/**
 * @brief To handle ticks and time out of RepeatMessageTime
 * @param ChannelConf - Channel configuration data ptr
 * @param ChannelInternal - Channel Internal runtime data ptr
 * @return void
 */

void FrNm_Internal_TickRepeatMessageTime( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal ) {
  if(ChannelInternal->RepeatMessageTimeLeft != 0) {
    if (ChannelConf->FrNmTimingConfig->FrNmMainFunctionPeriod >= ChannelInternal->RepeatMessageTimeLeft) {
        ChannelInternal->RepeatMessageTimeLeft = 0;
        /* @req FRNM120 */ /* @req FRNM112 */
        ChannelInternal->FrNm_RepeatMessage = FALSE;
#if (FRNM_PASSIVE_MODE_ENABLED == STD_OFF)
        if(ChannelInternal->FrNm_NetworkRequested == TRUE) {
            /* @req FRNM121 */
            FrNm_Internal_RepeatMessage_to_NormalOperation(ChannelConf, ChannelInternal);
        }else
#endif
        {
            /* @req FRNM122 */
            FrNm_Internal_RepeatMessage_to_ReadySleep(ChannelConf, ChannelInternal);
        }

    } else {
        ChannelInternal->RepeatMessageTimeLeft -=(ChannelConf->FrNmTimingConfig->FrNmMainFunctionPeriod);
    }
  }
}




/**
 * @brief Global time error states handling
 * @param ChannelConf - Channel configuration data ptr
 * @param ChannelInternal - Channel Internal runtime data ptr
 * @return void
 */
void FrNm_Internal_Globaltime_Error_Handling( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal ) {

    if(ChannelInternal->Mode == NM_MODE_NETWORK){
       if(ChannelInternal->State == NM_STATE_REPEAT_MESSAGE){
            /* @req FRNM386 */
#if (FRNM_CYCLE_COUNTER_EMULATION == STD_ON)
                ChannelInternal->Mode = NM_MODE_BUS_SLEEP;
                ChannelInternal->State = NM_STATE_BUS_SLEEP;
            /* @req FRNM134 */
#if ( FRNM_STATE_CHANGE_INDICATION_ENABLED == STD_ON) /* @req FRNM106 */
            Nm_StateChangeNotification( ChannelConf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef, NM_STATE_REPEAT_MESSAGE,NM_STATE_BUS_SLEEP);
#endif
            Nm_BusSleepMode( ChannelConf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef); /* @req FRNM135 */
            ChannelInternal->FrNm_RepeatMessage = FALSE; /* @req FRNM320 */
#else
            /* @req FRNM384 */
             ChannelInternal->Mode = NM_MODE_SYNCHRONIZE;
             ChannelInternal->State = NM_STATE_SYNCHRONIZE;
#if ( FRNM_STATE_CHANGE_INDICATION_ENABLED == STD_ON) /* @req FRNM106 */
             Nm_StateChangeNotification( ChannelConf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef, NM_STATE_REPEAT_MESSAGE,NM_STATE_SYNCHRONIZE);
#endif

#endif
        }else if(ChannelInternal->State == NM_STATE_NORMAL_OPERATION){
             /* @req FRNM342*/
             ChannelInternal->Mode = NM_MODE_SYNCHRONIZE;
             ChannelInternal->State = NM_STATE_SYNCHRONIZE;
#if ( FRNM_STATE_CHANGE_INDICATION_ENABLED == STD_ON) /* @req FRNM106 */
             Nm_StateChangeNotification( ChannelConf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef, NM_STATE_NORMAL_OPERATION,NM_STATE_SYNCHRONIZE);
#else
             (void)*ChannelConf;
#endif

        }else if(ChannelInternal->State == NM_STATE_READY_SLEEP) {
#if (FRNM_CYCLE_COUNTER_EMULATION == STD_ON)
            /* @req FRNM379*/
            if (ChannelConf->FrNmTimingConfig->FrNmMainFunctionPeriod >= ChannelInternal->syncLossTimerLeft){
                ChannelInternal->readySleepCntLeft--;
                ChannelInternal->syncLossTimerLeft = ChannelConf->FrNmTimingConfig->FrNmSyncLossTimer;/* @req FRNM380*/
             }
            if(ChannelInternal->readySleepCntLeft<1){
                /* As per 4.2.2 specification on below condition state should transit to BusSleep */
             ChannelInternal->Mode = NM_MODE_BUS_SLEEP;
             ChannelInternal->State = NM_STATE_BUS_SLEEP;
             /* @req FRNM134 */
#if ( FRNM_STATE_CHANGE_INDICATION_ENABLED == STD_ON) /* @req FRNM106 */
             Nm_StateChangeNotification( ChannelConf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef, NM_STATE_READY_SLEEP,NM_STATE_BUS_SLEEP);
#endif
             Nm_BusSleepMode( ChannelConf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef); /* @req FRNM135 */
             ChannelInternal->FrNm_RepeatMessage = FALSE; /* @req FRNM320 */
            }
#endif
         }else{
            /* Do Nothing */
           }
       FrNm_Internal_Resetvotedata(ChannelInternal);
       ChannelInternal->MessageTimeoutTimeLeft = 0;
#if (FRNM_CONTROL_BIT_VECTOR_ENABLED == STD_OFF)/* @req FRNM324 */
       ChannelInternal->FrNmTxDataPdu[CBV_INDEX] = 0x00; /* @req FRNM3829 */
#else
       /* due to startup error flags are reset */
       ChannelInternal->FrNmTxDataPdu[CBV_INDEX] &= (uint8)~FRNM_CBV_ACTIVE_WAKEUP; /* @req FRNM298 */
       ChannelInternal->FrNmTxDataPdu[CBV_INDEX] &= (uint8)~FRNM_CBV_REPEAT_MESSAGE_REQUEST;
#endif
    }
}

#if (FRNM_PASSIVE_MODE_ENABLED == STD_OFF)
/**
 * @brief Handle periodic NM message (vote, pdu and userdata) transmission
 * @param ChannelConf - Channel configuration data ptr
 * @param ChannelInternal - Channel Internal runtime data ptr
 * @return void
 */
Std_ReturnType FrNm_Internal_TransmitMessage( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal ) {
    /* @req FRNM148 */ /* @req FRNM151 */ /* @req FRNM100 */
    Std_ReturnType Status;
    PduInfoType datapdu= {
            .SduDataPtr = &ChannelInternal->FrNmTxDataPdu[VOTE_INDEX],
            .SduLength = ChannelConf->FrNmChannelIdentifiersConfig->FrNmTxPduLength,
    };
    PduInfoType votepdu= {
            .SduDataPtr = &ChannelInternal->FrNmTxVotePdu,
            .SduLength = VOTE_PDU_LENGTH,
    };
    PduIdType txdataPduId;
    PduIdType txvotePduId;
    /*This is only for initialization*/
    txdataPduId = ChannelConf->FrNmChannelIdentifiersConfig->FrNmTxPduList->FrNmTxConfPduId;
    txvotePduId = txdataPduId;
    if (FALSE == ChannelInternal->CommunicationEnabled) {
        /*lint -e{904} Return statement is necessary to avoid multiple if loops and hence increase readability in case of argument check */
          return E_NOT_OK;
     }
    Status=E_NOT_OK;
    /* @req FRNM160 */ /* @req FRNM337 */ /* @req FRNM147 */
    if((ChannelConf->FrNmChannelIdentifiersConfig->FrNmPduScheduleVariant== FRNM_PDU_SCHEDULE_VARIANT_1)
       ||(ChannelConf->FrNmChannelIdentifiersConfig->FrNmPduScheduleVariant== FRNM_PDU_SCHEDULE_VARIANT_2)) {
        if(E_OK == FrNm_Internal_Get_pdudataPtr_ForTx(ChannelConf, ChannelInternal,&datapdu,&txdataPduId)){
            if(ChannelInternal->FrNm_NetworkRequested==TRUE){
                /* @req FRNM055*/ /* @req FRNM156*/ /* @req FRNM215 */
                ChannelInternal->FrNmTxDataPdu[VOTE_INDEX] |= VOTE_VALUE;
            }else{
                ChannelInternal->FrNmTxDataPdu[VOTE_INDEX] &= (uint8)~VOTE_VALUE;
            }
            Status = FrNm_Internal_Get_Userdata_ForTx(ChannelConf, ChannelInternal,&datapdu);
            if(E_NOT_OK != Status){
                Status = FrIf_Transmit(txdataPduId, &datapdu);/* @req FRNM010 */
            }
         }
    } else if(ChannelConf->FrNmChannelIdentifiersConfig->FrNmPduScheduleVariant== FRNM_PDU_SCHEDULE_VARIANT_7) {/* @req FRNM160 */ /* @req FRNM337 */
        if(E_OK == FrNm_Internal_Get_pduVote_ForTx(ChannelConf,&txvotePduId)){
            if(ChannelInternal->FrNm_NetworkRequested==TRUE){
               /* @req FRNM161*/  /* @req FRNM055*/ /* @req FRNM219 */ /* @req FRNM234 */ /* @req FRNM162 */
               ChannelInternal->FrNmTxVotePdu = VOTE_VALUE|ChannelInternal->FrNmTxDataPdu[VOTE_INDEX];
            }else{
                ChannelInternal->FrNmTxVotePdu &= ((uint8)~VOTE_VALUE)|ChannelInternal->FrNmTxDataPdu[VOTE_INDEX];
            }
            Status = FrIf_Transmit(txvotePduId, &votepdu);/* @req FRNM147 */ /* @req FRNM010 */
        }
        if(Status == E_OK ){
            Status =FrNm_Internal_Transmit_Nmpdudata(ChannelConf, ChannelInternal,&datapdu,&txdataPduId);
        }

    } else if((ChannelConf->FrNmChannelIdentifiersConfig->FrNmPduScheduleVariant== FRNM_PDU_SCHEDULE_VARIANT_3)
            ||(ChannelConf->FrNmChannelIdentifiersConfig->FrNmPduScheduleVariant== FRNM_PDU_SCHEDULE_VARIANT_4)
            ||(ChannelConf->FrNmChannelIdentifiersConfig->FrNmPduScheduleVariant== FRNM_PDU_SCHEDULE_VARIANT_5)
            ||(ChannelConf->FrNmChannelIdentifiersConfig->FrNmPduScheduleVariant== FRNM_PDU_SCHEDULE_VARIANT_6)){/* @req FRNM160 *//* @req FRNM337 */
        if(E_OK == FrNm_Internal_Get_pduVote_ForTx(ChannelConf,&txvotePduId)){
            if(ChannelInternal->FrNm_NetworkRequested==TRUE){
                /* @req FRNM161*/ /* @req FRNM215 *//* @req FRNM216 */ /* @req FRNM218 */ /* @req FRNM219 */
                ChannelInternal->FrNmTxVotePdu = VOTE_VALUE;
            }else{
                ChannelInternal->FrNmTxVotePdu &=(uint8)~VOTE_VALUE;
            }
            Status = FrIf_Transmit(txvotePduId, &votepdu); /* @req FRNM147 *//* @req FRNM010 */
         }
        if(Status == E_OK ){
            Status =FrNm_Internal_Transmit_Nmpdudata(ChannelConf, ChannelInternal,&datapdu,&txdataPduId);
        }
    }else{
        /*Nothing to do */
    }
  return Status;
}

/**
 * @brief - To transmit pdu with user data to lower layers(FrIf).
 * @param ChannelConf - Channel configuration data ptr
 * @param ChannelInternal - Channel Internal runtime data ptr
 * @param dataPduPtr - Transmission pdu data pointer
 * @param dataPduId - FRIF pdu Id for transmission
 * @return
 */
Std_ReturnType FrNm_Internal_Transmit_Nmpdudata(const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal,PduInfoType* dataPduPtr, PduIdType* dataPduId){
    Std_ReturnType Status;
    Status = E_NOT_OK;
    if(E_OK == FrNm_Internal_Get_pdudataPtr_ForTx(ChannelConf, ChannelInternal,dataPduPtr,dataPduId)){
        /* @req FRNM214 */ /* @req FRNM157*/ /* @req FRNM161*/ /* @req FRNM055*/ /* @req FRNM156*/ /* @req FRNM216 */
        ChannelInternal->FrNmTxDataPdu[VOTE_INDEX] &= (uint8)~VOTE_VALUE;
        Status = FrNm_Internal_Get_Userdata_ForTx(ChannelConf, ChannelInternal,dataPduPtr);
        if(E_NOT_OK != Status ){
          /* @req FRNM147 */ /* @req FRNM010 */
          Status = FrIf_Transmit(*dataPduId,dataPduPtr );
        }
    }
return Status;
}
/**
 * @brief - To get the local vote data and Id for transmission.
 * @param ChannelConf - Channel configuration data ptr
 * @param ChannelInternal -Channel Internal local runtime data ptr
 * @param dataPduId - FRIF vote pdu Id for transmission
 * @return
 */
Std_ReturnType FrNm_Internal_Get_pduVote_ForTx( const FrNm_ChannelInfoType* ChannelConf,PduIdType* dataPduId ){

    Std_ReturnType pdutxstatus;
    uint8 pduIndex;
    pdutxstatus = E_NOT_OK;
    for(pduIndex=0;pduIndex<ChannelConf->FrNmChannelIdentifiersConfig->FrNmTxPduCount;pduIndex++) {
       if(ChannelConf->FrNmChannelIdentifiersConfig->FrNmTxPduList[pduIndex].FrNmTxConatainsVote == STD_ON) {
           *dataPduId  = ChannelConf->FrNmChannelIdentifiersConfig->FrNmTxPduList[pduIndex].FrIfTxPduId;
           pdutxstatus = E_OK;
        }
    }
   return pdutxstatus;
}


/**
 * @brief- To get the local User data and Id form upper layer transmission.
 * @param ChannelConf - Channel configuration data ptr
 * @param ChannelInternal -Channel Internal local runtime data ptr
 * @param dataPduPtr - Transmission user data pointer
 * @return
 */
Std_ReturnType FrNm_Internal_Get_Userdata_ForTx( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal, const PduInfoType* dataPduPtr ){
    Std_ReturnType usrDataStatus;
    usrDataStatus = E_OK;
    if(ChannelInternal->setUserDataEnable == FALSE){
#if (FRNM_USER_DATA_ENABLED == STD_ON)
#if (FRNM_COM_USER_DATA_SUPPORT == STD_ON)
        PduInfoType userData;
        if(NULL!=ChannelConf->FrNmChannelIdentifiersConfig->FrNmUserDataConfig){
            userData.SduDataPtr = &ChannelInternal->FrNmTxDataPdu[FRNM_INTERNAL_GET_USER_DATA_OFFSET];
            userData.SduLength = dataPduPtr->SduLength - FRNM_INTERNAL_GET_USER_DATA_OFFSET;
            /* IMPROVMENT: Add Det error when transmit is failing */
            usrDataStatus = PduR_FrNmTriggerTransmit(ChannelConf->FrNmChannelIdentifiersConfig->FrNmUserDataConfig->FrNmUserDataTxPduId,&userData);
#if ((FRNM_PNC_EIRA_CALC_ENABLED == STD_ON) && (FRNM_PNC_COUNT > 0))
            /* @req FRNM405 FRNM412 FRNM413 */
            if (ChannelConf->FrNmChannelIdentifiersConfig->FrNmPnEnabled == STD_ON) {
                FrNm_Internal_ProcessTxPdu(userData.SduDataPtr);
            }
#endif
        }

#endif
#endif
    }else{
        SchM_Enter_FrNm_EA_0();
        ChannelInternal->setUserDataEnable = FALSE;
        SchM_Exit_FrNm_EA_0();
    }


return usrDataStatus;
}

/**
 * @brief - To get the local pud data and Id for transmission.
 * @param ChannelConf - Channel configuration data ptr
 * @param ChannelInternal - Channel Internal runtime data ptr
 * @param dataPduPtr - Transmission pdu data pointer
 * @param dataPduId - FRIF pdu Id for transmission
 * @return
 */
Std_ReturnType FrNm_Internal_Get_pdudataPtr_ForTx( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal, PduInfoType* dataPduPtr, PduIdType* dataPduId ){

    Std_ReturnType pdutxstatus;
    uint8 pduIndex;
    pdutxstatus = E_NOT_OK;
    for(pduIndex=0;pduIndex<ChannelConf->FrNmChannelIdentifiersConfig->FrNmTxPduCount;pduIndex++) {
        if(ChannelConf->FrNmChannelIdentifiersConfig->FrNmTxPduList[pduIndex].FrNmTxContainsData == STD_ON) {
            dataPduPtr->SduDataPtr = &ChannelInternal->FrNmTxDataPdu[VOTE_INDEX];
            dataPduPtr->SduLength = ChannelConf->FrNmChannelIdentifiersConfig->FrNmTxPduList[pduIndex].FrNmTxPduLength;
            *dataPduId = ChannelConf->FrNmChannelIdentifiersConfig->FrNmTxPduList[pduIndex].FrIfTxPduId;
            pdutxstatus = E_OK;
        }
    }
 return pdutxstatus;
}





/**
 * @brief Tick timeout for Tx confirmation
 * @param ChannelConf - Channel configuration data ptr
 * @param ChannelInternal -Channel Internal local runtime data ptr
 * @return void
 */
/* TxTimeout Processing */
void FrNm_Internal_TickTxTimeout( const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal ){

    if (0 != ChannelInternal->MessageTimeoutTimeLeft ) {
        if (ChannelConf->FrNmTimingConfig->FrNmMainFunctionPeriod >= ChannelInternal->MessageTimeoutTimeLeft ) {
            /** @req FRNM035 */
            Nm_TxTimeoutException(ChannelConf->FrNmChannelIdentifiersConfig->FrNmComMNetworkHandleRef);
            ChannelInternal->MessageTimeoutTimeLeft = ChannelConf->FrNmTimingConfig->FrNmMsgTimeoutTime ;
        } else {
            ChannelInternal->MessageTimeoutTimeLeft -= ChannelConf->FrNmTimingConfig->FrNmMainFunctionPeriod;
        }
    }

}

/**
 * @brief Actions/Operation after entered in ReadySleepState
 * @param ChannelConf - Channel configuration data ptr
 * @param ChannelInternal -Channel Internal local runtime data ptr
 */
void FrNm_Internal_ReadySleepState_Handling(const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal){
    /* @req FRNM132 */ /* @req FRNM101 */
    if(ChannelInternal->readySleepCntLeft>=1){
        if(ChannelInternal->FrNm_RepeatMessage == TRUE){
            /* @req FRNM130 */
            FrNm_Internal_ReadySleep_to_RepeatMessage(ChannelConf, ChannelInternal);
        }else{
            /* @req FRNM131 */
            if(ChannelInternal->FrNm_NetworkRequested == TRUE){
                FrNm_Internal_ReadySleep_to_NormalOperation(ChannelConf, ChannelInternal);
            }else {
                ChannelInternal->readySleepCntLeft--;
            }
        }
    }else if(ChannelInternal->readySleepCntLeft<1){
        /* @req FRNM129 */ /* @req FRNM133 */
        FrNm_Internal_ReadySleep_to_BusSleep(ChannelConf, ChannelInternal);
#if (FRNM_CONTROL_BIT_VECTOR_ENABLED == STD_OFF)/* @req FRNM324 */
        ChannelInternal->FrNmTxDataPdu[CBV_INDEX] = 0x00; /* @req FRNM3829 */
#else
        ChannelInternal->FrNmTxDataPdu[CBV_INDEX] &= (uint8)~FRNM_CBV_ACTIVE_WAKEUP; /* @req FRNM298 */
#endif
    }else{
        /* Nothing to do */
    }
}

/**
 * @brief Actions/Operations for reception when hardware vector data is enable
 * @param HwvectorInfoPtr - Hardware vector pointer information
 * @param ChannelConf - Channel configuration data ptr
 * @param ChannelInternal -Channel Internal local runtime data ptr
 */
#if (FRNM_HW_VOTE_ENABLE == STD_ON)
void FrNm_Internal_Hardware_VectorData_Handling( const uint8* HwvectorInfoPtr, const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal ) {

    if(ChannelInternal->State == NM_STATE_READY_SLEEP) {
       if((HwvectorInfoPtr[VOTE_INDEX] & VOTE_VALUE) == VOTE_VALUE) {
          /* @req FRNM128 */ /* @req FRNM314 */
          ChannelInternal->readySleepCntLeft = ChannelConf->FrNmTimingConfig->FrNmReadySleepCnt;
#if (FRNM_CYCLE_COUNTER_EMULATION ==STD_ON) /* @req FRNM378 */
          ChannelInternal->syncLossTimerLeft = ChannelConf->FrNmTimingConfig->FrNmSyncLossTimer;
#endif
        }
    }
}
#endif

#else /* Passive mode */
/**
 * @brief Actions/Operation after entered in ReadySleepState
 * @param ChannelConf - Channel configuration data ptr
 * @param ChannelInternal -Channel Internal local runtime data ptr
 */
void FrNm_Internal_ReadySleepState_Handling(const FrNm_ChannelInfoType* ChannelConf, FrNm_Internal_ChannelType* ChannelInternal){
    ChannelInternal->readySleepCntLeft--;
    if(ChannelInternal->readySleepCntLeft<1){
        /* @req FRNM129 */ /* @req FRNM133 */
        FrNm_Internal_ReadySleep_to_BusSleep(ChannelConf, ChannelInternal);
    }

}
#endif


/**
 * @brief To reset vote and pdu data in startup error, global time error and NM state machine
 * @param ChannelInternal -Channel Internal local runtime data ptr
 */
void FrNm_Internal_Resetvotedata(FrNm_Internal_ChannelType* ChannelInternal){
#if (FRNM_PASSIVE_MODE_ENABLED == STD_OFF)
/* @req FRNM137 *//* @req FRNM308 */ /* @req FRNM126 */
    ChannelInternal->FrNmTxVotePdu &=(uint8)~VOTE_VALUE;
    ChannelInternal->FrNmTxDataPdu[VOTE_INDEX] &=(uint8)~VOTE_VALUE;
#else
    (void)ChannelInternal; /*lint !e920 unused pointer*/
#endif
}
#ifdef HOST_TEST
/**
 * * @brief - To compare the unit test cases result with internal data
 */
void GetFrNmChannelRunTimeData( const NetworkHandleType NetworkHandle , boolean* repeatMessage, boolean* networkRequested) {
     uint8 channelIndex;
    channelIndex = FrNm_ConfigPtr->FrNmChannelLookups[NetworkHandle];
    const FrNm_Internal_ChannelType* ChannelInternal = &FrNm_Internal.FrNmChannels[channelIndex];

    *repeatMessage =  ChannelInternal->FrNm_RepeatMessage;
    *networkRequested = ChannelInternal->FrNm_NetworkRequested;
}
#endif



