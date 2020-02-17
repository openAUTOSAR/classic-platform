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
#ifndef FRSM_INTERNAL_H_
#define FRSM_INTERNAL_H_

#include "FrSM.h"
#include "FrIf.h"

//lint -emacro(904,FRSM_DET_REPORTERROR) //904 PC-Lint exception to MISRA 14.7 (validate DET macros).

typedef enum {
    FRSM_STATUS_UNINIT,
    FRSM_STATUS_INIT
}FrSM_Internal_InitStatusType;

/* @req FrSm176 */
typedef enum {
    FRSM_ECU_ACTIVE,
    FRSM_ECU_PASSIVE
}FrSM_Internal_Passive_State;

/* @req FrSm032 */
typedef enum {
    FRSM_READY,
    FRSM_WAKEUP,
    FRSM_STARTUP,
    FRSM_HALT_REQ,
    FRSM_ONLINE,
    FRSM_ONLINE_PASSIVE,
    FRSM_KEYSLOT_ONLY,
    FRSM_LOW_NUMBER_OF_COLDSTARTERS,
}FrSM_Internal_Cluster_State;

typedef enum{
    SINGLE_CHANNEL_WAKEUP,
    DUAL_CHANNEL_WAKEUP,
    DUAL_CHANNEL_WAKEUPFORWARD,
    DUAL_CHANNEL_ECHO_WAKEUP,
    NO_WAKEUP
}FrSM_Internal_Wakeup_Type;

typedef enum{
    NO_ACTION,
    ECHO_MODE_OFF,
    ECHO_MODE_ON
}FrSM_Internal_Wakeup_Action;

typedef enum {
    NO_WU_BY_BUS,
    PARTIAL_WU_BY_BUS,
    ALL_WU_BY_BUS,
}FrSM_Internal_WakeType;

typedef struct {
    uint32                        timerT1;
    uint32                        timerT2;
    uint32                        timerT3;
    uint16                        wakeupCounter;
    uint8                         controllerId;
    uint8                         startupCounter;
    uint8                         wakeupRxstatus;
    Fr_POCStatusType              pocCcStatus;
    FrSM_Internal_Wakeup_Type     wakeupType;
    Fr_ChannelType                channelType;
    FrSM_Internal_Wakeup_Action   wakeUpAction;
    FrSM_Internal_WakeType        wuReason;
    boolean                       wakeupTransmitted;
    boolean                       busTrafficDetected;
    boolean                       allChannelsAwake;
    boolean                       t1_IsActive;
    boolean                       t2_IsActive;
    boolean                       t3_IsNotActive;
    boolean                       wakeupFinished;
    boolean                       lowNumberOfColdstarters;
    boolean                       t1Started;
    boolean                       t2Started;
    boolean                       t3Started;
    boolean                       t2action;
}FrSM_Internal_CtrlStatusType;

typedef struct {
    ComM_ModeType                 reqComMode;
    ComM_ModeType                 curComMode;
    FrSM_Internal_Cluster_State   frSMClusterState;
    FrSM_Internal_Passive_State   frSMClusterPassiveState;
    boolean                       bswMindication;
}FrSM_Internal_ClusterType;

typedef struct {
    FrSM_Internal_ClusterType*    clusterStatus;
    FrSM_Internal_CtrlStatusType* controllerMode;
    FrSM_Internal_InitStatusType  initStatus;
}FrSM_InternalType;


void FrSM_Internal_TrcvNormal(uint8 clusterId);
void FrSM_Internal_UpdateTimer(uint8 clusterId);
#if defined(USE_BSWM)
void FrSM_Internal_BswM_Indication(uint8 clusterId);
#endif
#define UPDATE_TIMER1(clusterId)  if(0 != FrSM_ConfigPointer->FrSMClusters[clusterId].FrSMDurationT1) { \
                            FrSM_Internal.controllerMode[clusterId].t1Started = TRUE;  \
                            FrSM_Internal.controllerMode[clusterId].timerT1 = 0; \
                        }
#define UPDATE_TIMER2(clusterId)  if(0 != FrSM_ConfigPointer->FrSMClusters[clusterId].FrSMDurationT2) { \
                            FrSM_Internal.controllerMode[clusterId].t2Started = TRUE;  \
                            FrSM_Internal.controllerMode[clusterId].timerT2 = 0; \
                        }
#define UPDATE_TIMER3(clusterId)  if(0 != FrSM_ConfigPointer->FrSMClusters[clusterId].FrSMDurationT3) { \
                            FrSM_Internal.controllerMode[clusterId].t3Started = TRUE;  \
                            FrSM_Internal.controllerMode[clusterId].timerT3 = 0; \
                        }
/* @req FrSm044 */
/* @req FrSm045 */
#if (FRSM_DEV_ERROR_DETECT == STD_ON)
#define FRSM_DET_REPORTERROR(_cond ,_api, _error, ...) \
    if (!_cond) { \
        (void)Det_ReportError(FRSM_MODULE_ID, 0, _api, _error); \
        return __VA_ARGS__; \
    }
#else
#define FRSM_DET_REPORTERROR(_cond ,_api, _error, ...) \
    if (!_cond) { \
        return __VA_ARGS__; \
    }
#endif

#if (FRSM_DEV_ERROR_DETECT == STD_ON)
#define FRSM_DET_REPORT_WITHOUT_RET(_api, _error) (void)Det_ReportError(FRSM_MODULE_ID, 0, _api, _error)
#else
#define FRSM_DET_REPORT_WITHOUT_RET(serviceId, errorId)
#endif

#if defined(USE_DEM)
#define FRSM_DEM_REPORTSTATUS(_cond, _demid, _status) \
    if (!_cond) { \
        (void)Dem_SetEventStatus(_demid, _status); \
    }
#else
#define FRSM_DEM_REPORTSTATUS(_cond, _demid, _status)
#endif

#endif /* FRSM_INTERNAL_H_ */
