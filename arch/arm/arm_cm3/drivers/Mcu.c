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
#include <assert.h>
#include "cpu.h"
#include <string.h>
#include "Ramlog.h"
#include "system_stm32f10x.h"

//#define USE_TRACE 1
//#define USE_DEBUG 1
#include "Trace.h"

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

#define SPR_PIR 286
#define SPR_PVR 287

#define CORE_PVR_E200Z1   0x81440000UL
#define CORE_PVR_E200Z0   0x81710000UL


typedef struct {
  char *name;
  uint32 pvr;
} core_info_t;

typedef struct {
  char *name;
  uint32 pvr;
} cpu_info_t;

cpu_info_t cpu_info_list[] = {
    {
    .name = "MPC5516",
    .pvr = CORE_PVR_E200Z1,
    },
    {
    .name = "MPC5516",
    .pvr = CORE_PVR_E200Z0,
    },
};

core_info_t core_info_list[] = {
    {
    .name = "CORE_E200Z1",
    .pvr = CORE_PVR_E200Z1,
    },
    {
    .name = "CORE_E200Z1",
    .pvr = CORE_PVR_E200Z1,
    },
};

// TODO: move
#define ARRAY_SIZE(_x)  (sizeof(_x)/sizeof((_x)[0]))

static cpu_info_t *Mcu_IdentifyCpu(uint32 pvr)
{
  int i;
  for (i = 0; i < ARRAY_SIZE(cpu_info_list); i++) {
    if (cpu_info_list[i].pvr == pvr) {
      return &cpu_info_list[i];
    }
  }

  return NULL;
}

static core_info_t *Mcu_IdentifyCore(uint32 pvr)
{
  int i;
  for (i = 0; i < ARRAY_SIZE(core_info_list); i++) {
    if (core_info_list[i].pvr == pvr) {
      return &core_info_list[i];
    }
  }

  return NULL;
}


static uint32 Mcu_CheckCpu( void ) {

  uint32 pvr;
  //uint32 pir;
  cpu_info_t *cpuType;
  core_info_t *coreType;

  // We have to registers to read here, PIR and PVR

#if 0
  pir = get_spr(SPR_PIR);
  pvr = get_spr(SPR_PVR);
#endif

  cpuType = Mcu_IdentifyCpu(pvr);
  coreType = Mcu_IdentifyCore(pvr);

  if( (cpuType == NULL) || (coreType == NULL) ) {
    // Just hang
    while(1);
  }

  //DEBUG(DEBUG_HIGH,"/drivers/mcu: Cpu:  %s( 0x%08x )\n",cpuType->name,pvr);
  //DEBUG(DEBUG_HIGH,"/drivers/mcu: Core: %s( 0x%08x )\n",coreType->name,pvr);

  return 0;
}


//-------------------------------------------------------------------

void Mcu_Init(const Mcu_ConfigType *configPtr)
{
  VALIDATE( ( NULL != configPtr ), MCU_INIT_SERVICE_ID, MCU_E_PARAM_CONFIG );

  if( !SIMULATOR() ) {
	  Mcu_CheckCpu();
  }

  memset(&Mcu_Global.stats,0,sizeof(Mcu_Global.stats));


  SystemInit();

  Irq_Enable();

  Mcu_Global.config = configPtr;
  Mcu_Global.initRun = 1;
}
//-------------------------------------------------------------------

void Mcu_DeInit()
{
  Mcu_Global.initRun = FALSE; // Very simple Deinit. Should we do more?
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
  Mcu_ClockSettingConfigType *clockSettingsPtr;
  VALIDATE_W_RV( ( 1 == Mcu_Global.initRun ), MCU_INITCLOCK_SERVICE_ID, MCU_E_UNINIT, E_NOT_OK );
  VALIDATE_W_RV( ( ClockSetting < Mcu_Global.config->McuClockSettings ), MCU_INITCLOCK_SERVICE_ID, MCU_E_PARAM_CLOCK, E_NOT_OK );

  Mcu_Global.clockSetting = ClockSetting;
  clockSettingsPtr = &Mcu_Global.config->McuClockSettingConfig[Mcu_Global.clockSetting];



#if 0
  /* 5516clock info:
   * Fsys - System frequency ( CPU + all periperals? )
   *
   *  Fsys = EXTAL_FREQ *(  (emfd+16) / ( (eprediv+1) * ( erfd+1 )) ) )
   */
  // Check ranges...
  assert((clockSettingsPtr->PllEmfd>=32) && (clockSettingsPtr->PllEmfd<=132));
  assert( (clockSettingsPtr->PllEprediv!=6) &&
          (clockSettingsPtr->PllEprediv!=8) &&
          (clockSettingsPtr->PllEprediv<10) );
  assert( clockSettingsPtr->PllErfd & 1); // Must be odd
#endif



#if defined(USE_DEBUG)
  {
    uint32    extal = Mcu_Global.config->McuClockSettingConfig[Mcu_Global.clockSetting].McuClockReferencePoint;
    uint32    f_sys;

    f_sys = CALC_SYSTEM_CLOCK( extal,
        clockSettingsPtr->PllEmfd,
        clockSettingsPtr->PllEprediv,
        clockSettingsPtr->PllErfd );

    //DEBUG(DEBUG_HIGH,"/drivers/mcu: F_sys will be:%08d Hz\n",f_sys);
  }
#endif

#if defined(CFG_MPC5516)
  // External crystal PLL mode.
  FMPLL.ESYNCR1.B.CLKCFG = 7; //TODO: Hur ställa detta för 5567?

  // Write pll parameters.
  FMPLL.ESYNCR1.B.EPREDIV = clockSettingsPtr->PllEprediv;
  FMPLL.ESYNCR1.B.EMFD    = clockSettingsPtr->PllEmfd;
  FMPLL.ESYNCR2.B.ERFD    = clockSettingsPtr->PllErfd;

  // Connect SYSCLK to FMPLL
  SIU.SYSCLK.B.SYSCLKSEL = SYSCLOCK_SELECT_PLL;
#elif defined(CFG_MPC5554) || defined(CFG_MPC5567)
  FMPLL.SYNCR.B.PREDIV 	= clockSettingsPtr->PllEprediv;
  FMPLL.SYNCR.B.MFD		= clockSettingsPtr->PllEmfd;
  FMPLL.SYNCR.B.RFD    	= clockSettingsPtr->PllErfd;
#endif

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

  if( !SIMULATOR() )
  {
#if 0
    if ( !FMPLL.SYNSR.B.LOCK )
    {
      rv = MCU_PLL_UNLOCKED;
    } else
    {
      rv = MCU_PLL_LOCKED;
    }
#endif
  }
  else
  {
    /* We are running on instruction set simulator. PLL is then always in sync... */
    rv = MCU_PLL_LOCKED;
  }

  return rv;
}

//-------------------------------------------------------------------

Mcu_ResetType Mcu_GetResetReason(void)
{
	Mcu_ResetType rv;

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
void Mcu_PerformReset(void)
{
  VALIDATE( ( 1 == Mcu_Global.initRun ), MCU_PERFORMRESET_SERVICE_ID, MCU_E_UNINIT );

  NVIC_SystemReset();
}
#endif

//-------------------------------------------------------------------

void Mcu_SetMode(const Mcu_ModeType McuMode)
{
  VALIDATE( ( 1 == Mcu_Global.initRun ), MCU_SETMODE_SERVICE_ID, MCU_E_UNINIT );
  VALIDATE( ( McuMode <= Mcu_Global.config->McuNumberOfMcuModes ), MCU_SETMODE_SERVICE_ID, MCU_E_PARAM_MODE );
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
  /*
   * System clock calculation
   *
   */

  // TODO: This of course wrong....
  uint32_t f_sys = 72000000UL;
#if 0
  uint32  extal = Mcu_Global.config->McuClockSettingConfig[Mcu_Global.clockSetting].McuClockReferencePoint;

  f_sys =  CALC_SYSTEM_CLOCK(extal,emfd,eprediv,erfd);
#endif

//  f_sys = extal * (emfd+16) / ( (eprediv+1) * ( erfd+1 ));
  return f_sys;
}

imask_t McuE_EnterCriticalSection()
{
#if 0
  uint32_t msr = get_msr();
  Irq_Disable();
  return msr;
#endif
  return 0;
}

void McuE_ExitCriticalSection(uint32_t old_state)
{
#if 0
  set_msr(old_state);
#endif
}

/**
 * Get the peripheral clock in Hz for a specific device
 */

#if 0
uint32_t McuE_GetPeripheralClock(McuE_PeriperalClock_t type)
{

	return 0;
}
#endif


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
