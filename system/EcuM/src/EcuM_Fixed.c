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

#include "Std_Types.h"
#include "EcuM.h"
#include <string.h>
#include <Os.h>
#include "EcuM_Generated_Types.h"
#include "EcuM_Internals.h"
#include "EcuM_Cbk.h"
#include "SchM_EcuM.h"
/*lint -e451 MISRA:EXTERNAL_FILE:file inclusion:[MISRA 2012 Directive 4.10, required] */
#include "MemMap.h"
#include "Mcu.h"
#include "ComStack_Types.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#if defined(USE_DEM)
#include "Dem.h"                /* @req SWS_EcuM_02875 */
#endif
#include "isr.h"
#if defined(USE_NVM)
#include "NvM.h"
#endif
#if defined(USE_RTE)
#include "Rte_Main.h"
#endif
#if defined(USE_SCHM)
#include "SchM.h"
#endif

/* @req SWS_EcuM_02757 The ECU Manager shall treat all errors immediately as errors. */

//#define USE_LDEBUG_PRINTF
#include "debug.h"

#if (ECUM_ARC_SAFETY_PLATFORM == STD_OFF)
/** @CODECOV:NOT_SUPPORTED:SafetyPlatform has its own partitioned ECUM main functions **/
__CODE_COVERAGE_OFF__

/** EcuM_StartupTwo
 * NOTE: This is the normal StartupTwo function for EcuM Fixed. It is not used in safety platform.
 * The order defined here is found in SWS_EcuM_02411
 */
void EcuM_StartupTwo(void) {

    if (EcuM_World.current_state == ECUM_STATE_STARTUP_ONE) {
    
        SetCurrentState(ECUM_STATE_STARTUP_TWO);

#if defined(USE_SCHM)
        /* Initialize the BSW scheduler */
        SchM_Init();
#endif
    }

    if (EcuM_World.current_state == ECUM_STATE_STARTUP_TWO) {

        EcuM_AL_DriverInitTwo(EcuM_World.config);

#if defined(USE_NVM)
        // Start timer to wait for NVM job to complete
        TickType tickTimerStart = GetOsTick();
#endif

#if defined(USE_RTE)
        // Prepare the system to startup RTE
        (void)Rte_Start();
#endif

#if defined(USE_NVM)
        /* Wait for the NVM job (NvM_ReadAll) to terminate. This assumes that:
         * - A task runs the memory MainFunctions, e.g. Ea_MainFunction(), Eep_MainFunction()
         *   are run in a higher priority task that the task that executes this code.
         */
#if defined(CFG_ARC_ECUM_NVM_READ_INIT) && defined(USE_FEE)
        EcuM_Arc_InitFeeReadAdminBlock(tickTimerStart, EcuM_World.config->EcuMNvramReadAllTimeout);
#endif

        TickType tickTimerElapsed;
        static NvM_RequestResultType readAllResult;
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
        // Initialize drivers that need NVRAM data
        EcuM_AL_DriverInitThree(EcuM_World.config);

    }

    // IMPROVEMENT: Indicate mode change to RTE

    /* IMPROVEMENT: If one of the wake up sources listed in 7.8.7 Wake up Sources and Reset Reason is set, then
     * exection shall continue with RUN state. In all other cases, execution shall continue with
     * WAKEUP VALIDATION state.
     * */
    EcuM_enter_run_mode();

}
#endif /*ECUM_ARC_SAFETY_PLATFORM == STD_OFF*/
__CODE_COVERAGE_ON__


/* Note: This is a generic public function for EcuM used both in normal EcuM Fixed and in Safety platform */
/* @req SWS_EcuM_00423 */
Std_ReturnType EcuM_GetState(EcuM_StateType* state) {
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    VALIDATE_RV(EcuM_World.initiated, ECUM_GETSTATE_ID, ECUM_E_UNINIT, E_NOT_OK);VALIDATE_RV(state != NULL, ECUM_GETSTATE_ID, ECUM_E_NULL_POINTER, E_NOT_OK);

    *state = EcuM_World.current_state;

    return E_OK;
}

/* Note: This is a generic public function for EcuM used both in normal EcuM Fixed and in Safety platform */
Std_ReturnType EcuM_RequestRUN(EcuM_UserType user) {
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    VALIDATE_RV(EcuM_World.initiated, ECUM_REQUESTRUN_ID, ECUM_E_UNINIT, E_NOT_OK);
    VALIDATE_RV(user < ECUM_USER_ENDMARK, ECUM_REQUESTRUN_ID, ECUM_E_INVALID_PAR, E_NOT_OK); //lint !e638 !e641 Ok. Data is validated

    //  LDEBUG_PRINTF("EcuM_RequestRUN(): User %d\n",user);
    if( FALSE == EcuM_World.killAllRequest ) {
        EcuM_World.run_requests |= (uint32) 1 << user;
    }

    /* @req SWS_EcuM_00144 */
    return E_OK;
}

/* Note: This is a generic public function for EcuM used both in normal EcuM Fixed and in Safety platform */
Std_ReturnType EcuM_ReleaseRUN(EcuM_UserType user) {
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    VALIDATE_RV(EcuM_World.initiated, ECUM_RELEASERUN_ID, ECUM_E_UNINIT, E_NOT_OK);
    VALIDATE_RV(user < ECUM_USER_ENDMARK, ECUM_RELEASERUN_ID, ECUM_E_INVALID_PAR, E_NOT_OK); //lint !e638 !e641 Ok. Data is validated

    /* @req SWS_EcuM_00144 */
#if defined(DEBUG_RELEASE_RUN)
    LDEBUG_PRINTF("EcuM_ReleaseRUN(): User %d\n",user);
#endif
    EcuM_World.run_requests &= ~((uint32) 1 << user);

    return E_OK;
}

#if (ECUM_ARC_SAFETY_PLATFORM == STD_OFF)
/**
 * Kill all RUN requests and perform shutdown without waiting for application
 * to finish
 * Note: EcuM_KillAllRUNRequest currently not supported in Safety platform. Only used in normal EcuM Fixed.
 * 
 */
void EcuM_KillAllRUNRequests(void) {
    /* @req SWS_EcuM_00872 */
    /* @req SWS_EcuM_00600 */

    /* @req SWS_EcuM_00668 */
    EcuM_World.killAllRequest = TRUE;

    EcuM_World.run_requests = 0;
#if defined(USE_COMM)
    EcuM_World_ComM_NVM.run_comm_requests = 0;
#endif

#if defined(USE_DEM)

    Dem_ReportErrorStatus(EcuM_World.config->EcuMDemAllRunRequestsKilledEventId , DEM_EVENT_STATUS_FAILED );
#endif

}
#endif

/* Note: This is a generic public function for EcuM used both in normal EcuM Fixed and in Safety platform */
void EcuM_KillAllPostRUNRequests(void) {

    EcuM_World.killAllPostrunRequest = TRUE;
    /* @req SWS_EcuMf_00102 */
    /* @req SWS_EcuMf_00103 */
    EcuM_World.postrun_requests = 0;

}

/* Note: This is a generic public function for EcuM used both in normal EcuM Fixed and in Safety platform */
Std_ReturnType EcuM_RequestPOST_RUN(EcuM_UserType user) {
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    VALIDATE_RV(EcuM_World.initiated, ECUM_REQUESTPOSTRUN_ID, ECUM_E_UNINIT, E_NOT_OK);
    VALIDATE_RV(user < ECUM_USER_ENDMARK, ECUM_REQUESTPOSTRUN_ID, ECUM_E_INVALID_PAR, E_NOT_OK); //lint !e638 !e641 Ok. Data is validated

    /* @req SWS_EcuMf_00103 */
    if( FALSE == EcuM_World.killAllPostrunRequest ) {
        EcuM_World.postrun_requests |= (uint32) 1 << user;
    }

    return E_OK;
}

/* Note: This is a generic public function for EcuM used both in normal EcuM Fixed and in Safety platform */
Std_ReturnType EcuM_ReleasePOST_RUN(EcuM_UserType user) {
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    VALIDATE_RV(EcuM_World.initiated, ECUM_RELEASEPOSTRUN_ID, ECUM_E_UNINIT, E_NOT_OK);
    VALIDATE_RV(user < ECUM_USER_ENDMARK, ECUM_RELEASEPOSTRUN_ID, ECUM_E_INVALID_PAR, E_NOT_OK); //lint !e638 !e641 Ok. Data is validated

    EcuM_World.postrun_requests &= ~((uint32) 1 << user);

    return E_OK;
}

/* Note: This is a generic public function for EcuM used both in normal EcuM Fixed and in Safety platform */
EcuM_WakeupStatusType EcuM_GetStatusOfWakeupSource( EcuM_WakeupSourceType sources ) {
    /* @req SWS_EcuM_00754 */

    EcuM_WakeupStatusType status = ECUM_WKSTATUS_NONE;

    if ((EcuM_GetValidatedWakeupEvents() & sources) != 0) {
        status = ECUM_WKSTATUS_VALIDATED;
    } else if ((EcuM_GetPendingWakeupEvents() & sources) != 0) {
        status = ECUM_WKSTATUS_PENDING;

    } else {
        status = ECUM_WKSTATUS_NONE;
    }
    return status;

}
