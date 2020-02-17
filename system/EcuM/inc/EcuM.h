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
/** @fileSafetyClassification ASIL **/ /* Same EcuM.h always used  */

/** @addtogroup EcuM ECU State Manager
 *  @{ */

/** @file EcuM.h
 * API and type definitions for ECU State Manager.
 */

/*
 * Include structure:
 *
 *
 *
 *   Rte_Type.h -->  Std_Types.h
 *       ^              ^
 *       |              |
 *   Rte_EcuM.h <--  EcuM_Types.h
 *       ^              ^
 *       |              |     /-----> EcuM_Cfg.h
 *       |              |    /------> EcuM_Generated_Types.h
 *       |              |   /         (Holds EcuM_ConfigType and includes all BSW modules )
 *       |              |  /-------> EcuM_Cbk.h
 *       |              | /           (want types EcuM_WakeupSourceType, EcuM_ConfigType *, EcuM_WakeupSourceType , EcuM_WakeupReactionType )
 *       |              |/
 *       |            EcuM.h  <----- EcuM_Callout_Stubs.c
 *       |              ^       \--- EcuM_PBCfg.c
 *       |              |
 *       |              |
 *       `---------- EcuM_xxx.c ---> Memmap.h
 *                               \-> Det.h, Dem.h
 *
 *
 *
 * Problems:
 * - Can_Cfg.h can include just "EcuM_Cbk.h"...
 *   .. it will need at EcuM.h.. problem is that EcuM.h includes EcuM_Cbk.h
 * - Most BSW modules uses DEM that in Dem_Types.h will include "Rte_Type.h"
 *   (if enabled by CFG_DEM_USE_RTE)
 *
 * - EcuM_Generated_Types.h is quite crappy since it includes the
 *
 * Changes:
 *   - EcuM_Cfg.h , must not include ANY include files.
 *   - EcuM_Pbcfg.c must include "EcuM_Generated_Types.h"
 *   - EcuM.c, etc must include "EcuM_Generated_Types.h"
 *   --> The GOOD, we keep circular include from EcuM_Generated_Types.h
 *
 *
 *
 */

/* @req SWS_EcuM_00991 */ /*The implementation of the ECU State Manager Fixed module shall provide one file EcuM.h containing fix type declarations, forward declaration to generated types, and function prototypes.*/
/* @req SWS_EcuM_00676 */ /*It shall only be necessary to include EcuM.h to use all services of the ECU State Manager.*/
/* @req SWS_EcuM_04036 */
/* @req ARC_SWS_ECUM_00011 The ECUM module shall be implemented as a compiler and hardware independent module */

#ifndef ECUM_H_
#define ECUM_H_

/* @req SWS_BSW_00201 */ /*Development errors should be of type uint8 */
#define ECUM_MODULE_ID			10u
#define ECUM_VENDOR_ID			60u

/* @req SWS_BSW_00059 Published information */
#define ECUM_SW_MAJOR_VERSION	3u
#define ECUM_SW_MINOR_VERSION	1u
#define ECUM_SW_PATCH_VERSION	0u

#define ECUM_AR_MAJOR_VERSION	4u
#define ECUM_AR_MINOR_VERSION	3u
#define ECUM_AR_PATCH_VERSION	0u

#define ECUM_CALLOUT_SW_MAJOR_VERSION   3u
#define ECUM_CALLOUT_SW_MINOR_VERSION   0u
#define ECUM_CALLOUT_SW_PATCH_VERSION   0u

#include "Os.h"
// ECUM_ARC_SAFETY_PLATFORM is based on the configuration enabled in makefile
#if defined(CFG_SAFETY_PLATFORM)
#define ECUM_ARC_SAFETY_PLATFORM STD_ON
#else
#define ECUM_ARC_SAFETY_PLATFORM STD_OFF
#endif
/* @req SWS_BSW_00020 */
/* @req SWS_EcuM_02993 *//* @req SWS_EcuM_02993 */
#include "EcuM_Cfg.h"
#include "EcuM_Types.h"
#include "EcuM_Cbk.h"

#if defined(USE_COM)
#include "ComStack_Types.h"
#endif

#if defined(USE_COMM)
#include "ComM.h"
#endif

#if defined(USE_NVM)
#include "NvM.h"
#endif

/** @name Error Codes */
/* @req SWS_BSW_00073 */
/* @req SWS_EcuM_02982 */ /* @req SWS_EcuM_02984 */ /* @req SWS_EcuM_04032 */
#define ECUM_E_UNINIT (0x10u)
#define ECUM_E_SERVICE_DISABLED (0x11u)
#define ECUM_E_NULL_POINTER (0x12u)
#define ECUM_E_INVALID_PAR (0x13u)
#define ECUM_E_MULTIPLE_RUN_REQUESTS (0x14u)
#define ECUM_E_MISMATCHED_RUN_RELEASE (0x15u)
#define ECUM_E_STATE_PAR_OUT_OF_RANGE (0x16u)
#define ECUM_E_UNKNOWN_WAKEUP_SOURCE (0x17u)
#define ECUM_E_ARC_TIMERERROR (0x18u)
#define ECUM_E_ARC_FAULTY_SHUTDOWN_TARGET (0x19u)
#define ECUM_E_ARC_FAULTY_CURRENT_STATE (0x1au)
#define ECUM_E_ARC_PARTITION_SYNC_TIMEOUT (0x1bu)
//@}

/** @name Service id's */
//@{
#define ECUM_REQUESTRUN_ID (0x03u)
#define ECUM_RELEASERUN_ID (0x04u)
#define ECUM_SELECTSHUTDOWNTARGET_ID (0x06u)
#define ECUM_GETSTATE_ID (0x07u)
#define ECUM_GETSHUTDOWNTARGET_ID (0x09u)
#define ECUM_REQUESTPOSTRUN_ID (0x0au)
#define ECUM_RELEASEPOSTRUN_ID (0x0bu)
#define ECUM_SETWAKEUPEVENT_ID  (0x0cu)
#define ECUM_SELECTAPPMODE_ID (0x0fu)
#define ECUM_COMM_RELEASERUN_ID (0x10u)
#define ECUM_GETAPPMODE_ID (0x11u)
#define ECUM_SELECT_BOOTARGET_ID (0x12u)
#define ECUM_GET_BOOTARGET_ID (0x13u)
#define ECUM_VALIDATE_WAKEUP_EVENT_ID (0x14u)
#define ECUM_GETSTATUSOFWAKEUPSOURCE_ID (0x17u)
#define ECUM_MAINFUNCTION_ID (0x18u)
#define ECUM_STARTUPTWO_ID (0x1au)
#define ECUM_SELECTSHUTDOWNCAUSE_ID (0x1bu)
#define ECUM_COMM_HASREQUESTEDRUN_ID (0x1bu)
#define ECUM_GETSHUTDOWNCAUSE_ID (0x1cu)
#define ECUM_GODOWN_ID (0x1fu)
#define ECUM_GOHALT_ID (0x20u)
#define ECUM_GOPOLL_ID (0x21)
#define ECUM_ARC_STARTUPTWO_ID (0x22u)
#define ECUM_ARC_MAINFUNCTION_A0_ID (0x23u)
#define ECUM_ARC_MAINFUNCTION_QM_ID (0x24u)
#define ECUM_GETVERSIONINFO_ID (0x00u)

#define ECUM_AR_VERSION 	((ECUM_AR_MAJOR_VERSION*10000)+(ECUM_AR_MINOR_VERSION*100)+ECUM_AR_PATCH_VERSION)

/* @req SWS_EcuM_02813 */
/* @req SWS_EcuM_02935 */ /* @req SWS_EcuM_04034 */

#if ( ECUM_VERSION_INFO_API == STD_ON)
void EcuM_GetVersionInfo( Std_VersionInfoType* VersionInfo );
#endif

/* @req SWS_EcuM_02811 */
void EcuM_Init( void );

/* @req SWS_EcuM_02838 */
void EcuM_StartupTwo(void);

/* @req SWS_EcuM_02812 */
void EcuM_Shutdown(void);

#if defined(USE_ECUM_FIXED)
/* @req SWS_EcuM_00823 */
Std_ReturnType EcuM_GetState(EcuM_StateType* state);

/* @req SWS_EcuM_04124 */
Std_ReturnType EcuM_RequestRUN(EcuM_UserType user);

/* @req SWS_EcuM_00815 */
Std_ReturnType EcuM_ReleaseRUN(EcuM_UserType user);

/* @req SWS_EcuM_00819 */
Std_ReturnType EcuM_RequestPOST_RUN(EcuM_UserType user);

/* @req SWS_EcuM_04129 */
Std_ReturnType EcuM_ReleasePOST_RUN(EcuM_UserType user);

/* @req SWS_EcuM_00821 */
void EcuM_KillAllRUNRequests(void);

/* @req SWS_EcuMf_00101 */
void EcuM_KillAllPostRUNRequests(void);
#endif /*USE_ECUM_FIXED*/

#if defined(USE_COMM) &&  (ECUM_AR_VERSION < 40300)
Std_ReturnType EcuM_ComM_RequestRUN(NetworkHandleType channel);
Std_ReturnType EcuM_ComM_ReleaseRUN(NetworkHandleType channel);
boolean EcuM_ComM_HasRequestedRUN(NetworkHandleType channel);
#endif

/* @req SWS_EcuM_02822 */
Std_ReturnType EcuM_SelectShutdownTarget(EcuM_StateType shutdownTarget, uint8 sleepMode);

/* @req SWS_EcuM_02824 */
Std_ReturnType EcuM_GetShutdownTarget(EcuM_StateType* shutdownTarget, uint8* sleepMode);
Std_ReturnType EcuM_GetLastShutdownTarget(EcuM_StateType* shutdownTarget, uint8* sleepMode);

Std_ReturnType EcuM_SelectBootTarget(EcuM_BootTargetType target);

Std_ReturnType EcuM_GetBootTarget(EcuM_BootTargetType* target);

/* @req SWS_EcuM_02827 */  /* @req SWS_EcuM_02827 */
EcuM_WakeupSourceType EcuM_GetPendingWakeupEvents(void);

/* @req SWS_EcuM_02828 */
void EcuM_ClearWakeupEvent(EcuM_WakeupSourceType source );

/* @req SWS_EcuM_02830 */   /* @req SWS_EcuM_02830 */
EcuM_WakeupSourceType EcuM_GetValidatedWakeupEvents(void);

/* @req SWS_EcuM_02831 */
EcuM_WakeupSourceType EcuM_GetExpiredWakeupEvents(void);

#if defined(USE_ECUM_FIXED)
/* @req SWS_EcuM_02832 */
EcuM_WakeupStatusType EcuM_GetStatusOfWakeupSource(EcuM_WakeupSourceType sources);
#endif

/* @req SWS_EcuM_02826*/  /* @req SWS_EcuM_02826*/
void EcuM_SetWakeupEvent(EcuM_WakeupSourceType sources);

/* @req SWS_EcuM_02829 */  /* @req SWS_EcuM_02829 */
void EcuM_ValidateWakeupEvent(EcuM_WakeupSourceType sources);

/* @req SWS_EcuM_02837 */
void EcuM_MainFunction(void);

#if defined(USE_ECUM_FLEXIBLE)
/* @req SWS_EcuM_04046 */
Std_ReturnType EcuM_GoDown(uint16 caller);

/* @req SWS_EcuM_04048 */
Std_ReturnType EcuM_GoHalt(void);

/* @req SWS_EcuM_04050 */
Std_ReturnType EcuM_SelectShutdownCause(EcuM_ShutdownCauseType target);

/* @req SWS_EcuM_04051 */
Std_ReturnType EcuM_GetShutdownCause(EcuM_ShutdownCauseType *shutdownCause);
#endif

#if (ECUM_ARC_SAFETY_PLATFORM == STD_ON)
EcuM_SP_RetStatus EcuM_MainFunction_Partition_A0(EcuM_StateType current_state);
EcuM_SP_RetStatus EcuM_MainFunction_Partition_QM(EcuM_StateType current_state);
void EcuM_SP_Sync_UpdateStatus(EcuM_SP_PartitionFunCompletion syncSubState);
#endif
#endif /*ECUM_H_*/
/** @} */
