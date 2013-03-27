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
void Wdg_Trigger (void);
Std_ReturnType Wdg_SetMode (WdgIf_ModeType Mode);

#if (WDG_VERSION_INFO_API == STD_ON)
void Wdg_GetVersionInfo( Std_VersionInfoType *versionInfo );
#define Wdg_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,WDG)
#endif

#endif /* WDG_H_ */
