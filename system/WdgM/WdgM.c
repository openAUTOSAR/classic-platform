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
	if(_SEId >= WDBG_NBR_OF_ALIVE_SIGNALS) {	\
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
}wdgMInternalState;

Std_ReturnType WdgM_UpdateAliveCounter (WdgM_SupervisedEntityIdType SEid)
{
  Std_ReturnType ret = E_NOT_OK;
  VALIDATE_ENTITY_ID(SEid, WDGM_UPDATEALIVECOUNTER_ID);
  WdgM_SupervisionType *supervisionPtr = &(wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_SupervisionPtr)[SEid];

  /** @req WDGM083 **/
  if (supervisionPtr->ActivationStatus == WDBG_SUPERVISION_ENABLED)
  {
	  /** @req WDGM114 **/
	  if (supervisionPtr->SupervisionStatus < WDBG_ALIVE_EXPIRED)
	  {
		  supervisionPtr->AliveCounter++;
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
  VALIDATE_ENTITY_ID(SEid, WDGM_ACTIVATEALIVESUPERVISION_ID);
  WdgM_SupervisionType *supervisionPtr = &(wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_SupervisionPtr)[SEid];

  supervisionPtr->ActivationStatus = WDBG_SUPERVISION_ENABLED;
  ret = E_OK;
  return (ret);
}

/** @req WDGM079 **/
/** @req WDGM157 **/
Std_ReturnType WdgM_DeactivateAliveSupervision (WdgM_SupervisedEntityIdType SEid)
{
  Std_ReturnType ret;
  VALIDATE_ENTITY_ID(SEid, WDGM_DEACTIVATEALIVESUPERVISION_ID);
  WdgM_SupervisionType *supervisionPtr = &(wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_SupervisionPtr)[SEid];

  /** @req WDGM114 **/
  if (supervisionPtr->SupervisionStatus < WDBG_ALIVE_EXPIRED)
  {
	  supervisionPtr->ActivationStatus = WDBG_SUPERVISION_DISABLED;
  }
  ret = E_OK;
  return (ret);
}

/** @req WDGM169 **/
Std_ReturnType WdgM_GetAliveSupervisionStatus (WdgM_SupervisedEntityIdType SEid, WdgM_AliveSupervisionStatusType *Status)
{
	Std_ReturnType ret;
	VALIDATE_ENTITY_ID(SEid, WDGM_GETALIVESUPERVISION_ID);
	WdgM_SupervisionType *supervisionPtr = &(wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_SupervisionPtr)[SEid];
	*Status = supervisionPtr->SupervisionStatus;
	ret = E_OK;
	return ret;
}

/** @req WDGM175 **/
Std_ReturnType WdgM_GetGlobalStatus (WdgM_AliveSupervisionStatusType *Status)
{
	Std_ReturnType ret = E_NOT_OK;
	VALIDATE((wdgMInternalState.WdgM_ConfigPtr != 0), WDGM_GETGLOBALSTATUS_ID, WDGM_E_NO_INIT);
	*Status = wdgMInternalState.WdgM_GlobalSupervisionStatus;
	ret = E_OK;
	return ret;
}

/** @req WDGM154 **/
Std_ReturnType WdgM_SetMode(WdgM_ModeType Mode)
{
  Std_ReturnType ret = E_NOT_OK;
  VALIDATE((wdgMInternalState.WdgM_ConfigPtr != 0), WDGM_SETMODE_ID, WDGM_E_NO_INIT);

  WdgIf_ModeType mode;
  uint8 i;
  uint8 modeFound = 0;

  switch (Mode)
  {
  case WDGM_OFF_MODE:
	  mode = WDGIF_OFF_MODE;
	  modeFound = 1;
	  break;
  case WDGM_SLOW_MODE:
	  mode = WDGIF_SLOW_MODE;
	  modeFound = 1;
	  break;
  case WDGM_FAST_MODE:
	  mode = WDGIF_FAST_MODE;
	  modeFound = 1;
	break;

  }
  if (modeFound)
  {
	  for (i = 0; i < wdgMInternalState.WdgM_ConfigPtr->WdgM_General->WdgM_NumberOfWatchdogs;i++)
	  {
		  WdgIf_SetMode (wdgMInternalState.WdgM_ConfigPtr->WdgM_General->WdgM_Watchdog->WdgM_DeviceRef[i].WdgIf_DeviceIndex, mode);
	  }
	  ret = E_OK;
  }

  return (ret);
}

/** @req WDGM168 **/
//Std_ReturnType WdgM_GetMode(WdgM_ModeType *Mode)
//{
//
//}

/** @req WDGM151 **/
void WdgM_Init(const WdgM_ConfigType *ConfigPtr)
{
  WdgM_SupervisedEntityIdType SEid;
  WdgM_SupervisionType *supervisionPtr;
  WdgM_SupervisedEntityType* supervisedEntityPtr;

  /** @req WDGM010 **/
  VALIDATE_NO_RETURNVAL((ConfigPtr != 0),WDGM_INIT_ID, WDGM_E_PARAM_CONFIG);

  /** @req WDGM018 **/
  for (SEid = 0; SEid < WDBG_NBR_OF_ALIVE_SIGNALS; SEid++)
  {
    supervisionPtr = &(ConfigPtr->WdgM_ConfigSet->WdgM_SupervisionPtr)[SEid];
    supervisedEntityPtr = (WdgM_SupervisedEntityType*)&(ConfigPtr->WdgM_ConfigSet->WdgM_SupervisedEntityPtr)[SEid];
    supervisionPtr->ActivationStatus = supervisedEntityPtr->WdgM_ActivationStatus;
    supervisionPtr->AliveCounter         = 0;
    supervisionPtr->SupervisionCycle     = 0;
    supervisionPtr->SupervisionStatus    = 0;
    supervisionPtr->NbrOfFailedRefCycles = 0;
  }
  wdgMInternalState.WdgM_GlobalSupervisionStatus = WDBG_ALIVE_OK;
  wdgMInternalState.WdgM_ConfigPtr = ConfigPtr;
}

/** @req WDGM060 **/
/** @req WDGM061 **/
/** @req WDGM063 **/
/** @req WDGM099 **/
/** @req WDGM159 **/
void WdgM_MainFunction_AliveSupervision (void)
{
  WdgM_SupervisedEntityIdType SEid;
  WdgM_SupervisionType *supervisionPtr;
  const WdgM_SupervisedEntityType *entityPtr;
  WdgM_SupervisionCounterType aliveCalc, nSC, nAl, eai;
  WdgM_AliveSupervisionStatusType maxLocal = WDBG_ALIVE_OK;
  static WdgM_SupervisionCounterType expiredSupervisionCycles = 0;

  for (SEid = 0; SEid < wdgMInternalState.WdgM_ConfigPtr->WdgM_General->WdgM_NumberOfSupervisedEntities; SEid++)
  {
    supervisionPtr = &(wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_SupervisionPtr)[SEid];
    entityPtr      = &(wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_SupervisedEntityPtr)[SEid];

    /** @req WDGM083 **/
    if (WDBG_SUPERVISION_ENABLED == supervisionPtr->ActivationStatus)
    {
      supervisionPtr->SupervisionCycle++;
      /** @req WDGM090 **/
      /* Only perform supervision on the reference cycle. */
      if (supervisionPtr->SupervisionCycle == entityPtr->WdgM_SupervisionReferenceCycle)
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
        nSC = supervisionPtr->SupervisionCycle;
        nAl = supervisionPtr->AliveCounter;
        aliveCalc = nAl - nSC + eai;
        supervisionPtr->test_aliveCalc = aliveCalc;

        /** @req WDGM091 **/
        if ((aliveCalc <= entityPtr->WdgM_MaxMargin) &&
            (aliveCalc >= -entityPtr->WdgM_MinMargin))
        {
          /* Entity alive OK. */
          /** @req WDGM113 **/
          if (supervisionPtr->SupervisionStatus <= WDBG_ALIVE_FAILED)
          {
            supervisionPtr->SupervisionStatus = WDBG_ALIVE_OK;
          }
        }
        else
        {
          /** @req WDGM024 **/
          /* Entity alive NOK. */
          supervisionPtr->SupervisionStatus = WDBG_ALIVE_FAILED;
          if (WDBG_ALIVE_FAILED > maxLocal)
          {
            maxLocal = WDBG_ALIVE_FAILED;
          }
        }

        /** @req WDGM097 **/
        if (WDBG_ALIVE_FAILED == supervisionPtr->SupervisionStatus)
        {
          /** @req WDGM125 **/
          /** @req WDGM130 **/
          if (supervisionPtr->NbrOfFailedRefCycles > entityPtr->WdgM_FailedSupervisionReferenceCycleTolerance)
          {
            supervisionPtr->SupervisionStatus = WDBG_ALIVE_EXPIRED;
            if (WDBG_ALIVE_EXPIRED > maxLocal)
            {
              maxLocal = WDBG_ALIVE_EXPIRED;
            }
          }
          else
          {
            supervisionPtr->NbrOfFailedRefCycles++;
          }
        }

        /* Reset counters. */
        supervisionPtr->SupervisionCycle = 0;
        supervisionPtr->AliveCounter = 0;
      }
    }
  }

  /** @req WDGM075 **/
  /** @req WDGM076 **/
  /** @req WDGM077 **/
  /** @req WDGM100 **/
  /** @req WDGM101 **/
  /* Try to heal global status. */
  if (WDBG_ALIVE_EXPIRED != wdgMInternalState.WdgM_GlobalSupervisionStatus)
  {
	  wdgMInternalState.WdgM_GlobalSupervisionStatus = maxLocal;
  }
  else
  {
	  wdgMInternalState.WdgM_GlobalSupervisionStatus = WDBG_ALIVE_EXPIRED;
  }

  if (WDBG_ALIVE_EXPIRED == wdgMInternalState.WdgM_GlobalSupervisionStatus)
  {
    expiredSupervisionCycles++;
  }

  /** @req WDGM117 **/
  if (expiredSupervisionCycles >= wdgMInternalState.WdgM_ConfigPtr->WdgM_ExpiredSupervisionCycleTolerance)
  {
	  wdgMInternalState.WdgM_GlobalSupervisionStatus = WDBG_ALIVE_STOPPED;
  }
}

boolean WdgM_IsAlive(void)
{
  /** @req WDGM119 **/
  /** @req WDGM120 **/
  /** @req WDGM121 **/
  /** @req WDGM122 **/
  if (WDBG_ALIVE_STOPPED > wdgMInternalState.WdgM_GlobalSupervisionStatus)
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
			   wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_Trigger[i].WdgM_TriggerReferenceCycle))
	   {
		 /** @req WDGM066 **/
	     WdgIf_Trigger(wdgMInternalState.WdgM_ConfigPtr->WdgM_ConfigSet->WdgM_Trigger[i].WdgM_WatchdogRef);
	   }
	}
  }
}


