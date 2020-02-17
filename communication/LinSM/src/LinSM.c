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

/**@req LINSM001 *//* LIN Interface – LinIf is interfaced with LINSM  */
/**@req LINSM002 *//* The LinSM module shall not use or access the LIN driver or assume information about it any way other than what the LinIf module provides through the function calls to the LinIf module  */
/**@req LINSM054 *//* The detection of development errors is configurable (ON/OFF) at precompile time  */
/**@req LINSM073 *//* The LinSM module shall not make any consistency check of the configuration in run-time inproduction software */
/**@req LINSM105 *//* Communication Manager – ComM is interfaced */
/**@req LINSM138 *//* Optional Interfaced */
/**@req LINSM196 *//* BSW Mode Manager - BswM is interfaced */
/**@req LINSM200 *//* Additional errors that are detected because of specific implementation and/or specific hardware propertiesshall be added in the module’s implementation documentation */
/**@req LINSM219 *//* Types are used by the LinSM module */

#include "Std_Types.h"
#include "Lin.h"
#include "LinIf.h"          /**@req LINSM012 */
#include "LinSM.h"
#include "LinSM_Cbk.h"
#include "LinIf_Types.h"
#if defined(USE_COM)
#include "Com.h"
#endif
#include "ComM_Types.h"
#include "ComStack_Types.h"      /**@req LINSM016 */
#include "Com_Types.h"
#include "ComM_BusSM.h"
#if ( LINSM_DEV_ERROR_DETECT == STD_ON )
#include "Det.h"           /**@req LINSM015 */
#endif
#include "SchM_LinSM.h"
#if defined(USE_BSWM)
/*@req LINSM201 Including BswM_LinSM.h and not BswM.h */
#include "BswM_LinSM.h"
#endif
#include "Mcu.h"
#include "LinSM_ConfigTypes.h"
/*==================[macros]==================================================*/


/*lint -emacro(904,VALIDATE,VALIDATE_W_RV)*/ /*904 PC-Lint exception to MISRA 14.7 (validate DET macros)*/

/**@req LINSM055 */ /**@req LINSM057 *//**@req LINSM086 */

/* Development error macros. */
#if ( LINSM_DEV_ERROR_DETECT == STD_ON )
#define LINSM_DET_REPORT_ERROR(_api, _err)  (void)Det_ReportError(LINSM_MODULE_ID,0,_api,_err)

#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          (void)Det_ReportError(LINSM_MODULE_ID,0,_api,_err); \
          return; \
        }

#define VALIDATE_W_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          (void)Det_ReportError(LINSM_MODULE_ID,0,_api,_err); \
          return (_rv); \
        }
#else
#define LINSM_DET_REPORT_ERROR(_api, _err)
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_W_RV(_exp,_api,_err,_rv )
#endif

#if defined(USE_BSWM)
#define BSWM_LINSM_CURRENTSCHEDULE(_ch, _sch) \
    BswM_LinSM_CurrentSchedule(_ch,_sch); \

#define BSWM_LINSM_CURRENTSTATE(_ch,_state) \
    BswM_LinSM_CurrentState(_ch,_state); \

#else
#define BSWM_LINSM_CURRENTSCHEDULE(_ch, _sch)
#define BSWM_LINSM_CURRENTSTATE(_ch,_state)
#endif

/*==================[internal data]===========================================*/

/* Timer values for every LinIf channel. These timers are used to monitor the
 * confirmation of a specific command after calling LinIf_GotoSleep,
 * LinIf_Wakeup or LinIf_ScheduleRequest. The arrays are indexed by the LinSM
 * channel index (NOT the LinIf channel id).
 *
 * See Specification of LIN State Manager, R4.0 Rev 3,
 * 7.1.8 Timeout of requests. */
/** @req LINSM175 */
static uint32 ScheduleRequestTimer [LINSM_CHANNEL_CNT];
static uint32 GoToSleepTimer       [LINSM_CHANNEL_CNT];
static uint32 WakeUpTimer          [LINSM_CHANNEL_CNT];
static boolean ScheduleRequest_Ok  [LINSM_CHANNEL_CNT];

static void DecrementTimer(uint32 *timer);

/* LinSM module state *//**@req LINSM161 */
static LinSM_StatusType LinSMStatus = LINSM_UNINIT;

/* Channel states. The arrays are indexed by the LinSM
 * channel index. */
static LinSM_StatusType LinSMChannelStatus[LINSM_CHANNEL_CNT];
static LinIf_SchHandleType LinSMSchTablCurr[LINSM_CHANNEL_CNT];
static LinIf_SchHandleType LinSMSchTablNew[LINSM_CHANNEL_CNT];

static const LinSM_ConfigType *LinSMConfigPtr;

/*=========================[Local functions]===========================*/
/**
 * Get the index (in LinSM configuration) for a Lin channel
 * @param channel
 * @param channelIndex
 * @return TRUE: index found, FALSE: invalid Lin channel
 */
static Std_ReturnType GetChannelIndex(NetworkHandleType channel, uint32 *channelIndex)
{
    Std_ReturnType ret = E_NOT_OK;
    for (uint32 i = 0; (i < LinSMConfigPtr->LinSMChannels_Size) && (E_OK != ret); i++) {
        if(channel == LinSMConfigPtr->LinSMChannels[i].LinSMComMNetworkHandleRef) {
            *channelIndex = i;
            ret = E_OK;
        }
    }
    return ret;
}

#if ( LINSM_DEV_ERROR_DETECT == STD_ON )
/**
 * Checks if a schedule is configured for a specific channel
 * @param channelIndex
 * @param schedule
 * @return TRUE: valid schedule, FALSE: schedule not configured for this channel
 */
static boolean CheckScheduleValid(uint32 channelIndex, LinIf_SchHandleType schedule)
{
    boolean isValid = FALSE;
    const LinSM_ScheduleType *schedulePtr = LinSMConfigPtr->LinSMChannels[channelIndex].LinSMSchedules;
    for(uint8 schIndex = 0; (schIndex < LinSMConfigPtr->LinSMChannels[channelIndex].LinSMSchedule_Cnt) && (FALSE == isValid); schIndex++) {
        /* IMPROVEMNT: Is it ok to request the NULL schedule? */
        if((schedule == schedulePtr->LinSMScheduleIndex) || (0 == schedule)) {
            isValid = TRUE;
        }
        schedulePtr++;
    }
    return isValid;
}
#endif
/*==================[Implementation of LinSM interface]=======================*/


/*------------------[Initialization]------------------------------------------*/

/**@req LINSM155 *//**@req LINSM020 */
void LinSM_Init(const LinSM_ConfigType* ConfigPtr)
{
    /**@req LINSM151 *//**@req LINSM166 *//**@req LINSM173 */
    uint32 i;

    VALIDATE( (ConfigPtr!=NULL), LINSM_INIT_SERVICE_ID, LINSM_E_PARAMETER_POINTER );
    LinSMConfigPtr = ConfigPtr;
    for (i = 0; i < LinSMConfigPtr->LinSMChannels_Size; i++)
    {
        /** @req LINSM160 *//**@req LINSM152 */
        LinSMChannelStatus[i] = LINSM_NO_COMMUNICATION;
        /** @req LINSM0216 */
        LinSMSchTablCurr[i]= 0; //NULL Schedule
        LinSMSchTablNew[i]=0;
        ScheduleRequestTimer[i] = 0;
        GoToSleepTimer[i] = 0;
        WakeUpTimer[i] = 0;
        ScheduleRequest_Ok[i] = TRUE;
    }
    LinSMStatus = LINSM_INIT; /** @req LINSM024 *//** @req LINSM025 *//** @req LINSM043 */
}

/*------------------[Deinitialization]----------------------------------------*/


void LinSM_DeInit()
{
    LinSMStatus = LINSM_UNINIT; /** @req LINSM022 */
}

/*------------------[Schedule Request]----------------------------------------*/
/**@req LINSM113*/
Std_ReturnType LinSM_ScheduleRequest(NetworkHandleType channel,LinIf_SchHandleType schedule)
{
    Std_ReturnType rv;
    uint32 channelIndex = 0;

    /** @req LINSM116 *//** @req LINSM032 */
    VALIDATE_W_RV( (LinSMStatus != LINSM_UNINIT), LINSM_SCHEDULE_REQUEST_SERVICE_ID, LINSM_E_UNINIT, E_NOT_OK);
    /** @req LINSM114 */
    rv = GetChannelIndex(channel, &channelIndex);
    VALIDATE_W_RV( (E_OK == rv), LINSM_SCHEDULE_REQUEST_SERVICE_ID, LINSM_E_NONEXISTENT_NETWORK, E_NOT_OK);
    /** @req LINSM115 */

    VALIDATE_W_RV( (TRUE == CheckScheduleValid(channelIndex, schedule)), LINSM_SCHEDULE_REQUEST_SERVICE_ID, LINSM_E_PARAMETER, E_NOT_OK);
    /** @req LINSM163  *//** @req LINSM10211 */
    if ((ScheduleRequestTimer[channelIndex] >= LINSM_MAIN_PROCESSING_PERIOD) || (LINSM_FULL_COMMUNICATION != LinSMChannelStatus[channelIndex])) {
        rv = E_NOT_OK;
    }
    else {
        /* Set the Schedule Request confirmation timer for the specified channel. */
        /** @req LINSM100 */
        ScheduleRequestTimer[channelIndex] = LinSMConfigPtr->LinSMChannels[channelIndex].LinSMConfirmationTimeout;
        LinSMSchTablNew[channelIndex] = schedule;
        /** @req LINSM164 *//**@req LINSM079*//**@req LINSM167*//**@req LINSM168*/
        rv = LinIf_ScheduleRequest(channel, schedule);
        if (rv != E_OK) {
            ScheduleRequest_Ok[channelIndex] = FALSE;
            ScheduleRequestTimer[channelIndex] = 0;
        }
    }
    return rv;
}

/*------------------[Get current COM mode]------------------------------------*/

/** @req LINSM021 *//** @req LINSM122 */
Std_ReturnType LinSM_GetCurrentComMode(NetworkHandleType network,ComM_ModeType* mode)
{
    Std_ReturnType ret;
    uint32 channelIndex = 0;
    /** @req LINSM182 */
    /* If active state is LINSM_UNINIT the state
     * COMM_NO_COMMUNICATION shall be returned. */
    if ((LinSMStatus == LINSM_UNINIT) && (mode != NULL_PTR)) {
        *mode= COMM_NO_COMMUNICATION;
    }

    /** @req LINSM125 */
    VALIDATE_W_RV( (LinSMStatus != LINSM_UNINIT), LINSM_GET_CURRENT_COM_MODE_SERVICE_ID, LINSM_E_UNINIT, E_NOT_OK);
    /** @req LINSM123 */
    ret = GetChannelIndex(network, &channelIndex);
    VALIDATE_W_RV( (E_OK == ret), LINSM_GET_CURRENT_COM_MODE_SERVICE_ID, LINSM_E_NONEXISTENT_NETWORK, E_NOT_OK);
    /** @req LINSM124 */
    VALIDATE_W_RV( (mode != NULL_PTR), LINSM_GET_CURRENT_COM_MODE_SERVICE_ID, LINSM_E_PARAMETER_POINTER, E_NOT_OK);

    switch (LinSMChannelStatus[channelIndex]) {
        case LINSM_FULL_COMMUNICATION:
            /** @req LINSM181 */
            *mode= COMM_FULL_COMMUNICATION;
            break;
        default:
            /** @req LINSM180 */
            *mode= COMM_NO_COMMUNICATION;
            break;
    }
    return E_OK;
}

/*------------------[Request new COM mode]------------------------------------*/

/** @req LINSM126 */
Std_ReturnType LinSM_RequestComMode(NetworkHandleType network,ComM_ModeType mode)
{
    Std_ReturnType res;
    uint32 channelIndex = 0;
    boolean tempflg;
    boolean timeflg;

    timeflg = FALSE;

    /** @req LINSM128 */
    VALIDATE_W_RV( (LinSMStatus != LINSM_UNINIT), LINSM_REQUEST_COM_MODE_SERVICE_ID, LINSM_E_UNINIT, E_NOT_OK);
    /** @req LINSM127 */
    res = GetChannelIndex(network, &channelIndex);
    VALIDATE_W_RV( (E_OK == res), LINSM_REQUEST_COM_MODE_SERVICE_ID, LINSM_E_NONEXISTENT_NETWORK, E_NOT_OK);
    /** @req LINSM191 */
    VALIDATE_W_RV( ((mode == COMM_NO_COMMUNICATION) || (mode == COMM_SILENT_COMMUNICATION) || (mode == COMM_FULL_COMMUNICATION)), LINSM_REQUEST_COM_MODE_SERVICE_ID, LINSM_E_PARAMETER_POINTER, E_NOT_OK);

    res = E_NOT_OK;
    /** @req LINSM035  */
    tempflg = (boolean)(((COMM_NO_COMMUNICATION == mode) && (LINSM_NO_COMMUNICATION == LinSMChannelStatus[channelIndex])) ||
                        ((COMM_FULL_COMMUNICATION == mode) && (LINSM_FULL_COMMUNICATION == LinSMChannelStatus[channelIndex])));

    if(0 != LinSMConfigPtr->LinSMChannels[channelIndex].LinSMConfirmationTimeout){
        timeflg = ((WakeUpTimer[channelIndex] >= LINSM_MAIN_PROCESSING_PERIOD) || (GoToSleepTimer[channelIndex] >= LINSM_MAIN_PROCESSING_PERIOD)) ? TRUE : FALSE;
    }
    /** @req LINSM0210  *//** @req LINSM174 *//** @req LINSM044 */
    if ((TRUE == tempflg) || (TRUE == timeflg)) {
        res = E_NOT_OK;
    } else {

        switch(mode)
        {   /** @req LINSM178 */ /** @req LINSM10209*/
            case COMM_NO_COMMUNICATION:

                /* Set the GotoSleep confirmation timer for the specified channel. */
                /** @req LINSM100 */
                GoToSleepTimer[channelIndex] = LinSMConfigPtr->LinSMChannels[channelIndex].LinSMConfirmationTimeout;
                /** @req LINSM036 *//** @req LINSM10208 */
                if (E_OK == LinIf_GotoSleep(network)){
                    res = E_OK;
                }
                else {
                    /** @req LINSM177 */
                    BSWM_LINSM_CURRENTSTATE(LinSMConfigPtr->LinSMChannels[channelIndex].LinSMComMNetworkHandleRef, LINSM_FULL_COM);
                    ComM_ModeType ComMode;
                    ComMode = COMM_FULL_COMMUNICATION;
                    ComM_BusSM_ModeIndication(LinSMConfigPtr->LinSMChannels[channelIndex].LinSMComMNetworkHandleRef, &ComMode);
                    GoToSleepTimer[channelIndex] = 0;
                }
                break;

            case COMM_SILENT_COMMUNICATION:
                // Standard say nothing about this case.
                /** @req LINSM183 */
                break;
            case COMM_FULL_COMMUNICATION:
                /* Set the Wakeup confirmation timer for the specified channel. */
                /** @req LINSM100 */
                WakeUpTimer[channelIndex] = LinSMConfigPtr->LinSMChannels[channelIndex].LinSMConfirmationTimeout;
                /** @req LINSM047 *//** @req LINSM164 *//** @req LINSM176 */
                if (E_OK == LinIf_WakeUp(network)){
                    res = E_OK;
                }
                else
                {
                    /** @req LINSM0202 */
                    BSWM_LINSM_CURRENTSTATE(LinSMConfigPtr->LinSMChannels[channelIndex].LinSMComMNetworkHandleRef,LINSM_NO_COM);
                    ComM_ModeType ComMode;
                    ComMode = COMM_NO_COMMUNICATION;
                    ComM_BusSM_ModeIndication(LinSMConfigPtr->LinSMChannels[channelIndex].LinSMComMNetworkHandleRef, &ComMode);
                    WakeUpTimer[channelIndex] = 0;
                }
                break;
            default:
                break;
        }
    }
    return res;
}

/*------------------[Schedule request confirmation]---------------------------*/

/** @req LINSM129 */
void LinSM_ScheduleRequest_Confirmation(NetworkHandleType channel,LinIf_SchHandleType schedule)
{
    Std_ReturnType ret;
    uint32 channelIndex = 0;
    /** @req LINSM131 */
    VALIDATE( (LinSMStatus != LINSM_UNINIT), LINSM_SCHEDULE_REQUEST_CONF_SERVICE_ID, LINSM_E_UNINIT);
    /** @req LINSM130 */
    ret = GetChannelIndex(channel, &channelIndex);
    VALIDATE( (E_OK == ret), LINSM_SCHEDULE_REQUEST_CONF_SERVICE_ID, LINSM_E_NONEXISTENT_NETWORK);

    if(LinSMSchTablNew[channelIndex] == schedule){
        if ((ScheduleRequestTimer[channelIndex] >= LINSM_MAIN_PROCESSING_PERIOD)||(0 == LinSMConfigPtr->LinSMChannels[channelIndex].LinSMConfirmationTimeout)){
            LinSMSchTablCurr[channelIndex] = LinSMSchTablNew[channelIndex];
            /** @req LINSM206 */
            BSWM_LINSM_CURRENTSCHEDULE(LinSMConfigPtr->LinSMChannels[channelIndex].LinSMComMNetworkHandleRef, LinSMSchTablCurr[channelIndex]);
        }
    }else{
        /**@req LINSM207 */
        BSWM_LINSM_CURRENTSCHEDULE(LinSMConfigPtr->LinSMChannels[channelIndex].LinSMComMNetworkHandleRef, schedule);
    }
    /** @req LINSM154 */
    ScheduleRequestTimer[channelIndex] = 0;

}

/*------------------[Wakeup confirmation]-------------------------------------*/

/** @req LINSM132 */
void LinSM_WakeUp_Confirmation(NetworkHandleType channel,boolean success)
{
    Std_ReturnType ret;
    uint32 channelIndex = 0;
    /** @req LINSM134 */
    VALIDATE( (LinSMStatus != LINSM_UNINIT), LINSM_WAKEUP_CONF_SERVICE_ID, LINSM_E_UNINIT);
    /** @req LINSM133 */
    ret = GetChannelIndex(channel, &channelIndex);
    VALIDATE( (E_OK == ret), LINSM_WAKEUP_CONF_SERVICE_ID, LINSM_E_NONEXISTENT_NETWORK);

    /** @req LINSM172 */
    if((WakeUpTimer[channelIndex] >= LINSM_MAIN_PROCESSING_PERIOD)||(0 == LinSMConfigPtr->LinSMChannels[channelIndex].LinSMConfirmationTimeout)){
        if(TRUE == success)
        {
            ComM_ModeType ComMode = COMM_FULL_COMMUNICATION;
            ComM_BusSM_ModeIndication(LinSMConfigPtr->LinSMChannels[channelIndex].LinSMComMNetworkHandleRef, &ComMode);
            /**@req LINSM049 */
            LinSMChannelStatus[channelIndex] = LINSM_FULL_COMMUNICATION;
            /** @req LINSM192 */
            BSWM_LINSM_CURRENTSTATE(LinSMConfigPtr->LinSMChannels[channelIndex].LinSMComMNetworkHandleRef,LINSM_FULL_COM);
        }
        else {
            LinSMChannelStatus[channelIndex] = LINSM_NO_COMMUNICATION;
            /** @req LINSM0202 */
            BSWM_LINSM_CURRENTSTATE(LinSMConfigPtr->LinSMChannels[channelIndex].LinSMComMNetworkHandleRef,LINSM_NO_COM);
            ComM_ModeType ComMode;
            ComMode = COMM_NO_COMMUNICATION;
            ComM_BusSM_ModeIndication(LinSMConfigPtr->LinSMChannels[channelIndex].LinSMComMNetworkHandleRef, &ComMode);
        }
    }

    /** @req LINSM154 */
    WakeUpTimer[channelIndex] = 0;
}

/*------------------[GotoSleep confirmation]----------------------------------*/

/** @req LINSM135 */
void LinSM_GotoSleep_Confirmation(NetworkHandleType channel,boolean success)
{
    Std_ReturnType ret;
    uint32 channelIndex = 0;
    /** @req LINSM137 */
    VALIDATE( (LinSMStatus != LINSM_UNINIT), LINSM_GOTO_SLEEP_CONF_SERVICE_ID, LINSM_E_UNINIT);
    /** @req LINSM136 */
    ret = GetChannelIndex(channel, &channelIndex);
    VALIDATE( (E_OK == ret), LINSM_GOTO_SLEEP_CONF_SERVICE_ID, LINSM_E_NONEXISTENT_NETWORK);

    /** @req LINSM172 */
    if((GoToSleepTimer[channelIndex] >= LINSM_MAIN_PROCESSING_PERIOD)||(0 == LinSMConfigPtr->LinSMChannels[channelIndex].LinSMConfirmationTimeout)){
        if(TRUE == success)
        {   /** @req LINSM045 *//** @req LINSM028 */
            LinSMChannelStatus[channelIndex] = LINSM_NO_COMMUNICATION;
            ComM_ModeType ComMode = COMM_NO_COMMUNICATION;
            ComM_BusSM_ModeIndication(LinSMConfigPtr->LinSMChannels[channelIndex].LinSMComMNetworkHandleRef, &ComMode);
            /** @req LINSM193 */
            BSWM_LINSM_CURRENTSTATE(LinSMConfigPtr->LinSMChannels[channelIndex].LinSMComMNetworkHandleRef,LINSM_NO_COM);
        }
        else {
            LinSMChannelStatus[channelIndex] = LINSM_FULL_COMMUNICATION;
            /** @req LINSM177 */
            BSWM_LINSM_CURRENTSTATE(LinSMConfigPtr->LinSMChannels[channelIndex].LinSMComMNetworkHandleRef,LINSM_FULL_COM);
            ComM_ModeType ComMode;
            /** @req LINSM046 */
            ComMode = COMM_FULL_COMMUNICATION;
            ComM_BusSM_ModeIndication(LinSMConfigPtr->LinSMChannels[channelIndex].LinSMComMNetworkHandleRef, &ComMode);
        }
    }
    /** @req LINSM154 */
    GoToSleepTimer[channelIndex] = 0;
}

/*==================[Main function]==========================================*/
/**@req LINSM156 */
void LinSM_MainFunction(void)
{
    uint32 channelIndex;
    ComM_ModeType ComMode;

    /** @req LINSM179 */
    VALIDATE( (LinSMStatus != LINSM_UNINIT), LINSM_MAIN_FUNCTION_SERVICE_ID, LINSM_E_UNINIT);

    SchM_Enter_LinSM_EA_0();
    /** @req LINSM157 *//** @req LINSM019 */
    for(channelIndex = 0; channelIndex < LinSMConfigPtr->LinSMChannels_Size; channelIndex++){
        /**@req LINSM103 */
        if(0 != LinSMConfigPtr->LinSMChannels[channelIndex].LinSMConfirmationTimeout)
        {
            if(FALSE == ScheduleRequest_Ok[channelIndex]){
            /** @req LINSM0213 */
                BSWM_LINSM_CURRENTSCHEDULE(LinSMConfigPtr->LinSMChannels[channelIndex].LinSMComMNetworkHandleRef, LinSMSchTablCurr[channelIndex]);
            }
            if(ScheduleRequestTimer[channelIndex] >= LINSM_MAIN_PROCESSING_PERIOD){

                DecrementTimer(&ScheduleRequestTimer[channelIndex]);
                if (ScheduleRequestTimer[channelIndex] < LINSM_MAIN_PROCESSING_PERIOD) {
                    /** @req LINSM0214 */
                    BSWM_LINSM_CURRENTSCHEDULE(LinSMConfigPtr->LinSMChannels[channelIndex].LinSMComMNetworkHandleRef, LinSMSchTablCurr[channelIndex]);
                    /** @req LINSM102 */
                    LINSM_DET_REPORT_ERROR(LINSM_MAIN_FUNCTION_SERVICE_ID, LINSM_E_CONFIRMATION_TIMEOUT);
                    }
            }
            if(GoToSleepTimer[channelIndex] >= LINSM_MAIN_PROCESSING_PERIOD){

                DecrementTimer(&GoToSleepTimer[channelIndex]);
                if (GoToSleepTimer[channelIndex] < LINSM_MAIN_PROCESSING_PERIOD) {
                    ComMode = COMM_FULL_COMMUNICATION;
                    /** @req LINSM170 *//** @req LINSM033 */
                    ComM_BusSM_ModeIndication(LinSMConfigPtr->LinSMChannels[channelIndex].LinSMComMNetworkHandleRef, &ComMode);
                    /** @req LINSM215 */
                    BSWM_LINSM_CURRENTSTATE(LinSMConfigPtr->LinSMChannels[channelIndex].LinSMComMNetworkHandleRef, LINSM_FULL_COM);
                    /** @req LINSM102 */
                    LINSM_DET_REPORT_ERROR(LINSM_MAIN_FUNCTION_SERVICE_ID, LINSM_E_CONFIRMATION_TIMEOUT);
                }
            }

            if(WakeUpTimer[channelIndex] >= LINSM_MAIN_PROCESSING_PERIOD){

                DecrementTimer(&WakeUpTimer[channelIndex]);
                if (WakeUpTimer[channelIndex] < LINSM_MAIN_PROCESSING_PERIOD) {
                    ComMode = COMM_NO_COMMUNICATION;
                    /** @req LINSM170 */ /** @req LINSM027 */
                    ComM_BusSM_ModeIndication(LinSMConfigPtr->LinSMChannels[channelIndex].LinSMComMNetworkHandleRef, &ComMode);
                    /** @req LINSM215 */
                    BSWM_LINSM_CURRENTSTATE(LinSMConfigPtr->LinSMChannels[channelIndex].LinSMComMNetworkHandleRef,LINSM_NO_COM);
                    /** @req LINSM102 */
                    LINSM_DET_REPORT_ERROR(LINSM_MAIN_FUNCTION_SERVICE_ID, LINSM_E_CONFIRMATION_TIMEOUT);
                }

            }

        }
    }
    SchM_Exit_LinSM_EA_0();
}

/** @req LINSM162*/
static void DecrementTimer(uint32 *timer) {
    /** @req LINSM159*/
    *timer = *timer - LINSM_MAIN_PROCESSING_PERIOD;
    /** @req LINSM101*/
    if(*timer < LINSM_MAIN_PROCESSING_PERIOD){
        /** @req LINSM102*/
        (void)Det_ReportError(LINSM_MODULE_ID,0,LINSM_MAIN_FUNCTION_SERVICE_ID,LINSM_E_CONFIRMATION_TIMEOUT);
    }
}

/*==================[end of file]=============================================*/


#ifdef HOST_TEST
LinSM_StatusType* readInternal_LinSmChnlStatus(void);

LinSM_StatusType* readInternal_LinSmChnlStatus(void){

    return LinSMChannelStatus;
}

LinIf_SchHandleType* readInternal_LinSmSchedule(void);
LinIf_SchHandleType* readInternal_LinSmSchedule(void){
    return LinSMSchTablCurr;
}

uint32* readInternal_ScheduleTimer(void);
uint32* readInternal_ScheduleTimer(void){
    return ScheduleRequestTimer;
}

uint32* readInternal_GoToSleepTimer(void);
uint32* readInternal_GoToSleepTimer(void){
    return GoToSleepTimer;
}

uint32* readInternal_WakeUpTimer(void);
uint32* readInternal_WakeUpTimer(void){
    return WakeUpTimer;
}


#endif


