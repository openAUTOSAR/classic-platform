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
 * -------------------------------- Arctic Core ------------------------------
 * WdgIf.c
 *
 *  Created on: 22 feb 2010
 *      Author: Fredrik
 */

// 904 PC-Lint MISRA 14.7: OK. Allow VALIDATE_ENTITY_ID, VALIDATE and VALIDATE_NO_RETURNVAL to return value.
//lint -emacro(904,VALIDATE,VALIDATE_NO_RETURNVAL)

#include "WdgIf.h"
#if defined(USE_DET)
#include "Det.h"
#endif

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
