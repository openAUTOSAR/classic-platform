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

#define TIMER_TICK2US( _x )		((_x) / (Timer_Freq/1000000uL))
#define TIMER_TICK2NS( _x )     (((_x)*1000) / (Timer_Freq/1000000uL))
#define TIMER_US2TICK( _x )            ((uint32_t)(((uint64_t)Timer_Freq * (_x) )/1000000uLL))

typedef uint32_t TimerTick;
typedef uint64_t TimerTick64;

void Timer_Init( void );
void Timer_InitIdx( uint8_t timerIdx );

/**
 * Get a 32-bit timer
 */

TimerTick Timer_GetTicks( void );
TimerTick Timer_GetTicksIdx( uint8_t timerIdx );

/**
 * Get a 64-bit timer
 */

uint64_t Timer_GetTicks64( void );
uint64_t Timer_GetTicks64Idx( uint8_t timerIdx );


/**
 * Busy wait for useconds micro seconds.
 *
 * @param useconds
 */

static inline void Timer_uDelay(uint32_t uSeconds ) {
    TimerTick startTick = Timer_GetTicks();
    TimerTick waitTicks;

    /* Calc the time to wait */
    waitTicks = (uint32_t)(((uint64_t)Timer_Freq * uSeconds)/1000000uLL);

    /* busy wait */
    while( (Timer_GetTicks() - startTick)  < waitTicks ) {
        ;
    }
}

static inline void Timer_uDelay64(uint32_t uSeconds ) {
    TimerTick64 startTick = Timer_GetTicks64();
    TimerTick64 waitTicks;

    /* Calc the time to wait */
    waitTicks = ((uint64_t)Timer_Freq * (uint64_t)uSeconds)/1000000uLL;

    /* busy wait */
    while( (Timer_GetTicks64() - startTick)  < waitTicks ) {
        ;
    }
}


#endif /* TIMER_H_ */
