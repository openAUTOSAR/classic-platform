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


/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.3.0 */
/** @fileSafetyClassification ASIL **/ /* Common for all EcuM variants  */

/*
 * @req SWS_EcuM_02994 @req SWS_EcuMf_02994 Callback/callouts should be in EcuM_Cbk.h
 * @req SWS_EcuM_02677 @req SWS_EcuMf_02677 Include only this file if you want to use the callbacks/callouts
 *
 * - EcuM_WakeupSourceType holds generated types (non-RTE)
 * - EcuM_WakeupReactionType (non-RTE)
 */

/* @req SWS_BSW_00172 Callback functions should have void as return type. */
/* @req SWS_BSW_00026 */
/* @req SWS_EcuM_02994 */ /* @req SWS_EcuM_02994 */
#ifndef ECUM_CBK_H_
#define ECUM_CBK_H_

/* @req SWS_EcuM_02677 */ /*It shall only be necessary to include EcuM_Cbk.h to interact with the callbacks and callouts of the ECU State Manager*/
#include "EcuM.h"

#if defined(CFG_ARC_ECUM_NVM_READ_INIT)
#include "Os.h"
#endif

struct EcuM_ConfigS;

typedef enum
{
    ECUM_WKACT_RUN = 0,       /**< Initialization into RUN state */
    ECUM_WKACT_TTII = 2,       /**< Execute time triggered increased inoperation protocol and shutdown */
    ECUM_WKACT_SHUTDOWN = 3   /**< Immediate shutdown */
} EcuM_WakeupReactionType;

/* @req SWS_EcuM_02904*/
void EcuM_ErrorHook(uint16 reason);

/* @req SWS_EcuM_02905*/
void EcuM_AL_DriverInitZero(void);

/* @req SWS_EcuM_02906*/
struct EcuM_ConfigS* EcuM_DeterminePbConfiguration(void);

/* @req SWS_EcuM_02907*/
void EcuM_AL_DriverInitOne(const struct EcuM_ConfigS* ConfigPtr);

#if defined(USE_ECUM_FIXED)
#if (ECUM_ARC_SAFETY_PLATFORM == STD_ON)
/* @req SWS_EcuM_00908*/
void EcuM_AL_DriverInitTwo_Partition_A0(const struct EcuM_ConfigS* ConfigPtr);
void EcuM_AL_DriverInitTwo_Partition_QM(const struct EcuM_ConfigS* ConfigPtr);
/* @req SWS_EcuM_00909*/
void EcuM_AL_DriverInitThree_Partition_A0(const struct EcuM_ConfigS* ConfigPtr);
void EcuM_AL_DriverInitThree_Partition_QM(const struct EcuM_ConfigS* ConfigPtr);

/* @req SWS_EcuM_00910 */
void EcuM_OnRTEStartup(void);

/* @req SWS_EcuM_00911 */
void EcuM_OnEnterRun_Partition_A0(void);
void EcuM_OnEnterRun_Partition_QM(void);

/* @req SWS_EcuM_00912 */
void EcuM_OnExitRun_Partition_A0(void);
void EcuM_OnExitRun_Partition_QM(void);

/* @req SWS_EcuM_00913 */
void EcuM_OnExitPostRun_Partition_A0(void);
void EcuM_OnExitPostRun_Partition_QM(void);

/* @req SWS_EcuM_00914 */
void EcuM_OnPrepShutdown_Partition_A0(void);
void EcuM_OnPrepShutdown_Partition_QM(void);

#else // ECUM_ARC_SAFETY_PLATFORM
/* @req SWS_EcuM_00908*/
void EcuM_AL_DriverInitTwo(const struct EcuM_ConfigS* ConfigPtr);

/* @req SWS_EcuM_00909*/
void EcuM_AL_DriverInitThree(const struct EcuM_ConfigS* ConfigPtr);

/* @req SWS_EcuM_00910 */
void EcuM_OnRTEStartup(void);

/* @req SWS_EcuM_00911 */
void EcuM_OnEnterRun(void);

/* @req SWS_EcuM_00912 */
void EcuM_OnExitRun(void);

/* @req SWS_EcuM_00913 */
void EcuM_OnExitPostRun(void);

/* @req SWS_EcuM_00914 */
void EcuM_OnPrepShutdown(void);

#endif // ECUM_ARC_SAFETY_PLATFORM
/* @req SWS_EcuM_00915 */
void EcuM_OnGoSleep(void);
#endif // USE_ECUM_FIXED

#if (ECUM_ARC_SAFETY_PLATFORM == STD_ON)
/* @req SWS_EcuM_02916 */
void EcuM_OnGoOffOne_Partition_A0(void);
void EcuM_OnGoOffOne_Partition_QM(void);
/* @req SWS_EcuM_02917 */
void EcuM_OnGoOffTwo_Partition_A0(void);
void EcuM_OnGoOffTwo_Partition_QM(void);
#else // ECUM_ARC_SAFETY_PLATFORM
/* @req SWS_EcuM_02916 */
void EcuM_OnGoOffOne(void);
/* @req SWS_EcuM_02917 */
void EcuM_OnGoOffTwo(void);
#endif // ECUM_ARC_SAFETY_PLATFORM

/* @req SWS_EcuM_02918 */
void EcuM_EnableWakeupSources(EcuM_WakeupSourceType wakeupSource);

/* @req SWS_EcuM_02922 */
void EcuM_DisableWakeupSources(EcuM_WakeupSourceType wakeupSource);

/* @req SWS_EcuM_02919 */
void EcuM_GenerateRamHash(void);

/* @req SWS_EcuM_02921 */
uint8 EcuM_CheckRamHash(void);

/* @req SWS_EcuM_02920 */
void EcuM_AL_SwitchOff(void);

/* @req SWS_EcuM_02923 */
void EcuM_AL_DriverRestart(const struct EcuM_ConfigS* ConfigPtr);

void EcuM_LoopDetection(void);

/* @req SWS_EcuM_02924 */
void EcuM_StartWakeupSources(EcuM_WakeupSourceType wakeupSource);

/* @req SWS_EcuM_02925 */
void EcuM_CheckValidation(EcuM_WakeupSourceType wakeupSource);

/* @req SWS_EcuM_02926 */
void EcuM_StopWakeupSources(EcuM_WakeupSourceType wakeupSource);

#if defined(USE_ECUM_FIXED)
/* @req SWS_EcuM_02927 */
EcuM_WakeupReactionType EcuM_OnWakeupReaction(EcuM_WakeupReactionType wact);
#endif

/* @req SWS_EcuM_02929 */
void EcuM_CheckWakeup(EcuM_WakeupSourceType wakeupSource);

/* @req SWS_EcuM_02928 */
void EcuM_SleepActivity(void);

/* @req SWS_EcuM_04065 */
void EcuM_AL_Reset(EcuM_ResetType reset);

void EcuM_Arc_RememberWakeupEvent(uint32 resetReason);

#if defined(CFG_ARC_ECUM_NVM_READ_INIT)
void EcuM_Arc_InitFeeReadAdminBlock(TickType tickTimerStart, uint32 EcuMNvramReadAllTimeout);
void EcuM_Arc_InitMemReadAllMains(void);
#endif

#endif /*ECUM_CBK_H_*/
