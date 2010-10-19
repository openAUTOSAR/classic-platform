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

/**
 * Creates an ISR dynamically
 * @param entry
 * @param prio
 * @param name
 *
 * @return The PID of the ISR created
 */
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

/**
 * Before we have proper editor for ISR2 use this function to add resources
 * to an ISR2
 *
 * @param isr
 * @param resource
 * @return
 */
StatusType Os_IsrAddResource( TaskType isr, ResourceType resource ) {
	return E_OK;
}

#if defined(CFG_ARM_CM3)
extern void Irq_EOI2( void );
#endif


/**
 * Handle ISR type 2 interrupts from interrupt controller.
 *
 * @param stack Ptr to the current stack
 * @param vector The vector that took the interrupt
 */
void *Os_Isr( void *stack, void *isr_p ) {
	struct OsPcb *isrPtr;
	struct OsPcb *pPtr = NULL;

	os_sys.int_nest_cnt++;

	/* Check if we interrupted a task or ISR */
	if( os_sys.int_nest_cnt == 1 ) {
		/* We interrupted a task */
		POSTTASKHOOK();

		/* Save info for preempted pcb */
		pPtr = get_curr_pcb();
		pPtr->stack.curr = stack;
		pPtr->state = ST_READY;
		OS_DEBUG(D_TASK,"Preempted %s\n",pPtr->name);

		Os_StackPerformCheck(pPtr);
	} else {
		/* We interrupted an ISR */
	}

	/* Grab the ISR "pcb" */
	isrPtr = (struct OsPcb *)isr_p;
	isrPtr->state = ST_RUNNING;

	if( isrPtr->proc_type & ( PROC_EXTENDED | PROC_BASIC ) ) {
		assert(0);
	}

#ifndef CFG_HCS12D
	Irq_Enable();
	isrPtr->entry();
	Irq_Disable();
#else
	isrPtr->entry();
#endif

	/* Check so that ISR2 haven't disabled the interrupts */
	/** @req OS368 */
	if( Os_IrqAnyDisabled() ) {
		Os_IrqClearAll();
		ERRORHOOK(E_OS_DISABLEDINT);
	}

	/* Check so that the ISR2 have called ReleaseResource() for each GetResource() */
	/** @req OS369 */
	if( Os_TaskOccupiesResources(isrPtr) ) {
		Os_ResourceFreeAll(isrPtr);
		ERRORHOOK(E_OS_RESOURCE);
	}

	isrPtr->state = ST_SUSPENDED;

	Irq_EOI();

	--os_sys.int_nest_cnt;

	// We have preempted a task
	if( (os_sys.int_nest_cnt == 0) ) {

		OsPcbType *new_pcb  = Os_TaskGetTop();

		Os_StackPerformCheck(new_pcb);

		if(     (new_pcb == os_sys.curr_pcb) ||
				(os_sys.curr_pcb->scheduling == NON) ||
				!Os_SchedulerResourceIsFree() )
		{
			/* Just bring the preempted task back to running */
			os_sys.curr_pcb->state = ST_RUNNING;
			PRETASKHOOK();
		} else {
			OS_DEBUG(D_TASK,"Found candidate %s\n",new_pcb->name);
#if defined(CFG_ARM_CM3)
			Irq_EOI2();
#endif
			Os_TaskSwapContextTo(NULL,new_pcb);
		}
	} else {
		/* We have a nested interrupt, do nothing */
	}

	return stack;
}
