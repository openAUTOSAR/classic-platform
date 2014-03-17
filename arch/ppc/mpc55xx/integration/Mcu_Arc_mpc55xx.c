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

/* ----------------------------[information]----------------------------------*/
/*
 * Author: mahi
 *
 * Description:
 *
 */


/* ----------------------------[includes]------------------------------------*/


#include "Std_Types.h"
#include "Mcu.h"
#include "io.h"
#include "mpc55xx.h"
#include "Mcu_Arc.h"
#if defined(USE_FEE)
#include "Fee_Memory_Cfg.h"
#endif
#if defined(USE_DMA)
#include "Dma.h"
#endif
#include "asm_ppc.h"
#include "Os.h"

/* ----------------------------[private define]------------------------------*/

/* ----------------------------[private macro]-------------------------------*/

#if defined (CFG_MPC5668)
#define ECSM_BASE 	0xfff40000
#define ECSM_ESR    0x47
#endif


/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/

#if defined(USE_FLS)
extern uint32 EccErrReg;
#endif

/* ----------------------------[private functions]---------------------------*/

#if defined(CFG_MCU_ARC_CONFIG)
void Mcu_Arc_InitMM( void ) {
	/* User: Setup TLBs if needed  */
	MM_TlbSetup( Mcu_Arc_ConfigData.tblTable );

	/* User: Enable caches if any */
#if defined(CFG_MPC5567) || defined(CFG_MPC5668)
	Cache_EnableU();
#endif

}
#endif


void Mcu_Arc_InitPre( const Mcu_ConfigType *configPtr ) {
	(void)configPtr;

}
/*
 * Called at a very early stage...
 */
void Mcu_Arc_InitPost( const Mcu_ConfigType *configPtr ) {
	(void)configPtr;
#if defined(CFG_MCU_ARC_CONFIG)
	Mcu_Arc_InitMM();
#elif !defined(CFG_MCU_ARC_CONFIG) && defined(CFG_MPC5567)
	Cache_EnableU();
#endif
}


/* ----------------------------[public functions]----------------------------*/

void Mcu_Arc_InitClockPre( const Mcu_ClockSettingConfigType *clockSettingsPtr )
{
}


void Mcu_Arc_InitClockPost( const Mcu_ClockSettingConfigType *clockSettingsPtr )
{
}

#if defined(CFG_MCU_ARC_LP)
/**
 *
 * @param mcuMode The mcuMode from Mcu_SetMode()
 */
void Mcu_Arc_SetModePre( Mcu_ModeType mcuMode)
{
#if defined(CFG_MCU_ARC_CONFIG)
	Mcu_Arc_SetModePre2(mcuMode, Mcu_Arc_ConfigData.sleepConfig );
#endif
}

/**
 *
 * @param mcuMode The mcuMode from Mcu_SetMode()
 */
void Mcu_Arc_SetModePost( Mcu_ModeType mcuMode)
{
#if defined(CFG_MCU_ARC_CONFIG)
	Mcu_Arc_SetModePost2(mcuMode,  Mcu_Arc_ConfigData.sleepConfig);
#endif
}

#endif

