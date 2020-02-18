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

#include "ComStack_Types.h"
#include "ComM_BusSM.h"
#include "FrSM.h"   /* @req FrSm055 */
#include "FrSM_Internal.h"
#include "FrNm.h"
#if (FRSM_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"   /* @req FrSm056 */
#endif
#if defined(USE_DEM)
#include "Dem.h"   /* @req FrSm059 */
#endif
#include "MemMap.h" /* @req FrSm057 */
#include "FrIf.h"  /* @req FrSm058 */
#include "SchM_FrSM.h"
#if defined(USE_BSWM)
#include "BswM_FrSM.h"
#endif

/* Globally fulfilled requirements */
/** @req FrSm051 */ /* Code file structure */
/** @req FrSm159 */ /* Timer T3 timeout implementation */
/** @req FrSm105 */ /* State machine actions will be in the order as specified by spec */
/** @req FrSm093 */ /* State Machine diagram  */
/** @req FrSm149 */ /* ignore return values of FrIf Apis */

#define CONTROLLER_0          (0x00u)
#define WAKEUP_INDICATOR_MASK (0x03u)
#define WAKEUP_CH_A_B         (0x03u)
#define WAKEUP_CH_A           (0x01u)
#define WAKEUP_CH_B           (0x02u)

void FrSM_MainFunction(uint8 clusterId);

/*lint -esym(9003,FrSM_ConfigPointer )*/
const FrSM_ConfigType* FrSM_ConfigPointer;

/* here controller count = FRSM_CLUSTER_COUNT since design Consideration for State machine
   is only one CC's of every cluster */
static FrSM_Internal_CtrlStatusType FrSM_InternalControllerStatus[FRSM_CLUSTER_COUNT];
static FrSM_Internal_ClusterType FrSM_InternalClusters[FRSM_CLUSTER_COUNT];

/** Static declarations */
FrSM_InternalType FrSM_Internal = {
        .initStatus = FRSM_STATUS_UNINIT,
        .clusterStatus = FrSM_InternalClusters,
        .controllerMode = FrSM_InternalControllerStatus,
};

/**
 * @brief Function indicates Upper ComM layer
 * @param clusterid
 * @param ComM_Mode
 */
static void FrSM_Internal_ComM_BusSM_ModeIndication( uint8 clusterid, ComM_ModeType ComM_Mode )
{
    /* Update internals */
    /* @req FrSm026 */
    FrSM_Internal.clusterStatus[clusterid].curComMode = ComM_Mode;
    /* Indicate to ComM */
    ComM_BusSM_ModeIndication(FrSM_ConfigPointer->FrSMClusters[clusterid].FrSMComMNetworkHandle, &ComM_Mode);
}

/**
 * @brief function calls the FrIf_AllowColdstart api if cluster is Cold start ECU
 * @param clusterId
 */
INLINE static void FrSM_Internal_AllowColdStart(uint8 clusterId)
{
    const FrSM_ClusterType* clusterCfg ;
    clusterCfg = &FrSM_ConfigPointer->FrSMClusters[clusterId];

    if(TRUE == clusterCfg->FrSMIsColdstartEcu) {
        (void)FrIf_AllowColdstart(clusterCfg->FrSMControllerRef[CONTROLLER_0].FrIfControllerId);
    } else {
        /* Do nothing */
    }
}

/**
 * @brief Reset WakeUp status variables
 * @param clusterId
 */
static void FrSM_Internal_WakeUp_Reset_Var(uint8 clusterId)
{
    FrSM_Internal.controllerMode[clusterId].wakeUpAction = ECHO_MODE_OFF;
    FrSM_Internal.controllerMode[clusterId].wakeupTransmitted = FALSE;
    FrSM_Internal.controllerMode[clusterId].wakeupFinished = FALSE;
    FrSM_Internal.controllerMode[clusterId].wakeupCounter = 0;
    FrSM_Internal.controllerMode[clusterId].busTrafficDetected  = FALSE;
}

/**
 * @brief Get POC states of First CC of Cluster
 * @param clusterId
 */
INLINE static void FrSM_Internal_PocStatus(uint8 clusterId) {
    const FrSM_ClusterType* clusterCfg;
    clusterCfg = &FrSM_ConfigPointer->FrSMClusters[clusterId];

    /* @req FrSm047 */
    (void)FrIf_GetPOCStatus(clusterCfg->FrSMControllerRef[CONTROLLER_0].FrIfControllerId, &FrSM_Internal.controllerMode[clusterId].pocCcStatus);
}

/**
 * @brief set lowNumberOfColdstarters to true/false
 * @param clusterId
 */
INLINE static void FrSM_Internal_StartupFrame_No(uint8 clusterId) {
    uint8 coldStartVal;

    /* @req FrSm192 */
    if( 0 != FrSM_ConfigPointer->FrSMClusters[clusterId].FrSMMinNumberOfColdstarter) {

        /* Call FrIf function to get startup numbers */
        (void)FrIf_GetNumOfStartupFrames( FrSM_ConfigPointer->FrSMClusters[clusterId].FrSMControllerRef[CONTROLLER_0].FrIfControllerId, &coldStartVal );

        if(coldStartVal < FrSM_ConfigPointer->FrSMClusters[clusterId].FrSMMinNumberOfColdstarter ) {
            /* @req FrSm187 */
            FrSM_Internal.controllerMode[clusterId].lowNumberOfColdstarters = TRUE;
        } else {
            /* @req FrSm188*/
            FrSM_Internal.controllerMode[clusterId].lowNumberOfColdstarters = FALSE;
        }
    } else {
        FrSM_Internal.controllerMode[clusterId].lowNumberOfColdstarters = FALSE;
    }
}

/**
 * @brief function does common actions required for READY to WakeUp transition
 * @param clusterId
 */
INLINE static void FrSM_Internal_ReadyToWakeup_Action(uint8 clusterId)
{
    const FrSM_ClusterType* clusterCfg;
    clusterCfg = &FrSM_ConfigPointer->FrSMClusters[clusterId];

    FrSM_Internal_TrcvNormal(clusterId);

    /* Send Wake up pattern and update the required variables */
    FrSM_Internal.controllerMode[clusterId].startupCounter = 0x01;
    /* Reset variable of WakeUp states */
    FrSM_Internal_WakeUp_Reset_Var(clusterId);
    /* Send the wakeUp pattern for first CC of Cluster */
    (void)FrIf_SendWUP(clusterCfg->FrSMControllerRef[CONTROLLER_0].FrIfControllerId);  /* Send Wake up pattern */

    /* If timer T1 is configured then start it */
    UPDATE_TIMER1(clusterId);
    /* If timer T3 is configured then start it */
    UPDATE_TIMER3(clusterId);

    FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_WAKEUP;
    FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;
    FrSM_Internal.controllerMode[clusterId].wakeupCounter++;
}

/**
 * @brief function does common action required for READY to StartUp transition
 * @param clusterId
 */
INLINE static void FrSM_Internal_ReadyToStartup_Action(uint8 clusterId)
{
     const FrSM_ClusterType* clusterCfg;
     clusterCfg = &FrSM_ConfigPointer->FrSMClusters[clusterId];
     uint8 ctlr;

    FrSM_Internal_TrcvNormal(clusterId);

    /* start with start up frame and updates required variable */
    FrSM_Internal.controllerMode[clusterId].startupCounter = 0x01;
    FrSM_Internal.controllerMode[clusterId].wakeupType = NO_WAKEUP;
    /* start the communication for all CC's of cluster */
    for(ctlr = 0; ctlr < clusterCfg->FrSMControllerCount; ctlr++) {
        (void)FrIf_StartCommunication(clusterCfg->FrSMControllerRef[ctlr].FrIfControllerId);
    }
    /* If timer T2 is configured then start it */
    UPDATE_TIMER2(clusterId);
    /* If timer T3 is configured then start it */
    UPDATE_TIMER3(clusterId);
}

/**
 * @brief function does transition from Ready to wakeup or starup state
 * @param clusterId
 */
INLINE static void FrSM_Internal_ReadyToWakeUpStartUp_Trans(uint8 clusterId)
{
    const FrSM_Internal_ClusterType* cluster;
    const FrSM_ClusterType* clusterCfg;
    cluster = &FrSM_Internal.clusterStatus[clusterId];
    clusterCfg = &FrSM_ConfigPointer->FrSMClusters[clusterId];

    if( (COMM_FULL_COMMUNICATION == cluster->reqComMode) &&
            (TRUE == clusterCfg->FrSMIsWakeupEcu) &&
            (NO_WU_BY_BUS == FrSM_Internal.controllerMode[clusterId].wuReason) ) {

        if(TRUE == clusterCfg->FrSMControllerRef[CONTROLLER_0].FrSMIsDualChannelNode) {
            /* @req FrSm151 trans T01 (b) */
            FrSM_Internal.controllerMode[clusterId].wakeupType = DUAL_CHANNEL_WAKEUP;
            /* Set wake up channel before sending wake up pattern */
            (void)FrIf_SetWakeupChannel(clusterCfg->FrSMControllerRef[CONTROLLER_0].FrIfControllerId,FR_CHANNEL_A);
        } else {
            /* @req FrSm072  trans T01 (a) */
            FrSM_Internal.controllerMode[clusterId].wakeupType = SINGLE_CHANNEL_WAKEUP;
        }

        /* Function call to send wake up pattern */
        FrSM_Internal_ReadyToWakeup_Action(clusterId);

    } else if( (COMM_FULL_COMMUNICATION == cluster->reqComMode) &&
            (TRUE == clusterCfg->FrSMIsWakeupEcu) &&
            (PARTIAL_WU_BY_BUS == FrSM_Internal.controllerMode[clusterId].wuReason) ) {

        /* @req FrSm152 trans T01 (c) */
        FrSM_Internal.controllerMode[clusterId].wakeupType = DUAL_CHANNEL_ECHO_WAKEUP;
        /* Set wake up channel before sending wake up pattern */
        (void)FrIf_SetWakeupChannel(clusterCfg->FrSMControllerRef[CONTROLLER_0].FrIfControllerId,FrSM_Internal.controllerMode[clusterId].channelType);
        /* Function call to send wake up pattern */
        FrSM_Internal_ReadyToWakeup_Action(clusterId);

    } else if((COMM_FULL_COMMUNICATION == cluster->reqComMode) &&
            ((FALSE == clusterCfg->FrSMIsWakeupEcu) || (ALL_WU_BY_BUS == FrSM_Internal.controllerMode[clusterId].wuReason))) {

        /* Function call to start with startup communication trans T02 (a) trans T02 (b)*/
        FrSM_Internal_ReadyToStartup_Action(clusterId);

        if((FALSE == clusterCfg->FrSMDelayStartupWithoutWakeup)) {
            /* @req FrSm073  trans T02 (a)*/
            FrSM_Internal_AllowColdStart(clusterId);
        } else {
            /* @req FrSm184 Trans T02 (b) */
            /* If timer T1 is configured then start it */
            UPDATE_TIMER1(clusterId);
        }
        /* transition to START UP state */
        FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_STARTUP;
        FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;
    } else {
        /* Do nothing */
    }
#if defined(USE_BSWM)
    /* Report change in cluster state to BswM module */
    if(TRUE == FrSM_Internal.clusterStatus[clusterId].bswMindication) {
        FrSM_Internal_BswM_Indication(clusterId);
    } else { /* Do Nothing */ }
#endif
}

/**
 * @brief function check wake Up action and set wakeupFinished variable to true/false
 * @param clusterId
 */
INLINE static void FrSM_Internal_WakeUp_Action(uint8 clusterId)
{
    const FrSM_ClusterType* clusterCfg;
    clusterCfg = &FrSM_ConfigPointer->FrSMClusters[clusterId];

    if(FR_POCSTATE_WAKEUP != FrSM_Internal.controllerMode[clusterId].pocCcStatus.State) {

        /* @req FrSm183 */
        if((FrSM_Internal.controllerMode[clusterId].wakeupCounter <= clusterCfg->FrSMNumWakeupPatterns) &&
                (FALSE == FrSM_Internal.controllerMode[clusterId].busTrafficDetected) ) {

            /* Proceed if POC state is in FR_POCSTATE_READY */
            if(FR_POCSTATE_READY == FrSM_Internal.controllerMode[clusterId].pocCcStatus.State) {

                if((FR_WAKEUP_RECEIVED_HEADER == FrSM_Internal.controllerMode[clusterId].pocCcStatus.WakeupStatus) ||
                        (FR_WAKEUP_RECEIVED_WUP == FrSM_Internal.controllerMode[clusterId].pocCcStatus.WakeupStatus) ) {

                    /* received a frame header/valid wakeup pattern on either channel during the initial listen phase */
                    FrSM_Internal.controllerMode[clusterId].busTrafficDetected = TRUE;
                    /* Wake Up is finished since wakeup pattern/header was recieved */
                    FrSM_Internal.controllerMode[clusterId].wakeupFinished = TRUE;
                } else if(FR_WAKEUP_TRANSMITTED == FrSM_Internal.controllerMode[clusterId].pocCcStatus.WakeupStatus) {

                    /* wakeup pattern was completely transmitted */
                    FrSM_Internal.controllerMode[clusterId].wakeupTransmitted = TRUE;
                    /* Wake Up is finished since wakeup pattern was completely transmitted */
                    FrSM_Internal.controllerMode[clusterId].wakeupFinished = TRUE;
                } else {
                    /* Wake up is not done ,try again by sending wake up pattern */
                    (void)FrIf_SendWUP(clusterCfg->FrSMControllerRef[CONTROLLER_0].FrIfControllerId);
                    /* memorize wake up attempts */
                    FrSM_Internal.controllerMode[clusterId].wakeupCounter++;
                }
            } else {
                FrSM_Internal.controllerMode[clusterId].wakeupFinished = FALSE;
            }
        } else {
            /* Do Nothing */
        }
    } else {
        /* Do Nothing */
    }
}

/**
 * @brief function to handle wake up pattern and No Com request
 * @param clusterId
 */
INLINE static void FrSM_Internal_WakeUp_Pattern(uint8 clusterId)
{
    uint8 controller;
    const FrSM_Internal_ClusterType* cluster;
    const FrSM_ClusterType* clusterCfg;
    cluster = &FrSM_Internal.clusterStatus[clusterId];
    clusterCfg = &FrSM_ConfigPointer->FrSMClusters[clusterId];

    /* @req FrSm085  trans T13 */
    if((COMM_NO_COMMUNICATION == cluster->reqComMode)) {
        /* Set TRCV mode to stand by and init all CC */
        for (controller = 0; controller < clusterCfg->FrSMControllerCount ; controller++) {

            if(TRUE == clusterCfg->FrSMControllerRef[controller].FrTrcvAvailable){
                if(TRUE == clusterCfg->FrSMControllerRef[controller].FrSMIsDualChannelNode) {
                    (void)FrIf_SetTransceiverMode(clusterCfg->FrSMControllerRef[controller].FrIfControllerId,FR_CHANNEL_A,FRTRCV_TRCVMODE_STANDBY );
                    (void)FrIf_SetTransceiverMode(clusterCfg->FrSMControllerRef[controller].FrIfControllerId,FR_CHANNEL_B,FRTRCV_TRCVMODE_STANDBY );

                } else {
                    (void)FrIf_SetTransceiverMode(clusterCfg->FrSMControllerRef[controller].FrIfControllerId,clusterCfg->FrSMControllerRef[controller].FrSMChannel, FRTRCV_TRCVMODE_STANDBY );
                }
            } else { /* Do nothing */ }

            /* Call FrIf_ControllerInit to enter into Ready state */
            (void)FrIf_ControllerInit(clusterCfg->FrSMControllerRef[controller].FrIfControllerId);
        }
        /* Cancle timerT3 and update state to FRSM_READY */
        FrSM_Internal.controllerMode[clusterId].t3Started = FALSE;
        FRSM_DEM_REPORTSTATUS((DEM_EVENT_ID_NULL == clusterCfg->FrSMClusterSyncLossDemEventId), clusterCfg->FrSMClusterSyncLossDemEventId, DEM_EVENT_STATUS_PASSED);
        FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_READY;
        FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;

    } else {
        /* proceed if timer T3 is not expired */
        if(FALSE == FrSM_Internal.controllerMode[clusterId].t3_IsNotActive) { /* @req FrSm160 trans T31 */

            switch (FrSM_Internal.controllerMode[clusterId].wakeUpAction) {
                case ECHO_MODE_OFF :
                    /* call function to check wake up is done or not */
                    FrSM_Internal_WakeUp_Action(clusterId);

                    if((TRUE == FrSM_Internal.controllerMode[clusterId].wakeupFinished ) && (DUAL_CHANNEL_ECHO_WAKEUP != FrSM_Internal.controllerMode[clusterId].wakeupType)){
                        /* Wake up for channel is done */
                        FrSM_Internal.controllerMode[clusterId].wakeUpAction = NO_ACTION;

                    } else if((TRUE == FrSM_Internal.controllerMode[clusterId].wakeupFinished ) && (FR_POCSTATE_READY == FrSM_Internal.controllerMode[clusterId].pocCcStatus.State)
                            && (COMM_FULL_COMMUNICATION == cluster->reqComMode) && (FALSE == FrSM_Internal.controllerMode[clusterId].pocCcStatus.Freeze) &&
                            (DUAL_CHANNEL_ECHO_WAKEUP == FrSM_Internal.controllerMode[clusterId].wakeupType)){

                        /* Wake up for one channel is done and start with other left channel by sending wake up pattern */
                        /* @req FrSm150  trans T103 */
                        FrSM_Internal.controllerMode[clusterId].wakeupType = DUAL_CHANNEL_WAKEUPFORWARD;
                        /* set wake up channel before sending pattern to a asleep channel */
                        if(FR_CHANNEL_A == FrSM_Internal.controllerMode[clusterId].channelType) {
                            (void)FrIf_SetWakeupChannel(clusterCfg->FrSMControllerRef[CONTROLLER_0].FrIfControllerId,FR_CHANNEL_B);
                        } else {
                            (void)FrIf_SetWakeupChannel(clusterCfg->FrSMControllerRef[CONTROLLER_0].FrIfControllerId,FR_CHANNEL_A);
                        }

                        /* Send wake up pattern and updates required variables */
                        FrSM_Internal.controllerMode[clusterId].wakeUpAction = ECHO_MODE_ON;
                        FrSM_Internal.controllerMode[clusterId].busTrafficDetected = FALSE;
                        FrSM_Internal.controllerMode[clusterId].wakeupTransmitted = FALSE;
                        FrSM_Internal.controllerMode[clusterId].wakeupFinished = FALSE;
                        (void)FrIf_SendWUP(clusterCfg->FrSMControllerRef[CONTROLLER_0].FrIfControllerId); /* send wake up pattern */
                        FrSM_Internal.controllerMode[clusterId].wakeupCounter = 1;
                    } else {
                        /* Do Nothing */
                    }
                    break;

                case ECHO_MODE_ON : /* ECHO_MODE_ON will excute only once during transition from Echo_wake up to wakeupForward */
                    /* call function to check wake up is done or not for other left channel */
                    FrSM_Internal_WakeUp_Action(clusterId);
                    if(TRUE == FrSM_Internal.controllerMode[clusterId].wakeupFinished ) {
                        /* Wake up for channel is done */
                        FrSM_Internal.controllerMode[clusterId].wakeUpAction = NO_ACTION;
                    } else {
                        /* Do Nothing */
                    }
                    break;

                default :
                    break;
            }
        } else{
            /* @req FrSm160 trans T31 */
            /* synchronisation has been fail, report to FrNm */
            FrNm_StartupError(clusterCfg->FrSMComMNetworkHandle);
            UPDATE_TIMER3(clusterId); //Reset t3 timer
            /* @req FrSm046 */
            FRSM_DET_REPORT_WITHOUT_RET(FRSM_SERVICE_ID_MAIN_FUNCTION, FRSM_E_CLUSTER_STARTUP);
            FRSM_DEM_REPORTSTATUS((DEM_EVENT_ID_NULL == clusterCfg->FrSMClusterStartUpDemEventId), clusterCfg->FrSMClusterStartUpDemEventId, DEM_EVENT_STATUS_FAILED);
        }
    }
}

/**
 * @brief function does transition from wakeup state to Start up
 * @param clusterId
 */
static void FrSM_Internal_WakeupToStartUp_Trans(uint8 clusterId)
{
    uint8 controller;
    const FrSM_Internal_ClusterType* cluster;
    const FrSM_ClusterType* clusterCfg;
    cluster = &FrSM_Internal.clusterStatus[clusterId];
    clusterCfg = &FrSM_ConfigPointer->FrSMClusters[clusterId];

    if((TRUE == FrSM_Internal.controllerMode[clusterId].wakeupFinished) && (COMM_FULL_COMMUNICATION == cluster->reqComMode)) {
        switch (FrSM_Internal.controllerMode[clusterId].wakeupType) {
            case SINGLE_CHANNEL_WAKEUP:
                if(1 == clusterCfg->FrSMNumWakeupPatterns) {
                    /* @req FrSm074 trans T03 (a) */
                    /* Update State to FRSM_READY and required varaible */
                    for (controller = 0; controller < clusterCfg->FrSMControllerCount ; controller++) {
                        (void)FrIf_StartCommunication(clusterCfg->FrSMControllerRef[controller].FrIfControllerId);
                    }
                    UPDATE_TIMER1(clusterId);
                    UPDATE_TIMER2(clusterId);
                    FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_STARTUP;
                    FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;
                } else if ((1 < clusterCfg->FrSMNumWakeupPatterns) &&
                        ((TRUE == FrSM_Internal.controllerMode[clusterId].wakeupTransmitted) || (FALSE == FrSM_Internal.controllerMode[clusterId].t1_IsActive)) ) {
                    /* @req FrSm183  trans T03 (b) */
                    for (controller = 0; controller < clusterCfg->FrSMControllerCount ; controller++) {
                        (void)FrIf_StartCommunication(clusterCfg->FrSMControllerRef[controller].FrIfControllerId);
                    }
                    FrSM_Internal.controllerMode[clusterId].t1Started = FALSE;
                    UPDATE_TIMER2(clusterId);
                    FrSM_Internal_AllowColdStart(clusterId);
                    FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_STARTUP;
                    FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;
                } else if((1 < clusterCfg->FrSMNumWakeupPatterns) && (FALSE == FrSM_Internal.controllerMode[clusterId].wakeupTransmitted)) {
                    /* @req FrSm184 T03 (c) */
                    for (controller = 0; controller < clusterCfg->FrSMControllerCount ; controller++) {
                        (void)FrIf_StartCommunication(clusterCfg->FrSMControllerRef[controller].FrIfControllerId);
                    }
                    FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_STARTUP;
                    FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;
                } else {
                    /* Do Nothing */
                }
                break;
            case DUAL_CHANNEL_WAKEUP:
                /* @req FrSm153 T03 (d) */
                for (controller = 0; controller < clusterCfg->FrSMControllerCount ; controller++) {
                    (void)FrIf_StartCommunication(clusterCfg->FrSMControllerRef[controller].FrIfControllerId);
                }
                UPDATE_TIMER2(clusterId);
                FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_STARTUP;
                FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;
                break;
            case DUAL_CHANNEL_WAKEUPFORWARD:
                /* @req FrSm154 T03 (e) */
                for (controller = 0; controller < clusterCfg->FrSMControllerCount ; controller++) {
                    (void)FrIf_StartCommunication(clusterCfg->FrSMControllerRef[controller].FrIfControllerId);
                }
                FrSM_Internal_AllowColdStart(clusterId);
                UPDATE_TIMER2(clusterId);
                FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_STARTUP;
                FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;
                break;

            default:
                break;
        }
    } else {
        /* Do Nothing */
    }
#if defined(USE_BSWM)
    /* Report change in cluster state to BswM module */
    if(TRUE == FrSM_Internal.clusterStatus[clusterId].bswMindication) {
        FrSM_Internal_BswM_Indication(clusterId);
    } else { /* Do Nothing */ }
#endif
}

/**
 * @brief function to the find the wake up reason
 * @param clusterId
 */
INLINE static void FrSM_Internal_WakeReason(uint8 clusterId)
{
    FrTrcv_TrcvWUReasonType wakeReason[2];
    const FrSM_ClusterType* clusterCfg;
    FrSM_Internal_CtrlStatusType* cluster;
    cluster = &FrSM_Internal.controllerMode[clusterId];
    clusterCfg = &FrSM_ConfigPointer->FrSMClusters[clusterId];

    if(FALSE == clusterCfg->FrSMCheckWakeupReason) {
        /* Channels not woken up by Bus */
        FrSM_Internal.controllerMode[clusterId].wuReason = NO_WU_BY_BUS;
    } else {
        if(TRUE == clusterCfg->FrSMControllerRef[CONTROLLER_0].FrSMIsDualChannelNode) {

            if(TRUE == clusterCfg->FrSMControllerRef[CONTROLLER_0].FrTrcvAvailable) {
                /* get Wakeup reasons for dual channel*/
                (void)FrIf_GetTransceiverWUReason(clusterCfg->FrSMControllerRef[CONTROLLER_0].FrIfControllerId,FR_CHANNEL_A,&wakeReason[0]);
                (void)FrIf_GetTransceiverWUReason(clusterCfg->FrSMControllerRef[CONTROLLER_0].FrIfControllerId,FR_CHANNEL_B,&wakeReason[1]);

                if((FRTRCV_WU_BY_BUS != wakeReason[0]) && (FRTRCV_WU_BY_BUS != wakeReason[1])) {
                    /* Channels not woken up by Bus */
                    FrSM_Internal.controllerMode[clusterId].wuReason = NO_WU_BY_BUS;
                } else if((FRTRCV_WU_BY_BUS == wakeReason[0]) && (FRTRCV_WU_BY_BUS != wakeReason[1])) {
                    /* Channel A is Awaken and Channel B is asleep */
                    FrSM_Internal.controllerMode[clusterId].wuReason = PARTIAL_WU_BY_BUS;
                    FrSM_Internal.controllerMode[clusterId].channelType = FR_CHANNEL_A;
                } else if((FRTRCV_WU_BY_BUS != wakeReason[0]) && (FRTRCV_WU_BY_BUS == wakeReason[1])){
                    /* Channel B is Awaken and Channel A is asleep */
                    FrSM_Internal.controllerMode[clusterId].wuReason = PARTIAL_WU_BY_BUS;
                    FrSM_Internal.controllerMode[clusterId].channelType = FR_CHANNEL_B;
                } else {
                    /* Channel A and B are Awaken */
                    FrSM_Internal.controllerMode[clusterId].wuReason = ALL_WU_BY_BUS;
                }
            } else {
                /* Get Wakeup Rx status */
                (void)FrIf_GetWakeupRxStatus(clusterCfg->FrSMControllerRef[CONTROLLER_0].FrIfControllerId,&(cluster->wakeupRxstatus));

                if(WAKEUP_CH_A_B == (WAKEUP_INDICATOR_MASK & cluster->wakeupRxstatus)) {
                    /* Channel A and B are Awaken */
                    FrSM_Internal.controllerMode[clusterId].wuReason = ALL_WU_BY_BUS;
                } else if (WAKEUP_CH_B == ((uint8)WAKEUP_INDICATOR_MASK & cluster->wakeupRxstatus))  {
                    /* Channel B is Awaken and Channel A is asleep */
                    FrSM_Internal.controllerMode[clusterId].wuReason = PARTIAL_WU_BY_BUS;
                    FrSM_Internal.controllerMode[clusterId].channelType = FR_CHANNEL_B;
                } else if(WAKEUP_CH_A == ((uint8)WAKEUP_INDICATOR_MASK & cluster->wakeupRxstatus)) {
                    /* Channel A is Awaken and Channel B is asleep */
                    FrSM_Internal.controllerMode[clusterId].wuReason = PARTIAL_WU_BY_BUS;
                    FrSM_Internal.controllerMode[clusterId].channelType = FR_CHANNEL_A;
                } else if(0x00 == ((uint8)WAKEUP_INDICATOR_MASK & cluster->wakeupRxstatus)) {
                    /* Channels not woken up by Bus */
                    FrSM_Internal.controllerMode[clusterId].wuReason = NO_WU_BY_BUS;
                } else {
                    /* Do Nothing */
                }
            }
        } else {

            if(TRUE == clusterCfg->FrSMControllerRef[CONTROLLER_0].FrTrcvAvailable) {
                /* get Wakeup reason for single channel*/
                (void)FrIf_GetTransceiverWUReason(clusterCfg->FrSMControllerRef[CONTROLLER_0].FrIfControllerId,clusterCfg->FrSMControllerRef[CONTROLLER_0].FrSMChannel,&wakeReason[0]);

                if((FRTRCV_WU_BY_BUS == wakeReason[0])) {
                    /* Channel awaken up by Bus */
                    FrSM_Internal.controllerMode[clusterId].wuReason = ALL_WU_BY_BUS;
                } else {
                    /* Channels not woken up by Bus */
                    FrSM_Internal.controllerMode[clusterId].wuReason = NO_WU_BY_BUS;
                }
            } else {
                (void)FrIf_GetWakeupRxStatus(clusterCfg->FrSMControllerRef[CONTROLLER_0].FrIfControllerId,&cluster->wakeupRxstatus);
                if((WAKEUP_CH_A == ((uint8)WAKEUP_INDICATOR_MASK & cluster->wakeupRxstatus)) ||(WAKEUP_CH_B == ((uint8)WAKEUP_INDICATOR_MASK & cluster->wakeupRxstatus)) ) {
                    /* Channel awaken up by Bus */
                    FrSM_Internal.controllerMode[clusterId].wuReason = ALL_WU_BY_BUS;
                }else {
                    /* Channels not woken up by Bus */
                    FrSM_Internal.controllerMode[clusterId].wuReason = NO_WU_BY_BUS;
                }
            }
        }
    }
}

/**
 * @brief function to find all channels waken up or not
 * @param clusterId
  */
INLINE static void FrSM_Internal_AllChannelsAWake(uint8 clusterId)
{
    const FrSM_ClusterType* clusterCfg;
    clusterCfg = &FrSM_ConfigPointer->FrSMClusters[clusterId];
    FrSM_Internal_CtrlStatusType* cluster;

    cluster = &FrSM_Internal.controllerMode[clusterId];
    (void)FrIf_GetWakeupRxStatus(clusterCfg->FrSMControllerRef[CONTROLLER_0].FrIfControllerId,&(cluster->wakeupRxstatus));

    /* Check Both Channel A and B waken or not */
    if(WAKEUP_CH_A_B == (((uint8)WAKEUP_INDICATOR_MASK & cluster->wakeupRxstatus))) {
        FrSM_Internal.controllerMode[clusterId].allChannelsAwake = TRUE;
    } else {
        FrSM_Internal.controllerMode[clusterId].allChannelsAwake = FALSE;
    }
}

/**
 * @brief function allows cold start if timer T1 is expired or Dual channels are Awaken
 * @param clusterId
 */
INLINE static void FrSM_Internal_StartUp_T1_Actions(uint8 clusterId)
{
    switch (FrSM_Internal.controllerMode[clusterId].wakeupType) {
        case SINGLE_CHANNEL_WAKEUP:
            /* @req FrSm075 trans T04 (a) */
            /* Wait till Timer1 expires before starting cold start */
            if(FALSE == FrSM_Internal.controllerMode[clusterId].t1_IsActive) {
                FrSM_Internal_AllowColdStart(clusterId);
                FrSM_Internal.controllerMode[clusterId].t2action = TRUE;
            }
            break;

        case DUAL_CHANNEL_WAKEUP:
            /* @req FrSm155 trans T04 (b) */
            /* Call function to check all channel are in awake state and start with Cold start if all channel are awaken */
            FrSM_Internal_AllChannelsAWake(clusterId);

            if(TRUE == FrSM_Internal.controllerMode[clusterId].allChannelsAwake) {
                FrSM_Internal_AllowColdStart(clusterId);
                FrSM_Internal.controllerMode[clusterId].t2action = TRUE;
            }
            break;

        default:
            break;
    }
}
/**
 * @brief Function decide whether to start START_UP state again with wakeUp/Without wakeUp.
 * @param clusterId
 */
static void FrSM_Internal_StartUp_T2_Actions(uint8 clusterId)
{
    const FrSM_ClusterType* clusterCfg;
    clusterCfg = &FrSM_ConfigPointer->FrSMClusters[clusterId];
    uint8 controller;

    if((NO_WAKEUP != FrSM_Internal.controllerMode[clusterId].wakeupType) &&
            ((FALSE == clusterCfg->FrSMStartupRepetitionsWithWakeupType) || (clusterCfg->FrSMStartupRepetitionsWithWakeup >= FrSM_Internal.controllerMode[clusterId].startupCounter)) ){

        /*startupCounter does not exceed the threshold FrSMStartupRepetitionsWithWakeup, the startup procedure will be repeated including the wakeup. */
        /* @req FrSm076 trans T05 */
        FrSM_Internal_TrcvNormal(clusterId);

        for (controller = 0; controller < clusterCfg->FrSMControllerCount ; controller++) {
            (void)FrIf_ControllerInit(clusterCfg->FrSMControllerRef[controller].FrIfControllerId);
        }
        /* Reset variable of WakeUp states */
        FrSM_Internal_WakeUp_Reset_Var(clusterId);
        (void)FrIf_SendWUP(clusterCfg->FrSMControllerRef[CONTROLLER_0].FrIfControllerId);
        FrSM_Internal.controllerMode[clusterId].startupCounter++;
        FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_WAKEUP;
        FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;
        FrSM_Internal.controllerMode[clusterId].wakeupCounter = 1; /* Wake pattern set = 1 since pattern is sent here */
        FrSM_Internal.controllerMode[clusterId].t2action = FALSE;

    } else if((( (FALSE == clusterCfg->FrSMStartupRepetitionsWithWakeupType) || (clusterCfg->FrSMStartupRepetitionsWithWakeup < FrSM_Internal.controllerMode[clusterId].startupCounter))
            || (NO_WAKEUP == FrSM_Internal.controllerMode[clusterId].wakeupType) )&&
            ( (FALSE == clusterCfg->FrSMStartupRepetitionsType) ||(clusterCfg->FrSMStartupRepetitions >= FrSM_Internal.controllerMode[clusterId].startupCounter) )) {
        /* @req FrSm077 trans T06 */
        /* startupCounter exceeds the threshold FrSMStartupRepetitionsWithWakeup but does not exceed the threshold FrSMStartupRepetitions,
         * the startup procedure will be repeated without wakeup.*/
        FrSM_Internal_TrcvNormal(clusterId);

        for (controller = 0; controller < clusterCfg->FrSMControllerCount ; controller++) {
            (void)FrIf_ControllerInit(clusterCfg->FrSMControllerRef[controller].FrIfControllerId);
            (void)FrIf_StartCommunication(clusterCfg->FrSMControllerRef[controller].FrIfControllerId);
        }
        FrSM_Internal_AllowColdStart(clusterId);
        FrSM_Internal.controllerMode[clusterId].startupCounter++;
        UPDATE_TIMER2(clusterId);

    } else {
        /* Do Nothing */
    }
}

/**
 * @brief funtion enter into Online state from StartUp state
 * @param clusterId
 */
INLINE static void FrSM_Internal_StartUp_To_Online_Key_Tras(uint8 clusterId)
{
#if defined(USE_DEM)
    const FrSM_ClusterType* clusterCfg;
    clusterCfg = &FrSM_ConfigPointer->FrSMClusters[clusterId];
#endif

    if(FR_SLOTMODE_ALL == FrSM_Internal.controllerMode[clusterId].pocCcStatus.SlotMode) {

        /* @req FrSm079 trans T08 */
        FrSM_Internal.controllerMode[clusterId].t1Started = FALSE;
        FrSM_Internal.controllerMode[clusterId].t2Started = FALSE;
        (void)FrIf_SetState(clusterId, FRIF_GOTO_ONLINE);
        FRSM_DEM_REPORTSTATUS((DEM_EVENT_ID_NULL == clusterCfg->FrSMClusterStartUpDemEventId), clusterCfg->FrSMClusterStartUpDemEventId, DEM_EVENT_STATUS_PASSED);
        FRSM_DEM_REPORTSTATUS((DEM_EVENT_ID_NULL == clusterCfg->FrSMClusterSyncLossDemEventId), clusterCfg->FrSMClusterSyncLossDemEventId, DEM_EVENT_STATUS_PASSED);
        FrSM_Internal_ComM_BusSM_ModeIndication(clusterId,COMM_FULL_COMMUNICATION);
        FrSM_Internal.controllerMode[clusterId].t3Started = FALSE;
        FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_ONLINE;
        FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;
        FrSM_Internal.controllerMode[clusterId].t2action = FALSE;
    } else if(FR_SLOTMODE_KEYSLOT == FrSM_Internal.controllerMode[clusterId].pocCcStatus.SlotMode){

        /* @req FrSm156 trans T108 */
        FrSM_Internal.controllerMode[clusterId].t1Started = FALSE;
        FrSM_Internal.controllerMode[clusterId].t2Started = FALSE;
        (void)FrIf_SetState(clusterId, FRIF_GOTO_ONLINE);
        FRSM_DEM_REPORTSTATUS((DEM_EVENT_ID_NULL == clusterCfg->FrSMClusterStartUpDemEventId), clusterCfg->FrSMClusterStartUpDemEventId, DEM_EVENT_STATUS_PASSED);
        FRSM_DEM_REPORTSTATUS((DEM_EVENT_ID_NULL == clusterCfg->FrSMClusterSyncLossDemEventId), clusterCfg->FrSMClusterSyncLossDemEventId, DEM_EVENT_STATUS_PASSED);
        FrSM_Internal.controllerMode[clusterId].t3Started = FALSE;
        FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_KEYSLOT_ONLY;
        FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;
        FrSM_Internal.controllerMode[clusterId].t2action = FALSE;
    } else {
        /* Do Nothing */
    }
}

/**
 * @brief Function does transition from startup to other states
 * @param clusterId
 */
static void FrSM_Internal_StartUp_Trans(uint8 clusterId)
{
    uint8 controller;
    const FrSM_Internal_ClusterType* cluster;
    const FrSM_ClusterType* clusterCfg;
    cluster = &FrSM_Internal.clusterStatus[clusterId];
    clusterCfg = &FrSM_ConfigPointer->FrSMClusters[clusterId];

    if(COMM_NO_COMMUNICATION == cluster->reqComMode) {
        /* @req FrSm084 trans T12 */
        for (controller = 0; controller < clusterCfg->FrSMControllerCount ; controller++) {
            if(TRUE == clusterCfg->FrSMControllerRef[controller].FrTrcvAvailable){
                if(TRUE == clusterCfg->FrSMControllerRef[controller].FrSMIsDualChannelNode) {
                    (void)FrIf_SetTransceiverMode(clusterCfg->FrSMControllerRef[controller].FrIfControllerId,FR_CHANNEL_A,FRTRCV_TRCVMODE_STANDBY );
                    (void)FrIf_SetTransceiverMode(clusterCfg->FrSMControllerRef[controller].FrIfControllerId,FR_CHANNEL_B,FRTRCV_TRCVMODE_STANDBY );

                } else {
                    (void)FrIf_SetTransceiverMode(clusterCfg->FrSMControllerRef[controller].FrIfControllerId,clusterCfg->FrSMControllerRef[controller].FrSMChannel, FRTRCV_TRCVMODE_STANDBY );
                }
            }
        }
        /* change state to FRSM_READY and updates required Variables */
        for (controller = 0; controller < clusterCfg->FrSMControllerCount ; controller++) {
            (void)FrIf_ControllerInit(clusterCfg->FrSMControllerRef[controller].FrIfControllerId);
        }
        FrSM_Internal.controllerMode[clusterId].t1Started = FALSE;
        FrSM_Internal.controllerMode[clusterId].t2Started = FALSE;
        FrSM_Internal.controllerMode[clusterId].t3Started = FALSE;
        FRSM_DEM_REPORTSTATUS((DEM_EVENT_ID_NULL == clusterCfg->FrSMClusterSyncLossDemEventId), clusterCfg->FrSMClusterSyncLossDemEventId, DEM_EVENT_STATUS_PASSED);
        FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_READY;
        FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;
        FrSM_Internal.controllerMode[clusterId].t2action = FALSE;
    } else {
        /* proceed if timer T3 is not expired.  trans T32*/
        if(FALSE == FrSM_Internal.controllerMode[clusterId].t3_IsNotActive) {

            if((FR_POCSTATE_NORMAL_ACTIVE != FrSM_Internal.controllerMode[clusterId].pocCcStatus.State) && (COMM_FULL_COMMUNICATION == cluster->reqComMode)) {

                if((FALSE == FrSM_Internal.controllerMode[clusterId].t2action) && ((SINGLE_CHANNEL_WAKEUP == FrSM_Internal.controllerMode[clusterId].wakeupType) ||
                       (DUAL_CHANNEL_WAKEUP == FrSM_Internal.controllerMode[clusterId].wakeupType)) ) {
                    /* if timer T1 is expired or Dual channel Awaken then allow coldstart */
                    FrSM_Internal_StartUp_T1_Actions(clusterId);
                } else {
                    /* if timer T2 is expired then repeat the startup procedure depending on the value of the startupCounter:*/
                    if(FALSE == FrSM_Internal.controllerMode[clusterId].t2_IsActive) {
                        FrSM_Internal_StartUp_T2_Actions(clusterId);
                        FrSM_Internal.controllerMode[clusterId].t2action = FALSE;
                    }
                }
            } else if((FR_POCSTATE_NORMAL_ACTIVE == FrSM_Internal.controllerMode[clusterId].pocCcStatus.State) &&
                    (FALSE == FrSM_Internal.controllerMode[clusterId].pocCcStatus.Freeze)  &&
                    (COMM_FULL_COMMUNICATION == cluster->reqComMode) ) {

                /* cluster state changes to Online or Key slot only */
                FrSM_Internal_StartUp_To_Online_Key_Tras(clusterId);
            } else {
                /* Do Nothing */
            }
        } else {
            /* @req FrSm161  trans T32 */
            /* synchronisation has been fail, report to FrNm */
            FrNm_StartupError(clusterCfg->FrSMComMNetworkHandle);
            UPDATE_TIMER3(clusterId); //Reset t3 timer
            /* @req FrSm046 */
            FRSM_DET_REPORT_WITHOUT_RET(FRSM_SERVICE_ID_MAIN_FUNCTION, FRSM_E_CLUSTER_STARTUP);
            FRSM_DEM_REPORTSTATUS((DEM_EVENT_ID_NULL == clusterCfg->FrSMClusterStartUpDemEventId), clusterCfg->FrSMClusterStartUpDemEventId, DEM_EVENT_STATUS_FAILED);
        }
    }
#if defined(USE_BSWM)
    /* Report change in cluster state to BswM module */
    if(TRUE == FrSM_Internal.clusterStatus[clusterId].bswMindication) {
        FrSM_Internal_BswM_Indication(clusterId);
    }
#endif
}

/**
 * @brief function to enter into Halt state
 * @param clusterId
 */
INLINE static void FrSM_Internal_HaltState_Trans(uint8 clusterId)
{
    const FrSM_Internal_ClusterType* cluster;
    const FrSM_ClusterType* clusterCfg;
    cluster = &FrSM_Internal.clusterStatus[clusterId];
    clusterCfg = &FrSM_ConfigPointer->FrSMClusters[clusterId];
    uint8 controller;

    if(FRSM_KEYSLOT_ONLY == cluster->frSMClusterState) {
        /* @req FrSm080 T09 (b)*/
        (void)FrIf_SetState(clusterId, FRIF_GOTO_OFFLINE);
        for (controller = 0; controller < clusterCfg->FrSMControllerCount ; controller++) {
            (void)FrIf_HaltCommunication(clusterCfg->FrSMControllerRef[controller].FrIfControllerId);
        }
    } else if(FRSM_ONLINE_PASSIVE == cluster->frSMClusterState) {
        /* @req FrSm125  Trans T14 */
        FRSM_DEM_REPORTSTATUS((DEM_EVENT_ID_NULL == clusterCfg->FrSMClusterSyncLossDemEventId), clusterCfg->FrSMClusterSyncLossDemEventId, DEM_EVENT_STATUS_PASSED);
        for (controller = 0; controller < clusterCfg->FrSMControllerCount ; controller++) {
            (void)FrIf_HaltCommunication(clusterCfg->FrSMControllerRef[controller].FrIfControllerId);
        }
        FrSM_Internal.controllerMode[clusterId].t3Started = FALSE;
    } else {
        /* Do Nothing */
    }
    /* Update state to HALT_REQ */
    FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_HALT_REQ;
    FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;
}

/**
 * @brief function call to return WakeUp state
 * @param clusterId
 */
INLINE static void FrSM_Internal_Return_WakeUp_state(uint8 clusterId)
{
    uint8 controller;
    const FrSM_Internal_ClusterType* cluster;
    const FrSM_ClusterType* clusterCfg;
    cluster = &FrSM_Internal.clusterStatus[clusterId];
    clusterCfg = &FrSM_ConfigPointer->FrSMClusters[clusterId];

    FrSM_Internal.controllerMode[clusterId].wakeupType = SINGLE_CHANNEL_WAKEUP;
    if((FRSM_ONLINE == cluster->frSMClusterState) || (FRSM_LOW_NUMBER_OF_COLDSTARTERS == cluster->frSMClusterState)) {
        /* @req FrSm0200 T20 (a) */
        (void)FrIf_SetState(clusterId, FRIF_GOTO_OFFLINE);
        FrSM_Internal_ComM_BusSM_ModeIndication(clusterId,COMM_NO_COMMUNICATION );

    } else if(FRSM_KEYSLOT_ONLY == cluster->frSMClusterState) {
        /* @req FrSm0200 T20 (b) */
        (void)FrIf_SetState(clusterId, FRIF_GOTO_OFFLINE);
    } else {
        /* @req FrSm0201 T20 (c)*/
    }
    for (controller = 0; controller < clusterCfg->FrSMControllerCount ; controller++) {
        (void)FrIf_ControllerInit(clusterCfg->FrSMControllerRef[controller].FrIfControllerId);
    }
    (void)FrIf_SendWUP(clusterCfg->FrSMControllerRef[CONTROLLER_0].FrIfControllerId);
    FrSM_Internal.controllerMode[clusterId].startupCounter = 1;
    UPDATE_TIMER1(clusterId);
    UPDATE_TIMER3(clusterId);
}
/**
 * @brief function call to transition from online to coldstart vice versa
 * @param clusterId
 */
INLINE static void FrSM_Internal_Online_ColdStart_trans(uint8 clusterId)
{
    if(TRUE == FrSM_Internal.controllerMode[clusterId].lowNumberOfColdstarters) {
        /* @req FrSm187 Trans T40 */
        if(FRSM_LOW_NUMBER_OF_COLDSTARTERS != FrSM_Internal.clusterStatus[clusterId].frSMClusterState) {
            FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;
            FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_LOW_NUMBER_OF_COLDSTARTERS;
        }
    } else {
        /* @req FrSm188 Trans T41 */
        if(FRSM_ONLINE != FrSM_Internal.clusterStatus[clusterId].frSMClusterState) {
            FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;
            FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_ONLINE;
        }
    }
}
/**
 * @brief function performs online State actions
 * @param clusterId
 */
static void FrSM_Internal_Online_Trans(uint8 clusterId)
{
    uint8 controller;
    const FrSM_Internal_ClusterType* cluster;
    const FrSM_ClusterType* clusterCfg;
    cluster = &FrSM_Internal.clusterStatus[clusterId];
    clusterCfg = &FrSM_ConfigPointer->FrSMClusters[clusterId];

    if(cluster->reqComMode == COMM_FULL_COMMUNICATION) {

        if(((FR_POCSTATE_HALT == FrSM_Internal.controllerMode[clusterId].pocCcStatus.State) ||
                (TRUE == FrSM_Internal.controllerMode[clusterId].pocCcStatus.Freeze))  &&
                (TRUE == clusterCfg->FrSMCheckWakeupReason) ) {

            /* @req FrSm081  Trans T10 (a)*/
            FRSM_DET_REPORT_WITHOUT_RET(FRSM_SERVICE_ID_MAIN_FUNCTION, FRSM_E_CLUSTER_SYNC_LOSS);
            FRSM_DEM_REPORTSTATUS((DEM_EVENT_ID_NULL == clusterCfg->FrSMClusterSyncLossDemEventId), clusterCfg->FrSMClusterSyncLossDemEventId, DEM_EVENT_STATUS_FAILED);
            (void)FrIf_SetState(clusterId, FRIF_GOTO_OFFLINE);
            FrSM_Internal_ComM_BusSM_ModeIndication(clusterId,COMM_NO_COMMUNICATION);
            for (controller = 0; controller < clusterCfg->FrSMControllerCount ; controller++) {
                (void)FrIf_ControllerInit(clusterCfg->FrSMControllerRef[controller].FrIfControllerId);
                (void)FrIf_StartCommunication(clusterCfg->FrSMControllerRef[controller].FrIfControllerId);
            }
            FrSM_Internal.controllerMode[clusterId].startupCounter = 1;
            UPDATE_TIMER2(clusterId);
            UPDATE_TIMER3(clusterId);
            FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_STARTUP;
            FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;

        } else if((FR_POCSTATE_NORMAL_PASSIVE == FrSM_Internal.controllerMode[clusterId].pocCcStatus.State) &&
                (FALSE == FrSM_Internal.controllerMode[clusterId].pocCcStatus.Freeze)) {

            /* @req FrSm087 Trans T16 (a)*/
            FRSM_DET_REPORT_WITHOUT_RET(FRSM_SERVICE_ID_MAIN_FUNCTION, FRSM_E_CLUSTER_SYNC_LOSS);
            FRSM_DEM_REPORTSTATUS((DEM_EVENT_ID_NULL == clusterCfg->FrSMClusterSyncLossDemEventId), clusterCfg->FrSMClusterSyncLossDemEventId, DEM_EVENT_STATUS_FAILED);
            (void)FrIf_SetState(clusterId, FRIF_GOTO_OFFLINE);
            FrSM_Internal_ComM_BusSM_ModeIndication(clusterId,COMM_NO_COMMUNICATION );
            UPDATE_TIMER3(clusterId);
            FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_ONLINE_PASSIVE;
            FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;

        } else if(((FR_POCSTATE_HALT == FrSM_Internal.controllerMode[clusterId].pocCcStatus.State) ||
                (TRUE == FrSM_Internal.controllerMode[clusterId].pocCcStatus.Freeze))  &&
                (FALSE == clusterCfg->FrSMCheckWakeupReason) && (TRUE == clusterCfg->FrSMIsWakeupEcu) ) {

            /* @req FrSm0200 */ /* Trans T20 (a) */
            FRSM_DET_REPORT_WITHOUT_RET(FRSM_SERVICE_ID_MAIN_FUNCTION, FRSM_E_CLUSTER_SYNC_LOSS);
            FRSM_DEM_REPORTSTATUS((DEM_EVENT_ID_NULL == clusterCfg->FrSMClusterSyncLossDemEventId), clusterCfg->FrSMClusterSyncLossDemEventId, DEM_EVENT_STATUS_FAILED);
            FrSM_Internal_Return_WakeUp_state(clusterId);
            FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_WAKEUP;
            FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;
            /* Reset Variables of WakeUp state to repeat wakeUp state */
            FrSM_Internal_WakeUp_Reset_Var(clusterId);
            FrSM_Internal.controllerMode[clusterId].wakeupCounter = 1;

        } else {

            FrSM_Internal_Online_ColdStart_trans(clusterId);
        }
    }
#if defined(USE_BSWM)
    if(TRUE == FrSM_Internal.clusterStatus[clusterId].bswMindication) {
        FrSM_Internal_BswM_Indication(clusterId);
    } else { /* Do Nothing */ }
#endif
}
/**
 * @brief function performs Keyslot State actions
 * @param clusterId
 */
static void FrSM_Internal_KeySlot_Actions(uint8 clusterId)
{
    uint8 controller;
    const FrSM_Internal_ClusterType* cluster;
    const FrSM_ClusterType* clusterCfg;
    cluster = &FrSM_Internal.clusterStatus[clusterId];
    clusterCfg = &FrSM_ConfigPointer->FrSMClusters[clusterId];

    if(COMM_NO_COMMUNICATION == cluster->reqComMode) {
        /* Function call to enter into HALT state -Trans T09 (b)*/
        FrSM_Internal_HaltState_Trans(clusterId);
    } else {
        if(((FR_POCSTATE_HALT == FrSM_Internal.controllerMode[clusterId].pocCcStatus.State) ||
                (TRUE == FrSM_Internal.controllerMode[clusterId].pocCcStatus.Freeze))  &&
                (TRUE == clusterCfg->FrSMCheckWakeupReason) ) {

            /* @req FrSm081  Trans T10 (b) */
            FRSM_DET_REPORT_WITHOUT_RET(FRSM_SERVICE_ID_MAIN_FUNCTION, FRSM_E_CLUSTER_SYNC_LOSS);
            FRSM_DEM_REPORTSTATUS((DEM_EVENT_ID_NULL == clusterCfg->FrSMClusterSyncLossDemEventId), clusterCfg->FrSMClusterSyncLossDemEventId, DEM_EVENT_STATUS_FAILED);
            (void)FrIf_SetState(clusterId, FRIF_GOTO_OFFLINE);
            for (controller = 0; controller < clusterCfg->FrSMControllerCount ; controller++) {
                (void)FrIf_ControllerInit(clusterCfg->FrSMControllerRef[controller].FrIfControllerId);
                (void)FrIf_StartCommunication(clusterCfg->FrSMControllerRef[controller].FrIfControllerId);
            }
            FrSM_Internal.controllerMode[clusterId].startupCounter = 1;
            UPDATE_TIMER2(clusterId);
            UPDATE_TIMER3(clusterId);
            FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_STARTUP;
            FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;

        } else if((FR_POCSTATE_NORMAL_PASSIVE == FrSM_Internal.controllerMode[clusterId].pocCcStatus.State) &&
                (FALSE == FrSM_Internal.controllerMode[clusterId].pocCcStatus.Freeze)) {

            /* @req FrSm087 Trans T16 (b)*/
            FRSM_DET_REPORT_WITHOUT_RET(FRSM_SERVICE_ID_MAIN_FUNCTION, FRSM_E_CLUSTER_SYNC_LOSS);
            FRSM_DEM_REPORTSTATUS((DEM_EVENT_ID_NULL == clusterCfg->FrSMClusterSyncLossDemEventId), clusterCfg->FrSMClusterSyncLossDemEventId, DEM_EVENT_STATUS_FAILED);
            (void)FrIf_SetState(clusterId, FRIF_GOTO_OFFLINE);
            UPDATE_TIMER3(clusterId);
            FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_ONLINE_PASSIVE;
            FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;

        } else if(((FR_POCSTATE_HALT == FrSM_Internal.controllerMode[clusterId].pocCcStatus.State) ||
                (TRUE ==FrSM_Internal.controllerMode[clusterId].pocCcStatus.Freeze))  &&
                (FALSE == clusterCfg->FrSMCheckWakeupReason) && (TRUE == clusterCfg->FrSMIsWakeupEcu) ) {

            /* @req FrSm0200 */ /* Trans T20 (b) */
            FRSM_DET_REPORT_WITHOUT_RET(FRSM_SERVICE_ID_MAIN_FUNCTION, FRSM_E_CLUSTER_SYNC_LOSS );
            FRSM_DEM_REPORTSTATUS((DEM_EVENT_ID_NULL == clusterCfg->FrSMClusterSyncLossDemEventId), clusterCfg->FrSMClusterSyncLossDemEventId, DEM_EVENT_STATUS_FAILED);
            FrSM_Internal_Return_WakeUp_state(clusterId);
            FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_WAKEUP;
            FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;
            /* Reset Variables of WakeUp state to repeat wakeUp state */
            FrSM_Internal_WakeUp_Reset_Var(clusterId);
            FrSM_Internal.controllerMode[clusterId].wakeupCounter = 1;

        } else if((FR_POCSTATE_NORMAL_ACTIVE == FrSM_Internal.controllerMode[clusterId].pocCcStatus.State) &&
                (FALSE == FrSM_Internal.controllerMode[clusterId].pocCcStatus.Freeze) &&
                (FR_SLOTMODE_ALL == FrSM_Internal.controllerMode[clusterId].pocCcStatus.SlotMode)) {

            /* @req FrSm157 Trans T101 */
            FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_ONLINE;
            FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;
            FrSM_Internal_ComM_BusSM_ModeIndication(clusterId,COMM_FULL_COMMUNICATION);
        } else { /* Do Nothing */ }
    }
#if defined(USE_BSWM)
    if(TRUE == FrSM_Internal.clusterStatus[clusterId].bswMindication) {
        FrSM_Internal_BswM_Indication(clusterId);
    } else { /* Do Nothing */ }
#endif
}
/**
 * @brief function performs online Passive State actions
 * @param clusterId
 */
static void FrSM_Internal_Online_Passive_Actions(uint8 clusterId)
{
    uint8 controller;
    const FrSM_Internal_ClusterType* cluster;
    const FrSM_ClusterType* clusterCfg;
    cluster = &FrSM_Internal.clusterStatus[clusterId];
    clusterCfg = &FrSM_ConfigPointer->FrSMClusters[clusterId];

    if(COMM_NO_COMMUNICATION == cluster->reqComMode) {
        /* Function call to enter into HALT state - Trans T14*/
        FrSM_Internal_HaltState_Trans(clusterId);
    } else {
        /* proceed if timer T3 is not expired.  trans T33*/
        /* @req FrSm173 */
        if(FALSE == FrSM_Internal.controllerMode[clusterId].t3_IsNotActive) {

            if(((FR_POCSTATE_HALT == FrSM_Internal.controllerMode[clusterId].pocCcStatus.State) ||
                    (TRUE == FrSM_Internal.controllerMode[clusterId].pocCcStatus.Freeze))  &&
                    (TRUE == clusterCfg->FrSMCheckWakeupReason) ) {

                /* @req FrSm117 Trans T17 */
                FrSM_Internal.controllerMode[clusterId].wakeupType = NO_WAKEUP;
                for (controller = 0; controller < clusterCfg->FrSMControllerCount ; controller++) {
                    (void)FrIf_ControllerInit(clusterCfg->FrSMControllerRef[controller].FrIfControllerId);
                    (void)FrIf_StartCommunication(clusterCfg->FrSMControllerRef[controller].FrIfControllerId);
                }
                FrSM_Internal.controllerMode[clusterId].startupCounter = 1;
                UPDATE_TIMER2(clusterId);
                FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_STARTUP;
                FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;
            } else if(((FR_POCSTATE_HALT == FrSM_Internal.controllerMode[clusterId].pocCcStatus.State) ||
                    (TRUE == FrSM_Internal.controllerMode[clusterId].pocCcStatus.Freeze))  &&
                    (FALSE == clusterCfg->FrSMCheckWakeupReason) && (TRUE == clusterCfg->FrSMIsWakeupEcu) ) {

                /* @req FrSm0201 */ /* Trans 20 (c) */
                FrSM_Internal_Return_WakeUp_state(clusterId);
                FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_WAKEUP;
                FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;
                /* Reset Variables of WakeUp state to repeat wakeUp state */
                FrSM_Internal_WakeUp_Reset_Var(clusterId);
                FrSM_Internal.controllerMode[clusterId].wakeupCounter = 1;
            } else if((FR_POCSTATE_NORMAL_ACTIVE == FrSM_Internal.controllerMode[clusterId].pocCcStatus.State) &&
                    (FALSE == FrSM_Internal.controllerMode[clusterId].pocCcStatus.Freeze) &&
                    (FR_SLOTMODE_ALL == FrSM_Internal.controllerMode[clusterId].pocCcStatus.SlotMode)) {

                /* @req FrSm086 Trans T15 */
                (void)FrIf_SetState(clusterId, FRIF_GOTO_ONLINE);
                FRSM_DEM_REPORTSTATUS((DEM_EVENT_ID_NULL == clusterCfg->FrSMClusterSyncLossDemEventId), clusterCfg->FrSMClusterSyncLossDemEventId, DEM_EVENT_STATUS_PASSED);
                FrSM_Internal_ComM_BusSM_ModeIndication(clusterId,COMM_FULL_COMMUNICATION);
                FrSM_Internal.controllerMode[clusterId].t3Started = FALSE;
                FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_ONLINE;
                FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;
            } else if((FR_POCSTATE_NORMAL_ACTIVE == FrSM_Internal.controllerMode[clusterId].pocCcStatus.State) &&
                    (FALSE == FrSM_Internal.controllerMode[clusterId].pocCcStatus.Freeze) &&
                    (FR_SLOTMODE_ALL != FrSM_Internal.controllerMode[clusterId].pocCcStatus.SlotMode)) {

                /* @req FrSm158 Trans T115 */
                (void)FrIf_SetState(clusterId, FRIF_GOTO_ONLINE);
                FRSM_DEM_REPORTSTATUS((DEM_EVENT_ID_NULL == clusterCfg->FrSMClusterSyncLossDemEventId), clusterCfg->FrSMClusterSyncLossDemEventId, DEM_EVENT_STATUS_PASSED);
                FrSM_Internal.controllerMode[clusterId].t3Started = FALSE;
                FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_KEYSLOT_ONLY;
                FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;
            }
            else {
                /* Do Nothing */
            }
        } else {
            /* @req FrSm173  trans T33 */
            /* synchronisation has been fail, report to FrNm */
            FrNm_StartupError(clusterCfg->FrSMComMNetworkHandle);
            UPDATE_TIMER3(clusterId); //Reset t3 timer
            /* @req FrSm046 */
            FRSM_DET_REPORT_WITHOUT_RET(FRSM_SERVICE_ID_MAIN_FUNCTION, FRSM_E_CLUSTER_STARTUP);
            /* the timer t3 expires, the FrSM will report the production error */
            FRSM_DEM_REPORTSTATUS((DEM_EVENT_ID_NULL == clusterCfg->FrSMClusterStartUpDemEventId), clusterCfg->FrSMClusterStartUpDemEventId, DEM_EVENT_STATUS_FAILED);
        }
    }
#if defined(USE_BSWM)
    if(TRUE == FrSM_Internal.clusterStatus[clusterId].bswMindication) {
        FrSM_Internal_BswM_Indication(clusterId);
    } else { /* Do Nothing */ }
#endif
}

/**
 * @brief Function does transition from halt state to Ready state
 * @param clusterId
 */
static void FrSM_Internal_Halt_State_Actions(uint8 clusterId)
{
    uint8 controller;
    const FrSM_ClusterType* clusterCfg;
    clusterCfg = &FrSM_ConfigPointer->FrSMClusters[clusterId];

    if((FR_POCSTATE_HALT == FrSM_Internal.controllerMode[clusterId].pocCcStatus.State) ||
            (TRUE == FrSM_Internal.controllerMode[clusterId].pocCcStatus.Freeze) ) {

        /* @req FrSm083 */ /* Transition T11 */
        /* Set TRCV mode to stand by and init all CC */
        SchM_Enter_FrSM_EA_0(); /* To satisfy the FrSm178 requirement - added the critical section */

        for (controller = 0; controller < clusterCfg->FrSMControllerCount ; controller++) {

            if(TRUE == clusterCfg->FrSMControllerRef[controller].FrTrcvAvailable){
                if(TRUE == clusterCfg->FrSMControllerRef[controller].FrSMIsDualChannelNode) {
                    (void)FrIf_SetTransceiverMode(clusterCfg->FrSMControllerRef[controller].FrIfControllerId,FR_CHANNEL_A,FRTRCV_TRCVMODE_STANDBY );
                    (void)FrIf_SetTransceiverMode(clusterCfg->FrSMControllerRef[controller].FrIfControllerId,FR_CHANNEL_B,FRTRCV_TRCVMODE_STANDBY );

                } else {
                    (void)FrIf_SetTransceiverMode(clusterCfg->FrSMControllerRef[controller].FrIfControllerId,clusterCfg->FrSMControllerRef[controller].FrSMChannel, FRTRCV_TRCVMODE_STANDBY );
                }
            } else { /* Do Nothing */ }

            /* Call FrIf_ControllerInit to enter into Ready state */
            (void)FrIf_ControllerInit(clusterCfg->FrSMControllerRef[controller].FrIfControllerId);
        }
        FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_READY;
        FrSM_Internal.clusterStatus[clusterId].bswMindication = TRUE;

        SchM_Exit_FrSM_EA_0(); /* Enable interrupt after state has been changed to READY */
    } else {
        /* Do Nothing */
    }
#if defined(USE_BSWM)
    if(TRUE == FrSM_Internal.clusterStatus[clusterId].bswMindication) {
        FrSM_Internal_BswM_Indication(clusterId);
    } else { /* Do Nothing */ }
#endif
}
/** APIs */
/* Init function for FrSM */
/* @req FrSm013 */
/**
 *
 * @param FrSM_ConfigPtr
 */
void FrSM_Init( const FrSM_ConfigType* FrSM_ConfigPtr )
{
    uint8 clstrCount;
    uint8 controller;

    /* @req FrSm015 */
    FRSM_DET_REPORTERROR((NULL != FrSM_ConfigPtr),FRSM_SERVICE_ID_INIT,FRSM_E_NULL_PTR);

    /* @req FrSm127 */
    FrSM_ConfigPointer = FrSM_ConfigPtr;

    /* loop for number of cluster and initialize all variables */
    for ( clstrCount = 0; clstrCount < FrSM_ConfigPointer->FrSMClusterCount; ++clstrCount) {
        FrSM_Internal.clusterStatus[clstrCount].reqComMode = COMM_NO_COMMUNICATION;
        FrSM_Internal.clusterStatus[clstrCount].curComMode = COMM_NO_COMMUNICATION;
        FrSM_Internal.clusterStatus[clstrCount].frSMClusterPassiveState = FRSM_ECU_ACTIVE;
        FrSM_Internal.clusterStatus[clstrCount].bswMindication  = FALSE;

        /* First CC of several CC's cluster are set to default values */
        FrSM_Internal.controllerMode[clstrCount].controllerId = FrSM_ConfigPointer->FrSMClusters[clstrCount].FrSMControllerRef[CONTROLLER_0].FrIfControllerId;
        FrSM_Internal.controllerMode[clstrCount].wakeUpAction = ECHO_MODE_OFF;
        FrSM_Internal.controllerMode[clstrCount].wakeupFinished = FALSE;
        FrSM_Internal.controllerMode[clstrCount].startupCounter = 0;
        FrSM_Internal.controllerMode[clstrCount].t1_IsActive    = FALSE;
        FrSM_Internal.controllerMode[clstrCount].timerT1        = 0;
        FrSM_Internal.controllerMode[clstrCount].t2_IsActive    = FALSE;
        FrSM_Internal.controllerMode[clstrCount].timerT2        = 0;
        FrSM_Internal.controllerMode[clstrCount].t3_IsNotActive = TRUE;
        FrSM_Internal.controllerMode[clstrCount].timerT3        = 0;
        FrSM_Internal.controllerMode[clstrCount].wakeupCounter  = 0;
        FrSM_Internal.controllerMode[clstrCount].busTrafficDetected = FALSE;
        FrSM_Internal.controllerMode[clstrCount].wakeupTransmitted  = FALSE;
        FrSM_Internal.controllerMode[clstrCount].t2action           = FALSE;

        for(controller = 0; controller < FrSM_ConfigPointer->FrSMClusters[clstrCount].FrSMControllerCount; controller++) {
            /* @req FrSm119 */
            (void)FrIf_ControllerInit(FrSM_ConfigPointer->FrSMClusters[clstrCount].FrSMControllerRef[controller].FrIfControllerId);

             /* @req FrSm176 */ /* Check TRCV enabled for specific CC in Cluster */
            if(TRUE == FrSM_ConfigPointer->FrSMClusters[clstrCount].FrSMControllerRef[controller].FrTrcvAvailable) {
                /* FrSM is concurrently in state FRSM_READY, the transceivers are in set into mode FRTRCV_TRCVMODE_STANDBY */
                if(TRUE == FrSM_ConfigPointer->FrSMClusters[clstrCount].FrSMControllerRef[controller].FrSMIsDualChannelNode) {
                    (void)FrIf_SetTransceiverMode(FrSM_ConfigPointer->FrSMClusters[clstrCount].FrSMControllerRef[controller].FrIfControllerId,FR_CHANNEL_A,FRTRCV_TRCVMODE_STANDBY );
                    (void)FrIf_SetTransceiverMode(FrSM_ConfigPointer->FrSMClusters[clstrCount].FrSMControllerRef[controller].FrIfControllerId,FR_CHANNEL_B,FRTRCV_TRCVMODE_STANDBY );
                } else
                {
                    (void)FrIf_SetTransceiverMode(FrSM_ConfigPointer->FrSMClusters[clstrCount].FrSMControllerRef[controller].FrIfControllerId,
                            FrSM_ConfigPointer->FrSMClusters[clstrCount].FrSMControllerRef[controller].FrSMChannel, FRTRCV_TRCVMODE_STANDBY );
                }
            }
        }
        /* @req FrSm126 */
        FrSM_Internal.clusterStatus[clstrCount].frSMClusterState = FRSM_READY;
    }
    /* @req FrSm128 */
    FrSM_Internal.initStatus = FRSM_STATUS_INIT;
}

/* @req FrSm020 */
/**
 *
 * @param NetworkHandle
 * @param ComM_Mode
 * @return
 */
Std_ReturnType FrSM_RequestComMode( NetworkHandleType NetworkHandle, ComM_ModeType ComM_Mode )
{
    uint8 controller;
    uint8 clusterId;
    /* @req FrSm061 */
    FRSM_DET_REPORTERROR((FRSM_STATUS_INIT == FrSM_Internal.initStatus),FRSM_SERVICE_ID_REQUEST_COM_MODE, FRSM_E_UNINIT, E_NOT_OK);

    /* @req FrSm018 */
    FRSM_DET_REPORTERROR(((NetworkHandle < COMM_CHANNEL_COUNT) && (FrSM_ConfigPointer->FrSMClusterLookups[NetworkHandle] != INVALID_FR_NW_HANDLE)),FRSM_SERVICE_ID_REQUEST_COM_MODE,FRSM_E_INV_HANDLE,E_NOT_OK);

    /* @req FrSm019 */
    FRSM_DET_REPORTERROR(!((COMM_NO_COMMUNICATION != ComM_Mode) && (COMM_SILENT_COMMUNICATION != ComM_Mode) && (COMM_FULL_COMMUNICATION != ComM_Mode)),FRSM_SERVICE_ID_REQUEST_COM_MODE,FRSM_E_INV_MODE,E_NOT_OK);

    /* @req FrSm141 */
    FRSM_DET_REPORTERROR((COMM_SILENT_COMMUNICATION != ComM_Mode),FRSM_SERVICE_ID_REQUEST_COM_MODE,FRSM_E_INV_MODE,E_NOT_OK);

    /* @req FrSm021 */
    clusterId = FrSM_ConfigPointer->FrSMClusterLookups[NetworkHandle];
    FrSM_Internal.clusterStatus[clusterId].reqComMode = ComM_Mode;

    /* @req FrSm022 */
    if(( COMM_NO_COMMUNICATION == ComM_Mode) && (COMM_FULL_COMMUNICATION == FrSM_Internal.clusterStatus[clusterId].curComMode)) {

        /* Transition T09 (a) */ /* @req FrSm080 */
        /* Set state to Offline */
        (void)FrIf_SetState(clusterId, FRIF_GOTO_OFFLINE);
        /* Halt Communication of all CCs */
        for(controller =0 ; controller < FrSM_ConfigPointer->FrSMClusters[clusterId].FrSMControllerCount; controller++ )
        {
           (void)FrIf_HaltCommunication(FrSM_ConfigPointer->FrSMClusters[clusterId].FrSMControllerRef[controller].FrIfControllerId);
        }

        /* Change Com mode and indicate to Upper ComM layer */
        FrSM_Internal.clusterStatus[clusterId].curComMode = ComM_Mode;
        ComM_BusSM_ModeIndication(FrSM_ConfigPointer->FrSMClusters[clusterId].FrSMComMNetworkHandle, &ComM_Mode);

        /* Cluster state set to Halt Req */
        FrSM_Internal.clusterStatus[clusterId].frSMClusterState = FRSM_HALT_REQ;

        /* Indication to BswM layer */
        if(FRSM_ECU_ACTIVE == FrSM_Internal.clusterStatus[clusterId].frSMClusterPassiveState)
        {
            BswM_FrSM_CurrentState(FrSM_ConfigPointer->FrSMClusters[clusterId].FrSMComMNetworkHandle,FRSM_BSWM_HALT_REQ);
        } else {
            BswM_FrSM_CurrentState(FrSM_ConfigPointer->FrSMClusters[clusterId].FrSMComMNetworkHandle,FRSM_BSWM_HALT_REQ_ECU_PASSIVE);
        }
    } else {
        /* Do nothing */
    }

    return E_OK;
}

/** This service shall put out the current communication mode of a FR network. */
/* @req FrSm024 */
/**
 *
 * @param NetworkHandle
 * @param ComM_ModePtr
 * @return
 */
Std_ReturnType FrSM_GetCurrentComMode( NetworkHandleType NetworkHandle, ComM_ModeType* ComM_ModePtr )
{
    uint8 clusterId;

    /* @req FrSm060 */
    FRSM_DET_REPORTERROR((FRSM_STATUS_INIT == FrSM_Internal.initStatus),FRSM_SERVICE_ID_GET_CURRENT_COM_MODE, FRSM_E_UNINIT, E_NOT_OK);

    /* @req FrSm028 */
    FRSM_DET_REPORTERROR((NULL != ComM_ModePtr),FRSM_SERVICE_ID_GET_CURRENT_COM_MODE,FRSM_E_NULL_PTR,E_NOT_OK);

    /* @req FrSm027 */
    FRSM_DET_REPORTERROR(((NetworkHandle < COMM_CHANNEL_COUNT) && (FrSM_ConfigPointer->FrSMClusterLookups[NetworkHandle] != INVALID_FR_NW_HANDLE)),FRSM_SERVICE_ID_GET_CURRENT_COM_MODE,FRSM_E_INV_HANDLE,E_NOT_OK);

    /* Get Cluster ID mapped to CoM channel */
    clusterId = FrSM_ConfigPointer->FrSMClusterLookups[NetworkHandle];

    /* @req FrSm025 */
    *ComM_ModePtr = FrSM_Internal.clusterStatus[clusterId].curComMode;

    return E_OK;
}

/* @req FrSm172 */
/**
 *
 * @param NetworkHandle
 * @return
 */
Std_ReturnType FrSM_AllSlots( NetworkHandleType NetworkHandle )
{
    Std_ReturnType status;
    uint8 controller;
    Std_ReturnType allSlotStatus[FRSM_CLUSTER_COUNT];
    uint8 clusterId;
    status = E_OK;
    /* @req FrSm169 */
    FRSM_DET_REPORTERROR((FRSM_STATUS_INIT == FrSM_Internal.initStatus),FRSM_SERVICE_ID_ALL_SLOTS, FRSM_E_UNINIT, E_NOT_OK);

    /* @req FrSm168 */
    FRSM_DET_REPORTERROR(((NetworkHandle < COMM_CHANNEL_COUNT) && (FrSM_ConfigPointer->FrSMClusterLookups[NetworkHandle] != INVALID_FR_NW_HANDLE)),FRSM_SERVICE_ID_ALL_SLOTS,FRSM_E_INV_HANDLE,E_NOT_OK);

    /* Get Cluster ID mapped to CoM channel */
    clusterId = FrSM_ConfigPointer->FrSMClusterLookups[NetworkHandle];

    for(controller = 0; controller < FrSM_ConfigPointer->FrSMClusters[clusterId].FrSMControllerCount; controller++) {
        /* @req FrSm171 */
        allSlotStatus[clusterId] = FrIf_AllSlots(FrSM_ConfigPointer->FrSMClusters[clusterId].FrSMControllerRef[controller].FrIfControllerId);

        if(allSlotStatus[clusterId] != E_OK) {
            status = E_NOT_OK;
            break;
        } else {
            /* Do nothing */
        }
    }

    return status;
}

/* @req FrSm174 */
/**
 *
 * @param FrSM_Passive
 * @return
 */
Std_ReturnType FrSM_SetEcuPassive( boolean FrSM_Passive )
{
    uint8 clstrCount;

    /* @req FrSm179 */
    FRSM_DET_REPORTERROR((FRSM_STATUS_INIT == FrSM_Internal.initStatus),FRSM_SERVICE_ID_SET_ECU_PASSIVE, FRSM_E_UNINIT, E_NOT_OK);

    /* @req FrSm177 */
    for ( clstrCount = 0; clstrCount < FrSM_ConfigPointer->FrSMClusterCount; ++clstrCount) {
        if(TRUE == FrSM_Passive) {
            FrSM_Internal.clusterStatus[clstrCount].frSMClusterPassiveState = FRSM_ECU_PASSIVE;
        }else {
            FrSM_Internal.clusterStatus[clstrCount].frSMClusterPassiveState = FRSM_ECU_ACTIVE;
        }

        /* @req FrSm178 */
        if(FRSM_READY != FrSM_Internal.clusterStatus[clstrCount].frSMClusterState) {
            FrSM_Internal_TrcvNormal(clstrCount);
        }
    }
    return E_OK;
}

/* @req FrSm030 */ /*  one state machine impl per cluster */
/**
 *
 * @param clusterId
 */
void FrSM_MainFunction(uint8 clusterId)
{
    /* @req FrSm181 */
    if( FRSM_STATUS_UNINIT == FrSM_Internal.initStatus ) {
#ifdef HOST_TEST
        ReportErrorStatus();
#endif
        /*lint -e{904} Return statement is necessary in case of reporting a DET error */
        return;
    }

    /* function call to know POC status for CC */
    FrSM_Internal_PocStatus(clusterId);

    /* @req FrSm048 */
    switch(FrSM_Internal.clusterStatus[clusterId].frSMClusterState) {
        case FRSM_READY:
            /* function call to know wakeup reason to first CC of cluster */
            FrSM_Internal_WakeReason(clusterId);
            /* function to change state from READY to WAKEUP/STARTUP */
            FrSM_Internal_ReadyToWakeUpStartUp_Trans(clusterId);
            break;

        case FRSM_WAKEUP:
            /* Function call to know Wake Type and to handle to No Com request */
            FrSM_Internal_WakeUp_Pattern(clusterId);
            /* function call to change state from WAKEUP to STARTUP */
            FrSM_Internal_WakeupToStartUp_Trans(clusterId);
            break;

        case FRSM_STARTUP:
            /* Function call for StartUp to other state transition */
            FrSM_Internal_StartUp_Trans(clusterId);
            break;

        case FRSM_HALT_REQ:
            /* Function call to transition from HALT to READY state */
            FrSM_Internal_Halt_State_Actions(clusterId);
            break;

        case FRSM_ONLINE:
        case FRSM_LOW_NUMBER_OF_COLDSTARTERS:
            /* function call to know number of startup frames */
            FrSM_Internal_StartupFrame_No(clusterId);
            FrSM_Internal_Online_Trans(clusterId);
            break;

        case FRSM_ONLINE_PASSIVE:
            FrSM_Internal_Online_Passive_Actions(clusterId);
            break;

        case FRSM_KEYSLOT_ONLY:
            FrSM_Internal_KeySlot_Actions(clusterId);
            break;

        default :
            break;
    }

    /* Function call to Update timer Variables*/
    FrSM_Internal_UpdateTimer(clusterId);
}

#ifdef HOST_TEST
FrSM_InternalType* readinternal_status(void );
FrSM_InternalType* readinternal_status(void)
{
    return &FrSM_Internal;
}
#endif
