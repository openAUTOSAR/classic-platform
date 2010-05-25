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


