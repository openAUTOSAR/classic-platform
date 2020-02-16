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

/*
 * @req 4.0.3/EcuM2994 Callback/callouts should be in EcuM_Cbk.h
 * @req 4.0.3/EcuM2667 Include only this file if you want to use the callbacks/callouts
 *
 * - EcuM_WakeupSourceType holds generated types (non-RTE)
 * - EcuM_WakeupReactionType (non-RTE)
 */

/* @req EcuM2994 */
#ifndef ECUM_CBK_H_
#define ECUM_CBK_H_

/* @req EcuM2677 */
#include "EcuM.h"

struct EcuM_ConfigS;

typedef enum
{
	ECUM_WKACT_RUN = 0,       /**< Initialization into RUN state */
	ECUM_WKACT_TTII = 2,       /**< Execute time triggered increased inoperation protocol and shutdown */
	ECUM_WKACT_SHUTDOWN = 3   /**< Immediate shutdown */
} EcuM_WakeupReactionType;


//void EcuM_CB_NfyNvMJobEnd(uint8 ServiceId, NvM_RequestResultType JobResult);

/* @req EcuM2904*/
void EcuM_ErrorHook(Std_ReturnType reason);

/* @req EcuM2905*/
void EcuM_AL_DriverInitZero(void);

/* @req EcuM2906*/
struct EcuM_ConfigS* EcuM_DeterminePbConfiguration(void);

/* @req EcuM2907*/
void EcuM_AL_DriverInitOne(const struct EcuM_ConfigS* ConfigPtr);

#if defined(USE_ECUM_FIXED) || defined(USE_ECUM)
/* @req EcuM908*/
void EcuM_AL_DriverInitTwo(const struct EcuM_ConfigS* ConfigPtr);

/* @req EcuM909*/
void EcuM_AL_DriverInitThree(const struct EcuM_ConfigS* ConfigPtr);

/* @req EcuM2910 */
void EcuM_OnRTEStartup(void);

/* @req EcuM2911 */
void EcuM_OnEnterRun(void);

/* @req EcuM2912 */
void EcuM_OnExitRun(void);

/* @req EcuM2913 */
void EcuM_OnExitPostRun(void);

/* @req EcuM2914 */
void EcuM_OnPrepShutdown(void);

/* @req EcuM2915 */
void EcuM_OnGoSleep(void);
#endif

/* @req EcuM2916 */
void EcuM_OnGoOffOne(void);

/* @req EcuM2917 */
void EcuM_OnGoOffTwo(void);

/* @req EcuM2918 */
void EcuM_EnableWakeupSources(EcuM_WakeupSourceType wakeupSource);

/* @req EcuM2922 */
void EcuM_DisableWakeupSources(EcuM_WakeupSourceType wakeupSource);

/* @req EcuM2919 */
void EcuM_GenerateRamHash(void);

/* @req EcuM2921 */
uint8 EcuM_CheckRamHash(void);

/* @req EcuM2920 */
void EcuM_AL_SwitchOff(void);

/* @req EcuM2923 */
void EcuM_AL_DriverRestart(const struct EcuM_ConfigS* ConfigPtr);

bool EcuM_LoopDetection(void);

/* @req EcuM2924 */
void EcuM_StartWakeupSources(EcuM_WakeupSourceType wakeupSource);

/* @req EcuM2925 */
void EcuM_CheckValidation(EcuM_WakeupSourceType wakeupSource);

/* @req EcuM2926 */
void EcuM_StopWakeupSources(EcuM_WakeupSourceType wakeupSource);

#if defined(USE_ECUM_FIXED) || defined(USE_ECUM)
/* @req EcuM2927 */
EcuM_WakeupReactionType EcuM_OnWakeupReaction(EcuM_WakeupReactionType wact);
#endif

/* @req EcuM2929 */
void EcuM_CheckWakeup(EcuM_WakeupSourceType wakeupSource);

/* @req EcuM2928 */
void EcuM_SleepActivity(void);

#if defined(USE_ECUM_FLEXIBLE)
/* @req EcuM4065 */
void EcuM_AL_Reset(EcuM_ResetType reset);
#endif

#endif /*ECUM_CBK_H_*/
