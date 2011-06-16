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

#include "mpc55xx.h"
#include "Wdg.h"
#include "Mcu.h"


static const Wdg_ConfigType *configWdgPtr;
static const Wdg_SettingsType *modeWdgConfig;

void StartWatchdog(void)
{
#if defined(CFG_MPC5567)
	ECSM.SWTCR.R =  0x00D8;;
#elif defined(CFG_MPC5606S)
	SWT.CR.R = 0x8000011B;
#else
	MCM.SWTCR.R = 0x00D8;
#endif

}

 void StopWatchdog(void)
 {
 #if defined(CFG_MPC5567)
 	ECSM.SWTCR.R =  0x0059;;
 #elif defined(CFG_MPC5606S)
 	SWT.SR.R = 0x0000c520;     /* Write keys to clear soft lock bit */
 	SWT.SR.R = 0x0000d928;
 	SWT.CR.R = 0x8000010A;
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
#if defined(CFG_MPC5606S)
		  StopWatchdog(); // must be stopped in order to change TO
 		  SWT.TO.R = modeWdgConfig->ReloadValue;
#endif
			StartWatchdog();
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
	uint32 tmp;

	tmp = McuE_EnterCriticalSection();

	//  According to MPC55xx manual:
	//  To prevent the watchdog timer from interrupting or resetting
	//  the SWTSR must be serviced by performing the following sequence:
	//  1. Write 0x55 to the SWTSR.
	//  2. Write 0xAA to the SWTSR.
#if defined(CFG_MPC5567)
	ECSM.SWTSR.R = 0x55;
	ECSM.SWTSR.R = 0xAA;
#elif defined(CFG_MPC5606S)
	SWT.SR.R = 0x0000A602;
	SWT.SR.R = 0x0000B480;
#else
	MCM.SWTSR.R = 0x55;
	MCM.SWTSR.R = 0xAA;
#endif

	McuE_ExitCriticalSection(tmp);
}
