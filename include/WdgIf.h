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

/* globally fullfilled reqs */
/** @req WDGIF003 */
/** @req WDGIF031 */
/** @req WDGIF009 */
/** @req WDGIF032 */
/** @req WDGIF028 */
/** @req WDGIF010 */
/** @req WDGIF017 */
/** @req WDGIF034 */
/** @req WDGIF037 */

/** @req WDGIF001 */

/** @req WDGIF052 */
/** @req WDGIF053 */
/** @req WDGIF054 */
/** @req WDGIF055 */

#ifndef WDGIF_H_
#define WDGIF_H_

#define WDGIF_AR_RELEASE_MAJOR_VERSION   	4
#define WDGIF_AR_RELEASE_MINOR_VERSION   	0
#define WDGIF_AR_RELEASE_REVISION_VERSION   3

#define WDGIF_VENDOR_ID          60
#define WDGIF_MODULE_ID          MODULE_ID_WDGIF
#define WDGIF_AR_MAJOR_VERSION   WDGIF_AR_RELEASE_MAJOR_VERSION
#define WDGIF_AR_MINOR_VERSION   WDGIF_AR_RELEASE_MINOR_VERSION
#define WDGIF_AR_PATCH_VERSION   WDGIF_AR_RELEASE_REVISION_VERSION

#define WDGIF_SW_MAJOR_VERSION   1
#define WDGIF_SW_MINOR_VERSION   0
#define WDGIF_SW_PATCH_VERSION   0


/** @req WDGIF002 */ /** wdgif_types is included by cfg.... */

#include "Modules.h"
/** @req WDGIF041 */
#include "Std_Types.h"
/** @req WDGIF049 */
/** @req WDGIF050 */
#include "WdgIf_Cfg.h"

#if (WDGIF_DEVICES == 1)
#include "Wdg.h"
#endif

#define WDGIF_SETMODE_ID             	0x01u
#define WDGIF_SETTRIGGERCONDITION_ID    0x02u
#define WDGIF_GETVERSIONINFO_ID			0x03u

/** @req WDGIF006 */
/** @req WDGIF030 */
#define WDGIF_E_PARAM_DEVICE         0x01u
#define WDGIF_E_INV_POINTER			 0x02u

/** @req WDGIF016 */
/** @req WDGIF018 */
/** @req WDGIF013 */
/** @req WDGIF042 */
#if (WDGIF_DEVICES > 1)
Std_ReturnType WdgIf_SetMode(uint8 DeviceIndex, WdgIf_ModeType Mode);

#else

/** @req WDGIF019 */
#define WdgIf_SetMode(DeviceIndex, Mode) \
    Wdg_SetMode(Mode)
#endif

/** @req WDGIF044 */
void WdgIf_SetTriggerCondition(uint8 DeviceIndex, uint16 Timeout);

/** @req WDGIF046 */
void WdgIf_GetVersionInfo(Std_VersionInfoType *VersionInfoPtr);

#endif /* WDGIF_H_ */
