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
#include "sys.h"
#include "pcb.h"
#include "internal.h"
#include "stm32f10x.h"
#include "core_cm3.h"
#include "int_ctrl.h"

/**
 * Init of free running timer.
 */
void Frt_Init( void ) {
	TaskType tid;
	tid = Os_CreateIsr(OsTick,6/*prio*/,"OsTick");
	IntCtrl_AttachIsr2(tid,NULL, SysTick_IRQn);
}

/**
 *
 * @param period_ticks How long the period in timer ticks should be. The timer
 *                     on PowerPC often driver by the CPU clock or some platform clock.
 *
 */

void Frt_Start(uint32_t period_ticks) {

	SysTick_Config(period_ticks);

	 /* Set SysTick Priority to 3 */
	NVIC_SetPriority(SysTick_IRQn, 0x0C);

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

uint32_t Frt_GetTimeElapsed( void )
{
	return (SysTick->VAL);
}
