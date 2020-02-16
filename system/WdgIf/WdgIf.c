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

// 904 PC-Lint MISRA 14.7: OK. Allow VALIDATE_ENTITY_ID, VALIDATE and VALIDATE_NO_RETURNVAL to return value.
//lint -emacro(904,VALIDATE,VALIDATE_NO_RETURNVAL)

/** @req WDGIF051 */
/** @req WDGIF056 Only supporting pre compile */


#include "WdgIf.h"


/* @req WDGIF005 */
#if !(((WDGIF_SW_MAJOR_VERSION == 1) && (WDGIF_SW_MINOR_VERSION == 0)) )
#error WdgIf: Expected BSW module version to be 1.0.*
#endif

/* @req 4.0.3/WDGM013 */
#if !(((WDGIF_AR_RELEASE_MAJOR_VERSION == 4) && (WDGIF_AR_RELEASE_MINOR_VERSION == 0)) )
#error WdgIf: Expected AUTOSAR version to be 4.0.*
#endif



/** @req WDGIF007 */
#if (WDGIF_DEV_ERROR_DETECT == STD_ON)
#include "Det.h"
#endif

#if (WDGIF_DEV_ERROR_DETECT == STD_ON)

#define VALIDATE(_expr, _api, _errorcode) \
	if(!_expr) {	\
		Det_ReportError(MODULE_ID_WDGIF,0,_api,_errorcode ); \
		return E_NOT_OK;	\
		}

#define VALIDATE_NO_RETURNVAL(_expr, _api, _errorcode) \
	if(!_expr) {	\
		Det_ReportError(MODULE_ID_WDGIF,0,_api,_errorcode ); \
		return;	\
		}

#endif

#if (WDGIF_DEVICES > 1)

/** @req WDGIF020 */
Std_ReturnType WdgIf_SetMode (uint8 DeviceIndex, WdgIf_ModeType Mode)
{
#if (WDGIF_DEV_ERROR_DETECT == STD_ON)
	VALIDATE((WdgIfConfig.WdgIf_General->WdgIf_NumberOfDevices > DeviceIndex), WDGIF_SETMODE_ID, WDGIF_E_PARAM_DEVICE);
#endif
	/** @req WDGIF043 */
	/** @req WDGIF057 */
	return WdgIfConfig.WdgIf_Device[DeviceIndex].WdgRef->Wdg_SetModeLocationPtr(Mode);
}

#endif

void WdgIf_SetTriggerCondition(uint8 DeviceIndex, uint16 Timeout)
{
#if (WDGIF_DEV_ERROR_DETECT == STD_ON)
	VALIDATE_NO_RETURNVAL((WdgIfConfig.WdgIf_General->WdgIf_NumberOfDevices > DeviceIndex), WDGIF_SETTRIGGERCONDITION_ID, WDGIF_E_PARAM_DEVICE);
#endif
	/** @req WDGIF045 */
	WdgIfConfig.WdgIf_Device[DeviceIndex].WdgRef->Wdg_TriggerLocationPtr(Timeout);
}

/** @req WDGIF036 */
#if (WDGIF_VERSION_INFO_API == STD_ON)
void WdgIf_GetVersionInfo(Std_VersionInfoType *VersionInfoPtr)
{
	/** @ req WDGIF058 */
#if (WDGIF_DEV_ERROR_DETECT == STD_ON)
	VALIDATE_NO_RETURNVAL((VersionInfoPtr != NULL_PTR), WDGIF_GETVERSIONINFO_ID, WDGIF_E_INV_POINTER);
#endif

	/** @req WDGIF035 */
	VersionInfoPtr->moduleID = MODULE_ID_WDGIF;
	VersionInfoPtr->vendorID = 0;
	VersionInfoPtr->sw_major_version = WDGIF_SW_MAJOR_VERSION;
	VersionInfoPtr->sw_minor_version = WDGIF_SW_MINOR_VERSION;
	VersionInfoPtr->sw_patch_version = WDGIF_SW_PATCH_VERSION;
}
#endif
