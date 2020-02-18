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

/*
 *  General requirements
 */
/* !req SWS_Fim_00029 *//* IMPROVEMENT: Check file structure */
/* @req SWS_Fim_00044 *//* Dependency to other modules */
/* @req SWS_Fim_00081 *//* Imported types */
/* !req SWS_Fim_00079 *//* Interfaces not used */

#include "FiM.h"
#include <string.h>

/*lint -emacro(904, VALIDATE_RV,VALIDATE_NO_RV) MISRA:OTHER::[MISRA 2012 Rule 14.5, advisory] */

#if (FIM_DEV_ERROR_DETECT == STD_ON)
/* @req SWS_Fim_00080 */
#include "Det.h"
#define VALIDATE_RV(_exp,_api,_err,_rv ) \
    if( !(_exp) ) { \
      (void)Det_ReportError(FIM_MODULE_ID, 0, _api, _err); \
      return _rv; \
    }

#define VALIDATE_NO_RV(_exp,_api,_err ) \
  if( !(_exp) ) { \
      (void)Det_ReportError(FIM_MODULE_ID, 0, _api, _err); \
      return; \
    }
#else
#define VALIDATE_RV(_exp,_api,_err,_rv )
#define VALIDATE_NO_RV(_exp,_api,_err )
#endif /* FIM_DEV_ERROR_DETECT */

#define IS_VALID_SUMMARY_EVENT(_x) ((_x) < FIM_NOF_SUMMARY_EVENTS)
#define IS_VALID_EVENT_ID(_x) ((_x) < FIM_NOF_DEM_EVENTS)

/* Local types */
typedef enum {
    FIM_UNINIT = 0u,
    FIM_INIT,
    FIM_DEMINIT
}FiMStateType;

typedef struct {
    Dem_MonitorStatusType MonitorStatus;
#if (FIM_EVENT_UPDATE_TRIGGERED_BY_DEM == STD_ON)
    boolean RefreshNeeded;
#endif
    boolean StatusValid;
}FiMMonitorStatusType;

/* Local variables */
#define FiM_START_SEC_VAR_INIT_UNSPECIFIED
#include "FiM_MemMap.h" /*lint !e9019 MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 20.1, advisory] */
static FiMStateType FiM_InitState = FIM_UNINIT; /* @req SWS_Fim_00059 */
#define FiM_STOP_SEC_VAR_INIT_UNSPECIFIED
#include "FiM_MemMap.h" /*lint !e9019 MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 20.1, advisory] */

#define FiM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "FiM_MemMap.h" /*lint !e9019 MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 20.1, advisory] */
static const FiM_ConfigType *FiM_CfgPtr = NULL;
#define FiM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "FiM_MemMap.h" /*lint !e9019 MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 20.1, advisory] */

#if (FIM_AVAILABILITY_SUPPORT == STD_ON)
#define FiM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "FiM_MemMap.h" /*lint !e9019 MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 20.1, advisory] */
static boolean FiMFunctionAvailable[FIM_NUM_FIDS];
#define FiM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "FiM_MemMap.h" /*lint !e9019 MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 20.1, advisory] */
#endif

#define FiM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "FiM_MemMap.h" /*lint !e9019 MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 20.1, advisory] */
static boolean FiMFunctionPermissionStatus[FIM_NUM_FIDS];
#define FiM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "FiM_MemMap.h" /*lint !e9019 MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 20.1, advisory] */

#define FiM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "FiM_MemMap.h" /*lint !e9019 MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 20.1, advisory] */
static boolean FiMInhibitConfigStatus[FIM_MAX_FIM_INHIBIT_CONFIGURATIONS];
#define FiM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "FiM_MemMap.h" /*lint !e9019 MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 20.1, advisory] */

#define FiM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "FiM_MemMap.h" /*lint !e9019 MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 20.1, advisory] */
static FiMMonitorStatusType FiMMonitorStatus[FIM_NOF_DEM_EVENTS];
#define FiM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "FiM_MemMap.h" /*lint !e9019 MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 20.1, advisory] */

#if (FIM_EVENT_UPDATE_TRIGGERED_BY_DEM == STD_ON)
#define FiM_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "FiM_MemMap.h" /*lint !e9019 MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 20.1, advisory] */
static boolean FiMRecalculateInhibitConfig[FIM_NOF_INH_CFGS];
#define FiM_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "FiM_MemMap.h" /*lint !e9019 MISRA:STANDARDIZED_INTERFACE::[MISRA 2012 Rule 20.1, advisory] */
#endif

/* Local functions */

/**
 * Compares monitor status to inhibition mask
 * @param monitorStatus
 * @param inhibitionMask
 * @return TRUE: status and mask match, FALSE: status and mask does NOT match
 */
static boolean compareStatusToMask(Dem_MonitorStatusType monitorStatus, uint8 inhibitionMask)
{
    boolean match = FALSE;

    switch(inhibitionMask) {
        case FIM_LAST_FAILED:
            match = (0u != (monitorStatus & DEM_MONITOR_STATUS_TF));
            break;
        case FIM_NOT_TESTED:
            match = (0u != (monitorStatus & DEM_MONITOR_STATUS_TNCTOC));
            break;
        case FIM_TESTED:
            match = (0u == (monitorStatus & DEM_MONITOR_STATUS_TNCTOC));
            break;
        case FIM_TESTED_AND_FAILED:
            match = (DEM_MONITOR_STATUS_TF == (monitorStatus & (DEM_MONITOR_STATUS_TF | DEM_MONITOR_STATUS_TNCTOC)));
            break;
        default:
            break;
    }
    return match;
}

/**
 * Updates all function permissions based on calculated status of inhibit configurations
 */
static void UpdateFunctionPermissions(void)
{
    const Fim_FIDCfgType *fidCfg;
    uint16 inhCfgIdx = 0;

    for( uint16 i = 0; i < FIM_NUM_FIDS; i++ ) {
        fidCfg = &FiM_CfgPtr->FIDConfig[i];
        /* @req SWS_Fim_00015 */
        FiMFunctionPermissionStatus[fidCfg->FID] = TRUE;
        inhCfgIdx = 0;
        while( (inhCfgIdx < FIM_NOF_INH_CFGS) && (FIM_NO_INH_CFG != fidCfg->InhCfgIndexList[inhCfgIdx]) ) {
            if( TRUE == FiMInhibitConfigStatus[fidCfg->InhCfgIndexList[inhCfgIdx]] ) {
                FiMFunctionPermissionStatus[fidCfg->FID] = FALSE;
            }
            inhCfgIdx++;
        }
    }
}

/**
 * Re-calculates the status of inhibit configuration
 * @param inhCfg
 * @return TRUE: Inhibit config inhibits the function, FALSE: Inhibit config does NOT inhibit the function
 */
static boolean RecalculateInhibitConfig(const FiM_InhibitionConfigurationType *inhCfg)
{
    boolean inhibited = FALSE;
#if (FIM_USE_SUMMARY_EVENT == STD_ON)
    const FiM_SummaryEventCfgType *summaryEvent;
    /* Re-calculate the summary events */
    /* @req SWS_Fim_00064 */
    for( uint16 summaryEvtIndex = 0u; (summaryEvtIndex < FIM_MAX_SUM_EVENTS_PER_FID_INHIBITION_CONFIGURATION) && (FALSE == inhibited); summaryEvtIndex++ ) {
        if( IS_VALID_SUMMARY_EVENT(inhCfg->InhSumIndexList[summaryEvtIndex]) ) {
            summaryEvent = &FiM_CfgPtr->SummeryEventCfg[inhCfg->InhSumIndexList[summaryEvtIndex]];
            for( uint16 evtIndex = 0; evtIndex < FIM_MAX_INPUT_EVENTS_PER_SUMMARY_EVENTS; evtIndex++ ) {
                if( (FIM_NO_DEM_EVENT_ID != summaryEvent->EventIndexList[evtIndex]) &&
                    (TRUE == FiMMonitorStatus[summaryEvent->EventIndexList[evtIndex]].StatusValid) &&
                    (TRUE == compareStatusToMask(FiMMonitorStatus[summaryEvent->EventIndexList[evtIndex]].MonitorStatus, *inhCfg->InhibitionMask)) ) {
                    inhibited = TRUE;
                }
            }
        }
    }
#endif
    /* Re-calculate the events */
    for( uint16 evtIndex = 0u; (evtIndex < FIM_MAX_EVENTS_PER_FID_INHIBIT_CONFIGURATION) && (FALSE == inhibited); evtIndex++ ) {
        if( IS_VALID_EVENT_ID(inhCfg->EventIndexList[evtIndex]) &&
                (TRUE == FiMMonitorStatus[inhCfg->EventIndexList[evtIndex]].StatusValid) ) {
            /* Compare to mask */
            /* @req SWS_Fim_00004 */
            if( TRUE == compareStatusToMask(FiMMonitorStatus[inhCfg->EventIndexList[evtIndex]].MonitorStatus, *inhCfg->InhibitionMask) ) {
                inhibited = TRUE;
            }
        }
    }
    return inhibited;
}

/**
 * Re-calculates all inhibit conditions
 */
static void RecalculateAllInhibitConditions(void)
{
    /* Refresh monitor status if needed */
    for( uint16 i = 0; i < FIM_NOF_DEM_EVENTS; i++ ) {
        /* @req SWS_Fim_00097 */
        /* @req SWS_Fim_00067 */
        /* @req SWS_Fim_00072 */
        FiMMonitorStatus[i].StatusValid = (E_OK == Dem_GetMonitorStatus(FiM_CfgPtr->EventInhCfg[i].EventId, &FiMMonitorStatus[i].MonitorStatus));
#if (FIM_EVENT_UPDATE_TRIGGERED_BY_DEM == STD_ON)
#if defined(CFG_FIM_REFRESH_INVALID_EVENTS)
        FiMMonitorStatus[i].RefreshNeeded = (FALSE == FiMMonitorStatus[i].StatusValid);
#else
        FiMMonitorStatus[i].RefreshNeeded = FALSE;
#endif
#endif
    }
    /* Re-calculate inhibit configuration */
    for(uint16 inhCfgIdx = 0u; inhCfgIdx < FIM_MAX_FIM_INHIBIT_CONFIGURATIONS; inhCfgIdx++) {
        FiMInhibitConfigStatus[inhCfgIdx] = RecalculateInhibitConfig(&FiM_CfgPtr->InhibitConfig[inhCfgIdx]);
    }
    /* Now set the permission per function */
    UpdateFunctionPermissions();
}

/* Exported functions */

/**
 * This service initializes the FIM.
 * @param FiMConfigPtr
 */
/* @req SWS_Fim_00077 */
void FiM_Init(const FiM_ConfigType* FiMConfigPtr)
{
    /* @req SWS_Fim_00045 */
    VALIDATE_NO_RV((NULL != FiMConfigPtr), FIM_INIT_ID, FIM_E_PARAM_POINTER);

    VALIDATE_NO_RV((FIM_UNINIT == FiM_InitState), FIM_INIT_ID, FIM_E_REINIT)

    if( FIM_UNINIT == FiM_InitState ) {
        FiM_CfgPtr = FiMConfigPtr;
        memset(FiMFunctionPermissionStatus, TRUE, FIM_NUM_FIDS);
#if (FIM_AVAILABILITY_SUPPORT == STD_ON)
        memset(FiMFunctionAvailable, TRUE, FIM_NUM_FIDS);
#endif
#if (FIM_EVENT_UPDATE_TRIGGERED_BY_DEM == STD_ON)
        memset(FiMMonitorStatus, 0u, sizeof(FiMMonitorStatus));
#endif
        FiM_InitState = FIM_INIT;
    }
}


/**
 * This service reports the permission state to the functionality.
 * @param FID
 * @param Permission
 * @return
 */
/* @req SWS_Fim_00011 */
Std_ReturnType FiM_GetFunctionPermission(FiM_FunctionIdType FID, boolean* Permission)
{
    /* @req SWS_Fim_00020 */

    /* @req SWS_Fim_00056 */
    if( NULL != Permission ) {
        *Permission = FALSE;
    }
    VALIDATE_RV((FIM_DEMINIT == FiM_InitState), FIM_GETFUNCTIONPERMISSION_ID, FIM_E_UNINIT, E_NOT_OK);
    /* @req SWS_Fim_00055 */
    VALIDATE_RV((FID < FIM_NUM_FIDS), FIM_GETFUNCTIONPERMISSION_ID, FIM_E_FID_OUT_OF_RANGE, E_NOT_OK);

    VALIDATE_RV((NULL != Permission), FIM_GETFUNCTIONPERMISSION_ID, FIM_E_PARAM_POINTER, E_NOT_OK);

    /* @req SWS_Fim_00003 */
    /* @req SWS_Fim_00025 */
#if (FIM_AVAILABILITY_SUPPORT == STD_ON)
    /* @req SWS_Fim_00105 */
    *Permission = (TRUE == FiMFunctionPermissionStatus[FID]) && (TRUE == FiMFunctionAvailable[FID]);
#else
    *Permission = FiMFunctionPermissionStatus[FID];
#endif
    return E_OK;
}

/**
 * This service sets the availability of a function. The function is only available if FiMAvailabilitySupport is configured as True
 * @param FID
 * @param Availability
 * @return
 */
/* @req SWS_Fim_00106 */
#if (FIM_AVAILABILITY_SUPPORT == STD_ON)
Std_ReturnType FiM_SetFunctionAvailable(FiM_FunctionIdType FID, boolean Availability)
{
    VALIDATE_RV((FIM_DEMINIT == FiM_InitState), FIM_SETFUNCTIONAVAILABLE_ID, FIM_E_UNINIT, E_NOT_OK);
    VALIDATE_RV((FID < FIM_NUM_FIDS), FIM_SETFUNCTIONAVAILABLE_ID, FIM_E_FID_OUT_OF_RANGE, E_NOT_OK);

    /* @req SWS_Fim_00003 */
    FiMFunctionAvailable[FID] = Availability;

    return E_OK;
}
#endif /* FIM_AVAILABILITY_SUPPORT */

/**
 * This service is provided to be called by the Dem in order to inform the Fim about monitor status changes.
 * @param EventId
 */
/* @req SWS_Fim_00021 */
void FiM_DemTriggerOnMonitorStatus(Dem_EventIdType EventId)
{
    /* !req SWS_Fim_00057 */
    /* @req SWS_Fim_00058 */
    VALIDATE_NO_RV((FIM_DEMINIT == FiM_InitState), FIM_DEMTRIGGERONMONITORSTATUS_ID, FIM_E_UNINIT);

    /* @req SWS_Fim_00010 */
    /* @req SWS_Fim_00043 */
#if (FIM_EVENT_UPDATE_TRIGGERED_BY_DEM == STD_ON)
    boolean done = FALSE;
    uint16 inhCfgIdx = 0;
    const FiM_EventInhCfgType *evtInhCfg;
    /* Find the config */
    for( uint16 i = 0; (i < FiM_CfgPtr->EventInhCfg_Size) && (FALSE == done); i++ ) {
        evtInhCfg = &FiM_CfgPtr->EventInhCfg[i];
        if( EventId == evtInhCfg->EventId ) {
            while( (inhCfgIdx < FIM_NOF_INH_CFGS) && (FIM_NO_INH_CFG != evtInhCfg->AffectedInhCfgIndexList[inhCfgIdx]) ) {
                FiMRecalculateInhibitConfig[evtInhCfg->AffectedInhCfgIndexList[inhCfgIdx]] = TRUE;
                inhCfgIdx++;
            }
            FiMMonitorStatus[i].RefreshNeeded = TRUE;
            done = TRUE;
        }
    }
#else
    (void)EventId;
#endif
}
/**
 * This service re-initializes the FIM.
 */
/* @req SWS_Fim_00006 */
void FiM_DemInit(void)
{
    VALIDATE_NO_RV((FIM_DEMINIT != FiM_InitState), FIM_DEMINIT_ID, FIM_E_REINIT);
    VALIDATE_NO_RV((FIM_INIT == FiM_InitState), FIM_DEMINIT_ID, FIM_E_UNINIT);
    /* @req SWS_Fim_00069 */
    /* @req SWS_Fim_00082 */
    if( FIM_INIT == FiM_InitState ) {
        /* @req SWS_Fim_00018 */
        RecalculateAllInhibitConditions();
        FiM_InitState = FIM_DEMINIT;
    }
}

/**
 *
 */
/* @req SWS_Fim_00060 */
void FiM_MainFunction(void)
{
    /* @req SWS_Fim_00043 */

    /* @req SWS_Fim_00065 */
    /* @req SWS_Fim_00022 */
    /* @req SWS_Fim_00012 */

    VALIDATE_NO_RV((FIM_DEMINIT == FiM_InitState), FIM_MAINFUNCTION_ID, FIM_E_UNINIT);

#if (FIM_EVENT_UPDATE_TRIGGERED_BY_DEM == STD_OFF)
    /* @req SWS_Fim_00070 */
    /* Now set the permission per function */
    RecalculateAllInhibitConditions();
#else
    /* Refresh monitor status if needed */
    for( uint16 i = 0; i < FIM_NOF_DEM_EVENTS; i++ ) {
        if( TRUE == FiMMonitorStatus[i].RefreshNeeded  ) {
            /* @req SWS_Fim_00097 */
            /* @req SWS_Fim_00067 */
            /* @req SWS_Fim_00072 */
            FiMMonitorStatus[i].StatusValid = (E_OK == Dem_GetMonitorStatus(FiM_CfgPtr->EventInhCfg[i].EventId, &FiMMonitorStatus[i].MonitorStatus));
        }
#if defined(CFG_FIM_REFRESH_INVALID_EVENTS)
        FiMMonitorStatus[i].RefreshNeeded = (FALSE == FiMMonitorStatus[i].StatusValid);
#else
        FiMMonitorStatus[i].RefreshNeeded = FALSE;
#endif
    }
    /* Re-calculate according to what has been reported by Dem */
    for(uint16 inhCfgIdx = 0u; inhCfgIdx < FIM_MAX_FIM_INHIBIT_CONFIGURATIONS; inhCfgIdx++) {
        if( TRUE == FiMRecalculateInhibitConfig[inhCfgIdx] ) {
            FiMInhibitConfigStatus[inhCfgIdx] = RecalculateInhibitConfig(&FiM_CfgPtr->InhibitConfig[inhCfgIdx]);
            FiMRecalculateInhibitConfig[inhCfgIdx] = FALSE;
        }
    }
    /* Now set the permission per function */
    UpdateFunctionPermissions();

#endif /* FIM_EVENT_UPDATE_TRIGGERED_BY_DEM */
}

#if (FIM_VERSION_INFO_API == STD_ON)

/**
 * This service returns the version information of this module.
 * @param versioninfo
 */
void FiM_GetVersionInfo(Std_VersionInfoType* versioninfo)
{
	VALIDATE_NO_RV((NULL != versioninfo), FIM_GETVERSIONINFO_ID, FIM_E_PARAM_POINTER);
	versioninfo->moduleID = FIM_MODULE_ID;
	versioninfo->sw_major_version = FIM_SW_MAJOR_VERSION;
	versioninfo->sw_minor_version = FIM_SW_MINOR_VERSION;
	versioninfo->sw_patch_version = FIM_SW_PATCH_VERSION;
	versioninfo->vendorID = 60;
}
#endif /* FIM_VERSION_INFO_API */
/**
 * Shuts down FiM
 */
void FiM_Shutdown(void)
{
    FiM_InitState = FIM_UNINIT;
}
