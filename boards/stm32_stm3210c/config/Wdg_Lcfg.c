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

#if defined(CFG_STM32F1X)
void Wdg_IWDG_Init (const Wdg_IWDG_ConfigType* ConfigPtr);
void Wdg_IWDG_Trigger (uint16 timeout);
Std_ReturnType Wdg_IWDG_SetMode (WdgIf_ModeType Mode);
void Wdg_IWDG_GetVersionInfo (void /* IMPROVEMENT Std_VersionInfoType* versioninfo*/);

void Wdg_WWDG_Init (const Wdg_WWDG_ConfigType* ConfigPtr);
void Wdg_WWDG_Trigger (uint16 timeout);
Std_ReturnType Wdg_WWDG_SetMode (WdgIf_ModeType Mode);
void Wdg_WWDG_GetVersionInfo (void /* IMPROVEMENT Std_VersionInfoType* versioninfo*/);
#endif


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

/* IMPROVEMENT: Add implementation for Independent WD as well. This will make it
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

