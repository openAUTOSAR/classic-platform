/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/


// 904 PC-Lint MISRA 14.7: OK. Allow VALIDATE_ENTITY_ID, VALIDATE and VALIDATE_NO_RETURNVAL to return value.
//lint -emacro(904,VALIDATE_ENTITY_ID,VALIDATE,VALIDATE_NO_RETURNVAL)
// lint -emacro(750,WDGM_REPORT_ERROR) //PC-Lint exception. Is used if Gpt is activated and no Gpt is available.

#include "WdgM.h"
#include "WdgIf.h"

#if defined(USE_DET)
#include "Det.h"
#endif

#if defined(USE_DEM)
#include "Dem.h"
#endif

#if defined(HOST_TEST)
#include "Dem_IntErrId.h"
#include "Dem_Types.h"
#endif

#if defined(CFG_WDGM_TEST)
#include <assert.h>
#endif

//#define USE_LDEBUG_PRINTF 1
#include "debug.h"

/** @req WDGM022 */
/** @req WDGM138 */
#if defined(USE_DEM) || defined(HOST_TEST)
#define REPORT_TO_DEM( _err ) Dem_ReportErrorStatus(_err, DEM_EVENT_STATUS_FAILED);
#else
#define REPORT_TO_DEM( _err )
#endif

#if (WDGM_DEV_ERROR_DETECT == STD_ON)
#define WDGM_REPORT_ERROR(_api,_errorcode) \
	Det_ReportError(MODULE_ID_WDGM,0,_api,_errorcode );

#define VALIDATE_ENTITY_ID(_SEId, _api) \
	if(_SEId >= WDGM_NBR_OF_ALIVE_SIGNALS) {	\
		Det_ReportError(MODULE_ID_WDGM,0,_api,WDGM_E_PARAM_SEID ); \
		ret = E_NOT_OK;	\
		return ret;	\
		}

#define VALIDATE(_expr, _api, _errorcode) \
	if(!_expr) {	\
		Det_ReportError(MODULE_ID_WDGM,0,_api,_errorcode ); \
		ret = E_NOT_OK;	\
		return ret;	\
		}

#define VALIDATE_NO_RETURNVAL(_expr, _api, _errorcode) \
	if(!_expr) {	\
		Det_ReportError(MODULE_ID_WDGM,0,_api,_errorcode ); \
		return;	\
		}
#else
#define WDGM_REPORT_ERROR(_api,_errorcode)
#define VALIDATE_ENTITY_ID(_SEId, _api)
#define VALIDATE(_expr, _api, _errorcode)
#define VALIDATE_NO_RETURNVAL(_expr, _api, _errorcode)
#endif

struct
{
	const WdgM_ConfigType           *WdgM_ConfigPtr;
	WdgM_AliveSupervisionStatusType WdgM_GlobalSupervisionStatus;
	WdgM_SupervisionCounterType     WdgM_ExpiredSupervisionCycles;
	WdgM_TriggerCounterType         WdgMTriggerCounter;
	WdgM_ModeType                   WdgMActiveMode;
	boolean							WdgMModeConfigured;
}wdgMInternalState;


/* Helper macros */
#define GET_ENTITY_STATE_PTR(_SEId)       (&(wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_AliveEntityStatePtr)[_SEId])

/* Function prototypes. */
static boolean WdgM_IsAlive(void);
static void wdgm_Check_AliveSupervision (void);
static void wdgm_Trigger (void);
void WdgM_Cbk_GptNotification (void);


Std_ReturnType WdgM_UpdateAliveCounter (WdgM_SupervisedEntityIdType SEid)
{
  Std_ReturnType ret = E_NOT_OK;
  /** @req WDGM027 **/
  VALIDATE_ENTITY_ID(SEid, WDGM_UPDATEALIVECOUNTER_ID);
  /** @req WDGM028 **/
  VALIDATE((wdgMInternalState.WdgM_ConfigPtr != 0), WDGM_UPDATEALIVECOUNTER_ID, WDGM_E_NO_INIT);
  WdgM_AliveEntityStateType *entityStatePtr = GET_ENTITY_STATE_PTR(SEid);

  /** @req WDGM083 **/
  /** @req WDGM114 **/
  if (entityStatePtr->SupervisionStatus < WDGM_ALIVE_EXPIRED)
  {
	  entityStatePtr->AliveCounter++;
	  ret = E_OK;
  }
  return (ret);
}

/** @req WDGM053 **/
/** @req WDGM059 **/
/** @req WDGM156 **/
Std_ReturnType WdgM_ActivateAliveSupervision (WdgM_SupervisedEntityIdType SEid)
{
  Std_ReturnType ret = E_NOT_OK;
  /** @req WDGM055 **/
  VALIDATE_ENTITY_ID(SEid, WDGM_ACTIVATEALIVESUPERVISION_ID);
  /** @req WDGM056 **/
  VALIDATE((wdgMInternalState.WdgM_ConfigPtr != 0), WDGM_ACTIVATEALIVESUPERVISION_ID, WDGM_E_NO_INIT);
  WdgM_AliveEntityStateType *entityStatePtr = GET_ENTITY_STATE_PTR(SEid);

  if ((WDGM_ALIVE_EXPIRED > entityStatePtr->SupervisionStatus) ||
      (WDGM_ALIVE_DEACTIVATED == entityStatePtr->SupervisionStatus))
  {
	  entityStatePtr->SupervisionStatus = WDGM_ALIVE_OK;
	  ret = E_OK;
  }
  return (ret);
}

/** @req WDGM079 **/
/** @req WDGM157 **/
Std_ReturnType WdgM_DeactivateAliveSupervision (WdgM_SupervisedEntityIdType SEid)
{
  Std_ReturnType ret = E_NOT_OK;
  /** @req WDGM057 **/
  VALIDATE_ENTITY_ID(SEid, WDGM_DEACTIVATEALIVESUPERVISION_ID);
  /** @req WDGM058 **/
  VALIDATE((wdgMInternalState.WdgM_ConfigPtr != 0), WDGM_DEACTIVATEALIVESUPERVISION_ID, WDGM_E_NO_INIT);
  WdgM_AliveEntityStateType *entityStatePtr = GET_ENTITY_STATE_PTR(SEid);
#if (WDGM_DEV_ERROR_DETECT == STD_ON)
  const WdgM_SupervisedEntityType *supervisedEntityPtr = wdgMInternalState.WdgM_ConfigPtr->WdgM_General->WdgM_SupervisedEntityPtr;
#endif

  /** @req WDGM082 **/
  /** @req WDGM174 **/
  /** @req WDGM108 **/
  VALIDATE((supervisedEntityPtr[SEid].WdgM_DeactivationAccessEnabled != 0), WDGM_DEACTIVATEALIVESUPERVISION_ID, WDGM_E_DEACTIVATE_NOT_ALLOWED);

  /** @req WDGM114 **/
  if (WDGM_ALIVE_EXPIRED > entityStatePtr->SupervisionStatus)
  {
	  entityStatePtr->SupervisionStatus = WDGM_ALIVE_DEACTIVATED;
	  ret = E_OK;
  }

  return (ret);
}

/** @req WDGM169 **/
Std_ReturnType WdgM_GetAliveSupervisionStatus (WdgM_SupervisedEntityIdType SEid, WdgM_AliveSupervisionStatusType *Status)
{
	Std_ReturnType ret;
	/** @req WDGM172 **/
	VALIDATE_ENTITY_ID(SEid, WDGM_GETALIVESUPERVISIONSTATUS_ID);
	/** @req WDGM173 **/
	VALIDATE((wdgMInternalState.WdgM_ConfigPtr != 0), WDGM_GETALIVESUPERVISIONSTATUS_ID, WDGM_E_NO_INIT);
	VALIDATE((Status != 0), WDGM_GETALIVESUPERVISIONSTATUS_ID, WDGM_E_NULL_POINTER);
	WdgM_AliveEntityStateType *entityStatePtr = GET_ENTITY_STATE_PTR(SEid);
	*Status = entityStatePtr->SupervisionStatus;
	ret = E_OK;
	return ret;
}

/** @req WDGM175 **/
Std_ReturnType WdgM_GetGlobalStatus (WdgM_AliveSupervisionStatusType *Status)
{
	Std_ReturnType ret = E_NOT_OK;
	/** @req WDGM176 **/
	VALIDATE((wdgMInternalState.WdgM_ConfigPtr != 0), WDGM_GETGLOBALSTATUS_ID, WDGM_E_NO_INIT);
	*Status = wdgMInternalState.WdgM_GlobalSupervisionStatus;
	ret = E_OK;
	return ret;
}

/** @req WDGM154 **/
Std_ReturnType WdgM_SetMode(WdgM_ModeType Mode) {
	Std_ReturnType ret = E_OK;
	/** @req WDGM021 **/
	VALIDATE((wdgMInternalState.WdgM_ConfigPtr != 0), WDGM_SETMODE_ID, WDGM_E_NO_INIT);
	/** @req WDGM020 **/
	VALIDATE(((Mode >= 0) && (Mode < WDGM_NBR_OF_MODES)), WDGM_SETMODE_ID, WDGM_E_PARAM_MODE);

	WdgM_AliveSupervisionStatusType maxLocal = WDGM_ALIVE_OK;
	WdgM_AliveEntityStateType *entityStatePtr;

	const WdgM_ModeConfigType * modeConfigPtr =	&wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_Mode[Mode];
	const WdgM_ModeConfigType * oldModeConfigPtr = &wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_Mode[wdgMInternalState.WdgMActiveMode];

#if WDGM_OFF_MODE_ENABLED == STD_OFF
	/** @req WDGM031 **/
	VALIDATE((modeConfigPtr->WdgM_Trigger->WdgM_WatchdogMode != WDGIF_OFF_MODE),WDGM_SETMODE_ID, WDGM_E_DISABLE_NOT_ALLOWED);
#endif

	WdgIf_ModeType mode;
	uint8 deviceIndex;
	uint8 i;
	boolean newModeActivated;
	boolean oldModeActivated;

	/** @req WDGM145 **/
	if (wdgMInternalState.WdgM_GlobalSupervisionStatus == WDGM_ALIVE_OK) {
		/* Compare activation type between new and old mode. */
		if (oldModeConfigPtr->WdgM_Activation.WdgM_IsGPTActivated
				!= modeConfigPtr->WdgM_Activation.WdgM_IsGPTActivated) {
			/** @req WDGM188 **/
			if (modeConfigPtr->WdgM_Activation.WdgM_IsGPTActivated) {
#if WDGM_GPT_USED == STD_ON
				const WdgM_ActivationGPTType * gptConfigPtr = &modeConfigPtr->WdgM_Activation.WdgM_ActivationGPT;
				/* New mode is activated from GPT callback. Start GPT. */
				Gpt_StartTimer(gptConfigPtr->WdgM_GptChannelRef, gptConfigPtr->WdgM_GptCycle);
				Gpt_EnableNotification(gptConfigPtr->WdgM_GptChannelRef);
#else
				WDGM_REPORT_ERROR( WDGM_SETMODE_ID, WDGM_E_PARAM_MODE );
				ret = E_NOT_OK;
#endif
			}
			/** @req WDGM187 **/
			else {
				/* Only if a mode already have been configured. */
				if (wdgMInternalState.WdgMModeConfigured) {
#if WDGM_GPT_USED == STD_ON
					/* Old mode was GPT driven, but not new mode. Disable GPT. */
					const WdgM_ActivationGPTType * oldGptConfigPtr = &oldModeConfigPtr->WdgM_Activation.WdgM_ActivationGPT;
					Gpt_DisableNotification(oldGptConfigPtr->WdgM_GptChannelRef);
					Gpt_StopTimer(oldGptConfigPtr->WdgM_GptChannelRef);
#else
					WDGM_REPORT_ERROR( WDGM_SETMODE_ID, WDGM_E_PARAM_MODE );
					ret = E_NOT_OK;
#endif
				} else {
					/* Do nothing since no mode have been configured. */
				}
			}
		} else {
			/* No activation change needed. */
		}

		for ( WdgM_SupervisedEntityIdType SEid = 0; SEid < WDGM_NBR_OF_ALIVE_SIGNALS; SEid++) {
			/* 4 Cases
			 *   old new
			 * 1  0  0  WDGM_ALIVE_DEACTIVATED
			 * 2  0  1  WDGM_ALIVE_DEACTIVATED
			 * 3  1  0  WDGM_ALIVE_OK
			 * 4  1  1  Retain
			 */
			entityStatePtr = GET_ENTITY_STATE_PTR(SEid);
			newModeActivated = modeConfigPtr->WdgM_AliveSupervisionPtr[SEid].WdgM_ActivationActivated;
			oldModeActivated  = oldModeConfigPtr->WdgM_AliveSupervisionPtr[SEid].WdgM_ActivationActivated;
			if (!newModeActivated) {
				/* @req 3.1.5/WDM0183 */ /* If deactivated in new mode set to WDGM_ALIVE_DEACTIVATED */
				entityStatePtr->SupervisionStatus = WDGM_ALIVE_DEACTIVATED;
			} else if (!oldModeActivated) {
				/* !req 3.1.5/WDM0184 */ /* If deactivated in old mode set new to WDGM_ALIVE_OK */
				entityStatePtr->SupervisionStatus = WDGM_ALIVE_OK;
			} else if ( newModeActivated && oldModeActivated ) {
				/* @req 3.1.5/WDM0182 Retain mode */
				maxLocal = MAX(entityStatePtr->SupervisionStatus,maxLocal);
			}

			entityStatePtr->AliveCounter = 0;
			entityStatePtr->SupervisionCycle = 0;
			entityStatePtr->NbrOfFailedRefCycles = 0;
		}

		/* @req 3.1.5/WDM0185 Compute new global supervision status*/
		wdgMInternalState.WdgM_GlobalSupervisionStatus = maxLocal;

		/* Write new mode to internal state. */
		wdgMInternalState.WdgMActiveMode = Mode;

		/* Pass mode to all watchdog instances. */
		for (i = 0; i < wdgMInternalState.WdgM_ConfigPtr->WdgM_General->WdgM_NumberOfWatchdogs; i++) {
			mode = wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_Mode[Mode].WdgM_Trigger[i].WdgM_WatchdogMode;
			deviceIndex = wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_Mode[Mode].WdgM_Trigger[i].WdgM_WatchdogRef->WdgM_DeviceRef->WdgIf_DeviceIndex;
			/** @req WDGM139 **/
			if (E_NOT_OK == WdgIf_SetMode(deviceIndex, mode)) {
				wdgMInternalState.WdgM_GlobalSupervisionStatus = WDGM_ALIVE_STOPPED;
				ret = E_NOT_OK;
			}
		}
	}

	if(ret != E_OK){
		REPORT_TO_DEM(WDGM_E_SET_MODE)
	}
	wdgMInternalState.WdgMModeConfigured = TRUE;
	return (ret);
}

/** @req WDGM168 **/
Std_ReturnType WdgM_GetMode(WdgM_ModeType *Mode)
{
	Std_ReturnType ret = E_NOT_OK;
	/** @req WDGM170 **/
	VALIDATE((wdgMInternalState.WdgM_ConfigPtr != 0), WDGM_GETMODE_ID, WDGM_E_NO_INIT);
	VALIDATE((Mode != 0), WDGM_GETMODE_ID, WDGM_E_NULL_POINTER);

	*Mode = wdgMInternalState.WdgMActiveMode;
	ret = E_OK;
	return ret;
}

/** @req WDGM151 **/
void WdgM_Init(const WdgM_ConfigType *ConfigPtr)
{
  WdgM_SupervisedEntityIdType SEid;
  WdgM_AliveEntityStateType *entityStatePtr;
  WdgM_ModeType initialMode;
  const WdgM_ModeConfigType *modeConfigPtr;

  /** @req WDGM010 **/
  VALIDATE_NO_RETURNVAL((ConfigPtr != 0),WDGM_INIT_ID, WDGM_E_PARAM_CONFIG);
  wdgMInternalState.WdgM_ConfigPtr = ConfigPtr;

  initialMode = wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_InitialMode;
  modeConfigPtr = &wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_Mode[initialMode];

#if WDGM_OFF_MODE_ENABLED == STD_OFF
  /** @req WDGM030 **/
  VALIDATE_NO_RETURNVAL((modeConfigPtr->WdgM_Trigger->WdgM_WatchdogMode != WDGIF_OFF_MODE),WDGM_INIT_ID, WDGM_E_DISABLE_NOT_ALLOWED);
#endif

  /** @req WDGM018 **/
  for (SEid = 0; SEid < WDGM_NBR_OF_ALIVE_SIGNALS; SEid++)
  {
	const WdgM_AliveSupervisionType* aliveSupervisionPtr = &(modeConfigPtr->WdgM_AliveSupervisionPtr[SEid]);
	entityStatePtr = GET_ENTITY_STATE_PTR(aliveSupervisionPtr->WdgM_AliveSupervisionConfigID);
	if (aliveSupervisionPtr->WdgM_ActivationActivated)
	{
		entityStatePtr->SupervisionStatus = WDGM_ALIVE_OK;
	}
	else
	{
		entityStatePtr->SupervisionStatus = WDGM_ALIVE_DEACTIVATED;
	}
	entityStatePtr->AliveCounter         = 0;
    entityStatePtr->SupervisionCycle     = 0;
    entityStatePtr->NbrOfFailedRefCycles = 0;
  }


#if defined(CFG_WDGM_TEST)
  /* WDGM generator validation */
  {
	  enum WdgM_Mode mode;

	  LDEBUG_PRINTF("Mode  ID   Found\n",SEid,cSID, mode );
	  for( mode = 0; mode < WDGM_NBR_OF_MODES; mode++ ) {

		  /* For each mode verify the configuration */
		  modeConfigPtr = &wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_Mode[mode];
		  assert( modeConfigPtr->WdgM_ModeId == mode );

		  for (SEid = 0; SEid < WDGM_NBR_OF_ALIVE_SIGNALS; SEid++) {
			  const WdgM_AliveSupervisionType* aliveSupervisionPtr = &(modeConfigPtr->WdgM_AliveSupervisionPtr[SEid]);
			  enum WdgM_SupervisedEntityId cSID = aliveSupervisionPtr->WdgM_SupervisedEntityRef->WdgM_SupervisedEntityID;

			  LDEBUG_PRINTF("%4d %4d ",mode, SEid  );
			  if( cSID != SEid ) {
				  assert(0);
				  LDEBUG_PRINTF("%4d\n",cSID);
			  } else {
				  LDEBUG_PRINTF("OK\n");
			  }
		  }
	  }
  }
#endif


  /* Start initial mode. Raise error if initial mode was not entered properly. */
  VALIDATE_NO_RETURNVAL((WdgM_SetMode(initialMode) == E_OK),WDGM_INIT_ID, WDGM_E_PARAM_CONFIG);

  wdgMInternalState.WdgM_GlobalSupervisionStatus = WDGM_ALIVE_OK;
  wdgMInternalState.WdgMActiveMode = initialMode;
  wdgMInternalState.WdgM_ExpiredSupervisionCycles = 0;
  wdgMInternalState.WdgMTriggerCounter = 0;
  wdgMInternalState.WdgMModeConfigured = FALSE;
}

/* Non standard API for test purpose.  */
void WdgM_DeInit( void)
{
	wdgMInternalState.WdgM_GlobalSupervisionStatus = 0;
	wdgMInternalState.WdgM_ConfigPtr = 0;
}

static void wdgm_Check_AliveSupervision (void)
{
  WdgM_SupervisedEntityIdType SEid;
  WdgM_AliveEntityStateType *entityStatePtr;
  const WdgM_AliveSupervisionType *entityPtr;
  WdgM_SupervisionCounterType aliveCalc, nSC, nAl, f;
  WdgM_AliveSupervisionStatusType maxLocal = WDGM_ALIVE_OK;

  for (SEid = 0; SEid < wdgMInternalState.WdgM_ConfigPtr->WdgM_General->WdgM_NumberOfSupervisedEntities; SEid++)
  {
    entityPtr      = &(wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_Mode[wdgMInternalState.WdgMActiveMode].WdgM_AliveSupervisionPtr)[SEid];
    entityStatePtr = GET_ENTITY_STATE_PTR(entityPtr->WdgM_AliveSupervisionConfigID);

    /** @req WDGM083 **/
    if (WDGM_ALIVE_DEACTIVATED != entityStatePtr->SupervisionStatus)
    {
    	entityStatePtr->SupervisionCycle++;
      /** @req WDGM090 **/
      /* Only perform supervision on the reference cycle. */
      if (entityStatePtr->SupervisionCycle == entityPtr->WdgM_SupervisionReferenceCycle)
      {
        /* Alive algorithm. *
         * n (Al) - n(SC) + f(SRC,EAI) == 0; f(SRC,EAI) = SRC - EAI
         * This algorithm is from revision 4.0 of the WdgM specification.
         * Revision 3.1 does not seem to work.. */
        nSC = entityStatePtr->SupervisionCycle;
        nAl = entityStatePtr->AliveCounter;
        f = entityPtr->WdgM_SupervisionReferenceCycle - entityPtr->WdgM_ExpectedAliveIndications;
        aliveCalc = nAl + f - nSC;

        /** @req WDGM091 **/
        if ((aliveCalc <= entityPtr->WdgM_MaxMargin) &&
            (aliveCalc >= -entityPtr->WdgM_MinMargin))
        {
          /* Entity alive OK. */
          /** @req WDGM113 **/
          if (entityStatePtr->SupervisionStatus <= WDGM_ALIVE_FAILED)
          {
        	  entityStatePtr->SupervisionStatus = WDGM_ALIVE_OK;
          }
        }
        else
        {
          /** @req WDGM024 **/
          /* Entity alive NOK. */
          entityStatePtr->SupervisionStatus = WDGM_ALIVE_FAILED;
          if (WDGM_ALIVE_FAILED > maxLocal)
          {
            maxLocal = WDGM_ALIVE_FAILED;
          }
        }

        /** @req WDGM097 **/
        if (WDGM_ALIVE_FAILED == entityStatePtr->SupervisionStatus)
        {
          /** @req WDGM125 **/
          /** @req WDGM130 **/
          if (++entityStatePtr->NbrOfFailedRefCycles >= entityPtr->WdgM_FailedSupervisionReferenceCycleTolerance)
          {
        	entityStatePtr->SupervisionStatus = WDGM_ALIVE_EXPIRED;
            if (WDGM_ALIVE_EXPIRED > maxLocal)
            {
              maxLocal = WDGM_ALIVE_EXPIRED;
            }
          }
        }

        /* Reset counters. */
        entityStatePtr->SupervisionCycle = 0;
        entityStatePtr->AliveCounter = 0;
      }
      else
      {
    	  /* No update, but we need to keep maximum status updated. */
    	  if (entityStatePtr->SupervisionStatus > maxLocal)
    	  {
    		  maxLocal = entityStatePtr->SupervisionStatus;
    	  }
      }
    }
  }

  /** @req WDGM075 **/
  /** @req WDGM076 **/
  /** @req WDGM077 **/
  /** @req WDGM100 **/
  /** @req WDGM101 **/
  /** @req WDGM117 **/
  /* Handle the global status. */
  switch (wdgMInternalState.WdgM_GlobalSupervisionStatus)
  {
    case WDGM_ALIVE_OK:
    case WDGM_ALIVE_FAILED:
      if (WDGM_ALIVE_FAILED == maxLocal)
        {
          wdgMInternalState.WdgM_ExpiredSupervisionCycles = 1;
          if (wdgMInternalState.WdgM_ExpiredSupervisionCycles >
    wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_Mode[wdgMInternalState.WdgMActiveMode].WdgM_ExpiredSupervisionCycleTol)
            {
              wdgMInternalState.WdgM_GlobalSupervisionStatus = WDGM_ALIVE_STOPPED;
#if (WDGM_DEM_ALIVE_SUPERVISION_REPORT == STD_ON)
            		  REPORT_TO_DEM(WDGM_E_ALIVE_SUPERVISION)
#endif
            }
          else
            {
              wdgMInternalState.WdgM_GlobalSupervisionStatus = WDGM_ALIVE_FAILED;
            }
        }
      else if (WDGM_ALIVE_EXPIRED == maxLocal)
        {
          wdgMInternalState.WdgM_ExpiredSupervisionCycles = 1;
          if (wdgMInternalState.WdgM_ExpiredSupervisionCycles >
          wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_Mode[wdgMInternalState.WdgMActiveMode].WdgM_ExpiredSupervisionCycleTol)
            {
              wdgMInternalState.WdgM_GlobalSupervisionStatus = WDGM_ALIVE_STOPPED;
#if (WDGM_DEM_ALIVE_SUPERVISION_REPORT == STD_ON)
            		  REPORT_TO_DEM(WDGM_E_ALIVE_SUPERVISION)
#endif
            }
          else
            {
              wdgMInternalState.WdgM_GlobalSupervisionStatus = WDGM_ALIVE_EXPIRED;
            }
        }
      else if (WDGM_ALIVE_OK == maxLocal)
        {
          wdgMInternalState.WdgM_ExpiredSupervisionCycles = 0;
          wdgMInternalState.WdgM_GlobalSupervisionStatus = WDGM_ALIVE_OK;
        }
      else
        {
          /* No more exits from state. Do nothing. */
        }
      break;
    case WDGM_ALIVE_EXPIRED:
      if (++wdgMInternalState.WdgM_ExpiredSupervisionCycles >
      wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_Mode[wdgMInternalState.WdgMActiveMode].WdgM_ExpiredSupervisionCycleTol)
        {
          wdgMInternalState.WdgM_GlobalSupervisionStatus = WDGM_ALIVE_STOPPED;
#if (WDGM_DEM_ALIVE_SUPERVISION_REPORT == STD_ON)
          REPORT_TO_DEM(WDGM_E_ALIVE_SUPERVISION)
#endif
        }
      break;
    case WDGM_ALIVE_DEACTIVATED:
    case WDGM_ALIVE_STOPPED:
      break;
    }
}

/** @req WDGM060 **/
/** @req WDGM061 **/
/** @req WDGM063 **/
/** @req WDGM099 **/
/** @req WDGM159 **/
void WdgM_MainFunction_AliveSupervision (void)
{
	VALIDATE_NO_RETURNVAL((wdgMInternalState.WdgM_ConfigPtr != 0), WDGM_MAINFUNCTION_ALIVESUPERVISION_ID, WDGM_E_NO_INIT);

	const WdgM_ModeConfigType * modeConfigPtr = &wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_Mode[wdgMInternalState.WdgMActiveMode];

	/** @req WDGM189 **/
	if (!modeConfigPtr->WdgM_Activation.WdgM_IsGPTActivated)
	{
		wdgm_Check_AliveSupervision();
	}
}

static boolean WdgM_IsAlive(void)
{
    boolean res;
	/** @req WDGM119 **/
	/** @req WDGM120 **/
	/** @req WDGM121 **/
	/** @req WDGM122 **/
	if (WDGM_ALIVE_STOPPED > wdgMInternalState.WdgM_GlobalSupervisionStatus)
	{
		res = TRUE;
	}
	else
	{
		res = FALSE;
	}
	return res;
}

/** @req WDGM002 **/
/** @req WDGM065 **/
static void wdgm_Trigger (void)
{
  uint8 i;

  /* Update trigger counter. */
  wdgMInternalState.WdgMTriggerCounter++;

  /** @req WDGM041 **/
  /** @req WDGM051 **/
  if ( WdgM_IsAlive() )
  {
	  /* Loop through all managed watchdogs. */
	for (i = 0; i < wdgMInternalState.WdgM_ConfigPtr->WdgM_General->WdgM_NumberOfWatchdogs;i++)
	{
	    /** @req WDGM040 **/
		/** @req WDGM103 **/
		/** @req WDGM109 **/
		/* Time to trig this particular watchdog instance? */
	   if (0 == (wdgMInternalState.WdgMTriggerCounter %
			     wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_Mode[wdgMInternalState.WdgMActiveMode].WdgM_Trigger[i].WdgM_TriggerReferenceCycle))
	   {
		 /** @req WDGM066 **/
	     WdgIf_Trigger(wdgMInternalState.WdgM_ConfigPtr->WdgM_General->WdgM_Watchdog[i].WdgM_DeviceRef->WdgIf_DeviceIndex);
	   }
	}
  }
}

/** @req WDGM160 **/
void WdgM_MainFunction_Trigger (void)
{
	/** @req WDGM068 **/
	VALIDATE_NO_RETURNVAL((wdgMInternalState.WdgM_ConfigPtr != 0), WDGM_MAINFUNCTION_TRIGGER_ID, WDGM_E_NO_INIT);
	const WdgM_ModeConfigType * modeConfigPtr = &wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_Mode[wdgMInternalState.WdgMActiveMode];

	/** @req WDGM189 **/
	if (!modeConfigPtr->WdgM_Activation.WdgM_IsGPTActivated)
	{
		wdgm_Trigger();
	}
}

void WdgM_Cbk_GptNotification (void)
{
	VALIDATE_NO_RETURNVAL((wdgMInternalState.WdgM_ConfigPtr != 0), WDGM_MAINFUNCTION_TRIGGER_ID, WDGM_E_NO_INIT);
	const WdgM_ModeConfigType * modeConfigPtr = &wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_Mode[wdgMInternalState.WdgMActiveMode];

	/** @req WDGM190 **/
	if (modeConfigPtr->WdgM_Activation.WdgM_IsGPTActivated)
	{
		wdgm_Check_AliveSupervision();
		wdgm_Trigger();
	}
}

const WdgM_AliveSupervisionType * WdgM_Arc_GetAliveSupervisionPtr( enum WdgM_Mode mode, enum WdgM_SupervisedEntityId supId )
{

  const WdgM_ModeConfigType * modeConfigPtr = &wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_Mode[mode];

  return modeConfigPtr->WdgM_AliveSupervisionPtr;
}

WdgM_AliveEntityStateType *WdgM_Arc_GetSupervisionPtr( enum WdgM_SupervisedEntityId supId ) {
	 return GET_ENTITY_STATE_PTR(supId);
}




