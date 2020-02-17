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

#include "FrSM.h"
#include "FrSM_Internal.h"
#if defined(USE_BSWM)
#include "BswM_FrSM.h"
#endif

extern const FrSM_ConfigType* FrSM_ConfigPointer;
extern FrSM_InternalType FrSM_Internal;

/**
 * @brief FE_TRCV_NORMAL functionality
 * @param clusterId
 */
void FrSM_Internal_TrcvNormal(uint8 clusterId)
{
    const FrSM_ClusterType* clusterCfg;
    clusterCfg = &FrSM_ConfigPointer->FrSMClusters[clusterId];
    uint8 cltr;
    for(cltr = 0; cltr < clusterCfg->FrSMControllerCount; cltr++) {
        /* Check TRCV enabled for specific CC in Cluster */
        if(TRUE == clusterCfg->FrSMControllerRef[cltr].FrTrcvAvailable) {
            if(FRSM_ECU_ACTIVE != FrSM_Internal.clusterStatus[clusterId].frSMClusterPassiveState) {

                if(TRUE == clusterCfg->FrSMControllerRef[cltr].FrSMIsDualChannelNode) {
                    (void)FrIf_SetTransceiverMode(clusterCfg->FrSMControllerRef[cltr].FrIfControllerId,FR_CHANNEL_A,FRTRCV_TRCVMODE_RECEIVEONLY );
                    (void)FrIf_SetTransceiverMode(clusterCfg->FrSMControllerRef[cltr].FrIfControllerId,FR_CHANNEL_B,FRTRCV_TRCVMODE_RECEIVEONLY );

                    (void)FrIf_ClearTransceiverWakeup( clusterCfg->FrSMControllerRef[cltr].FrIfControllerId,FR_CHANNEL_A);
                    (void)FrIf_ClearTransceiverWakeup( clusterCfg->FrSMControllerRef[cltr].FrIfControllerId,FR_CHANNEL_B);
                } else {
                    (void)FrIf_SetTransceiverMode(clusterCfg->FrSMControllerRef[cltr].FrIfControllerId,clusterCfg->FrSMControllerRef[cltr].FrSMChannel, FRTRCV_TRCVMODE_RECEIVEONLY );
                    (void)FrIf_ClearTransceiverWakeup(clusterCfg->FrSMControllerRef[cltr].FrIfControllerId,clusterCfg->FrSMControllerRef[cltr].FrSMChannel);
                }

            } else {

                if(TRUE == clusterCfg->FrSMControllerRef[cltr].FrSMIsDualChannelNode) {
                    (void)FrIf_SetTransceiverMode(clusterCfg->FrSMControllerRef[cltr].FrIfControllerId,FR_CHANNEL_A,FRTRCV_TRCVMODE_NORMAL );
                    (void)FrIf_SetTransceiverMode(clusterCfg->FrSMControllerRef[cltr].FrIfControllerId,FR_CHANNEL_B,FRTRCV_TRCVMODE_NORMAL );

                    (void)FrIf_ClearTransceiverWakeup( clusterCfg->FrSMControllerRef[cltr].FrIfControllerId,FR_CHANNEL_A);
                    (void)FrIf_ClearTransceiverWakeup( clusterCfg->FrSMControllerRef[cltr].FrIfControllerId,FR_CHANNEL_B);
                } else {
                    (void)FrIf_SetTransceiverMode(clusterCfg->FrSMControllerRef[cltr].FrIfControllerId,clusterCfg->FrSMControllerRef[cltr].FrSMChannel, FRTRCV_TRCVMODE_NORMAL );
                    (void)FrIf_ClearTransceiverWakeup(clusterCfg->FrSMControllerRef[cltr].FrIfControllerId,clusterCfg->FrSMControllerRef[cltr].FrSMChannel);
                }
            }
        }
    }
}

/**
 * @brief Function updates all timers after every main function call if applicable
 * @param clusterId
 */
void FrSM_Internal_UpdateTimer(uint8 clusterId)
{
    /* t1_IsActive flag is checked only if Started was set to true. If t1_IsActive flag is false it means a timeout. */
    if((TRUE == FrSM_Internal.controllerMode[clusterId].t1Started) &&
            (FrSM_Internal.controllerMode[clusterId].timerT1 < FrSM_ConfigPointer->FrSMClusters[clusterId].FrSMDurationT1)) {
        FrSM_Internal.controllerMode[clusterId].timerT1++;
        FrSM_Internal.controllerMode[clusterId].t1_IsActive = TRUE;
    } else {
        /* @req FrSm142 */
        FrSM_Internal.controllerMode[clusterId].t1_IsActive = FALSE;
    }

    /* t2_IsActive flag is checked only if Started was set to true. If t2_IsActive flag is false it means a timeout. */
    if((TRUE == FrSM_Internal.controllerMode[clusterId].t2Started) &&
            (FrSM_Internal.controllerMode[clusterId].timerT2 < FrSM_ConfigPointer->FrSMClusters[clusterId].FrSMDurationT2)) {
        FrSM_Internal.controllerMode[clusterId].timerT2++;
        FrSM_Internal.controllerMode[clusterId].t2_IsActive = TRUE;
    } else {
        /* @req FrSm143 */
        FrSM_Internal.controllerMode[clusterId].t2_IsActive = FALSE;
    }

    /* t3_IsNotActive flag is checked only if Started was set to true. If t3_IsNotActive flag is true it means a timeout. */
    if((TRUE == FrSM_Internal.controllerMode[clusterId].t3Started) &&
            (FrSM_Internal.controllerMode[clusterId].timerT3 < FrSM_ConfigPointer->FrSMClusters[clusterId].FrSMDurationT3)) {
        FrSM_Internal.controllerMode[clusterId].timerT3++;
        FrSM_Internal.controllerMode[clusterId].t3_IsNotActive = FALSE;

    } else {
        FrSM_Internal.controllerMode[clusterId].t3_IsNotActive = TRUE;
    }
}

/**
 * @brief Function updates BswM layer about FrSm cluster state
 * @param clusterId
 */
#if defined(USE_BSWM)
/* @req FrSm145 */
void FrSM_Internal_BswM_Indication(uint8 clusterId)
{
    const FrSM_ClusterType* clusterCfg;
    clusterCfg = &FrSM_ConfigPointer->FrSMClusters[clusterId];

    if(FRSM_WAKEUP == FrSM_Internal.clusterStatus[clusterId].frSMClusterState) {
        if(FRSM_ECU_ACTIVE == FrSM_Internal.clusterStatus[clusterId].frSMClusterPassiveState)
        {
            BswM_FrSM_CurrentState(clusterCfg->FrSMComMNetworkHandle,FRSM_BSWM_WAKEUP);
        } else {
            BswM_FrSM_CurrentState(clusterCfg->FrSMComMNetworkHandle,FRSM_BSWM_WAKEUP_ECU_PASSIVE);
        }

    } else if(FRSM_STARTUP == FrSM_Internal.clusterStatus[clusterId].frSMClusterState){
        if(FRSM_ECU_ACTIVE == FrSM_Internal.clusterStatus[clusterId].frSMClusterPassiveState)
        {
            BswM_FrSM_CurrentState(clusterCfg->FrSMComMNetworkHandle,FRSM_BSWM_STARTUP);
        } else {
            BswM_FrSM_CurrentState(clusterCfg->FrSMComMNetworkHandle,FRSM_BSWM_STARTUP_ECU_PASSIVE);
        }
    } else if(FRSM_HALT_REQ == FrSM_Internal.clusterStatus[clusterId].frSMClusterState){
        if(FRSM_ECU_ACTIVE == FrSM_Internal.clusterStatus[clusterId].frSMClusterPassiveState)
        {
            BswM_FrSM_CurrentState(clusterCfg->FrSMComMNetworkHandle,FRSM_BSWM_HALT_REQ);
        } else {
            BswM_FrSM_CurrentState(clusterCfg->FrSMComMNetworkHandle,FRSM_BSWM_HALT_REQ_ECU_PASSIVE);
        }
    } else if(FRSM_ONLINE == FrSM_Internal.clusterStatus[clusterId].frSMClusterState){
        if(FRSM_ECU_ACTIVE == FrSM_Internal.clusterStatus[clusterId].frSMClusterPassiveState)
        {
            BswM_FrSM_CurrentState(clusterCfg->FrSMComMNetworkHandle,FRSM_BSWM_ONLINE);
        } else {
            BswM_FrSM_CurrentState(clusterCfg->FrSMComMNetworkHandle,FRSM_BSWM_ONLINE_ECU_PASSIVE);
        }
    } else if(FRSM_ONLINE_PASSIVE == FrSM_Internal.clusterStatus[clusterId].frSMClusterState){
        if(FRSM_ECU_ACTIVE == FrSM_Internal.clusterStatus[clusterId].frSMClusterPassiveState)
        {
            BswM_FrSM_CurrentState(clusterCfg->FrSMComMNetworkHandle,FRSM_BSWM_ONLINE_PASSIVE);
        } else {
            BswM_FrSM_CurrentState(clusterCfg->FrSMComMNetworkHandle,FRSM_BSWM_ONLINE_PASSIVE_ECU_PASSIVE);
        }
    } else if(FRSM_KEYSLOT_ONLY == FrSM_Internal.clusterStatus[clusterId].frSMClusterState){
        if(FRSM_ECU_ACTIVE == FrSM_Internal.clusterStatus[clusterId].frSMClusterPassiveState)
        {
            BswM_FrSM_CurrentState(clusterCfg->FrSMComMNetworkHandle,FRSM_BSWM_KEYSLOT_ONLY);
        } else {
            BswM_FrSM_CurrentState(clusterCfg->FrSMComMNetworkHandle,FRSM_BSWM_KEYSLOT_ONLY_ECU_PASSIVE);
        }
    } else if(FRSM_LOW_NUMBER_OF_COLDSTARTERS == FrSM_Internal.clusterStatus[clusterId].frSMClusterState){
        if(FRSM_ECU_ACTIVE == FrSM_Internal.clusterStatus[clusterId].frSMClusterPassiveState)
        {
            BswM_FrSM_CurrentState(clusterCfg->FrSMComMNetworkHandle,FRSM_BSWM_LOW_NUMBER_OF_COLDSTARTERS);
        } else {
            BswM_FrSM_CurrentState(clusterCfg->FrSMComMNetworkHandle,FRSM_BSWM_LOW_NUMBER_OF_COLDSTARTERS_ECU_PASSIVE);
        }
    } else if(FRSM_READY == FrSM_Internal.clusterStatus[clusterId].frSMClusterState){
        if(FRSM_ECU_ACTIVE == FrSM_Internal.clusterStatus[clusterId].frSMClusterPassiveState)
        {
            BswM_FrSM_CurrentState(clusterCfg->FrSMComMNetworkHandle,FRSM_BSWM_READY);
        } else {
            BswM_FrSM_CurrentState(clusterCfg->FrSMComMNetworkHandle,FRSM_BSWM_READY_ECU_PASSIVE);
        }
    } else {
        /* Do Nothing */
    }

    FrSM_Internal.clusterStatus[clusterId].bswMindication = FALSE;
}
#endif


