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








#include "WdgM_Cfg.h"

Wdgm_SupervisionType Wdgm_Supervision[WDBG_NBR_OF_ALIVE_SIGNALS];


const WdgM_SupervisedEntityType WdgM_SupervisedEntity [WDBG_NBR_OF_ALIVE_SIGNALS] =
{
  {
    .WdgM_SupervisedEntityID = WDBG_ALIVE_LOOP_BLINK_COMPONENT,
    .WdgM_ActivationStatus = WDBG_SUPERVISION_DISABLED,
    .WdgM_ExpectedAliveIndications = 4,
    .WdgM_SupervisionReferenceCycle = 1,
    .WdgM_FailedSupervisionReferenceCycleTolerance = 1,
    .WdgM_MinMargin = 1,
    .WdgM_MaxMargin = 1
  },
};

const WdgM_ConfigType WdgMAliveSupervision =
{
  .WdgM_SupervisionCycle = 1,
  .WdgM_NumberOfSupervisedEntities = WDBG_NBR_OF_ALIVE_SIGNALS,
  .WdgM_ExpiredSupervisionCycleTolerance = 1,
  .WdgM_SupervisedEntityPtr = WdgM_SupervisedEntity,
  .Wdgm_SupervisionPtr = Wdgm_Supervision
};


