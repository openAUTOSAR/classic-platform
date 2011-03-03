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
 * -------------------------------- Arctic Core ------------------------------
 * Wdg.h
 *
 *  Created on: 27 maj 2010
 *      Author: Fredrik
 */

#ifndef WDG_H_
#define WDG_H_

#include "Wdg_Cfg.h"

void Wdg_IWDG_Init (const Wdg_IWDG_ConfigType* ConfigPtr);
void Wdg_IWDG_Trigger (void);
Std_ReturnType Wdg_IWDG_SetMode (WdgIf_ModeType Mode);
void Wdg_IWDG_GetVersionInfo (void /* TODO Std_VersionInfoType* versioninfo*/);

void Wdg_WWDG_Init (const Wdg_WWDG_ConfigType* ConfigPtr);
void Wdg_WWDG_Trigger (void);
Std_ReturnType Wdg_WWDG_SetMode (WdgIf_ModeType Mode);
void Wdg_WWDG_GetVersionInfo (void /* TODO Std_VersionInfoType* versioninfo*/);

void Wdg_Init (const Wdg_ConfigType* ConfigPtr);

#endif /* WDG_H_ */
