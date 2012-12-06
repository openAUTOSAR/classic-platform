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
#include "arc.h"
#include "regs.h"
#include "Mcu.h"

void Os_SysTickInit( void ) {
	ISR_INSTALL_ISR2("OsTick",OsTick,IRQ_TYPE_MCCNT_UNDERFLOW,6,0);
}

/**
 * @param period_ticks How long the period in timer ticks should be. The timer
 *                     on PowerPC often driver by the CPU clock or some platform clock.
 *
 */
void Os_SysTickStart(uint32_t period_ticks) {
	// Set timer 0 as output compare. Timer 0 is reserved for the OS.

	// TEN, timer enable
	// TSFRZ, stop in freeze mode (easier debugging)
	TSCR1 = TEN | TSFRZ;

	ICSYS = 0;

	// Modulus counter
	// MCZI, enable interrupt, MCEN enable modulus counter
	// Prescaler 4
	MCCTL =  MCZI | MCEN | MCPRE_VAL_4;

	// Set auto-reload
	MCCTL |= MODMC;

	// time = (count * prescaler)/(bus frequency)
	MCCNT = period_ticks / 8;
}
