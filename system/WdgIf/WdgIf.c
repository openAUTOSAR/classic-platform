/*
 * WdgIf.c
 *
 *  Created on: 22 feb 2010
 *      Author: Fredrik
 */

#include "WdgIf.h"
#include "Det.h"

#define VALIDATE(_expr, _api, _errorcode) \
	if(!_expr) {	\
		Det_ReportError(MODULE_ID_WDGIF,0,_api,_errorcode ); \
		ret = E_NOT_OK;	\
		return ret;	\
		}

#define VALIDATE_NO_RETURNVAL(_expr, _api, _errorcode) \
	if(!_expr) {	\
		Det_ReportError(MODULE_ID_WDGIF,0,_api,_errorcode ); \
		return;	\
		}


Std_ReturnType WdgIf_SetMode (uint8 DeviceIndex, WdgIf_ModeType Mode)
{
	Std_ReturnType ret = E_NOT_OK;
	VALIDATE((WdgIfConfig.WdgIf_General->WdgIf_NumberOfDevices > DeviceIndex), WDGIF_SETMODE_ID, WDGIF_E_PARAM_DEVICE);

	return WdgIfConfig.WdgIf_Device[DeviceIndex].WdgRef->Wdg_SetModeLocationPtr(Mode);
}

void WdgIf_Trigger (uint8 DeviceIndex)
{
	VALIDATE_NO_RETURNVAL((WdgIfConfig.WdgIf_General->WdgIf_NumberOfDevices > DeviceIndex), WDGIF_TRIGGER_ID, WDGIF_E_PARAM_DEVICE);
	WdgIfConfig.WdgIf_Device[DeviceIndex].WdgRef->Wdg_TriggerLocationPtr();
}
