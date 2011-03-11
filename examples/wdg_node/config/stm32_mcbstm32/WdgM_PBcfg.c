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


WdgM_AliveEntityStateType WdgM_Supervision[WDGM_NBR_OF_ALIVE_SIGNALS];

const WdgM_SupervisedEntityType WdgM_SupervisedEntity [WDGM_NBR_OF_ALIVE_SIGNALS] =
{
  {
     .WdgM_DeactivationAccessEnabled = TRUE,
     .WdgM_SupervisedEntityID        = WDGM_ALIVE_ETASK1,
  },
  {
     .WdgM_DeactivationAccessEnabled = TRUE,
     .WdgM_SupervisedEntityID        = WDGM_ALIVE_ETASK2,
  },
  {
     .WdgM_DeactivationAccessEnabled = TRUE,
     .WdgM_SupervisedEntityID        = WDGM_ALIVE_BTASK3,
  },
};


const WdgM_AliveSupervisionType WdgM_AliveSupervisionNarrow [WDGM_NBR_OF_ALIVE_SIGNALS] =
{
  {
	.WdgM_ActivationActivated = TRUE,
    .WdgM_AliveSupervisionConfigID = WDGM_ALIVE_ETASK1,   // TODO Is this really what we should do??
    .WdgM_ExpectedAliveIndications = 2,
    .WdgM_SupervisionReferenceCycle = 76,
    .WdgM_FailedSupervisionReferenceCycleTolerance = 2,
    .WdgM_MinMargin = 1,
    .WdgM_MaxMargin = 0,
    .WdgM_SupervisedEntityRef = &WdgM_SupervisedEntity[WDGM_ALIVE_ETASK1],
  },
  {
	.WdgM_ActivationActivated = TRUE,
    .WdgM_AliveSupervisionConfigID = WDGM_ALIVE_ETASK2,   // TODO Is this really what we should do??
    .WdgM_ExpectedAliveIndications = 2,
    .WdgM_SupervisionReferenceCycle = 76,
    .WdgM_FailedSupervisionReferenceCycleTolerance = 2,
    .WdgM_MinMargin = 1,
    .WdgM_MaxMargin = 0,
    .WdgM_SupervisedEntityRef = &WdgM_SupervisedEntity[WDGM_ALIVE_ETASK2],
  },
  {
	.WdgM_ActivationActivated = TRUE,
    .WdgM_AliveSupervisionConfigID = WDGM_ALIVE_BTASK3,   // TODO Is this really what we should do??
    .WdgM_ExpectedAliveIndications = 2,
    .WdgM_SupervisionReferenceCycle = 76,
    .WdgM_FailedSupervisionReferenceCycleTolerance = 2,
    .WdgM_MinMargin = 1,
    .WdgM_MaxMargin = 0,
    .WdgM_SupervisedEntityRef = &WdgM_SupervisedEntity[WDGM_ALIVE_BTASK3],
  },
};

// Much wider tolerance..
const WdgM_AliveSupervisionType WdgM_AliveSupervisionWide [WDGM_NBR_OF_ALIVE_SIGNALS] =
{
  {
	.WdgM_ActivationActivated = TRUE,
    .WdgM_AliveSupervisionConfigID = WDGM_ALIVE_ETASK1,   // TODO Is this really what we should do??
    .WdgM_ExpectedAliveIndications = 2,
    .WdgM_SupervisionReferenceCycle = 76,
    .WdgM_FailedSupervisionReferenceCycleTolerance = 10,
    .WdgM_MinMargin = 10,
    .WdgM_MaxMargin = 10,
    .WdgM_SupervisedEntityRef = &WdgM_SupervisedEntity[WDGM_ALIVE_ETASK1],
  },
  {
	.WdgM_ActivationActivated = TRUE,
    .WdgM_AliveSupervisionConfigID = WDGM_ALIVE_ETASK2,   // TODO Is this really what we should do??
    .WdgM_ExpectedAliveIndications = 2,
    .WdgM_SupervisionReferenceCycle = 76,
    .WdgM_FailedSupervisionReferenceCycleTolerance = 10,
    .WdgM_MinMargin = 10,
    .WdgM_MaxMargin = 10,
    .WdgM_SupervisedEntityRef = &WdgM_SupervisedEntity[WDGM_ALIVE_ETASK2],
  },
  {
	.WdgM_ActivationActivated = TRUE,
    .WdgM_AliveSupervisionConfigID = WDGM_ALIVE_BTASK3,   // TODO Is this really what we should do??
    .WdgM_ExpectedAliveIndications = 2,
    .WdgM_SupervisionReferenceCycle = 76,
    .WdgM_FailedSupervisionReferenceCycleTolerance = 10,
    .WdgM_MinMargin = 10,
    .WdgM_MaxMargin = 10,
    .WdgM_SupervisedEntityRef = &WdgM_SupervisedEntity[WDGM_ALIVE_BTASK3],
  },
};

const WdgM_WatchdogType WdgMWatchdog[] =
{
	{
		.WdgM_WatchdogName = "STM32 Windowed Watchdog",
		.WdgM_DeviceRef    = &WdgIfDevice[WDGM_TRIGGER_INSTANCE0],
	},
	{
		.WdgM_WatchdogName = "STM32 Independant Watchdog",
		.WdgM_DeviceRef    = &WdgIfDevice[WDGM_TRIGGER_INSTANCE1],
	}
};

/** @req WDGMXXX **/
const WdgM_ConfigSetType WdgMConfigSet =
{
    .WdgM_AliveEntityStatePtr = WdgM_Supervision,
	.WdgM_InitialMode = WDGM_GPT_MODE,
	.WdgM_Mode =
	{
		{
			.WdgM_ExpiredSupervisionCycleTol = 5,
			.WdgM_ModeId                     = WDGM_WIDE_MODE,
			.WdgM_Activation =
			{
				.WdgM_IsGPTActivated = FALSE,
				.WdgM_ActivationSchM = {.WdgM_SupervisionCycle = 0.001, .WdgM_TriggerCycle = 0.001},
				.WdgM_ActivationGPT = {0,0},
			},
			.WdgM_AliveSupervisionPtr        = WdgM_AliveSupervisionWide,
			.WdgM_Trigger                    =
			{
				{
					.WdgM_TriggerReferenceCycle = 1,
					.WdgM_WatchdogMode = WDGIF_SLOW_MODE,
					.WdgM_WatchdogRef = &WdgMWatchdog[WDGM_TRIGGER_INSTANCE0],
				},
				{
					.WdgM_TriggerReferenceCycle = 20,
					.WdgM_WatchdogMode = WDGIF_SLOW_MODE,
					.WdgM_WatchdogRef = &WdgMWatchdog[WDGM_TRIGGER_INSTANCE1],
				},
			},
		},
		{
			.WdgM_ExpiredSupervisionCycleTol = 1,
			.WdgM_ModeId                     = WDGM_NARROW_MODE,
			.WdgM_Activation =
			{
				.WdgM_IsGPTActivated = FALSE,
				.WdgM_ActivationSchM = {.WdgM_SupervisionCycle = 0.001, .WdgM_TriggerCycle = 0.001},
				.WdgM_ActivationGPT = {0,0},
			},
			.WdgM_AliveSupervisionPtr        = WdgM_AliveSupervisionNarrow,
			.WdgM_Trigger                    =
			{
				{
					.WdgM_TriggerReferenceCycle = 1,
					.WdgM_WatchdogMode = WDGIF_FAST_MODE,
					.WdgM_WatchdogRef = &WdgMWatchdog[WDGM_TRIGGER_INSTANCE0],
				},
				{
					.WdgM_TriggerReferenceCycle = 10,
					.WdgM_WatchdogMode = WDGIF_FAST_MODE,
					.WdgM_WatchdogRef = &WdgMWatchdog[WDGM_TRIGGER_INSTANCE1],
				},
			},
		},
		{
			.WdgM_ExpiredSupervisionCycleTol = 1,
			.WdgM_ModeId                     = WDGM_OFF_MODE,
			.WdgM_Activation =
			{
				.WdgM_IsGPTActivated = FALSE,
				.WdgM_ActivationSchM = {.WdgM_SupervisionCycle = 0.001, .WdgM_TriggerCycle = 0.001},
				.WdgM_ActivationGPT = {0,0},
			},
			.WdgM_AliveSupervisionPtr        = WdgM_AliveSupervisionWide,
			.WdgM_Trigger                    =
			{
				{
					.WdgM_TriggerReferenceCycle = 1,
					.WdgM_WatchdogMode = WDGIF_OFF_MODE,
					.WdgM_WatchdogRef = &WdgMWatchdog[WDGM_TRIGGER_INSTANCE0],
				},
				{
					.WdgM_TriggerReferenceCycle = 1,
					.WdgM_WatchdogMode = WDGIF_OFF_MODE,
					.WdgM_WatchdogRef = &WdgMWatchdog[WDGM_TRIGGER_INSTANCE1],
				},
			},
	   },
		{
			.WdgM_ExpiredSupervisionCycleTol = 1,
			.WdgM_ModeId                     = WDGM_GPT_MODE,
			.WdgM_Activation =
			{
				.WdgM_IsGPTActivated = TRUE,
				.WdgM_ActivationSchM = {.WdgM_SupervisionCycle = 0.001, .WdgM_TriggerCycle = 0.001},
				.WdgM_ActivationGPT = {.WdgM_GptCycle = 5000, .WdgM_GptChannelRef = GPT_CHANNEL_TIM_3},
			},
			.WdgM_AliveSupervisionPtr        = WdgM_AliveSupervisionWide,
			.WdgM_Trigger                    =
			{
				{
					.WdgM_TriggerReferenceCycle = 1,
					.WdgM_WatchdogMode = WDGIF_SLOW_MODE,
					.WdgM_WatchdogRef = &WdgMWatchdog[WDGM_TRIGGER_INSTANCE0],
				},
				{
					.WdgM_TriggerReferenceCycle = 10,
					.WdgM_WatchdogMode = WDGIF_SLOW_MODE,
					.WdgM_WatchdogRef = &WdgMWatchdog[WDGM_TRIGGER_INSTANCE1],
				},
			},
	   },
	}
};


const WdgM_GeneralType WdgMGeneral =
{
	.WdgM_NumberOfSupervisedEntities = WDGM_NBR_OF_ALIVE_SIGNALS,
	.WdgM_NumberOfWatchdogs          = sizeof(WdgMWatchdog)/sizeof(WdgMWatchdog[0]),
	//.WdgM_SupervisedEntityPtr;     = ,
	.WdgM_SupervisedEntityPtr        = WdgM_SupervisedEntity,
	.WdgM_Watchdog                   = WdgMWatchdog,
};

const WdgM_ConfigType WdgMConfig =
{
	.WdgM_General = &WdgMGeneral,
	.WdgM_ConfigSet = &WdgMConfigSet,
};
