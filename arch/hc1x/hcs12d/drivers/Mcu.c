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
#if defined(USE_DET)
#include "Det.h"
#endif
#include <assert.h>
#include "Cpu.h"
#include <string.h>
#include "Ramlog.h"

#define USE_LDEBUG_PRINTF 1
#include "debug.h"


#define  PORTIO_8	 *(volatile unsigned char *)
#define  IO_BASE	 0
#define  CLKSEL      PORTIO_8(IO_BASE + 0x39)   /* clock select register */
#define  PLLCTL      PORTIO_8(IO_BASE + 0x3a)   /* PLL control register */
#define  CRGFLG      PORTIO_8(IO_BASE + 0x37)   /* clock generator flag register */
#define  SYNR        PORTIO_8(IO_BASE + 0x34)   /* synthesizer register */
#define  REFDV       PORTIO_8(IO_BASE + 0x35)   /* reference divider register */
#define  S12_REFCLK	 8000000		// PLL internal reference clock

#define BM_PLLSEL	0x80
#define BM_PLLON	0x40
#define BM_AUTO		0x20
#define BM_LOCK		0x08
/*
#define BM_RTIF		0x80
#define BM_PORF		0x40
//#define reserved	0x20
#define BM_LOCKIF	0x10

#define BM_TRACK	0x04
#define BM_SCMIF	0x02
#define BM_SCM		0x01

// Bits in CRGINT:
#define BM_RTIE		0x80
//#define reserved	0x40
//#define reserved	0x20
#define BM_LOCKIE	0x10
//#define reserved	0x08
//#define reserved	0x04
#define BM_SCMIE	0x02
//#define reserved	0x01

// Bits in CLKSEL:

#define BM_PSTP		0x40
#define BM_SYSWAI	0x20
#define BM_ROAWAI	0x10
#define BM_PLLWAI	0x08
#define BM_CWAI		0x04
#define BM_RTIWAI	0x02
#define BM_COPWAI	0x01

// Bits in PLLCTL:
#define BM_CME		0x80
#define BM_ACQ		0x10
//#define reserved	0x08
#define BM_PRE		0x04
#define BM_PCE		0x02
#define BM_SCME		0x01



*/


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
  const Mcu_ClockSettingConfigType *clockSettingsPtr;
  VALIDATE_W_RV( ( 1 == Mcu_Global.initRun ), MCU_INITCLOCK_SERVICE_ID, MCU_E_UNINIT, E_NOT_OK );
  VALIDATE_W_RV( ( ClockSetting < Mcu_Global.config->McuClockSettings ), MCU_INITCLOCK_SERVICE_ID, MCU_E_PARAM_CLOCK, E_NOT_OK );

  Mcu_Global.clockSetting = ClockSetting;
  clockSettingsPtr = &Mcu_Global.config->McuClockSettingConfig[Mcu_Global.clockSetting];

  CLKSEL &= ~BM_PLLSEL; // Turn off PLL
  PLLCTL |= BM_PLLON+BM_AUTO;  // Enable PLL module, Auto Mode

  REFDV = clockSettingsPtr->Pll1;  // Set reference divider
  SYNR = clockSettingsPtr->Pll2;  // Set synthesizer multiplier

  while (Mcu_GetPllStatus() != MCU_PLL_LOCKED) ;
  CLKSEL |= BM_PLLSEL; // Switch to PLL clock

  return E_OK;
}

//-------------------------------------------------------------------

void Mcu_DistributePllClock(void)
{
  VALIDATE( ( 1 == Mcu_Global.initRun ), MCU_DISTRIBUTEPLLCLOCK_SERVICE_ID, MCU_E_UNINIT );
//  VALIDATE( ( FMPLL.SYNSR.B.LOCK == 1 ), MCU_DISTRIBUTEPLLCLOCK_SERVICE_ID, MCU_E_PLL_NOT_LOCKED );

  /* NOT IMPLEMENTED due to pointless function on this hardware */

}

//-------------------------------------------------------------------

Mcu_PllStatusType Mcu_GetPllStatus(void)
{
  VALIDATE_W_RV( ( 1 == Mcu_Global.initRun ), MCU_GETPLLSTATUS_SERVICE_ID, MCU_E_UNINIT, MCU_PLL_STATUS_UNDEFINED );
  Mcu_PllStatusType rv;

  if ((CRGFLG & BM_LOCK) == 0) {
	  rv = MCU_PLL_UNLOCKED;
  } else {
	  rv = MCU_PLL_LOCKED;
  }

  return rv;
}

//-------------------------------------------------------------------

Mcu_ResetType Mcu_GetResetReason(void)
{
	Mcu_ResetType rv = MCU_RESET_UNDEFINED;

  VALIDATE_W_RV( ( 1 == Mcu_Global.initRun ), MCU_GETRESETREASON_SERVICE_ID, MCU_E_UNINIT, MCU_RESET_UNDEFINED );

#if 0
  if( SIU.RSR.B.SSRS ) {
  	rv = MCU_SW_RESET;
  } else if( SIU.RSR.B.WDRS ) {
  	rv = MCU_WATCHDOG_RESET;
  } else if( SIU.RSR.B.PORS || SIU.RSR.B.ERS ) {
  	rv = MCU_POWER_ON_RESET;
  } else {
  	rv = MCU_RESET_UNDEFINED;
  }
#endif

  return rv;
}

//-------------------------------------------------------------------

Mcu_RawResetType Mcu_GetResetRawValue(void)
{
  VALIDATE_W_RV( ( 1 == Mcu_Global.initRun ), MCU_GETRESETREASON_SERVICE_ID, MCU_E_UNINIT, MCU_GETRESETRAWVALUE_UNINIT_RV );

  if( !Mcu_Global.initRun ) {
  	return MCU_GETRESETRAWVALUE_UNINIT_RV;
  }

#if 0
  return SIU.RSR.R;
#endif

  return 0;
}

//-------------------------------------------------------------------

#if ( MCU_PERFORM_RESET_API == STD_ON )
#error MCU_PERFORM_RESET_API - Software reset is not supported on HCS12
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
 * Function to setup the internal flash for optimal performance
 */

void Mcu_ConfigureFlash(void)
{

}

