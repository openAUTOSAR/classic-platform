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

/*
 * @req 4.0.3/EcuM2994 Callback/callouts should be in EcuM_Cbk.h
 * @req 4.0.3/EcuM2667 Include only this file if you want to use the callbacks/callouts
 *
 * - EcuM_WakeupSourceType holds generated types (non-RTE)
 * - EcuM_WakeupReactionType (non-RTE)
 */

#ifndef ECUM_CBK_H_
#define ECUM_CBK_H_

#include "EcuM.h"

struct EcuM_ConfigS;

typedef enum
{
	ECUM_WKACT_RUN = 0,       /**< Initialization into RUN state */
	ECUM_WKACT_TTII = 2,       /**< Execute time triggered increased inoperation protocol and shutdown */
	ECUM_WKACT_SHUTDOWN = 3   /**< Immediate shutdown */
} EcuM_WakeupReactionType;


//void EcuM_CB_NfyNvMJobEnd(uint8 ServiceId, NvM_RequestResultType JobResult);

void EcuM_ErrorHook(Std_ReturnType reason);

void EcuM_OnRTEStartup(void);

void EcuM_OnEnterRun(void);
void EcuM_OnExitRun(void);
void EcuM_OnExitPostRun(void);

void EcuM_OnPrepShutdown(void);
void EcuM_OnGoSleep(void);
void EcuM_OnGoOffOne(void);
void EcuM_OnGoOffTwo(void);

void EcuM_EnableWakeupSources(EcuM_WakeupSourceType wakeupSource);
void EcuM_DisableWakeupSources(EcuM_WakeupSourceType wakeupSource);

void EcuM_GenerateRamHash(void);
uint8 EcuM_CheckRamHash(void);

void EcuM_AL_SwitchOff(void);
void EcuM_AL_DriverRestart(void);
void EcuM_AL_DriverInitZero(void);

struct EcuM_ConfigS* EcuM_DeterminePbConfiguration(void);
void EcuM_AL_DriverInitOne(const struct EcuM_ConfigS* ConfigPtr);
void EcuM_AL_DriverInitTwo(const struct EcuM_ConfigS* ConfigPtr);
void EcuM_AL_DriverInitThree(const struct EcuM_ConfigS* ConfigPtr);


void EcuM_StartWakeupSources(EcuM_WakeupSourceType wakeupSource);
void EcuM_CheckValidation(EcuM_WakeupSourceType wakeupSource);
void EcuM_StopWakeupSources(EcuM_WakeupSourceType wakeupSource);

EcuM_WakeupReactionType EcuM_OnWakeupReaction(EcuM_WakeupReactionType wact);

void EcuM_CheckWakeup(EcuM_WakeupSourceType wakeupSource);
void EcuM_SleepActivity(void);

#endif /*ECUM_CBK_H_*/
