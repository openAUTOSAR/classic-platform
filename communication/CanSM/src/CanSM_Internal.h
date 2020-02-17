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
#include "CanIf.h"

/**  @req CANSM028  @req CANSM071 @req CANSM363 @req CANSM364 */
#if (CANSM_DEV_ERROR_DETECT == STD_ON)
/* @req CANSM074 */
#define CANSM_DET_REPORTERROR(_api, _error) (void)Det_ReportError(CANSM_MODULE_ID, 0, _api, _error)
#else
#define CANSM_DET_REPORTERROR(serviceId, errorId)
#endif

typedef struct {
    uint8 controllerId;
    boolean hasPendingCtrlIndication;
    CanIf_ControllerModeType pendingCtrlMode;
    CanIf_ControllerModeType indCtrlMode;
}CanSM_Internal_CtrlStatusType;

#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
typedef struct {
    uint8 trcvId;
    boolean hasPendingTrcvIndication;
    CanTrcv_TrcvModeType pendingTrcvMode;
    CanTrcv_TrcvModeType indTrcvMode;
    boolean checkWufPendingIndication;
    boolean clearWufPendingIndication;
}CanSM_Internal_TrcvStatusType;
#endif

typedef enum {
    CANSM_PRENOCOMM_EXIT_NONE,
    CANSM_PRENOCOMM_EXIT_NOCOM,
    CANSM_PRENOCOMM_EXIT_REP_MAX
} CanSM_Internal_PreNoCommExitPointType;

typedef enum {
    CANSM_PREFULLCOMM_EXIT_NONE,
    CANSM_PREFULLCOMM_EXIT_FULLCOMM,
    CANSM_PREFULLCOMM_EXIT_REP_MAX
} CanSM_Internal_PreFullCommExitPointType;

typedef enum {
    CANSM_FULLCOMM_EXIT_NONE,
    CANSM_FULLCOMM_EXIT_PRENOCOMM,
    CANSM_FULLCOMM_EXIT_SILENTCOMM
} CanSM_Internal_FullCommExitPointType;

typedef enum {
    CANSM_BSM_S_NOT_INITIALIZED = 0,/* @req CANSM424 */
    CANSM_BSM_S_PRE_NOCOM,
    CANSM_BSM_S_NOCOM,
    CANSM_BSM_S_PRE_FULLCOM,
    CANSM_BSM_S_FULLCOM,
    CANSM_BSM_S_SILENTCOM
//	CANSM_BSM_S_CHANGE_BAUDRATE
} CanSM_Internal_BsmStateType;

typedef enum {
    CANSM_PRENOCOMM_S_CC_STOPPED = 0,
#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
    CANSM_PRENOCOMM_S_CC_SLEEP,
    CANSM_PRENOCOMM_S_TRCV_NORMAL,
    CANSM_PRENOCOMM_S_TRCV_STANDBY,
    CANSM_PRENOCOMM_S_PN_CLEAR_WUF,
    CANSM_PRENOCOMM_S_CHECK_WUF_IN_CC_SLEEP,
    CANSM_PRENOCOMM_S_CHECK_WUF_IN_NOT_CC_SLEEP,
#else
    CANSM_PRENOCOMM_S_CC_SLEEP
#endif
}CanSM_Internal_PreNoCommStateType;

typedef enum {
#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
    CANSM_PREFULLCOMM_S_TRCV_NORMAL,
#endif
    CANSM_PREFULLCOMM_S_CC_STOPPED,
    CANSM_PREFULLCOMM_S_CC_STARTED
}CanSM_Internal_PreFullCommStateType;

typedef enum {

    CANSM_FULLCOMM_S_BUSOFF_CHECK = 0,
    CANSM_FULLCOMM_S_NO_BUSOFF,
    CANSM_FULLCOMM_S_RESTART_CC,
    CANSM_FULLCOMM_S_TX_OFF
}CanSM_Internal_FullCommStateType;

typedef enum {
    CANSM_STATUS_UNINIT,
    CANSM_STATUS_INIT
} CanSM_Internal_InitStatusType;

typedef struct {
    boolean initialNoComRun;
    uint16 busoffCounter; /* Need uint16 since we need to check if greater than configured value in ASR 4.0.2*/
    boolean busoffevent;
    uint32 subStateTimer;
    uint32 busoffTime;
    ComM_ModeType requestedMode;
    ComM_ModeType currentMode;
    CanSM_Internal_BsmStateType			BsmState;
    CanSM_Internal_PreNoCommStateType	PreNoCommState;
    CanSM_Internal_PreFullCommStateType	PreFullCommState;
    CanSM_Internal_FullCommStateType	FullCommState;
    uint8								RepeatCounter;
    boolean requestAccepted;
#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
    CanSM_Internal_TrcvStatusType 		TransceiverModeBuf;
#endif
} CanSM_Internal_NetworkType;

typedef struct {
    CanSM_Internal_InitStatusType 		InitStatus;
    CanSM_Internal_NetworkType*		Networks;
    CanSM_Internal_CtrlStatusType*	ControllerModeBuf;
} CanSM_InternalType;

typedef enum {
    //PreNoCom
    T_CC_STOPPED_CC_SLEEP = 0,
#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
    T_CC_SLEEP_TRCV_NORMAL,
    T_TRCV_NORMAL_TRCV_STANDBY,
    T_TRCV_STANDBY_NONE,
    T_TRCV_NORMAL_CC_STOPPED,
    T_PN_CLEAR_WUF_CC_STOPPED,
    T_CC_STOPPED_TRCV_NORMAL,
    T_TRCV_STANDBY_CC_SLEEP,
    T_CC_SLEEP_CHECK_WUF_IN_CC_SLEEP,
    T_CHECK_WUF_IN_CC_SLEEP_NONE,
    T_CHECK_WUF_IN_NOT_CC_SLEEP_PN_CLEAR_WUF,
#endif
    T_CC_SLEEP_NONE,
    T_CC_STOPPED_CC_STARTED,
    T_CC_STARTED_NONE, /*NOTE: This must be last!*/
    T_NOF_TRANSITIONS
}CanSM_Internal_TransitionType;

typedef enum {
    T_REQ_ACCEPTED = 0,
    T_WAIT_INDICATION,
    T_FAIL,
    T_DONE,
    T_REPEAT_MAX,
    T_TIMEOUT
}CanSM_Internal_TransitionReturnType;


CanSM_Internal_TransitionReturnType CanSM_Internal_Execute(CanSM_Internal_NetworkType *Network, NetworkHandleType CanSMNetworkIndex, CanSM_Internal_TransitionType transition);
Std_ReturnType CanSM_Internal_GetCanSMCtrlIdx(const uint8 controller, uint8* indexPtr);
#if (CANSM_CAN_TRCV_SUPPORT == STD_ON)
CanSM_Internal_TransitionReturnType CanSM_Internal_DeinitWithPnSupport(NetworkHandleType CanSMNetworkIndex,CanSM_Internal_NetworkType *Network,CanSM_Internal_PreNoCommExitPointType *exitPoint,CanSM_Internal_PreNoCommStateType *nextSubState);
Std_ReturnType CanIfTrcvChnlToCanSMNwHdl(uint8 TransceiverId,NetworkHandleType *NetworkHandle);
Std_ReturnType CanSM_Internal_SetNetworkTransceiverMode(NetworkHandleType CanSMNetworkIndex, CanTrcv_TrcvModeType trcvMode);
Std_ReturnType CanSM_Internal_GetTrcvrModeInd(NetworkHandleType CanSMNetworkIndex);
Std_ReturnType CanSM_Internal_CheckWuf(NetworkHandleType CanSMNetworkIndex);
Std_ReturnType CanSM_Internal_ClearWuf(NetworkHandleType CanSMNetworkIndex);
Std_ReturnType CanSM_Internal_CheckWufInd(NetworkHandleType CanSMNetworkIndex);
Std_ReturnType CanSM_Internal_ClearWufInd(NetworkHandleType CanSMNetworkIndex);
#endif
CanSM_Internal_TransitionReturnType CanSM_Internal_DeinitWithoutPnSupport(NetworkHandleType CanSMNetworkIndex,CanSM_Internal_NetworkType *Network,CanSM_Internal_PreNoCommExitPointType *exitPoint,CanSM_Internal_PreNoCommStateType *nextSubState);
#endif /* CANSM_INTERNAL_H */
