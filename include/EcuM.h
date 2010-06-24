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

/** @addtogroup EcuM ECU State Manager
 *  @{ */

/** @file EcuM.h
 * API and type definitions for ECU State Manager.
 */

#ifndef ECUM_H_
#define ECUM_H_

#include "Std_Types.h"
#include "Os.h"
#if defined(USE_COM)
#include "ComStack_Types.h"
#endif

/** @name Error Codes */
//@{
#define ECUM_E_NOT_INITIATED (0x10)
#define ECUM_E_SERVICE_DISABLED (0x11)
#define ECUM_E_NULL_POINTER (0x12)
#define ECUM_E_INVALID_PAR (0x13)
#define ECUM_E_MULTIPLE_RUN_REQUESTS (0x14)
#define ECUM_E_MISMATCHED_RUN_RELEASE (0x15)
#define ECUM_E_STATE_PAR_OUT_OF_RANGE (0x16)
#define ECUM_E_UNKNOWN_WAKEUP_SOURCE (0x17)
//@}

/** @name Service id's */
//@{
#define ECUM_GETSTATE_ID (0x07)
#define ECUM_SELECTAPPMODE_ID (0x0f)
#define ECUM_GETAPPMODE_ID (0x11)
#define ECUM_SELECT_BOOTARGET_ID (0x12)
#define ECUM_GET_BOOTARGET_ID (0x13)
#define ECUM_MAINFUNCTION_ID (0x18)

/** Possible states */
typedef enum {
	ECUM_STATE_APP_RUN = 0x32,
	ECUM_STATE_SHUTDOWN = 0x40,
	ECUM_STATE_WAKEUP = 0x20,
	ECUM_SUBSTATE_MASK = 0x0F,
	ECUM_STATE_WAKEUP_WAKESLEEP = 0x25,
	ECUM_STATE_WAKEUP_ONE = 0x21,
	ECUM_STATE_OFF = 0x80,
	ECUM_STATE_STARTUP = 0x10,
	ECUM_STATE_PREP_SHUTDOWN = 0x44,
	ECUM_STATE_RUN = 0x30,
	ECUM_STATE_STARTUP_TWO = 0x12,
	ECUM_STATE_WAKEUP_TTII = 0x26,
	ECUM_STATE_WAKEUP_VALIDATION = 0x22,
	ECUM_STATE_GO_SLEEP = 0x49,
	ECUM_STATE_STARTUP_ONE = 0x11,
	ECUM_STATE_WAKEUP_TWO = 0x24,
	ECUM_STATE_SLEEP = 0x50,
	ECUM_STATE_WAKEUP_REACTION = 0x23,
	ECUM_STATE_APP_POST_RUN = 0x33,
	ECUM_STATE_GO_OFF_TWO = 0x4e,
	ECUM_STATE_RESET = 0x90,
	ECUM_STATE_GO_OFF_ONE = 0x4d
} EcuM_StateType;

typedef uint8 EcuM_UserType;

enum {
	/** Internal reset of µC (bit 2).
	 *  The internal reset typically only resets the µC
	 *  core but not peripherals or memory
	 *  controllers. The exact behavior is hardware
	 *  specific.
	 *  This source may also indicate an unhandled
	 *  exception. */
	ECUM_WKSOURCE_INTERNAL_RESET = 0x04,

	/** Reset by external watchdog (bit 4), if
	 *  detection supported by hardware */
	ECUM_WKSOURCE_EXTERNAL_WDG = 0x10,

	/** Reset by internal watchdog (bit 3) */
	ECUM_WKSOURCE_INTERNAL_WDG = 0x08,

	/** Power cycle (bit 0) */
	ECUM_WKSOURCE_POWER = 0x01,

	/** ~0 to the power of 29 */
	ECUM_WKSOURCE_ALL_SOURCES = 0x3FFFFFFF,

	/** Hardware reset (bit 1).
	 *  If hardware cannot distinguish between a
	 *  power cycle and a reset reason, then this
	 *  shall be the default wakeup source */
	ECUM_WKSOURCE_RESET = 0x02,
};

typedef uint32 EcuM_WakeupSourceType;

typedef enum
{
	ECUM_WKSTATUS_NONE = 0,        /**< No pending wakeup event was detected */
	ECUM_WKSTATUS_PENDING = 1,     /**< The wakeup event was detected but not yet validated */
	ECUM_WKSTATUS_VALIDATED = 2,   /**< The wakeup event is valid */
	ECUM_WKSTATUS_EXPIRED = 3,     /**< The wakeup event has not been validated and has expired therefore */
} EcuM_WakeupStatusType;

typedef enum
{
	ECUM_WWKACT_RUN = 0,       /**< Initialization into RUN state */
	ECUM_WKACT_TTII = 2,       /**< Execute time triggered increased inoperation protocol and shutdown */
	ECUM_WKACT_SHUTDOWN = 3,   /**< Immediate shutdown */
} EcuM_WakeupReactionType;

typedef enum
{
	ECUM_BOOT_TARGET_APP = 0,          /**< The Ecu will boot into the application */
	ECUM_BOOT_TARGET_BOOTLOADER = 1,   /**< The Ecu will boot into the bootloader */
} EcuM_BootTargetType;


#define ECUM_MODULE_ID			MODULE_ID_ECUM
#define ECUM_VENDOR_ID			1

#define ECUM_SW_MAJOR_VERSION	1
#define ECUM_SW_MINOR_VERSION	0
#define ECUM_SW_PATCH_VERSION	0

#define ECUM_AR_MAJOR_VERSION	1
#define ECUM_AR_MINOR_VERSION	2
#define ECUM_AR_PATCH_VERSION	2

#include "EcuM_Cfg.h"

#if ( ECUM_VERSION_INFO_API == STD_ON)
#define EcuM_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,ECUM)
#endif

void EcuM_Init( void );
void EcuM_StartupTwo();
void EcuM_Shutdown();

Std_ReturnType EcuM_GetState(EcuM_StateType* state);

Std_ReturnType EcuM_RequestRUN(EcuM_UserType user);
Std_ReturnType EcuM_ReleaseRUN(EcuM_UserType user);

Std_ReturnType EcuM_RequestPOST_RUN(EcuM_UserType user);
Std_ReturnType EcuM_ReleasePOST_RUN(EcuM_UserType user);

void EcuM_KillAllRUNRequests();

#if defined(USE_COM)
Std_ReturnType EcuM_ComM_RequestRUN(NetworkHandleType channel);
Std_ReturnType EcuM_ComM_ReleaseRUN(NetworkHandleType channel);
boolean EcuM_ComM_HasRequestedRUN(NetworkHandleType channel);
#endif

Std_ReturnType EcuM_SelectShutdownTarget(EcuM_StateType target, uint8 mode);
Std_ReturnType EcuM_GetShutdownTarget(EcuM_StateType* target, uint8* mode);
Std_ReturnType EcuM_GetLastShutdownTarget(EcuM_StateType* target, uint8* mode);

EcuM_WakeupSourceType EcuM_GetPendingWakeupEvents();
void EcuM_ClearWakeupEvent(EcuM_WakeupSourceType sources);
EcuM_WakeupSourceType EcuM_GetValidatedWakeupEvents();
EcuM_WakeupSourceType EcuM_GetExpiredWakeupEvents();
EcuM_WakeupStatusType EcuM_GetStatusOfWakeupSource(EcuM_WakeupSourceType sources);

Std_ReturnType EcuM_SelectApplicationMode(AppModeType appMode);
Std_ReturnType EcuM_GetApplicationMode(AppModeType* appMode);

Std_ReturnType EcuM_SelectBootTarget(EcuM_BootTargetType target);
Std_ReturnType EcuM_GetBootTarget(EcuM_BootTargetType* target);

void EcuM_MainFunction(void);

void EcuM_OnGoOffTwo( void );
void EcuM_AL_SwitchOff( void );

#endif /*ECUM_H_*/
/** @} */
