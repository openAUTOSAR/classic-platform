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

#include "Os.h"
#include "internal.h"
#include "isr.h"
#include "irq.h"
#include "arc.h"
#include "irq_types.h"
#include "counter_i.h"
#include "mpc55xx.h"

/**
 * Init of free running timer.
 */
extern void OsTick( void );
extern OsTickType OsTickFreq;


#if defined(CFG_MPC5606S)
void Os_SysTick_MPC5606( void ) {
	/* Clear API interrupt */
	RTC.RTCS.B.APIF = 1;

	OsTick();
}
#endif

void Os_SysTickInit( void ) {
#if defined(CFG_MPC5606S)
	ISR_INSTALL_ISR2("OsTick",Os_SysTick_MPC5606,API_INT,6,0);
#else
	ISR_INSTALL_ISR2("OsTick",OsTick,INTC_SSCIR0_CLR7,6,0);
#endif
}

/**
 *
 *
 * @param period_ticks How long the period in timer ticks should be. The timer
 *                     on PowerPC often driver by the CPU clock or some platform clock.
 *
 */
void Os_SysTickStart(uint32_t period_ticks) {
#if defined(CFG_MPC5606S)
	CGM.SXOSC_CTL.B.OSCON = 1;	// enable the osc for RTC


	RTC.RTCC.R= 0;		// disable RTC counter
//	RTC.RTCC.B.CNTEN = 0;		// disable RTC counter
//	RTC.RTCC.B.APIEN = 0;		// disable API
	RTC.RTCS.B.RTCF = 1;		// clear RTC interrupt flag
	RTC.RTCS.B.APIF = 1;		// clear API interrupt flag
	RTC.RTCC.B.RTCIE = 0;		// disable RTC interrupt
	RTC.RTCC.B.APIIE = 1;		// enable API interrupt
	RTC.RTCC.B.FRZEN = 1;		// enable freeze mode
	RTC.RTCC.B.CLKSEL = 2;		// set 16MHz FIRC as input clock
	RTC.RTCC.B.DIV32EN = 1;     // Divide by 32 enable
	// ignore period_ticks, and set API compare value
	RTC.RTCC.B.APIVAL = (uint32_t)(500000 / OsTickFreq);
	(void)period_ticks;

	RTC.RTCC.B.APIEN = 1;		// start API
	RTC.RTCC.B.CNTEN = 1;		// enable RTC counter
#else
		uint32 tmp;

    	// Enable the TB
    	tmp = get_spr(SPR_HID0);
    	tmp |= HID0_TBEN;
    	set_spr(SPR_HID0, tmp);

    	/* Initialize the Decrementer */
    	set_spr(SPR_DEC, period_ticks);
    	set_spr(SPR_DECAR, period_ticks);

    	/* Set autoreload */
    	tmp = get_spr(SPR_TCR);
    	tmp |= TCR_ARE;
    	set_spr(SPR_TCR, tmp);

    	/* Enable notification */
        tmp = get_spr(SPR_TCR);
        tmp |= TCR_DIE;
        set_spr(SPR_TCR, tmp );
#endif
}

/**
 * ???
 * TODO: This function just subtract the max value?! ok??
 *
 * @return
 */

/** @req OS383 */
TickType Os_SysTickGetValue( void )
{
	uint32_t timer = get_spr(SPR_DECAR) - get_spr(SPR_DEC);
	return (timer);
}

TickType Os_SysTickGetElapsedValue( uint32_t preValue ) {
	uint32_t curr;
	uint32_t max;

	curr = get_spr(SPR_DEC);
	max  = get_spr(SPR_DECAR);
	return Os_CounterDiff((max - curr),preValue,max);
}

