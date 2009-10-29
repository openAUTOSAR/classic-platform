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
#include "int_ctrl.h"

/**
 * Init of free running timer.
 */
void Frt_Init( void ) {
	TaskType tid;
	tid = Os_CreateIsr(OsTick,6/*prio*/,"OsTick");
	IntCtrl_AttachIsr2(tid,NULL,7);
}

/**
 *
 *
 * @param period_ticks How long the period in timer ticks should be. The timer
 *                     on PowerPC often driver by the CPU clock or some platform clock.
 *
 */
void Frt_Start(uint32_t period_ticks) {
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

uint32_t Frt_GetTimeElapsed( void )
{
	uint32_t timer = get_spr(SPR_DECAR) - get_spr(SPR_DEC);
	return (timer);
}
