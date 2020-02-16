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


#include <stdint.h>
#include "Std_Types.h"
#include "Mcu.h"
#include "Det.h"
#include <assert.h>
#include "Cpu.h"
#include <string.h>
#include "Ramlog.h"

#define USE_LDEBUG_PRINTF 1
#include "debug.h"

/*****************************
 * the same, on idea at the moment
 ********************************/

typedef struct {
	uint32 lossOfLockCnt;
	uint32 lossOfClockCnt;
} Mcu_Stats;

void Mcu_ConfigureFlash(void);
/**
 * Type that holds all global data for Mcu
 */
typedef struct
{
  // Set if Mcu_Init() have been called
  boolean initRun;

  // Our config
  const Mcu_ConfigType *config;

  Mcu_ClockType clockSetting;

  Mcu_Stats stats;

} Mcu_GlobalType;

/* Development error macros. */
#if ( MCU_DEV_ERROR_DETECT == STD_ON )
#define VALIDATE(_exp,_api,_err ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_MCU,0,_api,_err); \
          return; \
        }

#define VALIDATE_W_RV(_exp,_api,_err,_rv ) \
        if( !(_exp) ) { \
          Det_ReportError(MODULE_ID_MCU,0,_api,_err); \
          return (_rv); \
        }
#else
#define VALIDATE(_exp,_api,_err )
#define VALIDATE_W_RV(_exp,_api,_err,_rv )
#endif

// Global config
Mcu_GlobalType Mcu_Global =
{
		.initRun = 0,
		.config = &McuConfigData[0],
};

//-------------------------------------------------------------------

#if 0
static void Mcu_LossOfLock( void  ) {
#if ( MCU_DEV_ERROR_DETECT == STD_ON )
	/* Should report MCU_E_CLOCK_FAILURE with DEM here.... but
	 * we do the next best thing. Report with Det with API = 0
	 */
	Det_ReportError(MODULE_ID_MCU,0,0,MCU_E_PLL_NOT_LOCKED);
#endif

	Mcu_Global.stats.lossOfLockCnt++;
	// Clear interrupt
//	FMPLL.SYNSR.B.LOLF = 1;

}
#endif

/*********************************************************
 * -------------------shuzhou add-------------------------
 **********************************************************/
/**
  * Set bus clocks. SysClk,AHBClk,APB1Clk,APB2Clk
  */
static void SetClocks(Mcu_ClockSettingConfigType *clockSettingsPtr)
{

}

/**
  * Initialize Peripherals clocks
  */
static void InitPerClocks()
{

}

/**
  * Initialize Flash, PLL and clocks.
  */
static void InitMcuClocks(Mcu_ClockSettingConfigType *clockSettingsPtr)
{
	SetClocks(clockSettingsPtr);
}

//---------------------------------------------------------

//-------------------------------------------------------------------

void Mcu_Init(const Mcu_ConfigType *configPtr)
{
  VALIDATE( ( NULL != configPtr ), MCU_INIT_SERVICE_ID, MCU_E_PARAM_CONFIG );

  memset(&Mcu_Global.stats,0,sizeof(Mcu_Global.stats));

  Irq_Enable();

  Mcu_Global.config = configPtr;
  Mcu_Global.initRun = 1;
}
//-------------------------------------------------------------------

void Mcu_DeInit()
{
  Mcu_Global.initRun = FALSE;
}

//-------------------------------------------------------------------
Std_ReturnType Mcu_InitRamSection(const Mcu_RamSectionType RamSection)
{
  VALIDATE_W_RV( ( 1 == Mcu_Global.initRun ), MCU_INITRAMSECTION_SERVICE_ID, MCU_E_UNINIT, E_NOT_OK );
  VALIDATE_W_RV( ( RamSection <= Mcu_Global.config->McuRamSectors ), MCU_INITRAMSECTION_SERVICE_ID, MCU_E_PARAM_RAMSECTION, E_NOT_OK );

  /* NOT SUPPORTED, reason: no support for external RAM */

  return E_OK;
}



//-------------------------------------------------------------------

Std_ReturnType Mcu_InitClock(const Mcu_ClockType ClockSetting)
{
//  const Mcu_ClockSettingConfigType *clockSettingsPtr;
  VALIDATE_W_RV( ( 1 == Mcu_Global.initRun ), MCU_INITCLOCK_SERVICE_ID, MCU_E_UNINIT, E_NOT_OK );
  VALIDATE_W_RV( ( ClockSetting < Mcu_Global.config->McuClockSettings ), MCU_INITCLOCK_SERVICE_ID, MCU_E_PARAM_CLOCK, E_NOT_OK );

  Mcu_Global.clockSetting = ClockSetting;
//  clockSettingsPtr = &Mcu_Global.config->McuClockSettingConfig[Mcu_Global.clockSetting];
  //what kind of instruction can I write here!!!!!!
  //there should be some PLL settings, but i did not find the register here.
  return E_OK;
}

//-------------------------------------------------------------------

void Mcu_DistributePllClock(void)
{
  VALIDATE( ( 1 == Mcu_Global.initRun ), MCU_DISTRIBUTEPLLCLOCK_SERVICE_ID, MCU_E_UNINIT );
  //VALIDATE( ( FMPLL.SYNSR.B.LOCK == 1 ), MCU_DISTRIBUTEPLLCLOCK_SERVICE_ID, MCU_E_PLL_NOT_LOCKED );

  /* NOT IMPLEMENTED due to pointless function on this hardware */

}

//-------------------------------------------------------------------

Mcu_PllStatusType Mcu_GetPllStatus(void)
{
	/*different function*/
	VALIDATE_W_RV( ( 1 == Mcu_Global.initRun ), MCU_GETPLLSTATUS_SERVICE_ID, MCU_E_UNINIT, MCU_PLL_STATUS_UNDEFINED );

	return MCU_PLL_LOCKED;
}

//-------------------------------------------------------------------

Mcu_ResetType Mcu_GetResetReason(void)
{
	return MCU_POWER_ON_RESET;
}

//-------------------------------------------------------------------

Mcu_RawResetType Mcu_GetResetRawValue(void)
{
  return 0;
}

//-------------------------------------------------------------------
#if ( MCU_PERFORM_RESET_API == STD_ON )
/**
 * Shell perform a microcontroller reset by using the hardware feature
 * of the micro controller.
 */
void Mcu_PerformReset(void)
{

}
#endif


//-------------------------------------------------------------------

void Mcu_SetMode(const Mcu_ModeType McuMode)
{
	VALIDATE( ( 1 == Mcu_Global.initRun ), MCU_SETMODE_SERVICE_ID, MCU_E_UNINIT );
	VALIDATE( (0), MCU_SETMODE_SERVICE_ID, MCU_E_PARAM_MODE );
	  //VALIDATE( ( McuMode <= Mcu_Global.config->McuNumberOfMcuModes ), MCU_SETMODE_SERVICE_ID, MCU_E_PARAM_MODE );
	(void) McuMode;
  /* NOT SUPPORTED */
}

//-------------------------------------------------------------------

/**
 * Get the system clock in Hz. It calculates the clock from the
 * different register settings in HW.
 */
uint32_t McuE_GetSystemClock(void)
{
	uint32_t f_sys = Mcu_Global.config->McuClockSettingConfig[Mcu_Global.clockSetting].McuClockReferencePointFrequency * 2 *
			         (Mcu_Global.config->McuClockSettingConfig[Mcu_Global.clockSetting].Pll2 + 1) / ( Mcu_Global.config->McuClockSettingConfig[Mcu_Global.clockSetting].Pll1+1);
	return f_sys;
}

/**
 * Get the peripheral clock in Hz for a specific device
 */
uint32_t McuE_GetPeripheralClock(McuE_PeriperalClock_t type)
{
	return 20000000;
}

/**
 * Function to setup the internal flash for optimal performance
 */

void Mcu_ConfigureFlash(void)
{

}

