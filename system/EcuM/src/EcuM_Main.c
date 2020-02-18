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
/** @fileSafetyClassification ASIL **/ /* Some functions in this file is used by ASIL and QM Ecu managment */

/* @req SWS_EcuM_02989 File structure */ /* @req SWS_EcuM_02989 */

#include "Std_Types.h"
#include "EcuM.h"
#include "EcuM_Generated_Types.h"
#include "EcuM_Internals.h"
#include "EcuM_SP_common.h"
#ifndef ECUM_NOT_SERVICE_COMPONENT
#include "Rte_EcuM.h"
#endif
#include "SchM_EcuM.h"
#if defined(USE_BSWM)
#include "BswM_EcuM.h"
#endif

/* @req SWS_EcuM_02875 */
#if defined(USE_DEM)
#include "Dem.h"
#endif
#if defined(USE_NVM)
#include "NvM.h"
#endif
#if defined(USE_COMM)
#include "ComM.h"
#include "ComM_EcuM.h"
#endif

//#define USE_LDEBUG_PRINTF
#include "debug.h"

#define ECUM_STR   "ECUM:"

/* ----------------------------[Memory Partition]---------------------------*/
/*lint -save -e9019 MISRA:OTHER:suppressed due to EcuM_MemMap.h include is needed:[MISRA 2012 Rule 20.1, advisory] */
#if defined(USE_NVM)
#define ECUM_START_SEC_VAR_NVM_CLEARED_32
#include "EcuM_BswMemMap.h"
uint32 EcuM_World_go_off_one_state_timeout = 0;
#define ECUM_STOP_SEC_VAR_NVM_CLEARED_32
#include "EcuM_BswMemMap.h"

#define ECUM_START_SEC_VAR_NVM_CLEARED_UNSPECIFIED
#include "EcuM_BswMemMap.h"
/*lint -esym(9003,writeAllResult) MISRA:OTHER:Readability:[MISRA 2012 Rule 8.9, advisory] */
NvM_RequestResultType writeAllResult;
#define ECUM_STOP_SEC_VAR_NVM_CLEARED_UNSPECIFIED
#include "EcuM_BswMemMap.h"
#endif

/* MISRA 2012 8.9 (Adv) : An object should be defined at block scope
 * if its identifier only appears in a single function.
 *
 * Depending on configuration it might NOT be possible.
 */
/*lint -esym(9003,EcuM_World_go_sleep_state_timeout) MISRA:OTHER:Readability:[MISRA 2012 Rule 8.9, advisory] */
#define ECUM_START_SEC_VAR_CLEARED_GLOBALMASTER_UNSPECIFIED
#include "EcuM_BswMemMap.h"
uint32 EcuM_World_go_sleep_state_timeout = 0;
#define ECUM_STOP_SEC_VAR_CLEARED_GLOBALMASTER_UNSPECIFIED
#include "EcuM_BswMemMap.h"

#ifndef ECUM_NOT_SERVICE_COMPONENT
#define ECUM_START_SEC_VAR_INIT_UNSPECIFIED
#include "EcuM_BswMemMap.h"
/** @req SWS_EcuM_00749 */
/** @req SWS_EcuMf_00104 */
static Rte_ModeType_EcuM_Mode currentMode = RTE_MODE_EcuM_Mode_STARTUP;
#define ECUM_STOP_SEC_VAR_INIT_UNSPECIFIED
#include "EcuM_BswMemMap.h"
#endif
/*lint -restore */
/* ----------------------------[End Memory Partition]---------------------------*/

#if defined(USE_LDEBUG_PRINTF)
/* Note: Not safety relevant. LDEBUG not used in safety context */
char *GetMainStateAsString( EcuM_StateType state ) {
    char *str = NULL;

    switch(state) {
    case ECUM_STATE_APP_RUN:
        str = "ECUM_STATE_APP_RUN";
        break;
    case ECUM_STATE_SHUTDOWN:
        str = "ECUM_STATE_SHUTDOWN";
        break;
    case ECUM_STATE_WAKEUP:
        str = "ECUM_STATE_WAKEUP";
        break;
//	case ECUM_SUBSTATE_MASK:
//		str = "ECUM_SUBSTATE_MASK";
//		break;
    case ECUM_STATE_WAKEUP_WAKESLEEP:
        str = "ECUM_STATE_WAKEUP_WAKESLEEP";
        break;
    case ECUM_STATE_WAKEUP_ONE:
        str = "ECUM_STATE_WAKEUP_ONE";
        break;
    case ECUM_STATE_OFF:
        str = "ECUM_STATE_OFF";
        break;
    case ECUM_STATE_STARTUP:
        str = "ECUM_STATE_STARTUP";
        break;
    case ECUM_STATE_PREP_SHUTDOWN:
        str = "ECUM_STATE_PREP_SHUTDOWN";
        break;
    case ECUM_STATE_RUN:
        str = "ECUM_STATE_RUN";
        break;
    case ECUM_STATE_STARTUP_TWO:
        str = "ECUM_STATE_STARTUP_TWO";
        break;
    case ECUM_STATE_WAKEUP_TTII:
        str = "ECUM_STATE_WAKEUP_TTII";
        break;
    case ECUM_STATE_WAKEUP_VALIDATION:
        str = "ECUM_STATE_WAKEUP_VALIDATION";
        break;
    case ECUM_STATE_GO_SLEEP:
        str = "ECUM_STATE_GO_SLEEP";
        break;
    case ECUM_STATE_STARTUP_ONE:
        str = "ECUM_STATE_STARTUP_ONE";
        break;
    case ECUM_STATE_WAKEUP_TWO:
        str = "ECUM_STATE_WAKEUP_TWO";
        break;
    case ECUM_STATE_SLEEP:
        str = "ECUM_STATE_SLEEP";
        break;
    case ECUM_STATE_WAKEUP_REACTION:
        str = "ECUM_STATE_WAKEUP_REACTION";
        break;
    case ECUM_STATE_APP_POST_RUN:
        str = "ECUM_STATE_APP_POST_RUN";
        break;
    case ECUM_STATE_GO_OFF_TWO:
        str = "ECUM_STATE_GO_OFF_TWO";
        break;
    case ECUM_STATE_RESET:
        str = "ECUM_STATE_RESET";
        break;
    case ECUM_STATE_GO_OFF_ONE:
        str = "ECUM_STATE_GO_OFF_ONE";
        break;
    default:
        ASSERT(0);
    }
    return str;
}

/* Note: Not safety relevant. LDEBUG not used in safety context */
const char *GetWakeupReactionAsString( EcuM_WakeupReactionType reaction ) {
    const char *WakeupReactionAsString[] = {
            "ECUM_WKACT_RUN",
            "UNKNONW?",
            "ECUM_WKACT_TTII",
            "ECUM_WKACT_SHUTDOWN",
    };
    return  WakeupReactionAsString[reaction];
}

#endif /*USE_LDEBUG_PRINTF*/

/* Note: This is a generic function to set the EcuM state used by both SP partition and for non-safety platform EcuM */
void SetCurrentState(EcuM_StateType state) {

    /* Update the state */
    EcuM_World.current_state = state;

#ifndef ECUM_NOT_SERVICE_COMPONENT

    Rte_ModeType_EcuM_Mode newMode = currentMode;
    switch( state ) {
    case ECUM_STATE_WAKEUP:
    case ECUM_STATE_WAKEUP_ONE:
    case ECUM_STATE_WAKEUP_VALIDATION:
    case ECUM_STATE_WAKEUP_REACTION:
    case ECUM_STATE_WAKEUP_TWO:
    case ECUM_STATE_SLEEP:
    case ECUM_STATE_SHUTDOWN:
        newMode = RTE_MODE_EcuM_Mode_SLEEP;
        break;
    case ECUM_STATE_GO_SLEEP:
        if( EcuM_World.shutdown_target == ECUM_STATE_SLEEP ) {
            newMode = RTE_MODE_EcuM_Mode_SLEEP; /** @req SWS_EcuM_00752 */
        }
        break;
    case ECUM_STATE_GO_OFF_ONE:
    case ECUM_STATE_GO_OFF_TWO:
        newMode = RTE_MODE_EcuM_Mode_SHUTDOWN;
        break;
    case ECUM_STATE_WAKEUP_TTII:
        if( EcuM_World.shutdown_target == ECUM_STATE_SLEEP ) {
            newMode = RTE_MODE_EcuM_Mode_WAKE_SLEEP; /** @req SWS_EcuM_00752 */
        }
        break;
    case ECUM_STATE_PREP_SHUTDOWN:
    case ECUM_STATE_APP_POST_RUN:
        newMode = RTE_MODE_EcuM_Mode_POST_RUN;
        break;
    case ECUM_STATE_APP_RUN:
        SchM_Enter_EcuM_EA_0();

        newMode = RTE_MODE_EcuM_Mode_RUN;

        /* We have a configurable minimum time (EcuMRunMinimumDuration) we have to stay in RUN state  */
        /* This should not be done in EcuM_enter_run_mode() because RTE_MODE_EcuM_Mode_RUN & EcuM_World_run_state_timeout  are
         * not set at the same time and this might lead to immediate timeout
         */
        EcuM_World_run_state_timeout = EcuM_World.config->EcuMRunMinimumDuration / ECUM_MAIN_FUNCTION_PERIOD; /** @req SWS_EcuM_00310 */
        SchM_Exit_EcuM_EA_0();

        break;
    case ECUM_STATE_STARTUP_TWO:
        newMode = RTE_MODE_EcuM_Mode_STARTUP;
        break;
    default:
        /* Do nothing */
        break;
    }

	if( newMode != currentMode ) {
		currentMode = newMode;
		(void)Rte_Switch_currentMode_currentMode(currentMode); /** @req SWS_EcuM_00750 */
	}

#else
    if (ECUM_STATE_APP_RUN == state) {
        /*lint -e534 MISRA:OTHER:ignoring return value:[MISRA 2004 Info,advisory] */
        SchM_Enter_EcuM_EA_0();
        /* We have a configurable minimum time (EcuMRunMinimumDuration) we have to stay in RUN state  */
        EcuM_World_run_state_timeout = EcuM_World.config->EcuMRunMinimumDuration / ECUM_MAIN_FUNCTION_PERIOD;  /** @req SWS_EcuM_00310 */
        SchM_Exit_EcuM_EA_0();
    }
#endif

    /* @req SWS_EcuMf_00014 */
#if (defined(USE_BSWM) && (ECUM_ARC_SAFETY_PLATFORM == STD_OFF))
    /* The BswM is not initialized in the ECUM_STATE_STARTUP_ONE state */
    if (ECUM_STATE_STARTUP_ONE != state) {
        BswM_EcuM_CurrentState(state);
    }
#endif

}

/* Note: QM code - This function is used both by EcuM_Main (non-SP) and EcuM_QM. */
/*lint -esym(522, SetComMCommunicationAllowed) MISRA:STANDARDIZED_INTERFACE:Ok when not using ComM:[MISRA 2012 Rule 2.2, required] */
void SetComMCommunicationAllowed(EcuM_ComMCommunicationGroupsType group, boolean Allowed) {

#if defined(USE_COMM)
        {
            /* @req SWS_EcuMf_00008 */
            uint8 i;
            for (i=0;i<ECUM_COMM_NETWORKS_CNT;i++) {
                const EcuM_ComMConfigType* comMConfigPtr = &EcuM_World.config->EcuMComMConfig[i];
                if ((EcuM_World.config->EcuMComMConfig != 0) &&
                    ((ALL == group) ||
                     ((ALL_WO_LIN == group) && (comMConfigPtr->EcuMComBusType != COMM_BUS_TYPE_LIN)) ||
                     ((ONLY_LIN == group) && (comMConfigPtr->EcuMComBusType == COMM_BUS_TYPE_LIN)))) {
                    DEBUG_ECUM_CALLOUT_W_ARG("ComM_CommunicationAllowed","%ld",(uint32)comMConfigPtr->EcuMComMNetworkHandle);
                    ComM_CommunicationAllowed(comMConfigPtr->EcuMComMNetworkHandle, Allowed);
                }
            }
        }
#else
        (void)group;  /*lint !e920 MISRA:STANDARDIZED_INTERFACE:Casting from a type to void:[MISRA 2012 Rule 1.3, required] */
        (void)Allowed; /*lint !e920 MISRA:STANDARDIZED_INTERFACE:Casting from a type to void:[MISRA 2012 Rule 1.3, required] */
#endif
}

#if (ECUM_ARC_SAFETY_PLATFORM == STD_OFF)
/** @CODECOV:NOT_SUPPORTED:SafetyPlatform has its own partitioned ECUM main functions **/
__CODE_COVERAGE_OFF__
/**
 * RUN II entry
 * - Called from EcuM_StartupTwo()
 * - Called from
 *
 * Note: Not used for safety platform
 */
void EcuM_enter_run_mode(void){
    EcuM_OnEnterRun(); /** @req SWS_EcuM_00308 */

    /* @req SWS_EcuMf_00019 */
    /* @req SWS_EcuMf_00008 */
    SetComMCommunicationAllowed(ALL, TRUE);

    /*
     * time we could leave the state before it has been completed.
     */
    SetCurrentState(ECUM_STATE_APP_RUN);

#if defined(USE_COMM) && (ECUM_AR_VERSION < 40000)
    /* !req SWS_EcuMf_00027
     * ECU in RUN state shall also perform wake up validation of sleeping busses
     */
    {
        uint32 cMask = EcuM_World_ComM_NVM.run_comm_requests;
        uint8  channel;

        for (; cMask; cMask &= ~(1ul << channel)) {
            channel = ilog2(cMask);
            DEBUG_ECUM_CALLOUT_W_ARG("ComM_EcuM_RunModeIndication","%ld",(uint32)channel);
            ComM_EcuM_RunModeIndication(channel);
        }
    }
#endif
}


//--------- Local functions ------------------------------------------------------------------------------------------------



/**
 * Enter GO SLEEP state ( soon in state ECUM_STATE_GO_SLEEP)
 * Note: Not used for safety platform
 */
static inline void enter_go_sleep_mode(void){
    SetCurrentState(ECUM_STATE_GO_SLEEP);

    DEBUG_ECUM_CALLOUT( ECUM_STR "EcuM_OnGoSleep");
    EcuM_OnGoSleep();

#if defined(USE_NVM)
    NvM_WriteAll();

    /* Start timer */
    EcuM_World_go_sleep_state_timeout = EcuM_World.config->EcuMNvramWriteAllTimeout / ECUM_MAIN_FUNCTION_PERIOD;
#endif

}

/**
 * In GO SLEEP state (in state ECUM_STATE_GO_SLEEP)
 * Note: Not used for safety platform
 */
static void in_state_goSleep( void ) {

    /* !req SWS_EcuMf_00026
     * The ECU State Manager Fixed module shall put all communication interfaces to standby state and shall arm the
     * wake up source before the ECU State Manager Fixed module may put the ECU into SLEEP state.
     */

    /* We only wait for NvM_WriteAll() for so long */
    if (EcuM_World_go_sleep_state_timeout != 0) {
        EcuM_World_go_sleep_state_timeout--;
#if defined(USE_NVM)
        {
            NvM_RequestResultType nvmResult;

            if((NvM_GetErrorStatus(0, &nvmResult) != E_OK) || (nvmResult != NVM_REQ_PENDING)) {
                /* Done or something is wrong...continue */
                EcuM_World_go_sleep_state_timeout = 0;
            }
        }
#endif
    }

    if( (EcuM_World_go_sleep_state_timeout == 0) ) {
        /*
         * We should go to sleep , enable source that should wake us
         */
        uint32 cMask;
        uint8  source;
        const EcuM_SleepModeType *sleepModePtr;

        /* Get the current sleep mode */

        /* @req SWS_EcuM_02185 */
        sleepModePtr = &EcuM_World.config->EcuMSleepModeConfig[EcuM_World.sleep_mode];

        cMask = sleepModePtr->EcuMWakeupSourceMask;/* @req SWS_EcuM_02546 */

		/* Loop over the WKSOURCE for this sleep mode */
		/* @req SWS_EcuM_02389 */
		/* @req SWS_EcuM_02546 */
        for (; cMask != 0; cMask &= ~(1UL << source)) {
            /*lint -e{734} -e{9033} -e{397} -e{834} MISRA:OTHER:return value wont exceed more than 32 so its fine:[MISRA 2004 Info, advisory] */
            source = (uint8)(ilog2(cMask));
            //			DEBUG_ECUM_CALLOUT_W_ARG("EcuM_EnableWakeupSources","0x%lx",(1ul<< source));
            EcuM_EnableWakeupSources( 1UL << source );
        }

        /* Let no one else run */
        (void)GetResource(RES_SCHEDULER);
        SetCurrentState(ECUM_STATE_SLEEP);

    } else if( EcuM_GetPendingWakeupEvents() != 0 ) {
        /* @req SWS_EcuM_02188 */

        /* We have pending wakeup events, need to startup again */
#if defined(USE_NVM)
        NvM_CancelWriteAll();
        SetCurrentState(ECUM_STATE_WAKEUP_ONE);
#endif
    } else {
        /* No action (terminating else statement required by MISRA) */
    }
}


/**
 * In "Sleep Sequence I"  (in state ECUM_STATE_SLEEP)
 * Note: Not used for safety platform
 */
static void in_state_sleep ( void ) {
    const EcuM_SleepModeType *sleepModePtr = &EcuM_World.config->EcuMSleepModeConfig[EcuM_World.sleep_mode];

    /* @req SWS_EcuM_02863 */
    DEBUG_ECUM_CALLOUT( ECUM_STR "EcuM_GenerateRamHash");
    EcuM_GenerateRamHash();

    DEBUG_ECUM_CALLOUT_W_ARG("Mcu_SetMode","%d",sleepModePtr->EcuMSleepModeMcuMode);
    Mcu_SetMode(sleepModePtr->EcuMSleepModeMcuMode);

    if( EcuM_CheckRamHash() == 0) {
#if defined(USE_DEM)
        EcuM_ErrorHook(EcuM_World.config->EcuMDemRamCheckFailedEventId);
#endif
    }

    SetCurrentState(ECUM_STATE_WAKEUP_ONE);
}

static inline void go_off_one_module_deinit(void){
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

static inline void enter_go_off_one_mode(void){
    SetCurrentState(ECUM_STATE_GO_OFF_ONE);
    EcuM_OnGoOffOne();

    /* @req SWS_EcuMf_00008 Only set LIN to false when not going to sleep.
     *                Kind a strange that COMM is deini on the next line.
     */
    /* @req SWS_EcuMf_00019 */
    SetComMCommunicationAllowed(ONLY_LIN, FALSE);

    go_off_one_module_deinit(); /*lint !e522 MISRA:CONFIGURATION:This function may not have any functionality depending on configuration:[MISRA 2012 Rule 2.2, required] */
}



/**
 * RUN II Loop (in state ECUM_STATE_APP_RUN)
 * - The entry to RUN II is done in
 * Note: Not used for safety platform
 */
static inline void in_state_appRun(void){

    if (EcuM_World_run_state_timeout != 0){
        EcuM_World_run_state_timeout--;
        LDEBUG_PRINTF( ECUM_STR "RUN Timeout=%ld\n",EcuM_World_run_state_timeout);
    }

    if ((FALSE == hasRunRequests()) && (EcuM_World_run_state_timeout == 0)){
        DEBUG_ECUM_CALLOUT( ECUM_STR "EcuM_OnExitRun");
        EcuM_OnExitRun(); /** @req SWS_EcuM_00865 */

        /* @req SWS_EcuMf_00008 */
        /* @req SWS_EcuMf_00020 */
        SetComMCommunicationAllowed(ALL_WO_LIN, FALSE);

        /* @req SWS_EcuM_00311 */
        SetCurrentState(ECUM_STATE_APP_POST_RUN);   /** @req SWS_EcuM_00865 */
    }
}


/**
 * RUN III states (in state ECUM_STATE_APP_POST_RUN)
 * Note: Not used for safety platform
 */
static inline void in_state_appPostRun(void){

    /* @req SWS_EcuMf_00025 Not enter sleep if we have a run request */

    if (TRUE == hasRunRequests()){
        /* We have run requests, return to RUN II */
        /* @req SWS_EcuM_00866 */
        DEBUG_ECUM_CALLOUT( ECUM_STR "EcuM_enter_run_mode");
        EcuM_enter_run_mode();

    } else if (FALSE == hasPostRunRequests()){
        DEBUG_ECUM_CALLOUT( ECUM_STR "EcuM_OnExitPostRun");
        EcuM_OnExitPostRun(); /** @req SWS_EcuM_00761 */

        SetCurrentState(ECUM_STATE_PREP_SHUTDOWN);
    } else {
        /* NOTE: We have postrun requests */
    }
}


/**
 * PREP SHUTDOWN state (in state ECUM_STATE_PREP_SHUTDOWN)
 * Note: Not used for safety platform
 */
static inline void in_state_prepShutdown(void){

    // The specification does not state what events to clear,
    // assuming all.
    EcuM_ClearWakeupEvent((EcuM_WakeupSourceType)0xFFFFFFFFU);

    DEBUG_ECUM_CALLOUT( ECUM_STR "EcuM_OnPrepShutdown");
    EcuM_OnPrepShutdown();

#if defined(USE_DEM)
    // DEM shutdown
    Dem_Shutdown();
#endif

    // Switch shutdown mode
    switch(EcuM_World.shutdown_target){
        //If in state Off or Reset go into Go_Off_One:
        case ECUM_STATE_OFF:
        case ECUM_STATE_RESET:
            /* @req SWS_EcuM_00288 */
            enter_go_off_one_mode();
            break;
        case ECUM_STATE_SLEEP:
            enter_go_sleep_mode();
            break;
        default:
            ECUM_DET_REPORT_ERROR(ECUM_MAINFUNCTION_ID, ECUM_E_ARC_FAULTY_SHUTDOWN_TARGET);
            break;
    }
}

/* Note: Not used for safety platform */
static inline void in_state_goOffOne(void){

    /* @req SWS_EcuM_00328 */

#if defined(USE_NVM)
    if (0u != EcuM_World_go_off_one_state_timeout){
        EcuM_World_go_off_one_state_timeout--;
    }

    // Wait for the NVM job (NvmWriteAll) to terminate

    Std_ReturnType ret = NvM_GetErrorStatus(0, &writeAllResult);

    if (((ret == E_OK) && (writeAllResult != NVM_REQ_PENDING)) ||
         (EcuM_World_go_off_one_state_timeout == 0)) {
        ShutdownOS(E_OK);
    }
#else
    ShutdownOS(E_OK);
#endif
}


//----- MAIN -----------------------------------------------------------------------------------------------------------------
/* Note: This MainFunciton is when not using safety platform. Safety platform partitions have one main function each. */
void EcuM_MainFunction(void) {
    boolean ret = TRUE;
    EcuM_WakeupSourceType wMask=0;
#if defined(USE_LDEBUG_PRINTF)
    static uint32 validationMask;
#endif
    static uint32 validationMaxTime;
    static uint32 pendingWkupMask = 0;

    /* @req SWS_EcuMf_00029 */
    if (FALSE == EcuM_World.initiated) {
        ret = FALSE;
    }
    if (ret == TRUE) {
#if defined(USE_LDEBUG_PRINTF)
        {
          static EcuM_StateType oldEcuMState = 0xff;
          if( oldEcuMState != EcuM_World.current_state) {
            DEBUG_ECUM_STATE(EcuM_World.current_state);
            oldEcuMState = EcuM_World.current_state;
          }
        }
#endif

        switch (EcuM_World.current_state) {
        case ECUM_STATE_APP_RUN:
            /* RUN II state */
            in_state_appRun();
            break;

        case ECUM_STATE_APP_POST_RUN:
            /* RUN III state */
            in_state_appPostRun();
            break;
        case ECUM_STATE_PREP_SHUTDOWN:
            in_state_prepShutdown();
            break;

        case ECUM_STATE_GO_OFF_ONE:
            in_state_goOffOne();
            break;

        case ECUM_STATE_GO_SLEEP:
          /* 4 cases:
           * 1. Wait for the NvM_WriteAll() - Stay in state
           * 2. Timeout on NvM_WriteAll()   - go to ECUM_STATE_SLEEP (Scheduler is locked)
           * 3. NvM_WriteAll() is done      - go to ECUM_STATE_SLEEP (Scheduler is locked)
           * 4. Run request                 - Call NvM_CancelAll() and go to ECUM_STATE_WAKEUP_ONE.
           */

          in_state_goSleep();

          if( EcuM_World.current_state != ECUM_STATE_SLEEP ) {
            break;
          }


          /*lint -e{825} MISRA:STANDARDIZED_INTERFACE:Flow Through, Scheduler is Locked:[MISRA 2012 Rule 16.3, required]*/
        case ECUM_STATE_SLEEP:
            in_state_sleep();
          /*lint -e{825} MISRA:STANDARDIZED_INTERFACE:Flow Through, Scheduler is Locked:[MISRA 2012 Rule 16.3, required]*/
        case ECUM_STATE_WAKEUP_ONE: {
            DEBUG_ECUM_STATE(EcuM_World.current_state);

            /*@req SWS_EcuM_02975 */

            Mcu_SetMode(EcuM_World.config->EcuMNormalMcuMode);

            wMask = EcuM_GetPendingWakeupEvents();

            DEBUG_ECUM_CALLOUT_W_ARG("EcuM_DisableWakeupSources", "0x%lx", (uint32) wMask);
            EcuM_DisableWakeupSources(wMask);

             /* @req SWS_EcuM_02562 */
            EcuM_AL_DriverRestart(EcuM_World.config);

            EcuM_World.killAllRequest = FALSE;	/* Enable run request again */
            EcuM_World.killAllPostrunRequest = FALSE;  /* Enable run request again */

            (void)ReleaseResource(RES_SCHEDULER);

#if defined(USE_LDEBUG_PRINTF)
            validationMask = 0;
#endif
            validationMaxTime = 0;
            const EcuM_WakeupSourceConfigType *wkupCfgPtr;

            SetCurrentState(ECUM_STATE_WAKEUP_VALIDATION);

            /*-------------- ECUM_STATE_WAKEUP_VALIDATION -------------------- */

            DEBUG_ECUM_CALLOUT(ECUM_STR "EcuM_GetPendingWakeupEvents");
            pendingWkupMask = EcuM_GetPendingWakeupEvents();

            DEBUG_ECUM_CALLOUT_W_ARG("EcuM_StartWakeupSources", "0x%lx",(uint32) pendingWkupMask);

            EcuM_StartWakeupSources(pendingWkupMask);
            EcuM_World.validationTimer = 0;

            /* Calculate the validation timing , if any*/

            /* @req SWS_EcuM_00494 */
            /* @req SWS_EcuM_02479 */

            for (uint32 i = 0; i < ECUM_WKSOURCE_USER_CNT; i++) {
                wkupCfgPtr = &EcuM_World.config->EcuMWakeupSourceConfig[i];

                /* Can't validate something that is not pending */
                if ((wMask & wkupCfgPtr->EcuMWakeupSourceId) != 0) {

                    /* No validation timeout == ECUM_VALIDATION_TIMEOUT_ILL */
                    /* @req SWS_EcuM_02566 */
                    if ((wkupCfgPtr->EcuMValidationTimeout
                            != ECUM_VALIDATION_TIMEOUT_ILL)
                            && ((wMask & wkupCfgPtr->EcuMWakeupSourceId) != 0)) {
                        /* Build a mask with the sources that need validation */
#if defined(USE_LDEBUG_PRINTF)
                        validationMask |= wkupCfgPtr->EcuMWakeupSourceId;
#endif
                        /* Use one validation timeout, take the longest */
                        validationMaxTime =
                                MAX( wkupCfgPtr->EcuMValidationTimeout / ECUM_MAIN_FUNCTION_PERIOD,
                                validationMaxTime);
                        EcuM_World.validationTimer = (validationMaxTime > 0U) ? validationMaxTime : 1U;
                    } else {
                        LDEBUG_PRINTF(ECUM_STR "No Validation for :0x%lx (EcuM_ValidateWakeupEvent() called)\n",
                                (uint32) wkupCfgPtr->EcuMWakeupSourceId);

                        /* Validate right away */
                        /* @req SWS_EcuM_02976 */
                        EcuM_ValidateWakeupEvent(wkupCfgPtr->EcuMWakeupSourceId);
                    }
                }
            }

            break;
        }

        case ECUM_STATE_WAKEUP_VALIDATION: {

            if (EcuM_World.validationTimer != 0) {
                /*
                 * Call EcuM_CheckValidation() while all events have not been validated and
                 * timeout have not expired. The call to EcuM_CheckValidation(..) triggers a call
                 * to EcuM_ValidateWakeupEvent(..) from the driver when validated.
                 */

                /* Check validation for the events that do not match, ie not yet validated */
                DEBUG_ECUM_CALLOUT_W_ARG(
                        "EcuM_CheckValidation",
                        "0x%lx",
                        (uint32)(EcuM_GetValidatedWakeupEvents() ^ pendingWkupMask));

                EcuM_CheckValidation( EcuM_GetValidatedWakeupEvents() ^ pendingWkupMask);
                /* @req SWS_EcuM_00495*/


                if (0 != EcuM_GetValidatedWakeupEvents()) {
                    /* All events have been validated */
                    EcuM_ValidateWakeupEvent(EcuM_GetValidatedWakeupEvents() ^ pendingWkupMask);
                    SetCurrentState(ECUM_STATE_WAKEUP_REACTION);
                } else {
                    LDEBUG_PRINTF( ECUM_STR "  Awaiting validation for mask: pending=%lx, expected=%lx\n",
                                    pendingWkupMask, validationMask);
                    LDEBUG_PRINTF(ECUM_STR "  Validation Timer            : %lu\n", EcuM_World.validationTimer);
                }

            } else {
                uint32 notValidatedMask = EcuM_GetValidatedWakeupEvents() ^ pendingWkupMask;

                /* Stop wakeupSources that are not validated */
                if (notValidatedMask != 0) {
                    DEBUG_ECUM_CALLOUT_W_ARG("EcuM_StopWakeupSources", "0x%lx",
                            (uint32) notValidatedMask);
                    EcuM_StopWakeupSources(notValidatedMask);

#if defined(USE_BSWM)
                    BswM_EcuM_CurrentWakeup(notValidatedMask, ECUM_WKSTATUS_EXPIRED);
#endif
                }
                SetCurrentState(ECUM_STATE_WAKEUP_REACTION);
            }

            if (EcuM_World.validationTimer != 0) {
                EcuM_World.validationTimer--;
            }

            break;
        }

        case ECUM_STATE_WAKEUP_REACTION: {
            /*
             * At this stage we want to know how to react to the wakeup, e.g. go
             * back to RUN or SHUTDOWN, etc.
             */
            EcuM_WakeupReactionType wReaction;

            wMask = EcuM_GetValidatedWakeupEvents();
            LDEBUG_PRINTF(ECUM_STR "EcuM_GetValidatedWakeupEvents() : %x\n", wMask);

            /* NOTE: We have skipped the TTII timer here */

            /* If the wakeup mask here is != 0 we have a validated wakeup event ->
             * go back to RUN */
            wReaction = (0 == wMask) ? ECUM_WKACT_SHUTDOWN : ECUM_WKACT_RUN;
            wReaction = EcuM_OnWakeupReaction(wReaction);

            LDEBUG_PRINTF(ECUM_STR "Wakeup Reaction: %s\n", GetWakeupReactionAsString(wReaction));

            if (wReaction == ECUM_WKACT_RUN) {
                /* @req SWS_EcuM_00568 */
                SetCurrentState(ECUM_STATE_WAKEUP_TWO);
            } else {
                /* From figure 28 it seems that we should go to SHUTDOWN/GO SLEEP) again from wakeup
                 * not going up to RUN/RUN II state again. */
                /* @req SWS_EcuM_00711 */
                /* @req SWS_EcuM_00567 */
                SetCurrentState(ECUM_STATE_GO_SLEEP);
            }
            break;
        }

        case ECUM_STATE_WAKEUP_TWO:
#if defined(USE_DEM)
            Dem_Init();
#endif
            EcuM_enter_run_mode();
            break;

        default:
            ECUM_DET_REPORT_ERROR(ECUM_MAINFUNCTION_ID, ECUM_E_ARC_FAULTY_CURRENT_STATE);
            break;
        }
    }
}
__CODE_COVERAGE_ON__
#endif /*!(defined(ECUM_ARC_SAFETY_PLATFORM))*/
