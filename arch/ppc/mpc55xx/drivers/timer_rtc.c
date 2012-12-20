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
 * DESCRIPTION:
 *  Free-running MPC560x series of processors.
 */


/* ----------------------------[includes]------------------------------------*/

#include "Os.h"
#include "timer.h"
#include "cpu.h"
#include "Mcu.h"


/* ----------------------------[private define]------------------------------*/

#define RTCC_CNTEN				(1<<(31-0))
#define RTCC_FRZEN				(1<<(31-2))
#define RTCC_CLKSEL_SXOSC		(0<<(31-19))
#define RTCC_CLKSEL_SIRC		(1<<(31-19))
#define RTCC_CLKSEL_FIRC		(2<<(31-19))
#define RTCC_CLKSEL_FXOSC		(3<<(31-19))

#define RTCC_DIV32EN			(1<<(21-31))

/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/

uint32_t Timer_Freq;

/**
 * Initialize the TB
 */
void Timer_Init( void ) {
	CGM.SXOSC_CTL.B.OSCON = 1;	// enable the osc for RTC

	RTC.RTCC.R = 0;
	/* FIRC=16Mhz */
	RTC.RTCC.R =  RTCC_CNTEN | RTCC_FRZEN | RTCC_CLKSEL_FIRC;

	Timer_Freq = McuE_GetSystemClock();
}

TickType Timer_GetTicks( void ) {
	return RTC.RTCCNT.R;
}

