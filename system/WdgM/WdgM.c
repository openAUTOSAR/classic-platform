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

#include "WdgM.h"

/** @req WDGM011*/
#include "Os.h"
/** @req WDGM126 */
#if defined(USE_DEM)
#include "Dem.h"
#endif
#include "WdgIf.h"
#include "BswM.h"

#if !(((WDGM_SW_MAJOR_VERSION == 1) && (WDGM_SW_MINOR_VERSION == 0)) )
#error WdgM: Expected BSW module version to be 1.0.*
#endif

/* @req 4.0.3/WDGM013 */
#if !(((WDGM_AR_RELEASE_MAJOR_VERSION == 4) && (WDGM_AR_RELEASE_MINOR_VERSION == 0)) )
#error WdgM: Expected AUTOSAR version to be 4.0.*
#endif



#if (WDGM_IMMEDIATE_RESET == STD_ON)
#include "Mcu.h"
#endif

#if (WDGM_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"

#define WDGM_REPORT_DET_ERROR(functionID, error) Det_ReportError(MODULE_ID_WDGM, 0, functionID, error)
#else
#define WDGM_REPORT_DET_ERROR(functionID, error)
#endif

#if defined(USE_DEM)
#define WDGM_REPORT_DEM_ERROR(functionID, error) if (error != DEM_EVENT_ID_NULL) { Dem_ReportErrorStatus(functionID, error); }
#else
#define WDGM_REPORT_DEM_ERROR(functionID, error)
#endif


#define WDGM_CHECK_INIT_NR(functionID) 							\
	{															\
		if (WdgM_instance.isInitiated == FALSE)					\
		{														\
			WDGM_REPORT_DET_ERROR(functionID, WDGM_E_NO_INIT);	\
			return;												\
		}														\
	}

#define WDGM_CHECK_INIT_RV(functionID) 							\
	{															\
		if (WdgM_instance.isInitiated == FALSE)					\
		{														\
			WDGM_REPORT_DET_ERROR(functionID, WDGM_E_NO_INIT);	\
			return E_NOT_OK;									\
		}														\
	}

#define WDGM_CHECK_NULL_POINTER_NR(ptr, functionID) 				\
	{																\
		if (ptr == NULL_PTR)										\
		{															\
			WDGM_REPORT_DET_ERROR(functionID, WDGM_E_INV_POINTER);	\
			return;													\
		}															\
	}

#define WDGM_CHECK_NULL_POINTER_RV(ptr, functionID) 				\
	{																\
		if (ptr == NULL_PTR)										\
		{															\
			WDGM_REPORT_DET_ERROR(functionID, WDGM_E_INV_POINTER);	\
			return E_NOT_OK;										\
		}															\
	}

#define WDGM_CHECK_MODE_RV(mode, functionID, error) 					\
	{																	\
		if (mode >= WdgM_instance.ConfigPtr->ConfigSet.Length_Modes)	\
		{																\
			WDGM_REPORT_DET_ERROR(functionID, error);					\
			return E_NOT_OK;											\
		}																\
	}

#define WDGM_CHECK_SEID_RV(SEId, functionID, error) 								\
	{																				\
		if (SEId > WdgM_instance.ConfigPtr->General.Length_SupervisedEntities)		\
		{																			\
			WDGM_REPORT_DET_ERROR(functionID, error);								\
			return E_NOT_OK;														\
		}																			\
	}

#define WDGM_CHECK_CP_RV(se, CPId, functionID) 									\
	{																				\
		if (CPId > se->Length_CheckpointIds)										\
		{																			\
			WDGM_REPORT_DET_ERROR(functionID, WDGM_E_CPID);							\
			return E_NOT_OK;														\
		}																			\
	}

WdgM_debuggable_internalData	WdgM_instance;
extern WdgM_internal_RuntimeData WdgM_runtimeData; /* Declared in config */

static void WdgM_internal_DeactivateAndResetSE(uint16 SEIndex);
static void WdgM_internal_CheckAlives();
static WdgM_Substate WdgM_internal_IsAliveHold(const WdgM_AliveSupervision *aliveCP, WdgM_internal_AliveSupervision *runtime_aliveCP);
static void WdgM_internal_CalculateGlobalState();
static void WdgM_internal_ResetSEConfigsForMode(WdgM_internal_Mode *runtime_Mode);
static void WdgM_internal_AliveMonitoring(const WdgM_SupervisedEntityConfiguration *seConf, WdgM_internal_SupervisedEntityConfig *runtime_seConf, WdgM_CheckpointIdType CPId);
static WdgM_Substate WdgM_internal_DeadlineMonitoring(WdgM_internal_SupervisedEntity *runtime_se, const WdgM_SupervisedEntityConfiguration *seConf, WdgM_internal_SupervisedEntityConfig *runtime_seConf, WdgM_CheckpointIdType CPId);
static WdgM_Substate WdgM_internalIsDeadlineHold(const WdgM_DeadlineSupervision *deadlineCP, WdgM_internal_DeadlineSupervision *runtime_deadlineCP);
static WdgM_Substate WdgM_internal_LogicalMonitoring(const WdgM_SupervisedEntity *se, WdgM_internal_SupervisedEntity *runtime_se, const WdgM_SupervisedEntityConfiguration *seConf, WdgM_CheckpointIdType CPId);
static Std_ReturnType WdgM_internal_deactivateCurrentTriggers(uint8 SID, uint8 errID);
static Std_ReturnType WdgM_internal_activateTriggers(const WdgM_Mode *Mode, uint8 SID, uint8 errID);

#if (WDGM_OFF_MODE_ENABLED != STD_OFF)
boolean WdgM_internal_isAtLeastOneWdogEnabled(WdgM_ModeType Mode, uint8 SID, uint8 errID)
{
	/* check wheter one Watchdog is != WDGIF_OFF_MODE */
	/* if not => DET-Error and return without doing anything */

	uint8 i = 0;
	boolean WdgActive = FALSE;
	const WdgM_Mode *newMode = &WdgM_instance.ConfigPtr->ConfigSet.Modes[Mode];

	for(i = 0; i < newMode->Length_Triggers; i++)
	{
		if (newMode->Triggers[i].WatchdogMode != WDGIF_OFF_MODE)
		{
			WdgActive = TRUE;
			break;
		}
	}

	if (WdgActive == FALSE)
	{
#if (WDGM_DEV_ERROR_DETECT == STD_ON)
		WDGM_REPORT_DET_ERROR(SID, errID);
#endif
	}

	return WdgActive;
}
#endif



/** @req WDGM018 */
/** @req WDGM135 */
void WdgM_Init(const WdgM_ConfigType *ConfigPtr)
{
	/** @req WDGM255 */
	WDGM_CHECK_NULL_POINTER_NR(ConfigPtr, WDGM_SID_INIT);

	uint16 i = 0;
	const WdgM_Mode *initialMode;

	if (WdgM_instance.isInitiated == TRUE)
	{
		return;
	}

	WdgM_instance.ConfigPtr = ConfigPtr;
	WdgM_instance.internalRuntimeDataPtr = &WdgM_runtimeData;
	initialMode = &WdgM_instance.ConfigPtr->ConfigSet.Modes[WdgM_instance.ConfigPtr->ConfigSet.initialModeId];


#if (WDGM_OFF_MODE_ENABLED != STD_OFF)
	/** @req WDGM030 */
	if (WdgM_internal_isAtLeastOneWdogEnabled(initialMode->Id, WDGM_SID_INIT, WDGM_E_DISABLE_NOT_ALLOWED) == FALSE)
	{
		return;
	}
#endif

	/* cycle through all SEs and deactivate them (afterwards active the ones which are configured for the initMode)
	 * also set active-flag to false and reset the internal-previous-cps */
	for (i = 0; i < WdgM_instance.ConfigPtr->General.Length_SupervisedEntities; i++)
	{
		/** @req WDGM269 */
		/** @req WDGM296 */
		WdgM_internal_DeactivateAndResetSE(i);
	}

	/* cycle trough all SEs which are configured for the initial-mode and activate them*/
	for (i = 0; i < initialMode->Length_SEConfigurations; i++)
	{
		uint16 SEId = initialMode->SEConfigurations[i].SupervisedEntityId;
		WdgM_internal_SupervisedEntity *runtime_se = &WdgM_instance.internalRuntimeDataPtr->SEs[SEId];

		/** @req WDGM268 */
		runtime_se->LocalState = WDGM_LOCAL_STATUS_OK;
	}

	/** @req WDGM298 */
	/* reset all timestamps and so on... */
	WdgM_internal_ResetSEConfigsForMode(&WdgM_instance.internalRuntimeDataPtr->Modes[initialMode->Id]);

	/** @req WDGM285 */
	WdgM_instance.GlobalState = WDGM_GLOBAL_STATUS_OK;

	WdgM_instance.isFirstExpiredSet = FALSE;
	WdgM_instance.firstExpiredSEID = 0;

	WdgM_instance.CurrentMode = initialMode;
	WdgM_instance.ResetInitiated = FALSE;
	WdgM_instance.CurrentTime = 0;

	/* now setmode */
	/** @req WDGM135 */
	if (WdgM_internal_activateTriggers(initialMode,
									   WDGM_SID_INIT,
#if defined(USE_DEM)
									   WdgM_instance.ConfigPtr->ConfigSet.DemEventIdRefs.SetMode
#else
									   0
#endif
									   ) == E_NOT_OK)
	{
		return;
	}

	/** @req WDGM179 */
	WdgM_instance.isInitiated = TRUE;
}

void WdgM_DeInit( void )
{
	/** @req WDGM288 */
	WDGM_CHECK_INIT_NR(WDGM_SID_DEINIT);

	if (WdgM_instance.GlobalState == WDGM_GLOBAL_STATUS_OK)
	{
		uint8 i = 0;

		/** @req WDGM286 */
		WdgM_instance.GlobalState = WDGM_GLOBAL_STATUS_DEACTIVATED;

		/* set all currently active wdogs (only the ones that are configured by wdgm!!) to off */
		if (WdgM_internal_deactivateCurrentTriggers(WDGM_SID_DEINIT, 0) == E_NOT_OK)
		{
			return;
		}

		/** @req WDGM223 */ /* not really a requirement here but spec says that in deactiveated state triggerconditions have to be 0. since deinit gets globalstate in deactiveated we do this here as well.. */
		for(i = 0; i < WdgM_instance.ConfigPtr->General.Length_Watchdogs; i++)
		{
			WdgIf_SetTriggerCondition(WdgM_instance.ConfigPtr->General.Watchdogs[i].WatchdogDeviceId, 0);
		}

		WdgM_instance.isInitiated = FALSE;
	}
}

#if (WDGM_DEFENSIVE_BEHAVIOR == STD_ON)
/* check whether the caller of this function is allowed for calling */
boolean WdgM_internal_isAllowedForCalling(uint16 CallerID)
{
	boolean callerAllowed = FALSE;
	uint8 i = 0;
	for (i = 0; i < WdgM_instance.ConfigPtr->General.CallerIds.Length_allowedCallerIds; i++)
	{
		if(WdgM_instance.ConfigPtr->General.CallerIds.allowedCallerIds[i] == CallerID)
		{
			callerAllowed = TRUE;
			break;
		}
	}

	if (callerAllowed == FALSE)
	{
		WDGM_REPORT_DEM_ERROR(WDGM_SID_SETMODE, WdgM_instance.ConfigPtr->ConfigSet.DemEventIdRefs.ImproperCaller);

	}
	return callerAllowed;
}
#endif

static Std_ReturnType WdgM_internal_deactivateCurrentTriggers(uint8 SID, uint8 errID)
{
	uint8 i = 0;

	/* set all currently used watchdogs to off (only the onces configured for that mode!!) */
	for(i = 0; i < WdgM_instance.CurrentMode->Length_Triggers; i++)
	{

	    uint8 DeviceIndex = WdgM_instance.ConfigPtr->General.Watchdogs[WdgM_instance.CurrentMode->Triggers[i].WatchdogId].WatchdogDeviceId;
	    (void)DeviceIndex; /* Remove compile warning when using only one watch dog driver */
	    Std_ReturnType potError = WdgIf_SetMode(DeviceIndex, WDGIF_OFF_MODE);

		if (potError != E_OK)
		{
			WdgM_instance.GlobalState = WDGM_GLOBAL_STATUS_STOPPED;

			WDGM_REPORT_DEM_ERROR(SID, errID);

			return E_NOT_OK;
		}
	}

	return E_OK;
}

static Std_ReturnType WdgM_internal_activateTriggers(const WdgM_Mode *Mode, uint8 SID, uint8 errID)
{
	uint8 i = 0;

	/* turn on all wdogs configured for the new mode */
	for(i = 0; i < Mode->Length_Triggers; i++)
	{
		/** @req WDGM186 */
		/** @req WDGM139 */
		uint8 DeviceIndex = WdgM_instance.ConfigPtr->General.Watchdogs[Mode->Triggers[i].WatchdogId].WatchdogDeviceId;
		Std_ReturnType potError = WdgIf_SetMode(DeviceIndex, Mode->Triggers[i].WatchdogMode);

		if (potError == E_OK && Mode->Triggers[i].WatchdogMode != WDGIF_OFF_MODE)
		{
			WdgIf_SetTriggerCondition(DeviceIndex, Mode->Triggers[i].TriggerConditionValue);
		}

		if (potError != E_OK)
		{
			WdgM_instance.GlobalState = WDGM_GLOBAL_STATUS_STOPPED;

			/** @req WDGM142 */
			WDGM_REPORT_DEM_ERROR(SID, errID);

			return E_NOT_OK;
		}
	}

	return E_OK;
}

/** @req WDGM317 */
Std_ReturnType WdgM_SetMode( WdgM_ModeType Mode, uint16 CallerID)
{
	Std_ReturnType retVal = E_OK;

	/** @req WDGM021 */
	WDGM_CHECK_INIT_RV(WDGM_SID_SETMODE);
	/** @req WDGM020 */
	WDGM_CHECK_MODE_RV(Mode, WDGM_SID_SETMODE, WDGM_E_PARAM_MODE);


#if (WDGM_OFF_MODE_ENABLED != STD_OFF)
	/** @req WDGM031 */
	if (WdgM_internal_isAtLeastOneWdogEnabled(Mode, WDGM_SID_SETMODE, WDGM_E_DISABLE_NOT_ALLOWED) == FALSE)
	{
		return E_NOT_OK;
	}
#endif

#if (WDGM_DEFENSIVE_BEHAVIOR == STD_ON)
	/** @req WDGM245 */
	if(WdgM_internal_isAllowedForCalling(CallerID) == FALSE)
	{
		return E_NOT_OK;
	}
#else
	(void)CallerID; /* To avoid compiler warning */
#endif

	/** @req WDGM316 */
	/** @req WDGM145 */
	if (WdgM_instance.GlobalState == WDGM_GLOBAL_STATUS_OK || WdgM_instance.GlobalState == WDGM_GLOBAL_STATUS_FAILED)
	{
		const WdgM_Mode *newMode = &WdgM_instance.ConfigPtr->ConfigSet.Modes[Mode];

		/* we now have to disable all SEs which aren't used in this mode */
		/* since global state is OK or FAILED there should be no mode which has bad local state value => we don't have to check this */

		/* we consider that all SEconfigurations are sorted in the way that the config with the lowest SEId is in first place */
		uint16 SEIndex = 0;
		uint16 ConfigIndex = 0;

		/** @req WDGM182 */
		while (ConfigIndex <= newMode->Length_SEConfigurations) /* we need "<=" for deactivating all SEs when the last SEId from the config was < the greatest SEId */
		{
			/* get next SEId of SEConfig */
			uint16 untilID;

			if (ConfigIndex < newMode->Length_SEConfigurations)
			{
				untilID = newMode->SEConfigurations[ConfigIndex].SupervisedEntityId;
			}
			else
			{
				/* we need this for deactivating all SEs when the last SEId from the config was < the greatest SEId */
				untilID = WdgM_instance.ConfigPtr->General.Length_SupervisedEntities;
			}

			/* until there deactivate all SEs */
			for(; SEIndex < untilID; SEIndex++)
			{
				/** @req WDGM207 */
				/** @req WDGM291 */
				/** @req WDGM315 */
				WdgM_internal_DeactivateAndResetSE(SEIndex);
			}

			if (ConfigIndex < newMode->Length_SEConfigurations)
			{
				/* now we are at the SE we don't want to activate or leave it like it is */
				/** @req WDGM209 */
				WdgM_internal_SupervisedEntity *runtime_se = &WdgM_instance.internalRuntimeDataPtr->SEs[SEIndex];

				/* since deactivation already reseted everything we just have to activate the se */
				/* when already active we don't have to do anything in order to retain all values */
				if (runtime_se->LocalState == WDGM_LOCAL_STATUS_DEACTIVATED)
				{
					runtime_se->LocalState = WDGM_LOCAL_STATUS_OK;
				}

				/* stepover this id since there is nothing more to do */
				SEIndex++;
			}

			/* get next ConfigId with the next cycle */
			ConfigIndex++;
		}

		/* the modechange was successfully so reset all data from the last mode */
		WdgM_internal_ResetSEConfigsForMode(&WdgM_instance.internalRuntimeDataPtr->Modes[WdgM_instance.CurrentMode->Id]);

		/* deactivate all old triggers and activate all new ones (depending on the new mode) */
		retVal |= WdgM_internal_deactivateCurrentTriggers(WDGM_SID_SETMODE,
#if defined(USE_DEM)
		        WdgM_instance.ConfigPtr->ConfigSet.DemEventIdRefs.SetMode
#else
		        0
#endif
		        );

		if (retVal == E_OK)
		{
			retVal |= WdgM_internal_activateTriggers(newMode, WDGM_SID_SETMODE,
#if defined(USE_DEM)
			        WdgM_instance.ConfigPtr->ConfigSet.DemEventIdRefs.SetMode
#else
			        0
#endif
			        );
		}

		WdgM_instance.CurrentMode = newMode;
	}

	return retVal;
}

Std_ReturnType WdgM_GetMode( WdgM_ModeType *Mode)
{
	/** @req WDGM253 */
	WDGM_CHECK_INIT_RV(WDGM_SID_GETMODE);
	/** @req WDGM254 */
	WDGM_CHECK_NULL_POINTER_RV(Mode, WDGM_SID_GETMODE);

	/** @req WDGM170 */
	*Mode = WdgM_instance.CurrentMode->Id;

	return E_OK;
}

/** @req WDGM295 */
/** @req WDGM297 */
Std_ReturnType WdgM_CheckpointReached( WdgM_SupervisedEntityIdType SEID, WdgM_CheckpointIdType CheckpointID)
{
	/** @req WDGM279 */
	WDGM_CHECK_INIT_RV(WDGM_SID_CHECKPOINTREACHED);
	/** @req WDGM278 */
	WDGM_CHECK_SEID_RV(SEID, WDGM_SID_CHECKPOINTREACHED, WDGM_E_PARAM_SEID);

	Std_ReturnType retVal = E_NOT_OK;

	/* first get the right SE */
	const WdgM_SupervisedEntity *se = &WdgM_instance.ConfigPtr->General.SupervisedEntities[SEID];
	WdgM_internal_SupervisedEntity *runtime_se = &WdgM_instance.internalRuntimeDataPtr->SEs[SEID];

	/** @req WDGM284 */
	WDGM_CHECK_CP_RV(se, CheckpointID, WDGM_SID_CHECKPOINTREACHED);

	/** @req WDGM208 */
	if (se != NULL_PTR && runtime_se->LocalState != WDGM_LOCAL_STATUS_DEACTIVATED) /* se != NULL_PTR is just for making lint happy.... */
	{
		/* and now the right config for the current mode */
		uint16 i = 0;
		const WdgM_SupervisedEntityConfiguration *seConf = NULL_PTR;
		WdgM_internal_SupervisedEntityConfig *runtime_seConf = NULL_PTR;

		for(i = 0; i < WdgM_instance.CurrentMode->Length_SEConfigurations; i++)
		{
			if(WdgM_instance.CurrentMode->SEConfigurations[i].SupervisedEntityId == SEID)
			{
				seConf = &WdgM_instance.CurrentMode->SEConfigurations[i];
				runtime_seConf = &WdgM_instance.internalRuntimeDataPtr->Modes[WdgM_instance.CurrentMode->Id].SE_Configs[i];
				break;
			}
		}

		assert(seConf != NULL);
		assert(runtime_seConf != NULL);

		/* value stays correct */
		WdgM_internal_AliveMonitoring(seConf, runtime_seConf, CheckpointID);

		/** @req WDGM322 */
		/* if this algo doesn't find anything value stays correct */
		runtime_se->SubstateDeadline = WdgM_internal_DeadlineMonitoring(runtime_se, seConf, runtime_seConf, CheckpointID);

		if (seConf->CheckInternalLogic == TRUE)
		{
			/** @req WDGM323 */
			runtime_se->SubstateLogical = WdgM_internal_LogicalMonitoring(se, runtime_se, seConf, CheckpointID);
		}

		retVal = E_OK;
	}
#if (WDGM_DEV_ERROR_DETECT == STD_ON)
	else
	{
		/** @req WDGM319 */
		WDGM_REPORT_DET_ERROR(WDGM_SID_CHECKPOINTREACHED, WDGM_E_SEDEACTIVATED);
	}
#endif

	return retVal;
}

Std_ReturnType WdgM_UpdateAliveCounter( WdgM_SupervisedEntityIdType SEID)
{
	/** @req WDGM028 */
	WDGM_CHECK_INIT_RV(WDGM_SID_UPDATEALIVECOUNTER);

	/** @req WDGM290 */
	WDGM_REPORT_DET_ERROR(WDGM_SID_UPDATEALIVECOUNTER, WDGM_E_DEPRECATED);

	{
		uint16 i = 0;
		boolean found = FALSE;
		for(i = 0; i < WdgM_instance.CurrentMode->Length_SEConfigurations; i++)
		{
			if (WdgM_instance.CurrentMode->SEConfigurations[i].SupervisedEntityId == SEID)
			{
				found = TRUE;
				if (WdgM_instance.CurrentMode->SEConfigurations[i].Length_AliveSupervisions > 1)
				{
#if (WDGM_DEV_ERROR_DETECT == STD_ON)
					/** @req WDGM320 */
					WDGM_REPORT_DET_ERROR(WDGM_SID_UPDATEALIVECOUNTER, WDGM_E_AMBIGIOUS);
#endif
					return E_NOT_OK;
				}
				else
				{
					break;
				}
			}
		}

		if (found == FALSE)
		{
#if (WDGM_DEV_ERROR_DETECT == STD_ON)
			/** @req WDGM027 */
			WDGM_REPORT_DET_ERROR(WDGM_SID_UPDATEALIVECOUNTER, WDGM_E_PARAM_SEID);
#endif
			return E_NOT_OK;
		}
		else
		{
			/* is there even an alive-supervision for that configuration? */
			if (WdgM_instance.CurrentMode->SEConfigurations[i].Length_AliveSupervisions == 0)
			{
				return E_NOT_OK;
			}

			/* we now can call checkpointreached with the right id */
			/** @req WDGM318 */
			return WdgM_CheckpointReached(SEID, WdgM_instance.CurrentMode->SEConfigurations[i].AliveSupervisions[0].CheckpointId);
		}
	}
}

/** @req WDGM171 */
Std_ReturnType WdgM_GetLocalStatus( WdgM_SupervisedEntityIdType SEID, WdgM_LocalStatusType *Status)
{
	/** @req WDGM173 */
	WDGM_CHECK_INIT_RV(WDGM_SID_GETLOCALSTATUS);
	/** @req WDGM257 */
	WDGM_CHECK_NULL_POINTER_RV(Status, WDGM_SID_GETLOCALSTATUS);
	/** @req WDGM172 */
	WDGM_CHECK_SEID_RV(SEID, WDGM_SID_GETLOCALSTATUS, WDGM_E_PARAM_SEID);

	*Status = WdgM_instance.internalRuntimeDataPtr->SEs[SEID].LocalState;

	return E_OK;
}

Std_ReturnType WdgM_GetGlobalStatus( WdgM_GlobalStatusType *Status)
{
	/** @req WDGM176 */
	WDGM_CHECK_INIT_RV(WDGM_SID_GETGLOBALSTATUS);
	/** @req WDGM344 */
	/** @req WDGM258 */
	WDGM_CHECK_NULL_POINTER_RV(Status, WDGM_SID_GETGLOBALSTATUS);

	*Status = WdgM_instance.GlobalState;

	return E_OK;
}

Std_ReturnType WdgM_PerformReset( void )
{
	/** @req WDGM270 */
	WDGM_CHECK_INIT_RV(WDGM_SID_PERFORMRESET);

	Std_ReturnType retVal = E_NOT_OK;

	/** @req WDGM233 */
	if (WdgM_instance.ResetInitiated == FALSE)
	{
		WdgM_instance.ResetInitiated = TRUE;
		retVal = E_OK;

		/** @req WDGM232 */
		uint8 i = 0;

		for(i = 0; i < WdgM_instance.ConfigPtr->General.Length_Watchdogs; i++)
		{
			WdgIf_SetTriggerCondition(WdgM_instance.ConfigPtr->General.Watchdogs[i].WatchdogDeviceId, 0);
		}
	}

	return retVal;
}

Std_ReturnType WdgM_GetFirstExpiredSEID( WdgM_SupervisedEntityIdType *SEID)
{
	/** @req WDGM348 */
	/* there is no init check 'cause function should be available before init is called */

	Std_ReturnType retVal = E_NOT_OK;

	/** @req WDGM347 */
	WDGM_CHECK_NULL_POINTER_RV(SEID, WDGM_SID_GETFIRSTEXPIREDSEID);

	/** @req WDGM349 */
	if (WdgM_instance.isFirstExpiredSet == TRUE)
	{
		*SEID = WdgM_instance.firstExpiredSEID;
		retVal = E_OK;
	}

	return retVal;
}

/** @req WDGM060 */
/** @req WDGM327 */
void WdgM_MainFunction( void )
{
#if (WDGM_DEFENSIVE_BEHAVIOR == STD_ON)
	/** @req WDGM039 */
	WDGM_CHECK_INIT_NR(WDGM_SID_MAINFUNCTION);
#endif

	/** @req WDGM292 */
	if (WdgM_instance.GlobalState != WDGM_GLOBAL_STATUS_DEACTIVATED)
	{
		uint8 i = 0;

		/* update time */
		/** @req WDGM063 */
		WdgM_instance.CurrentTime += WdgM_instance.CurrentMode->SupervisionCycle;

		/** @req WDGM324 */
		/* now do alive monitoring */
		WdgM_internal_CheckAlives();

		/** @req WDGM325 */
		/** @req WDGM326 */
		/** @req WDGM214 */
		/* now do the other stuff - local states are implicitly calculated during global state calculation */
		WdgM_internal_CalculateGlobalState();

		if (WdgM_instance.GlobalState == WDGM_GLOBAL_STATUS_STOPPED)
		{
#if (WDGM_DEM_ALIVE_SUPERVISION_REPORT == STD_ON)
			/** @req WDGM129 */
			/** @req WDGM006 */
			WDGM_REPORT_DEM_ERROR(WDGM_SID_MAINFUNCTION, WdgM_instance.ConfigPtr->ConfigSet.DemEventIdRefs.Monitoring);
#endif
#if (WDGM_IMMEDIATE_RESET == STD_ON)
			/** @req WDGM133 */
			/** @req WDGM134 */
			Mcu_PerformReset();
#endif
		}

		/* set the triggers for all wdgdevices != WDGIF_OFF_MODE */
		/** @req WDGM328 */
		/** @req WDGM223 */
		for(i = 0; i < WdgM_instance.CurrentMode->Length_Triggers; i++)
		{
			/** @req WDGM119 */
			/** @req WDGM120 */
			/** @req WDGM121 */
			/** @req WDGM122 */
			if (WdgM_instance.CurrentMode->Triggers[i].WatchdogMode != WDGIF_OFF_MODE)
			{
				uint16 triggerValue = (WdgM_instance.GlobalState == WDGM_GLOBAL_STATUS_STOPPED)? 0 : WdgM_instance.CurrentMode->Triggers[i].TriggerConditionValue;
				WdgIf_SetTriggerCondition(WdgM_instance.ConfigPtr->General.Watchdogs[WdgM_instance.CurrentMode->Triggers[i].WatchdogId].WatchdogDeviceId, triggerValue);
			}
		}

	}
}

static void WdgM_internal_CheckAlives()
{
	/* cycle through all seConfigs and check the alive states and update the internal substate of the corresponding se */
	uint16 i = 0;

	for(i = 0; i < WdgM_instance.CurrentMode->Length_SEConfigurations; i++)
	{
		const WdgM_SupervisedEntityConfiguration *seConf = &WdgM_instance.CurrentMode->SEConfigurations[i];
		WdgM_internal_SupervisedEntity *runtime_se = &WdgM_instance.internalRuntimeDataPtr->SEs[seConf->SupervisedEntityId];

		WdgM_Substate result = WDGM_SUBSTATE_CORRECT;
		boolean writeResult = FALSE;

		uint16 a = 0;
		/** @req WDGM083 */
		for(a = 0; a < seConf->Length_AliveSupervisions; a++)
		{
			const WdgM_AliveSupervision *aliveCP = &seConf->AliveSupervisions[a];

			WdgM_internal_AliveSupervision *runtime_aliveCP = &WdgM_instance.internalRuntimeDataPtr->Modes[WdgM_instance.CurrentMode->Id].SE_Configs[i].AliveSupervisions[a];

			/* first increment the supervisioncyclecounter */
			runtime_aliveCP->SupervisionCycleCounter++;

			/** @req WDGM074 */
			WdgM_Substate tempRes = WdgM_internal_IsAliveHold(aliveCP, runtime_aliveCP);
			writeResult = runtime_aliveCP->wasEvaluated ? TRUE : writeResult;

			/** @req WDGM115 */
			if (tempRes == WDGM_SUBSTATE_INCORRECT)
			{
				/* when once there was an incorrect result don't update again to correct */
				result = tempRes;
			}
		}

		/* when one aliveCP was evaluated then the result is written - if no aliveCP was evaluated the state stays the same!!! */
		runtime_se->SubstateAlive = (writeResult == TRUE)? result : runtime_se->SubstateAlive;
	}
}

static WdgM_Substate WdgM_internal_SpecAliveAlgo(const WdgM_AliveSupervision *aliveCP, WdgM_internal_AliveSupervision *runtimeAliveCP)
{
	WdgM_Substate retVal = WDGM_SUBSTATE_INCORRECT;

	sint16 f = (sint16) aliveCP->SupervisionReferenceCycle - (sint16) aliveCP->ExpectedAliveIndications; /* we indeed need a signed int with negative numbers up to -255 and +255 */
	sint16 temp = (sint16) runtimeAliveCP->AliveCounter - (sint16) runtimeAliveCP->SupervisionCycleCounter;
	temp += f;

	if (temp <= aliveCP->MaxMargin && temp >= -aliveCP->MinMargin)
	{
		retVal = WDGM_SUBSTATE_CORRECT;
	}

	runtimeAliveCP->AliveCounter = 0;				/* reset counter for next check */
	runtimeAliveCP->SupervisionCycleCounter = 0;	/* reset counter in order to count cycles from beginning again */

	return retVal;
}

static WdgM_Substate WdgM_internal_IsAliveHold(const WdgM_AliveSupervision *aliveCP, WdgM_internal_AliveSupervision *runtime_aliveCP)
{
	WdgM_Substate retVal = WDGM_SUBSTATE_CORRECT;
	/* check all constraints */

	/* checking depends on supervisionreferencecycle */
	/* so if src == 1 we have to check each cycle */
	/* when src > 1 we have to check when src is reached */

	runtime_aliveCP->wasEvaluated = FALSE;

	/** @req WDGM098 */
	if (aliveCP->SupervisionReferenceCycle == 1)
	{
		/* each cycle */
		retVal = WdgM_internal_SpecAliveAlgo(aliveCP, runtime_aliveCP);
		runtime_aliveCP->wasEvaluated = TRUE;
	}
	else
	{
		/* multiple cycle */

		/* check if multiple is already reached => therefore we can use the SupervisionCycleCounter */
		if (runtime_aliveCP->SupervisionCycleCounter == aliveCP->SupervisionReferenceCycle)
		{
			retVal = WdgM_internal_SpecAliveAlgo(aliveCP, runtime_aliveCP);
			runtime_aliveCP->wasEvaluated = TRUE;
		}
	}

	return retVal;
}

static void WdgM_internal_CalculateLocalState_FromFailedNoError(WdgM_internal_SupervisedEntity *runtime_se)
{
	/** @req WDGM300 */
	if (runtime_se->FailedAliveCyclesCounter > 1)
	{
		/* decrement and stay in failed */
		runtime_se->FailedAliveCyclesCounter--;
	}
	else
	/** @req WDGM205 */
	{
		runtime_se->FailedAliveCyclesCounter = 0;
		runtime_se->LocalState = WDGM_LOCAL_STATUS_OK;
	}
}

static void WdgM_internal_CalculateLocalState_FromOKOneError(WdgM_internal_SupervisedEntity *runtime_se, const WdgM_SupervisedEntityConfiguration *seConf, WdgM_internal_SupervisedEntityConfig *runtime_seConf)
{
	/** @req WDGM203 */
	if (runtime_se->FailedAliveCyclesCounter <= seConf->FailedAliveSupervisionReferenceCycleTol && runtime_se->SubstateAlive == WDGM_SUBSTATE_INCORRECT)
	{
		boolean wasEval = FALSE;
		uint16 i = 0;

		for(i = 0; i < runtime_seConf->Length_AliveSupervisions; i++)
		{
			if (runtime_seConf->AliveSupervisions[i].wasEvaluated == TRUE)
			{
				wasEval = TRUE;
				break;
			}
		}

		runtime_se->LocalState = WDGM_LOCAL_STATUS_FAILED;

		if (wasEval)
		{
			/* only increment when aliveSV was evaluated and is still FAILED */
			runtime_se->FailedAliveCyclesCounter++;
		}
	}

	/** @req WDGM202 */
	if (runtime_se->FailedAliveCyclesCounter > seConf->FailedAliveSupervisionReferenceCycleTol || runtime_se->SubstateAlive == WDGM_SUBSTATE_CORRECT) /* one of the others have to be incorrect (not alive ) */
	{
		runtime_se->LocalState = WDGM_LOCAL_STATUS_EXPIRED;
	}
}

static void WdgM_internal_CalculateLocalState_FromFailedOneError(WdgM_internal_SupervisedEntity *runtime_se, const WdgM_SupervisedEntityConfiguration *seConf, WdgM_internal_SupervisedEntityConfig *runtime_seConf)
{
	/** @req WDGM204 */
	if (runtime_se->FailedAliveCyclesCounter <= seConf->FailedAliveSupervisionReferenceCycleTol && runtime_se->SubstateAlive == WDGM_SUBSTATE_INCORRECT)
	{
		boolean wasEval = FALSE;
		uint16 i = 0;

		for(i = 0; i < runtime_seConf->Length_AliveSupervisions; i++)
		{
			if (runtime_seConf->AliveSupervisions[i].wasEvaluated == TRUE)
			{
				wasEval = TRUE;
				break;
			}
		}

		runtime_se->LocalState = WDGM_LOCAL_STATUS_FAILED;

		if (wasEval)
		{
			/* only increment when aliveSV was evaluated and is still FAILED */
			runtime_se->FailedAliveCyclesCounter++;
		}
	}

	/** @req WDGM206 */
	if (runtime_se->FailedAliveCyclesCounter > seConf->FailedAliveSupervisionReferenceCycleTol)
	{
		runtime_se->LocalState = WDGM_LOCAL_STATUS_EXPIRED;
	}
}

static void WdgM_internal_CalculateLocalState(WdgM_internal_SupervisedEntity *runtime_se, const WdgM_SupervisedEntityConfiguration *seConf, WdgM_internal_SupervisedEntityConfig *runtime_seConf)
{
	/* calculate local state for the given SE */

	uint8 temp = runtime_se->SubstateAlive + runtime_se->SubstateDeadline + runtime_se->SubstateLogical;

	switch (temp)
	{
	case 3:
		/* everything is fine - nothing needs to be done */
		/** @req WDGM201 */

		if (runtime_se->LocalState == WDGM_LOCAL_STATUS_FAILED)
		{
			WdgM_internal_CalculateLocalState_FromFailedNoError(runtime_se);
		}

		break;
	case 2:
		/* may it's still working */

		if (runtime_se->LocalState == WDGM_LOCAL_STATUS_OK)
		{
			WdgM_internal_CalculateLocalState_FromOKOneError(runtime_se, seConf, runtime_seConf);
			break;
		}

		if (runtime_se->LocalState == WDGM_LOCAL_STATUS_FAILED)
		{
			WdgM_internal_CalculateLocalState_FromFailedOneError(runtime_se, seConf, runtime_seConf);
			break;
		}

		break;
	case 1:
	case 0:
		/* really bad */
		runtime_se->LocalState = WDGM_LOCAL_STATUS_EXPIRED;
		break;
	}
}

static void WdgM_internal_updateFromGlobalOK(uint16 expired, uint16 failed)
{
	/** @req WDGM076 */
	if (failed > 0)
	{
		WdgM_instance.GlobalState = WDGM_GLOBAL_STATUS_FAILED;
		/* no return since expired isn't evaluated yet */
	}

	/** @req WDGM215 */
	if (expired > 0 && WdgM_instance.internalRuntimeDataPtr->Modes[WdgM_instance.CurrentMode->Id].ExpiredSupervisionCycleCounter < WdgM_instance.CurrentMode->ExpiredSupervisionCycleTol)
	{
		WdgM_instance.GlobalState = WDGM_GLOBAL_STATUS_EXPIRED;
		return;
	}

	/** @req WDGM216 */
	if (expired > 0 && WdgM_instance.internalRuntimeDataPtr->Modes[WdgM_instance.CurrentMode->Id].ExpiredSupervisionCycleCounter >= WdgM_instance.CurrentMode->ExpiredSupervisionCycleTol)
	{
		WdgM_instance.GlobalState = WDGM_GLOBAL_STATUS_STOPPED;
		return;
	}
}

static void WdgM_internal_updateFromGlobalFAILED(uint16 expired, uint16 failed)
{
	/** @req WDGM218 */
	if (failed == 0 && expired == 0)
	{
		WdgM_instance.GlobalState = WDGM_GLOBAL_STATUS_OK;
		return;
	}

	/** @req WDGM077 */
	if (expired > 0 && WdgM_instance.internalRuntimeDataPtr->Modes[WdgM_instance.CurrentMode->Id].ExpiredSupervisionCycleCounter < WdgM_instance.CurrentMode->ExpiredSupervisionCycleTol)
	{
		WdgM_instance.GlobalState = WDGM_GLOBAL_STATUS_EXPIRED;
		return;
	}

	/** @req WDGM117 */
	if (expired > 0 && WdgM_instance.internalRuntimeDataPtr->Modes[WdgM_instance.CurrentMode->Id].ExpiredSupervisionCycleCounter >= WdgM_instance.CurrentMode->ExpiredSupervisionCycleTol)
	{
		WdgM_instance.GlobalState = WDGM_GLOBAL_STATUS_STOPPED;
	}

	/** @req WDGM217 */
	/* remain in failed when nothing applies (failed > 0 && expired == 0).. so just do nothing */
}

static void WdgM_internal_updateFromGlobalEXPIRED(uint16 expired)
{
	/** @req WDGM219 */
	if (expired > 0 && WdgM_instance.internalRuntimeDataPtr->Modes[WdgM_instance.CurrentMode->Id].ExpiredSupervisionCycleCounter < WdgM_instance.CurrentMode->ExpiredSupervisionCycleTol)
	{
		/* stay in expired and increment the counter */
		WdgM_instance.internalRuntimeDataPtr->Modes[WdgM_instance.CurrentMode->Id].ExpiredSupervisionCycleCounter++;
		return;
	}

	/** @req WDGM220 */
	if (expired > 0 && WdgM_instance.internalRuntimeDataPtr->Modes[WdgM_instance.CurrentMode->Id].ExpiredSupervisionCycleCounter >= WdgM_instance.CurrentMode->ExpiredSupervisionCycleTol)
	{
		WdgM_instance.GlobalState = WDGM_GLOBAL_STATUS_STOPPED;
	}
}

/** @req WDGM078 */ /* there is nothing to do when status is ok or deactivated */
static void WdgM_internal_CalculateGlobalState()
{
	/* go trough all active SEs and calc the global state depending on their local states */
	uint16 expiredCounter = 0;
	uint16 failedCounter = 0;
	uint16 i = 0;

	for (i = 0; i < WdgM_instance.CurrentMode->Length_SEConfigurations; i++)
	{
		const WdgM_SupervisedEntityConfiguration *seConf = &WdgM_instance.CurrentMode->SEConfigurations[i];
		WdgM_internal_SupervisedEntityConfig *runtime_seConf = &WdgM_instance.internalRuntimeDataPtr->Modes[WdgM_instance.CurrentMode->Id].SE_Configs[i];
		const WdgM_SupervisedEntity *se = &WdgM_instance.ConfigPtr->General.SupervisedEntities[seConf->SupervisedEntityId];
		WdgM_internal_SupervisedEntity *runtime_se = &WdgM_instance.internalRuntimeDataPtr->SEs[seConf->SupervisedEntityId];

		WdgM_internal_CalculateLocalState(runtime_se, seConf, runtime_seConf);

		if (runtime_se->LocalState == WDGM_LOCAL_STATUS_EXPIRED)
		{
			/** @req WDGM351 */
			if (WdgM_instance.isFirstExpiredSet == FALSE)
			{
				WdgM_instance.firstExpiredSEID = i;
				WdgM_instance.isFirstExpiredSet = TRUE;
			}

			expiredCounter++;
		}

		if (runtime_se->LocalState == WDGM_LOCAL_STATUS_FAILED)
		{
			if (se->isOsApplicationRefSet == TRUE)
			{
				/** @req WDGM225 */
#if defined(USE_BSWM)
				BswM_WdgM_RequestPartitionReset(se->OsApplicationRef);
#endif
			}

			failedCounter++;
		}
	}

	if (WdgM_instance.GlobalState == WDGM_GLOBAL_STATUS_OK)
	{
		WdgM_internal_updateFromGlobalOK(expiredCounter, failedCounter);
		return;
	}

	if (WdgM_instance.GlobalState == WDGM_GLOBAL_STATUS_FAILED)
	{
		WdgM_internal_updateFromGlobalFAILED(expiredCounter, failedCounter);
		return;
	}

	if (WdgM_instance.GlobalState == WDGM_GLOBAL_STATUS_EXPIRED)
	{
		WdgM_internal_updateFromGlobalEXPIRED(expiredCounter);
		return;
	}

	/** @req WDGM221 */
	/* when stopped then stay in stopped */
}

static void WdgM_internal_ResetSEConfigsForMode(WdgM_internal_Mode *runtime_Mode)
{
	uint16 i = 0;

	for (i = 0; i < runtime_Mode->Length_SEConfigs; i++)
	{
		runtime_Mode->ExpiredSupervisionCycleCounter = 0;

		/* reset timestamps, alivecounters, previous cp and external active-flag */
		uint16 a = 0;
		for (a = 0; a < runtime_Mode->SE_Configs[i].Length_AliveSupervisions; a++)
		{
			runtime_Mode->SE_Configs[i].AliveSupervisions[a].AliveCounter = 0;
			runtime_Mode->SE_Configs[i].AliveSupervisions[a].SupervisionCycleCounter = 0;
		}

		for (a = 0; a < runtime_Mode->SE_Configs[i].Length_DeadlineSupervisions; a++)
		{
			/** @req WDGM298 */
			runtime_Mode->SE_Configs[i].DeadlineSupervisions[a].TimestampStart = 0;
		}
	}
}

static void WdgM_internal_AliveMonitoring(const WdgM_SupervisedEntityConfiguration *seConf, WdgM_internal_SupervisedEntityConfig *runtime_seConf, WdgM_CheckpointIdType CPId)
{
	uint16 i = 0;
	WdgM_internal_AliveSupervision *runtime_aliveCP = NULL_PTR;

	if (seConf != NULL_PTR) /* just for making lint happy */
	{
		/* search for the checkpoint */
		for(i = 0; i < seConf->Length_AliveSupervisions; i++)
		{
			if (seConf->AliveSupervisions[i].CheckpointId == CPId)
			{
				runtime_aliveCP =  &runtime_seConf->AliveSupervisions[i];
				break;
			}
		}

		if (runtime_aliveCP != NULL_PTR)
		{
			/** @req WDGM321 */
			runtime_aliveCP->AliveCounter++;
		}
	}
}

/** @req WDGM299 */
static WdgM_Substate WdgM_internal_DeadlineMonitoring(WdgM_internal_SupervisedEntity *runtime_se, const WdgM_SupervisedEntityConfiguration *seConf, WdgM_internal_SupervisedEntityConfig *runtime_seConf, WdgM_CheckpointIdType CPId)
{
	WdgM_Substate result = WDGM_SUBSTATE_CORRECT;
	uint16 i = 0;

	if (runtime_se != NULL_PTR && seConf != NULL_PTR) /* just for making lint happy */
	{
		/* search for checkpoints */
		for(i = 0; i < seConf->Length_DeadlineSupervisions; i++)
		{
			/* if it's a start dlcp then set timestamp */
			if (seConf->DeadlineSupervisions[i].CheckpointIdStart == CPId)
			{
				/** @req WDGM228 */
				runtime_seConf->DeadlineSupervisions[i].TimestampStart = WdgM_instance.CurrentTime;

				/** @req WDGM246 */
				runtime_se->PreviousCheckpointId_Deadline = CPId;
			}

			/* if it's a finsih dlcp then do evaluation */
			if (seConf->DeadlineSupervisions[i].CheckpointIdFinish == CPId && runtime_seConf->DeadlineSupervisions[i].TimestampStart != 0)
			{
				/** @req WDGM229 */
				/* measure the time and get calculate result */
				if (WdgM_internalIsDeadlineHold(&seConf->DeadlineSupervisions[i], &runtime_seConf->DeadlineSupervisions[i]) == FALSE)
				{
					result = WDGM_SUBSTATE_INCORRECT;
					break;
				}

				/* set back timestamp */
				runtime_seConf->DeadlineSupervisions[i].TimestampStart = 0;

				/** @req WDGM246 */
				runtime_se->PreviousCheckpointId_Deadline = CPId;
			}
		}
	}

	return result;
}

static WdgM_Substate WdgM_internalIsDeadlineHold(const WdgM_DeadlineSupervision *deadlineCP, WdgM_internal_DeadlineSupervision *runtime_deadlineCP)
{
	/** @req WDGM294 */
	if (WdgM_instance.CurrentTime >= runtime_deadlineCP->TimestampStart + deadlineCP->DeadlineMin &&
		WdgM_instance.CurrentTime <= runtime_deadlineCP->TimestampStart + deadlineCP->DeadlineMax)
	{
		return WDGM_SUBSTATE_CORRECT;
	}

	return WDGM_SUBSTATE_INCORRECT;
}

static WdgM_Substate WdgM_internal_LogicalMonitoring_GraphActive(const WdgM_SupervisedEntity *se, WdgM_internal_SupervisedEntity *runtime_se, WdgM_CheckpointIdType CPId)
{
	WdgM_Substate retVal = WDGM_SUBSTATE_INCORRECT;

	/** @req WDGM246 */
	/* check whether the transition from the previous checkpoint to the current is allowed */
	uint16 i = 0;

	for(i = 0; i < se->Length_Transitions; i++)
	{
		if(se->Transitions[i].CheckpointIdSource == runtime_se->PreviousCheckpointId_internalLogic)
		{
			if(se->Transitions[i].CheckpointIdDestination == CPId)
			{
				retVal = WDGM_SUBSTATE_CORRECT;

				/** @req WDGM246 */
				runtime_se->PreviousCheckpointId_internalLogic = CPId;

				break;
			}
		}
	}

	if (retVal == WDGM_SUBSTATE_CORRECT)
	{
		/* now check if this cp is a finishcp */
		for(i = 0; i < se->Length_FinalCheckpointIds; i++)
		{
			if (se->FinalCheckpointIds[i] == CPId)
			{
				/** @req WDGM331 */
				runtime_se->IsInternalGraphActive = FALSE;
				break;
			}
		}
	}

	return retVal;
}

static WdgM_Substate WdgM_internal_LogicalMonitoring_GraphInactive(const WdgM_SupervisedEntity *se, WdgM_internal_SupervisedEntity *runtime_se, WdgM_CheckpointIdType CPId)
{
	WdgM_Substate retVal = WDGM_SUBSTATE_INCORRECT;

	/* check whether the reported cp is a startcp */
	uint16 i = 0;

	for(i = 0; i < se->Length_StartCheckpointIds; i++)
	{
		if (se->StartCheckpointIds[i] == CPId)
		{
			retVal = WDGM_SUBSTATE_CORRECT;

			/** @req WDGM332 */
			/** @req WDGM273 */
			/** @req WDGM329 */
			runtime_se->IsInternalGraphActive = TRUE;

			/** @req WDGM246 */
			runtime_se->PreviousCheckpointId_internalLogic = CPId;

			break;
		}
	}

	return retVal;
}

static WdgM_Substate WdgM_internal_LogicalMonitoring(const WdgM_SupervisedEntity *se, WdgM_internal_SupervisedEntity *runtime_se, const WdgM_SupervisedEntityConfiguration *seConf, WdgM_CheckpointIdType CPId)
{
	WdgM_Substate retVal = WDGM_SUBSTATE_INCORRECT;

	if (se != NULL_PTR && seConf != NULL_PTR) /* just for making lint happy */
	{

		/* first of all check if this is a start-cp */
		/* when so we have to check if there is an external transition configured and additionally check the validness */
		/* when it's not a start-cp we have to check graph-internal transitions */

		/** @req WDGM274 */
		if (runtime_se->IsInternalGraphActive == TRUE)
		{
			retVal = WdgM_internal_LogicalMonitoring_GraphActive(se, runtime_se, CPId);
		}
		else
		{
			retVal = WdgM_internal_LogicalMonitoring_GraphInactive(se, runtime_se, CPId);
		}
	}

	return retVal;
}

static void WdgM_internal_DeactivateAndResetSE(uint16 SEIndex)
{
	WdgM_internal_SupervisedEntity *runtime_se = &WdgM_instance.internalRuntimeDataPtr->SEs[SEIndex];

	if (runtime_se != NULL_PTR)	/* just for making lint happy...... */
	{
		runtime_se->LocalState = WDGM_LOCAL_STATUS_DEACTIVATED;

		runtime_se->IsInternalGraphActive = FALSE;
		runtime_se->PreviousCheckpointId_internalLogic = 0; /* be aware that 0 is a valid value => but flag is inactive so this dosn't matter */
		runtime_se->PreviousCheckpointId_Deadline = 0;		/* be aware that 0 is a valid value => but flag is inactive so this dosn't matter */
		runtime_se->FailedAliveCyclesCounter = 0;

		runtime_se->SubstateAlive = WDGM_SUBSTATE_CORRECT;
		runtime_se->SubstateDeadline = WDGM_SUBSTATE_CORRECT;
		runtime_se->SubstateLogical = WDGM_SUBSTATE_CORRECT;
	}
}
