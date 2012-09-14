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

#ifndef WDGM_CONFIGTYPES_H_
#define WDGM_CONFIGTYPES_H_


typedef enum
{
  WDGM_ALIVE_OK,
  WDGM_ALIVE_FAILED,
  WDGM_ALIVE_EXPIRED,
  WDGM_ALIVE_STOPPED,
  WDGM_ALIVE_DEACTIVATED
}WdgM_AliveSupervisionStatusType;




typedef enum
{
  WDGM_SUPERVISION_DISABLED,
  WDGM_SUPERVISION_ENABLED
}WdgM_ActivationStatusType;

typedef int16_t WdgM_SupervisionCounterType ;
typedef uint16_t WdgM_TriggerCounterType ;

typedef struct
{
  WdgM_SupervisionCounterType     AliveCounter;
  WdgM_SupervisionCounterType     SupervisionCycle;
  WdgM_AliveSupervisionStatusType SupervisionStatus;
  WdgM_SupervisionCounterType     NbrOfFailedRefCycles;
}WdgM_AliveEntityStateType;

typedef struct
{
  /** @req WDGM093 **/
  const boolean                     WdgM_DeactivationAccessEnabled;
  const WdgM_SupervisedEntityIdType WdgM_SupervisedEntityID;
}WdgM_SupervisedEntityType;

/** @req WDGM046 **/
/** @req WDGM090 **/
/** @req WDGM091 **/
/** @req WDGM095 **/
/** @req WDGM096 **/
typedef struct
{
  const boolean                         WdgM_ActivationActivated;
  const WdgM_SupervisedEntityIdType     WdgM_AliveSupervisionConfigID;
  const WdgM_SupervisionCounterType     WdgM_ExpectedAliveIndications;
  const WdgM_SupervisionCounterType     WdgM_SupervisionReferenceCycle;
  const WdgM_SupervisionCounterType     WdgM_FailedSupervisionReferenceCycleTolerance;
  const WdgM_SupervisionCounterType     WdgM_MinMargin;
  const WdgM_SupervisionCounterType     WdgM_MaxMargin;
  const WdgM_SupervisedEntityType       *WdgM_SupervisedEntityRef;
}WdgM_AliveSupervisionType;

typedef struct
{
	const char_t           *WdgM_WatchdogName;
	const WdgIf_DeviceType *WdgM_DeviceRef;
}WdgM_WatchdogType;

/** @req WDGM002 **/
/** @req WDGM003 **/
typedef struct
{
	const uint16                     WdgM_NumberOfSupervisedEntities;
	const uint16                     WdgM_NumberOfWatchdogs;
	const WdgM_SupervisedEntityType  *WdgM_SupervisedEntityPtr;
    const WdgM_WatchdogType          *WdgM_Watchdog;
}WdgM_GeneralType;

/** @req WDGM116 **/
typedef struct
{
	const uint16                     WdgM_TriggerReferenceCycle;
	const WdgIf_ModeType             WdgM_WatchdogMode;
	const WdgM_WatchdogType          *WdgM_WatchdogRef;
}WdgM_TriggerType;

typedef struct
{
	const float32                    WdgM_SupervisionCycle;
	const float32                    WdgM_TriggerCycle;
}WdgM_ActivationSchMType;

#if (WDGM_GPT_USED == STD_ON)
typedef struct
{
	const uint32                     WdgM_GptCycle;
	const Gpt_ChannelType            WdgM_GptChannelRef;
}WdgM_ActivationGPTType;
#endif

typedef struct
{
	const boolean                    WdgM_IsGPTActivated;
	const WdgM_ActivationSchMType    WdgM_ActivationSchM;
#if (WDGM_GPT_USED == STD_ON)
	const WdgM_ActivationGPTType     WdgM_ActivationGPT;
#endif
}WdgM_ActivationType;

typedef struct
{
	const WdgM_SupervisionCounterType WdgM_ExpiredSupervisionCycleTol;
	const WdgM_ModeType               WdgM_ModeId;
	const WdgM_ActivationType         WdgM_Activation;
	const WdgM_AliveSupervisionType   *WdgM_AliveSupervisionPtr;
	const WdgM_TriggerType            WdgM_Trigger[WDGM_NBR_OF_WATCHDOGS];
}WdgM_ModeConfigType;

typedef struct
{
	const WdgM_ModeType             WdgM_InitialMode;
	WdgM_AliveEntityStateType       *WdgM_AliveEntityStatePtr;
	const WdgM_ModeConfigType       WdgM_Mode[];
}WdgM_ConfigSetType;

/** @req WDGM118 **/
typedef struct
{
  const WdgM_GeneralType   *WdgM_General;
  const WdgM_ConfigSetType *WdgM_ConfigSet;
}WdgM_ConfigType;
#endif /* WDGM_CONFIGTYPES_H_ */
