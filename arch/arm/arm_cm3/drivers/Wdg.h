/*
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
void Wdg_IWDG_SetMode (WdgIf_ModeType Mode);
void Wdg_IWDG_GetVersionInfo (void /* TODO Std_VersionInfoType* versioninfo*/);

void Wdg_WWDG_Init (const Wdg_WWDG_ConfigType* ConfigPtr);
void Wdg_WWDG_Trigger (void);
void Wdg_WWDG_SetMode (WdgIf_ModeType Mode);
void Wdg_WWDG_GetVersionInfo (void /* TODO Std_VersionInfoType* versioninfo*/);

void Wdg_Init (const Wdg_ConfigType* ConfigPtr);

#endif /* WDG_H_ */
