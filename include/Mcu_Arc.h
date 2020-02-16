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

#ifndef MCU_ARC_H_
#define MCU_ARC_H_

#include "Mcu_Arc_Cfg.h"
#include "mm.h"

void 	 Mcu_Arc_InitPre( const Mcu_ConfigType *configPtr );
void 	 Mcu_Arc_InitPost( const Mcu_ConfigType *configPtr );

void 	 Mcu_Arc_InitClockPre( const Mcu_ClockSettingConfigType *clockSettingsPtr );
void 	 Mcu_Arc_InitClockPost( const Mcu_ClockSettingConfigType *clockSettingsPtr );

void  	 Mcu_Arc_SetModePre( Mcu_ModeType mcuMode);
void  	 Mcu_Arc_SetModePost( Mcu_ModeType mcuMode);

/* Sleep is usually unique for each MCU */
#if defined(CFG_MCU_ARC_CONFIG)
void Mcu_Arc_SetModePre2( Mcu_ModeType mcuMode, const struct Mcu_Arc_SleepConfig *sleepCfg );
void Mcu_Arc_SetModePost2( Mcu_ModeType mcuMode, const struct Mcu_Arc_SleepConfig *sleepCfg );
void Mcu_Arc_LowPowerRecoverFlash( void );
#endif

void Mcu_Arc_InitMM( void );

#if defined(CFG_PPC)
uint32_t Mpc5xxx_ExceptionHandler(uint32_t exceptionVector);
void 	 Mpc5xxx_Panic( uint32 error, void *pData );
#endif

//void Mcu_Arc_InitMM( void );

#endif /* MCU_ARC_H_ */
