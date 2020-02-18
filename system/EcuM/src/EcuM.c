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
/** @fileSafetyClassification ASIL **/ /* Common functionality for all EcuM variants (Flexible, Fixed, Safety Platform) */

/* ----------------------------[information]----------------------------------*/
/*
 *
 *
 * Description:
 *   Implements the EcuM (fixed) module
 *
 * Support:
 *   General                  Have Support
 *   -------------------------------------------
 *   ECUM_TTII_ENABLED            		N
 *   ECUM_DEV_ERROR_DETECT				Y
 *   ECUM_VERSION_INFO_API				Y
 *   ECUM_INCLUDE_DEM					N (controlled by USE_x macro's instead)
 *   ECUM_INCLUDE_NVRAM_MGR				N (controlled by USE_x macro's instead)
 *   ECUM_INLCUDE_DET					N (controlled by USE_x macro's instead)
 *   ECUM_MAIN_FUNCTION_PERDIOD			Y
 *   ECUM_TTII_WKSOURCE					N
 *
 *   Configuration            Have Support
 *   -------------------------------------------
 *   ECUM_SLEEP_ACTIVITY_PERIOD			?
 *   ECUM_CONFIGCONSISTENCY_HASH		N
 *   ECUM_RUN_SELF_REQUEST_PERIOD		?
 *   ECUM_NVRAM_WRITEALL_TIMEOUT		Y
 *   ECUM_DEFAULT_APP_MODE				?
 *
 *
 *   DefaultShutdownTarget
 *   -------------------------------------------
 *   ECUM_DEFAULT_SHUTDOWN_TARGET		N
 *
 */

/* ----------------------------[includes]------------------------------------*/

/* @req SWS_EcuM_02986 */
/* @req SWS_EcuM_02987 */
/* @req SWS_EcuM_03009 */
/* @req SWS_EcuM_03023 */
/* @req SWS_EcuM_02862 */
/* @req SWS_EcuM_02810 */
/* @req SWS_EcuM_02757 The ECU Manager shall treat all errors immediately as errors. */
/* @req SWS_EcuM_02758 The ECU Manager shall not recover from an error */
/* @req SWS_EcuM_02759 The ECU Manager Module shall report all errors as events. */
/* @req SWS_EcuM_02763 Select boot target not supported */
/* @req SWS_EcuM_02858 */
/* @req SWS_EcuM_02859 */
/** These requirements are supported in .swcd file */
/* @req SWS_EcuM_03018 */ /* @req SWS_EcuM_03017 */ /* @req SWS_EcuM_03012 */ /* @req SWS_EcuM_03011 */ /* @req SWS_EcuM_02979 */

/* @req SWS_EcuM_04067 EcuM_UserType is defined by the RTE (RTE_EcuM_Type.h) */
/* @req SWS_EcuM_04042 EcuM_BootTargetType is defined by the RTE (RTE_EcuM_Type.h) */

/* @req SWS_BSW_00005 */
/* @req SWS_BSW_00004 */
#include "Std_Types.h"
#include "EcuM.h"
#include <string.h>
#include <Os.h>
#include "EcuM_Generated_Types.h"
#if defined(USE_HTMSS)
#include "Htmss.h"
#endif
#include "EcuM_Internals.h"
#include "EcuM_Cbk.h"
#include "SchM_EcuM.h"
#if defined(USE_BSWM)
#include "BswM_EcuM.h"
#endif
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
#if defined(USE_HTMSS)
#include "Htmss.h"
#endif


//#define USE_LDEBUG_PRINTF
#include "debug.h"

/* ----------------------------[Memory Partition]---------------------------*/

 /* @req SWS_EcuM_04035 */
/* @req SWS_BSW_00006 */
#define ECUM_START_SEC_VAR_CLEARED_GLOBALMASTER_UNSPECIFIED
/*lint -save -e9019 MISRA:OTHER:suppressed due to EcuM_MemMap.h include is needed:[MISRA 2012 Rule 20.1, advisory] */
#include "EcuM_BswMemMap.h"
EcuM_GlobalType EcuM_World;
#define ECUM_STOP_SEC_VAR_CLEARED_GLOBALMASTER_UNSPECIFIED
#include "EcuM_BswMemMap.h"

#define ECUM_START_SEC_VAR_CLEARED_GLOBALMASTER_UNSPECIFIED
#include "EcuM_BswMemMap.h"
uint32 EcuM_World_run_state_timeout = 0;
#define ECUM_STOP_SEC_VAR_CLEARED_GLOBALMASTER_UNSPECIFIED
#include "EcuM_BswMemMap.h"

#define ECUM_START_SEC_VAR_CLEARED_COMNVM_UNSPECIFIED
#include "EcuM_BswMemMap.h"
EcuM_GlobalType_Partition_COMMNVM EcuM_World_ComM_NVM;
#define ECUM_STOP_SEC_VAR_CLEARED_COMNVM_UNSPECIFIED
#include "EcuM_BswMemMap.h"
/*lint -restore */

/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
#if  ( ECUM_DEV_ERROR_DETECT == STD_ON )
void CheckWakeupSourceValidity(EcuM_WakeupSourceType sources, uint8 FunctionId);
#endif

/* ----------------------------[private functions]---------------------------*/

#if !defined(USE_BSWM)
#define BswM_EcuM_CurrentWakeup(source, state)
#endif

#if  ( ECUM_DEV_ERROR_DETECT == STD_ON )
void CheckWakeupSourceValidity(EcuM_WakeupSourceType sources, uint8 FunctionId) {
    EcuM_WakeupSourceType wkSource;
    const EcuM_SleepModeType *sleepModePtr;

    sleepModePtr = &EcuM_World.config->EcuMSleepModeConfig[EcuM_World.sleep_mode];
    wkSource =  sleepModePtr->EcuMWakeupSourceMask;

    /* @req SWS_EcuM_02625 */
    /* Predefined source does not require validation */
    if ( ( sources & (ECUM_WKSOURCE_POWER | ECUM_WKSOURCE_RESET
                    | ECUM_WKSOURCE_INTERNAL_RESET | ECUM_WKSOURCE_INTERNAL_WDG
                    | ECUM_WKSOURCE_EXTERNAL_WDG | ECUM_WKSOURCE_HWTEST_RESET) ) != 0 ) {
        /*Ok*/
    }

    else if( !((sources | wkSource) ==  wkSource)) {
        ECUM_DET_REPORT_ERROR(FunctionId, ECUM_E_UNKNOWN_WAKEUP_SOURCE );
        /*lint -e{904} MISRA:OTHER:Return statement is necessary in case of reporting a DET error:[MISRA 2012 Rule 15.5, advisory]*/
        return;
    } else {
        /* do nothing */
    }

    return;
}
#else
#define CheckWakeupSourceValidity(sources, FunctionId)
#endif


#if defined(CFG_POSTBUILD)
static boolean ValidatePostBuildConfiguration(const EcuM_ConfigType* config) {

    boolean status = TRUE;

    /* The hash is calculated both when generating the pre compile configuration
     * and the postbuild configuration. It is based on all pre compile configuration
     * parameters.
     * Generator version and vendor id also needs to be a part of the input to the
     * hash.
     */

    /* Chech that the pointer at least isn't NULL */
    if (config->PostBuildConfig == NULL) {
        status = FALSE;
    }

    /* Just a basic check that the it seems to be a valid post build area */
    else if (0x5A5A5A5A != config->PostBuildConfig->startPattern) {
        status = FALSE;
    }

    /* Check the variant  */
    else if (config->EcuMPostBuildVariant != config->PostBuildConfig->postBuildVariant) {
        status = FALSE;
    }

    /* Check the hash */
    else if ((config->EcuMConfigConsistencyHashLow != config->PostBuildConfig->preCompileHashLow) ||
        (config->EcuMConfigConsistencyHashHigh != config->PostBuildConfig->preCompileHashHigh)) {
        status = FALSE;
    } else {
        status = TRUE;
    }
    return status;
}

#endif

#if defined(USE_HTMSS)
/**
 * @brief   Get test results from Htmss
 * @param group
 */
static void EcuM_GetHWTestResults( HTMSS_TestGroupType group ) {

    HTMSS_TestStatusType rv;

    rv = HTMSS_GetTestStatus(group, NULL_PTR);

    if(rv != HTMSS_STATUS_OK) {
        HTMSS_StartupTestErrorHook();
    }
}
#endif

/* ----------------------------[public functions]----------------------------*/

#if !defined(USE_DET) && (ECUM_DEV_ERROR_DETECT == STD_ON)
#error EcuM configuration error. DET is not enabled when ECUM_DEV_ERROR_DETECT is set
#endif

/* @req SWS_EcuM_02411 */ /* @req SWS_EcuM_02684 */
/* @req SWS_EcuM_02980 */ /* Init function of all Bsw modules doesn't have return value hence error is ignored implicitly  */
/* @req SWS_BSW_00071 */
/**
 * Initialize EcuM.
 */
void EcuM_Init(void) {
    Mcu_ResetType resetReason;
    /* @req SWS_EcuM_04015 */
    boolean ret = TRUE;
#if (OS_NUM_CORES > 1)
    if (OS_CORE_IS_ID_MASTER(GetCoreID())) {
#endif
        Std_ReturnType status;
#if defined(USE_ECUM_FIXED)
        SetCurrentState(ECUM_STATE_STARTUP_ONE);
#endif
        // Initialize drivers that are needed to determine PostBuild configuration
        EcuM_AL_DriverInitZero();
        // Initialize the OS
        InitOS();
        // Setup interrupts
        Os_IsrInit();
        // Determine PostBuild configuration
        EcuM_World.config = EcuM_DeterminePbConfiguration();

#if defined(USE_HTMSS)
        // Initialize Safety Monitor
        /* @req ARC_SWS_ECUM_00012 SMAL_Init allowed to run before HTMSS_Init request test configuration */
        Smal_Init();
        HTMSS_Init(0u); /*lint !e910 MISRA:ARGUMENT CHECK:HTMSS_Init does not have support for using input arguments so the value sent is not relevant:[MISRA 2012 Rule 11.9, required] */
#endif

#if defined(CFG_POSTBUILD)
        /* @req SWS_EcuM_02796 */
        /* @req SWS_EcuM_02798 */
        if (FALSE == ValidatePostBuildConfiguration(EcuM_World.config)) {
#if defined(USE_DEM)
            EcuM_ErrorHook(EcuM_World.config->EcuMDemInconsistencyEventId);
#endif
            ret = FALSE;
        }
#endif
        /*lint -e{774} MISRA:CONFIGURATION:This may be FALSE if CFG_POSTBUILD is enabled: [MISRA 2012 Rule 14.3,required] */
        if (ret == TRUE) {
            // Initialize drivers needed before the OS-starts
            EcuM_AL_DriverInitOne(EcuM_World.config);

            resetReason = Mcu_GetResetReason();
            // Determine the reset/wakeup reason
            /* @req SWS_EcuM_02623 */ /* Validated immediately */
            switch (resetReason) {
            case MCU_POWER_ON_RESET:
                EcuM_ValidateWakeupEvent(ECUM_WKSOURCE_POWER);
                break;
            case MCU_SW_RESET:
            case MCU_RESET_UNDEFINED:
                EcuM_ValidateWakeupEvent(ECUM_WKSOURCE_RESET); /* @req SWS_EcuM_02601 ECUM_WKSOURCE_RESET shall be reported if no specific wk source detected  */
                break;
            case MCU_WATCHDOG_RESET:
                EcuM_ValidateWakeupEvent(ECUM_WKSOURCE_INTERNAL_WDG);
                break;
#if defined(USE_HTMSS)
            case MCU_HWTEST_RESET:
                EcuM_GetHWTestResults(HTMSS_SHUTDOWN);
                EcuM_ValidateWakeupEvent(ECUM_WKSOURCE_HWTEST_RESET);
                break;
#endif
            default:
                EcuM_Arc_RememberWakeupEvent((uint32)resetReason);
                break;
            }
#if defined(USE_HTMSS)
            Std_ReturnType testReturn = HTMSS_StartTest(HTMSS_STARTUP);

            if (testReturn == E_OK) {
                EcuM_GetHWTestResults(HTMSS_STARTUP);
            }
#endif
            /* Moved this here because EcuM_SelectShutdownTarget needs us to be initilized.*/
            EcuM_World.initiated = TRUE;

            /* Set default shutdown target */
            /* @req SWS_EcuM_02181 */
            status = EcuM_SelectShutdownTarget(EcuM_World.config->EcuMDefaultShutdownTarget,
                                               EcuM_World.config->EcuMDefaultSleepMode);
            if (status == E_OK) {

                EcuM_World.run_requests = 0;
                EcuM_World.postrun_requests = 0;
#if (ECUM_RESET_LOOP_DETECTION == STD_ON)
                /* NOTE: This returns true if a loop is detected. The spec does not say */
                /*      what to do if that happens. */
                EcuM_LoopDetection();
#endif
                // Start this baby up
                /* @req SWS_EcuMf_00010 */
                /* @req SWS_EcuM_02603 */
#if (OS_NUM_CORES > 1)
                StatusType coreStatus;
                StartCore(GetCoreID() + 1, &coreStatus);
#endif

                StartOS(EcuM_World.config->EcuMDefaultAppMode);  /* @req SWS_EcuM_00243 */
            }
        }
#if (OS_NUM_CORES > 1)
    } else { /* @req SWS_EcuM_04017 */
        InitOS();
        Os_IsrInit();
        StatusType coreStatus;
        StartCore(GetCoreID() + 1, &coreStatus);
        StartOS(OSDEFAULTAPPMODE);
    }
#endif
}

/* @req SWS_EcuM_04020 */
// Typically called from OS shutdown hook
void EcuM_Shutdown(void) {
#if defined(USE_ECUM_FIXED)
    SetCurrentState(ECUM_STATE_GO_OFF_TWO);
#endif

#if defined(USE_HTMSS)
    Std_ReturnType testReturn;

    if ((EcuM_World.shutdown_target == ECUM_STATE_HWTEST_OFF) || (EcuM_World.shutdown_target == ECUM_STATE_HWTEST_RESET) ) {
        testReturn = HTMSS_StartTest(HTMSS_SHUTDOWN);

        if (testReturn == E_NOT_OK) {
            HTMSS_ShutdownTestErrorHook();
        }
    }
#endif
    // Let the last drivers do a nice shutdown
#if (ECUM_ARC_SAFETY_PLATFORM == STD_ON)
    // As this is running in Supervisor mode (from SYS_CALL_ShutdownOS, both the partition call be called here
    EcuM_OnGoOffTwo_Partition_A0(); /*@req  ARC_SWS_ECUM_00002*/
    EcuM_OnGoOffTwo_Partition_QM();
#else
    EcuM_OnGoOffTwo();
#endif
    if (EcuM_World.shutdown_target == ECUM_STATE_OFF) {
        EcuM_AL_SwitchOff();          /* @req SWS_EcuM_04075 */
    } else {
        EcuM_AL_Reset(EcuM_World.sleep_mode);  /* @req SWS_EcuM_04074 */
    }
}

Std_ReturnType EcuM_SelectShutdownTarget(EcuM_StateType shutdownTarget,
                uint8 sleepMode) {
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    VALIDATE_RV(EcuM_World.initiated, ECUM_SELECTSHUTDOWNTARGET_ID, ECUM_E_UNINIT, E_NOT_OK);

#if (ECUM_DEV_ERROR_DETECT == STD_ON)
    boolean b_shutdownTarget = ((shutdownTarget == ECUM_STATE_OFF)      ||
                                 (shutdownTarget == ECUM_STATE_RESET)    ||
#if defined(USE_HTMSS)
                                 (shutdownTarget == ECUM_STATE_HWTEST_OFF)   ||
                                 (shutdownTarget == ECUM_STATE_HWTEST_RESET) ||
#endif
                                 (shutdownTarget == ECUM_STATE_SLEEP));
#endif

    /* @req SWS_EcuM_00624 */
    VALIDATE_RV(b_shutdownTarget, ECUM_SELECTSHUTDOWNTARGET_ID, ECUM_E_STATE_PAR_OUT_OF_RANGE, E_NOT_OK);

    LDEBUG_PRINTF("EcuM_SelectShutdownTarget(): shutdownTarget=%s, sleepMode=%d\n",
            GetMainStateAsString(shutdownTarget),
            sleepMode);

    /* @req SWS_EcuM_02585 */
    /* @req SWS_EcuM_00232 */
    /* @req SWS_EcuM_00624 */
    EcuM_World.shutdown_target = shutdownTarget;
    EcuM_World.sleep_mode = sleepMode;

    return E_OK;
}

Std_ReturnType EcuM_GetShutdownTarget(EcuM_StateType* shutdownTarget,
                uint8* sleepMode)
{
    /* @req SWS_EcuM_02788 */
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    /* @req SWS_BSW_00212 NULL pointer check */
    VALIDATE_RV(EcuM_World.initiated, ECUM_GETSHUTDOWNTARGET_ID, ECUM_E_UNINIT, E_NOT_OK);
    VALIDATE_RV(shutdownTarget != NULL, ECUM_GETSHUTDOWNTARGET_ID, ECUM_E_NULL_POINTER, E_NOT_OK);
    VALIDATE_RV(sleepMode != NULL, ECUM_GETSHUTDOWNTARGET_ID, ECUM_E_NULL_POINTER, E_NOT_OK);

    *shutdownTarget = EcuM_World.shutdown_target;
    *sleepMode = EcuM_World.sleep_mode;

    return E_OK;
}

Std_ReturnType EcuM_SelectBootTarget(EcuM_BootTargetType target) {
    // Not supported
    (void)target;
    return E_NOT_OK;
}

Std_ReturnType EcuM_GetBootTarget(EcuM_BootTargetType* target) {
    // Not supported
    //lint -estring(920,pointer)  /* cast to void */
    (void) target;
    //lint +estring(920,pointer)  /* cast to void */
    return E_NOT_OK;
}

Std_ReturnType EcuM_GetLastShutdownTarget(EcuM_StateType* shutdownTarget, uint8* sleepMode) {
    /* NOT SUPPORTED */

    /*lint --e{920} MISRA:STANDARDIZED_INTERFACE:cast is being made from the given type to void:[MISRA 2012 Rule 1.3, required] */
    (void)shutdownTarget;
    (void)sleepMode;

    return E_NOT_OK;

}




/**
 *
 * @param sources
 */
/* @req SWS_EcuM_02572 */ /* !req SWS_EcuM_04012 */ /* !req [SWS_EcuM_02712] */
void EcuM_SetWakeupEvent(EcuM_WakeupSourceType sources) {

    EcuM_WakeupSourceType wkSource;

    DEBUG_ECUM_CALLIN_W_ARG("EcuM_SetWakeupEvent","0x%lx",(uint32)sources);

    /* @req SWS_EcuM_02826 */
    /* @req SWS_EcuM_02171 */
    /* @req SWS_EcuM_02976 */
    /* @req SWS_EcuM_02565 */ /*Validate at once for pre-defined sources */
    wkSource = (ECUM_WKSOURCE_POWER | ECUM_WKSOURCE_RESET
                    | ECUM_WKSOURCE_INTERNAL_RESET | ECUM_WKSOURCE_INTERNAL_WDG
                    | ECUM_WKSOURCE_EXTERNAL_WDG);

    /* Sources that should be validated at once */
    wkSource &= sources;
    /* @req SWS_EcuM_02975 */
    if (wkSource != 0) {
        /* Validate pre-defined source at once */
        EcuM_ValidateWakeupEvent(wkSource);
    }

    /* Don't add pre-defined source to PENDING */
    sources ^= wkSource;

    if (sources != 0 ) {
        /* @req SWS_EcuM_02867 */
#if  ( ECUM_DEV_ERROR_DETECT == STD_ON )
        /* Get user defined sources */
        wkSource = EcuM_World.config->EcuMSleepModeConfig[EcuM_World.sleep_mode].EcuMWakeupSourceMask;

        /* Add always validated sources */
        wkSource |= (ECUM_WKSOURCE_POWER|ECUM_WKSOURCE_RESET | ECUM_WKSOURCE_INTERNAL_RESET |
                ECUM_WKSOURCE_INTERNAL_WDG | ECUM_WKSOURCE_EXTERNAL_WDG);

        if( !((sources | wkSource) == wkSource))
        {
            ECUM_DET_REPORT_ERROR(ECUM_SETWAKEUPEVENT_ID, ECUM_E_UNKNOWN_WAKEUP_SOURCE );
            /*lint -e{904} MISRA:ARGUMENT_CHECK:Return statement is necessary in case of reporting a DET error:[MISRA 2012 Rule 15.5, advisory] */
            return;
        }
#endif

        /* @req SWS_EcuM_01117 */
        EcuM_World.wakeupEvents |= sources;
#if defined(USE_BSWM)
        BswM_EcuM_CurrentWakeup(sources, ECUM_WKSTATUS_PENDING);
#endif
    }
    /* @req SWS_EcuM_02707 */
    /* @req SWS_EcuM_00714 */
    /* @req SWS_EcuM_02565 */
    /* @req SWS_EcuM_02709 */ /*@req SWS_EcuM_04004 */
    EcuM_World.validationTimer = ECUM_VALIDATION_TIMEOUT / ECUM_MAIN_FUNCTION_PERIOD; //lint !e835 PC-lint violation: Allow zero value as left parameter for configuration

}


/**
 * Clear wakeup events
 *
 * @return
 */
void EcuM_ClearWakeupEvent(EcuM_WakeupSourceType source) {
    /* @req SWS_EcuM_02807 */
    /* @req SWS_EcuM_02683 */
    EcuM_World.wakeupEvents &= ~(uint32)source;     /* Clear pending events */
    EcuM_World.validEvents &= ~(uint32)source;      /* Clear validated events */
}

/**
 * Get the pending wakeup events.
 *
 * @return
 */
EcuM_WakeupSourceType EcuM_GetPendingWakeupEvents(void) {
    /* @req SWS_EcuM_02827 */
    /* @req SWS_EcuM_02172  */


    /* @req SWS_EcuM_01156 */
    return EcuM_World.wakeupEvents;

}

/* @req SWS_EcuM_02532 */
EcuM_WakeupSourceType EcuM_GetValidatedWakeupEvents(void) {
    // is not really an enum.
    /* @req SWS_EcuM_02496 */
    /* @req SWS_EcuM_02533 */ /* @req SWS_EcuM_03003 */
    return EcuM_World.validEvents;

}

/**
 *
 * @param sources
 */
void EcuM_ValidateWakeupEvent(EcuM_WakeupSourceType sources) {
    /* @req SWS_EcuM_02625 */ /*No validation of any source */

    /* @req SWS_EcuM_00344 */
    /* @req SWS_EcuM_02868 */
    /* @req SWS_EcuM_02345 */

    CheckWakeupSourceValidity(sources, ECUM_VALIDATE_WAKEUP_EVENT_ID);

    /* @req SWS_EcuM_02496 */ /* @req SWS_EcuM_04078 */
    EcuM_World.validEvents |= sources;

    /* @req SWS_EcuM_02645 */
#if defined(USE_COMM)
    const EcuM_WakeupSourceConfigType *wkupCfgPtr;
    for (uint32 i = 0; i < ECUM_WKSOURCE_USER_CNT; i++) {
        wkupCfgPtr = &EcuM_World.config->EcuMWakeupSourceConfig[i];
        if ( ((sources & wkupCfgPtr->EcuMWakeupSourceId) != 0) && (wkupCfgPtr->EcuMComMChannel != ECUM_COMM_CHANNEL_ILL) ) {
            /*@req SWS_EcuM_00480*/
            ComM_EcuM_WakeUpIndication(wkupCfgPtr->EcuMComMChannel);
        }
    }
#endif
    /* !req SWS_EcuM_02790 */
    /* !req SWS_EcuM_02791 */
    /* @req SWS_EcuM_04003 *//* @req SWS_EcuM_04079 */
#if (ECUM_ARC_SAFETY_PLATFORM == STD_OFF)    
    BswM_EcuM_CurrentWakeup(sources, ECUM_WKSTATUS_VALIDATED);
#endif
}
/* @req SWS_BSW_00064 GetVersionInfo shall execute synchonously */
/* @req SWS_BSW_00052 GetVersion info shall only have one parameter */
/* @req SWS_BSW_00164 No restriction on calling context */
#if ( ECUM_VERSION_INFO_API == STD_ON )
void EcuM_GetVersionInfo( Std_VersionInfoType* VersionInfo )
{

    /** @req SWS_EcuMf_00034 */
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    /* @req SWS_BSW_00212 NULL pointer check */
    VALIDATE_NO_RV((VersionInfo != NULL_PTR),ECUM_GETVERSIONINFO_ID,ECUM_E_NULL_POINTER); /*@req SWS_EcuMf_00034*/

    VersionInfo->vendorID =  ECUM_VENDOR_ID;
    VersionInfo->moduleID =  ECUM_MODULE_ID;
    VersionInfo->sw_major_version =  ECUM_SW_MAJOR_VERSION;
    VersionInfo->sw_minor_version =  ECUM_SW_MINOR_VERSION;
    VersionInfo->sw_patch_version =  ECUM_SW_PATCH_VERSION;

}
#endif
