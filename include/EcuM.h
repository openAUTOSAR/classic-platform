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
 *   Rte_EcuM.h <--  EcuM_Types.h*
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
 * *) Only ifdef CFG_ECUM_USE_SERVICE_COMPONENT
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

/* @req EcuM2991 */
/* @req EcuM2676 */


#ifndef ECUM_H_
#define ECUM_H_

#include "Modules.h"

#define ECUM_MODULE_ID			MODULE_ID_ECUM
#define ECUM_VENDOR_ID			VENDOR_ID_ARCCORE

#define ECUM_SW_MAJOR_VERSION	1
#define ECUM_SW_MINOR_VERSION	0
#define ECUM_SW_PATCH_VERSION	0

#define ECUM_AR_MAJOR_VERSION	4
#define ECUM_AR_MINOR_VERSION	0
#define ECUM_AR_PATCH_VERSION	3


#define VALIDATE_STATE(_state) \
	do { \
		EcuM_StateType ecuMState;  \
		EcuM_GetState(&ecuMState); \
		assert(ecuMState == (_state) ); \
	} while(0)

/* @req EcuM2993 */

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


/* @req EcuMf0012 */

/** @name Error Codes */
//@{
#define ECUM_E_UNINIT (0x10)
#define ECUM_E_SERVICE_DISABLED (0x11)
#define ECUM_E_NULL_POINTER (0x12)
#define ECUM_E_INVALID_PAR (0x13)
#define ECUM_E_MULTIPLE_RUN_REQUESTS (0x14)
#define ECUM_E_MISMATCHED_RUN_RELEASE (0x15)
#define ECUM_E_STATE_PAR_OUT_OF_RANGE (0x16)
#define ECUM_E_UNKNOWN_WAKEUP_SOURCE (0x17)
#define ECUM_E_ARC_TIMERERROR (0x18)
//@}

/** @name Service id's */
//@{
#define ECUM_REQUESTRUN_ID (0x03)
#define ECUM_RELEASERUN_ID (0x04)
#define ECUM_SELECTSHUTDOWNTARGET_ID (0x06)
#define ECUM_GETSTATE_ID (0x07)
#define ECUM_GETSHUTDOWNTARGET_ID (0x09)
#define ECUM_REQUESTPOSTRUN_ID (0x0a)
#define ECUM_RELEASEPOSTRUN_ID (0x0b)
#define ECUM_SETWAKEUPEVENT_ID  (0x0c)
#define ECUM_SELECTAPPMODE_ID (0x0f)
#define ECUM_COMM_RELEASERUN_ID (0x10)
#define ECUM_GETAPPMODE_ID (0x11)
#define ECUM_SELECT_BOOTARGET_ID (0x12)
#define ECUM_GET_BOOTARGET_ID (0x13)
#define ECUM_VALIDATE_WAKEUP_EVENT_ID (0x14)
#define ECUM_GETSTATUSOFWAKEUPSOURCE_ID (0x17)
#define ECUM_MAINFUNCTION_ID (0x18)
#define ECUM_SELECTSHUTDOWNCAUSE_ID (0x1b)
#define ECUM_COMM_HASREQUESTEDRUN_ID (0x1b)
#define ECUM_GETSHUTDOWNCAUSE_ID (0x1c)
#define ECUM_GODOWN_ID (0x1f)
#define ECUM_GOHALT_ID (0x20)
#define ECUM_GOPOLL_ID (0x21)
#define ECUM_ARC_STARTUPTWO_ID (0x20)

#define ECUM_AR_VERSION 	(ECUM_AR_MAJOR_VERSION*10000)+\
							(ECUM_AR_MINOR_VERSION*100)+ECUM_AR_PATCH_VERSION

/* @req EcuM2813 */
/* @req EcuM2729 */
/* @req EcuM2728 */

#if ( ECUM_VERSION_INFO_API == STD_ON)
#define EcuM_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,ECUM)
#endif


/* @req EcuM2811 */
void EcuM_Init( void );

/* @req EcuM2838 */
void EcuM_StartupTwo(void);

/* @req EcuM2812 */
void EcuM_Shutdown(void);

#if defined(USE_ECUM_FIXED) || defined(USE_ECUM)
/* @req EcuM2823 */
Std_ReturnType EcuM_GetState(EcuM_StateType* state);

/* @req EcuM2814 */
Std_ReturnType EcuM_RequestRUN(EcuM_UserType user);

/* @req EcuM2815 */
Std_ReturnType EcuM_ReleaseRUN(EcuM_UserType user);

/* @req EcuM2819 */
Std_ReturnType EcuM_RequestPOST_RUN(EcuM_UserType user);

/* @req EcuM2820 */
Std_ReturnType EcuM_ReleasePOST_RUN(EcuM_UserType user);

/* @req EcuM2821 */
void EcuM_KillAllRUNRequests(void);
#endif

#if defined(USE_COMM) &&  (ECUM_AR_VERSION < 40300)
Std_ReturnType EcuM_ComM_RequestRUN(NetworkHandleType channel);
Std_ReturnType EcuM_ComM_ReleaseRUN(NetworkHandleType channel);
boolean EcuM_ComM_HasRequestedRUN(NetworkHandleType channel);
#endif

/* @req EcuM2822 */
Std_ReturnType EcuM_SelectShutdownTarget(EcuM_StateType shutdownTarget, uint8 sleepMode);

/* @req EcuM2824 */
Std_ReturnType EcuM_GetShutdownTarget(EcuM_StateType* shutdownTarget, uint8* sleepMode);

/* @req EcuM2825 */
Std_ReturnType EcuM_GetLastShutdownTarget(EcuM_StateType* shutdownTarget, uint8* sleepMode);

/* @req EcuM2827 */
EcuM_WakeupSourceType EcuM_GetPendingWakeupEvents(void);

/* @req EcuM2828 */
void EcuM_ClearWakeupEvent(EcuM_WakeupSourceType source );

/* @req EcuM2830 */
EcuM_WakeupSourceType EcuM_GetValidatedWakeupEvents(void);

/* @req EcuM2831 */
EcuM_WakeupSourceType EcuM_GetExpiredWakeupEvents(void);

#if defined(USE_ECUM_FIXED) || defined(USE_ECUM)
/* @req EcuM2832 */
EcuM_WakeupStatusType EcuM_GetStatusOfWakeupSource(EcuM_WakeupSourceType sources);
#endif

/* @req EcuM2835 */
Std_ReturnType EcuM_SelectBootTarget(EcuM_BootTargetType target);

/* @req EcuM2836 */
Std_ReturnType EcuM_GetBootTarget(EcuM_BootTargetType* target);

/* @req EcuM2826*/
void EcuM_SetWakeupEvent(EcuM_WakeupSourceType sources);

/* @req EcuM2829 */
void EcuM_ValidateWakeupEvent(EcuM_WakeupSourceType sources);

/* @req EcuM2837 */
void EcuM_MainFunction(void);

Std_ReturnType EcuM_SelectApplicationMode( EcuM_AppModeType appMode );

Std_ReturnType EcuM_GetApplicationMode( EcuM_AppModeType* appMode);

#if defined(USE_ECUM_FLEXIBLE)
/* @req EcuM4046 */
Std_ReturnType EcuM_GoDown(uint16 caller);

/* @req EcuM4048 */
Std_ReturnType EcuM_GoHalt(void);

/* @req EcuM4049 */
Std_ReturnType EcuM_GoPoll(void);

/* @req EcuM4050 */
Std_ReturnType EcuM_SelectShutdownCause(EcuM_ShutdownCauseType target);

/* @req EcuM4051 */
Std_ReturnType EcuM_GetShutdownCause(EcuM_ShutdownCauseType *shutdownCause);
#endif

#if defined(USE_ECUM_FIXED) || defined(USE_ECUM)
/* @req EcuM2839 */
#if defined(USE_NVM)
void EcuM_CB_NfyNvMJobEnd(uint8 ServiceId, NvM_RequestResultType JobResult);
#endif
#endif

#endif /*ECUM_H_*/
/** @} */
