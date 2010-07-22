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
	OsPcbType *i_pcb;
	int cnt = 0;

	TAILQ_FOREACH(i_pcb,&os_sys.ready_head,ready_list) {
		//printf("%02d: %02d %s\n",cnt,i_pcb->state,i_pcb->name);
		cnt++;
//		assert( i_pcb->state == ST_READY );
	}
}

// schedule()
static inline void Os_TaskRunningToReady( OsPcbType *pcb ) {
	assert(pcb->state == ST_RUNNING );
	pcb->state = ST_READY;
}


// ActivateTask(pid)
// SetEvent(pid)
static inline void Os_TaskMakeReady( OsPcbType *pcb ) {
	if( pcb->state != ST_READY ) {
		pcb->state = ST_READY;
		TAILQ_INSERT_TAIL(& os_sys.ready_head,pcb,ready_list);
		OS_DEBUG(D_TASK,"Added %s to ready list\n",pcb->name);
	}
}

// WaitEvent
static inline void Os_TaskMakeWaiting( OsPcbType *pcb )
{
	assert( pcb->state & (ST_READY|ST_RUNNING) );

	pcb->state = ST_WAITING;
	TAILQ_REMOVE(&os_sys.ready_head,pcb,ready_list);
	OS_DEBUG(D_TASK,"Removed %s from ready list\n",pcb->name);
}

// Terminate task
static inline void Os_TaskMakeSuspended( OsPcbType *pcb )
	{
	assert( pcb->state & (ST_READY|ST_RUNNING) );
	pcb->state = ST_SUSPENDED;
	TAILQ_REMOVE(&os_sys.ready_head,pcb,ready_list);
	OS_DEBUG(D_TASK,"Removed %s from ready list\n",pcb->name);
}


/**
 * Set the task to running state and remove from ready list
 *
 * @params pcb Ptr to pcb
 */
static inline void Os_TaskMakeRunning( OsPcbType *pcb ) {
	pcb->state = ST_RUNNING;
}

_Bool os_pcb_pid_valid( OsPcbType *restrict pcb );
void Os_TaskStartExtended( void );
void Os_TaskStartBasic( void );
void Os_ContextInit( OsPcbType *pcb );
OsPcbType *Os_TaskGetTop( void );

// Added by Mattias in order to avoid compiler warning
TaskType Os_AddTask( OsPcbType *pcb );

#if 0 // Not used any more
OsPcbType  *os_find_higher_priority_task( OsPriorityType prio );
#endif




extern OsPcbType pcb_list[];
extern const OsRomPcbType rom_pcb_list[];

static inline OsPcbType * os_get_pcb( OsTaskidType pid ) {
	return &pcb_list[pid];
}

static inline const OsRomPcbType * os_get_rom_pcb( OsTaskidType pid ) {
	return &rom_pcb_list[pid];
}

static inline OsPriorityType os_pcb_set_prio( OsPcbType *pcb, OsPriorityType new_prio ) {
	OsPriorityType 	old_prio;
	old_prio = pcb->prio;
	pcb->prio = new_prio;
	//printf("set_prio of %s to %d from %d\n",pcb->name,new_prio,pcb->prio);
	return old_prio;
}

#define os_pcb_get_state(pcb) ((pcb)->state)

void os_swap_context(OsPcbType *old_pcb, OsPcbType *new_pcb );
void Os_TaskSwapContextTo(OsPcbType *old_pcb, OsPcbType *new_pcb );


#endif /*TASK_I_H_*/
