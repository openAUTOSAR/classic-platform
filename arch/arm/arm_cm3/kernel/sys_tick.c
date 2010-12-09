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
#include "stm32f10x.h"
#include "irq.h"
#include "arc.h"


/**
 * Init of free running timer.
 */
void Os_SysTickInit( void ) {
	TaskType tid;
	tid = Os_Arc_CreateIsr(OsTick,6/*prio*/,"OsTick");
	Irq_AttachIsr2(tid,NULL, SysTick_IRQn);
}

/**
 * Start the Sys Tick timer
 *
 * @param period_ticks How long the period in timer ticks should be.
 *
 */

void Os_SysTickStart(uint32_t period_ticks) {

	/* Cortex-M3 have a 24-bit system timer that counts down
	 * from the reload value to zero.
	 */

	SysTick_Config(period_ticks);

#if 0
	// SysTick interrupt each 250ms with counter clock equal to 9MHz
	if (SysTick_Config((SystemFrequency / 8) / 4)) {
		// Capture error
		while (1)
			;
	}

	// Select HCLK/8 as SysTick clock source
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
#endif

}

/**
 * @return
 */

uint32_t Os_SysTickGetValue( void )
{
	return (SysTick->LOAD) - (SysTick->VAL);
}


TickType Os_SysTickGetElapsedValue( uint32_t preValue ) {
	uint32_t curr;
	uint32_t max;

	curr = (SysTick->VAL);
	max  = (SysTick->LOAD);
	return Os_CounterDiff((max - curr),preValue,max);
}

