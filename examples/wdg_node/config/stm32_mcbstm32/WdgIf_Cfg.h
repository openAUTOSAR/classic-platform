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

#ifndef WDGIF_CFG_H_
#define WDGIF_CFG_H_

#include "Std_Types.h"
#include "WdgIf_Types.h"

typedef struct
{
	uint8 WdgIf_DeviceIndex;
	const Wdg_GeneralType *WdgRef;
}WdgIf_DeviceType;

typedef struct
{
	uint8 WdgIf_NumberOfDevices;
}WdgIf_GeneralType;

typedef struct
{
	const WdgIf_GeneralType   *WdgIf_General;
	const WdgIf_DeviceType    *WdgIf_Device;
}WdgIf_ConfigType;

extern const WdgIf_DeviceType WdgIfDevice[];
extern const WdgIf_ConfigType WdgIfConfig;
#endif /* WDGIF_CFG_H_ */
