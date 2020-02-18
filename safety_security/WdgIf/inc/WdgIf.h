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

/* globally fullfilled reqs */
/** @req SWS_WdgIf_00003 */ /*The Watchdog Interface shall not add functionality to the watchdog drivers*/
/** @req SWS_WdgIf_00028 */ /*If more than one watchdog driver is configured and the default error detection is enabled for this module, the parameter DeviceIndex shall be checked for being an existing device within the module’s services*/
/** @req SWS_WdgIf_00017 */ /*The Watchdog Interface shall map the APIs specified in this chapter to the API of the underlying drivers*/
/** @req ARC_SWS_WdgIf_00002 */ /*The file structure. It is inspired by the AUTOSAR file structure but do have some deviations*/
/** @req SWS_WdgIf_00001 */ /* The Watchdog Interface shall comprise a header file WdgIf.h declaring the API of the Watchdog Interface*/

#ifndef WDGIF_H_
#define WDGIF_H_

#define WDGIF_AR_RELEASE_MAJOR_VERSION   	4u
#define WDGIF_AR_RELEASE_MINOR_VERSION   	3u
#define WDGIF_AR_RELEASE_REVISION_VERSION   0u

/* @req SWS_BSW_00059 Published information */
#define WDGIF_VENDOR_ID          60u
#define WDGIF_MODULE_ID          43u
#define WDGIF_AR_MAJOR_VERSION   WDGIF_AR_RELEASE_MAJOR_VERSION
#define WDGIF_AR_MINOR_VERSION   WDGIF_AR_RELEASE_MINOR_VERSION
#define WDGIF_AR_PATCH_VERSION   WDGIF_AR_RELEASE_REVISION_VERSION

#define WDGIF_SW_MAJOR_VERSION   2u
#define WDGIF_SW_MINOR_VERSION   0u
#define WDGIF_SW_PATCH_VERSION   0u

/** @req SWS_WdgIf_00049 */
/** @req SWS_WdgIf_00050 */
#include "WdgIf_Cfg.h"

#include "Wdg.h"

#define WDGIF_SETMODE_ID             	0x01u
#define WDGIF_SETTRIGGERCONDITION_ID    0x02u
#define WDGIF_GETVERSIONINFO_ID			0x03u

/** @req SWS_WdgIf_00006 */
/** @req SWS_WdgIf_00030 */
/** @req SWS_BSW_00201 Development errors should be of type uint8 */
/** @req SWS_BSW_00073 Implementation specific errors */
#define WDGIF_E_PARAM_DEVICE         0x01u
#define WDGIF_E_INV_POINTER			 0x02u

extern const WdgIf_ConfigType WdgIfConfig;

/** @req SWS_WdgIf_00016 */
/** @req SWS_WdgIf_00018 */
/** @req SWS_WdgIf_00013 */
/** @req SWS_WdgIf_00042 */
#if (WDGIF_DEVICES > 1)
/** Map the service WdgIf_SetMode to the service Wdg_SetMode of the corresponding Watchdog Driver.
 * @param[in]   DeviceIndex Identifies the Watchdog Driver instance.
 * @param[in]   Mode The watchdog driver mode (see Watchdog Driver).
 */
Std_ReturnType WdgIf_SetMode(uint8 DeviceIndex, WdgIf_ModeType Mode);

/** @req SWS_WdgIf_00044 */
/** Map the service WdgIf_SetTriggerCondition to the service Wdg_SetTriggerCondition of the corresponding Watchdog Driver.
 * @param[in]   DeviceIndex Identifies the Watchdog Driver instance.
 * @param[in]   Timeout Timeout value (milliseconds) for setting the trigger counter.
 */
void WdgIf_SetTriggerCondition(uint8 DeviceIndex, uint16 Timeout);

#else

/** @req SWS_WdgIf_00019 */
#define WdgIf_SetMode(DeviceIndex, Mode) \
    WdgIfConfig.WdgIf_Device[0u].WdgRef->Wdg_SetModeLocationPtr(Mode)

#define WdgIf_SetTriggerCondition(DeviceIndex, Timeout) \
    WdgIfConfig.WdgIf_Device[0u].WdgRef->Wdg_TriggerLocationPtr(Timeout)
#endif

#if (WDGIF_VERSION_INFO_API == STD_ON)
/** @req SWS_WdgIf_00046 */
/** Returns the version information.
 * @param[out]   *VersionInfoPtr Pointer to where to store the version information of this module.
 */
void WdgIf_GetVersionInfo(Std_VersionInfoType *VersionInfoPtr);
#endif
#endif /* WDGIF_H_ */
