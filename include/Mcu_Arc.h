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

#ifndef MCU_ARC_H_
#define MCU_ARC_H_

#include "Mcu_Arc_Cfg.h"

void 	 Mcu_Arc_InitClockPre( const Mcu_ClockSettingConfigType *clockSettingsPtr );
void 	 Mcu_Arc_InitClockPost( const Mcu_ClockSettingConfigType *clockSettingsPtr );
void 	 Mcu_Arc_SetModePre( Mcu_ModeType mcuMode);
void  	 Mcu_Arc_SetModePost( Mcu_ModeType mcuMode);
uint32_t Mcu_Arc_ExceptionHook(uint32_t exceptionVector);

void Mcu_Arc_SetModePre2( Mcu_ModeType mcuMode, const struct Mcu_Arc_SleepConfig *sleepCfg );
void Mcu_Arc_SetModePost2( Mcu_ModeType mcuMode, const struct Mcu_Arc_SleepConfig *sleepCfg );
void Mcu_Arc_LowPowerRecoverFlash( void );
//void Mcu_Arc_InitMM( void );

#endif /* MCU_ARC_H_ */
