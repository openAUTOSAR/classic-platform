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

#ifndef WDG_CFG_H_
#define WDG_CFG_H_

#include "Std_Types.h"
#include "WdgIf_Types.h"

typedef enum
{
    IWDG_CK_Counter_Clock_4 = 0,
    IWDG_CK_Counter_Clock_8,
    IWDG_CK_Counter_Clock_16,
    IWDG_CK_Counter_Clock_32,
    IWDG_CK_Counter_Clock_64,
    IWDG_CK_Counter_Clock_128,
    IWDG_CK_Counter_Clock_256,
}Wdg_IWDG_TimerBaseType;

typedef struct
{
   Wdg_IWDG_TimerBaseType TimerBase;
   uint16 ReloadValue;
   uint8 ActivationBit;
}Wdg_IWDG_SettingsType;

typedef struct
{
    WdgIf_ModeType Wdg_DefaultMode;
    Wdg_IWDG_SettingsType WdgSettingsFast;
    Wdg_IWDG_SettingsType WdgSettingsSlow;
    Wdg_IWDG_SettingsType WdgSettingsOff;
}Wdg_IWDG_ModeConfigType;

typedef struct
{
    const Wdg_GeneralType         *Wdg_General;
    const Wdg_IWDG_ModeConfigType *Wdg_IWDGModeConfig;
}Wdg_IWDG_ConfigType;

extern const Wdg_GeneralType WdgIWDGGeneral;
extern const Wdg_IWDG_ConfigType WdgIWDGConfig;

typedef enum
{
    WDG_CK_Counter_Clock_1 = 0,
    WDG_CK_Counter_Clock_2,
    WDG_CK_Counter_Clock_4,
    WDG_CK_Counter_Clock_8,
}Wdg_WWDG_TimerBaseType;

typedef struct
{
   Wdg_WWDG_TimerBaseType TimerBase;
   uint8 WindowValue;
   uint8 CounterPreset;
   uint8 ActivationBit;
}Wdg_WWDG_SettingsType;

typedef struct
{
    WdgIf_ModeType Wdg_DefaultMode;
    Wdg_WWDG_SettingsType WdgSettingsFast;
    Wdg_WWDG_SettingsType WdgSettingsSlow;
    Wdg_WWDG_SettingsType WdgSettingsOff;
}Wdg_WWDG_ModeConfigType;

typedef struct
{
    const Wdg_GeneralType         *Wdg_General;
    const Wdg_WWDG_ModeConfigType *Wdg_WWDGModeConfig;
}Wdg_WWDG_ConfigType;

typedef struct
{
  const Wdg_IWDG_ConfigType *Wdg_IWDG_Config;
  const Wdg_WWDG_ConfigType *Wdg_WWDG_Config;
}Wdg_ConfigType;

 extern const Wdg_GeneralType WdgWWDGGeneral;
 extern const Wdg_WWDG_ConfigType WdgWWDGConfig;
 extern const Wdg_ConfigType WdgConfig;

#endif /* WDG_CFG_H_ */
