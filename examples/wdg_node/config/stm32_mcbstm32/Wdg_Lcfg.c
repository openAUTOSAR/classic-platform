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

const Wdg_IWDG_ModeConfigType WdgIWDGModeConfig =
{
	.Wdg_DefaultMode = WDGIF_OFF_MODE,
	.WdgSettingsFast =
	{
			.TimerBase = IWDG_CK_Counter_Clock_8,
			.ReloadValue = 0x7FF,
			.ActivationBit = 1,
	},
	.WdgSettingsSlow =
	{
			.TimerBase = IWDG_CK_Counter_Clock_256,
			.ReloadValue = 0xFFF,
			.ActivationBit = 1,
	},
	.WdgSettingsOff =
	{
			.TimerBase = IWDG_CK_Counter_Clock_4,
			.ReloadValue = 0x7F,
			.ActivationBit = 0,
	},
};

const Wdg_GeneralType WdgIWDGGeneral =
{
	.Wdg_Index = 1,
	.Wdg_TriggerLocationPtr = Wdg_IWDG_Trigger,
	.Wdg_SetModeLocationPtr = Wdg_IWDG_SetMode,
};


const Wdg_IWDG_ConfigType WdgIWDGConfig =
{
  .Wdg_General = &WdgIWDGGeneral,
  .Wdg_IWDGModeConfig = &WdgIWDGModeConfig,
};


/* The windowed watchdog is clocked from PCLK1. Max allowed frequency
 * of this is 36Mhz.
 *
 * Max Twwdg = T_PCLK1 * 4096 * 2^TimerBase * (CounterPreset & 0x3F) + 1)=
 *
 * = 58.25 ms
 *
 *
 *
 * Min Twwdg = T_PCLK1 * 4096 * 2^TimerBase * (CounterPreset & 0x3F) + 1)=
 *
 * = 7.28 ms
 * */

/* TODO: Add implementation for Independent WD as well. This will make it
 * possible to test a multiple WD design within the STM32.  */
const Wdg_WWDG_ModeConfigType WdgWWDGModeConfig =
{
	.Wdg_DefaultMode = WDGIF_OFF_MODE,
	.WdgSettingsFast =
	{
			.TimerBase = WDG_CK_Counter_Clock_4,
			.WindowValue = 0x7F,
			.CounterPreset = 0x7F,
			.ActivationBit = 1,
	},
	.WdgSettingsSlow =
	{
			.TimerBase = WDG_CK_Counter_Clock_8,
			.WindowValue = 0x7F,
			.CounterPreset = 0x7F,
			.ActivationBit = 1,
	},
	.WdgSettingsOff =
	{
			.TimerBase = WDG_CK_Counter_Clock_8,
			.WindowValue = 0x7F,
			.CounterPreset = 0x7F,
			.ActivationBit = 0,
	},
};
const Wdg_GeneralType WdgWWDGGeneral =
{
	.Wdg_Index = 0,
	.Wdg_TriggerLocationPtr = Wdg_WWDG_Trigger,
	.Wdg_SetModeLocationPtr = Wdg_WWDG_SetMode,
};

const Wdg_WWDG_ConfigType WdgWWDGConfig =
{
  .Wdg_General = &WdgWWDGGeneral,
  .Wdg_WWDGModeConfig = &WdgWWDGModeConfig,
};

const Wdg_ConfigType WdgConfig =
{
	&WdgIWDGConfig,
	&WdgWWDGConfig,
};
