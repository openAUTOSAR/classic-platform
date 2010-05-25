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









#ifndef WDGM_CFG_H_
#define WDGM_CFG_H_

#include "Std_Types.h"
#include "WdgM_Lcfg.h"

typedef enum
{
  WDBG_ALIVE_OK,
  WDBG_ALIVE_FAILED,
  WDBG_ALIVE_EXPIRED,
  WDBG_ALIVE_STOPPED,
  WDBG_ALIVE_DEACTIVATED,
}WdgM_SupervisedStatusType;

typedef enum
{
  WDBG_SUPERVISION_DISABLED,
  WDBG_SUPERVISION_ENABLED
}WdgM_ActivationStatusType;

typedef int16_t WdgM_SupervisionCounterType ;

typedef struct
{
  WdgM_SupervisionCounterType AliveCounter;
  WdgM_SupervisionCounterType SupervisionCycle;
  WdgM_SupervisedStatusType SupervisionStatus;
  WdgM_SupervisionCounterType NbrOfFailedRefCycles;
  WdgM_ActivationStatusType ActivationStatus;
}Wdgm_SupervisionType;

typedef struct
{
  const WdgM_SupervisedEntityIdType WdgM_SupervisedEntityID;
  const WdgM_ActivationStatusType WdgM_ActivationStatus;
  const WdgM_SupervisionCounterType WdgM_ExpectedAliveIndications;
  const WdgM_SupervisionCounterType WdgM_SupervisionReferenceCycle;
  const WdgM_SupervisionCounterType WdgM_FailedSupervisionReferenceCycleTolerance;
  const WdgM_SupervisionCounterType WdgM_MinMargin;
  const WdgM_SupervisionCounterType WdgM_MaxMargin;
}WdgM_SupervisedEntityType;

typedef struct
{
  uint16 WdgM_SupervisionCycle;
  uint16 WdgM_NumberOfSupervisedEntities;
  uint16 WdgM_ExpiredSupervisionCycleTolerance;
  const WdgM_SupervisedEntityType *WdgM_SupervisedEntityPtr;
  Wdgm_SupervisionType *Wdgm_SupervisionPtr;
}WdgM_ConfigType;

extern const WdgM_ConfigType WdgMAliveSupervision;

#endif /* WDGM_CFG_H_ */
