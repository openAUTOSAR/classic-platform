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
 * -------------------------------- Arctic Core ------------------------------*/

/* ----------------------------[information]----------------------------------*/
/*
 * Author: Fredrik
 *
 * Part of Release:
 *   3.1.5
 */

#ifndef WDGIF_H_
#define WDGIF_H_

#define WDGIF_SW_MAJOR_VERSION	1
#define WDGIF_SW_MINOR_VERSION	0
#define WDGIF_SW_PATCH_VERSION	0

#include "Modules.h"
#include "Std_Types.h"
#include "WdgIf_Cfg.h"

#define WDGIF_SETMODE_ID             0x01
#define WDGIF_TRIGGER_ID             0x02

#define WDGIF_E_PARAM_DEVICE         0x01

Std_ReturnType WdgIf_SetMode (uint8 DeviceIndex, WdgIf_ModeType Mode);

void WdgIf_Trigger (uint8 DeviceIndex);

#endif /* WDGIF_H_ */
