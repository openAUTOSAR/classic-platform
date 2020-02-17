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






/** @req LINSM005 */

#ifndef LINSM_H_
#define LINSM_H_

#define LINSM_MODULE_ID            141u
#define LINSM_VENDOR_ID            60u

#define LINSM_SW_MAJOR_VERSION              2u
#define LINSM_SW_MINOR_VERSION              4u
#define LINSM_SW_PATCH_VERSION              0u
#define LINSM_AR_RELEASE_MAJOR_VERSION      4u
#define LINSM_AR_RELEASE_MINOR_VERSION      0u
#define LINSM_AR_RELEASE_PATCH_VERSION      3u

#include "ComStack_Types.h"
#include "Std_Types.h"
#include "Lin_GeneralTypes.h"
#include "LinSM_Cfg.h"
#include "Com_Types.h"
#if defined(USE_COM)
#include "Com.h"
#endif
#include "ComM_Types.h"
#include "LinIf.h"

#define LINSM_INIT_SERVICE_ID                   0x01u
#define LINSM_GET_CURRENT_COM_MODE_SERVICE_ID   0x11u
#define LINSM_REQUEST_COM_MODE_SERVICE_ID       0x12u
#define LINSM_SCHEDULE_REQUEST_SERVICE_ID       0x10u
#define LINSM_SCHEDULE_REQUEST_CONF_SERVICE_ID  0x20u
#define LINSM_WAKEUP_CONF_SERVICE_ID            0x21u
#define LINSM_GOTO_SLEEP_CONF_SERVICE_ID        0x22u
#define LINSM_MAIN_FUNCTION_SERVICE_ID          0x30u

/**@req LINSM052*//**@req LINSM053*/
/* --- Error codes --- */
#define LINSM_E_UNINIT                  (uint8) 0x00u
#define LINSM_E_ALREADY_INITIALIZED		(uint8) 0x10u
#define LINSM_E_NONEXISTENT_NETWORK		(uint8) 0x20u
#define LINSM_E_PARAMETER				(uint8) 0x30u
#define LINSM_E_PARAMETER_POINTER		(uint8) 0x40u
#define LINSM_E_CONFIRMATION_TIMEOUT	(uint8) 0x50u /* Assigned by DEM */

typedef enum {
    LINSM_UNINIT,
    LINSM_INIT,
    LINSM_NO_COMMUNICATION,
    LINSM_FULL_COMMUNICATION
}LinSM_StatusType;


typedef enum {
    /** @req LINSM0220 */
    LINSM_NO_COM = 1, /* @req LINSM026 */
    LINSM_FULL_COM =2  /* @req LINSM032 */
}LinSM_ModeType;

/**@req LINSM117 *//**@req LINSM118 *//**@req LINSM121 *//**@req LINSM165 */
#if (LINSM_VERSION_INFO_API == STD_ON)
void LinSM_GetVersionInfo( Std_VersionInfoType *versionInfo );
#define LinSM_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,LINSM)
#endif

void LinSM_Init(const LinSM_ConfigType* ConfigPtr);
void LinSM_DeInit(void);

Std_ReturnType LinSM_ScheduleRequest(NetworkHandleType channel,LinIf_SchHandleType schedule);

Std_ReturnType LinSM_GetCurrentComMode(NetworkHandleType network,ComM_ModeType* mode);

Std_ReturnType LinSM_RequestComMode(NetworkHandleType network,ComM_ModeType mode);

void LinSM_MainFunction(void);

#endif
