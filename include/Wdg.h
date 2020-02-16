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

#ifndef WDG_H_
#define WDG_H_

#include "Wdg_Cfg.h"

#if defined(CFG_ARM_CM3)
void Wdg_IWDG_Init (const Wdg_IWDG_ConfigType* ConfigPtr);
void Wdg_IWDG_Trigger (void);
Std_ReturnType Wdg_IWDG_SetMode (WdgIf_ModeType Mode);
void Wdg_IWDG_GetVersionInfo (void /* TODO Std_VersionInfoType* versioninfo*/);

void Wdg_WWDG_Init (const Wdg_WWDG_ConfigType* ConfigPtr);
void Wdg_WWDG_Trigger (void);
Std_ReturnType Wdg_WWDG_SetMode (WdgIf_ModeType Mode);
void Wdg_WWDG_GetVersionInfo (void /* TODO Std_VersionInfoType* versioninfo*/);
#endif

void Wdg_Init (const Wdg_ConfigType* ConfigPtr);
void Wdg_SetTriggerCondition (uint16 timeout);
Std_ReturnType Wdg_SetMode (WdgIf_ModeType Mode);

#if (WDG_VERSION_INFO_API == STD_ON)
void Wdg_GetVersionInfo( Std_VersionInfoType *versionInfo );
#define Wdg_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,WDG)
#endif

#endif /* WDG_H_ */
