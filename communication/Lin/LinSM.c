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








#include "Std_Types.h"
#include "LinIf.h"
#include "LinSM.h"
#include "LinSM_Cfg.h"
#include "Lin_Cfg.h"
#include "LinSM_Cbk.h"
#include "LinIf_Types.h"
#if defined(USE_COM)
#include "Com.h"
#endif
#include "ComM_Types.h"
#include "ComStack_Types.h"
#include "Com_Types.h"
#include "ComM_BusSm.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#include "Mcu.h"
#include <stdlib.h>
#include <string.h>

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
#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_W_RV(_exp,_api,_err,_rv )
#endif

#define INVALID_COM_GROUP 0xFFFF

static uint8 ScheduleRequestTimer[LINIF_CONTROLLER_CNT];
static uint8 GoToSleepTimer[LINIF_CONTROLLER_CNT];
static uint8 WakeUpTimer[LINIF_CONTROLLER_CNT];
static uint32 comRxPduIdGrp[LINIF_CONTROLLER_CNT], comTxPduIdGrp[LINIF_CONTROLLER_CNT];

static LinSM_StatusType LinSMStatus = LINSM_UNINIT;
static LinSM_StatusType LinSMChannelStatus[LINIF_CONTROLLER_CNT];

void LinSM_Init(const void* ConfigPtr)
{
    (void)ConfigPtr;
	uint8 i;

	for (i=0; i<LINIF_CONTROLLER_CNT; i++)
	{
		LinSMChannelStatus[i] = LINSM_NO_COM;
		ScheduleRequestTimer[i] = 0;
		GoToSleepTimer[i] = 0;
		WakeUpTimer[i] = 0;
		comRxPduIdGrp[i]= INVALID_COM_GROUP;
		comTxPduIdGrp[i]= INVALID_COM_GROUP;
	}

	LinIf_Init(0);
	LinSMStatus = LINSM_INIT;
}

void LinSM_DeInit()
{
	LinIf_DeInit();
	LinSMStatus = LINSM_UNINIT;
}

Std_ReturnType LinSM_ScheduleRequest(NetworkHandleType channel,LinIf_SchHandleType schedule)
{
	VALIDATE_W_RV( (LinSMStatus != LINSM_UNINIT), LINSM_SCHEDULE_REQUEST_SERVICE_ID, LINSM_E_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (channel < LINIF_CONTROLLER_CNT), LINSM_SCHEDULE_REQUEST_SERVICE_ID, LINSM_E_NOXEXISTENT_CHANNEL, E_NOT_OK);
	VALIDATE_W_RV( (LinSMChannelStatus[channel] == LINSM_RUN_SCHEDULE), LINSM_SCHEDULE_REQUEST_SERVICE_ID, LINSM_E_NOT_IN_RUN_SCHEDULE, E_NOT_OK);
	VALIDATE_W_RV( (schedule < LINIF_SCH_CNT), LINSM_SCHEDULE_REQUEST_SERVICE_ID, LINSM_E_PARAMETER, E_NOT_OK);

	ScheduleRequestTimer[channel]=LINSM_SCHEDULE_REQUEST_TIMEOUT;
	return LinIf_ScheduleRequest(channel, schedule);
}

Std_ReturnType LinSM_GetCurrentComMode(NetworkHandleType network,ComM_ModeType* mode)
{
	VALIDATE_W_RV( (LinSMStatus != LINSM_UNINIT), LINSM_GET_CURRENT_COM_MODE_SERVICE_ID, LINSM_E_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (network < LINIF_CONTROLLER_CNT), LINSM_GET_CURRENT_COM_MODE_SERVICE_ID, LINSM_E_NOXEXISTENT_CHANNEL, E_NOT_OK);
	VALIDATE_W_RV( (mode != NULL), LINSM_GET_CURRENT_COM_MODE_SERVICE_ID, LINSM_E_PARAMETER_POINTER, E_NOT_OK);

	switch (LinSMChannelStatus[network])
	{
	case LINSM_RUN_SCHEDULE:
		*mode= COMM_FULL_COMMUNICATION;
		break;
	default:
		*mode= COMM_NO_COMMUNICATION;
		break;
	}
	return E_OK;
}

Std_ReturnType LinSM_RequestComMode(NetworkHandleType network,ComM_ModeType mode)
{
	Std_ReturnType res = E_NOT_OK;
	VALIDATE_W_RV( (LinSMStatus != LINSM_UNINIT), LINSM_REQUEST_COM_MODE_SERVICE_ID, LINSM_E_UNINIT, E_NOT_OK);
	VALIDATE_W_RV( (network < LINIF_CONTROLLER_CNT), LINSM_REQUEST_COM_MODE_SERVICE_ID, LINSM_E_NOXEXISTENT_CHANNEL, E_NOT_OK);

	switch(mode)
	{
		case COMM_NO_COMMUNICATION:
			if (E_OK == LinIf_GotoSleep(network)){
				LinSMChannelStatus[network] = LINSM_GOTO_SLEEP;
				GoToSleepTimer[network]=LINSM_GOTO_SLEEP_TIMEOUT;
				res = E_OK;
			}
			break;
		case COMM_SILENT_COMMUNICATION:
			// Standard say nothing about this case.
			break;
		case COMM_FULL_COMMUNICATION:
			WakeUpTimer[network]=LINSM_WAKEUP_TIMEOUT; //should be done here since some implementations will confirm immediatly
			if (E_OK == LinIf_WakeUp(network)){
				res = E_OK;
			}
			else
			{
				WakeUpTimer[network]=0;
			}
			break;
		default:
			break;
	}
	return res;
}

void LinSM_ScheduleRequest_Confirmation(NetworkHandleType channel){
	VALIDATE( (LinSMStatus != LINSM_UNINIT), LINSM_SCHEDULE_REQUEST_CONF_SERVICE_ID, LINSM_E_UNINIT);
	VALIDATE( (channel < LINIF_CONTROLLER_CNT), LINSM_SCHEDULE_REQUEST_CONF_SERVICE_ID, LINSM_E_NOXEXISTENT_CHANNEL);

	if(ScheduleRequestTimer[channel]!=0){
		if (INVALID_COM_GROUP != comRxPduIdGrp[channel]) {
			Com_IpduGroupStop(comRxPduIdGrp[channel]);
			Com_IpduGroupStop(comTxPduIdGrp[channel]);
		}
		comRxPduIdGrp[channel] = LinSMChannelType[channel].LinSMSchedule->LinSMRxPduGroupRef;
		Com_IpduGroupStart(comRxPduIdGrp[channel],FALSE);
		comTxPduIdGrp[channel] = LinSMChannelType[channel].LinSMSchedule->LinSMTxPduGroupRef;
		Com_IpduGroupStart(comTxPduIdGrp[channel],FALSE);

		ScheduleRequestTimer[channel]=0;
	}
}

void LinSM_WakeUp_Confirmation(NetworkHandleType channel,boolean success){
	VALIDATE( (LinSMStatus != LINSM_UNINIT), LINSM_WAKEUP_CONF_SERVICE_ID, LINSM_E_UNINIT);
	VALIDATE( (channel < LINIF_CONTROLLER_CNT), LINSM_WAKEUP_CONF_SERVICE_ID, LINSM_E_NOXEXISTENT_CHANNEL);

	if(TRUE == success)
	{
		if(WakeUpTimer[channel]!=0){
			WakeUpTimer[channel]=0;
			ComM_BusSM_ModeIndication(channel,COMM_FULL_COMMUNICATION);
			LinSMChannelStatus[channel] = LINSM_RUN_SCHEDULE;
		}
	}
}

void LinSM_GotoSleep_Confirmation(NetworkHandleType channel,boolean success){
	VALIDATE( (LinSMStatus != LINSM_UNINIT), LINSM_GOTO_SLEEP_CONF_SERVICE_ID, LINSM_E_UNINIT);
	VALIDATE( (channel < LINIF_CONTROLLER_CNT), LINSM_GOTO_SLEEP_CONF_SERVICE_ID, LINSM_E_NOXEXISTENT_CHANNEL);

	if(TRUE == success)
	{
		if(GoToSleepTimer[channel]!=0){
			GoToSleepTimer[channel]=0;
			LinSMChannelStatus[channel] = LINSM_NO_COM;
			ComM_BusSM_ModeIndication(channel,COMM_NO_COMMUNICATION);
		}
	}
}

void LinSM_TimerTick()
{
	uint8 channel;
	for(channel=0;channel<LINIF_CONTROLLER_CNT;channel++){
		// Check timers
		if(ScheduleRequestTimer[channel] > 0){
			ScheduleRequestTimer[channel]--;
			if(ScheduleRequestTimer[channel]==0){
				Det_ReportError(MODULE_ID_LINSM,0,LINSM_REQUEST_COM_MODE_SERVICE_ID,LINSM_E_CONFIRMATION_TIMEOUT);
				LinSMChannelStatus[channel] = LINSM_NO_COM;
			}
		}
		if(GoToSleepTimer[channel] > 0){
			GoToSleepTimer[channel]--;
			if(GoToSleepTimer[channel]==0){
				Det_ReportError(MODULE_ID_LINSM,0,LINSM_REQUEST_COM_MODE_SERVICE_ID,LINSM_E_CONFIRMATION_TIMEOUT);
				LinSMChannelStatus[channel] = LINSM_NO_COM;
			}
		}
		if(WakeUpTimer[channel] > 0){
			WakeUpTimer[channel]--;
			if(WakeUpTimer[channel]==0){
				Det_ReportError(MODULE_ID_LINSM,0,LINSM_REQUEST_COM_MODE_SERVICE_ID,LINSM_E_CONFIRMATION_TIMEOUT);
				LinSMChannelStatus[channel] = LINSM_NO_COM;
			}
		}
	}
}
