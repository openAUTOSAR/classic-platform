/*
 * WdgIf.c
 *
 *  Created on: 22 feb 2010
 *      Author: Fredrik
 */

#include "WdgIf.h"


void WdgIf_SetMode (uint8 DeviceIndex, WdgIf_ModeType Mode)
{
	WdgIfConfig.WdgIf_Device[DeviceIndex].WdgRef->Wdg_SetModeLocationPtr(Mode);
}

void WdgIf_Trigger (uint8 DeviceIndex)
{
	WdgIfConfig.WdgIf_Device[DeviceIndex].WdgRef->Wdg_TriggerLocationPtr();
}
