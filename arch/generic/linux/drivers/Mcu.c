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


#include <assert.h>
#include <string.h>
#include "Std_Types.h"
#include "Mcu.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#if defined(USE_DEM)
#include "Dem.h"
#endif
#include "Cpu.h"
#include "Ramlog.h"
#include "Os.h"
#include "isr.h"

//#define USE_LDEBUG_PRINTF 1
#include "debug.h"


void Mcu_Init(const Mcu_ConfigType *configPtr)
{

}

void Mcu_SetMode( Mcu_ModeType McuMode ) {
	(void)McuMode;
}

/**
 * Get the system clock in Hz. It calculates the clock from the
 * different register settings in HW.
 */
uint32_t McuE_GetSystemClock(void)
{
  return 10;
}
Std_ReturnType Mcu_InitClock( const Mcu_ClockType ClockSetting ) {
  return E_OK;
}

void Mcu_PerformReset( void ) {

}

Mcu_PllStatusType Mcu_GetPllStatus( void ) {
	return MCU_PLL_LOCKED;
}
