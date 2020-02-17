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

#ifndef OSEKNM_OSEKDIRECTNM_H_
#define OSEKNM_OSEKDIRECTNM_H_
#include "Std_Types.h"
#include "OsekNm_ConfigTypes.h"
#include "OsekNm_Cfg.h"

typedef struct {
    TickType osekdirectNmTTx;
    TickType osekdirectNmTTyp;
    TickType osekdirectNmTMax;
    TickType osekdirectNmTError;
    TickType osekdirectNmTWbs;
}OseknmDirectTimer;

typedef struct {
    NetworkStatusType sMask;
    NetworkStatusType networkStatus;
    SignallingMode sMode;
    TaskType taskId;
    EventMaskType eMask;
}OseknmStatusType;

typedef union{
    uint32 b;
    struct {
        unsigned stable : 1;
        unsigned limphome : 1;
    }B;
}OsekNMMerkerType;


typedef struct{
    OseknmStatusType osekNmStatus;
    OsekNmDirectNmStateType nmState;
    OsekNm_PduType nmTxPdu;
}OsekNm_InternalNetType;


typedef struct {
    OsekNm_PduType nmTransmitPdu;
    OsekNm_PduType nmRxPdu[OSEKNM_DIRECT_MAX_NODE_NUM];
    NetworkStatusType networkStatus;
    OsekNMMerkerType nmMerker;
    OseknmDirectTimer timer;
    OsekNm_CmaskParamsType nmDirectMask;
    OsekNm_ConfigParamsType nmDirectConfig;
    StatusRefType sMask;
    StatusRefType targetStatus;
    OsekNmDirectNmStateType nmState;
    uint8 nmRxRetryCounter;
    uint8 nmTxRetryCounter;
    uint8 nmRxPduCount;
    boolean rcvdNmpdu;
    boolean transNmpdu;

}OsekDirect_InternalType;
/******* Function Declerations ***********/

/* ********* System Generation Support ************* */

/*InitNMType is a directive to select a NM from a given set of NM implementations.*/
void InitNMType(NetIdType netId, NMType nmType);

/* InitNMScaling is a directive for scaling the given NM of the referenced net  */
void InitNMScaling(NetIdType netId,ScalingParamType scalingParams);

/*SelectHWRoutines is a directive to select routines from a given set of routines to drive the bus hardware.*/
void SelectHWRoutines(NetIdType netId,RoutineRefType busInit,RoutineRefType busAwake, RoutineRefType busSleep,RoutineRefType busRestart,RoutineRefType busShutdown);

/*InitCMaskTable is a directive for initializing an element of a table of relevant configuration
masks to be used by the signalling of changed configurations*/
void OsekDirectNm_InitCMaskTable(NetIdType netId,ConfigKindName configKind,ConfigRefType cMask);

/*InitTargetConfigTable is a directive for initializing an element of a table of relevant target
configurations to be used by the signalling of changed configurations*/
void Direct_InitTargetConfigTable(NetIdType netId,ConfigKindName configKind,ConfigRefType targetConfig);

/* InitIndDeltaConfig is a directive for specifying the indication of configuration changes. The concerned configuration is specified by <ConfigKind>. */
void InitIndDeltaConfig(NetIdType netId,ConfigKindName configKind,SignallingMode sMode,TaskType taskId,EventMaskType eMask);

/* InitSMaskTable is a directive for initializing an element of a table of relevant status masks
to be used by the signalling of changed network states */
/* void InitSMaskTable(NetIdType NetId,StatusRefType SMask); */

/*InitTargetStatusTable is a directive for initializing an element of a table of relevant target
network states to be used by the signalling of changed network states */
/* void InitTargetStatusTable(NetIdType NetId,StatusRefType TargetStatus); */

/*InitIndDeltaStatus is a directive for specifying the indication of status changes*/
/* void InitIndDeltaStatus(NetIdType NetId,SignallingMode SMode,TaskType TaskId,EventMaskType EMask); */


/* ********* Services Support for Config ************* */
/* This service makes the NM start or restart the configuration management */
StatusType InitConfig(NetIdType netId);

/* This service provides the actual configuration of the kind specified by <ConfigKind> */
StatusType OsekDirectNm_GetConfig(NetIdType netId,ConfigRefType config,ConfigKindName configKind);

/* The test configuration <TestConfig> is compared to the specified reference configuration
<RefConfig> taking account of the mask <CMask>.*/
StatusType OsekDirectNm_CmpConfig(NetIdType netId,ConfigRefType testConfig,ConfigRefType refConfig,ConfigRefType cMask);

/* A set of predefined parameter is selectable to drive the signalling of changed configurations */
StatusType SelectDeltaConfig(NetIdType netId,ConfigKindName configKind,ConfigHandleType configHandle,ConfigHandleType cMaskHandle);

/** @req OSEKNM004 */
/* StartNM starts the local Network Management. This causes the state transition from NMOff to NMOn*/
StatusType OsekDirectNm_StartNM(NetIdType netId);

/* StopNM stops the local Network Management. This causes the state transition from NMOn
 * to NMShutDown and after processing of the shutdown activities to NMOff*/
StatusType OsekDirectNm_StopNM(NetIdType netId);

/* GotoMode serves to set the NM operating mode specified by <NewMode> */
StatusType OsekDirectNm_GotoMode(NetIdType netId,NMModeName newMode);

/* This service provides the current status of the network */
StatusType OsekDirectNm_GetStatus(NetIdType netId,StatusRefType networkStatus); /*lint !e9018 'NetworkStatus' with union based type 'StatusRefType */

/* A set of predefined parameter is selectable to drive the signaling of changed states */
/* StatusType SelectDeltaStatus(NetIdType NetId,StatusHandleType StatusHandle,StatusHandleType SMaskHandle); */

void OsekDirectNm_MainFunction(NetIdType netId);

/* ********* Services Support Direct Nm ************* */

/* InitDirectNMParams is a directive for initializing the parameters of the direct NM */
void InitDirectNMParams(NetIdType NetId,NodeIdType nodeId,TickType timerTyp,TickType timerMax, TickType timerError,TickType timerWaitBusSleep,TickType timerTx);

/* SilentNM disables the communication of the NM. This causes the state transition from NMActive to NMPassive*/
StatusType OsekDirectNm_SilentNM(NetIdType netId);

/* TalkNM enables the communication of the NM again, after a previous call of SilentNM. */
StatusType OsekDirectNm_TalkNM(NetIdType netId);

/* InitIndRingData is a directive for specifying the indication of received data in the data field
of a ring message, which is addressed to this node */
void InitIndRingData(NetIdType netId,SignallingMode sMode,TaskType taskId,EventMaskType eMask);

/* ReadRingData enables the application to read the data that has been received by a ring message  */
void ReadRingData(NetIdType netId,RingDataRefType ringData);

/* This service enables the application to transmit data via the ring message */
void TransmitRingData(NetIdType netId,RingDataRefType ringData);


/* ********* DLL Functions ************* */
void OsekDirectNm_ControllerBusOff(NetIdType netId);
void OsekDirectNm_TxConformation(NetIdType netId);
void OsekDirectNm_RxIndication(NetIdType netId, const OsekNm_PduType* nmPdu);
#if (ISO_OSEK_NM == STD_ON)
void OsekDirectNm_WakeupIndication(NetIdType netId);
#endif


#endif /* OSEKNM_OSEKDIRECTNM_H_ */
