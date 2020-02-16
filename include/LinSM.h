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








#ifndef LINSM_H_
#define LINSM_H_

#define LINSM_MODULE_ID            MODULE_ID_LINSM
#define LINSM_VENDOR_ID            VENDOR_ID_ARCCORE

#define LINSM_SW_MAJOR_VERSION              2
#define LINSM_SW_MINOR_VERSION              0
#define LINSM_SW_PATCH_VERSION              0
#define LINSM_AR_RELEASE_MAJOR_VERSION      4
#define LINSM_AR_RELEASE_MINOR_VERSION      0
#define LINSM_AR_RELEASE_PATCH_VERSION      3

#include "ComStack_Types.h"
#include "Std_Types.h"
#include "LinSM_Cfg.h"
#include "Com_Types.h"
#if defined(USE_COM)
#include "Com.h"
#endif
#include "ComM_Types.h"
#include "LinIf.h"

#define LINSM_INIT_SERVICE_ID                   0x01
#define LINSM_GET_CURRENT_COM_MODE_SERVICE_ID   0x11
#define LINSM_REQUEST_COM_MODE_SERVICE_ID       0x12
#define LINSM_SCHEDULE_REQUEST_SERVICE_ID       0x10
#define LINSM_SCHEDULE_REQUEST_CONF_SERVICE_ID  0x20
#define LINSM_WAKEUP_CONF_SERVICE_ID            0x21
#define LINSM_GOTO_SLEEP_CONF_SERVICE_ID        0x22
#define LINSM_MAIN_FUNCTION_SERVICE_ID          0x30


/* --- Error codes --- */
#define LINSM_E_UNINIT                  0x00
#define LINSM_E_ALREADY_INITIALIZED		0x10
#define LINSM_E_NONEXISTENT_NETWORK		0x20
#define LINSM_E_PARAMETER				0x30
#define LINSM_E_PARAMETER_POINTER		0x40
#define LINSM_E_CONFIRMATION_TIMEOUT	0x60 //Assigned by DEM

typedef enum {
	LINSM_UNINIT,
	LINSM_INIT,
	LINSM_NO_COMMUNICATION,
	LINSM_FULL_COMMUNICATION
}LinSM_StatusType;


typedef enum {
    /** @req LINSM0220 */
    LINSM_NO_COM = 1,
    LINSM_FULL_COM =2
}LinSM_ModeType;

#if (LINSM_VERSION_INFO_API == STD_ON)
void LinSM_GetVersionInfo( Std_VersionInfoType *versionInfo );
#define LinSM_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,LINSM)
#endif

void LinSM_Init(const LinSM_ConfigType* ConfigPtr);
void LinSM_DeInit();

Std_ReturnType LinSM_ScheduleRequest(NetworkHandleType channel,LinIf_SchHandleType schedule);

Std_ReturnType LinSM_GetCurrentComMode(NetworkHandleType network,ComM_ModeType* mode);

Std_ReturnType LinSM_RequestComMode(NetworkHandleType network,ComM_ModeType mode);

void LinSM_MainFunction();

#endif
