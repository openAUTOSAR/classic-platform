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
#include "Os.h"
#include "Compiler.h"
#include "internal.h"
#include "application.h"
#include "sys.h"
#include "isr.h"
#include "irq.h"
#include "arc.h"

#define ILL_VECTOR	0xff

extern uint8_t Os_VectorToIsr[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS];
#if OS_ISR_CNT!=0
extern const OsIsrConstType Os_IsrConstList[OS_ISR_CNT];
#endif

#if OS_ISR_MAX_CNT!=0
OsIsrVarType Os_IsrVarList[OS_ISR_MAX_CNT];
#endif

uint8_t Os_IsrStack[OS_INTERRUPT_STACK_SIZE] __balign(0x100);

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
#if OS_ISR_CNT != 0
static void Os_IsrAddWithId( const OsIsrConstType * restrict isrPtr, int id ) {
	Os_IsrVarList[id].constPtr = isrPtr;
	Os_IsrVarList[id].id = id;
#if defined(CFG_OS_ISR_HOOKS)
	Os_IsrVarList[id].preemtedId = INVALID_ISR;
#endif
	Os_VectorToIsr[isrPtr->vector + IRQ_INTERRUPT_OFFSET ] = id;
	Irq_EnableVector( isrPtr->vector, isrPtr->priority, Os_ApplGetCore(isrPtr->appOwner )  );
}
#endif

void Os_IsrInit( void ) {

	Irq_Init();

	Os_Sys.isrCnt = OS_ISR_CNT;
	/* Probably something smarter, but I cant figure out what */
	memset(Os_VectorToIsr,ILL_VECTOR,NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS);

#if OS_ISR_CNT != 0
	/* Attach the interrupts */
	for (int i = 0; i < Os_Sys.isrCnt; i++) {
		Os_IsrAddWithId(&Os_IsrConstList[i],i);
	}
#endif
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
	ISRType installedId;

	assert( isrPtr != NULL );
	assert( (isrPtr->vector + IRQ_INTERRUPT_OFFSET) < NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS );

	/* Check if we already have installed it */
	installedId = Os_VectorToIsr[isrPtr->vector + IRQ_INTERRUPT_OFFSET ];

	if( installedId != ILL_VECTOR ) {
		/* The vector is already installed */
		id = installedId;
	} else {
		/* It a new vector */
		id = Os_Sys.isrCnt++;
		/* Since OS_ISR_MAX_CNT defines the allocation limit for Os_IsrVarList,
		 * we must not allocate more IDs than that */
		assert(id<OS_ISR_MAX_CNT);

		Os_IsrVarList[id].constPtr = isrPtr;
		Os_IsrVarList[id].id = id;
#if defined(CFG_OS_ISR_HOOKS)
		Os_IsrVarList[id].preemtedId = INVALID_ISR;
#endif
		Os_VectorToIsr[isrPtr->vector + IRQ_INTERRUPT_OFFSET ] = id;
		Irq_EnableVector( isrPtr->vector, isrPtr->priority, Os_ApplGetCore(isrPtr->appOwner )  );
	}

	return id;
}

#if 0
const OsIsrConstType * Os_IsrGet( int16_t vector) {
	return &Os_IsrConstList[Os_VectorToIsr[vector]];
}
#endif

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


#if 0
/**
 * Before we have proper editor for ISR2 use this function to add resources
 * to an ISR2
 *
 * @param isr
 * @param resource
 * @return
 */
StatusType Os_IsrAddResource( TaskType isr, ResourceType resource ) {
	(void)isr;
	(void)resource;

	return E_OK;
}
#endif

#if defined(CFG_ARM_CM3)
extern void Irq_EOI2( void );

void TailChaining(void *stack)
{
	struct OsTaskVar *pPtr = NULL;

	POSTTASKHOOK();

	/* Save info for preempted pcb */
	pPtr = Os_SysTaskGetCurr();
	pPtr->stack.curr = stack;
	pPtr->state = ST_READY;
	OS_DEBUG(D_TASK,"Preempted %s\n",pPtr->name);

	Os_StackPerformCheck(pPtr);

	/* We interrupted a task */
	OsTaskVarType *new_pcb  = Os_TaskGetTop();

	Os_StackPerformCheck(new_pcb);

	if(     (new_pcb == Os_Sys.currTaskPtr) ||
			(Os_Sys.currTaskPtr->constPtr->scheduling == NON) ||
			!Os_SchedulerResourceIsFree() )
	{
		/* Just bring the preempted task back to running */
		Os_TaskSwapContextTo(NULL,Os_Sys.currTaskPtr);
	} else {
		OS_DEBUG(D_TASK,"Found candidate %s\n",new_pcb->name);
		Os_TaskSwapContextTo(NULL,new_pcb);
	}
}

void Os_Isr_cm3( int16_t vector ) {

	OsIsrVarType *isrPtr =  &Os_IsrVarList[Os_VectorToIsr[vector]];

	assert( isrPtr != NULL );

	if( isrPtr->constPtr->type == ISR_TYPE_1) {
		isrPtr->constPtr->entry();
		return;
	}

	Os_Sys.intNestCnt++;
	isrPtr->state = ST_ISR_RUNNING;

	Irq_Enable();
	isrPtr->constPtr->entry();
	Irq_Disable();

	/* Check so that ISR2 haven't disabled the interrupts */
	/** @req OS368 */
	if( Os_SysIntAnyDisabled() ) {
		Os_SysIntClearAll();
		ERRORHOOK(E_OS_DISABLEDINT);
	}

	/* Check so that the ISR2 have called ReleaseResource() for each GetResource() */
	/** @req OS369 */
	if( Os_IsrOccupiesResources(isrPtr) ) {
		Os_IsrResourceFreeAll(isrPtr);
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

const OsIsrVarType *Os_IsrGet( ISRType id ) {
#if OS_ISR_MAX_CNT != 0
	if( id < Os_Sys.isrCnt ) {
		return &Os_IsrVarList[id];
	} else  {
		return NULL;
	}
#else
	(void)id;
	return NULL;
#endif
}

ApplicationType Os_IsrGetApplicationOwner( ISRType id ) {
	ApplicationType rv = INVALID_OSAPPLICATION;

#if (OS_ISR_MAX_CNT!=0)
	if( id < Os_Sys.isrCnt ) {
		rv = Os_IsrGet(id)->constPtr->appOwner;
	}
#else
	(void)id;
#endif
	return rv;
}


#if defined(CFG_ARM_CR4)
void *Os_Isr_cr4( void *stack, int16_t virtualVector, int16_t vector ) {
	OsIsrVarType *isrPtr =  &Os_IsrVarList[Os_VectorToIsr[virtualVector]];
	isrPtr->activeVector = vector;
	return Os_Isr(stack, virtualVector);
}
#endif

/**
 * Handle ISR type 2 interrupts from interrupt controller.
 *
 * @param stack Pointer to the current stack
 * @param vector
 */
void *Os_Isr( void *stack, int16_t vector ) {

	OsIsrVarType *isrPtr =  &Os_IsrVarList[Os_VectorToIsr[vector]];
	OsTaskVarType *taskPtr = NULL;
	OsIsrVarType *oldIsrPtr;

	assert( isrPtr != NULL );

	if( isrPtr->constPtr->type == ISR_TYPE_1) {
		isrPtr->constPtr->entry();
		Irq_EOI();
		return stack;
	}

	/* Check if we interrupted a task or ISR */
	if( Os_Sys.intNestCnt == 0 ) {
		/* We interrupted a task */
		POSTTASKHOOK();

		/* Save info for preempted pcb */
		taskPtr = Os_SysTaskGetCurr();
		taskPtr->stack.curr = stack;
		taskPtr->state = ST_READY;
		OS_DEBUG(D_TASK,"Preempted %s\n",taskPtr->constPtr->name);

		Os_StackPerformCheck(taskPtr);
	} else {
		/* We interrupted an ISR, save it */
		oldIsrPtr = Os_Sys.currIsrPtr;
#if defined(CFG_OS_ISR_HOOKS)
		isrPtr->preemtedId = oldIsrPtr->id;
		Os_PostIsrHook(oldIsrPtr->id);
#endif
	}

	Os_Sys.intNestCnt++;

	isrPtr->state = ST_ISR_RUNNING;
	Os_Sys.currIsrPtr = isrPtr;

	Irq_SOI();

#if defined(CFG_OS_ISR_HOOKS)
	Os_PreIsrHook(isrPtr->id);
#endif


#if defined(CFG_HCS12D)
	isrPtr->constPtr->entry();
#else
	Irq_Enable();
	isrPtr->constPtr->entry();
	Irq_Disable();
#endif

#if defined(CFG_OS_ISR_HOOKS)
	Os_PostIsrHook(isrPtr->id );
	if( isrPtr->preemtedId != INVALID_ISR ) {
		Os_Sys.currIsrPtr = &Os_IsrVarList[isrPtr->preemtedId];
		Os_PreIsrHook(isrPtr->preemtedId );
		isrPtr->preemtedId = INVALID_ISR;
	}
#endif


	/* Check so that ISR2 haven't disabled the interrupts */
	/** @req OS368 */
	if( Os_SysIntAnyDisabled() ) {
		Os_SysIntClearAll();
		ERRORHOOK(E_OS_DISABLEDINT);
	}

	/* Check so that the ISR2 have called ReleaseResource() for each GetResource() */
	/** @req OS369 */
	if( Os_IsrOccupiesResources(isrPtr) ) {
		Os_IsrResourceFreeAll(isrPtr);
		ERRORHOOK(E_OS_RESOURCE);
	}

	isrPtr->state = ST_ISR_NOT_RUNNING;
	Os_Sys.currIsrPtr = isrPtr;

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
			OS_DEBUG(D_TASK,"Found candidate %s\n",new_pcb->constPtr->name);
			Os_TaskSwapContextTo(NULL,new_pcb);
		}
	} else {
		/* We have a nested interrupt */

		/* Restore current running ISR from stack */
		Os_Sys.currIsrPtr = oldIsrPtr;
	}
#endif

	return stack;
}

void Os_Arc_GetIsrInfo( Arc_PcbType *pcbPtr, ISRType isrId ) {
	const OsIsrVarType *isrPtr = Os_IsrGet(isrId);

	if( isrPtr != NULL ) {
		strncpy(pcbPtr->name,Os_IsrGet(isrId)->constPtr->name,OS_ARC_PCB_NAME_SIZE);
	}

}

int Os_Arc_GetIsrCount( void ) {
	return (int)Os_Sys.isrCnt;
}

