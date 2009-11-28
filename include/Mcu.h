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








#ifndef MCU_H_
#define MCU_H_

#define MCU_SW_MAJOR_VERSION	1
#define MCU_SW_MINOR_VERSION	0
#define MCU_SW_PATCH_VERSION	0

#include "Cpu.h"
#include "irq.h"
//#include "mpc55xx_aos.h"

/* Service ID's */
#define MCU_INIT_SERVICE_ID                 0x00
#define MCU_INITRAMSECTION_SERVICE_ID       0x01
#define MCU_INITCLOCK_SERVICE_ID            0x02
#define MCU_DISTRIBUTEPLLCLOCK_SERVICE_ID   0x03
#define MCU_GETPLLSTATUS_SERVICE_ID         0x04
#define MCU_GETRESETREASON_SERVICE_ID       0x05
#define MCU_GETRESETRAWVALUE_SERVICE_ID     0x06
#define MCU_PERFORMRESET_SERVICE_ID         0x07
#define MCU_SETMODE_SERVICE_ID              0x08
#define MCU_GETVERSIONINFO_SERVICE_ID       0x09
#define MCU_INTCVECTORINSTALL_SERVICE_ID    0x0A // Not in spec but follows pattern

/* Development error codes */
#define MCU_E_PARAM_CONFIG                  0x0A
#define MCU_E_PARAM_CLOCK                   0x0B
#define MCU_E_PARAM_MODE                    0x0C
#define MCU_E_PARAM_RAMSECTION              0x0D
#define MCU_E_PLL_NOT_LOCKED                0x0E
#define MCU_E_UNINIT                        0x0F

/* Specific return values */
#define MCU_GETRESETRAWVALUE_NORESETREG_RV  0x00 // MCU006
#define MCU_GETRESETRAWVALUE_UNINIT_RV      0xffffffff // MCU135

typedef enum {
	MCU_PLL_LOCKED,
	MCU_PLL_UNLOCKED,
	MCU_PLL_STATUS_UNDEFINED,
} Mcu_PllStatusType;


typedef enum {
	MCU_MODE_NORMAL=0
} Mcu_ModeType;

//TODO
typedef uint8_t Mcu_RamSectionType;

typedef uint32_t Mcu_RawResetType;

typedef enum {
	MCU_POWER_ON_RESET,
	MCU_WATCHDOG_RESET,
	MCU_SW_RESET,
	MCU_RESET_UNDEFINED
} Mcu_ResetType;

#include "Mcu_Cfg.h"

void Mcu_Init( const Mcu_ConfigType *configPtr );
void Mcu_DeInit();
Std_ReturnType Mcu_InitRamSection( const Mcu_RamSectionType RamSection );
Std_ReturnType Mcu_InitClock( const Mcu_ClockType ClockSetting );
void Mcu_DistributePllClock( void );
Mcu_PllStatusType Mcu_GetPllStatus( void );
Mcu_ResetType Mcu_GetResetReason( void );
Mcu_RawResetType Mcu_GetResetRawValue( void );
#if ( MCU_PERFORM_RESET_API == STD_ON )
void Mcu_PerformReset( void );
#endif
void Mcu_SetMode( const Mcu_ModeType McuMode );

#if ( MCU_VERSION_INFO_API == STD_ON )
#define MCU_SW_MAJOR_VERSION    	1
#define MCU_SW_MINOR_VERSION   		0
#define MCU_SW_PATCH_VERSION    	0
#define MCU_AR_MAJOR_VERSION     	2
#define MCU_AR_MINOR_VERSION     	2
#define MCU_AR_PATCH_VERSION     	2

#define Mcu_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,MCU)
#endif

typedef uint32_t imask_t;

/* ArcCore extensions */
void IntCtrl_InstallVector(void (*func)(), IrqType vector, uint8_t priority, Cpu_t cpu );
void IntCtrl_GenerateSoftInt( IrqType vector );
uint8_t IntCtrl_GetCurrentPriority( Cpu_t cpu);
uint32_t McuE_GetSystemClock( void );
#if defined(CFG_MPC55XX)
uint32_t McuE_GetPeripheralClock( McuE_PeriperalClock_t type );
#endif
imask_t McuE_EnterCriticalSection(void);
void McuE_ExitCriticalSection(imask_t old_state);
void McuE_EnableInterrupts(void);
void McuE_DisableInterrupts(void);



#endif /*MCU_H_*/
