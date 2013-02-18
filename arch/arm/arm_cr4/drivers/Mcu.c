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


#include "Std_Types.h"
#include "Mcu.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#if defined(USE_DEM)
#include "Dem.h"
#endif
#include <assert.h>
#include "Cpu.h"
#include <string.h>
#include "Ramlog.h"

#include "core_cr4.h"

//#define USE_LDEBUG_PRINTF 1
#include "debug.h"

/* PLLCTL1 register */
#define	MCU_RESET_ON_SLIP	0  				// Reset on slip is off
#define	MCU_RESET_ON_SLIP_OFFSET	31  	// Offset in PLLCTL1

#define	MCU_BYPASS_ON_SLIP	2  				// Bypass on slip is off
#define	MCU_BYPASS_ON_SLIP_OFFSET	29  	// Offset in PLLCTL1 (2 bits)

#define	MCU_RESET_ON_OSC_FAIL	0  			// Reset on oscillator fail is off
#define	MCU_RESET_ON_OSC_FAIL_OFFSET	23  // Offset in PLLCTL1

#define	MCU_PLLDIV_OFFSET	24  			// Offset in PLLCTL1 (5 bits)
#define MCU_PLLDIV_MASK     (0x1F << MCU_PLLDIV_OFFSET)
#define	MCU_REFCLKDIV_OFFSET	16  		// Offset in PLLCTL1 (6 bits)
#define MCU_REFCLKDIV_MASK  (0x3F << MCU_REFCLKDIV_OFFSET)
#define	MCU_PLLMUL_OFFSET	0  				// Offset in PLLCTL1 (16 bits)
#define MCU_PLLMUL_MASK     (0xFFFF << MCU_PLLMUL_OFFSET)

/* PLLCTL2 register */
#define MCU_FM_ENABLE	0  					// Frequency modulation is off
#define MCU_FM_ENABLE_OFFSET	31 			// Offset in PLLCTL2

#define MCU_SPREADING_RATE	0 				// Spreading rate
#define MCU_SPREADING_RATE_OFFSET	22 		// Offset in PLLCTL2 (9 bits)

#define MCU_BWADJ	0 						// Bandwidth adjustment
#define MCU_BWADJ_OFFSET	12 				// Offset in PLLCTL2 (9 bits)

#define MCU_ODPLL_OFFSET	9				// Offset in PLLCTL2 (3 bits)
#define MCU_ODPLL_MASK     (0x7 << MCU_ODPLL_OFFSET)

#define MCU_SPREADING_AMOUNT	0 			// Spreading amount
#define MCU_SPREADING_AMOUT_OFFSET	0 		// Offset in PLLCTL2 (9 bits)


/* CSDIS (Clock source disable) register offsets */
#define MCU_CLK_SOURCE_OSC_OFFSET		0
#define MCU_CLK_SOURCE_FMZPLL_OFFSET	1
#define MCU_CLK_SOURCE_LPO_OFFSET		4
#define MCU_CLK_SOURCE_HPO_OFFSET		5
#define MCU_CLK_SOURCE_FPLL_OFFSET		6


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(_x)  (sizeof(_x)/sizeof((_x)[0]))
#endif

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


#define CORE_CPUID_CORTEX_M3   	0x411FC231UL

typedef struct {
	uint32 lossOfLockCnt;
	uint32 lossOfClockCnt;
} Mcu_Stats;






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


// Global config
Mcu_GlobalType Mcu_Global =
{
		.initRun = 0,
		.config = &McuConfigData[0],
};

//-------------------------------------------------------------------

typedef struct {
  char *name;
  uint32 pvr;
} core_info_t;

typedef struct {
  char *name;
  uint32 pvr;
} cpu_info_t;


void Mcu_ConfigureFlash(void);



/* Haven't found any ID accessable from memory.
 * There is the DBGMCU_IDCODE (0xe0042000) found in RM0041 but it
 * you can't read from that address..
 */
#if 0
cpu_info_t cpu_info_list[] = {
    {
    .name = "????",
    .pvr = 0,
    },
};
#endif

/* The supported cores
 */
core_info_t core_info_list[] = {
    {
    .name = "CORE_ARM_CORTEX_M3",
    .pvr = CORE_CPUID_CORTEX_M3,
    },
};

/**
 * Identify the core, just to check that we have support for it.
 *
 * @return
 */
static uint32 Mcu_CheckCpu( void ) {


	return 0;
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

	// INTCLK 1.63MHz - 6.53MHz
	// NR (REFCLKDIV) 1 - 64
	// INTCLK = CLKIN / REFCLKDIV;

	// Output CLK 120MHz - 500MHz
	// NF (PLLMUL) 92 - 184
	// OutputCLK = INTCLK * PLLMUL;

	// OD (ODPLL) 1 - 8
	// R (PLLDIV)  1 - 32
	// PLLCLK = OutputCLK / ODPLL / PLLDIV;

	// Algorithm
	// PLLCLK = (CLKIN * PLLMUL) / (REFCLKDIV * ODPLL * PLLDIV);


	/** - Setup pll control register 1:
	*     - Setup reset on oscillator slip
	*     - Setup bypass on pll slip
	*     - Setup reset on oscillator fail
	*      - Setup Pll output clock divider
	*     - Setup reference clock divider
	*     - Setup Pll multiplier*
	*/
	systemREG1->PLLCTL1 =
		  (MCU_RESET_ON_SLIP << MCU_RESET_ON_SLIP_OFFSET)
		| (MCU_BYPASS_ON_SLIP << MCU_BYPASS_ON_SLIP_OFFSET)
		| (MCU_RESET_ON_OSC_FAIL << MCU_RESET_ON_OSC_FAIL_OFFSET)
		| ((clockSettingsPtr->Pll1 - 1) << MCU_REFCLKDIV_OFFSET)
		| (((clockSettingsPtr->Pll2 - 1) * 256) << MCU_PLLMUL_OFFSET)
		| ((clockSettingsPtr->Pll4 - 1) << MCU_PLLDIV_OFFSET);


	/** Setup PLLCTL2
	 *     - Setup internal Pll output divider
	 *     - Enable/Disable frequency modulation (NOT USED)
	 *     - Setup spreading rate (NOT USED)
	 *     - Setup bandwidth adjustment (NOT USED)
	 *     - Setup spreading amount (NOT USED)
	 */
	systemREG1->PLLCTL2 =
		  (MCU_FM_ENABLE << MCU_FM_ENABLE_OFFSET)
		| (MCU_SPREADING_RATE << MCU_SPREADING_RATE_OFFSET)
		| (MCU_BWADJ << MCU_BWADJ_OFFSET)
		| (MCU_SPREADING_AMOUNT << MCU_SPREADING_AMOUT_OFFSET)
		| ((clockSettingsPtr->Pll3 - 1) << MCU_ODPLL_OFFSET);

	/** - Wait for until clocks are locked */
	while ((systemREG1->CSVSTAT & ((systemREG1->CSDIS ^ 0xFF) & 0xFF)) != ((systemREG1->CSDIS ^ 0xFF) & 0xFF)) ;


}

//-------------------------------------------------------------------

void Mcu_Init(const Mcu_ConfigType *configPtr)
{
	VALIDATE( ( NULL != configPtr ), MCU_INIT_SERVICE_ID, MCU_E_PARAM_CONFIG );

#if !defined(USE_SIMULATOR)
	Mcu_CheckCpu();
#endif

	memset(&Mcu_Global.stats,0,sizeof(Mcu_Global.stats));

  /** Enable/Disable the right clocks
	 *  0 = enable, 1 = disable */
	systemREG1->CSDIS =
		  (0 << MCU_CLK_SOURCE_OSC_OFFSET)
		| (0 << MCU_CLK_SOURCE_FMZPLL_OFFSET)
		| (1 << MCU_CLK_SOURCE_LPO_OFFSET)
		| (0 << MCU_CLK_SOURCE_HPO_OFFSET)
		| (1 << MCU_CLK_SOURCE_FPLL_OFFSET);


	/** - Wait for until clocks are locked */
	while ((systemREG1->CSVSTAT & ((systemREG1->CSDIS ^ 0xFF) & 0xFF)) != ((systemREG1->CSDIS ^ 0xFF) & 0xFF)) ;

	/** - Setup GCLK, HCLK and VCLK clock source for normal operation, power down mode and after wakeup */
	systemREG1->GHVSRC = (SYS_PLL << 24U) // Selectes PLL clock (clock source 1) as wakeup clock source.
					   | (SYS_PLL << 16U) // Select PLL clock (clock source 1) as wakeup when GCLK is off as clock source.
					   |  SYS_PLL;  // Select PLL clock (clock source 1) as current clock source.

	/** - Power-up clocks to all peripharals */
	pcrREG->PSPWRDWNCLR0 = 0xFFFFFFFFU;
	pcrREG->PSPWRDWNCLR1 = 0xFFFFFFFFU;
	pcrREG->PSPWRDWNCLR2 = 0xFFFFFFFFU;
	pcrREG->PSPWRDWNCLR3 = 0xFFFFFFFFU;

	/** - Setup synchronous peripheral clock dividers for VCLK1 and VCLK2
	 * 1 = divide by 2
	 */
	systemREG1->VCLKR  = 1U;
	systemREG1->VCLK2R = 1U;

	/** - Setup RTICLK1 and RTICLK2 clocks */
	systemREG1->RCLKSRC = (0U << 24U) // RTICLK2 divider is 1
						| (SYS_VCLK << 16U)  // Select VCLK as source for RTICLK2.
						| (0U << 8U) // RTICLK2 divider is 1
						|  SYS_VCLK; // Select VCLK as source for RTICLK1.

	/** - Setup asynchronous peripheral clock sources for AVCLK1 and AVCLK2 */
	systemREG1->VCLKASRC = (SYS_VCLK << 8U)
						 |  SYS_VCLK;

	/** - Enable Peripherals */
	systemREG1->PENA = 1U;

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

	return E_NOT_OK;
}



//-------------------------------------------------------------------

Std_ReturnType Mcu_InitClock(const Mcu_ClockType ClockSetting)
{
	const Mcu_ClockSettingConfigType *clockSettingsPtr;
	VALIDATE_W_RV( ( 1 == Mcu_Global.initRun ), MCU_INITCLOCK_SERVICE_ID, MCU_E_UNINIT, E_NOT_OK );
	VALIDATE_W_RV( ( ClockSetting < Mcu_Global.config->McuClockSettings ), MCU_INITCLOCK_SERVICE_ID, MCU_E_PARAM_CLOCK, E_NOT_OK );

	Mcu_Global.clockSetting = ClockSetting;
	clockSettingsPtr = &Mcu_Global.config->McuClockSettingConfig[Mcu_Global.clockSetting];

	InitMcuClocks(clockSettingsPtr);
	InitPerClocks(clockSettingsPtr);

	return E_OK;





  return E_OK;
}

//-------------------------------------------------------------------

void Mcu_DistributePllClock(void)
{
	VALIDATE( ( 1 == Mcu_Global.initRun ), MCU_DISTRIBUTEPLLCLOCK_SERVICE_ID, MCU_E_UNINIT );
}

//-------------------------------------------------------------------


Mcu_PllStatusType Mcu_GetPllStatus(void) {
	VALIDATE_W_RV( ( 1 == Mcu_Global.initRun ), MCU_GETPLLSTATUS_SERVICE_ID, MCU_E_UNINIT, MCU_PLL_STATUS_UNDEFINED );

	if ((systemREG1->CSVSTAT & ((systemREG1->CSDIS ^ 0xFF) & 0xFF)) != ((systemREG1->CSDIS ^ 0xFF) & 0xFF)) {
		return MCU_PLL_UNLOCKED;
	}

	return MCU_PLL_LOCKED;
}

//-------------------------------------------------------------------

/**
 *
 * @return
 */
Mcu_ResetType Mcu_GetResetReason(void) {
	VALIDATE_W_RV( ( 1 == Mcu_Global.initRun ), MCU_GETRESETREASON_SERVICE_ID, MCU_E_UNINIT, MCU_RESET_UNDEFINED );
	 Mcu_ResetType reason = MCU_RESET_UNDEFINED;

	if (systemREG1->SYSESR & 0x00008000) {
		reason = MCU_POWER_ON_RESET;
		systemREG1->SYSESR = 0x00008000;
	} else if (systemREG1->SYSESR & 0x00004000) {
		reason = MCU_OSC_FAILURE_RESET;
		systemREG1->SYSESR = 0x00004000;
	} else if (systemREG1->SYSESR & 0x00002000) {
		reason = MCU_WATCHDOG_RESET;
		systemREG1->SYSESR = 0x00002000;
	} else if (systemREG1->SYSESR & 0x00000020) {
		reason = MCU_CPU_RESET;
		systemREG1->SYSESR = 0x00000020;
	} else if (systemREG1->SYSESR & 0x00000010) {
		reason = MCU_SW_RESET;
		systemREG1->SYSESR = 0x00000010;
	} else if (systemREG1->SYSESR & 0x00000008) {
		reason = MCU_EXT_RESET;
		systemREG1->SYSESR = 0x00000008;
	} else if (systemREG1->SYSESR & 0x00000004) {
		reason = MCU_VSW_RESET;
		systemREG1->SYSESR = 0x00000004;
	} else {
		reason = MCU_RESET_UNDEFINED;
	}

/* USER CODE BEGIN (23) */
/* USER CODE END */

	return reason;
}

//-------------------------------------------------------------------

/**
 * Shall read the raw reset value from hardware register if the hardware
 * supports this.
 *
 * @return
 */

Mcu_RawResetType Mcu_GetResetRawValue(void) {
	VALIDATE_W_RV( ( 1 == Mcu_Global.initRun ), MCU_GETRESETREASON_SERVICE_ID, MCU_E_UNINIT, MCU_GETRESETRAWVALUE_UNINIT_RV );

	Mcu_RawResetType reason = 0xFFFFFFFF;
	reason = systemREG1->SYSESR & 0x0000E03B;
	systemREG1->SYSESR = 0x0000E03B;

	return reason;
}

//-------------------------------------------------------------------

#if ( MCU_PERFORM_RESET_API == STD_ON )
/**
 * Shell perform a microcontroller reset by using the hardware feature
 * of the micro controller.
 */
void Mcu_PerformReset(void)
{
	VALIDATE( ( 1 == Mcu_Global.initRun ), MCU_PERFORMRESET_SERVICE_ID, MCU_E_UNINIT );
	systemREG1->SYSECR = 0x00008000;
}
#endif

//-------------------------------------------------------------------

void Mcu_SetMode(const Mcu_ModeType McuMode)
{
	VALIDATE( ( 1 == Mcu_Global.initRun ), MCU_SETMODE_SERVICE_ID, MCU_E_UNINIT );
  /* NOT SUPPORTED */
}

//-------------------------------------------------------------------

/**
 * Get the system clock in Hz. It calculates the clock from the
 * different register settings in HW.
 */
uint32_t McuE_GetSystemClock(void)
{
  uint32_t f_sys;

  // PLLCLK = (CLKIN * PLLMUL) / (REFCLKDIV * ODPLL * PLLDIV);

  uint32 odpll = ((systemREG1->PLLCTL2 & MCU_ODPLL_MASK) >> MCU_ODPLL_OFFSET) + 1;
  uint32 plldiv = ((systemREG1->PLLCTL1 & MCU_PLLDIV_MASK) >> MCU_PLLDIV_OFFSET) + 1;
  uint32 refclkdiv = ((systemREG1->PLLCTL1 & MCU_REFCLKDIV_MASK) >> MCU_REFCLKDIV_OFFSET) + 1;
  uint32 pllmult = (((systemREG1->PLLCTL1 & MCU_PLLMUL_MASK) >> MCU_PLLMUL_OFFSET) / 256) + 1;

  f_sys = Mcu_Global.config->McuClockSettingConfig[Mcu_Global.clockSetting].McuClockReferencePointFrequency;
  f_sys = f_sys * pllmult / (refclkdiv * odpll * plldiv);

  return f_sys;
}

/**
 * Get the peripheral clock in Hz for a specific device
 */
uint32_t McuE_GetPeripheralClock(McuE_PeriperalClock_t type)
{
	if (type == PERIPHERAL_CLOCK_CAN) {
		uint8 vclockDiv = systemREG1->VCLKR;
		return McuE_GetSystemClock() / (vclockDiv + 1);
	}
	return 0;
}

/**
 * Get frequency of the oscillator
 */
uint32_t McuE_GetClockReferencePointFrequency()
{
	return Mcu_Global.config->McuClockSettingConfig[Mcu_Global.clockSetting].McuClockReferencePointFrequency;
}

/**
 * Function to setup the internal flash for optimal performance
 */
void Mcu_ConfigureFlash(void)
{

}

