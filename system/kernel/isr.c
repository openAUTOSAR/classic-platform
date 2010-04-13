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

#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include "internal.h"
#include "irq.h"
#if 0


#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/queue.h>
#include <string.h>
#include "internal.h"

#endif


// TODO: remove. Make soft links or whatever
#if defined(CFG_ARM_CM3)
#include "irq_types.h"
//#include "stm32f10x.h"
//#include "stm32f10x_arc.h"
#endif

extern caddr_t *sbrk(int);

#define os_alloc(_x)	sbrk(_x)

OsPcbType * os_alloc_new_pcb( void ) {
	void *h = os_alloc(sizeof(OsPcbType));
	memset(h,0,sizeof(OsPcbType));
	assert(h!=NULL);
	return h;
}

#if 0
typedef void (*Os_IsrEntryType)(void);


typedef Os_IsrInfo_s {
	Os_IsrEntryType entry;
	uint32_t vector;
	uint8_t priority;
} Os_IsrInfoType;
#endif


extern TaskType Os_AddTask( OsPcbType *pcb );

static uint8 stackTop = 0x42;

TaskType Os_Arc_CreateIsr( void (*entry)(void ), uint8_t prio, const char *name )
{
	OsPcbType *pcb = os_alloc_new_pcb();
	strncpy(pcb->name,name,TASK_NAME_SIZE);
	pcb->vector = -1;
	pcb->prio = prio;
	/* TODO: map to interrupt controller priority */
	assert(prio<=OS_TASK_PRIORITY_MAX);
	pcb->proc_type  = PROC_ISR2;
	pcb->state = ST_SUSPENDED;
	pcb->entry = entry;
	pcb->stack.top = &stackTop;

	return Os_AddTask(pcb);
}


#if defined(CFG_ARM_CM3)
extern void Irq_EOI2(void *pc);
#endif


/**
 * Handle ISR type 2 interrupts from interrupt controller.
 *
 * @param stack Ptr to the current stack
 * @param vector The vector that took the interrupt
 */
void *Os_Isr( void *stack, void *pcb_p ) {
	struct OsPcb *pcb;
	struct OsPcb *preempted_pcb;

	os_sys.int_nest_cnt++;

	// Save info for preempted pcb
	preempted_pcb = get_curr_pcb();
	preempted_pcb->stack.curr = stack;
	preempted_pcb->state = ST_READY;
	OS_DEBUG(D_TASK,"Preempted %s\n",preempted_pcb->name);

	POSTTASKHOOK();

	pcb = (struct OsPcb *)pcb_p;
	pcb->state = ST_RUNNING;
	set_curr_pcb(pcb);

	PRETASKHOOK();

	// We should not get here if we're NON
	if( pcb->scheduling == NON) {
		// TODO:
		// assert(0);
		while(1);
	}

	Irq_Enable();
	pcb->entry();
	Irq_Disable();

	/** @req OS368 */
	if( Os_IrqAnyDisabled() ) {
		Os_IrqClearAll();
		ERRORHOOK(E_OS_DISABLEDINT);
	}

	/** @req OS369 */
	Os_ResourceCheckAndRelease(pcb);

	pcb->state = ST_SUSPENDED;
	POSTTASKHOOK();

	Irq_EOI();

	--os_sys.int_nest_cnt;

	// TODO: Check stack check marker....
	// We have preempted a task
	if( (os_sys.int_nest_cnt == 0) && (os_sys.scheduler_lock==0) ) { //&& is_idle_task() ) {
		/* If we get here:
		 * - the preempted task is saved with large context.
		 * - We are on interrupt stack..( this function )
		 *
		 * if we find a new task:
		 * - just switch in the new context( don't save the old because
		 *   its already saved )
		 */
		OsPcbType *new_pcb;
		new_pcb = Os_TaskGetTop();
		if( new_pcb != preempted_pcb ) {
			OS_DEBUG(D_TASK,"Found candidate %s\n",new_pcb->name);
//#warning Os_TaskSwapContextTo should call the pretaskswaphook
// TODO: This shuould go away!!!!
#if defined(CFG_ARM_CM3)
			void *p;
			p = &&really_ugly;
			Irq_EOI2(p);
really_ugly:
#endif
			Os_TaskSwapContextTo(NULL,new_pcb);
		} else {
			if( new_pcb == NULL ) {
				assert(0);
			}
			preempted_pcb->state = ST_RUNNING;
			set_curr_pcb(preempted_pcb);
		}
	} else {
		set_curr_pcb(preempted_pcb);
		PRETASKHOOK();
	}

	return stack;
}
