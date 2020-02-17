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
/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.2 */
/** @tagSettings DEFAULT_ARCHITECTURE=PPC*/

#ifndef CANTRCV_H_
#define CANTRCV_H_

#define CANTRCV_AR_RELEASE_MAJOR_VERSION   	    4
#define CANTRCV_AR_RELEASE_MINOR_VERSION   	    1

#define CANTRCV_AR_RELEASE_REVISION_VERSION     2

#define CANTRCV_VENDOR_ID          60
#define CANTRCV_MODULE_ID          70u
#define CANTRCV_AR_MAJOR_VERSION   CANTRCV_AR_RELEASE_MAJOR_VERSION
#define CANTRCV_AR_MINOR_VERSION   CANTRCV_AR_RELEASE_MINOR_VERSION
#define CANTRCV_AR_PATCH_VERSION   CANTRCV_AR_RELEASE_REVISION_VERSION

#define CANTRCV_SW_MAJOR_VERSION   1
#define CANTRCV_SW_MINOR_VERSION   0
#define CANTRCV_SW_PATCH_VERSION   0



// API service with wrong parameterS
/** @name Error Codes */
//@{
#define CANTRCV_E_INVALID_TRANSCEIVER         0x01
#define CANTRCV_E_PARAM_POINTER               0x02
#define CANTRCV_E_UNINIT                      0x11
#define CANTRCV_E_TRCV_NOT_STANDBY            0x21
#define CANTRCV_E_TRCV_NOT_NORMAL             0x22
#define CANTRCV_E_PARAM_TRCV_WAKEUP_MODE      0x23
#define CANTRCV_E_PARAM_TRCV_OPMODE           0x24
#define CANTRCV_E_BAUDRATE_NOT_SUPPORTED      0x25
#define CANTRCV_E_NO_TRCV_CONTROL             0x26
//@}


// Service IDs
//@{
#define CANTRCV_INIT_ID                       0x00
#define CANTRCV_SET_OPMODE_ID                 0x01
#define CANTRCV_GET_OPMODE_ID                 0x02
#define CANTRCV_GET_BUSWUREASON_ID            0x03
#define CANTRCV_GET_VERSIONINFO_ID            0x04
#define CANTRCV_SET_WAKEUPMODE_ID             0x05
#define CANTRCV_GET_TRCVSYSTEMDATA_ID         0x09
#define CANTRCV_CLEAR_TRCVWUFFLAG_ID          0x0a
#define CANTRCV_READ_TRCVTIMEOUTFLAG_ID       0x0b
#define CANTRCV_CLEAR_TRCVTIMEOUTFLAG_ID      0x0c
#define CANTRCV_READ_TRCVSILENCEFLAG_ID       0x0d
#define CANTRCV_CHECK_WAKEUP_ID               0x07
#define CANTRCV_SET_PNACTIVATIONSTATE_ID      0x0f
#define CANTRCV_CHECK_WAKEFLAG_ID             0x0e
#define CANTRCV_MAINFUNCTION_ID               0x06
#define CANTRCV_MAINFUNCTION_DIAGNOSTICS_ID   0x08
//@}

/***** INCLUDES **************************************************************/
/* @req SWS_CanTrcv_00147 **/
#include "ComStack_Types.h"
#include "Can_GeneralTypes.h" /* @req SWS_CanTrcv_00162 */
#include "Std_Types.h"
#include "CanTrcv_ConfigTypes.h"
#include "CanTrcv_Cfg.h"
#include "CanTrcv_PBcfg.h"

/***** PUBLIC TYPES **********************************************************/
/** Datatype used for describing whether PN wakeup functionality in
  * CanTrcv is enabled or disabled. */
typedef enum {
	/** PN wakeup functionality in CanTrcv is enabled. */
	PN_ENABLED = 0,

	/** PN wakeup functionality in CanTrcv is disabled. */
	PN_DISABLED

} CanTrcv_PNActivationType;

/** Provides the state of a flag in the transceiver hardware. */
typedef enum {
	/** The flag is set in the transceiver hardware. */
	CANTRCV_FLAG_SET = 0,

	/** The flag is cleared in the transceiver hardware. */
	CANTRCV_FLAG_CLEARED
} CanTrcv_TrcvFlagStateType;

/***** PUBLIC FUNCTIONS ******************************************************/
/** Initializes the CanTrcv module. */
/* @req SWS_CanTrcv_00001 **/
void CanTrcv_Init(const CanTrcv_ConfigType *ConfigPtr);

/** Sets the mode of the Transceiver to the value OpMode. */
/* @req SWS_CanTrcv_00002 **/
Std_ReturnType CanTrcv_SetOpMode(uint8 Transceiver, CanTrcv_TrcvModeType OpMode);

/** Gets the mode of the Transceiver and returns it in OpMode. */
/* @req SWS_CanTrcv_00005 **/
Std_ReturnType CanTrcv_GetOpMode(uint8 Transceiver, CanTrcv_TrcvModeType* OpMode);

/** Gets the wakeup reason for the Transceiver and returns it in parameter Reason. */
/* @req  SWS_CanTrcv_00007 **/
Std_ReturnType CanTrcv_GetBusWuReason(uint8 Transceiver, CanTrcv_TrcvWakeupReasonType* reason);

/** Gets the version of the module and returns it in VersionInfo. */
/* @req  SWS_CanTrcv_00008 **/
#if ( CANTRCV_GET_VERSION_INFO == STD_ON )
#define CanTrcv_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,CANTRCV)
#endif

/** Enables, disables or clears wake-up events of the Transceiver according to TrcvWakeupMode. */
/* @req SWS_CanTrcv_00009 */
Std_ReturnType CanTrcv_SetWakeupMode(uint8 Transceiver, CanTrcv_TrcvWakeupModeType TrcvWakeupMode);

/** Reads the transceiver configuration/status data and returns it through para */
/* !req  SWS_CanTrcv_00213 **/
Std_ReturnType CanTrcv_GetTrcvSystemData(uint8 Transceiver, uint32* TrcvSysData);

/** Clears the WUF flag in the transceiver hardware. This API shall exist only if CanTrcvHwPnSupport = TRUE. */
/*  @req  SWS_CanTrcv_00214 **/
Std_ReturnType CanTrcv_ClearTrcvWufFlag(uint8 Transceiver);


/** Reads the status of the timeout flag from the transceiver hardware.
  * This API shall exist only if CanTrcvHwPnSupport = TRUE. */
/*  !req  SWS_CanTrcv_00215 **/
Std_ReturnType CanTrcv_ReadTrcvTimeoutFlag(uint8 Transceiver, CanTrcv_TrcvFlagStateType* FlagState);


/** Clears the status of the timeout flag in the transceiver hardware.
  * This API shall exist only if CanTrcvHwPnSupport = TRUE. */
/*  !req . SWS_CanTrcv_00216 **/
Std_ReturnType CanTrcv_ClearTrcvTimeoutFlag(uint8 Transceiver);

/** Reads the status of the silence flag from the transceiver hardware.
  * This API shall exist only if CanTrcvHwPnSupport = TRUE. */
/*  @req  SWS_CanTrcv_00217 **/
Std_ReturnType CanTrcv_ReadTrcvSilenceFlag(uint8 Transceiver, CanTrcv_TrcvFlagStateType* FlagState);

/** Requests to check the status of the wakeup flag from the transceiver hardware. */
/*  @req  SWS_CanTrcv_00223 **/
Std_ReturnType CanTrcv_CheckWakeFlag(uint8 Transceiver);

/** Service is called by underlying CANIF in case a wake up interrupt is detected. */
/*  @req  SWS_CanTrcv_00143 **/
Std_ReturnType CanTrcv_CheckWakeup(uint8 Transceiver);

/** The API configures the wake-up of the transceiver for Standby and Sleep Mode:
  * Either the CAN transceiver is woken up by a remote wake-up pattern
  * (standard CAN wake-up) or by the configured remote wake-up frame. */
/*  @req  SWS_CanTrcv_00219 **/
Std_ReturnType CanTrcv_SetPNActivationState(CanTrcv_PNActivationType ActivationState);

/** Service to scan all busses for wake up events and perform these event. */
/*  @req  SWS_CanTrcv_00013 **/
void CanTrcv_MainFunction(void);

/** Reads the transceiver diagnostic status periodically and sets product/development accordingly. */
/* !req SWS_CanTrcv_00218 **/
void CanTrcv_MainFunctionDiagnostics(void);

#endif /*CANTRCV_H_*/
