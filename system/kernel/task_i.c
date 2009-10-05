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
#include "pcb.h"
#include "sys.h"
#include <stdlib.h>
//#include "arch_offset.h"
#include "hooks.h"
#include "task_i.h"
#include "arch.h"
#include "ext_config.h"
#include "assert.h"
#include "internal.h"

_Bool os_pcb_pid_valid( pcb_t *restrict pcb ) {
	return ( pcb->pid > Oil_GetTaskCnt() ) ? 0 : 1;
}
/**
 * Start an extended task.
 * Tasks done:
 * - Grab the internal resource for the process
 * - Set it running state.
 * - Start to execute the process
 *
 */
void os_proc_start_extended( void ) {
	pcb_t *pcb;

	// TODO: Get internal resource
	PRETASKHOOK();

	pcb = os_get_curr_pcb();
	os_resource_get_internal();
	os_pcb_make_running(pcb);

	os_arch_first_call();

	// If we have a extented process that that exits, we end up here
	// See OS052,OS069, Autosar SWS
	ERRORHOOK(E_OS_MISSINGEND);

	/* TODO: Terminate the task */
	//while(1);
}

/**
 * Start an basic task.
 * See extended task.
 */

void os_proc_start_basic( void ) {
	pcb_t *pcb;

	// TODO: Get internal resource
	PRETASKHOOK();

	pcb = os_get_curr_pcb();
	os_resource_get_internal();
	os_pcb_make_running(pcb);
	os_arch_first_call();

	TerminateTask();
//	ERRORHOOK(E_OS_MISSINGEND);
}

/**
 * Setup the context for a pcb. The context differs for different arch's
 * so we call the arch dependent functions also.
 * The context at setup is always a small context.
 *
 * @param pcb Ptr to the pcb to setup context for.
 */
void os_setup_context( pcb_t *pcb ) {
	uint8_t *bottom;

	/* Find bottom of the stack so that we can place the
	 * context there.
	 *
	 * stack bottom = high address. stack top = low address
	 */
	bottom = (uint8_t *)pcb->stack.top + pcb->stack.size;
	pcb->stack.curr = bottom;
	// TODO: aligments here..
	// TODO :use function os_arch_get_call_size() ??

	// Make some space for back-chain.
	bottom -= 16;
	// Set the current stack so that it points to the context
	pcb->stack.curr = bottom - os_arch_get_sc_size();

	os_arch_setup_context(pcb);
}

/**
 * Search for a specific task in the pcb list.
 *
 * @param tid The task id to search for
 * @return Ptr to the found pcb or NULL
 */
pcb_t *os_find_task( TaskType tid ) {
	pcb_t *i_pcb;

	/* TODO: Implement this as an array */
	TAILQ_FOREACH(i_pcb,& os_sys.pcb_head,pcb_list) {
		if(i_pcb->pid == tid ) {
			return i_pcb;
		}
	}
	assert(0);
	return NULL;
}

/**
 * Adds a pcb to the list of pcb's
 * @param pcb
 */
TaskType os_add_task( pcb_t *pcb ) {
	long msr;

	Irq_Save(msr);  // Save irq status and disable interrupts

	pcb->pid = os_sys.task_cnt;
	// Add to list of PCB's
	TAILQ_INSERT_TAIL(& os_sys.pcb_head,pcb,pcb_list);
	os_sys.task_cnt++;

	Irq_Restore(msr);  // Restore interrupts
	return pcb->pid;
}


#define PRIO_ILLEGAL	-100
// TODO: we can't have O(n) search here.. hash on prio instead

/**
 * Find the top priority task. Even the running task is included.
 *
 * @return
 */

pcb_t *os_find_top_prio_proc( void ){
	pcb_t *i_pcb;
	pcb_t *top_prio_pcb = NULL;
	prio_t top_prio = PRIO_ILLEGAL;

	os_isr_printf(D_TASK,"os_find_top_prio_proc\n");

	TAILQ_FOREACH(i_pcb,& os_sys.ready_head,ready_list) {
		// all ready task are canidates
		if( i_pcb->state & (ST_READY|ST_RUNNING)) {
			if( top_prio != PRIO_ILLEGAL ) {
				if( i_pcb->prio > top_prio ) {
					top_prio = i_pcb->prio;
					top_prio_pcb = i_pcb;
				}
			} else {
				top_prio = i_pcb->prio;
				top_prio_pcb = i_pcb;
			}
		} else {
			assert(0);
		}
	}
	os_isr_printf(D_TASK,"Found %s\n",top_prio_pcb->name);

	return top_prio_pcb;
}


/**
 * Used at startup to initialize a pcb. Is sometimes
 * used to restore the state of a basic process.
 *
 * @param pcb The pcb to make virgin
 */

void os_pcb_make_virgin( pcb_t *pcb ) {
	if( pcb->autostart ) {
		os_pcb_make_ready(pcb);
	} else {
		pcb->state = ST_SUSPENDED;
	}

	/* TODO: cleanup resource here ?? */
	pcb->ev_set = 0;
	pcb->ev_wait = 0;

	os_setup_context(pcb);
}

#if 0
pcb_t *os_find_higher_priority_task( prio_t prio ) {
	pcb_t *i_pcb;
	pcb_t *h_prio_pcb = NULL;
	prio_t t_prio = prio;

	TAILQ_FOREACH(i_pcb,& os_sys.ready_head,ready_list) {
		if( i_pcb->prio > t_prio ) {
			t_prio = i_pcb->prio;
			h_prio_pcb = i_pcb;
		}
	}
	return h_prio_pcb;
}
#endif


