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








#ifndef LINSM_H_
#define LINSM_H_

#define LINSM_MODULE_ID            MODULE_ID_LINSM
#define LINSM_VENDOR_ID            VENDOR_ID_ARCCORE

#define LINSM_SW_MAJOR_VERSION    1
#define LINSM_SW_MINOR_VERSION    0
#define LINSM_SW_PATCH_VERSION    0
#define LINSM_AR_MAJOR_VERSION    3
#define LINSM_AR_MINOR_VERSION    1
#define LINSM_AR_PATCH_VERSION    5

#include "ComStack_Types.h"
#include "Std_Types.h"
#include "LinSM_Cfg.h"
#include "Com_Types.h"
#if defined(USE_COM)
#include "Com.h"
#endif
#include "ComM_Types.h"
#include "LinIf.h"

#define LINSM_INIT_SERVICE_ID 0x01
#define LINSM_REQUEST_COM_MODE_SERVICE_ID 0x10
#define LINSM_GET_CURRENT_COM_MODE_SERVICE_ID 0x11
#define LINSM_SCHEDULE_REQUEST_SERVICE_ID 0x12
#define LINSM_GOTO_SLEEP_CONF_SERVICE_ID 0x20
#define LINSM_WAKEUP_CONF_SERVICE_ID 0x21
#define LINSM_SCHEDULE_REQUEST_CONF_SERVICE_ID 0x22

/* --- Error codes --- */
#define LINSM_E_UNINIT                  0x00
#define LINSM_E_ALREADY_INITIALIZED		0x10
#define LINSM_E_NOXEXISTENT_CHANNEL		0x20
#define LINSM_E_PARAMETER				0x30
#define LINSM_E_PARAMETER_POINTER		0x40
#define LINSM_E_NOT_IN_RUN_SCHEDULE		0x50
#define LINSM_E_CONFIRMATION_TIMEOUT	0x60 //Assigned by DEM

typedef enum {
	LINSM_UNINIT,
	LINSM_INIT,
	LINSM_NO_COM,
	LINSM_FULL_COM,
	LINSM_RUN_SCHEDULE,
	LINSM_GOTO_SLEEP,
}LinSM_StatusType;

#if (LINSM_VERSION_INFO_API == STD_ON)
void LinSM_GetVersionInfo( Std_VersionInfoType *versionInfo );
#define LinSM_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,LINSM)
#endif

void LinSM_Init(const void* ConfigPtr);
void LinSM_DeInit();

Std_ReturnType LinSM_ScheduleRequest(NetworkHandleType channel,LinIf_SchHandleType schedule);

Std_ReturnType LinSM_GetCurrentComMode(NetworkHandleType network,ComM_ModeType* mode);

Std_ReturnType LinSM_RequestComMode(NetworkHandleType network,ComM_ModeType mode);


#endif
