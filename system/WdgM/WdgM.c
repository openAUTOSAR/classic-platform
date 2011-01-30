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








#include "WdgM.h"
#include "WdgIf.h"
#include "Det.h"

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
struct
{
	const WdgM_ConfigType           *WdgM_ConfigPtr;
	WdgM_AliveSupervisionStatusType WdgM_GlobalSupervisionStatus;
	WdgM_TriggerCounterType         WdgMTriggerCounter;
	WdgM_ModeType                   WdgMActiveMode;
}wdgMInternalState;

/* Helper macros */
#define GET_ENTITY_STATE_PTR(_SEId)       (&(wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_AliveEntityStatePtr)[_SEId])
#define GET_SUPERVISED_ENTITY_PTR(_SEId)  ()
//WdgM_Mode[wdgMInternalState.WdgMActiveMode]
Std_ReturnType WdgM_UpdateAliveCounter (WdgM_SupervisedEntityIdType SEid)
{
  Std_ReturnType ret = E_NOT_OK;
  /** @req WDGM027 **/
  VALIDATE_ENTITY_ID(SEid, WDGM_UPDATEALIVECOUNTER_ID);
  /** @req WDGM028 **/
  VALIDATE((wdgMInternalState.WdgM_ConfigPtr != 0), WDGM_UPDATEALIVECOUNTER_ID, WDGM_E_NO_INIT);
  WdgM_AliveEntityStateType *entityStatePtr = GET_ENTITY_STATE_PTR(SEid);

  /** @req WDGM083 **/
  if (entityStatePtr->ActivationStatus == WDGM_SUPERVISION_ENABLED)
  {
	  /** @req WDGM114 **/
	  if (entityStatePtr->SupervisionStatus < WDGM_ALIVE_EXPIRED)
	  {
		  entityStatePtr->AliveCounter++;
		  ret = E_OK;
	  }
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

  entityStatePtr->ActivationStatus = WDGM_SUPERVISION_ENABLED;
  ret = E_OK;
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
  const WdgM_SupervisedEntityType *supervisedEntityPtr = wdgMInternalState.WdgM_ConfigPtr->WdgM_General->WdgM_SupervisedEntityPtr;
  /** @req WDGM082 **/
  /** @req WDGM174 **/
  /** @req WDGM108 **/
  VALIDATE((supervisedEntityPtr->WdgM_DeactivationAccessEnabled != 0), WDGM_DEACTIVATEALIVESUPERVISION_ID, WDGM_E_DEACTIVATE_NOT_ALLOWED);

  /** @req WDGM114 **/
  if (entityStatePtr->SupervisionStatus < WDGM_ALIVE_EXPIRED)
  {
	  entityStatePtr->ActivationStatus = WDGM_SUPERVISION_DISABLED;
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
Std_ReturnType WdgM_SetMode(WdgM_ModeType Mode)
{
  Std_ReturnType ret = E_NOT_OK;
  /** @req WDGM021 **/
  VALIDATE((wdgMInternalState.WdgM_ConfigPtr != 0), WDGM_SETMODE_ID, WDGM_E_NO_INIT);
  /** @req WDGM020 **/
  VALIDATE(((Mode >= 0) && (Mode < WDGM_NBR_OF_MODES)), WDGM_SETMODE_ID, WDGM_E_PARAM_MODE);

  WdgIf_ModeType mode;
  uint8 i;


  /** @req WDGM145 **/
  if (wdgMInternalState.WdgM_GlobalSupervisionStatus == WDGM_ALIVE_OK)
  {
	  mode = wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_Mode[Mode].WdgM_Trigger->WdgM_WatchdogMode;

	  /* Write new mode to internal state. */
	  wdgMInternalState.WdgMActiveMode = Mode;

	  /* Pass mode to all watchdog instances. */
	  for (i = 0; i < wdgMInternalState.WdgM_ConfigPtr->WdgM_General->WdgM_NumberOfWatchdogs;i++)
	  {
		  /** @req WDGM139 **/
		  if (E_NOT_OK == WdgIf_SetMode (wdgMInternalState.WdgM_ConfigPtr->WdgM_General->WdgM_Watchdog->WdgM_DeviceRef[i].WdgIf_DeviceIndex, mode))
		  {
			  wdgMInternalState.WdgM_GlobalSupervisionStatus = WDGM_ALIVE_STOPPED;
		  }
	  }
	  ret = E_OK;
  }
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

  /** @req WDGM010 **/
  VALIDATE_NO_RETURNVAL((ConfigPtr != 0),WDGM_INIT_ID, WDGM_E_PARAM_CONFIG);

  wdgMInternalState.WdgM_ConfigPtr = ConfigPtr;
  initialMode = wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_InitialMode;
  /** @req WDGM018 **/
  for (SEid = 0; SEid < WDGM_NBR_OF_ALIVE_SIGNALS; SEid++)
  {
	entityStatePtr = GET_ENTITY_STATE_PTR(SEid);
    entityStatePtr->ActivationStatus = wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_Mode[initialMode].WdgM_AliveSupervisionPtr[SEid].WdgM_ActivationStatus;
    entityStatePtr->AliveCounter         = 0;
    entityStatePtr->SupervisionCycle     = 0;
    entityStatePtr->SupervisionStatus    = 0;
    entityStatePtr->NbrOfFailedRefCycles = 0;
  }
  wdgMInternalState.WdgM_GlobalSupervisionStatus = WDGM_ALIVE_OK;
  wdgMInternalState.WdgMActiveMode = wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_InitialMode;
}

/* Non standard API for test purpose.  */
void WdgM_DeInit( void)
{
	wdgMInternalState.WdgM_GlobalSupervisionStatus = 0;
	wdgMInternalState.WdgM_ConfigPtr = 0;
}


/** @req WDGM060 **/
/** @req WDGM061 **/
/** @req WDGM063 **/
/** @req WDGM099 **/
/** @req WDGM159 **/
void WdgM_MainFunction_AliveSupervision (void)
{
  VALIDATE_NO_RETURNVAL((wdgMInternalState.WdgM_ConfigPtr != 0), WDGM_MAINFUNCTION_ALIVESUPERVISION_ID, WDGM_E_NO_INIT);

  WdgM_SupervisedEntityIdType SEid;
  WdgM_AliveEntityStateType *entityStatePtr;
  const WdgM_AliveSupervisionType *entityPtr;
  WdgM_SupervisionCounterType aliveCalc, nSC, nAl, eai;
  WdgM_AliveSupervisionStatusType maxLocal = WDGM_ALIVE_OK;
  static WdgM_SupervisionCounterType expiredSupervisionCycles = 0;

  for (SEid = 0; SEid < wdgMInternalState.WdgM_ConfigPtr->WdgM_General->WdgM_NumberOfSupervisedEntities; SEid++)
  {
	entityStatePtr = GET_ENTITY_STATE_PTR(SEid);
    entityPtr      = &(wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_Mode[wdgMInternalState.WdgMActiveMode].WdgM_AliveSupervisionPtr)[SEid];

    /** @req WDGM083 **/
    if (WDGM_SUPERVISION_ENABLED == entityStatePtr->ActivationStatus)
    {
    	entityStatePtr->SupervisionCycle++;
      /** @req WDGM090 **/
      /* Only perform supervision on the reference cycle. */
      if (entityStatePtr->SupervisionCycle == entityPtr->WdgM_SupervisionReferenceCycle)
      {
        /* Alive algorithm. *
         * n (Al) - n(SC) + EAI == 0 */
        if (entityPtr->WdgM_ExpectedAliveIndications > entityPtr->WdgM_SupervisionReferenceCycle)
        {
          /* Scenario A */
          eai = -entityPtr->WdgM_ExpectedAliveIndications + 1;

        }
        else
        {
          /* Scenario B */
          eai = entityPtr->WdgM_SupervisionReferenceCycle - 1;
        }
        nSC = entityStatePtr->SupervisionCycle;
        nAl = entityStatePtr->AliveCounter;
        aliveCalc = nAl - nSC + eai;

        /** @req WDGM091 **/
        if ((aliveCalc <= entityPtr->WdgM_MaxMargin) &&
            (aliveCalc >= entityPtr->WdgM_MinMargin))
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
          if (entityStatePtr->NbrOfFailedRefCycles > entityPtr->WdgM_FailedSupervisionReferenceCycleTolerance)
          {
        	  entityStatePtr->SupervisionStatus = WDGM_ALIVE_EXPIRED;
            if (WDGM_ALIVE_EXPIRED > maxLocal)
            {
              maxLocal = WDGM_ALIVE_EXPIRED;
            }
          }
          else
          {
        	  entityStatePtr->NbrOfFailedRefCycles++;
          }
        }

        /* Reset counters. */
        entityStatePtr->SupervisionCycle = 0;
        entityStatePtr->AliveCounter = 0;
      }
    }
  }

  /** @req WDGM075 **/
  /** @req WDGM076 **/
  /** @req WDGM077 **/
  /** @req WDGM100 **/
  /** @req WDGM101 **/
  /* Try to heal global status. */
  if (WDGM_ALIVE_EXPIRED != wdgMInternalState.WdgM_GlobalSupervisionStatus)
  {
	  wdgMInternalState.WdgM_GlobalSupervisionStatus = maxLocal;
  }
  else
  {
	  wdgMInternalState.WdgM_GlobalSupervisionStatus = WDGM_ALIVE_EXPIRED;
  }

  if (WDGM_ALIVE_EXPIRED == wdgMInternalState.WdgM_GlobalSupervisionStatus)
  {
    expiredSupervisionCycles++;
  }

  /** @req WDGM117 **/
  if (expiredSupervisionCycles >= wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_Mode[wdgMInternalState.WdgMActiveMode].WdgM_ExpiredSupervisionCycleTol)
  {
	  wdgMInternalState.WdgM_GlobalSupervisionStatus = WDGM_ALIVE_STOPPED;
  }
}

boolean WdgM_IsAlive(void)
{
  /** @req WDGM119 **/
  /** @req WDGM120 **/
  /** @req WDGM121 **/
  /** @req WDGM122 **/
  if (WDGM_ALIVE_STOPPED > wdgMInternalState.WdgM_GlobalSupervisionStatus)
  {
    return (TRUE);
  }
  else
  {
    return (FALSE);
  }
}

/** @req WDGM002 **/
/** @req WDGM065 **/
/** @req WDGM160 **/
void WdgM_MainFunction_Trigger (void)
{
  uint8 i;
  /** @req WDGM068 **/
  VALIDATE_NO_RETURNVAL((wdgMInternalState.WdgM_ConfigPtr != 0), WDGM_MAINFUNCTION_TRIGGER_ID, WDGM_E_NO_INIT);

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


