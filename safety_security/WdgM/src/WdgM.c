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

/*Globaly fulfilled requirements*/
/** @req SWS_WdgM_00011 */ /*Data types used by Watchdog Manager module defined in other modules*/
/** @req SWS_WdgM_91001 */ /*Port intefaces for module*/
/** @req SWS_WdgM_00333 */ /*Port intefaces for module*/
/** @req SWS_WdgM_91004 */ /*Port intefaces for module*/
/** @req SWS_WdgM_00150 */ /*Port intefaces for module*/
/** @req SWS_WdgM_00149 */ /*Port intefaces for module*/
/** @req SWS_WdgM_91002 */ /*Naming of service ports for module*/
/** @req SWS_WdgM_00147 */ /*Naming of service ports for module*/
/** @req SWS_WdgM_91003 */ /*Naming of service ports for module*/

#include "WdgM.h"

#if defined(USE_RTE)
/** @req SWS_WdgM_00334 */ /*Defined in Rte_WdgM_Type.h*/
/** @req SWS_WdgM_00360 */ /*Defined in Rte_WdgM_Type.h*/
/** @req SWS_WdgM_00359 */ /*Defined in Rte_WdgM_Type.h*/
/** @req SWS_WdgM_00357 */ /*Defined in Rte_WdgM_Type.h*/
/** @req SWS_WdgM_00356 */ /*Defined in Rte_WdgM_Type.h*/
/** @req SWS_WdgM_00358 */ /*Defined in Rte_WdgM_Type.h*/
#include "Rte_WdgM.h"
#endif

#include "Os.h"
/** @req SWS_WdgM_00126 */
#if defined(USE_DEM)
#include "Dem.h"
#endif
#include "WdgIf.h"

#if defined(USE_BSWM)
#include "BswM.h"
#endif

#if !(((WDGM_SW_MAJOR_VERSION == 3u) && (WDGM_SW_MINOR_VERSION == 0u)) )
#error WdgM: Expected BSW module version to be 3.0.*
#endif

#if !(((WDGM_AR_RELEASE_MAJOR_VERSION == 4u) && (WDGM_AR_RELEASE_MINOR_VERSION == 3u)) )
#error WdgM: Expected AUTOSAR version to be 4.3.*
#endif



#if (WDGM_IMMEDIATE_RESET == STD_ON)
#include "Mcu.h"
#endif

/* @req SWS_BSW_00045 Development errors should be reported to DET module */
#if (WDGM_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"

#define WDGM_REPORT_DET_ERROR(functionID, error) (void)Det_ReportError(WDGM_MODULE_ID, 0u, functionID, error)
#else
#define WDGM_REPORT_DET_ERROR(functionID, error)
#endif

#if defined(USE_DEM)
#define WDGM_REPORT_DEM_ERROR(eventId) if (eventId != DEM_EVENT_ID_NULL) { Dem_ReportErrorStatus(eventId, DEM_EVENT_STATUS_FAILED); }
#else
#define WDGM_REPORT_DEM_ERROR(eventId)
#define DEM_EVENT_ID_NULL 0
#endif

/*lint -save -e904 MISRA:ARGUMENT_CHECK:argument check:[MISRA 2012 Rule 15.5, advisory]*/

 /* ARGUMENT_CHECK Macros used for checking arguments before performing any functionality*/
/* @req SWS_BSW_00203 API parameter checking enablement. */
/* @req SWS_BSW_00042 Detection of DevErrors should only be performed if configuration parameter for Development errors is set. */
#if (WDGM_DEV_ERROR_DETECT == STD_ON)

/* @req SWS_BSW_00049 API parameter checking */
#define WDGM_CHECK_INIT_NO_RV(functionID)                              \
    {                                                               \
        if (WdgM_instance.isInitiated == (boolean)FALSE)            \
        {                                                           \
            WDGM_REPORT_DET_ERROR(functionID, WDGM_E_NO_INIT);      \
            return;                                                 \
        }                                                           \
    }

/* @req SWS_BSW_00049 API parameter checking */
#define WDGM_CHECK_DEINIT_NO_RV(functionID)                            \
    {                                                               \
        if (WdgM_instance.isInitiated == (boolean)TRUE)             \
        {                                                           \
            WDGM_REPORT_DET_ERROR(functionID, WDGM_E_NO_DEINIT);    \
            return;                                                 \
        }                                                           \
    }

/* @req SWS_BSW_00049 API parameter checking */
#define WDGM_CHECK_INIT_RV(functionID)                          \
    {                                                           \
        if (WdgM_instance.isInitiated == (boolean)FALSE)        \
        {                                                       \
            WDGM_REPORT_DET_ERROR(functionID, WDGM_E_NO_INIT);  \
            return E_NOT_OK;                                    \
        }                                                       \
    }

#define WDGM_CHECK_NULL_POINTER_NO_RV(ptr, functionID)                 \
    {                                                               \
        if (ptr == NULL_PTR)                                        \
        {                                                           \
            WDGM_REPORT_DET_ERROR(functionID, WDGM_E_INV_POINTER);  \
            return;                                                 \
        }                                                           \
    }

#define WDGM_CHECK_NULL_POINTER_RV(ptr, functionID)                 \
    {                                                               \
        if (ptr == NULL_PTR)                                        \
        {                                                           \
            WDGM_REPORT_DET_ERROR(functionID, WDGM_E_INV_POINTER);  \
            return E_NOT_OK;                                        \
        }                                                           \
    }

#define WDGM_CHECK_MODE_RV(mode, functionID, error)                     \
    {                                                                   \
        if (mode >= WdgM_instance.ConfigPtr->ConfigSet.Length_Modes)    \
        {                                                               \
            WDGM_REPORT_DET_ERROR(functionID, error);                   \
            return E_NOT_OK;                                            \
        }                                                               \
    }

#define WDGM_CHECK_SEID_RV(SEId, functionID, error)                                 \
    {                                                                               \
        if (SEId >= WdgM_instance.ConfigPtr->General.Length_SupervisedEntities)      \
        {                                                                           \
            WDGM_REPORT_DET_ERROR(functionID, error);                               \
            return E_NOT_OK;                                                        \
        }                                                                           \
    }

#define WDGM_CHECK_CP_RV(se, CPId, functionID)                                      \
    {                                                                               \
        if (CPId >= se->Length_CheckpointIds)                                        \
        {                                                                           \
            WDGM_REPORT_DET_ERROR(functionID, WDGM_E_CPID);                         \
            return E_NOT_OK;                                                        \
        }                                                                           \
    }

#define WDGM_CHECK_VALID_SEID_POINTER_RV(ptr, functionID, error)                  \
    {                                                               \
        if (ptr == NULL_PTR)                                        \
        {                                                           \
            WDGM_REPORT_DET_ERROR(functionID, error);  \
            return E_NOT_OK;                                        \
        }                                                           \
    }

#else

#define WDGM_CHECK_INIT_NO_RV(functionID)                          \
    {                                                           \
        if (WdgM_instance.isInitiated == (boolean)FALSE)        \
        {                                                       \
            return;                                             \
        }                                                       \
    }

#define WDGM_CHECK_DEINIT_NO_RV(functionID)                        \
    {                                                           \
        if (WdgM_instance.isInitiated == (boolean)TRUE)         \
        {                                                       \
            return;                                             \
        }                                                       \
    }

#define WDGM_CHECK_INIT_RV(functionID)                          \
    {                                                           \
        if (WdgM_instance.isInitiated == (boolean)FALSE)        \
        {                                                       \
            return E_NOT_OK;                                    \
        }                                                       \
    }

#define WDGM_CHECK_NULL_POINTER_NO_RV(ptr, functionID)                 \
    {                                                               \
        if (ptr == NULL_PTR)                                        \
        {                                                           \
            return;                                                 \
        }                                                           \
    }

#define WDGM_CHECK_NULL_POINTER_RV(ptr, functionID)                 \
    {                                                               \
        if (ptr == NULL_PTR)                                        \
        {                                                           \
            return E_NOT_OK;                                        \
        }                                                           \
    }

#define WDGM_CHECK_MODE_RV(mode, functionID, error)                     \
    {                                                                   \
        if (mode >= WdgM_instance.ConfigPtr->ConfigSet.Length_Modes)    \
        {                                                               \
            return E_NOT_OK;                                            \
        }                                                               \
    }

#define WDGM_CHECK_SEID_RV(SEId, functionID, error)                                 \
    {                                                                               \
        if (SEId >= WdgM_instance.ConfigPtr->General.Length_SupervisedEntities)      \
        {                                                                           \
            return E_NOT_OK;                                                        \
        }                                                                           \
    }

#define WDGM_CHECK_CP_RV(se, CPId, functionID)                                  \
    {                                                                               \
        if (CPId >= se->Length_CheckpointIds)                                        \
        {                                                                           \
            return E_NOT_OK;                                                        \
        }                                                                           \
    }

#define WDGM_CHECK_VALID_SEID_POINTER_RV(ptr, functionID, error)                  \
    {                                                               \
        if (ptr == NULL_PTR)                                        \
        {                                                           \
            return E_NOT_OK;                                        \
        }                                                           \
    }

#endif
/*lint -restore */

/** @req SWS_WdgM_00387 */
#define WDGM_START_SEC_VAR_INIT_UNSPECIFIED
#include "WdgM_BswMemMap.h"  /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
WdgM_debuggable_runtimeData    WdgM_instance = {.GlobalState = WDGM_GLOBAL_STATUS_DEACTIVATED, .isInitiated = (boolean)FALSE}; /*lint !e785 OTHER [MISRA 2012 Rule 9.3, required] the GlobalState needs to be initialized according to requirement and isInitiated need to be FALSE, while the other are loaded at WdgM_Init function*/
#define WDGM_STOP_SEC_VAR_INIT_UNSPECIFIED
#include "WdgM_BswMemMap.h" /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

#define WDGM_START_SEC_VAR_NO_INIT_16
#include "WdgM_BswMemMap.h" /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
static uint16 firstExpiredSEID;
#define WDGM_STOP_SEC_VAR_NO_INIT_16
#include "WdgM_BswMemMap.h" /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
#define WDGM_START_SEC_VAR_NO_INIT_16
#include "WdgM_BswMemMap.h" /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/
static uint16 firstExpiredSEIDInverse;
#define WDGM_STOP_SEC_VAR_NO_INIT_16
#include "WdgM_BswMemMap.h" /*lint !e9019 OTHER [MISRA 2012 Rule 20.1, advisory] OTHER AUTOSAR specified way of using MemMap*/

static void WdgM_internal_DeactivateAndResetSE(uint16 SEIndex);
static void WdgM_internal_CheckAlives(void);
static WdgM_Substate WdgM_internal_IsAliveHold(const WdgM_AliveSupervision *aliveCP, WdgM_runtime_AliveSupervision *runtime_aliveCP);
static void WdgM_internal_CalculateGlobalState(void);
static void WdgM_internal_ResetSEConfigsForMode(WdgM_runtime_Mode *runtime_Mode);
static void WdgM_internal_AliveMonitoring(const WdgM_SupervisedEntityConfiguration *seConf, const WdgM_runtime_SupervisedEntityConfig *runtime_seConf, WdgM_CheckpointIdType CPId);
static WdgM_Substate WdgM_internal_DeadlineMonitoring(const WdgM_SupervisedEntityConfiguration *seConf, const WdgM_runtime_SupervisedEntityConfig *runtime_seConf, WdgM_CheckpointIdType CPId);
static WdgM_Substate WdgM_internalIsDeadlineHold(const CounterType counter_id, const WdgM_DeadlineSupervision *deadlineCP, WdgM_runtime_DeadlineSupervision *runtime_deadlineCP);
static WdgM_Substate WdgM_internal_LogicalMonitoring(const WdgM_SupervisedEntity *se, WdgM_runtime_SupervisedEntity *runtime_se, WdgM_CheckpointIdType CPId);
static Std_ReturnType WdgM_internal_deactivateCurrentTriggers(uint16 errID);
static Std_ReturnType WdgM_internal_activateTriggers(const WdgM_Mode *Mode, uint16 errID);
static void WdgM_Internal_ReportLocalModeChange(const WdgM_runtime_SupervisedEntity *runtime_se, const uint16 SEIndex);
static void Wdgm_internal_ReportGlobalModeChange(void);
#if (WDGM_OFF_MODE_ENABLED != STD_ON)
static boolean WdgM_internal_isAtLeastOneWdogEnabled(WdgM_ModeType Mode, uint8 SID, uint8 errID);
#endif

/* @req SWS_BSW_00064 GetVersionInfo shall execute synchonously */
/* @req SWS_BSW_00052 GetVersion info shall only have one parameter */
/* @req SWS_BSW_00164 No restriction on calling context */
#if ( WDGM_VERSION_INFO_API == STD_ON )
void WdgM_GetVersionInfo( Std_VersionInfoType* VersionInfo )
{

    /** @req SWS_WdgM_00256 */
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    /* @req SWS_BSW_00212 NULL pointer check */
    /*lint -e904 MISRA:ARGUMENT_CHECK:argument check:[MISRA 2012 Rule 15.5, advisory]*/
    WDGM_CHECK_NULL_POINTER_NO_RV(VersionInfo, WDGM_SID_GETVERSIONINFO);

    VersionInfo->vendorID =  WDGM_VENDOR_ID;
    VersionInfo->moduleID =  WDGM_MODULE_ID;
    VersionInfo->sw_major_version =  WDGM_SW_MAJOR_VERSION;
    VersionInfo->sw_minor_version =  WDGM_SW_MINOR_VERSION;
    VersionInfo->sw_patch_version =  WDGM_SW_PATCH_VERSION;

}
#endif

/** @req SWS_WdgM_00018 */
/** @req SWS_WdgM_00135 */
void WdgM_Init(const WdgM_ConfigType *ConfigPtr)
{

    const WdgM_Mode *initialMode;
    uint16 i;
    boolean status = (boolean)TRUE;
    Std_ReturnType ret = E_OK;

    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    /* @req SWS_BSW_00212 NULL pointer check */
    WDGM_CHECK_NULL_POINTER_NO_RV(ConfigPtr, WDGM_SID_INIT);

    /** @req SWS_WdgM_00389 */ /*The error code in the requirement seems misspelled and the reuirement is missing a "not"*/
    /** @req SWS_WdgM_00390 */
    WDGM_CHECK_DEINIT_NO_RV(WDGM_SID_INIT);

    WdgM_instance.ConfigPtr = ConfigPtr;

    WdgM_instance.runtimeDataPtr = &WdgM_runtimeData;
    initialMode = &WdgM_instance.ConfigPtr->ConfigSet.Modes[WdgM_instance.ConfigPtr->ConfigSet.initialModeId];

#if (WDGM_OFF_MODE_ENABLED != STD_ON)
    /** @req SWS_WdgM_00030 */
    /*lint -e838 MISRA:OTHER:Previously assigned value to variable 'status' has not been used:[MISRA 2004 Info, advisory] */
    status = WdgM_internal_isAtLeastOneWdogEnabled(initialMode->Id, WDGM_SID_INIT, WDGM_E_DISABLE_NOT_ALLOWED);
#endif

    if ((boolean)TRUE == status) {/*lint !e774 MISRA:CONFIGURATION:Boolean within 'if' always evaluates to True:[MISRA 2012 Rule 14.3, required] */
         /* cycle through all SEs and deactivate them (afterwards active the ones which are configured for the initMode)
         * also set active-flag to false and reset the internal-previous-cps */
        for (i = 0u; i < WdgM_instance.ConfigPtr->General.Length_SupervisedEntities; i++)
        {
            /** @req SWS_WdgM_00269 */
            /** @req SWS_WdgM_00296 */
            WdgM_internal_DeactivateAndResetSE(i);
        }

        /* cycle trough all SEs which are configured for the initial-mode and activate them*/
        for (i = 0u; i < initialMode->Length_SEConfigurations; i++)
        {
            uint16 SEId = initialMode->SEConfigurations[i].SupervisedEntityId;
            WdgM_runtime_SupervisedEntity *runtime_se = &WdgM_instance.runtimeDataPtr->SEs[SEId];

            /** @req SWS_WdgM_00268 */
            runtime_se->LocalState = WDGM_LOCAL_STATUS_OK;
            WdgM_Internal_ReportLocalModeChange(runtime_se, SEId);
        }

        /** @req SWS_WdgM_00298 */
        /* reset all timestamps and counters for mode */
        WdgM_internal_ResetSEConfigsForMode(&WdgM_instance.runtimeDataPtr->Modes[initialMode->Id]);

        /** @req SWS_WdgM_00285 */
        WdgM_instance.GlobalState = WDGM_GLOBAL_STATUS_OK;
        Wdgm_internal_ReportGlobalModeChange();

        /** @req SWS_WdgM_00370 */
        WdgM_instance.isFirstExpiredSet = (boolean)FALSE;
        firstExpiredSEIDInverse = 0;
        firstExpiredSEID = 0;

        WdgM_instance.CurrentMode = initialMode;
        WdgM_instance.ResetInitiated = (boolean)FALSE;

        /* now setmode */
        /** @req SWS_WdgM_00135 */
#if defined(USE_DEM)
        ret = WdgM_internal_activateTriggers(initialMode,WdgM_instance.ConfigPtr->ConfigSet.DemEventIdRefs.SetMode);
#else
        ret = WdgM_internal_activateTriggers(initialMode,DEM_EVENT_ID_NULL);
#endif

        if (E_OK == ret)
        {
            /** @req SWS_WdgM_00179 */
            WdgM_instance.isInitiated = (boolean)TRUE;
        }
    }

}

void WdgM_DeInit( void )
{
    Std_ReturnType ret;
    ret = E_OK;

    /** @req SWS_WdgM_00288 */
    /** @req SWS_WdgM_00388 */
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    WDGM_CHECK_INIT_NO_RV(WDGM_SID_DEINIT);/*lint !e904 MISRA:ARGUMENT_CHECK:argument check:[MISRA 2012 Rule 15.5, advisory]*/

    if (WdgM_instance.GlobalState == WDGM_GLOBAL_STATUS_OK)
    {
        uint8 i ;

        /** @req SWS_WdgM_00286 */
        WdgM_instance.GlobalState = WDGM_GLOBAL_STATUS_DEACTIVATED;
        Wdgm_internal_ReportGlobalModeChange();
        /* set all currently active wdogs (only the ones that are configured by wdgm!!) to off */
        ret = WdgM_internal_deactivateCurrentTriggers(DEM_EVENT_ID_NULL);
        if (E_OK == ret) {
            /** @req SWS_WdgM_00223 */ /* not really a requirement here but spec says that in deactiveated state triggerconditions have to be 0. since deinit gets globalstate in deactiveated we do this here as well.. */
            if (WdgM_instance.ResetInitiated == (boolean)FALSE)
            {
                for(i = 0u; i < WdgM_instance.ConfigPtr->General.Length_Watchdogs; i++)
                {
                    WdgIf_SetTriggerCondition(WdgM_instance.ConfigPtr->General.Watchdogs[i].WatchdogDeviceId, 0u);
                }
            }

            WdgM_instance.isInitiated = (boolean)FALSE;
        }
    }
}

Std_ReturnType WdgM_SetMode( WdgM_ModeType Mode)
{
    uint16 SetModeEventId;
    Std_ReturnType retVal = E_OK;
    boolean status = (boolean)TRUE;

    /** @req SWS_WdgM_00021 */
    /** @req SWS_WdgM_00393 */
    /** @req SWS_WdgM_00392 */
    /** @req SWS_WdgM_00394 */
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    WDGM_CHECK_INIT_RV(WDGM_SID_SETMODE);/*lint !e904 MISRA:ARGUMENT_CHECK:argument check:[MISRA 2012 Rule 15.5, advisory]*/
    /** @req SWS_WdgM_00020 */
    WDGM_CHECK_MODE_RV(Mode, WDGM_SID_SETMODE, WDGM_E_PARAM_MODE);/*lint !e904 MISRA:ARGUMENT_CHECK:argument check:[MISRA 2012 Rule 15.5, advisory]*/

#if (WDGM_OFF_MODE_ENABLED != STD_ON)
    /** @req SWS_WdgM_00031 */
    /*lint -e838 MISRA:OTHER:Previously assigned value to variable 'status' has not been used:[MISRA 2004 Info, advisory] */
    status = WdgM_internal_isAtLeastOneWdogEnabled(Mode, WDGM_SID_SETMODE, WDGM_E_DISABLE_NOT_ALLOWED);
#endif

    if ((boolean)TRUE == status) {/*lint !e774 MISRA:CONFIGURATION:Boolean within 'if' always evaluates to True:[MISRA 2012 Rule 14.3, required] */
        /** @req SWS_WdgM_00316 */
        /** @req SWS_WdgM_00145 */
        if ((WdgM_instance.GlobalState == WDGM_GLOBAL_STATUS_OK) || (WdgM_instance.GlobalState == WDGM_GLOBAL_STATUS_FAILED)) {
            const WdgM_Mode *newMode = &WdgM_instance.ConfigPtr->ConfigSet.Modes[Mode];

            /* we now have to disable all SEs which aren't used in this mode */
            /* since global state is OK or FAILED there should be no mode which has bad local state value => we don't have to check this */

            /* we consider that all SEconfigurations are sorted in the way that the config with the lowest SEId is in first place */
            uint16 SEIndex = 0u;
            uint16 ConfigIndex = 0u;

            /** @req SWS_WdgM_00182 */
            while (ConfigIndex <= newMode->Length_SEConfigurations) { /* we need "<=" for deactivating all SEs when the last SEId from the config was < the greatest SEId */
                /* get next SEId of SEConfig */
                uint16 untilID;

                if (ConfigIndex < newMode->Length_SEConfigurations) {
                    untilID = newMode->SEConfigurations[ConfigIndex].SupervisedEntityId;
                }
                else {
                    /* we need this for deactivating all SEs when the last SEId from the config was < the greatest SEId */
                    untilID = WdgM_instance.ConfigPtr->General.Length_SupervisedEntities;
                }
                /* until there deactivate all SEs */
                for(; SEIndex < untilID; SEIndex++) {
                    /** @req SWS_WdgM_00207 */
                    /** @req SWS_WdgM_00291 */
                    /** @req SWS_WdgM_00315 */
                    WdgM_internal_DeactivateAndResetSE(SEIndex);
                }
                if (ConfigIndex < newMode->Length_SEConfigurations) {
                    /* now we are at the SE we don't want to activate or leave it like it is */
                    /** @req SWS_WdgM_00209 */
                    WdgM_runtime_SupervisedEntity *runtime_se = &WdgM_instance.runtimeDataPtr->SEs[SEIndex];

                    /* since deactivation already reseted everything we just have to activate the se */
                    /* when already active we don't have to do anything in order to retain all values */
                    if (runtime_se->LocalState == WDGM_LOCAL_STATUS_DEACTIVATED) {
                        runtime_se->LocalState = WDGM_LOCAL_STATUS_OK;
                        WdgM_Internal_ReportLocalModeChange(runtime_se, SEIndex);

                    }

                    /* stepover this id since there is nothing more to do */
                    SEIndex++;
                }
                /* get next ConfigId with the next cycle */
                ConfigIndex++;
            }

            /* the modechange was successfully so reset all data from the last mode */
            WdgM_internal_ResetSEConfigsForMode(&WdgM_instance.runtimeDataPtr->Modes[WdgM_instance.CurrentMode->Id]);

            /* deactivate all old triggers and activate all new ones (depending on the new mode) */
    #if defined(USE_DEM)
            SetModeEventId = (uint16)WdgM_instance.ConfigPtr->ConfigSet.DemEventIdRefs.SetMode;
    #else
            SetModeEventId = DEM_EVENT_ID_NULL;
    #endif
            retVal = WdgM_internal_deactivateCurrentTriggers(SetModeEventId);

            if (retVal == E_OK) {
                retVal = WdgM_internal_activateTriggers(newMode, SetModeEventId);
            }

            WdgM_instance.CurrentMode = newMode;
        }
    } else {
        retVal = E_NOT_OK;
    }
    return retVal;
}

Std_ReturnType WdgM_GetMode( WdgM_ModeType *Mode)
{
    /** @req SWS_WdgM_00253 */
    /** @req SWS_WdgM_00395 */
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    WDGM_CHECK_INIT_RV(WDGM_SID_GETMODE);/*lint !e904 MISRA:ARGUMENT_CHECK:argument check:[MISRA 2012 Rule 15.5, advisory]*/
    /* @req SWS_BSW_00212 NULL pointer check */
    /* @req SWS_WdgM_00254 */
    WDGM_CHECK_NULL_POINTER_RV(Mode, WDGM_SID_GETMODE);/*lint !e904 MISRA:ARGUMENT_CHECK:argument check:[MISRA 2012 Rule 15.5, advisory]*/

    /** @req SWS_WdgM_00170 */ /*WdgM_instance.CurrentMode is set last in SetMode*/
    *Mode = WdgM_instance.CurrentMode->Id;

    return E_OK;
}

/** @req SWS_WdgM_00295 */
/** @req SWS_WdgM_00297 */
Std_ReturnType WdgM_CheckpointReached( WdgM_SupervisedEntityIdType SEID, WdgM_CheckpointIdType CheckpointID)
{
    /** @req SWS_WdgM_00279 */
    /** @req SWS_WdgM_00396 */
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    WDGM_CHECK_INIT_RV(WDGM_SID_CHECKPOINTREACHED);/*lint !e904 MISRA:ARGUMENT_CHECK:argument check:[MISRA 2012 Rule 15.5, advisory]*/
    /** @req SWS_WdgM_00278 */
    WDGM_CHECK_SEID_RV(SEID, WDGM_SID_CHECKPOINTREACHED, WDGM_E_PARAM_SEID);/*lint !e904 MISRA:ARGUMENT_CHECK:argument check:[MISRA 2012 Rule 15.5, advisory]*/

    Std_ReturnType retVal = E_NOT_OK;

    /* first get the right SE */
    const WdgM_SupervisedEntity *se = &WdgM_instance.ConfigPtr->General.SupervisedEntities[SEID];
    WdgM_runtime_SupervisedEntity *runtime_se = &WdgM_instance.runtimeDataPtr->SEs[SEID];

    /** @req SWS_WdgM_00284 */
    WDGM_CHECK_CP_RV(se, CheckpointID, WDGM_SID_CHECKPOINTREACHED);/*lint !e904 MISRA:ARGUMENT_CHECK:argument check:[MISRA 2012 Rule 15.5, advisory]*/

    /** @req SWS_WdgM_00208 */
    if (runtime_se->LocalState!= WDGM_LOCAL_STATUS_DEACTIVATED)
    {
        /* and now the right config for the current mode */
        uint16 i;
        const WdgM_SupervisedEntityConfiguration *seConf = NULL_PTR;
        const WdgM_runtime_SupervisedEntityConfig *runtime_seConf = NULL_PTR;

        /** @CODECOV:SEARCHING_FOR_ELEMENT_IN_ARRAY:The end of the for-loop is not reached as we break once the element is found. **/
        __CODE_COVERAGE_IGNORE__
        for(i = 0u; i < WdgM_instance.CurrentMode->Length_SEConfigurations; i++)
        {
            if(WdgM_instance.CurrentMode->SEConfigurations[i].SupervisedEntityId == SEID)
            {
                seConf = &WdgM_instance.CurrentMode->SEConfigurations[i];
                runtime_seConf = &WdgM_instance.runtimeDataPtr->Modes[WdgM_instance.CurrentMode->Id].SE_Configs[i];
                break;
            }
        }

        WDGM_CHECK_VALID_SEID_POINTER_RV(seConf,WDGM_SID_CHECKPOINTREACHED,WDGM_E_PARAM_SEID);/*lint !e904 MISRA:ARGUMENT_CHECK:argument check:[MISRA 2012 Rule 15.5, advisory]*/
        WDGM_CHECK_VALID_SEID_POINTER_RV(runtime_seConf,WDGM_SID_CHECKPOINTREACHED,WDGM_E_PARAM_SEID);/*lint !e904 MISRA:ARGUMENT_CHECK:argument check:[MISRA 2012 Rule 15.5, advisory]*/

        /* value stays correct */
        WdgM_internal_AliveMonitoring(seConf, runtime_seConf, CheckpointID);

        /** @req SWS_WdgM_00322 */
        /* if this algo doesn't find anything value stays correct */
        runtime_se->SubstateDeadline = WdgM_internal_DeadlineMonitoring(seConf, runtime_seConf, CheckpointID);

        if (seConf->CheckInternalLogic == (boolean)TRUE)
        {
            /** @req SWS_WdgM_00323 */
            runtime_se->SubstateLogical = WdgM_internal_LogicalMonitoring(se, runtime_se, CheckpointID);
        }

        retVal = E_OK;
    }
#if (WDGM_DEV_ERROR_DETECT == STD_ON)
    else
    {
        /** @req SWS_WdgM_00319 */
        WDGM_REPORT_DET_ERROR(WDGM_SID_CHECKPOINTREACHED, WDGM_E_SEDEACTIVATED);
    }
#endif

    return retVal;
}

/** @req SWS_WdgM_00171 */
Std_ReturnType WdgM_GetLocalStatus( WdgM_SupervisedEntityIdType SEID, WdgM_LocalStatusType *Status)
{
    /** @req SWS_WdgM_00173 */
    /** @req SWS_WdgM_00397 */
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    WDGM_CHECK_INIT_RV(WDGM_SID_GETLOCALSTATUS);/*lint !e904 MISRA:ARGUMENT_CHECK:argument check:[MISRA 2012 Rule 15.5, advisory]*/
    /** @req SWS_WdgM_00257 */
    /* @req SWS_BSW_00212 NULL pointer check */
    WDGM_CHECK_NULL_POINTER_RV(Status, WDGM_SID_GETLOCALSTATUS);/*lint !e904 MISRA:ARGUMENT_CHECK:argument check:[MISRA 2012 Rule 15.5, advisory]*/
    /** @req SWS_WdgM_00172 */
    WDGM_CHECK_SEID_RV(SEID, WDGM_SID_GETLOCALSTATUS, WDGM_E_PARAM_SEID);/*lint !e904 MISRA:ARGUMENT_CHECK:argument check:[MISRA 2012 Rule 15.5, advisory]*/

    *Status = WdgM_instance.runtimeDataPtr->SEs[SEID].LocalState;

    return E_OK;
}

Std_ReturnType WdgM_GetGlobalStatus( WdgM_GlobalStatusType *Status)
{
    /** @req SWS_WdgM_00176 */
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    WDGM_CHECK_INIT_RV(WDGM_SID_GETGLOBALSTATUS);/*lint !e904 MISRA:ARGUMENT_CHECK:argument check:[MISRA 2012 Rule 15.5, advisory]*/
    /** @req SWS_WdgM_00344 */
    /** @req SWS_WdgM_00258 */
    /* @req SWS_BSW_00212 NULL pointer check */
    WDGM_CHECK_NULL_POINTER_RV(Status, WDGM_SID_GETGLOBALSTATUS);/*lint !e904 MISRA:ARGUMENT_CHECK:argument check:[MISRA 2012 Rule 15.5, advisory]*/

    *Status = WdgM_instance.GlobalState;

    return E_OK;
}

void WdgM_PerformReset( void )
{
    /** @req SWS_WdgM_00270 */
    /** @req SWS_WdgM_00401 */
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    WDGM_CHECK_INIT_NO_RV(WDGM_SID_PERFORMRESET);/*lint !e904 MISRA:ARGUMENT_CHECK:argument check:[MISRA 2012 Rule 15.5, advisory]*/
    if (WdgM_instance.ResetInitiated == (boolean)FALSE)
    {
        /** @req SWS_WdgM_00233 */
        WdgM_instance.ResetInitiated = (boolean)TRUE;
        /** @req SWS_WdgM_00232 */
        uint8 i;

        for(i = 0u; i < WdgM_instance.ConfigPtr->General.Length_Watchdogs; i++)
        {
            WdgIf_SetTriggerCondition(WdgM_instance.ConfigPtr->General.Watchdogs[i].WatchdogDeviceId, 0u);
        }
    }
}

Std_ReturnType WdgM_GetFirstExpiredSEID( WdgM_SupervisedEntityIdType *SEID)
{
    /** @req SWS_WdgM_00348 */
    /* there is no init check because function should be available before init is called */

    Std_ReturnType retVal = E_NOT_OK;

    /** @req SWS_WdgM_00347 */
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    /* @req SWS_BSW_00212 NULL pointer check */
    WDGM_CHECK_NULL_POINTER_RV(SEID, WDGM_SID_GETFIRSTEXPIREDSEID);/*lint !e904 MISRA:ARGUMENT_CHECK:argument check:[MISRA 2012 Rule 15.5, advisory]*/

    /** @req SWS_WdgM_00349 */
    uint16 invertedSEID = (~firstExpiredSEID);

    if (invertedSEID == firstExpiredSEIDInverse) {
        *SEID = firstExpiredSEID;
        retVal = E_OK;
    } else {
        *SEID = 0u;
        retVal = E_NOT_OK;
    }

    return retVal;
}

/** @req SWS_WdgM_00327 */
void WdgM_MainFunction( void )
{
    /** @req SWS_WdgM_00039 */
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    WDGM_CHECK_INIT_NO_RV(WDGM_SID_MAINFUNCTION);/*lint !e904 MISRA:ARGUMENT_CHECK:argument check:[MISRA 2012 Rule 15.5, advisory]*/

    uint8 i;

    /** @req SWS_WdgM_00324 */
    /* now do alive monitoring */
    WdgM_internal_CheckAlives();

    /** @req SWS_WdgM_00325 */
    /** @req SWS_WdgM_00326 */
    /** @req SWS_WdgM_00214 */
    /* now do the other stuff - local states are implicitly calculated during global state calculation */
    WdgM_internal_CalculateGlobalState();

    if (WdgM_instance.GlobalState == WDGM_GLOBAL_STATUS_STOPPED)
    {
#if (WDGM_DEM_ALIVE_SUPERVISION_REPORT == STD_ON)
        /** @req SWS_WdgM_00129 */
        /** @req SWS_WdgM_00375 */
        WDGM_REPORT_DEM_ERROR(WdgM_instance.ConfigPtr->ConfigSet.DemEventIdRefs.Supervision);
#endif
#if (WDGM_IMMEDIATE_RESET == STD_ON)
        /** @req SWS_WdgM_00133 */
        /** @req SWS_WdgM_00134 */
        Mcu_PerformReset();
#endif
    }

    /* set the triggers for all wdgdevices != WDGIF_OFF_MODE */
    /** @req SWS_WdgM_00328 */
    /** @req SWS_WdgM_00223 */
    for(i = 0u; i < WdgM_instance.CurrentMode->Length_Triggers; i++)
    {
        /** @req SWS_WdgM_00119 */
        /** @req SWS_WdgM_00120 */
        /** @req SWS_WdgM_00121 */
        /** @req SWS_WdgM_00122 */
        if (WdgM_instance.CurrentMode->Triggers[i].WatchdogMode != WDGIF_OFF_MODE)
        {
            uint16 triggerValue = (WdgM_instance.GlobalState == WDGM_GLOBAL_STATUS_STOPPED)? 0u : WdgM_instance.CurrentMode->Triggers[i].TriggerConditionValue;
            if (WdgM_instance.ResetInitiated == (boolean)FALSE)
            {
                WdgIf_SetTriggerCondition(WdgM_instance.ConfigPtr->General.Watchdogs[WdgM_instance.CurrentMode->Triggers[i].WatchdogId].WatchdogDeviceId, triggerValue);
            }
        }
    }
}

#if (WDGM_OFF_MODE_ENABLED != STD_ON)
static boolean WdgM_internal_isAtLeastOneWdogEnabled(WdgM_ModeType Mode, uint8 SID, uint8 errID) {
    /* check wheter one Watchdog is != WDGIF_OFF_MODE */
    /* if not => DET-Error and return without doing anything */

    uint8 i;
    boolean WdgActive = (boolean)FALSE;

    const WdgM_Mode *newMode = &WdgM_instance.ConfigPtr->ConfigSet.Modes[Mode];

    for(i = 0u; i < newMode->Length_Triggers; i++)
    {
        if (newMode->Triggers[i].WatchdogMode != WDGIF_OFF_MODE)
        {
            WdgActive = (boolean)TRUE;
            break;
        }
    }

    if ((boolean)FALSE == WdgActive)
    {
        WDGM_REPORT_DET_ERROR(SID, errID);
    }

    return WdgActive;
}
#endif

static Std_ReturnType WdgM_internal_deactivateCurrentTriggers(uint16 errID)
{
    uint8 i;
    Std_ReturnType ret = E_OK;

    /* set all currently used watchdogs to off (only the onces configured for that mode!!) */
    for(i = 0u; (i < WdgM_instance.CurrentMode->Length_Triggers) && (E_OK == ret); i++)
    {
        uint8 DeviceIndex = WdgM_instance.ConfigPtr->General.Watchdogs[WdgM_instance.CurrentMode->Triggers[i].WatchdogId].WatchdogDeviceId;
        (void)DeviceIndex; /* Remove compile warning when using only one watch dog driver */
        Std_ReturnType potError = WdgIf_SetMode(DeviceIndex, WDGIF_OFF_MODE);

        if (potError != E_OK)
        {

            WdgM_instance.GlobalState = WDGM_GLOBAL_STATUS_STOPPED;
            Wdgm_internal_ReportGlobalModeChange();

            WDGM_REPORT_DEM_ERROR(errID);
            ret = E_NOT_OK;
        }
    }

    return ret;
}

static Std_ReturnType WdgM_internal_activateTriggers(const WdgM_Mode *Mode, uint16 errID)
{
    uint8 i;
    Std_ReturnType ret = E_OK;

    /* turn on all wdogs configured for the new mode */
    for(i = 0u; (i < Mode->Length_Triggers) && (E_OK == ret); i++)
    {
        /** @req SWS_WdgM_00186 */
        uint8 DeviceIndex = WdgM_instance.ConfigPtr->General.Watchdogs[Mode->Triggers[i].WatchdogId].WatchdogDeviceId;
        (void)DeviceIndex; /* Remove compile warning when using only one watch dog driver */
        Std_ReturnType potError = WdgIf_SetMode(DeviceIndex, Mode->Triggers[i].WatchdogMode);

        if ((potError == E_OK) && (Mode->Triggers[i].WatchdogMode != WDGIF_OFF_MODE))
        {
            if (WdgM_instance.ResetInitiated == (boolean)FALSE)
            {
                WdgIf_SetTriggerCondition(DeviceIndex, Mode->Triggers[i].TriggerConditionValue);
            }
        }

        if (potError != E_OK)
        {
            /** @req SWS_WdgM_00139 */
            WdgM_instance.GlobalState = WDGM_GLOBAL_STATUS_STOPPED;
            Wdgm_internal_ReportGlobalModeChange();

            /** @req SWS_WdgM_00142 */
            /** @req SWS_WdgM_00376 */
            WDGM_REPORT_DEM_ERROR(errID);
            ret = E_NOT_OK;
        }
    }

    return ret;
}

static void WdgM_internal_CheckAlives(void)
{
    /* cycle through all seConfigs and check the alive states and update the internal substate of the corresponding se */
    uint16 i;

    for(i = 0u; i < WdgM_instance.CurrentMode->Length_SEConfigurations; i++)
    {
        const WdgM_SupervisedEntityConfiguration *seConf = &WdgM_instance.CurrentMode->SEConfigurations[i];
        WdgM_runtime_SupervisedEntity *runtime_se = &WdgM_instance.runtimeDataPtr->SEs[seConf->SupervisedEntityId];

        WdgM_Substate result = WDGM_SUBSTATE_CORRECT;
        boolean writeResult = (boolean)FALSE;

        uint16 a;
        /** @req SWS_WdgM_00083 */
        for(a = 0u; a < seConf->Length_AliveSupervisions; a++)
        {
            const WdgM_AliveSupervision *aliveCP = &seConf->AliveSupervisions[a];

            WdgM_runtime_AliveSupervision *runtime_aliveCP = &WdgM_instance.runtimeDataPtr->Modes[WdgM_instance.CurrentMode->Id].SE_Configs[i].AliveSupervisions[a];

            /* first increment the supervisioncyclecounter */
            runtime_aliveCP->SupervisionCycleCounter++;

            /** @req SWS_WdgM_00074 */
            WdgM_Substate tempRes = WdgM_internal_IsAliveHold(aliveCP, runtime_aliveCP);

            if ((boolean)TRUE == runtime_aliveCP->wasEvaluated) {
                writeResult = TRUE;
            }

            /** @req SWS_WdgM_00115 */
            if (tempRes == WDGM_SUBSTATE_INCORRECT)
            {
                /* when once there was an incorrect result don't update again to correct */
                result = tempRes;
            }
        }

        /* when one aliveCP was evaluated then the result is written - if no aliveCP was evaluated the state stays the same */
        if ((boolean)TRUE == writeResult) {
            runtime_se->SubstateAlive = result;
        }

    }
}

static WdgM_Substate WdgM_internal_SpecAliveAlgo(const WdgM_AliveSupervision *aliveCP, WdgM_runtime_AliveSupervision *runtimeAliveCP)
{
    WdgM_Substate retVal = WDGM_SUBSTATE_INCORRECT;

    sint16 temp = (sint16)runtimeAliveCP->AliveCounter - (sint16)aliveCP->ExpectedAliveIndications;

    if ((temp <= (sint16)aliveCP->MaxMargin) && (temp >= (0 - (sint16)aliveCP->MinMargin)))
    {
        retVal = WDGM_SUBSTATE_CORRECT;
    }

    runtimeAliveCP->AliveCounter = 0u;              /* reset counter for next check */
    runtimeAliveCP->SupervisionCycleCounter = 0u;   /* reset counter in order to count cycles from beginning again */

    return retVal;
}

static WdgM_Substate WdgM_internal_IsAliveHold(const WdgM_AliveSupervision *aliveCP, WdgM_runtime_AliveSupervision *runtime_aliveCP)
{
    WdgM_Substate retVal = WDGM_SUBSTATE_CORRECT;
    /* check all constraints */

    /* checking depends on supervisionreferencecycle */
    /* so if src == 1 we have to check each cycle */
    /* when src > 1 we have to check when src is reached */

    runtime_aliveCP->wasEvaluated = (boolean)FALSE;

    /** @req SWS_WdgM_00098 */
    if (aliveCP->SupervisionReferenceCycle == 1u)
    {
        /* each cycle */
        retVal = WdgM_internal_SpecAliveAlgo(aliveCP, runtime_aliveCP);
        runtime_aliveCP->wasEvaluated = (boolean)TRUE;
    }
    else
    {
        /* multiple cycle */

        /* check if multiple is already reached => therefore we can use the SupervisionCycleCounter */
        if (runtime_aliveCP->SupervisionCycleCounter == aliveCP->SupervisionReferenceCycle)
        {
            retVal = WdgM_internal_SpecAliveAlgo(aliveCP, runtime_aliveCP);
            runtime_aliveCP->wasEvaluated = (boolean)TRUE;
        }
    }

    return retVal;
}

static void WdgM_internal_CalculateLocalState_FromFailedNoError(WdgM_runtime_SupervisedEntity *runtime_se, const WdgM_SupervisedEntityConfiguration *seConf)
{
    /** @req SWS_WdgM_00300 */
    if (runtime_se->FailedAliveCyclesCounter > 1u)
    {
        /* decrement and stay in failed */
        runtime_se->FailedAliveCyclesCounter--;
    }
    else
    /** @req SWS_WdgM_00205 */
    {
        runtime_se->FailedAliveCyclesCounter = 0u;
        runtime_se->LocalState = WDGM_LOCAL_STATUS_OK;
        WdgM_Internal_ReportLocalModeChange(runtime_se, seConf->SupervisedEntityId);
    }
}

static void WdgM_internal_CalculateLocalState_FromOKOneError(WdgM_runtime_SupervisedEntity *runtime_se, const WdgM_SupervisedEntityConfiguration *seConf)
{
    /** @req SWS_WdgM_00203 */
    /*runtime_se->FailedAliveCyclesCounter will alwys be equal or less than <= seConf->FailedAliveSupervisionReferenceCycleTol when arriving at this function, it is therefore not needed to be checked*/
    if (runtime_se->SubstateAlive == WDGM_SUBSTATE_INCORRECT)
    {

        runtime_se->LocalState = WDGM_LOCAL_STATUS_FAILED;
        WdgM_Internal_ReportLocalModeChange(runtime_se, seConf->SupervisedEntityId);
        /* The ReportModeChange shall not be performed here, it may be expired directly depending on configuration parameters*/
        /* Increment the first time, it may be expired directly */
        runtime_se->FailedAliveCyclesCounter++;
    }

    /** @req SWS_WdgM_00202 */
    if ((runtime_se->FailedAliveCyclesCounter > seConf->FailedAliveSupervisionReferenceCycleTol) || (runtime_se->SubstateAlive == WDGM_SUBSTATE_CORRECT)) /* one of the others have to be incorrect (not alive ) */
    {
        runtime_se->LocalState = WDGM_LOCAL_STATUS_EXPIRED;
        WdgM_Internal_ReportLocalModeChange(runtime_se, seConf->SupervisedEntityId);
    }

}

static void WdgM_internal_CalculateLocalState_FromFailedOneError(WdgM_runtime_SupervisedEntity *runtime_se, const WdgM_SupervisedEntityConfiguration *seConf, const WdgM_runtime_SupervisedEntityConfig *runtime_seConf)
{
    /** @req SWS_WdgM_00204 */
    /*runtime_se->FailedAliveCyclesCounter will alwys be equal or less than <= seConf->FailedAliveSupervisionReferenceCycleTol when arriving at this function, it is therefore not needed to be checked*/
    if (runtime_se->SubstateAlive == WDGM_SUBSTATE_INCORRECT)
    {
        boolean wasEval = (boolean)FALSE;
        uint16 i;

        for(i = 0u; i < runtime_seConf->Length_AliveSupervisions; i++)
        {
            if (runtime_seConf->AliveSupervisions[i].wasEvaluated == (boolean)TRUE)
            {
                wasEval = (boolean)TRUE;
                break;
            }
        }

        /*No mode change has occured, so runtime_se->LocalState is still equal to WDGM_LOCAL_STATUS_FAILED and no reporting is needed (unless it goes to expired)*/

        if ((boolean)TRUE == wasEval)
        {
            /* only increment when aliveSV was evaluated and is still FAILED */
            runtime_se->FailedAliveCyclesCounter++;
        }
    }

    /** @req SWS_WdgM_00206 */
    if (runtime_se->FailedAliveCyclesCounter > seConf->FailedAliveSupervisionReferenceCycleTol)
    {
        runtime_se->LocalState = WDGM_LOCAL_STATUS_EXPIRED;
        WdgM_Internal_ReportLocalModeChange(runtime_se, seConf->SupervisedEntityId);
    }
}

#define WDGM_ALLSUBSTATECORRECT 3U
#define WDGM_ONESUBSTATEINCORRECT 2U

static void WdgM_internal_CalculateLocalState(WdgM_runtime_SupervisedEntity *runtime_se, const WdgM_SupervisedEntityConfiguration *seConf, const WdgM_runtime_SupervisedEntityConfig *runtime_seConf)
{
    /* calculate local state for the given SE, the value of each substate can be either 0 (WDGM_SUBSTATE_INCORRECT) or 1 (WDGM_SUBSTATE_CORRECT) so the value of nrOfCorrectSubstates can be between 0 and 3 */

    uint8 nrOfCorrectSubstates = runtime_se->SubstateAlive + runtime_se->SubstateDeadline + runtime_se->SubstateLogical;

    switch (nrOfCorrectSubstates) {
        case WDGM_ALLSUBSTATECORRECT:
            /* everything is fine - nothing needs to be done */
            /** @req SWS_WdgM_00201 */

            if (runtime_se->LocalState == WDGM_LOCAL_STATUS_FAILED)
            {
                WdgM_internal_CalculateLocalState_FromFailedNoError(runtime_se, seConf);
            }

            break;
        case WDGM_ONESUBSTATEINCORRECT:
            /* maybe it's still working */

            if (runtime_se->LocalState == WDGM_LOCAL_STATUS_OK)
            {
                WdgM_internal_CalculateLocalState_FromOKOneError(runtime_se, seConf);
                break;
            }

            if (runtime_se->LocalState == WDGM_LOCAL_STATUS_FAILED)
            {
                WdgM_internal_CalculateLocalState_FromFailedOneError(runtime_se, seConf, runtime_seConf);
                break;
            }

            break;
        default:
            runtime_se->LocalState = WDGM_LOCAL_STATUS_EXPIRED;
            WdgM_Internal_ReportLocalModeChange(runtime_se, seConf->SupervisedEntityId);
            break;
    }
}

/* @req SWS_WdgM_00197 */
static void WdgM_Internal_ReportLocalModeChange(const WdgM_runtime_SupervisedEntity *runtime_se, const uint16 SEIndex)
{

    #if defined(USE_RTE)


    switch (runtime_se->LocalState) {
        case WDGM_LOCAL_STATUS_OK:
            (void)modeFunctionSwitchPointer[SEIndex](RTE_MODE_WdgMMode_SUPERVISION_OK);
            break;
        case WDGM_LOCAL_STATUS_FAILED:
            (void)modeFunctionSwitchPointer[SEIndex](RTE_MODE_WdgMMode_SUPERVISION_FAILED);
            break;
        case WDGM_LOCAL_STATUS_EXPIRED:
            (void)modeFunctionSwitchPointer[SEIndex](RTE_MODE_WdgMMode_SUPERVISION_EXPIRED);
            break;
        case WDGM_LOCAL_STATUS_DEACTIVATED:
            (void)modeFunctionSwitchPointer[SEIndex](RTE_MODE_WdgMMode_SUPERVISION_DEACTIVATED);
            break;
        /** @CODECOV:DEFAULT_CASE:Default statement is required for defensive programming.**/
		__CODE_COVERAGE_IGNORE__
        default:
            break;
    }
    #endif

}



/* @req SWS_WdgM_00198 */
static void Wdgm_internal_ReportGlobalModeChange()
{

    #if defined(USE_RTE)
    switch (WdgM_instance.GlobalState) {
        case WDGM_GLOBAL_STATUS_OK:
            (void)Rte_Switch_globalMode_currentMode(RTE_MODE_WdgMMode_SUPERVISION_OK);
            break;
        case WDGM_GLOBAL_STATUS_FAILED:
            (void)Rte_Switch_globalMode_currentMode(RTE_MODE_WdgMMode_SUPERVISION_FAILED);
            break;
        case WDGM_GLOBAL_STATUS_EXPIRED:
            (void)Rte_Switch_globalMode_currentMode(RTE_MODE_WdgMMode_SUPERVISION_EXPIRED);
            break;
        case WDGM_GLOBAL_STATUS_STOPPED:
            (void)Rte_Switch_globalMode_currentMode(RTE_MODE_WdgMMode_SUPERVISION_STOPPED);
            break;
        case WDGM_GLOBAL_STATUS_DEACTIVATED:
            (void)Rte_Switch_globalMode_currentMode(RTE_MODE_WdgMMode_SUPERVISION_DEACTIVATED);
            break;
        /** @CODECOV:DEFAULT_CASE:Default statement is required for defensive programming.**/
		__CODE_COVERAGE_IGNORE__
        default:
            break;
    }
    #endif

}


static void WdgM_internal_updateFromGlobalOK(uint16 expired, uint16 failed)
{
    /** @req SWS_WdgM_00076 */
    if (failed > 0u) /*How many local states which are in failed state*/
    {
        WdgM_instance.GlobalState = WDGM_GLOBAL_STATUS_FAILED;
        /* no return since expired isn't evaluated yet */
    }

    if (expired > 0u) { /*How many local states which are in expired state*/
        if (WdgM_instance.runtimeDataPtr->Modes[WdgM_instance.CurrentMode->Id].ExpiredSupervisionCycleCounter < WdgM_instance.CurrentMode->ExpiredSupervisionCycleTol) {
            /** @req SWS_WdgM_00215 */
            WdgM_instance.GlobalState = WDGM_GLOBAL_STATUS_EXPIRED;
        } else {
            /** @req SWS_WdgM_00216 */
            WdgM_instance.GlobalState = WDGM_GLOBAL_STATUS_STOPPED;
        }

    } else {
        /** @req SWS_WdgM_00078 */ /* there is nothing to do when status is ok or deactivated */
    }

}

static void WdgM_internal_updateFromGlobalFAILED(uint16 expired, uint16 failed)
{

    /** @req SWS_WdgM_00218 */
    if ((failed == 0u) && (expired == 0u)) { /*How many local states which are in failed state and in expired state*/
        WdgM_instance.GlobalState = WDGM_GLOBAL_STATUS_OK;
    } else if (expired > 0u){

        if (WdgM_instance.runtimeDataPtr->Modes[WdgM_instance.CurrentMode->Id].ExpiredSupervisionCycleCounter < WdgM_instance.CurrentMode->ExpiredSupervisionCycleTol) {
            /** @req SWS_WdgM_00077 */
            WdgM_instance.GlobalState = WDGM_GLOBAL_STATUS_EXPIRED;
        } else {
            /** @req SWS_WdgM_00117 */
            WdgM_instance.GlobalState = WDGM_GLOBAL_STATUS_STOPPED;
        }
    } else {
        /** @req SWS_WdgM_00217 */
        /* remain in failed when nothing applies (failed > 0 && expired == 0).. so just do nothing */
    }
}

static void WdgM_internal_updateFromGlobalEXPIRED(uint16 expired)
{

    /** @req SWS_WdgM_00219 */
    if (expired > 0u) { /*How many local states which are in expired state*/
        if (WdgM_instance.runtimeDataPtr->Modes[WdgM_instance.CurrentMode->Id].ExpiredSupervisionCycleCounter < WdgM_instance.CurrentMode->ExpiredSupervisionCycleTol) {
            /* stay in expired and increment the counter */
            WdgM_instance.runtimeDataPtr->Modes[WdgM_instance.CurrentMode->Id].ExpiredSupervisionCycleCounter++;
        } else {
            /** @req SWS_WdgM_00220 */
            WdgM_instance.GlobalState = WDGM_GLOBAL_STATUS_STOPPED;
        }
    } else {
        /* Do nothing */
    }
}

static void WdgM_internal_CalculateGlobalState()
{
    /* go trough all active SEs and calc the global state depending on their local states */
    uint16 expiredCounter = 0u;
    uint16 failedCounter = 0u;
    uint16 i;

    WdgM_GlobalStatusType initalState = WdgM_instance.GlobalState;

    for (i = 0u; i < WdgM_instance.CurrentMode->Length_SEConfigurations; i++)
    {
        const WdgM_SupervisedEntityConfiguration *seConf = &WdgM_instance.CurrentMode->SEConfigurations[i];
        const WdgM_runtime_SupervisedEntityConfig *runtime_seConf = &WdgM_instance.runtimeDataPtr->Modes[WdgM_instance.CurrentMode->Id].SE_Configs[i];

        WdgM_runtime_SupervisedEntity *runtime_se = &WdgM_instance.runtimeDataPtr->SEs[seConf->SupervisedEntityId];

        WdgM_internal_CalculateLocalState(runtime_se, seConf, runtime_seConf);

        if (runtime_se->LocalState == WDGM_LOCAL_STATUS_EXPIRED)
        {
            /** @req SWS_WdgM_00351 */
            if (WdgM_instance.isFirstExpiredSet == (boolean)FALSE)
            {
                firstExpiredSEID = i;
                firstExpiredSEIDInverse = ~i;
                WdgM_instance.isFirstExpiredSet = (boolean)TRUE;
            }

            expiredCounter++;
        }

        if (runtime_se->LocalState == WDGM_LOCAL_STATUS_FAILED)
        {
            failedCounter++;
        }
    }

    switch(WdgM_instance.GlobalState) {
        case WDGM_GLOBAL_STATUS_OK:
            WdgM_internal_updateFromGlobalOK(expiredCounter, failedCounter);
            break;
        case WDGM_GLOBAL_STATUS_FAILED:
            WdgM_internal_updateFromGlobalFAILED(expiredCounter, failedCounter);
            break;
        case WDGM_GLOBAL_STATUS_EXPIRED:
            WdgM_internal_updateFromGlobalEXPIRED(expiredCounter);
            break;
        case WDGM_GLOBAL_STATUS_STOPPED:
            /** @req SWS_WdgM_00221 */
            /* when stopped then stay in stopped */
            break;
        /** @CODECOV:DEFAULT_CASE:Default statement is required for defensive programming.**/
		__CODE_COVERAGE_IGNORE__
        default:
            /* Invalid state because it shouldn't be possible with WDGM_GLOBAL_STATUS_DEACTIVATED if WdgM is initialized */
            break;
    }

    if (initalState != WdgM_instance.GlobalState) {
        /* @req SWS_WdgM_00199 */
        Wdgm_internal_ReportGlobalModeChange();
    }
}

static void WdgM_internal_ResetSEConfigsForMode(WdgM_runtime_Mode *runtime_Mode)
{
    uint16 i;

    runtime_Mode->ExpiredSupervisionCycleCounter = 0u;

    for (i = 0u; i < runtime_Mode->Length_SEConfigs; i++)
    {
        /* reset timestamps, alivecounters, previous cp and external active-flag */
        uint16 a ;
        for (a = 0u; a < runtime_Mode->SE_Configs[i].Length_AliveSupervisions; a++)
        {
            runtime_Mode->SE_Configs[i].AliveSupervisions[a].AliveCounter = 0u;
            runtime_Mode->SE_Configs[i].AliveSupervisions[a].SupervisionCycleCounter = 0u;
        }

        for (a = 0u; a < runtime_Mode->SE_Configs[i].Length_DeadlineSupervisions; a++)
        {
            /** @req SWS_WdgM_00298 */
            runtime_Mode->SE_Configs[i].DeadlineSupervisions[a].LastTickValue = 0u;
        }
    }
}

static void WdgM_internal_AliveMonitoring(const WdgM_SupervisedEntityConfiguration *seConf, const WdgM_runtime_SupervisedEntityConfig *runtime_seConf, WdgM_CheckpointIdType CPId)
{
    uint16 i;
    WdgM_runtime_AliveSupervision *runtime_aliveCP = NULL_PTR;

    /* search for the checkpoint */
    for(i = 0u; i < seConf->Length_AliveSupervisions; i++)
    {
        if (seConf->AliveSupervisions[i].CheckpointId == CPId)
        {
            runtime_aliveCP =  &runtime_seConf->AliveSupervisions[i];
            break;
        }
    }

    if (runtime_aliveCP != NULL_PTR)
    {
        /** @req SWS_WdgM_00321 */
        runtime_aliveCP->AliveCounter++;
    }
}

/** @req SWS_WdgM_00299 */
static WdgM_Substate WdgM_internal_DeadlineMonitoring(const WdgM_SupervisedEntityConfiguration *seConf, const WdgM_runtime_SupervisedEntityConfig *runtime_seConf, WdgM_CheckpointIdType CPId)
{
    WdgM_Substate result = WDGM_SUBSTATE_CORRECT;
    uint16 i;

    /* search for checkpoints */
    for(i = 0u; i < seConf->Length_DeadlineSupervisions; i++)
    {
        /* if it's a start dlcp then set timestamp */
        if (seConf->DeadlineSupervisions[i].CheckpointIdStart == CPId)
        {
            TickType elapsedTicks = 0u;
            TickRefType elapsedTicksPointer;
            elapsedTicksPointer = &elapsedTicks;
            /** @req SWS_WdgM_00228 */
            TickRefType LastTickPointer = (TickRefType)&(runtime_seConf->DeadlineSupervisions[i].LastTickValue);/*lint !e929 MISRA:CONFIGURATION:cast from one pointer to other:[MISRA 2012 Rule 11.3, required] */
            (void)GetElapsedValue(seConf->OSCounter, LastTickPointer, elapsedTicksPointer); /*Choosing which counter shall be used is not supported*/
        }

        /* if it's a finsih dlcp then do evaluation */
        if ((seConf->DeadlineSupervisions[i].CheckpointIdFinish == CPId) && (runtime_seConf->DeadlineSupervisions[i].LastTickValue != 0u))
        {
            /** @req SWS_WdgM_00229 */
            /* measure the time and get calculate result */
            if (WdgM_internalIsDeadlineHold(seConf->OSCounter, &seConf->DeadlineSupervisions[i], &runtime_seConf->DeadlineSupervisions[i]) == WDGM_SUBSTATE_INCORRECT)
            {
                result = WDGM_SUBSTATE_INCORRECT;
                break;
            }

            /** @req SWS_WdgM_00354 */
            runtime_seConf->DeadlineSupervisions[i].LastTickValue = 0u;
        }
    }

    return result;
}

#define WDGM_OSTICKSPERSECOND (1000000000UL / OSTICKDURATION)

static WdgM_Substate WdgM_internalIsDeadlineHold(const CounterType counter_id, const WdgM_DeadlineSupervision *deadlineCP, WdgM_runtime_DeadlineSupervision *runtime_deadlineCP)
{
    WdgM_Substate ret;

    TickType elapsedTicks = 0u;
    TickRefType elapsedTicksPointer;
    elapsedTicksPointer = &elapsedTicks;
    /** @req SWS_WdgM_00373 */
    /** @req SWS_WdgM_00374 */ /*The computation has the same effect as 00374 but does not use that procedure*/
    TickRefType LastTickPointer = (TickRefType)&(runtime_deadlineCP->LastTickValue);/*lint !e929 MISRA:CONFIGURATION:cast from one pointer to other:[MISRA 2012 Rule 11.3, required] */
    (void)GetElapsedValue(counter_id, LastTickPointer, elapsedTicksPointer); /*Choosing which counter shall be used is not supported*/
    ret = WDGM_SUBSTATE_INCORRECT;
    /** @req SWS_WdgM_00294 */
    if ((elapsedTicks >= (deadlineCP->DeadlineMin * WDGM_OSTICKSPERSECOND)) && (elapsedTicks <= (deadlineCP->DeadlineMax * WDGM_OSTICKSPERSECOND)))
    {
        ret = WDGM_SUBSTATE_CORRECT;
    }

    return ret;
}

static WdgM_Substate WdgM_internal_LogicalMonitoring_GraphActive(const WdgM_SupervisedEntity *se, WdgM_runtime_SupervisedEntity *runtime_se, WdgM_CheckpointIdType CPId)
{
    WdgM_Substate retVal = WDGM_SUBSTATE_INCORRECT;

    /* check whether the transition from the previous checkpoint to the current is allowed */
    uint16 i;

    for(i = 0u; i < se->Length_Transitions; i++)
    {
        if(se->Transitions[i].CheckpointIdSource == runtime_se->PreviousCheckpointId_internalLogic)
        {
            if(se->Transitions[i].CheckpointIdDestination == CPId)
            {
                retVal = WDGM_SUBSTATE_CORRECT;

                /** @req SWS_WdgM_00246 */
                runtime_se->PreviousCheckpointId_internalLogic = CPId;

                break;
            }
        }
    }

    if (retVal == WDGM_SUBSTATE_CORRECT)
    {
        /* now check if this cp is a finishcp */
        for(i = 0u; i < se->Length_FinalCheckpointIds; i++)
        {
            if (se->FinalCheckpointIds[i] == CPId)
            {
                /** @req SWS_WdgM_00331 */
                runtime_se->IsInternalGraphActive = (boolean)FALSE;
                break;
            }
        }
    }

    return retVal;
}

static WdgM_Substate WdgM_internal_LogicalMonitoring_GraphInactive(const WdgM_SupervisedEntity *se, WdgM_runtime_SupervisedEntity *runtime_se, WdgM_CheckpointIdType CPId)
{
    WdgM_Substate retVal = WDGM_SUBSTATE_INCORRECT;

    /* check whether the reported cp is a startcp */
    uint16 i;

    for(i = 0u; i < se->Length_StartCheckpointIds; i++)
    {
        if (se->StartCheckpointIds[i] == CPId)
        {
            retVal = WDGM_SUBSTATE_CORRECT;

            /** @req SWS_WdgM_00332 */
            /** @req SWS_WdgM_00273 */
	    /** @req SWS_WdgM_00329 */
            runtime_se->IsInternalGraphActive = (boolean)TRUE;

            /** @req SWS_WdgM_00246 */
            runtime_se->PreviousCheckpointId_internalLogic = CPId;

            break;
        }
    }

    return retVal;
}

static WdgM_Substate WdgM_internal_LogicalMonitoring(const WdgM_SupervisedEntity *se, WdgM_runtime_SupervisedEntity *runtime_se, WdgM_CheckpointIdType CPId)
{
    WdgM_Substate retVal = WDGM_SUBSTATE_INCORRECT;

    /** @req SWS_WdgM_00274 */
    if (runtime_se->IsInternalGraphActive == (boolean)TRUE)
    {
        retVal = WdgM_internal_LogicalMonitoring_GraphActive(se, runtime_se, CPId);
    }
    else
    {
        retVal = WdgM_internal_LogicalMonitoring_GraphInactive(se, runtime_se, CPId);
    }

    return retVal;
}

static void WdgM_internal_DeactivateAndResetSE(uint16 SEIndex)
{
    WdgM_runtime_SupervisedEntity *runtime_se = &WdgM_instance.runtimeDataPtr->SEs[SEIndex];

    runtime_se->LocalState = WDGM_LOCAL_STATUS_DEACTIVATED;
    WdgM_Internal_ReportLocalModeChange(runtime_se, SEIndex);


    runtime_se->IsInternalGraphActive = (boolean)FALSE;
    runtime_se->PreviousCheckpointId_internalLogic = 0u; /* be aware that 0 is a valid value => but flag is inactive so this dosn't matter */
    runtime_se->FailedAliveCyclesCounter = 0u;

    runtime_se->SubstateAlive = WDGM_SUBSTATE_CORRECT;
    runtime_se->SubstateDeadline = WDGM_SUBSTATE_CORRECT;
    runtime_se->SubstateLogical = WDGM_SUBSTATE_CORRECT;

}
