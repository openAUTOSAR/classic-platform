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


#ifndef WDGM_H
#define WDGM_H

#define WDGM_AR_RELEASE_MAJOR_VERSION       4u
#define WDGM_AR_RELEASE_MINOR_VERSION       3u
#define WDGM_AR_RELEASE_REVISION_VERSION    0u

/* @req SWS_BSW_00059 Published information */
#define WDGM_VENDOR_ID          60u
#define WDGM_MODULE_ID          13u
#define WDGM_AR_MAJOR_VERSION   WDGM_AR_RELEASE_MAJOR_VERSION
#define WDGM_AR_MINOR_VERSION   WDGM_AR_RELEASE_MINOR_VERSION
#define WDGM_AR_PATCH_VERSION   WDGM_AR_RELEASE_REVISION_VERSION

#define WDGM_SW_MAJOR_VERSION   3u
#define WDGM_SW_MINOR_VERSION   0u
#define WDGM_SW_PATCH_VERSION   0u


/* globally fulfilled requirements */
/** @req SWS_WdgM_00200 */ /*The Watchdog Manager module shall track the Local Supervision Status of each Supervised Entity*/
/** @req SWS_WdgM_00085 */ /*The Watchdog Manager module shall derive the required number of independent data resources to perform the Alive Supervision*/
/** @req SWS_WdgM_00293 */ /*The Watchdog Manager module shall derive the required number of independent data resources to perform the Deadline Supervision*/
/** @req ARC_SWS_WdgM_00014 */ /* ARCCORE defined file structure.  */

#define WDGM_SID_INIT                   0x00u
#define WDGM_SID_DEINIT                 0x01u
#define WDGM_SID_GETVERSIONINFO         0x02u
#define WDGM_SID_SETMODE                0x03u
#define WDGM_SID_GETMODE                0x0Bu
#define WDGM_SID_CHECKPOINTREACHED      0x0Eu
#define WDGM_SID_UPDATEALIVECOUNTER     0x04u
#define WDGM_SID_GETLOCALSTATUS         0x0Cu
#define WDGM_SID_GETGLOBALSTATUS        0x0Du
#define WDGM_SID_PERFORMRESET           0x0Fu
#define WDGM_SID_GETFIRSTEXPIREDSEID    0x10u
#define WDGM_SID_MAINFUNCTION           0x08u

/** @req SWS_WdgM_00004 */
/** @req SWS_BSW_00201 Development errors should be of type uint8 */
/** @req SWS_BSW_00073 Implementation specific errors */
#define WDGM_E_NO_INIT                  0x10u
#define WDGM_E_NO_DEINIT                0x1Au
#define WDGM_E_PARAM_CONFIG             0x11u
#define WDGM_E_PARAM_MODE               0x12u
#define WDGM_E_PARAM_SEID               0x13u
#define WDGM_E_INV_POINTER              0x14u
#define WDGM_E_DISABLE_NOT_ALLOWED      0x15u
#define WDGM_E_CPID                     0x16u
#define WDGM_E_DEPRECATED               0x17u
#define WDGM_E_AMBIGIOUS                0x18u
#define WDGM_E_SEDEACTIVATED            0x19u

/** @req SWS_WdgM_00369 */
#include "Rte_WdgM_Type.h"

#include "WdgM_runtimeTypes.h"
/** @req SWS_WdgM_00002 */ /*The Watchdog Manager module shall support the parallel usage of multiple watchdogs*/
/** @req SWS_WdgM_00011 */ /*The following data types are used by Watchdog Manager module*/
/** @req SWS_WdgM_00161 */ /*Required interfaces from other modules*/
/** @req SWS_WdgM_00162 */ /*Optional interfaces from other modules*/

/** @req SWS_WdgM_00151 */
/** Function which initializes the module
 * @param[in]   *ConfigPtr Pointer to post-build configuration data
 */
void WdgM_Init(const WdgM_ConfigType *ConfigPtr);

/** @req SWS_WdgM_00261 */
/** Function which deinitializes the module
 */
void WdgM_DeInit( void );

/** @req SWS_WdgM_00153 */
#if ( WDGM_VERSION_INFO_API == STD_ON )
/** Returns the version information of this module.
 * @param[out]   *ConfigPtr Pointer to where to store the version information of the module WdgM.
 */
void WdgM_GetVersionInfo( Std_VersionInfoType* VersionInfo );
#endif


/** @req SWS_WdgM_00154 */
/** Sets the current mode of Watchdog Manager.
 * @param[in]   Mode One of the configured Watchdog Manager modes.
 */
Std_ReturnType WdgM_SetMode( WdgM_ModeType Mode);

/** @req SWS_WdgM_00168 */
/** Gets the current mode of Watchdog Manager.
 * @param[out]   *Mode Current mode of the Watchdog Manager.
 */
Std_ReturnType WdgM_GetMode( WdgM_ModeType *Mode);

/** @req SWS_WdgM_00263 */
/** Indicates to the Watchdog Manager that a Checkpoint within a Supervised Entity has been reached.
 * @param[in]   SEID Identifier of the Supervised Entity that reports a Checkpoint.
 * @param[in]   CheckpointID Identifier of the Checkpoint within a Supervised Entity that has been reached.
 */
Std_ReturnType WdgM_CheckpointReached( WdgM_SupervisedEntityIdType SEID, WdgM_CheckpointIdType CheckpointID);

/** @req SWS_WdgM_00169 */
/** Returns the supervision status of an individual Supervised Entity.
 * @param[in]   SEID Identifier of the supervised entity whose supervision status shall be returned.
 * @param[out]   *Status Supervision status of the given supervised entity.
 */
Std_ReturnType WdgM_GetLocalStatus( WdgM_SupervisedEntityIdType SEID, WdgM_LocalStatusType *Status);

/** @req SWS_WdgM_00175 */
/** Returns the global supervision status of the Watchdog Manager.
 * @param[out]   *Status Global supervision status of the Watchdog Manager.
 */
Std_ReturnType WdgM_GetGlobalStatus( WdgM_GlobalStatusType *Status);

/** @req SWS_WdgM_00264 */
/** Instructs the Watchdog Manager to cause a watchdog reset.
 */
void WdgM_PerformReset( void );

/** @req SWS_WdgM_00346 */
/** Returns SEID that first reached the state WDGM_LOCAL_STATUS_EXPIRED.
 * @param[out]   *SEID Identifier of the supervised entity that first reached the state WDGM_LOCAL_STATUS_EXPIRED.
 */
Std_ReturnType WdgM_GetFirstExpiredSEID( WdgM_SupervisedEntityIdType *SEID);

/** @req SWS_WdgM_00159 */
/** Performs the processing of the cyclic Watchdog Manager jobs.
 */
void WdgM_MainFunction( void );

#if (WDGM_OFF_MODE_ENABLED != STD_OFF)
/** Checks if at least onw watchdog is enabled
 * @param[in]   Mode Current mode of the Watchdog Manager.
 * @param[in]   SID Software function ID for the function which called WdgM_internal_isAtLeastOneWdogEnabled in case none is active
 * @param[in]   errID error ID which shall be reported to DET in case none is active
 */
boolean WdgM_internal_isAtLeastOneWdogEnabled(WdgM_ModeType Mode, uint8 SID, uint8 errID);
#endif
/*lint -e9003 MISRA:OTHER:defining external variable:[MISRA 2012 Rule 8.9, advisory]*/
extern WdgM_RuntimeData WdgM_runtimeData; /* Declared in config, used in WdgM.c */
#endif
