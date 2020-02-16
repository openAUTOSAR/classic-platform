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








#include "Std_Types.h"
#include "Lin.h"
#include "LinIf.h"
#include "LinSM.h"
#include "LinSM_Cbk.h"
#include "LinIf_Types.h"
#if defined(USE_COM)
#include "Com.h"
#endif
#include "ComM_Types.h"
#include "ComStack_Types.h"
#include "Com_Types.h"
#include "ComM_BusSm.h"
#include "Det.h"
#if defined(USE_BSWM)
#include "BswM_LinSM.h"
#endif
#include "Mcu.h"

/*==================[macros]==================================================*/

/* Development error macros. */
#if ( LINSM_DEV_ERROR_DETECT == STD_ON )
#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_LINSM,0,_api,_err); \
          return; \
        }

#define VALIDATE_W_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_LINSM,0,_api,_err); \
          return (_rv); \
        }
#endif
#if defined(USE_BSWM)
#define BSWM_LINSM_CURRENTSCHEDULE(_ch) \
    BswM_LinSM_CurrentSchedule(_ch,LinSMSchTablCurr[_ch]); \

#define BSWM_LINSM_CURRENTSTATE(_ch,_state) \
    BswM_LinSM_CurrentState(_ch,_state); \

#else
#define BSWM_LINSM_CURRENTSCHEDULE(_ch)
#define BSWM_LINSM_CURRENTSTATE(_ch,_state)
#endif

/*==================[internal data]===========================================*/

/* Timer values for every LinIf channel. These timers are used to monitor the
 * confirmation of a specific command after calling LinIf_GotoSleep,
 * LinIf_Wakeup or LinIf_ScheduleRequest. The arrays are indexed by LinIf
 * channel Id.
 *
 * See Specification of LIN State Manager, R4.0 Rev 3,
 * 7.1.8 Timeout of requests. */
/** @req LINSM175 */
static uint16 ScheduleRequestTimer [LINIF_CONTROLLER_CNT];
static uint16 GoToSleepTimer       [LINIF_CONTROLLER_CNT];
static uint16 WakeUpTimer          [LINIF_CONTROLLER_CNT];

static void DecrementTimer(uint16 *timer);

/* LinSM module state */
static LinSM_StatusType LinSMStatus = LINSM_UNINIT;

/* Channel states. The array is indexed by LinSM channel Id. */
static LinSM_StatusType LinSMChannelStatus[LINIF_CONTROLLER_CNT];
static LinIf_SchHandleType LinSMSchTablCurr[LINIF_CONTROLLER_CNT], LinSMSchTablNew[LINIF_CONTROLLER_CNT];


/*==================[Implementation of LinSM interface]=======================*/


/*------------------[Initialization]------------------------------------------*/


void LinSM_Init(const LinSM_ConfigType* ConfigPtr)
{
	uint8 i;

	/** @req LINSM217 LINSM218 */
    VALIDATE( (ConfigPtr!=NULL), LINSM_INIT_SERVICE_ID, LINSM_E_PARAMETER_POINTER );

	for (i=0; i<LINIF_CONTROLLER_CNT; i++)
	{
	    /** @req LINSM160 */
		LinSMChannelStatus[i] = LINSM_NO_COMMUNICATION;
		/** @req LINSM0216 */
		LinSMSchTablCurr[i]= 0; //NULL Schedule
		LinSMSchTablNew[i]=0;
		ScheduleRequestTimer[i] = 0;
		GoToSleepTimer[i] = 0;
		WakeUpTimer[i] = 0;
	}
	LinSMStatus = LINSM_INIT;
}

/*------------------[Deinitialization]----------------------------------------*/


void LinSM_DeInit()
{
	LinSMStatus = LINSM_UNINIT;
}

/*------------------[Schedule Request]----------------------------------------*/


Std_ReturnType LinSM_ScheduleRequest(NetworkHandleType channel,LinIf_SchHandleType schedule)
{
    Std_ReturnType rv;

    /** @req LINSM116 */
	VALIDATE_W_RV( (LinSMStatus != LINSM_UNINIT), LINSM_SCHEDULE_REQUEST_SERVICE_ID, LINSM_E_UNINIT, E_NOT_OK);
	/** @req LINSM114 */
	VALIDATE_W_RV( (channel < LINIF_CONTROLLER_CNT), LINSM_SCHEDULE_REQUEST_SERVICE_ID, LINSM_E_NONEXISTENT_NETWORK, E_NOT_OK);
	/** @req LINSM115 */
	VALIDATE_W_RV( (schedule < LINIF_SCH_CNT), LINSM_SCHEDULE_REQUEST_SERVICE_ID, LINSM_E_PARAMETER, E_NOT_OK);
    /** @req LINSM163 LINSM211 */
    if (ScheduleRequestTimer[channel] >= LINSM_MAIN_PROCESSING_PERIOD || LINSM_FULL_COMMUNICATION != LinSMChannelStatus[channel]) {
        return E_NOT_OK;
    }
    else {
        /* Set the Schedule Request confirmation timer for the specified channel. */
        /** @req LINSM100 */
        ScheduleRequestTimer[channel] = LinSM_Config.LinSMChannels[channel].LinSMConfirmationTimeout;
        LinSMSchTablNew[channel] = schedule;
        rv = LinIf_ScheduleRequest(channel, schedule);
        if (rv != E_OK) {
            /** @req LINSM0213 */
            BSWM_LINSM_CURRENTSCHEDULE(channel);
            LinSMSchTablNew[channel] = LinSMSchTablCurr[channel];
            ScheduleRequestTimer[channel] = 0;
        }
        return rv;
    }
}

/*------------------[Get current COM mode]------------------------------------*/


Std_ReturnType LinSM_GetCurrentComMode(NetworkHandleType network,ComM_ModeType* mode)
{
    /** @req LINSM182 */
    /* If active state is LINSM_UNINIT the state
     * COMM_NO_COMMUNICATION shall be returned. */
    if (network < LINIF_CONTROLLER_CNT)
    {
        if (LinSMStatus == LINSM_UNINIT && mode != NULL_PTR)
        {
            *mode= COMM_NO_COMMUNICATION;
        }
    }

    /** @req LINSM125 */
    VALIDATE_W_RV( (LinSMStatus != LINSM_UNINIT), LINSM_GET_CURRENT_COM_MODE_SERVICE_ID, LINSM_E_UNINIT, E_NOT_OK);
	/** @req LINSM123 */
	VALIDATE_W_RV( (network < LINIF_CONTROLLER_CNT), LINSM_GET_CURRENT_COM_MODE_SERVICE_ID, LINSM_E_NONEXISTENT_NETWORK, E_NOT_OK);
	/** @req LINSM124 */
	VALIDATE_W_RV( (mode != NULL_PTR), LINSM_GET_CURRENT_COM_MODE_SERVICE_ID, LINSM_E_PARAMETER_POINTER, E_NOT_OK);

	switch (LinSMChannelStatus[network])
	{
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


Std_ReturnType LinSM_RequestComMode(NetworkHandleType network,ComM_ModeType mode)
{
	Std_ReturnType res = E_NOT_OK;
	boolean tempflg = FALSE;

	/** @req LINSM128 */
	VALIDATE_W_RV( (LinSMStatus != LINSM_UNINIT), LINSM_REQUEST_COM_MODE_SERVICE_ID, LINSM_E_UNINIT, E_NOT_OK);
	/** @req LINSM127 */
	VALIDATE_W_RV( (network < LINIF_CONTROLLER_CNT), LINSM_REQUEST_COM_MODE_SERVICE_ID, LINSM_E_NONEXISTENT_NETWORK, E_NOT_OK);
    /** @req LINSM191 */
    VALIDATE_W_RV( ((mode == COMM_NO_COMMUNICATION) || (mode == COMM_SILENT_COMMUNICATION) || (mode == COMM_FULL_COMMUNICATION)), LINSM_REQUEST_COM_MODE_SERVICE_ID, LINSM_E_PARAMETER_POINTER, E_NOT_OK);

    tempflg = ((COMM_NO_COMMUNICATION == mode) && (LINSM_NO_COMMUNICATION == LinSMChannelStatus[network])) || ((COMM_FULL_COMMUNICATION == mode) && (LINSM_FULL_COMMUNICATION == LinSMChannelStatus[network])) ;
    /** @req LINSM0210 LINSM174 */
    if (tempflg || WakeUpTimer[network] >= LINSM_MAIN_PROCESSING_PERIOD ) {
        return res;
    }

    switch(mode)
	{
		case COMM_NO_COMMUNICATION:
		    /* Set the GotoSleep confirmation timer for the specified channel. */
		    /** @req LINSM100 */
		    GoToSleepTimer[network] = LinSM_Config.LinSMChannels[network].LinSMConfirmationTimeout;
			if (E_OK == LinIf_GotoSleep(network)){
			    /** @req LINSM223 */
				LinSMChannelStatus[network] = LINSM_NO_COMMUNICATION;
				res = E_OK;
			}
			else {
			    /** @req LINSM177 */
			    BSWM_LINSM_CURRENTSTATE(network,LINSM_FULL_COM);
			    ComM_ModeType ComMode;
			    ComMode = COMM_FULL_COMMUNICATION;
			    ComM_BusSM_ModeIndication(network, &ComMode);
			    GoToSleepTimer[network] = 0;
			}
			break;
		case COMM_SILENT_COMMUNICATION:
			// Standard say nothing about this case.
		    /** @req LINSM183 */
			break;
		case COMM_FULL_COMMUNICATION:
		    /* Set the Wakeup confirmation timer for the specified channel. */
		    /** @req LINSM100 */
		    WakeUpTimer[network] = LinSM_Config.LinSMChannels[network].LinSMConfirmationTimeout;
		    /** @req LINSM047 */
			if (E_OK == LinIf_WakeUp(network)){
				res = E_OK;
			}
			else
			{
			    /** @req LINSM0202 */
			    BSWM_LINSM_CURRENTSTATE(network,LINSM_NO_COM);
			    ComM_ModeType ComMode;
			    ComMode = COMM_NO_COMMUNICATION;
			    ComM_BusSM_ModeIndication(network, &ComMode);
				WakeUpTimer[network]=0;
			}
			break;
		default:
			break;
	}
	return res;
}

/*------------------[Schedule request confirmation]---------------------------*/


void LinSM_ScheduleRequest_Confirmation(NetworkHandleType channel){

    /** @req LINSM131 */
	VALIDATE( (LinSMStatus != LINSM_UNINIT), LINSM_SCHEDULE_REQUEST_CONF_SERVICE_ID, LINSM_E_UNINIT);
	/** @req LINSM130 */
	VALIDATE( (channel < LINIF_CONTROLLER_CNT), LINSM_SCHEDULE_REQUEST_CONF_SERVICE_ID, LINSM_E_NONEXISTENT_NETWORK);

	if (ScheduleRequestTimer[channel] >= LINSM_MAIN_PROCESSING_PERIOD) {

	    LinSMSchTablCurr[channel] = LinSMSchTablNew[channel];
	    /** @req LINSM206 */
	    BSWM_LINSM_CURRENTSCHEDULE(channel);
	}
	/** @req LINSM154 */
	ScheduleRequestTimer[channel] = 0;

}

/*------------------[Wakeup confirmation]-------------------------------------*/


void LinSM_WakeUp_Confirmation(NetworkHandleType channel,boolean success){
    /** @req LINSM134 */
	VALIDATE( (LinSMStatus != LINSM_UNINIT), LINSM_WAKEUP_CONF_SERVICE_ID, LINSM_E_UNINIT);
	/** @req LINSM133 */
	VALIDATE( (channel < LINIF_CONTROLLER_CNT), LINSM_WAKEUP_CONF_SERVICE_ID, LINSM_E_NONEXISTENT_NETWORK);

    /** @req LINSM172 */
    if(WakeUpTimer[channel] >= LINSM_MAIN_PROCESSING_PERIOD){
        if(TRUE == success)
        {
            ComM_ModeType ComMode = COMM_FULL_COMMUNICATION;
            ComM_BusSM_ModeIndication(channel, &ComMode);
            LinSMChannelStatus[channel] = LINSM_FULL_COMMUNICATION;
            /** @req LINSM192 */
            BSWM_LINSM_CURRENTSTATE(channel,LINSM_FULL_COM);
        }
        else {
            /** @req LINSM0202 */
            BSWM_LINSM_CURRENTSTATE(channel,LINSM_NO_COM);
            ComM_ModeType ComMode;
            ComMode = COMM_NO_COMMUNICATION;
            ComM_BusSM_ModeIndication(channel, &ComMode);
        }
    }

    /** @req LINSM154 */
    WakeUpTimer[channel]=0;
}

/*------------------[GotoSleep confirmation]----------------------------------*/


void LinSM_GotoSleep_Confirmation(NetworkHandleType channel,boolean success){
    /** @req LINSM137 */
	VALIDATE( (LinSMStatus != LINSM_UNINIT), LINSM_GOTO_SLEEP_CONF_SERVICE_ID, LINSM_E_UNINIT);
	/** @req LINSM136 */
	VALIDATE( (channel < LINIF_CONTROLLER_CNT), LINSM_GOTO_SLEEP_CONF_SERVICE_ID, LINSM_E_NONEXISTENT_NETWORK);

    /** @req LINSM172 */
    if(GoToSleepTimer[channel] >= LINSM_MAIN_PROCESSING_PERIOD){
        if(TRUE == success)
        {
            LinSMChannelStatus[channel] = LINSM_NO_COMMUNICATION;
            ComM_ModeType ComMode = COMM_NO_COMMUNICATION;
            ComM_BusSM_ModeIndication(channel, &ComMode);
            /** @req LINSM193 */
            BSWM_LINSM_CURRENTSTATE(channel,LINSM_NO_COM);
        }
        else {
            /** @req LINSM177 */
            BSWM_LINSM_CURRENTSTATE(channel,LINSM_FULL_COM);
            ComM_ModeType ComMode;
            ComMode = COMM_FULL_COMMUNICATION;
            ComM_BusSM_ModeIndication(channel, &ComMode);
        }
    }

    /** @req LINSM154 */
    GoToSleepTimer[channel] = 0;
}

/*==================[Main function]==========================================*/

void LinSM_MainFunction()
{
	uint8 channel;
	ComM_ModeType ComMode;

    /** @req LINSM179 */
    VALIDATE( (LinSMStatus != LINSM_UNINIT), LINSM_MAIN_FUNCTION_SERVICE_ID, LINSM_E_UNINIT);
    /** @req LINSM157 */
	for(channel=0;channel<LINIF_CONTROLLER_CNT;channel++){
		// Check timers

	    if(ScheduleRequestTimer[channel] >= LINSM_MAIN_PROCESSING_PERIOD){

	        DecrementTimer(&ScheduleRequestTimer[channel]);
            if (ScheduleRequestTimer[channel] < LINSM_MAIN_PROCESSING_PERIOD) {
                /** @req LINSM0214 */
                BSWM_LINSM_CURRENTSCHEDULE(channel);
            }
	    }
	    if(GoToSleepTimer[channel] >= LINSM_MAIN_PROCESSING_PERIOD){

	        DecrementTimer(&GoToSleepTimer[channel]);
            if (GoToSleepTimer[channel] < LINSM_MAIN_PROCESSING_PERIOD) {
                ComMode = COMM_FULL_COMMUNICATION;
                ComM_BusSM_ModeIndication(channel, &ComMode);
                /** @req LINSM170 */
                BSWM_LINSM_CURRENTSTATE(channel,LINSM_FULL_COM);
            }
	    }

        if(WakeUpTimer[channel] >= LINSM_MAIN_PROCESSING_PERIOD){

            DecrementTimer(&WakeUpTimer[channel]);
            if (WakeUpTimer[channel] < LINSM_MAIN_PROCESSING_PERIOD) {
                ComMode = COMM_NO_COMMUNICATION;
                ComM_BusSM_ModeIndication(channel, &ComMode);
                /** @req LINSM170 */
                BSWM_LINSM_CURRENTSTATE(channel,LINSM_NO_COM);
            }

        }
	}
}

/** @req LINSM162*/
static void DecrementTimer(uint16 *timer) {
    /** @req LINSM159*/
    *timer = *timer - LINSM_MAIN_PROCESSING_PERIOD;
    /** @req LINSM101*/
    if(*timer < LINSM_MAIN_PROCESSING_PERIOD){
        /** @req LINSM102*/
        Det_ReportError(MODULE_ID_LINSM,0,LINSM_MAIN_FUNCTION_SERVICE_ID,LINSM_E_CONFIRMATION_TIMEOUT);
    }
}

/*==================[end of file]=============================================*/
