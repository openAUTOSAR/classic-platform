/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/

/* ----------------------------[includes]------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include "Os.h"

#include "application.h"
#include "internal.h"
#include "task_i.h"
#include "sys.h"
#include "arc.h"
#include "arch.h"
#include <string.h>
#include "multicore_i.h"


/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
#if OS_TASK_CNT!=0
OsTaskVarType Os_TaskVarList[OS_TASK_CNT];
#endif

/* ----------------------------[private functions]---------------------------*/
// schedule()
static inline void Os_TaskRunningToReady( OsTaskVarType *pcb ) {
	assert(pcb->state == ST_RUNNING );
	pcb->state = ST_READY;
}


/**
 * Make a task go the READY state
 * Used by API: ActivateTask(), SetEvent()
 *
 * @param pcb
 */
void Os_TaskMakeReady( OsTaskVarType *pcb ) {
	if( !( pcb->state & ( ST_READY | ST_RUNNING )) ) {
		pcb->state = ST_READY;
		TAILQ_INSERT_TAIL(& OS_SYS_PTR->ready_head,pcb,ready_list);
		OS_DEBUG(D_TASK,"Added %s to ready list\n",pcb->constPtr->name);
	}
}

/**
 * Make a task go the the WAITING state
 * Used by API: WaitEvent
 *
 *
 * @param pcb Ptr to the task
 */
void Os_TaskMakeWaiting( OsTaskVarType *pcb )
{
	assert( pcb->state & (ST_READY|ST_RUNNING) );

	pcb->state = ST_WAITING;
	TAILQ_REMOVE(&OS_SYS_PTR->ready_head,pcb,ready_list);
	OS_DEBUG(D_TASK,"Removed %s from ready list\n",pcb->constPtr->name);
}

// Sleeping
#if defined(USE_KERNEL_EXTRA)
static inline void Os_TaskMakeSleeping( OsTaskVarType *pcb )
{
	assert( pcb->state & (ST_READY|ST_RUNNING) );

	pcb->state = ST_WAITING | ST_SLEEPING;
	TAILQ_REMOVE(&OS_SYS_PTR->ready_head,pcb,ready_list);
	OS_DEBUG(D_TASK,"Removed %s from ready list\n",pcb->constPtr->name);
}


static inline void Os_TaskMakeWaitingOnSem( OsTaskVarType *pcb )
{
	assert( pcb->state & (ST_READY|ST_RUNNING) );

	pcb->state = ST_WAITING_SEM;
	TAILQ_REMOVE(&OS_SYS_PTR->ready_head,pcb,ready_list);
	OS_DEBUG(D_TASK,"Removed %s from ready list\n",pcb->constPtr->name);
}


#endif

// Terminate task
static inline void Os_TaskMakeSuspended( OsTaskVarType *pcb )
	{
	assert( pcb->state & (ST_READY|ST_RUNNING) );
	pcb->state = ST_SUSPENDED;
	TAILQ_REMOVE(&OS_SYS_PTR->ready_head,pcb,ready_list);
	OS_DEBUG(D_TASK,"Removed %s from ready list\n",pcb->constPtr->name);
}


/* ----------------------------[public functions]----------------------------*/



/** @req OS067 */

/**
 * Start an extended task.
 * Tasks done:
 * - Grab the internal resource for the process
 * - Set it running state.
 * - Start to execute the process
 *
 */
void Os_TaskStartExtended( void ) {

	OsTaskVarType *pcb;

	pcb = Os_SysTaskGetCurr();
#if 0
	Os_ResourceGetInternal();
	Os_TaskMakeRunning(pcb);
#endif

//	PRETASKHOOK();

	Os_ArchFirstCall();

	/* We got back without any TerminateTask() or ChainTask()
	 *
	 * OSEK:
	 *    Each task shall terminate itself at the end of its code.
	 *    Ending the task without a call to TerminateTask or ChainTask
	 *    is strictly forbidden and causes undefined behaviour.
	 *
	 * Autosar:
	 *    OS052, OS069, OS070 and OS239
	 * */

	/** @req OS239 */
	Irq_Disable();
	if( Os_SysIntAnyDisabled() ) {
		Os_SysIntClearAll();
	}

	/** @req OS070 */
	if( Os_TaskOccupiesResources(pcb) ) {
		Os_TaskResourceFreeAll(pcb);
	}

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

	OsTaskVarType *pcb;

	pcb = Os_SysTaskGetCurr();
#if 0
	Os_ResourceGetInternal();
	Os_TaskMakeRunning(pcb);
#endif

//	PRETASKHOOK();

	Os_ArchFirstCall();

	/** @req OS239 */
	Irq_Disable();
	if( Os_SysIntAnyDisabled() ) {
		Os_SysIntClearAll();
	}

	/** @req OS070 */
	if( Os_TaskOccupiesResources(pcb) ) {
		Os_TaskResourceFreeAll(pcb);
	}

	/** @req OS069 */
	ERRORHOOK(E_OS_MISSINGEND);

	/** @req OS052 */
	TerminateTask();
}


static void Os_StackSetup( OsTaskVarType *pcbPtr ) {
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
static void Os_StackFill(OsTaskVarType *pcbPtr) {
	uint8_t *p = pcbPtr->stack.curr;

	assert(pcbPtr->stack.curr > pcbPtr->stack.top);

	memset(pcbPtr->stack.top,STACK_PATTERN, p - (uint8_t *)pcbPtr->stack.top );
	}


/**
 * Setup the context for a pcb. The context differs for different arch's
 * so we call the arch dependent functions also.
 * The context at setup is always a small context.
 *
 * @param pcb Ptr to the pcb to setup context for.
 */
void Os_TaskContextInit( OsTaskVarType *pcb ) {

	if( pcb->constPtr->autostart ) {
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
 * Find the top priority task. Even the running task is included.
 * TODO: There should be a priority queue (using a heap?) here instead.
 *        giving O(log n) for instertions and (1) for getting the top
 *        prio task. The curerent implementation is ehhhh bad.
 * @return
 */

OsTaskVarType *Os_TaskGetTop( void ){
	OsTaskVarType *i_pcb;
	OsTaskVarType *top_prio_pcb = NULL;
	OsPriorityType top_prio = PRIO_ILLEGAL;

//	OS_DEBUG(D_TASK,"os_find_top_prio_proc\n");

	TAILQ_FOREACH(i_pcb,& OS_SYS_PTR->ready_head,ready_list) {
		// all ready task on the current core are canidates
		if (Os_OnRunningCore(OBJECT_TASK,i_pcb->constPtr->pid)) {
			if( i_pcb->state & (ST_READY|ST_RUNNING)) {
				if( top_prio != PRIO_ILLEGAL ) {
					if( i_pcb->activePriority > top_prio ) {
						top_prio = i_pcb->activePriority;
						top_prio_pcb = i_pcb;
					}
				} else {
					top_prio = i_pcb->activePriority;
					top_prio_pcb = i_pcb;
				}
			} else {
				assert(0);
			}
			}
	}

	assert(top_prio_pcb!=NULL);

	OS_DEBUG(D_TASK,"Found %s\n",top_prio_pcb->constPtr->name);

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
void Os_Dispatch( uint32_t op ) {
	OsTaskVarType *pcbPtr;
	OsTaskVarType *currPcbPtr = Os_SysTaskGetCurr();

	assert(OS_SYS_PTR->intNestCnt == 0);
	assert(Os_SchedulerResourceIsFree());

	/* When calling post hook we must still be in ST_RUNNING */
	assert( currPcbPtr->state & ST_RUNNING );
	POSTTASKHOOK();

	/* Go the correct state for running task */
	if( op  & ( OP_SET_EVENT | OP_SCHEDULE | OP_RELEASE_RESOURCE )) {
		Os_TaskRunningToReady(currPcbPtr);
	} else if( op & (OP_WAIT_EVENT) ) {
		Os_TaskMakeWaiting(currPcbPtr);
#if defined(USE_KERNEL_EXTRA)
	} else if( op & OP_WAIT_SEMAPHORE) {
		Os_TaskMakeWaitingOnSem(currPcbPtr);
	} else if( op & OP_SIGNAL_SEMAPHORE) {
		Os_TaskRunningToReady(currPcbPtr);

	} else if( op & (OP_SLEEP )) {
		Os_TaskMakeSleeping(currPcbPtr);
#endif
	} else if( op & OP_ACTIVATE_TASK ) {
		Os_TaskMakeReady(currPcbPtr);
	} else if( op & OP_CHAIN_TASK ) {
		assert( OS_SYS_PTR->chainedPcbPtr != NULL );

		/*  #  from chain  top
		 * ----------------------------------------------------------
		 *  1    1     1     1    1->RUNNING
		 *  2    1     1     2    1->READY,            2->RUNNING
		 *  3    1     2     2    1->SUSPENDED/READY*, 2->RUNNING
		 *  4    1     2     3    1->SUSPENDED/READY*, 2->READY  , 3-RUNNING
		 *
		 *  *) Depends on the number of activations.
		 *
		 *  - Chained task is always READY when coming from ChainTask()
		 */
		if( currPcbPtr != OS_SYS_PTR->chainedPcbPtr ) {
			/* #3 and #4 */
			--currPcbPtr->activations;
			if( currPcbPtr->activations <= 0 ) {
				currPcbPtr->activations = 0;
				Os_TaskMakeSuspended(currPcbPtr);
			} else {
				Os_TaskRunningToReady(currPcbPtr);
			}
			/* Chained task is already in READY */
		}
		OS_SYS_PTR->chainedPcbPtr = NULL;

	} else if( op & OP_TERMINATE_TASK ) {
		/*@req OSEK TerminateTask
		 * In case of tasks with multiple activation requests,
		 * terminating the current instance of the task automatically puts the next
		 * instance of the same task into the ready state
		 */
		--currPcbPtr->activations;

		if( currPcbPtr->activations <= 0 ) {
			currPcbPtr->activations = 0;
			Os_TaskMakeSuspended(currPcbPtr);
		}
	} else {
		assert(0);
	}

	pcbPtr = Os_TaskGetTop();

	/* Swap if we found any process or are forced (multiple activations)*/
	if( pcbPtr != currPcbPtr ) {

		if( (op & OP_CHAIN_TASK) && ( currPcbPtr == OS_SYS_PTR->chainedPcbPtr ) ) {
			/* #2 */
			Os_TaskRunningToReady(currPcbPtr);
		}
		/*
		 * Swap context
		 */
		assert(pcbPtr!=NULL);

		Os_ResourceReleaseInternal();

#if (OS_STACK_MONITORING == 1)
		if( !Os_StackIsEndmarkOk(currPcbPtr) ) {
#if (  OS_SC1 == STD_ON) || (  OS_SC2 == STD_ON )
			/** @req OS068 */
			ShutdownOS(E_OS_STACKFAULT);
#else
			/** @req OS396
			 * If a stack fault is detected by stack monitoring AND the configured scalability
			 * class is 3 or 4, the Operating System module shall call the ProtectionHook() with
			 * the status E_OS_STACKFAULT.
			 * */
			PROTECTIONHOOK(E_OS_STACKFAULT);
#endif
		}
#endif
		OS_DEBUG(D_TASK,"Swapping to: %s\n",pcbPtr->constPtr->name);
		Os_TaskSwapContext(currPcbPtr,pcbPtr);
	} else {
		OS_DEBUG(D_TASK,"Continuing task %s\n",pcbPtr->constPtr->name);
		/* Setup the stack again, and just call the basic task */
		Os_StackSetup(pcbPtr);
		/* TODO: release and get the internal resource ? */
		Os_TaskMakeRunning(pcbPtr);
		PRETASKHOOK();
		Os_ArchSetSpAndCall(pcbPtr->stack.curr,Os_TaskStartBasic);
		assert(0);
	}
}


/*
 * Thoughts on task switching and memory protection
 *
 * If we would have had memory protection:
 * - Applications have their own MMU settings.
 * - Swapping between tasks in same Application does NOT involve the MMU.
 * - When running a non-trusted Application I need will have to:
 *   - Run kernel in supervisor mode.
 *   - Trap the start of each task
 *   - All calls to the kernel will have a trap interface, i.e.  Os_ResourceGetInternal(ActivateTask(TASK_ID_foo);
 *   - An ISR2:
 *     - The interupt is taken, the kernel runs in supervisor mode
 *     - If the ISR2 activates
 *
 * Stack design:
 * ALT1: 1 kernel stack...
 * ALT2:
 *
 *  Do we need virtual memory??
 */

void Os_TaskSwapContext(OsTaskVarType *old_pcb, OsTaskVarType *new_pcb ) {
	Os_SysTaskSetCurr(new_pcb);
#if	(OS_USE_APPLICATIONS == STD_ON)
	OS_SYS_PTR->currApplId = new_pcb->constPtr->applOwnerId;
#endif
	Os_ResourceGetInternal();
	Os_TaskMakeRunning(new_pcb);
	/* TODO: The pretask hook is not called with the right stack
	 * (it's called on the old task stack, not the new ) */
	PRETASKHOOK();
	Os_ArchSwapContext(old_pcb,new_pcb);
}

void Os_TaskSwapContextTo(OsTaskVarType *old_pcb, OsTaskVarType *new_pcb ) {
	Os_SysTaskSetCurr(new_pcb);
#if	(OS_USE_APPLICATIONS == STD_ON)
	OS_SYS_PTR->currApplId = new_pcb->constPtr->applOwnerId;
#endif
	Os_ResourceGetInternal();
	Os_TaskMakeRunning(new_pcb);
	PRETASKHOOK();
	Os_ArchSwapContextTo(old_pcb,new_pcb);
	assert(0);
}


void Os_Arc_GetStackInfo( TaskType task, StackInfoType *s) {
	OsTaskVarType *pcb 	= Os_TaskGet(task);

	s->curr 	= Os_ArchGetStackPtr();
	s->top 		= pcb->stack.top;
	s->at_swap 	= pcb->stack.curr;
	s->size 	= pcb->stack.size;
	s->usage 	= (void *)Os_StackGetUsage(pcb);
}


void Os_Arc_GetTaskName(char *str, TaskType taskId) {
	strncpy(str, Os_TaskGet(taskId)->constPtr->name, TASK_NAME_SIZE);
}

#define TASK_CHECK_ID(x) 				\
	if( (x) > OS_TASK_CNT) { \
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
	OsTaskVarType *pcb 	= Os_TaskGet(TaskId);

	TASK_CHECK_ID(TaskId);

#if	(OS_APPLICATION_CNT > 1) && ( OS_NUM_CORES > 1)
	if (Os_ApplGetCore(pcb->constPtr->applOwnerId) != GetCoreID()) {
		StatusType status = Os_NotifyCore(Os_ApplGetCore(pcb->constPtr->applOwnerId),
		                                  OSServiceId_GetTaskState,
		                                  TaskId,
		                                  (uint32_t)State,
		                                  (uint32_t)0);
		return status;
	}
#endif

	curr_state = os_pcb_get_state(pcb);

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
StatusType GetTaskID( TaskRefType TaskID ) {
	StatusType rv = E_OK;
	*TaskID = INVALID_TASK;

	/* Test specification say return CALLEVEL if in ISR
	 * but impl. spec says otherwise */
	if( OS_SYS_PTR->intNestCnt == 0 ) {
		if( OS_SYS_PTR->currTaskPtr->state & ST_RUNNING ) {
			*TaskID = OS_SYS_PTR->currTaskPtr->constPtr->pid;
		} else {
			/* This is not a real error since this could
			 * be the case when called from ErrorHook */
		}
	}

    return rv;
}


/**
 * This service returns the identifier of the currently executing ISR
 *
 * If its caller is not a category 2 ISR (or Hook routines called
 * inside a category 2 ISR), GetISRID() shall return INVALID_ISR.
 *
 * @return
 */
ISRType GetISRID( void ) {

	/** @req OS264 */
	if(OS_SYS_PTR->intNestCnt == 0 ) {
		return INVALID_ISR;
	}

	/** @req OS263 */
	return (ISRType)Os_SysIsrGetCurr()->id;
}

static inline void Os_Arc_SetCleanContext( OsTaskVarType *pcb ) {
	if (pcb->constPtr->proc_type == PROC_EXTENDED) {
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
	OsTaskVarType *pcb = Os_TaskGet(TaskID);
	StatusType rv = E_OK;

	OS_DEBUG(D_TASK,"# ActivateTask %s\n",pcb->constPtr->name);

#if (OS_STATUS_EXTENDED == STD_ON )
	/* extended */
	TASK_CHECK_ID(TaskID);
#endif

	Irq_Save(msr);

#if	(OS_APPLICATION_CNT > 1)

	rv = Os_ApplHaveAccess( pcb->constPtr->accessingApplMask );
	if( rv != E_OK ) {
		goto err;
	}
#if (OS_NUM_CORES > 1)

	if (Os_ApplGetCore(pcb->constPtr->applOwnerId) != GetCoreID()) {
		StatusType status = Os_NotifyCore(Os_ApplGetCore(pcb->constPtr->applOwnerId),
		                                  OSServiceId_ActivateTask,
		                                  TaskID,
		                                  (uint32_t)NULL,
		                                  (uint32_t)NULL);
		return status;
	}	
#endif
#endif

	/* @req OS093 ActivateTask */
	if( Os_SysIntAnyDisabled() ) {
		/* Standard */
		rv = E_OS_DISABLEDINT;
		goto err;
	}

	pcb->activations++;
	if( os_pcb_get_state(pcb) != ST_SUSPENDED ) {
		/** @req OSEK_? Too many task activations */
		if( pcb->activations >= (pcb->constPtr->activationLimit + 1) ) {
			/* Standard */
			rv=E_OS_LIMIT;
			Irq_Restore(msr);
			--pcb->activations;
			goto err;
		}
	} else {
		/* We have a suspended task, make it ready for use */
		assert( pcb->activations == 1 );
		Os_Arc_SetCleanContext(pcb);
		Os_TaskMakeReady(pcb);
	}

	/* Preempt only if we are preemptable and target has higher prio than us */
	if(	(Os_SysTaskGetCurr()->constPtr->scheduling == FULL) &&
		(OS_SYS_PTR->intNestCnt == 0) &&
		(pcb->activePriority > Os_SysTaskGetCurr()->activePriority) &&
		(Os_SchedulerResourceIsFree()))
	{
		Os_Dispatch(OP_ACTIVATE_TASK);
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
	OsTaskVarType *curr_pcb = Os_SysTaskGetCurr();
	StatusType rv;

	OS_DEBUG(D_TASK,"# TerminateTask %s\n",curr_pcb->constPtr->name);

#if (OS_STATUS_EXTENDED == STD_ON )


	if( OS_SYS_PTR->intNestCnt != 0 ) {
		rv =  E_OS_CALLEVEL;
		goto err;
	}

	if( Os_TaskOccupiesResources(curr_pcb) ) {
		/* Note! Do NOT release the resource here */
		rv =  E_OS_RESOURCE;
		goto err;
	}

	if (Os_TaskOccupiesSpinlocks(curr_pcb) ) {
		rv =  E_OS_SPINLOCK;
		goto err;
	}

#endif
	Irq_Disable();

	/* Force the dispatcher to find something, even if its us */
	Os_Dispatch(OP_TERMINATE_TASK);

	assert(0);

	OS_STD_END(OSServiceId_TerminateTask);
}

StatusType ChainTask( TaskType TaskId ) {
	OsTaskVarType *curr_pcb = Os_SysTaskGetCurr();
	StatusType rv = E_OK;
	uint32_t flags;
	OsTaskVarType *pcb = Os_TaskGet(TaskId);


	OS_DEBUG(D_TASK,"# ChainTask %s\n",curr_pcb->constPtr->name);

#if (OS_STATUS_EXTENDED == STD_ON )
	/* extended */
	TASK_CHECK_ID(TaskId);

	if( OS_SYS_PTR->intNestCnt != 0 ) {
		/* extended */
		rv = E_OS_CALLEVEL;
		goto err;
	}

#endif

	Irq_Save(flags);

#if (OS_STATUS_EXTENDED == STD_ON )
#if 0
	if ( Os_SchedulerResourceIsOccupied() ) {
		/* extended */
		rv = E_OS_RESOURCE;
		Irq_Restore(flags);
		goto err;
	}
#endif

	if( Os_TaskOccupiesResources(curr_pcb) ) {
		/* extended */
		rv = E_OS_RESOURCE;
		Irq_Restore(flags);
		goto err;
	}

	if (Os_TaskOccupiesSpinlocks(curr_pcb) ) {
		rv =  E_OS_SPINLOCK;
		Irq_Restore(flags);
		goto err;
	}
#endif


//	if( os_pcb_get_state(pcb) != ST_SUSPENDED ) {
	if (curr_pcb != pcb) {
		/** @req OSEK_? Too many task activations */
		if( (pcb->activations + 1) >  pcb->constPtr->activationLimit ) {
			/* standard */
			rv = E_OS_LIMIT;
			Irq_Restore(flags);
			goto err;
		}

		if( os_pcb_get_state(pcb) == ST_SUSPENDED ) {
			assert( pcb->activations == 0 );
			Os_Arc_SetCleanContext(pcb);
			Os_TaskMakeReady(pcb);
		}

		pcb->activations++;

	}


	OS_SYS_PTR->chainedPcbPtr = pcb;


#if	(OS_APPLICATION_CNT > 1) && (OS_NUM_CORES > 1)
	if (Os_ApplGetCore(pcb->constPtr->applOwnerId) != GetCoreID()) {
		StatusType status = Os_NotifyCore(Os_ApplGetCore(pcb->constPtr->applOwnerId),
		                                  OSServiceId_ChainTask,
		                                  TaskId,
		                                  0,
		                                  0);
		OS_SYS_PTR->chainedPcbPtr = curr_pcb;
		(void)status;
	}
#endif

	Os_Dispatch(OP_CHAIN_TASK);

	assert( 0 );

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
	StatusType rv = E_OK;
	uint32_t flags;
	OsTaskVarType *curr_pcb = Os_SysTaskGetCurr();

	OS_DEBUG(D_TASK,"# Schedule %s\n",curr_pcb->constPtr->name);

	/* Check that we are not calling from interrupt context */
	if( OS_SYS_PTR->intNestCnt != 0 ) {
		rv =  E_OS_CALLEVEL;
		goto err;
	}

	if ( Os_TaskOccupiesResources(curr_pcb) ) {
		rv = E_OS_RESOURCE;
		goto err;
	}

	assert( Os_SysTaskGetCurr()->state & ST_RUNNING );

	/* We need to figure out if we have an internal resource,
	 * otherwise no re-scheduling.
	 * NON  - Have internal resource prio OS_RES_SCHEDULER_PRIO (32+)
	 * FULL - Assigned internal resource OR
	 *        No assigned internal resource.
	 * */
	if( Os_SysTaskGetCurr()->constPtr->scheduling != NON ) {
		return E_OK;
	}
	{
        Irq_Save(flags);
        OsTaskVarType* top_pcb = Os_TaskGetTop();
        /* only dispatch if some other ready task has higher prio */
        if (top_pcb->activePriority > Os_SysTaskGetCurr()->activePriority) {
            Os_Dispatch(OP_SCHEDULE);
        }

        Irq_Restore(flags);
	}
	// Prevent label warning. Remove this when proper error handling is implemented.
	if (0) goto err;

	OS_STD_END(OSServiceId_Schedule);
}


#if TASK_NAME_SIZE != OS_ARC_PCB_NAME_SIZE
#error Sized does not match. Correct it
#endif

void Os_Arc_GetTaskInfo( Arc_PcbType *pcbPtr, TaskType taskId ) {
	OsTaskVarType *taskPtr = Os_TaskGet(taskId);

	strncpy(pcbPtr->name,taskPtr->constPtr->name,TASK_NAME_SIZE);
}



