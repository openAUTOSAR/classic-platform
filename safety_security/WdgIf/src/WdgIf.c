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

/** @req SWS_WdgIf_00026 */ /*The Watchdog Interface provides uniform access to services of the underlying watchdog drivers*/
/** @req SWS_WdgIf_00047 */ /* Wdg_SetMode and Wdg_SetTriggerCondition are available*/


#include "WdgIf.h"

#if !(((WDGIF_SW_MAJOR_VERSION == 2u) && (WDGIF_SW_MINOR_VERSION == 0u)) )
#error WdgIf: Expected BSW module version to be 2.0.*
#endif

#if !(((WDGIF_AR_RELEASE_MAJOR_VERSION == 4u) && (WDGIF_AR_RELEASE_MINOR_VERSION == 3u)) )
#error WdgIf: Expected AUTOSAR version to be 4.3.*
#endif



#if (WDGIF_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif
/*lint -e904 MISRA:STANDARDIZED_INTERFACE:argument check:[MISRA 2012 Rule 15.5, advisory]*/
/** @req SWS_WdgIf_00048 */
/** @req SWS_BSW_00203 API parameter checking enablement. */
/** @req SWS_BSW_00042 Detection of DevErrors should only be performed if configuration parameter for Development errors is set. */
#if (WDGIF_DEV_ERROR_DETECT == STD_ON)

/* @req SWS_BSW_00045 Development errors should be reported to DET module */
/* @req SWS_BSW_00049 API parameter checking */
#define WDGIF_VALIDATE_RV(_expr, _api, _errorcode) \
    if(!_expr) {    \
        (void)Det_ReportError(WDGIF_MODULE_ID,0u,_api,_errorcode ); \
        return E_NOT_OK;    \
        }

/* @req SWS_BSW_00045 Development errors should be reported to DET module */
/* @req SWS_BSW_00049 API parameter checking */
#define WDGIF_VALIDATE_NO_RV(_expr, _api, _errorcode) \
    if(!_expr) {    \
        (void)Det_ReportError(WDGIF_MODULE_ID,0u,_api,_errorcode ); \
        return; \
        }

#else

#define WDGIF_VALIDATE_RV(_expr, _api, _errorcode) \
    if(!_expr) {    \
        return E_NOT_OK;    \
        }

#define WDGIF_VALIDATE_NO_RV(_expr, _api, _errorcode) \
    if(!_expr) {    \
        return; \
        }

#endif

#if (WDGIF_DEVICES > 1u)

/** @req SWS_WdgIf_00020 */
Std_ReturnType WdgIf_SetMode (uint8 DeviceIndex, WdgIf_ModeType Mode)
{
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    WDGIF_VALIDATE_RV((WdgIfConfig.WdgIf_General->WdgIf_NumberOfDevices > DeviceIndex), WDGIF_SETMODE_ID, WDGIF_E_PARAM_DEVICE);
    /** @req SWS_WdgIf_00043 */
    /** @req SWS_WdgIf_00057 */
    /*Wdg_SetModeLocationPtr points to the Wdg<XXX>_SetMode function for the Wdg device with index DeviceIndex*/
    return WdgIfConfig.WdgIf_Device[DeviceIndex].WdgRef->Wdg_SetModeLocationPtr(Mode);
}

void WdgIf_SetTriggerCondition(uint8 DeviceIndex, uint16 Timeout)
{
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    WDGIF_VALIDATE_NO_RV((DeviceIndex < WdgIfConfig.WdgIf_General->WdgIf_NumberOfDevices), WDGIF_SETTRIGGERCONDITION_ID, WDGIF_E_PARAM_DEVICE);
    /** @req SWS_WdgIf_00045 */
    /*Wdg_TriggerLocationPtr points to the Wdg<XXX>_SetTriggerCondition function for the Wdg device with index DeviceIndex*/
    WdgIfConfig.WdgIf_Device[DeviceIndex].WdgRef->Wdg_TriggerLocationPtr(Timeout);
}

#endif

/* @req SWS_BSW_00064 GetVersionInfo shall execute synchonously */
/* @req SWS_BSW_00052 GetVersion info shall only have one parameter */
/* @req SWS_BSW_00164 No restriction on calling context */
#if (WDGIF_VERSION_INFO_API == STD_ON)
void WdgIf_GetVersionInfo(Std_VersionInfoType *VersionInfoPtr)
{
    /** @req SWS_WdgIf_00058 */
    /* Validates parameters, and if it fails will immediately leave the function with the specified return code */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
	/* @req SWS_BSW_00212 NULL pointer check */	
    WDGIF_VALIDATE_NO_RV((VersionInfoPtr != NULL_PTR), WDGIF_GETVERSIONINFO_ID, WDGIF_E_INV_POINTER);

    /** @req SWS_WdgIf_00035 */
    VersionInfoPtr->moduleID = WDGIF_MODULE_ID;
    VersionInfoPtr->vendorID = WDGIF_VENDOR_ID;
    VersionInfoPtr->sw_major_version = WDGIF_SW_MAJOR_VERSION;
    VersionInfoPtr->sw_minor_version = WDGIF_SW_MINOR_VERSION;
    VersionInfoPtr->sw_patch_version = WDGIF_SW_PATCH_VERSION;
}
#endif
