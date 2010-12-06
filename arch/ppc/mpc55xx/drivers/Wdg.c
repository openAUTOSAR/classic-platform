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








/*
 * Wdg.c
 *
 *  Created on: 2009-jul-22
 *      Author: rosa
 */


#include "mpc55xx.h"
#include "Mcu.h"
void StartWatchdog(void)
{
  // Setup watchdog
  // R0    =  0 Not read only
  // SWRWH =  0 SWT stops counting if the processor core is halted.
  // SWE   =  1 SWT is enabled.
  // SWRI  =  2 If a time-out occurs, the SWT generates a system reset.
  // SWT   = 24 For SWT = n, then time-out period = 2^n system clock cycles, n = 8 9,..., 31.
  //            SWT = 24  =>  period = 262144 clock cycles ( 254ms @ 66MHz )
#if defined(CFG_MPC5567)
  ECSM.SWTCR.R =  0x00D8;;
#else
  MCM.SWTCR.R = 0x00D8;
#endif
}

void StopWatchdog(void)
{
  // Stop the watchdog
  // R0 = 0     Not read only
  // SWRWH = 0  SWT stops counting if the processor core is halted.
  // SWE = 0    SWT is disabled.
  // SWRI = 2   If a time-out occurs, the SWT generates a system reset.
  // SWT = 19   For SWT = n, then time-out period = 2^n system clock cycles, n = 8 9,..., 31.
  //            SWT = 19  =>  period = 524288 clock cycles ( 8.7ms @ 60MHz )
#if defined(CFG_MPC5567)
  ECSM.SWTCR.R =  0x0059;;
#else
  MCM.SWTCR.R = 0x0059;
#endif
}


/* This function services the internal Watchdog timer */
void KickWatchdog(void)
{
  uint32 prevIEN;

  prevIEN = McuE_EnterCriticalSection();

//  According to MPC55xx manual:
//  To prevent the watchdog timer from interrupting or resetting
//  the SWTSR must be serviced by performing the following sequence:
//  1. Write 0x55 to the SWTSR.
//  2. Write 0xAA to the SWTSR.
#if defined(CFG_MPC5567)
  ECSM.SWTSR.R = 0x55;
  ECSM.SWTSR.R = 0xAA;
#else
  MCM.SWTSR.R = 0x55;
  MCM.SWTSR.R = 0xAA;
#endif
  McuE_ExitCriticalSection(prevIEN);
}
