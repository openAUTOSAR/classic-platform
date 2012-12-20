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
 *   Timer Cortex M3 (the only Free Running Timer I could find)
 */

/* ----------------------------[includes]------------------------------------*/

#include "Os.h"
#include "timer.h"
#include "cpu.h"
#include "Mcu.h"
#include "io.h"

/* ----------------------------[private define]------------------------------*/

#define DWT_CYCCNT 		0xE0001004
#define DWT_CONTROL		0xE0001000
#define SCB_DEMCR		0xE000EDFC

/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/


uint32_t Timer_Freq;

/**
 * Initialize
 */
void Timer_Init( void ) {

	READWRITE32(SCB_DEMCR,0,0x01000000);
	WRITE32(DWT_CYCCNT,0 );
	READWRITE32(DWT_CONTROL,0,1);

	Timer_Freq = McuE_GetSystemClock();
}

TickType Timer_GetTicks( void ) {
	return READ32(DWT_CYCCNT);
}
