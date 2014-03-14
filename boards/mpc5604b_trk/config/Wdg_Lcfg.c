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

#include "Wdg.h"

const Wdg_ModeConfigType WdgModeConfig =
{
	.Wdg_DefaultMode = WDGIF_OFF_MODE,
	.WdgSettingsFast =
	{
		.ReloadValue = 200,  // ms
		.ActivationBit = 1,
	},
	.WdgSettingsSlow =
	{
		.ReloadValue = 2000,  // ms
		.ActivationBit = 1,
	},
	.WdgSettingsOff =
	{
		.ReloadValue = 0,
		.ActivationBit = 0,
	},
};

const Wdg_ConfigType WdgConfig =
{
  .Wdg_ModeConfig = &WdgModeConfig,
};


