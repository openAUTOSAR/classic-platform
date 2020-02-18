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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.3.0 */

/* ----------------------------[includes]------------------------------------*/

#include "os_i.h"
#include "debug.h"
/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
void Os_StackSetup( OsTaskVarType *pcbPtr );
/* ----------------------------[private variables]---------------------------*/
#if OS_TASK_CNT!=0
OsTaskVarType Os_TaskVarList[OS_TASK_CNT];
#endif

/* ----------------------------[private functions]---------------------------*/
/**
 * @brief   Change state of a running task to not running.
 * @param[in] pcb  Pointer to task
 */
static inline void Os_TaskRunningToReady( OsTaskVarType *currPcbPtr ) {
    ASSERT(currPcbPtr->state == ST_RUNNING );
    currPcbPtr->state = ST_READY;
}

/**
 * @brief   Remove task from ready queue
 * @param[in]  pcb  Pointer to task
 */
/*lint --e{818} MISRA:OTHER:need not be const pointer:[MISRA 2012 Rule 8.13, advisory] */
static void Os_ReadyQRemove(OsTaskVarType *tPtr) {
    /*lint -e{9012, 9036} MISRA:EXTERNAL_FILE::[MISRA 2012 Rule 15.6, required], [MISRA 2012 Rule 14.4, required] */
    TAILQ_REMOVE(&OS_SYS_PTR->ready_head,tPtr,ready_list);
    OS_DEBUG(D_TASK,"Removed %s from ready list\n",pcb->constPtr->name);
}

/**
 * @brief   Add task to ready queue
 * @param[in] pcb  Pointer to task
 */
static void Os_ReadyQEnqueue(OsTaskVarType *pcb) {
    /*lint -e{9036} MISRA:EXTERNAL_FILE::[MISRA 2012 Rule 14.4, required] */
    TAILQ_INSERT_TAIL(& OS_SYS_PTR->ready_head,pcb,ready_list);
}



#if defined(CFG_KERNEL_EXTRA)
/**
 * @brief  Change state of a task to sleeping and remove from
 *         ready queue.
 * @param  pcb  Pointer to task
 */
static inline void Os_TaskMakeSleeping( OsTaskVarType *pcb )
{
    ASSERT( pcb->state & (ST_READY|ST_RUNNING) );

    pcb->state = ST_SLEEPING;
    Os_ReadyQRemove(pcb);
}


/**
 * @brief  Change state of a task to waiting on semaphore.
 *         Remove from ready queue.
 * @param  pcb  Pointer to task
 */
static inline void Os_TaskMakeWaitingOnSem( OsTaskVarType *pcb )
{
    ASSERT( pcb->state & (ST_READY|ST_RUNNING) );

    pcb->state = ST_WAIT_SEM;
    Os_ReadyQRemove(pcb);
}

#endif


/**
 * @brief   Change state of a task to waiting on suspended.
 *          Remove from ready queue.
 *
 * @param[in] pcb  Pointer to task
 */
static inline void Os_TaskMakeSuspended( OsTaskVarType *pcb )
    {
    ASSERT( pcb->state & (ST_READY|ST_RUNNING) );
    pcb->state = ST_SUSPENDED;
    Os_ReadyQRemove(pcb);

}

/**
 * @brief   Set a clean context of a task.
 * @details When a task a restarted (chained, terminate/activate) the context
 *          is setup as virgin.
 *
 * @param[in] pcb  Pointer to task
 */
static inline void Os_Arc_SetCleanContext( OsTaskVarType *pcb ) {
    if (pcb->constPtr->proc_type == PROC_EXTENDED) {
        /** OSEK ActivateTask Cleanup events
         * OSEK,ActivateTask, When an extended task is transferred from suspended
         * state into ready state all its events are cleared.*/
        pcb->ev_set = 0;
        pcb->ev_wait = 0;
    }
    Os_StackSetup(pcb);
    Os_ArchSetupContext(pcb);
}

/* ----------------------------[public functions]----------------------------*/


/**
 * Make a task go the READY state
 * Used by API: ActivateTask(), SetEvent()
 *
 * @param[in] currPcbPtr
 */
void Os_TaskMakeReady( OsTaskVarType *pcb ) {
    if( ( pcb->state & ( ST_READY | ST_RUNNING ) ) == 0 ) {
        pcb->state = ST_READY;
        Os_ReadyQEnqueue(pcb);
        OS_DEBUG(D_TASK,"Added %s to ready list\n",pcb->constPtr->name);
    }
}

/**
 * Make a task go the the WAITING state
 * Used by API: WaitEvent
 *
 *
 * @param[in] pcb Ptr to the task
 */
void Os_TaskMakeWaiting( OsTaskVarType *pcb )
{
    ASSERT( pcb->state & (ST_READY|ST_RUNNING) );

    pcb->state = ST_WAIT_EVENT;
    Os_ReadyQRemove(pcb);
    OS_DEBUG(D_TASK,"Removed %s from ready list\n",pcb->constPtr->name);
}

/**
 * Fill the stack with a predefined pattern
 *
 * @param pcbPtr Pointer to the pcb to fill with pattern
 */
static void Os_StackFill(const OsTaskVarType *pcbPtr) {
    const uint8 *p = pcbPtr->stack.curr;

    /*lint -e{946}  MISRA:FALSE_POSITIVE:Pointers belong to the same "array":[MISRA 2012 Rule 18.3, required]*/
    ASSERT(pcbPtr->stack.curr > pcbPtr->stack.top);
    /*lint -e{9033, 946, 947}  MISRA:FALSE_POSITIVE:Pointers belong to the same "array":
    *[MISRA 2012 Rule 18.2, required], [MISRA 2012 Rule 18.3, required]
    */
    memset(pcbPtr->stack.top,STACK_PATTERN, (size_t)(p - (uint8 *)pcbPtr->stack.top) );
}

/* ----------------------------[public functions prototype]----------------------------*/
void Os_TaskPost( void );
//void Os_TaskSetupAndSwap( void );
/* ----------------------------[public functions definition]----------------------------*/


#if defined(CFG_PPC) || defined(CFG_ARMV7_AR) || defined (CFG_ARM_CR4) || defined(CFG_RH850) || defined(CFG_TMS570LC43X) || defined(CFG_TC2XX) || defined(CFG_TC3XX)
/**
 * We come here after a task have returned.
 *
 */

void Os_TaskPost( void ) {

    /* We must manipulate OS structures so swap to privileged mode for SC3 and SC4 alone*/
#if ((OS_SC3 == STD_ON) || (OS_SC4 == STD_ON))
#if defined(CFG_PPC) || defined(CFG_TMS570)
    OS_TRAP_Os_SetPrivilegedMode();
#elif defined(CFG_TC2XX)
    uint32 pcxi = Os_GetCurrentPcxi();
    Os_ArchToPrivilegedMode(pcxi);
#endif // arch
#endif // SC3 || SC4


    OsTaskVarType *currPcbPtr= OS_SYS_PTR->currTaskPtr;

    /** @req SWS_Os_00239 */
    Irq_Disable();
    if( Os_SysIntAnyDisabled() ) {
        Os_SysIntClearAll();
    }

    /** @req SWS_Os_00070 */
    if( Os_TaskOccupiesResources(currPcbPtr) ) {
        Os_TaskResourceFreeAll(currPcbPtr);
    }

    /** @req SWS_Os_00069 */
    Os_CallErrorHook(E_OS_MISSINGEND);

    /** @req SWS_Os_00052 */
    (void)TerminateTask();
}


#else // for Cortex M


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

	/** @req SWS_Os_00239 */
	Irq_Disable();
	if( Os_SysIntAnyDisabled() ) {
		Os_SysIntClearAll();
	}

	/** @req SWS_Os_00070 */
	if( Os_TaskOccupiesResources(pcb) ) {
		Os_TaskResourceFreeAll(pcb);
	}

	/** @req SWS_Os_00069 */
	Os_CallErrorHook(E_OS_MISSINGEND);

	/** @req SWS_Os_00052 */
	(void)TerminateTask();

}

/**
 * Start an basic task.
 * Starting Basic & Extended task is same for Autosar (not like OSEK)
 * Hence calling Os_TaskStartExtended() inside Os_TaskStartBasic to keep
 * backward compatibility
 */

void Os_TaskStartBasic( void ) {
	Os_TaskStartExtended();
}

#endif

/** @req SWS_Os_00067 */
void Os_StackSetup( OsTaskVarType *pcbPtr ) {
    uint8 *bottom;

    /* Find bottom of the stack so that we can place the
     * context there.
     *
     * stack bottom = high address. stack top = low address
     */
    /*lint -e{9016} MISRA:FALSE_POSITIVE:Allow calculation of address using pointers arithmetic:[MISRA 2012 Rule 18.4, advisory] */
    bottom = (uint8 *)pcbPtr->stack.top + pcbPtr->stack.size;

    /*lint -e{923}  MISRA:FALSE_POSITIVE:Pointer is converted to integer to check the alignment:[MISRA 2012 Rule 11.6, required] */
    ASSERT( ((uint32)bottom & 0x7UL) == 0 );

    pcbPtr->stack.curr = bottom;
    Os_StackSetStartmark(pcbPtr);

    // Make some space for back-chain.
    bottom -= ARCH_BACKCHAIN_SIZE;
    // Set the current stack so that it points to the context
    pcbPtr->stack.curr = bottom - Os_ArchGetScSize();

    Os_StackSetEndmark(pcbPtr);
}

/**
 * Setup the context for a pcb. The context differs for different arch's
 * so we call the arch dependent functions also.
 * The context at setup is always a small context.
 *
 * @param pcb Ptr to the pcb to setup context for.
 */
void Os_TaskContextInit( OsTaskVarType *pcb ) {

    if( pcb->constPtr->autostart == TRUE) {
		pcb->activations++;
        Os_TaskMakeReady(pcb);
    } else {
        pcb->state = ST_SUSPENDED;
    }

    Os_StackSetup(pcb);
    Os_StackFill(pcb);

    Os_ArchSetupContext(pcb);
}


/**
 * Find the top priority task. Even the running task is included.
 * @return
 */

OsTaskVarType *Os_TaskGetTop( void ){
    OsTaskVarType *i_pcb;
    OsTaskVarType *top_prio_pcb = NULL_PTR;
    OsPriorityType top_prio = 0;
    boolean found = FALSE;

    OS_DEBUG(D_TASK,"os_find_top_prio_proc\n");
    /*lint -e{9036} MISRA:EXTERNAL_FILE::[MISRA 2012 Rule 14.4, required] */
    TAILQ_FOREACH(i_pcb,& OS_SYS_PTR->ready_head,ready_list) {
        // all ready task on the current core are candidates
        if (Os_OnRunningCore(OBJECT_TASK,i_pcb->constPtr->pid)) {

        	ASSERT((i_pcb->state & (ST_READY | ST_RUNNING)) != 0u);
        	if( found == TRUE) {
        		if( i_pcb->activePriority > top_prio ) {
        			top_prio = i_pcb->activePriority;
        			top_prio_pcb = i_pcb;
        		}
        	} else {
        		found = TRUE;

        		top_prio = i_pcb->activePriority;
        		top_prio_pcb = i_pcb;
        	}
        }
    }

    ASSERT(top_prio_pcb!=NULL_PTR);

    OS_DEBUG(D_TASK,"Found %s\n",top_prio_pcb->constPtr->name);

    return top_prio_pcb;
}

#if defined(CFG_ARMV7_AR) || defined(CFG_ARM_CR4) || defined(CFG_RH850) || defined(CFG_TMS570LC43X) || defined(CFG_TC2XX)
void Os_TaskSetupAndSwap( void ) {
    OsTaskVarType *pcbPtr = Os_SysTaskGetCurr();

    Os_ArchSetupContext(pcbPtr);
    Os_ArchSwapContextTo(NULL_PTR, pcbPtr);
}
#endif

#ifdef CFG_T1_ENABLE
void Os_SwapPreHandler(OpType op,OsTaskVarType *old_pcb, OsTaskVarType *new_pcb){
    switch (op) {
        case OP_SET_EVENT:
            break;
        case OP_WAIT_EVENT:
            Os_WaitEventHook(old_pcb->constPtr->pid);
            break;
#if defined(CFG_KERNEL_EXTRA)
        case OP_WAIT_SEMAPHORE:
            Os_WaitEventHook(old_pcb->constPtr->pid);
            break;
        case OP_SLEEP:
            Os_WaitEventHook(old_pcb->constPtr->pid);
            break;
#endif
        default:
            /* @CODECOV:DEFAULT_CASE:Default statement is required for defensive programming. */
            ASSERT(0);
            break;
    }

}
void Os_SwapPostHandler(OpType op,OsTaskVarType *old_pcb, OsTaskVarType *new_pcb){
    switch (op) {
        case OP_WAIT_EVENT:
            Os_ResumeEventHook(old_pcb->constPtr->pid);
            break;
#if defined(CFG_KERNEL_EXTRA)
        case OP_WAIT_SEMAPHORE:
            Os_ResumeEventHook(old_pcb->constPtr->pid);
            break;
        case OP_SLEEP:
            Os_ResumeEventHook(old_pcb->constPtr->pid);
            break;
#endif
        default:
            break;
    }
}
#endif



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

#if defined(CFG_KERNEL_EXTRA)


/**
 * Insert a task into the timer queue sorted with shortest timeout first.
 *
 * @param tPtr Task to insert.
 */
void Os_TimerQInsertSorted(OsTaskVarType *tPtr) {
    OsTaskVarType *i_pcb;
    uint8 insertLast = 1u;

    ASSERT(tPtr!=NULL_PTR);

    if ( TAILQ_EMPTY(& OS_SYS_PTR->timerHead) ) {
        ASSERT( TAILQ_FIRST(& OS_SYS_PTR->timerHead) != tPtr );
        TAILQ_INSERT_HEAD(& OS_SYS_PTR->timerHead,tPtr, timerEntry);
    } else {
        TAILQ_FOREACH(i_pcb,& OS_SYS_PTR->timerHead,timerEntry) {

            ASSERT(i_pcb != tPtr );

            if ( (tPtr->tmo) < (i_pcb->tmo) ) {
                TAILQ_INSERT_BEFORE(i_pcb, tPtr, timerEntry);
                insertLast = 0u;
                break;
            }
        }
        if( insertLast == 1u) {
            ASSERT(i_pcb != tPtr );
            TAILQ_INSERT_TAIL(& OS_SYS_PTR->timerHead,tPtr, timerEntry);
        }
    }
}

boolean Os_TimerQIsPresent(OsTaskVarType *tPtr) {
    boolean rv = FALSE;
    OsTaskVarType *i_pcb;

    ASSERT(tPtr!=NULL_PTR);

    TAILQ_FOREACH(i_pcb,& OS_SYS_PTR->timerHead,timerEntry) {
        if( i_pcb == tPtr ) {
            rv = TRUE;
            break;
        }
    }
    return rv;
}


/**
 * returns true if list is empty
 * @return
 */
boolean Os_TimerQIsEmpty( void ) {
    return TAILQ_EMPTY(&OS_SYS_PTR->timerHead);
}

/**
 * returns first task in timer queue (it does NOT remove it)
 * @return
 */
OsTaskVarType * Os_TimerQFirst( void ) {
    return TAILQ_FIRST(&OS_SYS_PTR->timerHead);
}

/**
 * Remove task from timer queue.
 * @param tPtr
 */
void Os_TimerQRemove(OsTaskVarType *tPtr) {

    tPtr->tmo = TMO_INFINITE;
    TAILQ_REMOVE(&OS_SYS_PTR->timerHead, tPtr, timerEntry);
}



StatusType Os_DispatchToSleepWithTmo( OpType op, TickType tmo) {

    StatusType rv = E_OK;
    OsTaskVarType *currPcbPtr  = Os_SysTaskGetCurr();

    ASSERT(tmo != 0);
    /* Check if there is a dispatch lock */
    if ( Os_SchedulerResourceIsFree() ) {
        TickType now;

        currPcbPtr->tmoVal = tmo;
        if( tmo != TMO_INFINITE) {
            now = GetOsTick();
            currPcbPtr->tmo = now + tmo;
            Os_TimerQInsertSorted(currPcbPtr );
        }

        currPcbPtr->rv = E_OK;
        Os_Dispatch(op);
        rv = currPcbPtr->rv;

        /*
         * rv = E_OK - Released from SignalSem/xx
         * rv = E_OS_TIMEOUT - Released from OsTick
         *
         * Cases:
         *  rv               tmo           Action
         *  ------------------------------------
         *  E_OK             0             None
         *  E_OS_TIMEOUT     0             None
         *  E_OK             X             Add to queue
         *  E_OS_TIMEOUT     X             Add to queue
         *  E_OK             INF           None
         *  E_OS_TIMEOUT     INF           None
         */

    } else {
        /* Dispatched locked, can't go to sleep */
        rv = E_OS_RESOURCE;
    }

    return rv;
}
#endif


/*
 * Function: Os_TaskStartFromBeginning
 * This function does the context setup and makes a fresh start of the task
 */
void Os_TaskStartFromBeginning( OsTaskVarType *pcbPtr )
{

#if defined(CFG_PPC)
        Os_TaskMakeRunning(pcbPtr);
        Os_SysTaskSetCurr(pcbPtr);
        Os_CallPreTaskHook();
#ifdef CFG_T1_ENABLE
        Os_StartTaskHook(pcbPtr->constPtr->pid);
#endif // CFG_T1_ENABLE
        /* Adjust stack pointer beyond the context area */
        Os_StackSetup(pcbPtr);
        Os_ArchSetStackPointer(pcbPtr->stack.curr);
        Os_ArchSetupContext(pcbPtr);
        Os_ArchSwapContextTo(NULL_PTR, pcbPtr);
#elif defined(CFG_ARMV7_AR) || defined(CFG_ARM_CR4) || defined(CFG_RH850) || defined(CFG_TMS570LC43X) || defined(CFG_TC2XX)
        Os_TaskMakeRunning(pcbPtr);
        Os_SysTaskSetCurr(pcbPtr);
        Os_CallPreTaskHook();
#ifdef CFG_T1_ENABLE
        Os_StartTaskHook(pcbPtr->constPtr->pid);
#endif // CFG_T1_ENABLE
        /* Adjust stack pointer beyond the context area */
        Os_StackSetup(pcbPtr);
        Os_ArchSetSpAndCall(pcbPtr->stack.curr,Os_TaskSetupAndSwap);
#else // ARCH other than PPC, ARMV7_AR, ARMCR4, RH850, TMS570, TC2XX
        /* Setup the stack again, and just call the basic task */
        Os_StackSetup(pcbPtr);
        /* NOTE: release and get the internal resource ? */
        Os_TaskMakeRunning(pcbPtr);
        Os_SysTaskSetCurr(pcbPtr);
        Os_CallPreTaskHook();
#ifdef CFG_T1_ENABLE
        Os_StartTaskHook(pcbPtr->constPtr->pid);
#endif // CFG_T1_ENABLE
        Os_ArchSetSpAndCall(pcbPtr->stack.curr,Os_TaskStartBasic);
        ASSERT(0);
#endif
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
void Os_Dispatch(OpType op) {
    OsTaskVarType *pcbPtr;
    OsTaskVarType *currPcbPtr = Os_SysTaskGetCurr();

    ASSERT(OS_SYS_PTR->intNestCnt == 0);
    ASSERT(Os_SchedulerResourceIsFree());

    /* When calling post hook we must still be in ST_RUNNING */
    ASSERT(currPcbPtr->state & ST_RUNNING); Os_CallPostTaskHook();

    /* Go the correct state for running task */
    switch (op) {
        case OP_SET_EVENT:
        case OP_SCHEDULE:
        case OP_RELEASE_RESOURCE:
            Os_TaskRunningToReady(currPcbPtr);
            break;
        case OP_WAIT_EVENT:
            Os_TaskMakeWaiting(currPcbPtr);
            break;
#if defined(CFG_KERNEL_EXTRA)
            case OP_WAIT_SEMAPHORE:
            Os_TaskMakeWaitingOnSem(currPcbPtr);
            break;
            case OP_SIGNAL_SEMAPHORE:
            Os_TaskRunningToReady(currPcbPtr);
            break;
            case OP_SLEEP:
            Os_TaskMakeSleeping(currPcbPtr);
            break;
#endif
        case OP_ACTIVATE_TASK:
            Os_TaskMakeReady(currPcbPtr);
            break;
        case OP_CHAIN_TASK:
            ASSERT(OS_SYS_PTR->chainedPcbPtr != NULL_PTR)
            ;

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
            if (currPcbPtr != OS_SYS_PTR->chainedPcbPtr) {
                /* #3 and #4 */
                ASSERT(currPcbPtr->activations > 0);

                --currPcbPtr->activations;
                if (currPcbPtr->activations == 0) {
                    Os_TaskMakeSuspended(currPcbPtr);
                }
                else {
                    Os_TaskRunningToReady(currPcbPtr);
                    currPcbPtr->reactivation = REACTIVATE_FROM_CHAINTASK;
                }
                /* Chained task is already in READY */
            }
            break;
        case OP_TERMINATE_TASK:
#ifdef CFG_T1_ENABLE
            Os_StopTaskHook(currPcbPtr->constPtr->pid);
#endif
            /* OSEK TerminateTask
             * In case of tasks with multiple activation requests,
             * terminating the current instance of the task automatically puts the next
             * instance of the same task into the ready state
             */
            ASSERT(currPcbPtr->activations > 0)
            ;
            --currPcbPtr->activations;
            if (currPcbPtr->activations == 0) {
                Os_TaskMakeSuspended(currPcbPtr);
            }

            break;
        __CODE_COVERAGE_IGNORE__
        default:
            /* @CODECOV:DEFAULT_CASE: Default statement is required for defensive programming. */
            ASSERT(0)
            ;
            break;
    }

    pcbPtr = Os_TaskGetTop();

    /* Swap if we found any process or are forced (multiple activations)*/
    if (pcbPtr != currPcbPtr) {
        /*
         * Swap context
         */
        ASSERT(pcbPtr!=NULL_PTR);

        Os_ResourceReleaseInternal();

        // Check for stack faults (under/overflow)
        Os_StackPerformCheck(currPcbPtr);
        OS_DEBUG(D_TASK,"Swapping to: %s\n",pcbPtr->constPtr->name);

#ifdef CFG_T1_ENABLE
        Os_SwapPreHandler(op,currPcbPtr,pcbPtr);
#endif

#if defined(CFG_TC2XX) || defined(CFG_TC3XX)
        if ( (op == OP_TERMINATE_TASK) || (op == OP_CHAIN_TASK) ) {
            if (pcbPtr->reactivation == REACTIVATE_FROM_CHAINTASK) {
                pcbPtr->reactivation = REACTIVATE_NORMAL;
                Os_TaskStartFromBeginning(pcbPtr);
            }
            else {
                Os_TaskSwapContextTo(NULL_PTR,pcbPtr);
            }
        }
        else {
            Os_TaskSwapContext(currPcbPtr,pcbPtr);
        }
#else
        if ((pcbPtr->reactivation == REACTIVATE_FROM_CHAINTASK)) {
            pcbPtr->reactivation = REACTIVATE_NORMAL;
            Os_TaskStartFromBeginning(pcbPtr);
        }
        else {
            /* Adjust stack pointer beyond the context area */
            Os_TaskSwapContext(currPcbPtr, pcbPtr);
        }

#endif

#ifdef CFG_T1_ENABLE
        Os_SwapPostHandler(op,currPcbPtr,pcbPtr);
#endif

    }
    else {
        OS_DEBUG(D_TASK,"Continuing task %s\n",pcbPtr->constPtr->name);
        Os_TaskStartFromBeginning(pcbPtr);
    }
}


/*
 * Functions: Os_TaskSwapContext
 * Description: Function to change the context to new task and
 * saving current task context to resume.
 */

void Os_TaskSwapContext(OsTaskVarType *old_pcb, OsTaskVarType *new_pcb ) {
    Os_SysTaskSetCurr(new_pcb);
#if	(OS_USE_APPLICATIONS == STD_ON)
    OS_SYS_PTR->currApplId = new_pcb->constPtr->applOwnerId;
#endif
    Os_ResourceGetInternal();
    Os_TaskMakeRunning(new_pcb);
    /* NOTE: The pretask hook is not called with the right stack
     * (it's called on the old task stack, not the new ) */
    Os_CallPreTaskHook();
#ifdef CFG_T1_ENABLE
        Os_StartTaskHook(new_pcb->constPtr->pid);
#endif
    Os_ArchSwapContext(old_pcb,new_pcb);
}

/*
 * Functions: Os_TaskSwapContextTo
 * Description: Used to change the context to new task
 * when current running task need not be resumed. E.g. from Terminatetask, Chaintask, ISR.
 */
void Os_TaskSwapContextTo(OsTaskVarType *old_pcb, OsTaskVarType *new_pcb ) {
    Os_SysTaskSetCurr(new_pcb);
#if	(OS_USE_APPLICATIONS == STD_ON)
    OS_SYS_PTR->currApplId = new_pcb->constPtr->applOwnerId;
#endif
    Os_ResourceGetInternal();
    Os_TaskMakeRunning(new_pcb);
    Os_CallPreTaskHook();
#ifdef CFG_T1_ENABLE
    Os_StartTaskHook(new_pcb->constPtr->pid);
#endif
    Os_ArchSwapContextTo(old_pcb,new_pcb);
    ASSERT(0);
}


#if !(defined(CFG_SAFETY_PLATFORM) || defined(BUILD_OS_SAFETY_PLATFORM))
void Os_Arc_GetStackInfo( TaskType task, StackInfoType *s) {
    OsTaskVarType *pcb 	= Os_TaskGet(task);
    if (s != NULL_PTR) {
		s->curr 	= Os_ArchGetStackPtr();
		s->top 		= pcb->stack.top;
		s->at_swap 	= pcb->stack.curr;
		s->size 	= pcb->stack.size;
		s->usage 	= (void *)Os_StackGetUsage(pcb);

	    /*lint -e{923} MISRA:FALSE_POSITIVE:Pointer shall not be converted to integer vause because of undefined alignement issue, here the pointer is converted to integer to check the alignment:[MISRA 2012 Rule 11.6, required] */
		/*lint -e{734} MISRA:FALSE_POSITIVE:No precision loss due to this typecasting:[MISRA 2004 Info, advisory] */
		s->usageInPercent = ((s->size - (uint32)((size_t)s->usage - (size_t)s->top))*100)/s->size;
    }
}
#endif

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
    const OsTaskVarType *pcb;

    /* Validation of parameters, if failure, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_2( (State != NULL_PTR), E_OS_PARAM_POINTER ,
    		           OSServiceId_GetTaskState,TaskId, State);   /* @req SWS_Os_00566 */
#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
    OS_VALIDATE_STD_2( (OS_VALIDATE_ADDRESS_RANGE(State,sizeof(TaskStateType)) == TRUE ) , E_OS_ILLEGAL_ADDRESS ,
    		           OSServiceId_GetTaskState,TaskId, State);/*@req SWS_Os_00051 */
#endif

    OS_VALIDATE_STD_2( TASK_CHECK_ID(TaskId) , E_OS_ID,
    		           OSServiceId_GetTaskState,TaskId, State); /* @req OSEK_SWS_TM_00030 */
    pcb = Os_TaskGet(TaskId);

#if	(OS_APPLICATION_CNT > 1) && ( OS_NUM_CORES > 1)
    if (Os_ApplGetCore(pcb->constPtr->applOwnerId) != GetCoreID()) {
        StatusType status = Os_NotifyCore(Os_ApplGetCore(pcb->constPtr->applOwnerId),
                                          OSServiceId_GetTaskState,
                                          TaskId,
                                          (uint32)State,
                                          (uint32)0);
        return status;
    }
#endif

    curr_state = os_pcb_get_state(pcb);

    switch(curr_state) {
    case ST_RUNNING:
        *State = TASK_STATE_RUNNING;
        break;
    case ST_WAIT_EVENT:
        *State = TASK_STATE_WAITING;
        break;
#if defined(CFG_KERNEL_EXTRA)
    case ST_WAIT_SEM:
        *State = TASK_STATE_WAITING_SEM;
        break;
#endif
    case ST_SUSPENDED:
        *State = TASK_STATE_SUSPENDED;
        break;
    case ST_READY:
        *State = TASK_STATE_READY;
        break;
    __CODE_COVERAGE_IGNORE__
    default:
        /** @CODECOV:DEFAULT_CASE:Default statement is required for defensive programming. */
    	ASSERT(0);
    	break;
    }

    return rv;
}


/**
 * GetTaskID returns the information about the TaskID of the task
 * which is currently running.
 *
 * @param task_id Reference to the task which is currently running
 * @return
 */
/* @req OSEK_SWS_TM_00015 */
StatusType GetTaskID(TaskRefType TaskID) {
    StatusType rv = E_OK;

    /* Validation of parameters, if failure, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_1((TaskID != NULL_PTR), E_OS_PARAM_POINTER, OSServiceId_GetTaskID, TaskID); /* @req SWS_Os_00566 */
#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
    OS_VALIDATE_STD_1( (OS_VALIDATE_ADDRESS_RANGE(TaskID, sizeof(TaskType)) == TRUE) , E_OS_ILLEGAL_ADDRESS ,
            OSServiceId_GetTaskID,TaskID);/*@req SWS_Os_00051 */
#endif

    *TaskID = INVALID_TASK;

    if ( OS_SYS_PTR->intNestCnt == 0) {
        /* A task is running */
        *TaskID = OS_SYS_PTR->currTaskPtr->constPtr->pid; /* @req OSEK_SWS_TM_00016 */
    }

    return rv;
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
    imask_t flags;
    const OsTaskVarType *currPcbPtr = Os_SysTaskGetCurr();
    OsTaskVarType *destPcbPtr;
    StatusType rv = E_OK;

    OS_DEBUG(D_TASK,"# ActivateTask %s\n",currPcbPtr->constPtr->name);

    OS_VALIDATE_STD_1( TASK_CHECK_ID(TaskID) , E_OS_ID,
    		           OSServiceId_ActivateTask,TaskID); /* @req OSEK_SWS_TM_00001 */

    destPcbPtr = Os_TaskGet(TaskID);

#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
	/* @req SWS_Os_00448
	 * The Operating System module shall prevent access of OS-Applications, trusted or non-trusted,
	 * to objects not belonging to this OS-Application,
	 * except access rights for such objects are explicitly granted by configuration.
	 * NOTE: As this requirement is generic to OS applications, requirement tagging done here alone. */

    if( destPcbPtr->constPtr->applOwnerId != currPcbPtr->constPtr->applOwnerId ) {
    	ApplicationType appId;
    	/* @req SWS_Os_00509
    	 * If a service call is made on an Operating System object that is owned by another OS-Application
    	 * without state APPLICATION_ACCESSIBLE, then the Operating System module shall return E_OS_ACCESS.
    	 * NOTE: As this requirement is generic to OS applications, requirement tagging done here alone. */
        OS_VALIDATE_STD_1( (Os_ApplCheckState(destPcbPtr->constPtr->applOwnerId) != E_OS_ACCESS) , E_OS_ACCESS,
        		           OSServiceId_ActivateTask,TaskID);
        /* Do we have access to the task we are activating */
        appId = Os_GetCurrTaskISROwnerId();
        OS_VALIDATE_STD_1( (Os_ApplCheckAccess(appId , destPcbPtr->constPtr->accessingApplMask) != E_OS_ACCESS) , E_OS_ACCESS,
        		           OSServiceId_ActivateTask,TaskID);

#if (OS_NUM_CORES > 1)

        if (Os_ApplGetCore(destPcbPtr->constPtr->applOwnerId) != GetCoreID()) {
            StatusType status = Os_NotifyCore(Os_ApplGetCore(destPcbPtr->constPtr->applOwnerId),
                                              OSServiceId_ActivateTask,
                                              TaskID,
                                              (uint32)NULL,
                                              (uint32)NULL);
            return status;
        }
#endif
    }
#endif

    /* @req SWS_Os_00093 ActivateTask */
    OS_VALIDATE_STD_1( (Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT,
    		           OSServiceId_ActivateTask,TaskID);

    Irq_Save(flags);

#ifdef CFG_T1_ENABLE
    Os_ActivateTaskHook(destPcbPtr->constPtr->pid);
#endif

    destPcbPtr->activations++;

    if( os_pcb_get_state(destPcbPtr) != ST_SUSPENDED ) {
        /** Too many task activations */
        if( destPcbPtr->activations > destPcbPtr->constPtr->activationLimit ) {
            /* Standard */
            rv=E_OS_LIMIT; /* OSEK Standard Status */ /* @req OSEK_SWS_TM_00002 */
            Irq_Restore(flags);
            --destPcbPtr->activations;
            /* OS_STD_ERR_1: Function will return after calling ErrorHook */
            /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
            OS_STD_ERR_1(OSServiceId_ActivateTask,TaskID);
        }
    } else {
        /* We have a suspended task, make it ready for use */
        /* @req OSEK_SWS_TM_00003 */
        ASSERT( destPcbPtr->activations == 1 );
        Os_Arc_SetCleanContext(destPcbPtr);
        Os_TaskMakeReady(destPcbPtr);
    }

    /* Preempt only if we are preemptable and target has higher prio than us */
    /*lint -e{9007} MISRA:FALSE_POSITIVE:No side effects of Os_SchedulerResourceIsFree:[MISRA 2012 Rule 13.5, required]*/
    if(	(OS_SYS_PTR->intNestCnt == 0) &&
        (currPcbPtr->constPtr->scheduling == FULL) &&
        (destPcbPtr->activePriority > Os_SysTaskGetCurr()->activePriority)) {
        Os_Dispatch(OP_ACTIVATE_TASK);
    }

    Irq_Restore(flags);

    return rv;
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
    StatusType rv = E_OK;
    OsTaskVarType *currPcbPtr = Os_SysTaskGetCurr();

    /* Validation of parameters, if failure, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD( (Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT , OSServiceId_TerminateTask);   /* @req SWS_Os_00093 */
    OS_DEBUG(D_TASK,"# TerminateTask %s\n",OS_SYS_PTR->currTaskPtr->constPtr->name);
    OS_VALIDATE_STD( OS_SYS_PTR->intNestCnt == 0, E_OS_CALLEVEL, OSServiceId_TerminateTask); /* @req OSEK_SWS_TM_00005 */
    OS_VALIDATE_STD( !Os_TaskOccupiesResources(currPcbPtr), E_OS_RESOURCE , OSServiceId_TerminateTask); /* @req OSEK_SWS_TM_00004 */
#if (OS_NUM_CORES > 1)
    OS_VALIDATE_STD( !Os_TaskOccupiesSpinlocks(currPcbPtr), E_OS_SPINLOCK , OSServiceId_TerminateTask);    /* SWS_Os_00612 */
#endif

    Irq_Disable();

    /* Force the dispatcher to find something, even if its us */
    Os_Dispatch(OP_TERMINATE_TASK); /* @req OSEK_SWS_TM_00006 */

    ASSERT(0);

    return rv;
}

StatusType ChainTask( TaskType TaskId ) {
    imask_t flags;
    StatusType rv = E_OK;
    OsTaskVarType *currPcbPtr = Os_SysTaskGetCurr();
    OsTaskVarType *destPcbPtr;

    /* Validation of parameters, if failure, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_1((Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT,
    		          OSServiceId_ChainTask,TaskId);   /* @req SWS_Os_00093 */

    OS_VALIDATE_STD_1(TASK_CHECK_ID(TaskId) , E_OS_ID,
        		      OSServiceId_ChainTask,TaskId);   /* @req SWS_Os_00093 */ /* @req OSEK_SWS_TM_00007*/

    destPcbPtr = Os_TaskGet(TaskId);

    OS_DEBUG(D_TASK,"# ChainTask %s\n",destPcbPtr->constPtr->name);

    /* Validation of parameters, if failure, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_1( OS_SYS_PTR->intNestCnt == 0, E_OS_CALLEVEL ,
    		           OSServiceId_ChainTask,TaskId); /* @req OSEK_SWS_TM_00010 */
    OS_VALIDATE_STD_1( !Os_TaskOccupiesResources(currPcbPtr), E_OS_RESOURCE ,
    		           OSServiceId_ChainTask,TaskId); /* @req OSEK_SWS_TM_00009 */
#if (OS_NUM_CORES > 1)
    OS_VALIDATE_STD_1( !Os_TaskOccupiesSpinlocks(currPcbPtr), E_OS_SPINLOCK ,
    		           OSServiceId_ChainTask,TaskId);    /* SWS_Os_00612 */
#endif

    Irq_Save(flags);

    if (currPcbPtr != destPcbPtr) {
    	/** Too many task activations */
        if( (destPcbPtr->activations + 1) >  destPcbPtr->constPtr->activationLimit ) {
            rv = E_OS_LIMIT;    /* OSEK Standard Status */ /* @req OSEK_SWS_TM_00008*/
            Irq_Restore(flags);
            /* OS_STD_ERR_1: Function will return after calling ErrorHook */
            /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
            OS_STD_ERR_1(OSServiceId_ChainTask,TaskId);
        }

        if( os_pcb_get_state(destPcbPtr) == ST_SUSPENDED ) {
            ASSERT( destPcbPtr->activations == 0 );
            Os_Arc_SetCleanContext(destPcbPtr);
            Os_TaskMakeReady(destPcbPtr);
        }
        destPcbPtr->activations++;
    }

    OS_SYS_PTR->chainedPcbPtr = destPcbPtr;


#if	(OS_APPLICATION_CNT > 1) && (OS_NUM_CORES > 1)
    if (Os_ApplGetCore(destPcbPtr->constPtr->applOwnerId) != GetCoreID()) {
        StatusType status = Os_NotifyCore(Os_ApplGetCore(destPcbPtr->constPtr->applOwnerId),
                                          OSServiceId_ChainTask,
                                          TaskId,
                                          0,
                                          0);
        OS_SYS_PTR->chainedPcbPtr = destPcbPtr;
        (void)status;
    }
#endif

    Os_Dispatch(OP_CHAIN_TASK); /* @req OSEK_SWS_TM_00011 */

    ASSERT( 0 );

    return rv;

}

/**
 * If a higher-priority task is ready, the internal resource of the task
 * is released, the current task is put into the  ready state, its
 * context is saved and the higher-priority task is executed.
 * Otherwise the calling task is continued.
 *
 * NOTE: The OSEK spec says a lot of strange things under "particulareties"
 * that I don't understand
 *
 * See OSEK/VDX 13.2.3.4
 *
 */
StatusType Schedule( void ) {
    StatusType rv = E_OK;
    imask_t flags;
    OsTaskVarType *currPcbPtr = Os_SysTaskGetCurr();


    OS_DEBUG(D_TASK,"# Schedule %s\n",currPcbPtr->constPtr->name);

    /* Validation of parameters, if failure, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD( (Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT , OSServiceId_Schedule);   /* @req SWS_Os_00093 */
    OS_VALIDATE_STD( OS_SYS_PTR->intNestCnt == 0, E_OS_CALLEVEL, OSServiceId_Schedule);              /* @req SWS_Os_00088  */ /* @req OSEK_SWS_TM_00013 */
    OS_VALIDATE_STD( !Os_TaskOccupiesResources(currPcbPtr), E_OS_RESOURCE , OSServiceId_Schedule);    /* OSEK */ /* @req OSEK_SWS_TM_00012 */
#if (OS_NUM_CORES > 1)
    OS_VALIDATE_STD( !Os_TaskOccupiesSpinlocks(currPcbPtr), E_OS_SPINLOCK , OSServiceId_Schedule);    /* SWS_Os_00624 */
#endif


    ASSERT( Os_SysTaskGetCurr()->state & ST_RUNNING);

    /* We need to figure out if we have an internal resource,
     * otherwise no re-scheduling.
     * NON  - Have internal resource prio OS_RES_SCHEDULER_PRIO (32+)
     * FULL - Assigned internal resource OR
     *        No assigned internal resource.
     * */
    if( Os_SysTaskGetCurr()->constPtr->scheduling == NON ) {
        Irq_Save(flags);
        const OsTaskVarType* top_pcb = Os_TaskGetTop();
        /* only dispatch if some other ready task has higher prio */
        if (top_pcb->activePriority > Os_SysTaskGetCurr()->activePriority) {
            Os_Dispatch(OP_SCHEDULE); /* @req OSEK_SWS_TM_00014 */
        }

        Irq_Restore(flags);
    }

    return rv;
}

#if !(defined(CFG_SAFETY_PLATFORM) || defined(BUILD_OS_SAFETY_PLATFORM))
void Os_Arc_GetTaskInfo( Arc_PcbType *pcbPtr, TaskType taskId, uint32 flags ) {
    const OsTaskVarType *tP = Os_TaskGet(taskId);

    ASSERT(pcbPtr!=NULL_PTR);

	if( (flags & OS_ARC_F_TASK_BASIC) != 0u) {
		strncpy(pcbPtr->name,tP->constPtr->name,OS_ARC_PCB_NAME_SIZE);
		pcbPtr->tasktype = (tP->constPtr->proc_type == (proc_type_t)PROC_EXTENDED ) ? (uint32)ARC_TASKTYPE_EXENDED : (uint32)ARC_TASKTYPE_BASIC;
	}

	if( (flags & OS_ARC_F_TASK_STACK) != 0u ) {
		Os_Arc_GetStackInfo(taskId, &pcbPtr->stack );
	}
}
#endif

void Os_StackPerformCheck( OsTaskVarType *pcbPtr ) {
#if (OS_STACK_MONITORING == 1)
    if((FALSE == Os_StackIsEndmarkOk(pcbPtr)) || (FALSE == Os_StackIsStartmarkOk(pcbPtr)) ) /*lint !e9007, OK side effect tested */
    {
        /** @req SWS_Os_00396
         * If a stack fault is detected by stack monitoring AND the configured scalability
         * class is 3 or 4, the Operating System module shall call the ProtectionHook() with
         * the status E_OS_STACKFAULT.
         * */
        Os_CallProtectionHook(E_OS_STACKFAULT);
    }
#endif
}




