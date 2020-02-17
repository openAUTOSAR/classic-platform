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

#include "Wdg.h"

const Wdg_ModeConfigType WdgModeConfig =
{
    .Wdg_DefaultMode = WDGIF_OFF_MODE,
    .WdgSettingsFast =
    {
        .ReloadValue = 0x280,	// 5 ms
        .ActivationBit = 1,
    },
    .WdgSettingsSlow =
    {
        .ReloadValue = 0xA00,	// 20 ms
        .ActivationBit = 1,
    },
    .WdgSettingsOff =
    {
        .ReloadValue = 0x7D00,
        .ActivationBit = 0,
    },
};

const Wdg_ConfigType WdgConfig =
{
  .Wdg_ModeConfig = &WdgModeConfig,
};


