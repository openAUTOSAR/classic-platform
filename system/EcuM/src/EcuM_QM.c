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

/** @fileSafetyClassification QM **/ /* This file handles QM partition ECU management */
/* @req SWS_BSW_00005 */
#include "Std_Types.h"
#include "EcuM.h"
#include <Os.h>
#include "EcuM_Generated_Types.h"
#include "EcuM_Internals.h"
#include "EcuM_SP_common.h"
#include "SchM_EcuM.h"
#include "Mcu.h"

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

/* @req SWS_BSW_00006 */
#if defined(USE_NVM)
#define ECUM_START_SEC_VAR_NVM_CLEARED_8
#include "EcuM_BswMemMap.h" /*lint !e9019 MISRA:OTHER:suppressed due to EcuM_MemMap.h include is needed:[MISRA 2012 Rule 20.1, advisory] */
static NvM_RequestResultType readAllResult;
#define ECUM_STOP_SEC_VAR_NVM_CLEARED_8
#include "EcuM_BswMemMap.h" /*lint !e9019 MISRA:OTHER:suppressed due to EcuM_MemMap.h include is needed:[MISRA 2012 Rule 20.1, advisory] */
#endif

#ifndef SYS_CALL_ShutdownOS
#define SYS_CALL_ShutdownOS ShutdownOS
#endif
/* ----------------------------[Extern function]---------------------------*/
#if defined(USE_BSWM)
extern void BswM_EcuM_CurrentState ( EcuM_StateType CurrentState );
#endif

/*
 * get the sync status (written by Asil parition only)
 */
static inline EcuM_SP_SyncStatus get_sync_status_master(void){
    return EcuM_World.syncStatusMaster;
}

/*
 * Tracking the variable for QM to sync with other partition
 */
static inline void set_sync_status_partition_QM(EcuM_SP_SyncStatus syncStatus){
    EcuM_World_ComM_NVM.syncStatusPartition_QM = syncStatus;
}
/*
 * get the QM func status
 */
static inline EcuM_SP_SyncStatus get_sync_status_partition_QM(void){
    return EcuM_World_ComM_NVM.syncStatusPartition_QM;
}
/*
 * Update the QM func status
 */

/* inline function to update QM work done and inform for sync */
static inline EcuM_SP_RetStatus update_function_finish_partition_QM() {
    set_sync_status_partition_QM(get_sync_status_master() | (EcuM_SP_SyncStatus)ECUM_SP_PARTITION_FUN_COMPLETED_QM);
    return ECUM_SP_RELEASE_SYNC;
}
static inline boolean is_currentSyncState(EcuM_SP_SyncStatus syncStatus) {
    return ((get_sync_status_master() & (EcuM_SP_SyncStatus)ECUM_MAINSTATE_MASK) == (syncStatus & (EcuM_SP_SyncStatus)ECUM_MAINSTATE_MASK)) ? TRUE : FALSE;
}
/* if the supplied major status is not finished */
static inline boolean is_exec_required_for_state_partition_QM(EcuM_SP_SyncStatus syncStatus) {
    return ((is_currentSyncState(syncStatus) == TRUE) && !((get_sync_status_partition_QM() & (EcuM_SP_SyncStatus)ECUM_MAINSTATE_MASK) == (syncStatus & (EcuM_SP_SyncStatus)ECUM_MAINSTATE_MASK)
		/** @CODECOV:PARAMETER_VALIDATION_PRIVATE_FUNCTION:QM status can never not be set as complete when above check is also true*/
		__CODE_COVERAGE_IGNORE__
		&& (get_sync_status_partition_QM() & (EcuM_SP_SyncStatus)ECUM_SP_PARTITION_FUN_COMPLETED_QM) == ECUM_SP_PARTITION_FUN_COMPLETED_QM));
}
#if defined(USE_BSWM)
static inline boolean is_state_changed_partition_QM(EcuM_StateType current_state) {
    return (EcuM_World_ComM_NVM.current_state_partition_QM != current_state);
}
static inline void update_state_changed_partition_QM(EcuM_StateType current_state) {
    EcuM_World_ComM_NVM.current_state_partition_QM = current_state;
    set_sync_status_partition_QM(ECUM_SP_SYNC_NOT_REQUIRED);
}
#endif

/*
 * startup II for partition QM (in state ECUM_STATE_STARTUP_TWO)
 */
static inline EcuM_SP_RetStatus in_state_startup_two_partition_QM(){

    EcuM_SP_RetStatus rv = ECUM_SP_OK;

    if (is_exec_required_for_state_partition_QM(ECUM_SP_SYNC_STARTUP_TWO_START) == TRUE){
        EcuM_StartupTwo_Partition_QM();
        rv = update_function_finish_partition_QM();
    }
    else if (is_exec_required_for_state_partition_QM(ECUM_SP_SYNC_INIT_RUN_START) == TRUE) {
        EcuM_OnEnterRun_Partition_QM(); /** @req SWS_EcuM_00308 */
        /* @req SWS_EcuMf_00019 */
        /* @req SWS_EcuMf_00008 */
        SetComMCommunicationAllowed(ALL, TRUE);

        rv = update_function_finish_partition_QM();
    }
    else {
        // in synchronization with other partition
    }
return rv;
}

/*
 * RUN II for partition QM (in state ECUM_STATE_APP_RUN)
 */
static inline EcuM_SP_RetStatus in_state_appRun_partition_QM(){

    EcuM_SP_RetStatus rv = ECUM_SP_OK;

    if (is_exec_required_for_state_partition_QM(ECUM_SP_SYNC_EXIT_RUN_START) == TRUE){

        EcuM_OnExitRun_Partition_QM(); /** @req SWS_EcuM_00865 */
        /* @req SWS_EcuMf_00008 */
        /* @req SWS_EcuMf_00020 */
        SetComMCommunicationAllowed(ALL_WO_LIN, FALSE);

        rv = update_function_finish_partition_QM();

    }
    else
    {
        // in synchronization with other partition
    }
    return rv;
}

/*
 * RUN III states (in state ECUM_STATE_APP_POST_RUN) for partition QM
 */
static inline EcuM_SP_RetStatus in_state_appPostRun_partition_QM(){

    EcuM_SP_RetStatus rv = ECUM_SP_OK;

    if (is_exec_required_for_state_partition_QM(ECUM_SP_SYNC_INIT_RUN_START) == TRUE){
        /* We have run requests, return to RUN II */
        /* @req SWS_EcuM_00866 */
        EcuM_OnEnterRun_Partition_QM(); /** @req SWS_EcuM_00308 */
        /* @req SWS_EcuMf_00019 */
        /* @req SWS_EcuMf_00008 */
        SetComMCommunicationAllowed(ALL, TRUE);

        rv = update_function_finish_partition_QM();

    } else if (is_exec_required_for_state_partition_QM(ECUM_SP_SYNC_POST_RUN_START) == TRUE) {
        EcuM_OnExitPostRun_Partition_QM(); /** @req SWS_EcuM_00761 */

        rv = update_function_finish_partition_QM();

    } else {
        /* NOTE: We have postrun requests */
    }
    return rv;
}

/*
 * PREP SHUTDOWN state (in state ECUM_STATE_PREP_SHUTDOWN) for QM
 */
static inline EcuM_SP_RetStatus in_state_prepShutdown_Partition_QM(void){
    EcuM_SP_RetStatus rv = ECUM_SP_OK;

    if (is_exec_required_for_state_partition_QM(ECUM_SP_SYNC_PREP_SHUTDOWN_START) == TRUE){
        EcuM_OnPrepShutdown_Partition_QM();

#if defined(USE_DEM)
        // DEM shutdown
        Dem_Shutdown();
#endif
        rv = update_function_finish_partition_QM();
    }
    else{
        // in synchronization with other partition
    }
    return rv;
}

static inline void go_off_one_module_deinit_QM(void){
#if defined(USE_COMM)
    ComM_DeInit();
#endif

#if defined(USE_BSWM)
    /* @req SWS_EcuMf_00017 */
    BswM_Deinit();
#endif

#if defined(USE_NVM)

    // Start NvM_WriteAll and timeout timer
    NvM_WriteAll();

    EcuM_World_go_off_one_state_timeout = EcuM_World.config->EcuMNvramWriteAllTimeout / ECUM_MAIN_FUNCTION_PERIOD;
#endif
}

/*
 * GO OFF ONE state (in state ECUM_STATE_GO_OFF_ONE) for QM
 */
static inline EcuM_SP_RetStatus in_go_off_one_mode_partition_QM(void){
    EcuM_SP_RetStatus rv = ECUM_SP_OK;

    if (is_exec_required_for_state_partition_QM(ECUM_SP_SYNC_GO_OFF_ONE_START) == TRUE){
        EcuM_OnGoOffOne_Partition_QM();
        /* @req SWS_EcuMf_00008 Only set LIN to false when not going to sleep.
        *                Kind a strange that COMM is deini on the next line.
        */
        /* @req SWS_EcuMf_00019 */
        SetComMCommunicationAllowed(ONLY_LIN, FALSE);
        go_off_one_module_deinit_QM();  /*lint !e522 MISRA:CONFIGURATION: This function may not have any functionality depending on configuration:[MISRA 2012 Rule 2.2, required] */
        rv = update_function_finish_partition_QM();
    }
    else if (is_exec_required_for_state_partition_QM(ECUM_SP_SYNC_INIT_GO_OFF_TWO_START) == TRUE) {
        /* @req SWS_EcuM_00328 */
#if defined(USE_NVM)
        if (0u != EcuM_World_go_off_one_state_timeout){
            EcuM_World_go_off_one_state_timeout--;
        }
        // Wait for the NVM job (NvmWriteAll) to terminate
        Std_ReturnType ret = NvM_GetErrorStatus(0, &writeAllResult);
        if (((ret == E_OK) && (writeAllResult != NVM_REQ_PENDING)) ||
                            (EcuM_World_go_off_one_state_timeout == 0)) {
            rv = update_function_finish_partition_QM();
        }
#else
        rv = update_function_finish_partition_QM();
#endif
    }
    else
    {
        // in synchronization with other partition
    }
    return rv;
}

/*
 * Mainfunction for QM
 */
EcuM_SP_RetStatus EcuM_MainFunction_Partition_QM(EcuM_StateType current_state) {
    EcuM_SP_RetStatus rv = ECUM_SP_OK;
    // Check for EcuM state change, in that case QM needs to update BswM
#if defined(USE_BSWM)
    if (is_state_changed_partition_QM(current_state) == TRUE)
    {
        update_state_changed_partition_QM(current_state);
        /* The BswM is not initialized in the ECUM_STATE_STARTUP_ONE state */
        /* @req SWS_EcuMf_00014 */
        if (ECUM_STATE_STARTUP_ONE != current_state) {
            BswM_EcuM_CurrentState(current_state);
        }
    }
#endif
    switch (current_state) {
        case ECUM_STATE_APP_RUN:
            /* RUN II state */
            rv = in_state_appRun_partition_QM();
            break;

        case ECUM_STATE_APP_POST_RUN:
            /* RUN III state */
            rv = in_state_appPostRun_partition_QM();
            break;
        case ECUM_STATE_PREP_SHUTDOWN:
            rv = in_state_prepShutdown_Partition_QM();
            break;
        case ECUM_STATE_GO_OFF_ONE:
            rv = in_go_off_one_mode_partition_QM();
            break;
        case ECUM_STATE_STARTUP_ONE:
            // Master partition will change the state to ECUM_STARTUP_TWO
            break;
        case ECUM_STATE_STARTUP_TWO:
            rv = in_state_startup_two_partition_QM();
            break;
        default:
            ECUM_DET_REPORT_ERROR(ECUM_ARC_MAINFUNCTION_QM_ID, ECUM_E_ARC_FAULTY_CURRENT_STATE);
            break;
    }
    return rv;
}

/*
 * The order defined here is found in EcuM2411
 */
void EcuM_StartupTwo_Partition_QM(void)
{
    if (EcuM_World.current_state == ECUM_STATE_STARTUP_TWO) {

        // Initialize drivers that don't need NVRAM data for QM paritition
        EcuM_AL_DriverInitTwo_Partition_QM(EcuM_World.config);

#if defined(USE_NVM)
        // Start timer to wait for NVM job to complete
        TickType tickTimerStart = GetOsTick();
#endif

    /* Prepare the system to startup RTE */
#if defined(USE_RTE)
        (void)Rte_Start_partitionQM();
#endif /* USE_RTE */

#if defined(USE_NVM)
        /* Wait for the NVM job (NvM_ReadAll) to terminate. This assumes that:
         * - A task runs the memory MainFunctions, e.g. Ea_MainFunction(), Eep_MainFunction()
         *   are run in a higher priority task that the task that executes this code.
         */
#if defined(CFG_ARC_ECUM_NVM_READ_INIT) && defined(USE_FEE)
        EcuM_Arc_InitFeeReadAdminBlock(tickTimerStart, EcuM_World.config->EcuMNvramReadAllTimeout);
#endif

        TickType tickTimerElapsed;
        Std_ReturnType status;

        do {
            /* Read the multiblock status */
            status = NvM_GetErrorStatus(0, &readAllResult);

            tickTimerElapsed = OS_TICKS2MS_OS_TICK(GetOsTick() - tickTimerStart);

#if defined(CFG_ARC_ECUM_NVM_READ_INIT)
            EcuM_Arc_InitMemReadAllMains();
#endif

            /* The timeout EcuMNvramReadAllTimeout is in ms */
        } while( (readAllResult == NVM_REQ_PENDING) && (tickTimerElapsed < EcuM_World.config->EcuMNvramReadAllTimeout) );
        
        if(status!=E_OK) {
            ECUM_DET_REPORT_ERROR(ECUM_STARTUPTWO_ID, ECUM_E_ARC_TIMERERROR);
        }

#endif /* USE_NVM */
        /* Initialize drivers that need NVRAM data for QM paritition */
        EcuM_AL_DriverInitThree_Partition_QM(EcuM_World.config);

    }

}
