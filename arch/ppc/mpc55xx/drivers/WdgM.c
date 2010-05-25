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
#include "Mcu.h"

const WdgM_ConfigType *wdgMConfigPtr;
static WdgM_SupervisedStatusType WdgM_GlobalSupervisionStatus = WDBG_ALIVE_OK;

Std_ReturnType WdgM_UpdateAliveCounter (WdgM_SupervisedEntityIdType SEid)
{
  Wdgm_SupervisionType *supervisionPtr = &(wdgMConfigPtr->Wdgm_SupervisionPtr)[SEid];

  if (supervisionPtr->ActivationStatus == WDBG_SUPERVISION_ENABLED)
  {
    supervisionPtr->AliveCounter++;
  }
  return (E_OK);
}

Std_ReturnType WdgM_ActivateAliveSupervision (WdgM_SupervisedEntityIdType SEid)
{
  Wdgm_SupervisionType *supervisionPtr = &(wdgMConfigPtr->Wdgm_SupervisionPtr)[SEid];

  supervisionPtr->ActivationStatus = WDBG_SUPERVISION_ENABLED;

  return (E_OK);
}

Std_ReturnType WdgM_DeactivateAliveSupervision (WdgM_SupervisedEntityIdType SEid)
{
  Wdgm_SupervisionType *supervisionPtr = &(wdgMConfigPtr->Wdgm_SupervisionPtr)[SEid];

  supervisionPtr->ActivationStatus = WDBG_SUPERVISION_DISABLED;
  return (E_OK);
}

void WdgM_Init(const WdgM_ConfigType *ConfigPtr)
{
  WdgM_SupervisedEntityIdType SEid;
  Wdgm_SupervisionType *supervisionPtr;
  WdgM_SupervisedEntityType* supervisedEntityPtr;

  for (SEid = 0; SEid < WDBG_NBR_OF_ALIVE_SIGNALS; SEid++)
  {
    supervisionPtr = &(ConfigPtr->Wdgm_SupervisionPtr)[SEid];
    supervisedEntityPtr = (WdgM_SupervisedEntityType*)&(ConfigPtr->WdgM_SupervisedEntityPtr)[SEid];
    supervisionPtr->ActivationStatus = supervisedEntityPtr->WdgM_ActivationStatus;
  }
  wdgMConfigPtr = ConfigPtr;

}

void WdgM_MainFunction_AliveSupervision (void)
{
  WdgM_SupervisedEntityIdType SEid;
  Wdgm_SupervisionType *supervisionPtr;
  const WdgM_SupervisedEntityType *entityPtr;
  WdgM_SupervisionCounterType aliveCalc, nSC, nAl, eai;
  WdgM_SupervisedStatusType maxLocal = WDBG_ALIVE_OK;
  static WdgM_SupervisionCounterType expiredSupervisionCycles = 0;

  for (SEid = 0; SEid < WDBG_NBR_OF_ALIVE_SIGNALS; SEid++)
  {
    supervisionPtr = &(wdgMConfigPtr->Wdgm_SupervisionPtr)[SEid];
    entityPtr      = &(wdgMConfigPtr->WdgM_SupervisedEntityPtr)[SEid];

    if (WDBG_SUPERVISION_ENABLED == supervisionPtr->ActivationStatus)
    {
      supervisionPtr->SupervisionCycle++;
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

        if ((aliveCalc <= entityPtr->WdgM_MaxMargin) &&
            (aliveCalc >= -entityPtr->WdgM_MinMargin))
        {
          /* Entity alive OK. */
          supervisionPtr->SupervisionStatus = WDBG_ALIVE_OK;
        }
        else
        {
          /* Entity alive NOK. */
          supervisionPtr->SupervisionStatus = WDBG_ALIVE_FAILED;
          if (WDBG_ALIVE_FAILED > maxLocal)
          {
            maxLocal = WDBG_ALIVE_FAILED;
          }
        }

        if (WDBG_ALIVE_FAILED == supervisionPtr->SupervisionStatus)
        {
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

  /* Try to heal global status. */
  if (WDBG_ALIVE_EXPIRED != WdgM_GlobalSupervisionStatus)
  {
    WdgM_GlobalSupervisionStatus = maxLocal;
  }
  else
  {
    WdgM_GlobalSupervisionStatus = WDBG_ALIVE_EXPIRED;
  }

  if (WDBG_ALIVE_EXPIRED == WdgM_GlobalSupervisionStatus)
  {
    expiredSupervisionCycles++;
  }

  if (expiredSupervisionCycles >= wdgMConfigPtr->WdgM_ExpiredSupervisionCycleTolerance)
  {
    WdgM_GlobalSupervisionStatus = WDBG_ALIVE_STOPPED;
  }
}

boolean WdgM_IsAlive(void)
{

  if ( WDBG_ALIVE_STOPPED > WdgM_GlobalSupervisionStatus )
  {
    return (TRUE);
  }
  else
  {
    return (FALSE);
  }
}

void WdgM_MainFunction_Trigger (void)
{
  if ( WdgM_IsAlive() )
  {
    KickWatchdog();
  }
}


