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

#ifndef TASK_I_H_
#define TASK_I_H_

#include <stdlib.h>
#include <assert.h>
#include "internal.h"
#include "Ramlog.h"

static inline void os_pcb_print_rq( void ) {
	pcb_t *i_pcb;
	int cnt = 0;

	TAILQ_FOREACH(i_pcb,&os_sys.ready_head,ready_list) {
		//printf("%02d: %02d %s\n",cnt,i_pcb->state,i_pcb->name);
		cnt++;
//		assert( i_pcb->state == ST_READY );
	}
}

// schedule()
static inline void os_pcb_running_to_ready( pcb_t *pcb ) {
	assert(pcb->state == ST_RUNNING );
	pcb->state = ST_READY;
}


// ActivateTask(pid)
// SetEvent(pid)
// os_pcb_make_virgin(pcb_t*)
static inline void os_pcb_make_ready( pcb_t *pcb ) {
	if( pcb->state != ST_READY ) {
		pcb->state = ST_READY;
		TAILQ_INSERT_TAIL(& os_sys.ready_head,pcb,ready_list);
		os_isr_printf(D_TASK,"Added %s to ready list\n",pcb->name);
	}
}

// WaitEvent
static inline void os_pcb_make_waiting( pcb_t *pcb )
{
	assert( pcb->state & (ST_READY|ST_RUNNING) );

	pcb->state = ST_WAITING;
	TAILQ_REMOVE(&os_sys.ready_head,pcb,ready_list);
	os_isr_printf(D_TASK,"Removed %s from ready list\n",pcb->name);
}

// Terminate task
static inline void os_pcb_make_suspended( pcb_t *pcb )
	{
	assert( pcb->state & (ST_READY|ST_RUNNING) );
	pcb->state = ST_SUSPENDED;
	TAILQ_REMOVE(&os_sys.ready_head,pcb,ready_list);
	os_isr_printf(D_TASK,"Removed %s from ready list\n",pcb->name);
}


/**
 * Set the task to running state and remove from ready list
 *
 * @params pcb Ptr to pcb
 */
static inline void os_pcb_make_running( pcb_t *pcb ) {
	pcb->state = ST_RUNNING;
}


_Bool os_pcb_pid_valid( pcb_t *restrict pcb );
void os_proc_start_extended( void );
void os_proc_start_basic( void );
void os_setup_context( pcb_t *pcb );
pcb_t  *os_find_top_prio_proc( void );

void os_pcb_make_virgin( pcb_t *pcb );

// Added by Mattias in order to avoid compiler warning
TaskType os_add_task( pcb_t *pcb );

#if 0 // Not used any more
pcb_t  *os_find_higher_priority_task( prio_t prio );
#endif


#endif /*TASK_I_H_*/
