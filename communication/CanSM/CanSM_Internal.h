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


#ifndef CANSM_INTERNAL_H
#define CANSM_INTERNAL_H

#include "CanSM.h"

/**  @req CANSM028  @req CANSM071 @req CANSM363 @req CANSM364 */
#if (CANSM_DEV_ERROR_DETECT == STD_ON)
/* @req CANSM074 */
#define CANSM_DET_REPORTERROR(serviceId, errorId)			\
	Det_ReportError(MODULE_ID_CANSM, 0, serviceId, errorId)
#else
#define CANSM_DET_REPORTERROR(serviceId, errorId)
#endif

typedef struct {
	uint8 controllerId;
	boolean hasPendingCtrlIndication;
	CanIf_ControllerModeType pendingCtrlMode;
	CanIf_ControllerModeType indCtrlMode;
}CanSM_Internal_CtrlStatusType;

typedef enum {
	CANSM_PRENOCOMM_EXITPOINT_NONE,
	CANSM_PRENOCOMM_EXITPOINT_NOCOM
} CanSM_Internal_PreNoCommExitPointType;

typedef enum {
	CANSM_PREFULLCOMM_EXITPOINT_NONE,
	CANSM_PREFULLCOMM_EXITPOINT_FULLCOMM,
	CANSM_PREFULLCOMM_EXITPOINT_REPEAT_MAX
} CanSM_Internal_PreFullCommExitPointType;

typedef enum {
	CANSM_FULLCOMM_EXITPOINT_NONE,
	CANSM_FULLCOMM_EXITPOINT_PRENOCOMM,
	CANSM_FULLCOMM_EXITPOINT_SILENTCOMM
} CanSM_Internal_FullCommExitPointType;

typedef enum {
	CANSM_BSM_S_NOT_INITIALIZED = 0,/* @req CANSM424 */
	CANSM_BSM_S_PRE_NOCOM,
	CANSM_BSM_S_NOCOM,
	CANSM_BSM_S_PRE_FULLCOM,
	CANSM_BSM_S_FULLCOM,
	CANSM_BSM_S_SILENTCOM,
//	CANSM_BSM_S_CHANGE_BAUDRATE
} CanSM_Internal_BsmStateType;

typedef enum {
	CANSM_PRENOCOMM_S_CC_STOPPED = 0,
	CANSM_PRENOCOMM_S_CC_STOPPED_WAIT,
	CANSM_PRENOCOMM_S_CC_SLEEP,
	CANSM_PRENOCOMM_S_CC_SLEEP_WAIT,
	CANSM_PRENOCOMM_S_TRCV_NORMAL,
	CANSM_PRENOCOMM_S_TRCV_NORMAL_WAIT,
	CANSM_PRENOCOMM_S_TRCV_STANDBY,
	CANSM_PRENOCOMM_S_TRCV_STANDBY_WAIT
}CanSM_Internal_PreNoCommStateType;

typedef enum {

	CANSM_PREFULLCOMM_S_TRCV_NORMAL= 0,
	CANSM_PREFULLCOMM_S_TRCV_NORMAL_WAIT,
	CANSM_PREFULLCOMM_S_CC_STOPPED,
	CANSM_PREFULLCOMM_S_CC_STOPPED_WAIT,
	CANSM_PREFULLCOMM_S_CC_STARTED,
	CANSM_PREFULLCOMM_S_CC_STARTED_WAIT,
}CanSM_Internal_PreFullCommStateType;

typedef enum {

	CANSM_FULLCOMM_S_BUSOFF_CHECK = 0,
	CANSM_FULLCOMM_S_NO_BUSOFF,
	CANSM_FULLCOMM_S_RESTART_CC,
	CANSM_FULLCOMM_S_RESTART_CC_WAIT,
	CANSM_FULLCOMM_S_TX_OFF,
}CanSM_Internal_FullCommStateType;

typedef enum {
	CANSM_STATUS_UNINIT,
	CANSM_STATUS_INIT
} CanSM_Internal_InitStatusType;

typedef struct {
	boolean initialNoComRun;
	uint8 busoffCounter;
	boolean busoffevent;
	uint32 subStateTimer;
	ComM_ModeType requestedMode;
	ComM_ModeType currentMode;
	CanSM_Internal_BsmStateType			BsmState;
	CanSM_Internal_PreNoCommStateType	PreNoCommState;
	CanSM_Internal_PreFullCommStateType	PreFullCommState;
	CanSM_Internal_FullCommStateType	FullCommState;
	uint8								RepeatCounter;
} CanSM_Internal_NetworkType;

typedef struct {
	CanSM_Internal_InitStatusType 		InitStatus;
	CanSM_Internal_NetworkType*		Networks;
	CanSM_Internal_CtrlStatusType*	ControllerModeBuf;
} CanSM_InternalType;


Std_ReturnType CanSM_Internal_GetNetworkControllerModeIndicated(NetworkHandleType NetworkHandle);
Std_ReturnType CanSM_Internal_SetNetworkControllerMode(NetworkHandleType NetworkHandle, CanIf_ControllerModeType controllerMode);
void CanSM_Internal_SetNetworkPduMode(NetworkHandleType NetworkHandle, CanIf_PduSetModeType pduMode);
Std_ReturnType CanSM_Internal_GetCanSMControllerIndex(const uint8 controller, uint8* indexPtr);

#endif /* CANSM_INTERNAL_H */
