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
 * isr.c
 *
 *  Created on: Jul 13, 2009
 *      Author: mahi
 */

#include <stdint.h>
#include "sys.h"
#include "pcb.h"
#include "internal.h"
#include "hooks.h"
#include "swap.h"
// TODO: remove. Make soft links or whatever
#if defined(CFG_ARM_CM3)
#include "irq.h"
//#include "stm32f10x.h"
//#include "stm32f10x_arc.h"
#endif
#include "int_ctrl.h"

/**
 * Handle ISR type 2 interrupts from interrupt controller.
 *
 * @param stack Ptr to the current stack
 * @param vector The vector that took the interrupt
 */
void *Os_Isr( void *stack, void *pcb_p ) {
	struct pcb_s *pcb;
	struct pcb_s *preempted_pcb;

	os_sys.int_nest_cnt++;

	// Save info for preempted pcb
	preempted_pcb = get_curr_pcb();
	preempted_pcb->stack.curr = stack;
	preempted_pcb->state = ST_READY;
	os_isr_printf(D_TASK,"Preempted %s\n",preempted_pcb->name);

	POSTTASKHOOK();

	pcb = (struct pcb_s *)pcb_p;
	pcb->state = ST_RUNNING;
	set_curr_pcb(pcb);

	PRETASKHOOK();

	// We should not get here if we're SCHEDULING_NONE
	if( pcb->scheduling == SCHEDULING_NONE) {
		// TODO:
		// assert(0);
		while(1);
	}

	Irq_Enable();
	pcb->entry();
	Irq_Disable();

	pcb->state = ST_SUSPENDED;
	POSTTASKHOOK();

	IntCtrl_EOI();

	--os_sys.int_nest_cnt;

	// TODO: Check stack check marker....
	// We have preempted a task
	if( (os_sys.int_nest_cnt == 0) ) { //&& is_idle_task() ) {
		/* If we get here:
		 * - the preempted task is saved with large context.
		 * - We are on interrupt stack..( this function )
		 *
		 * if we find a new task:
		 * - just switch in the new context( don't save the old because
		 *   its already saved )
		 *
		 */
		pcb_t *new_pcb;
		new_pcb = os_find_top_prio_proc();
		if( new_pcb != preempted_pcb ) {
			os_isr_printf(D_TASK,"Found candidate %s\n",new_pcb->name);
//#warning os_swap_context_to should call the pretaskswaphook
			os_swap_context_to(NULL,new_pcb);
		} else {
			if( new_pcb == NULL ) {
				assert(0);
			}
			preempted_pcb->state = ST_RUNNING;
			set_curr_pcb(preempted_pcb);
		}
	}

	return stack;
}
