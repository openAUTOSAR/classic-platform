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
/** @req LINIF376 */ /* Error codes detected by LINIF */
/** @req LINIF270 */ /* The detection of production code errors can not be switched off.*/
/** @req LINIF308 *//* The LIN Interface shall not use a Node Model */
/** @req LINIF359.partially *//* Among Mandatory interface LINTP interface not supported   */
/** !req LINIF360 */ /* None of the optional interface mentioned in autosar are supported */
/** !req LINIF374 */ /* Post builf not supported */
/** @req LINIF376 *//** @reqLINIF579 */ /* DET error codes  */
/** @req LINIF458 */ /* The LIN Interface shall not report a header error to the upper layers when the return code of the LIN Drivermodule’s function Lin_GetStatus is LIN_TX_HEADER_ERROR*/
/** @req LINIF469 */ /* Types Included from different modules */
/** @req LINIF129 */ /* Accessing Functions provided by LIN Driver */
/** @req LINIF237 */ /* The power management of the LIN 2.1 specification shall not be applicable to the LIN Interface */
/** @req LINIF241 */ /* Code structure files.*/
/** @req LINIF248 */ /* The LIN Interface shall support the behavior of the master in the LIN 2.1 specification.*/
/** @req LINIF249 */ /* The LIN Interface shall realize the master behavior so that existing slaves can be reused.*/
/** @req LINIF375 */ /* The LIN Interface shall not make any consistency check of the configuration in run-time in production software. It may be done if the development error detection is enabled. */
/** @req LINIF472 */ /*The LIN Interface shall not use reserved frames.*/
/** @req LINIF579 *//* Additional errors that are detected because of specific implementation */
/** @req LINIF261 *//* The delay between processing two frames shall be a multiple of the LIN Interface time-base, This is checked in LinIf.chk */

#include "LinIf.h"      /** @req LINIF242 */
#include "LinIf_Types.h"
#include "LinIf_Cbk.h"
#include "Lin.h"            /** @req LINIF434 */
#include "LinSM_Cbk.h"       /** @req LINIF556 */
#include "PduR_LinIf.h"     /** @req LINIF497 */
#include "SchM_LinIf.h"
/** @req LINIF498 */
#if (LINIF_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif

/*lint -emacro(904,VALIDATE,VALIDATE_W_RV)*/ /*904 PC-Lint exception to MISRA 14.7 (validate DET macros)*/

/** @req LINIF269*/ /** @req LINIF271*/
/* Development error macros. */
#if ( LINIF_DEV_ERROR_DETECT == STD_ON )
#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          (void)Det_ReportError(LINIF_MODULE_ID,0,_api,_err); \
          return; \
        }

#define VALIDATE_W_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          (void)Det_ReportError(LINIF_MODULE_ID,0,_api,_err); \
          return (_rv); \
        }

#define DET_REPORT_ERROR(_api,_err) (void)Det_ReportError(LINIF_MODULE_ID, 0, _api, _err);

#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_W_RV(_exp,_api,_err,_rv )
#define DET_REPORT_ERROR(_api,_err)
#endif

static const LinIf_ConfigType *LinIf_ConfigPtr;

static LinIf_StatusType LinIfStatus = LINIF_UNINIT;
static LinIf_StatusType LinIfChannelStatus[LINIF_CONTROLLER_CNT];

static const LinIf_ScheduleTableType *currentSchedule[LINIF_CONTROLLER_CNT];
static const LinIf_ScheduleTableType *previousSchedule[LINIF_CONTROLLER_CNT];
static uint16 currentIndex[LINIF_CONTROLLER_CNT];
static uint16 previousIndex[LINIF_CONTROLLER_CNT];
static uint16 currentDelayInTicks[LINIF_CONTROLLER_CNT];

static boolean newScheduleRequest[LINIF_CONTROLLER_CNT];
static uint8 chSleepCmdAttmpt[LINIF_CONTROLLER_CNT]; /* This is required for tx a sleep frame */

static LinIf_SchHandleType newSchedule[LINIF_CONTROLLER_CNT];

/* Internal function declarations  */
static inline void handelSchedTransmission(uint8 chIndex);


/** @req LINIF198 */
void LinIf_Init( const LinIf_ConfigType* ConfigPtr )
{/*lint !e9046 LinIf_Init and LINIF_INIT are in the AutoSAR specification. */
    /** @req LINIF373 */
    LinIf_ConfigPtr = ConfigPtr;

    /** @req LINIF486 */
    VALIDATE( (LinIf_ConfigPtr!=NULL), LINIF_INIT_SERVICE_ID, LINIF_E_PARAMETER_POINTER );
    /** @req LINIF562 */
    VALIDATE((LinIfStatus != LINIF_INIT), LINIF_INIT_SERVICE_ID, LINIF_E_ALREADY_INITIALIZED);

    uint8 i;
    for (i=0;i<LINIF_CONTROLLER_CNT;i++)
    {
        /** @req LINIF507 */
        LinIfChannelStatus[i] = LinIf_ConfigPtr->LinIfChannel[i].LinIfStartupState;
        if (LINIF_CHANNEL_SLEEP == LinIfChannelStatus[i]) {
            (void)Lin_GoToSleepInternal(LinIf_ConfigPtr->LinIfChannel[i].LinIfLinChannelId);//It is expected that Lin Driver goes to sleep
        }
        /** @req LINIF233 */
        currentSchedule[i] = LinIf_ConfigPtr->LinIfChannel[i].LinIfScheduleTable;
        previousSchedule[i]= LinIf_ConfigPtr->LinIfChannel[i].LinIfScheduleTable;
        currentIndex[i] = 0;
        previousIndex[i] = 0;
        currentDelayInTicks[i] = 0;
        newScheduleRequest[i] = FALSE;
        chSleepCmdAttmpt[i] = 0;
    }
    /** @req LINIF381 */
    LinIfStatus = LINIF_INIT;
}

void LinIf_DeInit()
{
    LinIfStatus = LINIF_UNINIT;
}
/** @req LINIF201 */
Std_ReturnType LinIf_Transmit(PduIdType LinTxPduId,const PduInfoType* PduInfoPtr)
{
    (void)LinTxPduId;
    //lint -estring(920,pointer)  /* cast to void */
    (void)PduInfoPtr;
    //lint +estring(920,pointer)  /* cast to void */
    /* Sporadic frames not supported in this release */
    /*printf("LinIf_Transmit received request. Id: %d, Data: %d\n", LinTxPduId, *(PduInfoPtr->SduDataPtr));*/
    return E_OK;
}

/** @req LINIF202 */
/*lint -e{578} FALSE POSITIVE Declaration of Schedule should hide Schedule(void)*/
Std_ReturnType LinIf_ScheduleRequest(NetworkHandleType Channel,LinIf_SchHandleType Schedule)
{
    /** @req LINIF535 */
    VALIDATE_W_RV( (LinIfStatus != LINIF_UNINIT), LINIF_SCHEDULEREQUEST_SERVICE_ID, LINIF_E_UNINIT, E_NOT_OK);
    /** @req LINIF563 */
    VALIDATE_W_RV( ((Channel < LinIf_ConfigPtr->LinIfChannelMapSize) && (LinIf_ConfigPtr->LinIfChannelMap[Channel] < LINIF_CONTROLLER_CNT)), LINIF_SCHEDULEREQUEST_SERVICE_ID, LINIF_E_NONEXISTENT_CHANNEL, E_NOT_OK);
    /** @req LINIF567 */
    VALIDATE_W_RV( (Schedule < LINIF_SCH_CNT), LINIF_SCHEDULEREQUEST_SERVICE_ID, LINIF_E_SCHEDULE_REQUEST_ERROR, E_NOT_OK);
    /* Get the corresponding LinIf index */
    NetworkHandleType LinIfIndex = LinIf_ConfigPtr->LinIfChannelMap[Channel];
    /** @req LINIF467 */
    VALIDATE_W_RV( ( (LinIfChannelStatus[LinIfIndex] != LINIF_CHANNEL_SLEEP) && (LinIfChannelStatus[LinIfIndex] != LINIF_CHANNEL_SLEEP_TRANS) ), LINIF_SCHEDULEREQUEST_SERVICE_ID, LINIF_E_SCHEDULE_REQUEST_ERROR, E_NOT_OK);

    newScheduleRequest[LinIfIndex] = TRUE;
    /** @req LINIF389 */
    newSchedule[LinIfIndex] = Schedule;
    return E_OK;
}

/** @req LINIF204 */
Std_ReturnType LinIf_GotoSleep(NetworkHandleType Channel)
{
    /** @req LINIF535 */
    VALIDATE_W_RV( (LinIfStatus != LINIF_UNINIT), LINIF_GOTOSLEEP_SERVICE_ID, LINIF_E_UNINIT, E_NOT_OK);
    /** @req LINIF564 */
    VALIDATE_W_RV( ((Channel < LinIf_ConfigPtr->LinIfChannelMapSize) && (LinIf_ConfigPtr->LinIfChannelMap[Channel] < LINIF_CONTROLLER_CNT)), LINIF_GOTOSLEEP_SERVICE_ID, LINIF_E_NONEXISTENT_CHANNEL, E_NOT_OK);
    /* Get the corresponding LinIf index */
    NetworkHandleType LinIfIndex = LinIf_ConfigPtr->LinIfChannelMap[Channel];
    if (LinIfChannelStatus[LinIfIndex] == LINIF_CHANNEL_OPERATIONAL) {
        /** @req LINIF488 */
        LinIfChannelStatus[LinIfIndex] = LINIF_CHANNEL_SLEEP_TRANS;
        chSleepCmdAttmpt[LinIfIndex] = 0;
    }
    /** !req LINIF597 */ /* is not implemented since channel is already in sleep and calling Lin_GoToSleepInternal doesn't make difference */
    /** @req LINIF113 */
    return E_OK;
}

/** @req LINIF205 */
Std_ReturnType LinIf_WakeUp(NetworkHandleType Channel)
{
    uint8 *Lin_SduPtr;

    /** @req LINIF535 */
    VALIDATE_W_RV( (LinIfStatus != LINIF_UNINIT), LINIF_WAKEUP_SERVICE_ID, LINIF_E_UNINIT, E_NOT_OK);
    /** @req LINIF565 */
    VALIDATE_W_RV( ((Channel < LinIf_ConfigPtr->LinIfChannelMapSize) && (LinIf_ConfigPtr->LinIfChannelMap[Channel] < LINIF_CONTROLLER_CNT)), LINIF_WAKEUP_SERVICE_ID, LINIF_E_NONEXISTENT_CHANNEL, E_NOT_OK);
    NetworkHandleType LinChannelIndex = LinIf_ConfigPtr->LinIfChannelMap[Channel];
    switch(LinIfChannelStatus[LinChannelIndex]) {
        case LINIF_CHANNEL_SLEEP:
            /** @req LINIF296 */
            (void)Lin_Wakeup(LinIf_ConfigPtr->LinIfChannel[LinChannelIndex].LinIfLinChannelId); //Driver always returns E_OK
            currentIndex[LinChannelIndex] = 0;
            currentDelayInTicks[LinChannelIndex] = 0;
            break;

        case LINIF_CHANNEL_OPERATIONAL:
            /** @req LINIF670 */
            LinSM_WakeUp_Confirmation(Channel, TRUE);
            break;

        case LINIF_CHANNEL_SLEEP_TRANS:
            /** @req LINIF459 */
            if(0 == chSleepCmdAttmpt[LinChannelIndex]){
                LinIfChannelStatus[LinChannelIndex] = LINIF_CHANNEL_OPERATIONAL;
            } else if (Lin_GetStatus(LinIf_ConfigPtr->LinIfChannel[LinChannelIndex].LinIfLinChannelId, &Lin_SduPtr) == LIN_CH_SLEEP) {
                LinIfChannelStatus[LinChannelIndex] = LINIF_CHANNEL_SLEEP;
                /** @req LINIF460 */
                (void)Lin_Wakeup(LinIf_ConfigPtr->LinIfChannel[LinChannelIndex].LinIfLinChannelId); //Driver always returns E_OK
                currentIndex[LinChannelIndex] = 0;
                currentDelayInTicks[LinChannelIndex] = 0;
                chSleepCmdAttmpt[LinChannelIndex] = 0;
            } else {
                LinIfChannelStatus[LinChannelIndex] = LINIF_CHANNEL_OPERATIONAL;
                LinSM_WakeUp_Confirmation(LinIf_ConfigPtr->LinIfChannel[LinChannelIndex].LinIfComMHandle, TRUE);
                chSleepCmdAttmpt[LinChannelIndex] = 0;
            }
            break;
        default:
            break;
    }

    /* LINIF432: The function LinIf_WakeUp shall do nothing and return E_OK when the */
    /* referenced channel is not in the sleep state. */
    /** @req LINIF432 */
    return E_OK;

}
/** @req LINIF039*//** @req LINIF287*//** @req LINIF384*/
void LinIf_MainFunction(void)
{
    SchM_Enter_LinIf_EA_0();
    uint8 chIndex;
    uint8 *Lin_SduPtr;
    if (LinIfStatus == LINIF_UNINIT) {
        SchM_Exit_LinIf_EA_0();
        /*lint -e{904} Return statement is necessary to avoid multiple if loops and hence increase readability in case of argument check */
        return;
    }
    /** @req LINIF473 */ /** @req LINIF290 *//** @req LINIF386 */
    for(chIndex = 0; chIndex < LINIF_CONTROLLER_CNT; chIndex++) {
            /* Normal scheduling */
            if(currentDelayInTicks[chIndex] > 0){
                /* Not time for sending yet */
                currentDelayInTicks[chIndex]--;
                continue;
            }
        /* Check if there are any pending sleep transitions */
        if (LinIfChannelStatus[chIndex] == LINIF_CHANNEL_SLEEP_TRANS) {
            if (Lin_GetStatus(LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfLinChannelId, &Lin_SduPtr) != LIN_CH_SLEEP) {
                if (0==chSleepCmdAttmpt[chIndex]) {
                    /** @req LINIF453 */
                    (void)Lin_GoToSleep(LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfLinChannelId); /* Driver always returns E_OK */
                    chSleepCmdAttmpt[chIndex]++;
                }
                else if(chSleepCmdAttmpt[chIndex] <= LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfSleepTransitionCycle) {
                    chSleepCmdAttmpt[chIndex]++;

                } else {
                    /** @req LINIF454  */
                    /* The delay of sleep mode frame is assumed to be time_base arbitarily since ASR is not specific */
                    LinIfChannelStatus[chIndex] = LINIF_CHANNEL_OPERATIONAL;
                    chSleepCmdAttmpt[chIndex] = 0;
                    /** @req LINIF558 *//** @req LINIF521 */
                    LinSM_GotoSleep_Confirmation(LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfComMHandle, FALSE);
                }
            } else {
                /** @req LINIF455 */
                LinIfChannelStatus[chIndex] = LINIF_CHANNEL_SLEEP;
                /** @req LINIF557 */
                LinSM_GotoSleep_Confirmation(LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfComMHandle, TRUE);
                /* Set NULL schedule at sleep */
                /** @req LINIF444 *//** @req LINIF293 *//** @req LINIF043*/
                currentIndex[chIndex] = 0;
                currentDelayInTicks[chIndex] = 0;
                currentSchedule[chIndex] = LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfScheduleTable;
            }
            continue;
        }
        /* Check if there are any wakeup transitions */
        if (LinIfChannelStatus[chIndex] == LINIF_CHANNEL_SLEEP){
            if (Lin_GetStatus(LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfLinChannelId, &Lin_SduPtr) != LIN_CH_SLEEP) {
                /** @req LINIF478 */
                LinIfChannelStatus[chIndex] = LINIF_CHANNEL_OPERATIONAL;
                /** @req LINIF496 */ /** @req LINIF522 */
                LinSM_WakeUp_Confirmation(LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfComMHandle, TRUE);
            }
        }
        /** @req LINIF053 *//** @req LINIF189 *//** @req LINIF030 *//* by virtue of continue statement in the for loop where currentDelayInTicks is decremented this req LINIF030 is taken care*/
        if(LinIfChannelStatus[chIndex] == LINIF_CHANNEL_OPERATIONAL) {
            /* Check if NULL schedule is present otherwise check status of last sent */
            if( (currentSchedule[chIndex] != NULL) && (currentSchedule[chIndex]->LinIfEntry != NULL)){ /*lint !e9032 Comparison with NULL is ok */
                const LinIfEntryType *ptrEntry = &currentSchedule[chIndex]->LinIfEntry[currentIndex[chIndex]];
                const LinIf_FrameType *ptrFrame = &LinIf_ConfigPtr->LinIfFrameConfig[ptrEntry->LinIfFrameRef];
                /* Handle received and sent frames */
                if(ptrFrame->LinIfPduDirection == LinIfRxPdu){
                    Lin_StatusType linSts = Lin_GetStatus(LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfLinChannelId, &Lin_SduPtr);
                    if(linSts == LIN_RX_OK){
                        PduInfoType outgoingPdu;
                        outgoingPdu.SduDataPtr = Lin_SduPtr;
                        outgoingPdu.SduLength = ptrFrame->LinIfLength;
                        /** @req LINIF289 *//** @req LINIF033 */ /** @req LINIF530 */
                        PduR_LinIfRxIndication(ptrFrame->LinIfTxTargetPduId,&outgoingPdu);
                     }else {/* RX_ERROR or BUSY */
                         /** @req LINIF254 *//** @req LINIF466*/
                         DET_REPORT_ERROR(LINIF_MAINFUNCTION_SERVICE_ID,LINIF_E_RESPONSE);
                     }
                } else if(ptrFrame->LinIfPduDirection == LinIfTxPdu){
                    Lin_StatusType status = Lin_GetStatus(LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfLinChannelId, &Lin_SduPtr);
                    if(status == LIN_TX_OK){
                        /** @req LINIF289 *//** @req LINIF128 *//** @req LINIF529*/
                        PduR_LinIfTxConfirmation(ptrFrame->LinIfTxTargetPduId);
                    }else{/* TX_ERROR or BUSY */
                        /** @req LINIF036 */ /** @req LINIF465*//** @req LINIF466*/
                        DET_REPORT_ERROR(LINIF_MAINFUNCTION_SERVICE_ID,LINIF_E_RESPONSE);
                    }
                }else{
                    DET_REPORT_ERROR(LINIF_MAINFUNCTION_SERVICE_ID,LINIF_E_UNEXPECTED_EXECUTION);
                }
                /* Update index after getting status of last frame */
                currentIndex[chIndex] = (currentIndex[chIndex] + 1) % currentSchedule[chIndex]->LinIfNofEntries;
                /** @req LINIF485 */ /** @req LINIF397 */
                if((currentIndex[chIndex] == 0)&&(currentSchedule[chIndex]->LinIfRunMode == RUN_ONCE)&&(newScheduleRequest[chIndex] == FALSE)){
                    currentSchedule[chIndex] = previousSchedule[chIndex];
                    if(previousSchedule[chIndex]->LinIfResumePosition == CONTINUE_AT_IT_POINT){
                        currentIndex[chIndex] = previousIndex[chIndex];
                    }else{
                        currentIndex[chIndex] = 0;
                    }
                    LinSM_ScheduleRequest_Confirmation(LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfComMHandle,currentSchedule[chIndex]->LinIfScheduleTableIndex); /** Informing SM about Schedule table switching*/
                }
            }
            handelSchedTransmission(chIndex);
        }
    }
    SchM_Exit_LinIf_EA_0();
}

static inline void handelSchedTransmission(uint8 chIndex)
{
    uint8 buf[8];

    /* Set new schedule if ordered */
    if(newScheduleRequest[chIndex] == TRUE){
        /** @req LINIF028 */
        if(currentSchedule[chIndex]->LinIfRunMode == RUN_CONTINUOUS){
            if(LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfScheduleTable[newSchedule[chIndex]].LinIfRunMode == RUN_ONCE)
            {
                previousSchedule[chIndex] =  currentSchedule[chIndex];
                previousIndex[chIndex] = currentIndex[chIndex];
            }
            currentSchedule[chIndex] = &LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfScheduleTable[newSchedule[chIndex]];
            currentIndex[chIndex] = 0;
            newScheduleRequest[chIndex]=FALSE;
            /** @req LINIF495 *//** @req LINIF520 */
            LinSM_ScheduleRequest_Confirmation(LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfComMHandle,currentSchedule[chIndex]->LinIfScheduleTableIndex);
        }else{
            /** @req LINIF393 */
            if(currentIndex[chIndex] == 0){
                currentSchedule[chIndex] = &LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfScheduleTable[newSchedule[chIndex]];
                currentIndex[chIndex] = 0;
                newScheduleRequest[chIndex]=FALSE;
                /** @req LINIF495 *//** @req LINIF520 */
                LinSM_ScheduleRequest_Confirmation(LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfComMHandle,currentSchedule[chIndex]->LinIfScheduleTableIndex);
            }
        }
    }

    /* Handle new transmissions */
    if( (currentSchedule[chIndex] != NULL) && (currentSchedule[chIndex]->LinIfEntry != NULL_PTR)){
        Lin_PduType PduInfo;
        const LinIfEntryType *ptrEntry = &currentSchedule[chIndex]->LinIfEntry[currentIndex[chIndex]];
        const LinIf_FrameType *ptrFrame = &LinIf_ConfigPtr->LinIfFrameConfig[ptrEntry->LinIfFrameRef];
        /** @req LINIF286 LINIF287 */
        /* Only UNCONDITIONAL frames is supported in first version */
        if (ptrFrame->LinIfFrameType == UNCONDITIONAL){
            /* SendHeader */
            if(ptrFrame->LinIfChecksumType==ENHANCED){
                PduInfo.Cs = LIN_ENHANCED_CS;
            }else{
                PduInfo.Cs = LIN_CLASSIC_CS;
            }
            PduInfo.Pid = ptrFrame->LinIfPid;
            PduInfo.SduPtr = buf; /* Data will be added in PduR_LinIfTriggerTransmit */
            PduInfo.Dl = (Lin_FrameDIType)ptrFrame->LinIfLength;
            if(ptrFrame->LinIfPduDirection == LinIfTxPdu){
                PduInfo.Drc = LIN_MASTER_RESPONSE;
            }else{
                PduInfo.Drc = LIN_SLAVE_RESPONSE;
            }


            /* Maybe send response also */
            if(ptrFrame->LinIfPduDirection == LinIfTxPdu){
                if(ptrFrame->LinIfFixedFrameSdu != NULL){
                    PduInfo.SduPtr = ptrFrame->LinIfFixedFrameSdu;
                    /** @req LINIF224*/
                    (void)Lin_SendFrame(LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfLinChannelId,  &PduInfo);//Driver always returns E_OK
                }else{
                    PduInfoType outgoingPdu;
                    outgoingPdu.SduDataPtr = PduInfo.SduPtr;
                    outgoingPdu.SduLength = (PduLengthType)PduInfo.Dl;
                    /* TX */
                    /** @req LINIF289 */ /** @req LINIF528 *//** @req LINIF225 */
                    (void)PduR_LinIfTriggerTransmit(ptrFrame->LinIfTxTargetPduId, &outgoingPdu);
                    /** @req LINIF224*//** @req LINIF419*//** @req LINIF226 */
                    (void)Lin_SendFrame(LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfLinChannelId,  &PduInfo);//Driver always returns E_OK
                }
            }
            else {
                    /* RX */
                (void)Lin_SendFrame(LinIf_ConfigPtr->LinIfChannel[chIndex].LinIfLinChannelId,  &PduInfo);//Driver always returns E_OK
            }
        }

        /* Set new delay */
        uint16 temp = ptrEntry->LinIfDelay / LinIf_ConfigPtr->LinIfTimeBase ;
        currentDelayInTicks[chIndex] =  (temp > 0)? (temp - 1): 0; //LinIfDelay can be zero for the first frame on the table
    }
}


#ifdef HOST_TEST
LinIf_StatusType* readInternal_LinIfChnlStatus(void);

LinIf_StatusType* readInternal_LinIfChnlStatus(void){

    return LinIfChannelStatus ;

}
#endif





