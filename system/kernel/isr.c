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


#if defined(__GNUC__)
#include <sys/types.h>
#endif
#include <stdint.h>
#include <string.h>
#include "Compiler.h"
#include "internal.h"
#include "isr.h"
#include "irq.h"


extern const uint8_t Os_VectorToIsr[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS];
extern const OsIsrConstType Os_IsrConstList[OS_ISR_CNT];

#if OS_ISR_MAX_CNT!=0
OsIsrVarType Os_IsrVarList[OS_ISR_MAX_CNT];
#endif

SECTION_BALIGN(0x10) uint8_t Os_IsrStack[OS_INTERRUPT_STACK_SIZE];

// TODO: remove. Make soft links or whatever
#if defined(CFG_ARM_CM3)
#include "irq_types.h"
//#include "stm32f10x.h"
//#include "stm32f10x_arc.h"
#endif

#if 0
extern caddr_t *sbrk(int);

#define os_alloc(_x)	sbrk(_x)

OsTaskVarType * os_alloc_new_pcb( void ) {
	void *h = os_alloc(sizeof(OsTaskVarType));
	memset(h,0,sizeof(OsTaskVarType));
	assert(h!=NULL);
	return h;
}
#endif


//extern TaskType Os_AddTask( OsTaskVarType *pcb );

//static uint8 stackTop = 0x42;



void Os_IsrInit( void ) {

	Irq_Init();

	/* Attach the interrupts */
	for (int i = 0; i < sizeof(Os_IsrConstList) / sizeof(OsIsrConstType); i++) {
		Os_IsrAdd(&Os_IsrConstList[i]);
	}
}


/**
 * Adds an ISR to a list of Isr's. The ISRType (id) is returned
 * for the "created" ISR.
 *
 * @param isrPtr Pointer to const data holding ISR information.
 * @return
 */
ISRType Os_IsrAdd( const OsIsrConstType * restrict isrPtr ) {
	ISRType id;

	id = Os_Sys.isrCnt++;
	/* We have no VAR entires for ISR1 */
	if( isrPtr->type == ISR_TYPE_2) {
		Os_IsrVarList[id].constPtr = isrPtr;
	}

	Irq_EnableVector( isrPtr->vector, isrPtr->priority, Os_ApplGetCore(isrPtr->appOwner )  );

	return id;
}

const OsIsrConstType * Os_IsrGet( int16_t vector) {
	return &Os_IsrConstList[Os_VectorToIsr[vector]];
}

#if 0
void Os_IsrDisable( ISRType isr) {

}

void Os_IsrEnable( ISRType isr) {

}
#endif


/*
 * Resources:
 *   Irq_VectorTable[]
 *   Irq_IsrTypeTable[]
 *   Irq_PriorityTable[]
 *
 *   exception table
 *   interrupt table
 *
 * Usual HW resources.
 * - prio in HW (ppc and arm (even cortex m4))
 *
 *
 * TOOL GENERATES ALL
 *   Irq_VectorTable   CONST
 *   Irq_IsrTypeTable  CONST
 *   Irq_PriorityTable CONST  Can probably be a table with ISR_MAX number
 *                            of for a CPU with prio registers.  For masking
 *                            CPUs it's better to keep an array to that indexing
 *                            can be used.
 *
 *   The problem with this approach is that the tool needs to know everything.
 *
 * TOOL GENERATES PART
 *   Irq_VectorTable   VAR     Since we must add vectors later
 *   Irq_IsrTypeTable  VAR     Since we must add vectors later
 *   Irq_PriorityTable VAR
 *
 *   We move the
 *
 */



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

void TailChaining(void *stack)
{
	struct OsTaskVar *pPtr = NULL;

	POSTTASKHOOK();

	/* Save info for preempted pcb */
	pPtr = get_curr_pcb();
	pPtr->stack.curr = stack;
	pPtr->state = ST_READY;
	OS_DEBUG(D_TASK,"Preempted %s\n",pPtr->name);

	Os_StackPerformCheck(pPtr);

	/* We interrupted a task */
	OsTaskVarType *new_pcb  = Os_TaskGetTop();

	Os_StackPerformCheck(new_pcb);

	if(     (new_pcb == Os_Sys.currTaskPtr) ||
			(Os_Sys.currTaskPtr->scheduling == NON) ||
			!Os_SchedulerResourceIsFree() )
	{
		/* Just bring the preempted task back to running */
		Os_TaskSwapContextTo(NULL,Os_Sys.currTaskPtr);
	} else {
		OS_DEBUG(D_TASK,"Found candidate %s\n",new_pcb->name);
		Os_TaskSwapContextTo(NULL,new_pcb);
	}
}

void Os_Isr_cm3( void *isr_p ) {

	struct OsTaskVar *isrPtr;

	Os_Sys.intNestCnt++;

	/* Grab the ISR "pcb" */
	isrPtr = (struct OsTaskVar *)isr_p;
	isrPtr->state = ST_RUNNING;

	if( isrPtr->proc_type & ( PROC_EXTENDED | PROC_BASIC ) ) {
		assert(0);
	}

	Irq_Enable();
	isrPtr->entry();
	Irq_Disable();

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

	--Os_Sys.intNestCnt;

	/* Scheduling is done in PendSV handler for ARM CM3 */
	*((uint32_t volatile *)0xE000ED04) = 0x10000000; // PendSV
}
#endif

/*-----------------------------------------------------------------*/

void Os_IsrGetStackInfo( OsIsrStackType *stack ) {
	stack->top = Os_IsrStack;
	stack->size = sizeof(Os_IsrStack);
}


/**
 * Handle ISR type 2 interrupts from interrupt controller.
 *
 * @param stack Pointer to the current stack
 * @param vector
 */
void *Os_Isr( void *stack, int16_t vector ) {
	uint8_t isrId = Os_VectorToIsr[vector];
	OsTaskVarType *taskPtr = NULL;

	/* Check if we interrupted a task or ISR */
	if( Os_Sys.intNestCnt == 0 ) {
		/* We interrupted a task */
		POSTTASKHOOK();

		/* Save info for preempted pcb */
		taskPtr = get_curr_pcb();
		taskPtr->stack.curr = stack;
		taskPtr->state = ST_READY;
		OS_DEBUG(D_TASK,"Preempted %s\n",taskPtr->name);

		Os_StackPerformCheck(taskPtr);
	} else {
		/* We interrupted an ISR */
	}

	Os_Sys.intNestCnt++;

	/* Grab the ISR "pcb" */
	Os_IsrVarList[isrId].state = ST_ISR_RUNNING;

	Irq_SOI();

#if defined(CFG_HCS12D)
	Os_IsrConstList[isrId].entry();
#else
	Irq_Enable();
	Os_IsrConstList[isrId].entry();
	Irq_Disable();
#endif

	/* Check so that ISR2 haven't disabled the interrupts */
	/** @req OS368 */
	if( Os_IrqAnyDisabled() ) {
		Os_IrqClearAll();
		ERRORHOOK(E_OS_DISABLEDINT);
	}

	/* Check so that the ISR2 have called ReleaseResource() for each GetResource() */
	/** @req OS369 */
	if( Os_TaskOccupiesResources(taskPtr) ) {
		Os_ResourceFreeAll(taskPtr);
		ERRORHOOK(E_OS_RESOURCE);
	}

	Os_IsrVarList[isrId].state = ST_ISR_NOT_RUNNING;

	Irq_EOI();

	--Os_Sys.intNestCnt;

#if defined(CFG_ARM_CM3)
		/* Scheduling is done in PendSV handler for ARM CM3 */
		*((uint32_t volatile *)0xE000ED04) = 0x10000000; // PendSV
#else
	// We have preempted a task
	if( (Os_Sys.intNestCnt == 0) ) {
		OsTaskVarType *new_pcb  = Os_TaskGetTop();

		Os_StackPerformCheck(new_pcb);

		if(     (new_pcb == Os_Sys.currTaskPtr) ||
				(Os_Sys.currTaskPtr->constPtr->scheduling == NON) ||
				!Os_SchedulerResourceIsFree() )
		{
			/* Just bring the preempted task back to running */
			Os_Sys.currTaskPtr->state = ST_RUNNING;
			PRETASKHOOK();
		} else {
			OS_DEBUG(D_TASK,"Found candidate %s\n",new_pcb->name);
			Os_TaskSwapContextTo(NULL,new_pcb);
		}
	} else {
		/* We have a nested interrupt, do nothing */
	}
#endif

	return stack;
}
