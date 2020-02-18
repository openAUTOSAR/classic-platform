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


#ifndef FIM_H_
#define FIM_H_

#define FIM_AR_RELEASE_MAJOR_VERSION        4u
#define FIM_AR_RELEASE_MINOR_VERSION        3u
#define FIM_AR_RELEASE_REVISION_VERSION     0u


#define FIM_MODULE_ID                      11u
#define FIM_VENDOR_ID                        60u

#define FIM_SW_MAJOR_VERSION    1u
#define FIM_SW_MINOR_VERSION    1u
#define FIM_SW_PATCH_VERSION    0u
#define FIM_AR_MAJOR_VERSION    FIM_AR_RELEASE_MAJOR_VERSION
#define FIM_AR_MINOR_VERSION    FIM_AR_RELEASE_MINOR_VERSION
#define FIM_AR_PATCH_VERSION    FIM_AR_RELEASE_REVISION_VERSION

#include "Dem.h"
#include "FiM_Types.h"
#include "FiM_Cfg.h"

#if (FIM_DEV_ERROR_DETECT == STD_ON)
/* @req SWS_Fim_00076 *//* Error codes reported by this module defined by AUTOSAR */
#define FIM_E_UNINIT                0x01u
#define FIM_E_FID_OUT_OF_RANGE      0x02u
#define FIM_E_EVENTID_OUT_OF_RANGE  0x03u
#define FIM_E_PARAM_POINTER         0x04u
#define FIM_E_INIT_FAILED           0x05u
/* ARCCORE extra errors */
#define FIM_E_REINIT                0x10u
#endif

/* API IDs */
#define FIM_INIT_ID 0x00u
#define FIM_GETFUNCTIONPERMISSION_ID 0x01u
#define FIM_SETFUNCTIONAVAILABLE_ID 0x07u
#define FIM_DEMTRIGGERONMONITORSTATUS_ID 0x02u
#define FIM_DEMINIT_ID 0x03u
#define FIM_GETVERSIONINFO_ID 0x04
#define FIM_MAINFUNCTION_ID 0x05u


/* Interface ECUState Manager <-> FiM */
void FiM_Init(const FiM_ConfigType* FiMConfigPtr);/* @req SWS_Fim_00077 */

/* Interface SW-Components <-> FiM */
Std_ReturnType FiM_GetFunctionPermission(FiM_FunctionIdType FID, boolean* Permission);/* @req SWS_Fim_00011 */
Std_ReturnType FiM_SetFunctionAvailable(FiM_FunctionIdType FID, boolean Availability);/* @req SWS_Fim_00106 */

/* Interface Dem <-> FiM */
void FiM_DemTriggerOnMonitorStatus(Dem_EventIdType EventId);/* @req SWS_Fim_00021 */
void FiM_DemInit(void);/* @req SWS_Fim_00006 */

#if (FIM_VERSION_INFO_API == STD_ON)
/* @req SWS_Fim_00078 */
void FiM_GetVersionInfo(Std_VersionInfoType* versioninfo);
#endif
void FiM_MainFunction(void);/* @req SWS_Fim_00060 */

void FiM_Shutdown(void);

#endif /* FIM_H_ */
