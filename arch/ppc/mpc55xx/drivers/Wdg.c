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
 * Author: ?
 *
 * Part of Release:
 *   ?
 *
 * Description:
 *   Implements the Watchdog Driver module
 *
 * Support:
 *   General                  Have Support
 *   -------------------------------------------
 *   WDG_DEV_ERROR_DETECT      N
 *   WDG_DISABLE_ALLOWED       N
 *   WdgIndex                  Y
 *   WDG_TRIGGER_LOCATION      Y
 *   WdgVersionInfoApi         N
 *
 * Implementation Notes:
 *   MPC5516
 *     SWT: driven by SIU_SYSCLK.SWTCLKSEL (here System Clock, can be 16Mhz IRC)
 *      - SWTCR.SE=1. Enabled by default. Override with RCHW.
 *      - RCHW (WTE bit, 0-disabled, 1-enabled) (This area is read by BAM)
 *     OnChip
 *      - BookE, disabled by default.  SPR_TCR.WRC!=0 enables watchdog
 */

#include "mpc55xx.h"
#include "Wdg.h"
#include "Mcu.h"
#include "io.h"

#if defined(CFG_MPC5668)
#define SWT_BASE			0xFFF38000
#define SWT_CR				0x0
#define SWT_TO				0x8
#define SWT_SR				0x10

#define CR_RIA				0x100
#define CR_SLK				0x10
#define CR_CSL				0x8
#define CR_FRZ				0x2
#define CR_WEN				0x1
#endif


static const Wdg_ConfigType *configWdgPtr;
static const Wdg_SettingsType *modeWdgConfig;

#if defined(CFG_MPC5516)

#define SWTCR_SWRI(_x) ((_x)<<5)
#define SWTCR_SWT(_x) 	(_x)
#define SWTCR_SWE 		(1<<7)
#endif

void StartWatchdog(uint32 timeout_in_ms)
{
#if defined(CFG_MPC5567)
	(void)timeout_in_ms;
	ECSM.SWTCR.R =  0x00D8;
#elif defined(CFG_MPC560X)|| defined(CFG_MPC563XM)
	(void)timeout_in_ms;
	SWT.CR.R = 0x8000011B;
#elif defined(CFG_MPC5668)
	/* Clocked by 16 MHz IRC clock */

	/* Clear softlock */
	WRITE32(SWT_BASE + SWT_SR, 0x0000c520);
	WRITE32(SWT_BASE + SWT_SR, 0x0000d928);

	/* Write TMO */
	WRITE32(SWT_BASE + SWT_TO, timeout_in_ms * 16000 );

	/* Enable Watchdog */
	WRITE32(SWT_BASE + SWT_CR,0x80000000UL + CR_RIA + CR_SLK + CR_CSL + CR_FRZ + CR_WEN);

#elif defined(CFG_MPC5516)
	/* We running on system clock, ie SIU_SYSCLK.SWTCLKSEL,  so get the value */

	/* The timeout is 2^x, so get best possible value
	 *   Table for 80Mhz
	 *   ------------------
	 *   2^9  = 512 = 6.4 uS
	 *   2^15 =       400 uS
	 *   2^20 =       13 mS
	 *   2^28 =       3.3 S
	 *   2^31 =       26,84 S
	 *
	 * Formula:
	 *   1/clock * 2^n  = tmo_in_s
	 *   2^n = tmo_in_s * clock -> n = log2(tmo_in_s * clock) = log2(tmo_in_ms * clock / 1000 )
	 *  */
	uint32 swtVal = ilog2( McuE_GetSystemClock()/1000 * timeout_in_ms );
#if defined(CFG_WDG_TEST)
	MCM.SWTCR.R = (SWTCR_SWE + SWTCR_SWRI(WDG_SWRI_VAL) + swtVal);
#else
	MCM.SWTCR.R = (SWTCR_SWE + SWTCR_SWRI(2) + swtVal);
#endif

#else
	MCM.SWTCR.R = 0x00D8;
#endif
}

/**
 * Disable the watchdog
 */
void StopWatchdog(void)
{
#if defined(CFG_MPC5567)
 	ECSM.SWTCR.R =  0x0059;;
#elif defined(CFG_MPC560X) || defined(CFG_MPC5668) || defined(CFG_MPC563XM)
 	SWT.SR.R = 0x0000c520;     /* Write keys to clear soft lock bit */
 	SWT.SR.R = 0x0000d928;
 	SWT.CR.R = 0x8000010A;
#elif defined(CFG_MPC5516)
 	MCM.SWTCR.R = 0x0;	  /* Disable the watchdog */
#else
	MCM.SWTCR.R = 0x0059;
#endif
}

void Wdg_Init (const Wdg_ConfigType* ConfigPtr)
{
	/* Keep a pointer to the config. */
	configWdgPtr = ConfigPtr;

	Wdg_SetMode(ConfigPtr->Wdg_ModeConfig->Wdg_DefaultMode);
}

Std_ReturnType Wdg_SetMode (WdgIf_ModeType Mode)
{
	Std_ReturnType res = E_NOT_OK;
	switch (Mode)
	{
	case WDGIF_OFF_MODE:
		modeWdgConfig = &configWdgPtr->Wdg_ModeConfig->WdgSettingsOff;
		break;
	case WDGIF_FAST_MODE:
		modeWdgConfig = &configWdgPtr->Wdg_ModeConfig->WdgSettingsFast;
		break;
	case WDGIF_SLOW_MODE:
		modeWdgConfig = &(configWdgPtr->Wdg_ModeConfig->WdgSettingsSlow);
		break;
	default:
		modeWdgConfig = 0;
		break;
	}
	if (modeWdgConfig != 0)
	{
		/* Enable watchdog if config tell us to.. */
		if (modeWdgConfig->ActivationBit)
		{
#if defined(CFG_MPC560X) || defined(CFG_MPC563XM)
		  StopWatchdog(); // must be stopped in order to change TO
 		  SWT.TO.R = modeWdgConfig->ReloadValue;
#endif
			StartWatchdog(modeWdgConfig->ReloadValue);
		}
		else
		{
			StopWatchdog();
		}
		res = E_OK;
	}

	return res;
}


/* This function services the internal Watchdog timer */
void Wdg_Trigger (void)
{
	imask_t state;

    Irq_Save(state);

	//  According to MPC55xx manual:
	//  To prevent the watchdog timer from interrupting or resetting
	//  the SWTSR must be serviced by performing the following sequence:
	//  1. Write 0x55 to the SWTSR.
	//  2. Write 0xAA to the SWTSR.
#if defined(CFG_MPC5567)
	ECSM.SWTSR.R = 0x55;
	ECSM.SWTSR.R = 0xAA;
#elif defined(CFG_MPC560X) || defined(CFG_MPC563XM) || defined(CFG_MPC5668)
	SWT.SR.R = 0x0000A602;
	SWT.SR.R = 0x0000B480;
#elif defined(CFG_MPC5516)
	MCM.SWTSR.R = 0x55;
	MCM.SWTSR.R = 0xAA;
#else
	MCM.SWTSR.R = 0x55;
	MCM.SWTSR.R = 0xAA;
#endif

    Irq_Restore(state);
}
