/*
 * WdgIf.h
 *
 *  Created on: 22 feb 2010
 *      Author: Fredrik
 */

#ifndef WDGIF_H_
#define WDGIF_H_

#include "Std_Types.h"
#include "WdgIf_Cfg.h"

#define WDGIF_SETMODE_ID             0x01
#define WDGIF_TRIGGER_ID             0x02

#define WDGIF_E_PARAM_DEVICE         0x01

Std_ReturnType WdgIf_SetMode (uint8 DeviceIndex, WdgIf_ModeType Mode);

void WdgIf_Trigger (uint8 DeviceIndex);

#endif /* WDGIF_H_ */
