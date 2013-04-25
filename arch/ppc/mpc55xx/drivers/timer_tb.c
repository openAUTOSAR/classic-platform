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
 * DESCRPTION:
 *   Timer for most PPC chips (Not MPC560x)
 */

/* ----------------------------[includes]------------------------------------*/

#include "Os.h"
#include "timer.h"
#include "Cpu.h"
#include "Mcu.h"

/* ----------------------------[private define]------------------------------*/
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
	uint32_t tmp;

	/* Clear TB */
	set_spr(SPR_TBU_W,0);
	set_spr(SPR_TBL_W,0);

	// Enable the TB
	tmp = get_spr(SPR_HID0);
	tmp |= HID0_TBEN;
	set_spr(SPR_HID0, tmp);

	Timer_Freq = McuE_GetSystemClock();
}

TickType Timer_GetTicks( void ) {
	return get_spr(SPR_TBL_R);
}
