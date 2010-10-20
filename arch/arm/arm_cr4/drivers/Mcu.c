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
#include "Det.h"
#if defined(USE_DEM)
#include "Dem.h"
#endif
#include <assert.h>
#include "cpu.h"
#include <string.h>
#include "Ramlog.h"

#include "core_cr4.h"

//#define USE_TRACE 1
//#define USE_LDEBUG_PRINTF 1
#include "debug.h"


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



static core_info_t *Mcu_IdentifyCore(uint32 pvr)
{


  return NULL;
}

/**
 * Identify the core, just to check that we have support for it.
 *
 * @return
 */
static uint32 Mcu_CheckCpu( void ) {

  return 0;
}

static uint32_t GetPllValueFromMult(uint8_t pll)
{
	return (((uint32_t)pll - 2) << 18);
}
static uint32_t GetPll2ValueFromMult(uint8_t pll)
{
	return (((uint32_t)pll - 2) << 8);
}

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

}

//-------------------------------------------------------------------

void Mcu_Init(const Mcu_ConfigType *configPtr)
{

}
//-------------------------------------------------------------------

void Mcu_DeInit()
{
}

//-------------------------------------------------------------------
Std_ReturnType Mcu_InitRamSection(const Mcu_RamSectionType RamSection)
{

  return E_OK;
}



//-------------------------------------------------------------------

Std_ReturnType Mcu_InitClock(const Mcu_ClockType ClockSetting)
{
    /** @b Initialize @b Pll: */

    /** - Setup pll control register 1:
    *     - Setup reset on oscillator slip
    *     - Setup bypass on pll slip
    *     - Setup Pll output clock divider
    *     - Setup reset on oscillator fail
    *     - Setup reference clock divider
    *     - Setup Pll multiplier
    */
	systemREG1->PLLCTL1 =  0x00000000U
	                        |  0x20000000U
	                        | (1U << 24U)
	                        |  0x00000000U
	                        | (4U << 16U)
	                        | (119U  << 8U);

    /** - Setup pll control register 1
    *     - Enable/Disable frequency modulation
    *     - Setup spreading rate
    *     - Setup bandwidth adjustment
    *     - Setup internal Pll output divider
    *     - Setup spreading amount
    */
    systemREG1->PLLCTL2 = 0x00000000U
                        | (255U << 22U)
                        | (0U << 12U)
                        | (1U << 9U)
                        |  61U;


    /** @b Initialize @b Clock @b Tree: */

    /** - Start clock source lock */
    systemREG1->CSDIS = 0x00000040U
                      | 0x00000000U
                      | 0x00000000U
                      | 0x00000008U
                      | 0x00000004U
                      | 0x00000000U
                      | 0x00000000U;

    /** - Wait for until clocks are locked */
    while ((systemREG1->CSVSTAT & ((systemREG1->CSDIS ^ 0xFF) & 0xFF)) != ((systemREG1->CSDIS ^ 0xFF) & 0xFF));

    /** - Setup GCLK, HCLK and VCLK clock source for normal operation, power down mode and after wakeup */
    systemREG1->GHVSRC = (SYS_PLL << 24U)
                       | (SYS_PLL << 16U)
                       |  SYS_PLL;

    /** - Power-up all peripharals */
    pcrREG->PSPWRDWNCLR0 = 0xFFFFFFFFU;
    pcrREG->PSPWRDWNCLR1 = 0xFFFFFFFFU;
    pcrREG->PSPWRDWNCLR2 = 0xFFFFFFFFU;
    pcrREG->PSPWRDWNCLR3 = 0xFFFFFFFFU;

    /** - Setup synchronous peripheral clock dividers for VCLK1 and VCLK2 */
    systemREG1->VCLKR  = 15U;
    systemREG1->VCLK2R = 1U;
    systemREG1->VCLKR  = 1U;

    /** - Setup RTICLK1 and RTICLK2 clocks */
    systemREG1->RCLKSRC = (1U << 24U)
                        | (SYS_VCLK << 16U)
                        | (1U << 8U)
                        |  SYS_VCLK;

    /** - Setup asynchronous peripheral clock sources for AVCLK1 and AVCLK2 */
    systemREG1->VCLKASRC = (SYS_FR_PLL << 8U)
                         |  SYS_VCLK;

    /** - Enable Peripherals */
    systemREG1->PENA = 1U;
  return E_OK;
}

//-------------------------------------------------------------------

void Mcu_DistributePllClock(void)
{

}

//-------------------------------------------------------------------


Mcu_PllStatusType Mcu_GetPllStatus(void) {

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
	Mcu_ResetType rv;

	return rv;
}

//-------------------------------------------------------------------

/**
 * Shall read the raw reset value from hardware register if the hardware
 * supports this.
 *
 * @return
 */

Mcu_RawResetType Mcu_GetResetRawValue(void) {

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

  return f_sys;
}

imask_t McuE_EnterCriticalSection()
{
	uint32_t val;
	return val;
}

void McuE_ExitCriticalSection(uint32_t old_state)
{

}

/**
 * Get the peripheral clock in Hz for a specific device
 */
uint32_t McuE_GetPeripheralClock(McuE_PeriperalClock_t type)
{

}


/**
 * Function to setup the internal flash for optimal performance
 */

void Mcu_ConfigureFlash(void)
{

}

void McuE_EnableInterrupts(void)
{
  Irq_Enable();
}

void McuE_DisableInterrupts(void)
{
  Irq_Disable();
}
