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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.3.0 */

/** @fileSafetyClassification ASIL **/ /* This file handles ASIL partition ECU management */
/* @req SWS_BSW_00005 */
#include "Std_Types.h"
#include "EcuM.h"
#include <Os.h>
#include "EcuM_Generated_Types.h"
#include "EcuM_Internals.h"
#include "EcuM_SP_common.h"
#include "SchM_EcuM.h"
#include "Mcu.h"
#if defined(USE_RTM)
#include "Rtm.h"
#endif
#if defined(USE_HTMSS)
#include "Htmss.h"
#endif
#if defined(USE_DET)
#include "Det.h"
#endif
#if defined(USE_DEM)
#include "Dem.h"                /* @req SWS_EcuM_02875 */
#endif
#if defined(USE_NVM)
#include "NvM.h"
#endif
#if defined(USE_RTE)
#include "Rte_Main.h"
#endif
#if defined(USE_SCHM)
#include "SchM_SP.h"
#endif

#ifndef SYS_CALL_ShutdownOS
#define SYS_CALL_ShutdownOS ShutdownOS
#endif

/* @req SWS_BSW_00006 */
#define ECUM_START_SEC_VAR_CLEARED_16
#include "EcuM_BswMemMap.h" /*lint !e9019 suppressed due to EcuM_MemMap.h include is required */
/*lint -esym(9003,writeAllResult ) MISRA Exception: Readability (MISRA 2012 Rule 8.9, advisory)*/
uint16 EcuM_World_SP_sync_timeout = 0;
#define ECUM_STOP_SEC_VAR_CLEARED_16
#include "EcuM_BswMemMap.h" /*lint !e9019 suppressed due to EcuM_MemMap.h include is required */

/*
 * Tracking the variable for QM to sync with other partition
 */
static inline void set_sync_status_master(EcuM_SP_SyncStatus syncStatus){
    EcuM_World.syncStatusMaster= syncStatus;
}
static inline void clear_sync_status_master(){
    set_sync_status_master(ECUM_SP_SYNC_NOT_REQUIRED);
}

/*
 * get the sync status (written by Asil parition only)
 */
static inline EcuM_SP_SyncStatus get_sync_status_master(void){
    return EcuM_World.syncStatusMaster;
}

/*
 * Start Sync status (written by Asil partition only)
 */
static inline EcuM_SP_RetStatus start_sync_status_master(EcuM_SP_SyncStatus syncStatus){
    EcuM_World.syncStatusMaster = syncStatus;
    return ECUM_SP_REQ_SYNC;
}

/*
 * Start Sync status (written by Asil partition only)
 */
static inline EcuM_SP_SyncStatus update_sync_sub_status_master(EcuM_SP_PartitionFunCompletion syncStatus){
    EcuM_World.syncStatusMaster|= (syncStatus & (EcuM_SP_SyncStatus)ECUM_SUBSTATE_MASK);
    return EcuM_World.syncStatusMaster;
}

static inline boolean is_exec_required_for_state_master() {
    return (((get_sync_status_master() & (EcuM_SP_SyncStatus)ECUM_SP_PARTITION_FUN_COMPLETED_A0) == ECUM_SP_PARTITION_FUN_COMPLETED_A0) ? FALSE : TRUE);
}


static inline boolean is_currentSyncState(EcuM_SP_SyncStatus syncStatus) {
    return (((get_sync_status_master() & syncStatus) == syncStatus) ? TRUE : FALSE);
}

/*
 * startup II for partition A0 (in state ECUM_STATE_STARTUP_TWO)
 */
static inline EcuM_SP_RetStatus in_state_startup_two_partition_A0(void){
    EcuM_SP_RetStatus rv = ECUM_SP_OK;

    if (is_exec_required_for_state_master() == TRUE) {
        EcuM_StartupTwo_Partition_A0();
        rv = start_sync_status_master (ECUM_SP_SYNC_STARTUP_TWO_START);
        EcuM_SP_Sync_UpdateStatus(ECUM_SP_PARTITION_FUN_COMPLETED_A0);
    }
    else if (is_currentSyncState(ECUM_SP_SYNC_STARTUP_TWO_PARTITION_ALL_DONE) == TRUE) {
        EcuM_AL_DriverInitThree_Partition_A0(EcuM_World.config);
        EcuM_OnEnterRun_Partition_A0(); /** @req SWS_EcuM_00308 */
        rv = start_sync_status_master (ECUM_SP_SYNC_INIT_RUN_START);
        EcuM_SP_Sync_UpdateStatus(ECUM_SP_PARTITION_FUN_COMPLETED_A0);
    }
    else {
        // in synchronization
    }
    return rv;
}

/*
 * RUN II for partition A0 (in state ECUM_STATE_APP_RUN)
 */
static inline EcuM_SP_RetStatus in_state_appRun_partition_A0(void){
     EcuM_SP_RetStatus rv = ECUM_SP_OK;

     if (EcuM_World_run_state_timeout != 0) {
          EcuM_World_run_state_timeout--;
     }

     if (is_exec_required_for_state_master() == TRUE) {
          if ((FALSE == hasRunRequests()) && (EcuM_World_run_state_timeout == 0)) {
               EcuM_OnExitRun_Partition_A0(); /** @req SWS_EcuM_00865 */
               rv = start_sync_status_master (ECUM_SP_SYNC_EXIT_RUN_START);
               EcuM_SP_Sync_UpdateStatus(ECUM_SP_PARTITION_FUN_COMPLETED_A0);
          }
          else {
               // in run state
          }
     }
     return rv;
}

/*
 * RUN III states (in state ECUM_STATE_APP_POST_RUN) for partition A0
 */
static inline EcuM_SP_RetStatus in_state_appPostRun_partition_A0(void){
    EcuM_SP_RetStatus rv = ECUM_SP_OK;
    if (is_exec_required_for_state_master() == TRUE) {
        if (TRUE == hasRunRequests()){
            /* We have run requests, return to RUN II */
            /* @req SWS_EcuM_00866 */
            EcuM_OnEnterRun_Partition_A0(); /** @req SWS_EcuM_00308 */
            rv = start_sync_status_master (ECUM_SP_SYNC_INIT_RUN_START);
            EcuM_SP_Sync_UpdateStatus(ECUM_SP_PARTITION_FUN_COMPLETED_A0);

        } else if (FALSE == hasPostRunRequests()){
            EcuM_OnExitPostRun_Partition_A0(); /** @req SWS_EcuM_00761 */
            rv = start_sync_status_master (ECUM_SP_SYNC_POST_RUN_START);
            EcuM_SP_Sync_UpdateStatus(ECUM_SP_PARTITION_FUN_COMPLETED_A0);

        } else {
            /* NOTE: We have postrun requests */
        }
    }
    return rv;
}

/*
 * PREP SHUTDOWN state (in state ECUM_STATE_PREP_SHUTDOWN) for A0
 */
static inline EcuM_SP_RetStatus in_state_prepShutdown_Partition_A0(void){

    EcuM_SP_RetStatus rv = ECUM_SP_OK;
    if (is_exec_required_for_state_master() == TRUE) {
        EcuM_OnPrepShutdown_Partition_A0();

        // Switch shutdown mode
        if ((EcuM_World.shutdown_target == ECUM_STATE_OFF) || (EcuM_World.shutdown_target == ECUM_STATE_RESET)) {
            //If in state Off or Reset go into Go_Off_One:
            rv = start_sync_status_master (ECUM_SP_SYNC_PREP_SHUTDOWN_START);
            EcuM_SP_Sync_UpdateStatus(ECUM_SP_PARTITION_FUN_COMPLETED_A0);
        }
        else{
            rv = ECUM_SP_PARTITION_ERR;
        }
    }
    return rv;
}

/*
 * GO OFF ONE state (in state ECUM_STATE_GO_OFF_ONE) for A0
 */
static inline EcuM_SP_RetStatus in_go_off_one_mode_partition_A0(void){
    EcuM_SP_RetStatus rv = ECUM_SP_OK;

    if (is_exec_required_for_state_master() == TRUE) {
        EcuM_OnGoOffOne_Partition_A0();

        rv = start_sync_status_master (ECUM_SP_SYNC_GO_OFF_ONE_START);
        EcuM_SP_Sync_UpdateStatus(ECUM_SP_PARTITION_FUN_COMPLETED_A0);
    }
    else if (is_currentSyncState(ECUM_SP_SYNC_GO_OFF_ONE_PARTITION_ALL_DONE) == TRUE) {
        rv = start_sync_status_master (ECUM_SP_SYNC_INIT_GO_OFF_TWO_START);
        EcuM_SP_Sync_UpdateStatus(ECUM_SP_PARTITION_FUN_COMPLETED_A0);
    }

    else {
        // in synchronization with other partition
    }
    return rv;
}

#if defined(USE_HTMSS)
/*
 * Get test results from Htmss
 */
Std_ReturnType EcuM_StartHWTests( HTMSS_TestGroupType group ) {

    Std_ReturnType rv = E_OK;

    // As long as return from HTMSS is ok, keep calling
    while(rv == E_OK) {
        rv = HTMSS_StartTest(group);
    }

    // Check if return is because of list end or error
    if (rv == E_OK_ENDOFLIST) {
        rv = E_OK;
    } else {
        rv = E_NOT_OK;
    }

    return rv;
}

/*
 * Get test results from Htmss
 */
void EcuM_GetHWTestResults( HTMSS_TestGroupType group ) {

    HTMSS_TestStatusType rv;

    while(Htmss_getState() != HTMSS_IDLE) {

        rv = HTMSS_GetTestStatus(group, NULL_PTR);

        if(rv != HTMSS_STATUS_OK) {
            HTMSS_StartupTestErrorHook();
        }
    }
}
#endif

/*
 * Main function A0
 */
EcuM_SP_RetStatus EcuM_MainFunction_Partition_A0(EcuM_StateType current_state) {
    EcuM_SP_RetStatus rv = ECUM_SP_OK;
    switch (current_state) {
        case ECUM_STATE_APP_RUN:
            /* RUN II state */
            rv = in_state_appRun_partition_A0();
            break;
        case ECUM_STATE_APP_POST_RUN:
            /* RUN III state */
            rv = in_state_appPostRun_partition_A0();
            break;
        case ECUM_STATE_PREP_SHUTDOWN:
            rv = in_state_prepShutdown_Partition_A0();
            break;
        case ECUM_STATE_GO_OFF_ONE:
            rv = in_go_off_one_mode_partition_A0();
            break;
        case ECUM_STATE_STARTUP_ONE:
        case ECUM_STATE_STARTUP_TWO:
            rv = in_state_startup_two_partition_A0();
            break;
        default:
            ECUM_DET_REPORT_ERROR(ECUM_ARC_MAINFUNCTION_A0_ID, ECUM_E_ARC_FAULTY_CURRENT_STATE);
            break;
    }

    EcuM_SP_SyncStatus currentSyncStatus = get_sync_status_master();
    if (currentSyncStatus != ECUM_SP_SYNC_NOT_REQUIRED) {
		//If we reached ALL_DONE, code above shall go to SYNC_NOT_REQUIRED or next main status START. Otherwise there is some internal error (we will be stuck in ALL_DONE).
		ASSERT((currentSyncStatus & (EcuM_SP_SyncStatus)ECUM_SUBSTATE_MASK) != ECUM_PARTITIONS_IN_SYNC);

		EcuM_World_SP_sync_timeout++;
	} else {
		EcuM_World_SP_sync_timeout = 0;
	}

	/* @req ARC_SWS_ECUM_00010 If synchronisation takes more than the ECUM_SYNC_TIMEOUT_LIMIT then EcuM shall log RTM fault. */
    if (EcuM_World_SP_sync_timeout > ECUM_SYNC_TIMEOUT_LIMIT) {
#if defined(USE_RTM)

        Rtm_EntryType EntryType;
        
        EntryType.moduleId = ECUM_MODULE_ID;
        EntryType.instanceId = 0u;
        EntryType.apiId = ECUM_MAINFUNCTION_ID;
        EntryType.errorId = ECUM_E_ARC_PARTITION_SYNC_TIMEOUT;
        EntryType.errorType = 0u;

        Rtm_ReportFailure(EntryType);
#endif
    }
    return rv;
}

/*
 * Update the EcuM status based on the sync status, this is only called from master.
 */
void EcuM_SP_Sync_UpdateStatus(EcuM_SP_PartitionFunCompletion syncSubState)
{
    EcuM_SP_SyncStatus syncState = update_sync_sub_status_master(syncSubState);

    switch (syncState) {
        case ECUM_SP_SYNC_EXIT_RUN_PARTITION_ALL_DONE:
            clear_sync_status_master();
            /* @req SWS_EcuM_00311 */
            SetCurrentState(ECUM_STATE_APP_POST_RUN);   /** @req SWS_EcuM_00865 */
            break;
        case ECUM_SP_SYNC_POST_RUN_PARTITION_ALL_DONE:
            clear_sync_status_master();
            SetCurrentState(ECUM_STATE_PREP_SHUTDOWN);
            break;
        case ECUM_SP_SYNC_PREP_SHUTDOWN_PARTITION_ALL_DONE:
            clear_sync_status_master();
            SetCurrentState(ECUM_STATE_GO_OFF_ONE);
            break;
        case ECUM_SP_SYNC_GO_OFF_ONE_PARTITION_ALL_DONE:
            // change state on ECUM_SP_SYNC_INIT_GO_OFF_TWO_PARTITION_ALL_DONE
            break;
        case ECUM_SP_SYNC_INIT_GO_OFF_TWO_PARTITION_ALL_DONE:
            clear_sync_status_master();
            SYS_CALL_ShutdownOS(E_OK);
            break;
        case ECUM_SP_SYNC_STARTUP_TWO_PARTITION_ALL_DONE:
            // change state on ECUM_SP_SYNC_INIT_RUN_PARTITION_ALL_DONE
            break;
        case ECUM_SP_SYNC_INIT_RUN_PARTITION_ALL_DONE:
            clear_sync_status_master();
            SetCurrentState(ECUM_STATE_APP_RUN);
            break;
        default:
            /*Partitions not synchronized yet, do nothing until they are*/
            break;
    }
}

/*
 * The order defined here is found in SWS_EcuM_02411
 */
void EcuM_StartupTwo_Partition_A0(void)
{
  ASSERT(EcuM_World.current_state == ECUM_STATE_STARTUP_ONE);
        
  SetCurrentState(ECUM_STATE_STARTUP_TWO);

  /* Initialize drivers that don't need NVRAM data */
  EcuM_AL_DriverInitTwo_Partition_A0(EcuM_World.config);
  /* Prepare the system to startup RTE */
#if defined(USE_RTE)
  (void)Rte_Start_partitionA0();
#endif
}
