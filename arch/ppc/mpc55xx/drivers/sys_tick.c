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
#include "irq_types.h"

/**
 * Init of free running timer.
 */
void Os_SysTickInit( void ) {

	ISR_INSTALL_ISR2( "OsTick", OsTick, INTC_SSCIR0_CLR7, 6, 0 );

//	TaskType tid;

//	Irq_Attach(INTC_SSCIR0_CLR7);
//	Irq_Attach2( INTC_SSCIR0_CLR7 );

#if 0
	IRQ_DECL_ISR2("MyIsr", 7 , CPU_CORE0, 6	, OsTick, 0 , NULL );
	IRQ_ATTACH(7);

// else
	tid = ISR_INSTALL_ISR2( "OsTick", OsTick, _vector, 6/*prio*/, 0 );
	Irq_AttachIsr2(tid,NULL,7);
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
	uint32_t tmp;

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

