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
/*
 * strace.c
 *
 *  Created on: 2009-apr-19
 *      Author: mahi
 *
 * System trace...
 * Idea to trace
 *
 * Inspiration:
 *   http://ltt.polymtl.ca/tracingwiki/index.php/TracingBook
 *   http://developer.apple.com/tools/performance/optimizingwithsystemtrace.html
 *   http://benno.id.au/docs/lttng_data_format.pml
 */

/*
 * Channel
 *
 * ISR
 * TASK
 *
 */

typedef enum {
	STRACE_CH_ISR = 1,
	STRACE_CH_TASK = (1<<1),
	STRACE_CH_KERNEL = (1<<2),
} strace_ch_t;

typedef enum {
	STRACE_EV_ISR_START,
	STRACE_EV_ISR_STOP,
	STRACE_EV_TASK_START,
	STRACE_EV_TASK_STOP,
} strace_ev_t;


struct strace_attr_s {
	strace_ch_t
	/* A timestamp in some unit */
	uint32_t timestamp;
};


/**
 *
 * @param ch channel to record
 */
void strace( strace_ev_t event) {
	switch( ch ) {
	case STRACE_CH_ISR:
		break;
	case STRACE_CH_TASK:
		break;
	}
}

/**
 * Channels to use.
 */
void strace_init( strace_ch_t ch ) {

}

void strace_print( void ) {

}


