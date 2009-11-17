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








#ifndef LINIF_H_
#define LINIF_H_

#define LINIF_SW_MAJOR_VERSION    1
#define LINIF_SW_MINOR_VERSION    0
#define LINIF_SW_PATCH_VERSION    0
#define LINIF_AR_MAJOR_VERSION    2
#define LINIF_AR_MINOR_VERSION    0
#define LINIF_AR_PATCH_VERSION    1

#include "Std_Types.h"
#include "LinIf_Cfg.h"
#include "LinIf_Types.h"
#include "ComStack_Types.h"
#include "ComM_Types.h"




/* --- Service IDs --- */
#define LINIF_INIT_SERVICE_ID               0x00
#define LINIF_GETVERSIONINFO_SERVICE_ID     0x03
#define LINIF_SCHEDULEREQUEST_SERVICE_ID    0x05
#define LINIF_GOTOSLEEP_SERVICE_ID    		0x06
#define LINIF_WAKEUP_SERVICE_ID     		0x07
#define LINIF_MAINFUNCTION_SERVICE_ID       0x80

/* --- Error codes --- */
#define LINIF_E_UNINIT 0x00
#define LINIF_E_ALREADY_INITIALIZED 0x10
#define LINIF_E_NONEXISTENT_CHANNEL 0x20
#define LINIF_E_PARAMETER 0x30
#define LINIF_E_PARAMETER_POINTER 0x40
#define LINIF_E_SCHEDULE_OVERFLOW 0x50
#define LINIF_E_SCHEDULE_REQUEST_ERROR 0x51
#define LINIF_E_RESPONSE 0x52 //Assigned
#define LINIF_E_NC_NO_RESPONSE 0x53 //Assigned by DEM
#define LINIF_E_CHANNEL_X_SLAVE_Y 0x54 //Assigned by DEM

#if (LINIF_VERSION_INFO_API == STD_ON)
void LinIf_GetVersionInfo( Std_VersionInfoType *versionInfo );
#define LinIf_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,LINIF)
#endif

void LinIf_Init( const void* ConfigPtr );

void LinIf_DeInit();

Std_ReturnType LinIf_Transmit(PduIdType LinTxPduId,const PduInfoType* PduInfoPtr);

Std_ReturnType LinIf_ScheduleRequest(NetworkHandleType Channel,LinIf_SchHandleType Schedule);

Std_ReturnType LinIf_GotoSleep(NetworkHandleType Channel);

Std_ReturnType LinIf_WakeUp(NetworkHandleType Channel);

void LinIf_MainFunction();

#endif
