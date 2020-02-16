/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/


#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>

extern uint32_t Timer_Freq;

#define TIMER_TICK2US( _x )		((_x) / (Timer_Freq/1000000))

typedef uint32_t TimerTick;


void Timer_Init( void );

/**
 * Get a 32-bit timer
 */

TimerTick Timer_GetTicks( void );

/**
 * Busy wait for useconds micro seconds.
 *
 * @param useconds
 */

static inline void Timer_uDelay(uint32_t uSeconds ) {
	TimerTick startTick = Timer_GetTicks();
	TimerTick waitTicks;

	/* Calc the time to wait */
	waitTicks = (uint32_t)(((uint64_t)Timer_Freq * uSeconds)/1000000);

	/* busy wait */
	while( (Timer_GetTicks() - startTick)  < waitTicks ) {
		;
	}
}



#endif /* TIMER_H_ */
