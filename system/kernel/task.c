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

#include <stdlib.h>
#include "Os.h"

#include "internal.h"
#include "arc.h"
#include "arch.h"

/** @req OS067 */

_Bool os_pcb_pid_valid( OsPcbType *restrict pcb ) {
	return ( pcb->pid > Os_CfgGetTaskCnt() ) ? 0 : 1;
}
/**
 * Start an extended task.
 * Tasks done:
 * - Grab the internal resource for the process
 * - Set it running state.
 * - Start to execute the process
 *
 */
void Os_TaskStartExtended( void ) {
	OsPcbType *pcb;

	pcb = Os_TaskGetCurrent();
	Os_ResourceGetInternal();
	Os_TaskMakeRunning(pcb);

	PRETASKHOOK();

	Os_ArchFirstCall();

	/** @req OS239 */
	Irq_Disable();
	if( Os_IrqAnyDisabled() ) {
		Os_IrqClearAll();
	}

// TODO:Dont I have to check this at terminate task also?

	/** @req OS069 */
	ERRORHOOK(E_OS_MISSINGEND);

	/** @req OS052 */
	TerminateTask();
}

/**
 * Start an basic task.
 * See extended task.
 */

void Os_TaskStartBasic( void ) {
	OsPcbType *pcb;

	pcb = Os_TaskGetCurrent();
	Os_ResourceGetInternal();
	Os_TaskMakeRunning(pcb);

	PRETASKHOOK();

	Os_ArchFirstCall();


	/** @req OS239 */
	Irq_Disable();
	if( Os_IrqAnyDisabled() ) {
		Os_IrqClearAll();
	}

	/** @req OS069 */
	ERRORHOOK(E_OS_MISSINGEND);

	/** @req OS052 */
	TerminateTask();
}


static void Os_StackSetup( OsPcbType *pcbPtr ) {
	uint8_t *bottom;

	/* Find bottom of the stack so that we can place the
	 * context there.
	 *
	 * stack bottom = high address. stack top = low address
	 */
	bottom = (uint8_t *)pcbPtr->stack.top + pcbPtr->stack.size;
	pcbPtr->stack.curr = bottom;
	// TODO: alignments here..
	// TODO :use function os_arch_get_call_size() ??

	// Make some space for back-chain.
	bottom -= 16;
	// Set the current stack so that it points to the context
	pcbPtr->stack.curr = bottom - Os_ArchGetScSize();

	Os_StackSetEndmark(pcbPtr);
}

/**
 * Fill the stack with a predefined pattern
 *
 * @param pcbPtr Pointer to the pcb to fill with pattern
 */
static void Os_StackFill(OsPcbType *pcbPtr) {
	uint8_t *p = pcbPtr->stack.curr;

	assert(pcbPtr->stack.curr > pcbPtr->stack.top);

	while (p > (uint8_t *) pcbPtr->stack.top) {
		--p;
		*p = STACK_PATTERN;
	}
}

#if 0
/**
 *
 * @param pcbPtr
 */
static void Os_TaskSetEntry(OsPcbType *pcbPtr ) {

}
#endif


/**
 * Setup the context for a pcb. The context differs for different arch's
 * so we call the arch dependent functions also.
 * The context at setup is always a small context.
 *
 * @param pcb Ptr to the pcb to setup context for.
 */
void Os_ContextInit( OsPcbType *pcb ) {

	if( pcb->autostart ) {
		Os_TaskMakeReady(pcb);
	} else {
		pcb->state = ST_SUSPENDED;
	}

	Os_StackSetup(pcb);
	Os_StackFill(pcb);
	Os_ArchSetTaskEntry(pcb);

	Os_ArchSetupContext(pcb);
}

/**
 *
 * @param pcb
 */
void Os_ContextReInit( OsPcbType *pcbPtr ) {
	Os_StackSetup(pcbPtr);
}

/**
 * Search for a specific task in the pcb list.
 *
 * @param tid The task id to search for
 * @return Ptr to the found pcb or NULL
 */
OsPcbType *os_find_task( TaskType tid ) {
	OsPcbType *i_pcb;

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
TaskType Os_AddTask( OsPcbType *pcb ) {
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

/**
 * Find the top priority task. Even the running task is included.
 * TODO: There should be a priority queue (using a heap?) here instead.
 *        giving O(log n) for instertions and (1) for getting the top
 *        prio task. The curerent implementation is ehhhh bad.
 * @return
 */

OsPcbType *Os_TaskGetTop( void ){
	OsPcbType *i_pcb;
	OsPcbType *top_prio_pcb = NULL;
	OsPriorityType top_prio = PRIO_ILLEGAL;

	OS_DEBUG(D_TASK,"os_find_top_prio_proc\n");

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

	assert(top_prio_pcb!=NULL);

	OS_DEBUG(D_TASK,"Found %s\n",top_prio_pcb->name);

	return top_prio_pcb;
}


#define USE_LDEBUG_PRINTF
#include "debug.h"

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

OsPcbType *Os_FindTopPrioTask( void ) {


	return NULL;
}

/**
 * Tries to Dispatch.
 *
 * Used by:
 *   ActivateTask()
 *   WaitEvent()
 *   TerminateTask()
 *   ChainTask()
 *
 * @param force Force a re-scheduling
 *
 */
void Os_Dispatch( _Bool force ) {
	OsPcbType *pcbPtr;
	OsPcbType *currPcbPtr;
	(void)force;

	assert(os_sys.int_nest_cnt == 0);
	assert(os_sys.scheduler_lock == 0 );

	pcbPtr = Os_TaskGetTop();
	currPcbPtr = Os_TaskGetCurrent();
	/* Swap if we found any process or are forced (multiple activations)*/
	if( pcbPtr != currPcbPtr ) {

		/* Add us to the ready list */
		if( currPcbPtr->state & ST_RUNNING ) {
			/** @req OS052 */
			POSTTASKHOOK();
			Os_TaskRunningToReady(currPcbPtr);
		}

		/*
		 * Swap context
		 */
		assert(pcbPtr!=NULL);

		Os_ResourceReleaseInternal();

#if (OS_STACK_MONITORING == 1)
		if( !Os_StackIsEndmarkOk(currPcbPtr) ) {
#if (  OS_SC1 == 1) || (  OS_SC2 == 1)
			/** @req OS068 */
			ShutdownOS(E_OS_STACKFAULT);
#else
#error SC3 or SC4 not supported. Protection hook should be called here
#endif
		}
#endif

		Os_ArchSwapContext(currPcbPtr,pcbPtr);

		pcbPtr = Os_TaskGetCurrent();
		Os_TaskMakeRunning(pcbPtr);

		Os_ResourceGetInternal();

		PRETASKHOOK();

	} else {
		/* We want to run the same task, again. This only happens
		 * when we have multiple activation of a basic task (
		 * extended tasks have an activation limit of 1)
		 */

		/* Setup the stack again, and just call the basic task */
		Os_StackSetup(pcbPtr);
		Os_ArchSetSpAndCall(pcbPtr->stack.curr,Os_TaskStartBasic);
	}
}

// We come here from
// - os_init

/**
 * Called when a task is to be run for the first time.
 */
void Os_TaskSwapContextTo(OsPcbType *old_pcb, OsPcbType *new_pcb ) {

	Os_ArchSwapContextTo(old_pcb,new_pcb);
	/* TODO: When do we return here ?? */
}


void Os_Arc_GetStackInfo( TaskType task, StackInfoType *s) {
	OsPcbType *pcb 	= os_get_pcb(task);

	s->curr 	= Os_ArchGetStackPtr();
	s->top 		= pcb->stack.top;
	s->at_swap 	= pcb->stack.curr;
	s->size 	= pcb->stack.size;
	s->usage 	= (void *)Os_StackGetUsage(pcb);
}


#define TASK_CHECK_ID(x) 				\
	if( (x) > Os_CfgGetTaskCnt()) { \
		rv = E_OS_ID;					\
		goto err; 						\
	}


/**
 * Returns the state of a task (running, ready, waiting, suspended)
 * at the time of calling GetTaskState.
 *
 * @param TaskId  Task reference
 * @param State   Reference to the state of the task
 */

StatusType GetTaskState(TaskType TaskId, TaskStateRefType State) {
	state_t curr_state;
	StatusType rv = E_OK;

	TASK_CHECK_ID(TaskId);

	curr_state = os_pcb_get_state(os_get_pcb(TaskId));

	// TODO: Lazy impl. for now */
	switch(curr_state) {
	case ST_RUNNING: 	*State = TASK_STATE_RUNNING;  	break;
	case ST_WAITING: 	*State = TASK_STATE_WAITING;  	break;
	case ST_SUSPENDED: 	*State = TASK_STATE_SUSPENDED;  break;
	case ST_READY: 		*State = TASK_STATE_READY;  	break;
	}

	// Prevent label warning. Remove when proper error handling is implemented.
	if (0) goto err;

	OS_STD_END_2(OSServiceId_GetTaskState,TaskId, State);
}


/**
 * GetTaskID returns the information about the TaskID of the task
 * which is currently running.
 *
 * @param task_id Reference to the task which is currently running
 * @return
 */
StatusType GetTaskID( TaskRefType task_id ) {
	*task_id = os_sys.curr_pcb->pid;
	return E_OK;
}


ISRType GetISRID( void ) {

	/** @req OS264 */
	if(os_sys.int_nest_cnt == 0 ) {
		return INVALID_ISR;
	}

	/** @req OS263 */
	return (ISRType)Os_TaskGetCurrent()->pid;
}

static inline void Os_Arc_SetCleanContext( OsPcbType *pcb ) {
	if (pcb->proc_type == PROC_EXTENDED) {
		/** @req OSEK ActivateTask Cleanup events
		 * OSEK,ActivateTask, When an extended task is transferred from suspended
		 * state into ready state all its events are cleared.*/
		pcb->ev_set = 0;
		pcb->ev_wait = 0;
	}
	Os_StackSetup(pcb);
	Os_ArchSetTaskEntry(pcb);
	Os_ArchSetupContext(pcb);
}

/**
 * The task <TaskID> is transferred from the suspended state into
 * the  ready state. The operating system ensures that the task
 * code is being executed from the first statement.
 *
 * The service may be called from interrupt  level and from task
 * level (see Figure 12-1).
 * Rescheduling after the call to  ActivateTask depends on the
 * place it is called from (ISR, non preemptable task, preemptable
 * task).
 *
 * If E_OS_LIMIT is returned the activation is ignored.
 * When an extended task is transferred from suspended state
 * into ready state all its events are cleared.
 *
 * Note!
 * ActivateTask will not immediately change the state of the task
 * in case of multiple activation requests. If the task is not
 * suspended, the activation will only be recorded and performed later.
 *
 * @param pid
 * @return
 */

StatusType ActivateTask( TaskType TaskID ) {
	long msr;
	OsPcbType *pcb = os_get_pcb(TaskID);
	StatusType rv = E_OK;

	OS_DEBUG(D_TASK,"ActivateTask %s\n",pcb->name);

#if (OS_STATUS_EXTENDED == STD_ON )
	TASK_CHECK_ID(TaskID);

	/* @req OS093 ActivateTask */
	if( Os_IrqAnyDisabled() ) {
		rv = E_OS_DISABLEDINT;
		goto err;
	}
#endif

	Irq_Save(msr);

	if( os_pcb_get_state(pcb) == ST_SUSPENDED ) {
		pcb->activations++;
		Os_Arc_SetCleanContext(pcb);
		Os_TaskMakeReady(pcb);
	} else {

		if( pcb->proc_type == PROC_EXTENDED ) {
			/** @req OSEK Activate task.
			 * An extended task be activated once. See Chapter 4.3 in OSEK
			 */
			rv = E_OS_LIMIT;
			goto err;
		}

		/** @req OSEK_? Too many task activations */
		if( pcb->activations == pcb->activationLimit ) {
			rv=E_OS_LIMIT;
			goto err;
		} else {
			pcb->activations++;
		}
	}


	/* Preempt only if we are preemptable and target has higher prio than us */
	if(	(Os_TaskGetCurrent()->scheduling == FULL) &&
		(os_sys.int_nest_cnt == 0) &&
		(pcb->prio > Os_TaskGetCurrent()->prio) &&
		(Os_SchedulerResourceIsFree()))
	{
		Os_Dispatch(0);
	}

	Irq_Restore(msr);

	OS_STD_END_1(OSServiceId_ActivateTask,TaskID);
}

/**
 * This  service  causes  the  termination  of  the  calling  task.  The
 * calling  task  is  transferred  from  the  running  state  into  the
 * suspended state.
 *
 * An internal resource assigned to the calling task is automatically
 * released. Other resources occupied by the task shall have been
 * released before the call to TerminateTask. If a resource is still
 * occupied in standard status the behaviour is undefined.
 *
 *   If the call was successful, TerminateTask does not return to the
 * call level and the status can not be evaluated.
 *
 *   If the version with extended status is used, the service returns
 * in case of error, and provides a status which can be evaluated
 * in the application.
 *
 *   If the service TerminateTask is called successfully, it enforces a
 * rescheduling.
 *
 *  [ Ending   a   task   function   without   call   to   TerminateTask
 *    or ChainTask is strictly forbidden and may leave the system in an
 *    undefined state. ]
 *
 * [] is an OSEK requirement and is overridden by OS052
 *
 * @return
 */

StatusType TerminateTask( void ) {
	OsPcbType *curr_pcb = Os_TaskGetCurrent();
	StatusType rv = E_OK;
	uint32_t flags;

	OS_DEBUG(D_TASK,"TerminateTask %s\n",curr_pcb->name);

#if (OS_STATUS_EXTENDED == STD_ON )


	if( os_sys.int_nest_cnt != 0 ) {
		rv =  E_OS_CALLEVEL;
		goto err;
	}

	if ( Os_SchedulerResourceIsOccupied() ) {
		rv =  E_OS_RESOURCE;
		goto err;
	}

	/** @req OS070 */
	if( Os_ResourceCheckAndRelease(curr_pcb) == 1 ) {
		rv =  E_OS_RESOURCE;
		goto err;

	}


#endif


	Irq_Save(flags);

	--curr_pcb->activations;


//	assert(curr_pcb->activations>=0);

	/*@req OSEK TerminateTask
	 * In case of tasks with multiple activation requests,
	 * terminating the current instance of the task automatically puts the next
	 * instance of the same task into the ready state
	 */
	if( curr_pcb->activations <= 0 ) {
		curr_pcb->activations = 0;
		POSTTASKHOOK();
		Os_TaskMakeSuspended(curr_pcb);
	} else {
		/* We need to add ourselves to the ready list again,
		 * with a startup context. */

		/* We are already in ready list..
		 * This should give us a clean start /tojo */
//		Os_Arc_SetCleanContext(curr_pcb);
	}

//	Os_ContextReInit(curr_pcb);

	/* Force the dispatcher to find something, even if its us */
	Os_Dispatch(1);

	Irq_Restore(flags);
	// It must find something here...otherwise something is very wrong..
	assert(0);

	rv = E_NOT_OK;
	goto err;


	OS_STD_END(OSServiceId_TerminateTask);
}

StatusType ChainTask( TaskType TaskId ) {
	OsPcbType *curr_pcb = Os_TaskGetCurrent();
	StatusType rv = E_OK;
	uint32_t flags;

#if (OS_STATUS_EXTENDED == STD_ON )
	TASK_CHECK_ID(TaskId);

	if( os_sys.int_nest_cnt != 0 ) {
		rv =  E_OS_CALLEVEL;
		goto err;
	}

	if ( Os_SchedulerResourceIsOccupied() ) {
		rv =  E_OS_RESOURCE;
		goto err;
	}

	if( Os_ResourceCheckAndRelease(curr_pcb) == 1 ) {
		rv =  E_OS_RESOURCE;
		goto err;
	}
#endif
	OsPcbType *pcb = os_get_pcb(TaskId);

	Irq_Save(flags);

	if (curr_pcb == pcb) {
		/* If we are chaining same task Dispatch will give us a clean start */
		Os_Dispatch(1);

		Irq_Restore(flags);
		/* It must find something here...otherwise something is very wrong.. */
		assert(0);

	} else {
		/* We are chaining another task
		 * We need to make sure it is in valid state */
		if( os_pcb_get_state(pcb) != ST_SUSPENDED ) {
			if( pcb->proc_type == PROC_EXTENDED ) {
				/** @req OSEK Activate task.
				 * An extended task be activated once. See Chapter 4.3 in OSEK */
				rv = E_OS_LIMIT;
				goto err;
			}

			/** @req OSEK_? Too many task activations */
			if( pcb->activations == pcb->activationLimit ) {
				rv=E_OS_LIMIT;
				goto err;
			}
		}

		/* Terminate current task */
		--curr_pcb->activations;
		if( curr_pcb->activations <= 0 ) {
			curr_pcb->activations = 0;
			POSTTASKHOOK();
			Os_TaskMakeSuspended(curr_pcb);
		}

		/* Activate chained task
		 * We know it's ok here */
		pcb->activations++;
		if( os_pcb_get_state(pcb) == ST_SUSPENDED ) {
			Os_TaskMakeReady(pcb);
		}

		if(	(os_sys.int_nest_cnt == 0) &&
			(Os_SchedulerResourceIsFree()))
		{
			Os_Dispatch(1);
		}

		/* we will only come back here if we had activations left */
		assert(curr_pcb->activations > 0);

		/* restart ourselves */
		Os_StackSetup(curr_pcb);
		Os_ArchSetSpAndCall(curr_pcb->stack.curr, Os_TaskStartBasic);
	}

	Irq_Restore(flags);

	if (rv != E_OK) goto err;

	OS_STD_END_1(OSServiceId_ChainTask,TaskId);
}

/**
 * If a higher-priority task is ready, the internal resource of the task
 * is released, the current task is put into the  ready state, its
 * context is saved and the higher-priority task is executed.
 * Otherwise the calling task is continued.
 *
 * TODO: The OSEK spec says a lot of strange things under "particulareties"
 * that I don't understand
 *
 * See OSEK/VDX 13.2.3.4
 *
 */
StatusType Schedule( void ) {
//	OsPcbType *pcb;
//	OsPcbType *curr_pcb = get_curr_pcb();
	StatusType rv = E_OK;
	uint32_t flags;

	/* Check that we are not calling from interrupt context */
	if( os_sys.int_nest_cnt != 0 ) {
		rv =  E_OS_CALLEVEL;
		goto err;
	}

	/* We need to figure out if we have an internal resource,
	 * otherwise no re-scheduling.
	 * NON  - Have internal resource prio OS_RES_SCHEDULER_PRIO (32+)
	 * FULL - Assigned internal resource OR
	 *        No assigned internal resource.
	 * */
	if( Os_TaskGetCurrent()->scheduling != NON ) {
		return E_OK;
	}

#if 0
	if( os_get_resource_int_p() == NULL ) {
		/* We do nothing */
		return E_OK;
	}
#endif

	OsPcbType* top_pcb = Os_TaskGetTop();
	/* only dispatch if some other ready task has higher prio */
	if (top_pcb->prio > Os_TaskGetCurrent()->prio) {
		Irq_Save(flags);
		Os_Dispatch(0);
		Irq_Restore(flags);
	}

	// Prevent label warning. Remove this when proper error handling is implemented.
	if (0) goto err;

	OS_STD_END(OSServiceId_Schedule);
}

