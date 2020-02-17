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
/** @fileSafetyClassification ASIL **/ /* Common types for all EcuM variants  */

/**
 * @addtogroup EcuM ECU State Manager
 * @{
 *
 * @file EcuM_Types.h
 * Typedefs needed by EcuM module. Note that types that are generated together with
 * Ecum Service Ports should be guarded by _DEFINED_TYPEDEF_FOR_<Type>_ to avoid re-definition.
 */

/* @req SWS_EcuM_04037 */



#ifndef ECUM_TYPES_H_
#define ECUM_TYPES_H_

/* @req SWS_BSW_00024 Include AUTOSAR Standard Types Header in implementation header */
#include "Std_Types.h"
#include "Os.h"

/* @req SWS_EcuM_03025 */
/* @req SWS_EcuMf_00048 */ /*EcuM_UserType defined in Rte*/
/* @req SWS_EcuMf_00105 */ /*EcuM_StateType defined in Rte*/
/* @req SWS_EcuMf_00036 */ /*EcuM_BootTargetType defined in Rte*/
#include "Rte_EcuM_Type.h"

/* To extract main state of both EcuM state and synchronization sate.
 * e.g. Main state: RUN, Sub state: Post Run */
#ifndef ECUM_MAINSTATE_MASK
#define ECUM_MAINSTATE_MASK 0xf0
#endif /* ECUM_SUBSTATE_MASK */

/** Enum literals for EcuM_StateType */
/* @req SWS_EcuM_00507 */
/* @req SWS_EcuM_02664 */
#ifndef ECUM_SUBSTATE_MASK
#define ECUM_SUBSTATE_MASK 15U
#endif /* ECUM_SUBSTATE_MASK */

#ifndef ECUM_STATE_STARTUP
#define ECUM_STATE_STARTUP 16U
#endif /* ECUM_STATE_STARTUP */

#ifndef ECUM_STATE_STARTUP_ONE
#define ECUM_STATE_STARTUP_ONE 17U
#endif /* ECUM_STATE_STARTUP_ONE */

#ifndef ECUM_STATE_STARTUP_TWO
#define ECUM_STATE_STARTUP_TWO 18U
#endif /* ECUM_STATE_STARTUP_TWO */

#ifndef ECUM_STATE_WAKEUP
#define ECUM_STATE_WAKEUP 32U
#endif /* ECUM_STATE_WAKEUP */

#ifndef ECUM_STATE_WAKEUP_ONE
#define ECUM_STATE_WAKEUP_ONE 33U
#endif /* ECUM_STATE_WAKEUP_ONE */

#ifndef ECUM_STATE_WAKEUP_VALIDATION
#define ECUM_STATE_WAKEUP_VALIDATION 34U
#endif /* ECUM_STATE_WAKEUP_VALIDATION */

#ifndef ECUM_STATE_WAKEUP_REACTION
#define ECUM_STATE_WAKEUP_REACTION 35U
#endif /* ECUM_STATE_WAKEUP_REACTION */

#ifndef ECUM_STATE_WAKEUP_TWO
#define ECUM_STATE_WAKEUP_TWO 36U
#endif /* ECUM_STATE_WAKEUP_TWO */

#ifndef ECUM_STATE_WAKEUP_WAKESLEEP
#define ECUM_STATE_WAKEUP_WAKESLEEP 37U
#endif /* ECUM_STATE_WAKEUP_WAKESLEEP */

#ifndef ECUM_STATE_WAKEUP_TTII
#define ECUM_STATE_WAKEUP_TTII 38U
#endif /* ECUM_STATE_WAKEUP_TTII */

#ifndef ECUM_STATE_RUN
#define ECUM_STATE_RUN 48U
#endif /* ECUM_STATE_RUN */

#ifndef ECUM_STATE_APP_RUN
#define ECUM_STATE_APP_RUN 50U
#endif /* ECUM_STATE_APP_RUN */

#ifndef ECUM_STATE_APP_POST_RUN
#define ECUM_STATE_APP_POST_RUN 51U
#endif /* ECUM_STATE_APP_POST_RUN */

#ifndef ECUM_STATE_SHUTDOWN
#define ECUM_STATE_SHUTDOWN 64U
#endif /* ECUM_STATE_SHUTDOWN */

#ifndef ECUM_STATE_PREP_SHUTDOWN
#define ECUM_STATE_PREP_SHUTDOWN 68U
#endif /* ECUM_STATE_PREP_SHUTDOWN */

#ifndef ECUM_STATE_GO_SLEEP
#define ECUM_STATE_GO_SLEEP 73U
#endif /* ECUM_STATE_GO_SLEEP */

#ifndef ECUM_STATE_GO_OFF_ONE
#define ECUM_STATE_GO_OFF_ONE 77U
#endif /* ECUM_STATE_GO_OFF_ONE */

#ifndef ECUM_STATE_GO_OFF_TWO
#define ECUM_STATE_GO_OFF_TWO 78U
#endif /* ECUM_STATE_GO_OFF_TWO */

#ifndef ECUM_STATE_SLEEP
#define ECUM_STATE_SLEEP 80U
#endif /* ECUM_STATE_SLEEP */

#if defined(USE_HTMSS)
#ifndef ECUM_STATE_HWTEST_RESET
#define ECUM_STATE_HWTEST_RESET 96U
#endif /* ECUM_STATE_HWTEST_RESET */

#ifndef ECUM_STATE_HWTEST_OFF
#define ECUM_STATE_HWTEST_OFF 97U
#endif /* ECUM_STATE_HWTEST_RESET */
#endif

#ifndef ECUM_STATE_OFF
#define ECUM_STATE_OFF 128U
#endif /* ECUM_STATE_OFF */

#ifndef ECUM_STATE_RESET
#define ECUM_STATE_RESET 144U
#endif /* ECUM_STATE_RESET */

/** Enum literals for EcuM_BootTargetType */
#ifndef ECUM_BOOT_TARGET_APP
#define ECUM_BOOT_TARGET_APP 0U
#endif /* ECUM_BOOT_TARGET_APP */

#ifndef ECUM_BOOT_TARGET_OEM_BOOTLOADER
#define ECUM_BOOT_TARGET_OEM_BOOTLOADER 1U
#endif /* ECUM_BOOT_TARGET_OEM_BOOTLOADER */

#ifndef ECUM_BOOT_TARGET_SYS_BOOTLOADER
#define ECUM_BOOT_TARGET_SYS_BOOTLOADER 2U
#endif /* ECUM_BOOT_TARGET_SYS_BOOTLOADER */

/* @req SWS_EcuM_04040 */ /*@req SWS_EcuMf_00049*/
typedef uint32 EcuM_WakeupSourceType;

/* @req SWS_EcuM_04041 */
typedef enum
{
    ECUM_WKSTATUS_NONE = 0,        /**< No pending wakeup event was detected */
    ECUM_WKSTATUS_PENDING = 1,     /**< The wakeup event was detected but not yet validated */
    ECUM_WKSTATUS_VALIDATED = 2,   /**< The wakeup event is valid */
    ECUM_WKSTATUS_EXPIRED = 3,     /**< The wakeup event has not been validated and has expired therefore */
    ECUM_WKSTATUS_DISABLED = 4     /**< The wakeup source is disabled and does not detect wakeup events. */
} EcuM_WakeupStatusType;


#if defined(USE_WDGM)
#include "WdgM.h"
#endif

#include "Mcu.h"

#if defined(USE_COMM)
#include "ComM.h"
#endif

typedef struct EcuM_WakeupSourceConfig {
    EcuM_WakeupSourceType 	EcuMWakeupSourceId;
    uint32 					EcuMValidationTimeout;
#if defined(USE_COMM)
    uint8 					EcuMComMChannel;
#endif
} EcuM_WakeupSourceConfigType;

typedef struct EcuM_SleepMode
{
   uint8 					EcuMSleepModeId;
   EcuM_WakeupSourceType 	EcuMWakeupSourceMask;
   Mcu_ModeType  			EcuMSleepModeMcuMode;
 } EcuM_SleepModeType;

#if defined(USE_COMM)
typedef struct EcuM_ComMConfig {
    uint8                   EcuMComMNetworkHandle;
    ComM_BusTypeType        EcuMComBusType;
} EcuM_ComMConfigType;
#endif

typedef AppModeType EcuM_AppModeType;


 /* Defines for illegal modes/channels */
#define ECUM_SLEEP_MODE_WDGM_MODE_ILL		0xff
#define ECUM_COMM_CHANNEL_ILL				0xff
#define ECUM_VALIDATION_TIMEOUT_ILL   0xffffffffuL

/* Data structure for EcuM Safety Platform */

/*
 *  Status for sync
 */
typedef enum
{
	ECUM_SP_PARTITION_ERR,
	ECUM_SP_REQ_SYNC,
	ECUM_SP_RELEASE_SYNC,
	ECUM_SP_OK
} EcuM_SP_RetStatus;

/*
 *  Status saying the partition completes its requested functionality
 */

typedef uint16 EcuM_SP_PartitionFunCompletion;
#define ECUM_SP_PARTITION_FUN_COMPLETED_A0  0x01
#define ECUM_SP_PARTITION_FUN_COMPLETED_QM  0x02

/*
 *  EcuM_SP_SyncStatus = 0x<major_state><sync_state> (e.g. 0x30, 3- major state, 0-sync state (no partition completed)
 *  with this 4 bit sync state max 3 partition can be tracked if more partition added then this bits has to be increased
 */

typedef uint16 EcuM_SP_SyncStatus;

#define ECUM_SP_SYNC_NOT_REQUIRED  0x0

#define ECUM_PARTITIONS_IN_SYNC 0x03

#define ECUM_SP_SYNC_STARTUP_TWO_START  0x10
#define ECUM_SP_SYNC_STARTUP_TWO_PARTITION_A0_DONE  0x11
#define ECUM_SP_SYNC_STARTUP_TWO_PARTITION_QM_DONE  0x12
#define ECUM_SP_SYNC_STARTUP_TWO_PARTITION_ALL_DONE  0x13

#define ECUM_SP_SYNC_INIT_RUN_START  0x20
#define ECUM_SP_SYNC_INIT_RUN_PARTITION_A0_DONE  0x21
#define ECUM_SP_SYNC_INIT_RUN_PARTITION_QM_DONE  0x22
#define ECUM_SP_SYNC_INIT_RUN_PARTITION_ALL_DONE  0x23

#define ECUM_SP_SYNC_ENTER_RUN_START  0x30
#define ECUM_SP_SYNC_ENTER_RUN_PARTITION_A0_DONE  0x31
#define ECUM_SP_SYNC_ENTER_RUN_PARTITION_QM_DONE  0x32
#define ECUM_SP_SYNC_ENTER_RUN_PARTITION_ALL_DONE  0x33

#define ECUM_SP_SYNC_EXIT_RUN_START  0x40
#define ECUM_SP_SYNC_EXIT_RUN_PARTITION_A0_DONE  0x41
#define ECUM_SP_SYNC_EXIT_RUN_PARTITION_QM_DONE  0x42
#define ECUM_SP_SYNC_EXIT_RUN_PARTITION_ALL_DONE  0x43

#define ECUM_SP_SYNC_POST_RUN_START  0x50
#define ECUM_SP_SYNC_POST_RUN_PARTITION_A0_DONE  0x51
#define ECUM_SP_SYNC_POST_RUN_PARTITION_QM_DONE  0x52
#define ECUM_SP_SYNC_POST_RUN_PARTITION_ALL_DONE  0x53

#define ECUM_SP_SYNC_PREP_SHUTDOWN_START  0x60
#define ECUM_SP_SYNC_PREP_SHUTDOWN_PARTITION_A0_DONE  0x61
#define ECUM_SP_SYNC_PREP_SHUTDOWN_PARTITION_QM_DONE  0x62
#define ECUM_SP_SYNC_PREP_SHUTDOWN_PARTITION_ALL_DONE  0x63

#define ECUM_SP_SYNC_GO_OFF_ONE_START  0x70
#define ECUM_SP_SYNC_GO_OFF_ONE_PARTITION_A0_DONE  0x71
#define ECUM_SP_SYNC_GO_OFF_ONE_PARTITION_QM_DONE  0x72
#define ECUM_SP_SYNC_GO_OFF_ONE_PARTITION_ALL_DONE  0x73

#define ECUM_SP_SYNC_INIT_GO_OFF_TWO_START  0x80
#define ECUM_SP_SYNC_INIT_GO_OFF_TWO_PARTITION_A0_DONE  0x81
#define ECUM_SP_SYNC_INIT_GO_OFF_TWO_PARTITION_QM_DONE  0x82
#define ECUM_SP_SYNC_INIT_GO_OFF_TWO_PARTITION_ALL_DONE  0x83

#define ECUM_SP_SYNC_GO_OFF_TWO_START  0x90
#define ECUM_SP_SYNC_GO_OFF_TWO_PARTITION_A0_DONE  0x91
#define ECUM_SP_SYNC_GO_OFF_TWO_PARTITION_QM_DONE  0x92
#define ECUM_SP_SYNC_GO_OFF_TWO_PARTITION_ALL_DONE  0x93

/* @req SWS_EcuM_04045 */
typedef uint8 EcuM_ShutdownCauseType;
/* @req SWS_EcuM_04044 */
typedef uint8 EcuM_ResetType;

#endif /* ECUM_TYPES_H_ */
/** @} */
