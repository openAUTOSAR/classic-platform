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









#include "types.h"
#include "pcb.h"
#include "arch.h"
#include "debug.h"
//#include "arch_offset.h"
#include "sys.h"
#include "kernel.h"
#include "assert.h"
#include "swap.h"
#include "task_i.h"
#include "hooks.h"
#include "internal.h"


#define USE_DEBUG
#include "Trace.h"

// we come here from
// - WaitEvent()
//   old_pcb -> WAITING
//   new_pcb -> READY(RUNNING)
// - Schedule(),
//   old_pcb -> READY
//   new_pcb -> READY/RUNNING

/*
 * two strategies
 * 1. When running ->
 *    - remove from ready queue
 *    - set state == ST_RUNNING
 *
 * 2. When running ->
 *    * leave in ready queue
 *    * set state == ST_RUNNING
 *    - ready queue and ST_READY not the same
 *    + No need to remove the running process from ready queue
 */



/**
 * Swap context, from one pcb to another
 */
void os_swap_context(pcb_t *old_pcb, pcb_t *new_pcb ) {

	POSTTASKHOOK();

	assert(new_pcb!=NULL);

	os_resource_release_internal();

	if( !os_arch_stack_endmark_ok(old_pcb) ) {
		ShutdownOS(E_OS_STACKFAULT);
	}

#if 0
	// Make a simple stack check for prio procs...
	// See OS068, Autosar SWS
	{
		uint32_t stackp = (uint32_t)os_arch_get_stackptr();
		uint32_t smallc_size = os_arch_get_sc_size();

		// enough size to place a small context on the stack
		// top( low address ) + small context > current stackpointer
		if( (uint32_t)(old_pcb->stack.top + smallc_size) > stackp ) {
			ShutdownOS(E_OS_STACKFAULT);
		}
	}
#endif

//	os_arch_print_context("NEW:",new_pcb);
	os_arch_swap_context(old_pcb,new_pcb);


	{
		pcb_t *t_pcb = os_get_curr_pcb();
//		dbg_printf("New pcb: %s\n",t_pcb->name);
		os_pcb_make_running(t_pcb);
	}
	os_resource_get_internal();
	PRETASKHOOK();
}

// We come here from
// - os_init

/**
 * Called when a task is to be run for the first time.
 */
void os_swap_context_to(pcb_t *old_pcb, pcb_t *new_pcb ) {


	os_arch_swap_context_to(old_pcb,new_pcb);
	/* TODO: When do we return here ?? */
}

