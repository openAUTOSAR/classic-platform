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

#define WDG_INDEX		0
#define WDG_VERSION_INFO_API    STD_ON
#define WDG_DEV_ERROR_DETECT    STD_ON


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
