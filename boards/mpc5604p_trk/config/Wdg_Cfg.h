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

#ifndef WDG_CFG_H_
#define WDG_CFG_H_

#include "Std_Types.h"
#include "WdgIf_Types.h"

#define WDG_INDEX		0

typedef struct
{
   uint32 ReloadValue;
   uint8 ActivationBit;
}Wdg_SettingsType;

typedef struct
{
	WdgIf_ModeType Wdg_DefaultMode;
	Wdg_SettingsType WdgSettingsFast;
	Wdg_SettingsType WdgSettingsSlow;
	Wdg_SettingsType WdgSettingsOff;
}Wdg_ModeConfigType;

typedef struct
{
	const Wdg_ModeConfigType *Wdg_ModeConfig;
}Wdg_ConfigType;

 extern const Wdg_ConfigType WdgConfig;

#endif /* WDG_CFG_H_ */
