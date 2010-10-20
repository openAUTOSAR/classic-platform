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
#include "core_cr4.h"
#include "irq.h"
#include "arc.h"


/**
 * Init of free running timer.
 */
void Os_SysTickInit( void ) {
	TaskType tid;
	tid = Os_Arc_CreateIsr(OsTick,6,"OsTick");
	Irq_AttachIsr2(tid,NULL, 2);
}


static inline uint32_t SysTick_Config(uint32_t ticks)
{

	    /** - Setup NTU source, debug options and disable both counter blocks */
	    rtiREG1->GCTRL = 0x0;

	    /** - Setup timebase for free running counter 0 */
	    rtiREG1->TBCTRL = 0x0;

	    /** - Enable/Disable capture event sources for both counter blocks */
	    rtiREG1->CAPCTRL = 0x0;

	    /** - Setup input source compare 0-3 */
	    rtiREG1->COMPCTRL = 0x0;

	    /** - Reset up counter 0 */
	    rtiREG1->CNT[0U].UCx = 0x00000000U;

	    /** - Reset free running counter 0 */
	    rtiREG1->CNT[0U].FRCx = 0x00000000U;

	    /** - Setup up counter 0 compare value
	    *     - 0x00000000: Divide by 2^32
	    *     - 0x00000001-0xFFFFFFFF: Divide by (CPUC0 + 1)
	    */
	    rtiREG1->CNT[0U].CPUCx = 4U;

	    /** - Setup compare 0 value. This value is compared with selected free running counter. */
	    rtiREG1->CMP[0U].COMPx = 9360U;

	    /** - Setup update compare 0 value. This value is added to the compare 0 value on each compare match. */
	    rtiREG1->CMP[0U].UDCPx = 9360U;

	    /** - Clear all pending interrupts */
	    rtiREG1->INTFLAG = 0x0;

	    /** - Disable all interrupts */
	    rtiREG1->CLEARINT = 0x0;

  return (0);                                                                            /* Function successful */
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
	rtiREG1->GCTRL  = 0x1;
	rtiREG1->SETINT = 0x1;


	 /* Set SysTick Priority to 3 */
	// TODO removeNVIC_SetPriority(SysTick_IRQn, 0x0C);

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
	//return (SysTick->LOAD) - (SysTick->VAL);
	return 0;
}


TickType Os_SysTickGetElapsedValue( uint32_t preValue ) {
	uint32_t curr;
	uint32_t max;

	/*
	curr = (SysTick->VAL);
	max  = (SysTick->LOAD);
	*/
	curr = 0;
	max = 0;
	return Os_CounterDiff((max - curr),preValue,max);
}

