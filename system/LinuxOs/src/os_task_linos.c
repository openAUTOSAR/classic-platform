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
#ifndef _WIN32
/* Not POSIX, but needed for debugging using pthread_setname_np */
#define __USE_GNU
#include <pthread.h>
#include <setjmp.h>
#include <unistd.h> /* sleep */
#include <stdlib.h> /* exit */
#include <ctype.h>  /* isprint */
#include <errno.h>
#include <signal.h>
#include <fcntl.h>  /* open */
#endif
#include <stdio.h>

#include "linos_logger.h" /* Logger functions */
#ifndef _WIN32
#include <asm/param.h> /* HZ */
#endif
#include <time.h>

#include "EcuM.h"

#include <os_i.h>       /* GEN_TASK_HEAD  */
#include <os_sys.h>  /* OS_SYS_PTR */
#include "os_main.h"


extern ThreadTaskType ThreadTasks[(OS_TASK_CNT+GNULINUX_TASK_CNT)]; /* Normal tasks and special GNULinux tasks */

void Os_ResourceReleaseInternal( void ) {
}

void Os_ResourceGetInternal( void ) {
}

/*        */

int GetTaskId (void) {

    int TId = -1; // If NOT found this will be returned
    int i;
#ifndef _WIN32
    pthread_t id = pthread_self();

    for (i=0; i< OS_TASK_CNT;i++){
        if(pthread_equal(id,(ThreadTasks[i].tid))){
            TId = i;
            i = OS_TASK_CNT; // Exit for loop
        }
    }
#endif
    return TId;
}


/* Stuff here is taken from core/system/Os/os_task.c */


/* ----------------------------[includes]------------------------------------*/

#include "os_i.h"


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
static inline void Os_TaskRunningToReady( OsTaskVarType *currPcbPtr ) {
    ASSERT(currPcbPtr->state == ST_RUNNING );
    currPcbPtr->state = ST_READY;
}


/**
 * Make a task go the READY state
 * Used by API: ActivateTask(), SetEvent()
 *
 * @param currPcbPtr
 */
void Os_TaskMakeReady( OsTaskVarType *pcb ) {
    if( ( pcb->state & ( ST_READY | ST_RUNNING ) ) == 0 ) {
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
    ASSERT( pcb->state & (ST_READY|ST_RUNNING) );

    pcb->state = ST_WAIT_EVENT;
    TAILQ_REMOVE(&OS_SYS_PTR->ready_head,pcb,ready_list);
    OS_DEBUG(D_TASK,"Removed %s from ready list\n",pcb->constPtr->name);
}

// Sleeping
#if defined(CFG_KERNEL_EXTRA)
static inline void Os_TaskMakeSleeping( OsTaskVarType *pcb )
{
    ASSERT( pcb->state & (ST_READY|ST_RUNNING) );

    pcb->state = ST_WAITING | ST_SLEEPING;
    TAILQ_REMOVE(&OS_SYS_PTR->ready_head,pcb,ready_list);
    OS_DEBUG(D_TASK,"Removed %s from ready list\n",pcb->constPtr->name);
}


static inline void Os_TaskMakeWaitingOnSem( OsTaskVarType *pcb )
{
    ASSERT( pcb->state & (ST_READY|ST_RUNNING) );

    pcb->state = ST_WAITING_SEM;
    TAILQ_REMOVE(&OS_SYS_PTR->ready_head,pcb,ready_list);
    OS_DEBUG(D_TASK,"Removed %s from ready list\n",pcb->constPtr->name);
}


#endif

// Terminate task
static inline void Os_TaskMakeSuspended( OsTaskVarType *pcb )
    {
    ASSERT( pcb->state & (ST_READY|ST_RUNNING) );
    pcb->state = ST_SUSPENDED;
    TAILQ_REMOVE(&OS_SYS_PTR->ready_head,pcb,ready_list);
    OS_DEBUG(D_TASK,"Removed %s from ready list\n",pcb->constPtr->name);
}


/* ----------------------------[public functions]----------------------------*/


#if defined(CFG_PPC) || defined(CFG_ARMV7_AR) || defined (CFG_ARM_CR4) || defined(CFG_RH850) || defined(CFG_TMS570LC43X)
/**
 * We come here after a task have returned.
 *
 */

void Os_TaskPost( void ) {
    OsTaskVarType *currPcbPtr= OS_SYS_PTR->currTaskPtr;

    /** @req OS239 */
    Irq_Disable();
    if( Os_SysIntAnyDisabled() ) {
        Os_SysIntClearAll();
    }

    /** @req OS070 */
    if( Os_TaskOccupiesResources(currPcbPtr) ) {
        Os_TaskResourceFreeAll(currPcbPtr);
    }

    /** @req OS069 */
    Os_CallErrorHook(E_OS_MISSINGEND);

    /** @req OS052 */
    (void)TerminateTask();
}


#else
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

//    PRETASKHOOK();

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
    Os_CallErrorHook(E_OS_MISSINGEND);

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

//    PRETASKHOOK();

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
    Os_CallErrorHook(E_OS_MISSINGEND);

    /** @req OS052 */
    TerminateTask();
}

#endif

#if !defined(CFG_GNULINUX)
void Os_StackSetup( OsTaskVarType *pcbPtr ) {
    uint8_t *bottom;

    /* Find bottom of the stack so that we can place the
     * context there.
     *
     * stack bottom = high address. stack top = low address
     */
    //lint -e{9016} MISRA 2012 18.4 (adv)   Allow calculation of address using pointers arithmetic
    bottom = (uint8_t *)pcbPtr->stack.top + pcbPtr->stack.size;

    ASSERT( ((uint32_t)bottom & 0x7UL) == 0 );

    pcbPtr->stack.curr = bottom;
    // IMPROVEMENT: alignments here..
    // IMPROVEMENT :use function os_arch_get_call_size() ??

    // Make some space for back-chain.
    bottom -= 16;
    // Set the current stack so that it points to the context
    pcbPtr->stack.curr = bottom - Os_ArchGetScSize();

    Os_StackSetEndmark(pcbPtr);
}
#endif /* !defined(CFG_GNULINUX) */

/**
 * Fill the stack with a predefined pattern
 *
 * @param pcbPtr Pointer to the pcb to fill with pattern
 */
#if !defined(CFG_GNULINUX)
static void Os_StackFill(OsTaskVarType *pcbPtr) {
    uint8_t *p = pcbPtr->stack.curr;

    ASSERT(pcbPtr->stack.curr > pcbPtr->stack.top);

    //lint -e{946, 947} MISRA  False positive. Pointers belong to the same "array"
    memset(pcbPtr->stack.top,STACK_PATTERN, (size_t)(p - (uint8_t *)pcbPtr->stack.top) );
}
#endif /* !defined(CFG_GNULINUX) */

/**
 * Setup the context for a pcb. The context differs for different arch's
 * so we call the arch dependent functions also.
 * The context at setup is always a small context.
 *
 * @param pcb Ptr to the pcb to setup context for.
 */
#if !defined(CFG_GNULINUX)
void Os_TaskContextInit( OsTaskVarType *pcb ) {

    if( pcb->constPtr->autostart ) {
        Os_TaskMakeReady(pcb);
    } else {
        pcb->state = ST_SUSPENDED;
    }

    Os_StackSetup(pcb);
    Os_StackFill(pcb);

    Os_ArchSetupContext(pcb);
}
#endif /* !defined(CFG_GNULINUX) */

/**
 * Find the top priority task. Even the running task is included.
 * NOTE: There should be a priority queue (using a heap?) here instead.
 *        giving O(log n) for instertions and (1) for getting the top
 *        prio task. The curerent implementation is ehhhh bad.
 * @return
 */

OsTaskVarType *Os_TaskGetTop( void ){
    OsTaskVarType *i_pcb;
    OsTaskVarType *top_prio_pcb = NULL;
    OsPriorityType top_prio = 0;
    boolean found = FALSE;


//    OS_DEBUG(D_TASK,"os_find_top_prio_proc\n");

    TAILQ_FOREACH(i_pcb,& OS_SYS_PTR->ready_head,ready_list) {
        // all ready task on the current core are candidates
        if (Os_OnRunningCore(OBJECT_TASK,i_pcb->constPtr->pid)) {
            if( i_pcb->state & (ST_READY | ST_RUNNING)) {
                if( found ) {
                    if( i_pcb->activePriority > top_prio ) {
                        top_prio = i_pcb->activePriority;
                        top_prio_pcb = i_pcb;
                    }
                } else {
                    found = TRUE;
                    top_prio = i_pcb->activePriority;
                    top_prio_pcb = i_pcb;
                }
            } else {
                ASSERT(0);
            }
        }
    }

    ASSERT(top_prio_pcb!=NULL);

    OS_DEBUG(D_TASK,"Found %s\n",top_prio_pcb->constPtr->name);

    return top_prio_pcb;
}

void Os_TaskSetupAndSwap( void ) {
    OsTaskVarType *pcbPtr = Os_SysTaskGetCurr();

    Os_ArchSetupContext(pcbPtr);
    Os_ArchSwapContextTo(NULL, pcbPtr);
}


//#define USE_LDEBUG_PRINTF
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
#if !defined(CFG_GNULINUX)
void Os_Dispatch( OpType op ) {
    OsTaskVarType *pcbPtr;
    OsTaskVarType *currPcbPtr = Os_SysTaskGetCurr();

    ASSERT(OS_SYS_PTR->intNestCnt == 0);
    ASSERT(Os_SchedulerResourceIsFree());

    /* When calling post hook we must still be in ST_RUNNING */
    ASSERT( currPcbPtr->state & ST_RUNNING );
    POSTTASKHOOK();

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
            ASSERT( OS_SYS_PTR->chainedPcbPtr != NULL );

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
                if (currPcbPtr->activations > 0) {
                    --currPcbPtr->activations;
                }
                if( currPcbPtr->activations == 0 ) {
                    Os_TaskMakeSuspended(currPcbPtr);
                } else {
                    Os_TaskRunningToReady(currPcbPtr);
                }
                /* Chained task is already in READY */
            }
            OS_SYS_PTR->chainedPcbPtr = NULL;
            break;
        case OP_TERMINATE_TASK:
            /*@req OSEK TerminateTask
             * In case of tasks with multiple activation requests,
             * terminating the current instance of the task automatically puts the next
             * instance of the same task into the ready state
             */
            if (currPcbPtr->activations > 0) {
                --currPcbPtr->activations;
            }
            if( currPcbPtr->activations == 0 ) {
                Os_TaskMakeSuspended(currPcbPtr);
            }
            break;
        default:
            ASSERT(0);
            break;
    }

    pcbPtr = Os_TaskGetTop();

    /* Swap if we found any process or are forced (multiple activations)*/
    if( pcbPtr != currPcbPtr ) {
        if( (OP_CHAIN_TASK == op) && ( currPcbPtr == OS_SYS_PTR->chainedPcbPtr ) ) {
            /* #2 */
            Os_TaskRunningToReady(currPcbPtr);
        }
        /*
         * Swap context
         */
        ASSERT(pcbPtr!=NULL);

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
#if defined(CFG_TC2XX) || defined(CFG_TC3XX)
        if ( (op == OP_TERMINATE_TASK) || (op == OP_CHAIN_TASK) ) {
            Os_TaskSwapContextTo(NULL,pcbPtr);
        } else {
            Os_TaskSwapContext(currPcbPtr,pcbPtr);
        }
#else
        Os_TaskSwapContext(currPcbPtr,pcbPtr);
#endif

    } else {
        OS_DEBUG(D_TASK,"Continuing task %s\n",pcbPtr->constPtr->name);
#if defined(CFG_PPC)
        Os_TaskMakeRunning(pcbPtr);
        PRETASKHOOK();
        /* Adjust stack pointer beyond the context area */
        Os_StackSetup(pcbPtr);
        Os_ArchSetStackPointer(pcbPtr->stack.curr);
        Os_ArchSetupContext(pcbPtr);
        Os_ArchSwapContextTo(NULL, pcbPtr);
#elif defined(CFG_ARMV7_AR) || defined(CFG_ARM_CR4) || defined(CFG_RH850) || defined(CFG_TMS570LC43X)
        Os_TaskMakeRunning(pcbPtr);
        PRETASKHOOK();
        /* Adjust stack pointer beyond the context area */
        Os_StackSetup(pcbPtr);
        Os_ArchSetSpAndCall(pcbPtr->stack.curr,Os_TaskSetupAndSwap);
#else
        /* Setup the stack again, and just call the basic task */
        Os_StackSetup(pcbPtr);
        /* NOTE: release and get the internal resource ? */
        Os_TaskMakeRunning(pcbPtr);
        PRETASKHOOK();
        Os_ArchSetSpAndCall(pcbPtr->stack.curr,Os_TaskStartBasic);
        ASSERT(0);
#endif
    }
}
#endif /* !defined(CFG_GNULINUX) */

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
#if    (OS_USE_APPLICATIONS == STD_ON)
    OS_SYS_PTR->currApplId = new_pcb->constPtr->applOwnerId;
#endif
    Os_ResourceGetInternal();
    Os_TaskMakeRunning(new_pcb);
    /* NOTE: The pretask hook is not called with the right stack
     * (it's called on the old task stack, not the new ) */
    PRETASKHOOK();
    Os_ArchSwapContext(old_pcb,new_pcb);
}

void Os_TaskSwapContextTo(OsTaskVarType *old_pcb, OsTaskVarType *new_pcb ) {
    Os_SysTaskSetCurr(new_pcb);
#if    (OS_USE_APPLICATIONS == STD_ON)
    OS_SYS_PTR->currApplId = new_pcb->constPtr->applOwnerId;
#endif
    Os_ResourceGetInternal();
    Os_TaskMakeRunning(new_pcb);
    PRETASKHOOK();
    Os_ArchSwapContextTo(old_pcb,new_pcb);
    ASSERT(0);
}


void Os_Arc_GetStackInfo( TaskType task, StackInfoType *s) {


    logger(LOG_ERR, "void Os_Arc_GetStackInfo( IMPROVEMENT )");

//    OsTaskVarType *pcb     = Os_TaskGet(task);
//
//    s->curr     = Os_ArchGetStackPtr();
//    s->top         = pcb->stack.top;
//    s->at_swap     = pcb->stack.curr;
//    s->size     = pcb->stack.size;
//    s->usage     = (void *)Os_StackGetUsage(pcb);
}


void Os_Arc_GetTaskName(char *str, TaskType taskId) {
    const char *name = Os_TaskGet(taskId)->constPtr->name;
    strncpy(str, name , strlen(name));
}


/**
 * Returns the state of a task (running, ready, waiting, suspended)
 * at the time of calling GetTaskState.
 *
 * @param TaskId  Task reference
 * @param State   Reference to the state of the task
 */
#if !defined(CFG_GNULINUX)
StatusType GetTaskState(TaskType TaskId, TaskStateRefType State) {
    state_t curr_state;
    StatusType rv = E_OK;
    OsTaskVarType *pcb;

    OS_VALIDATE_STD_2( TASK_CHECK_ID(TaskId) , E_OS_ID, 
                   OSServiceId_GetTaskState,TaskId, State); 
    pcb = Os_TaskGet(TaskId);

#if    (OS_APPLICATION_CNT > 1) && ( OS_NUM_CORES > 1)
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

    // IMPROVEMENT: Lazy impl. for now */
    switch(curr_state) {
    case ST_RUNNING:
        *State = TASK_STATE_RUNNING;
        break;
    case ST_WAITING:
        *State = TASK_STATE_WAITING;
        break;
    case ST_SUSPENDED:
        *State = TASK_STATE_SUSPENDED;
        break;
    case ST_READY:
        *State = TASK_STATE_READY;
        break;
    }

    return rv;
}
#endif /* !defined(CFG_GNULINUX) */

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
    int i;

    i = GetTaskId();

    /* If valid task then point to data */
    if ( i > -1 )
    {
        *TaskID = ThreadTasks[i].pid;
    }

//    /* Test specification say return CALLEVEL if in ISR
//     * but impl. spec says otherwise */
//    if( OS_SYS_PTR->intNestCnt == 0 ) {
//        if( OS_SYS_PTR->currTaskPtr->state & ST_RUNNING ) {
//            *TaskID = OS_SYS_PTR->currTaskPtr->constPtr->pid;
//        } else {
//            /* This is not a real error since this could
//             * be the case when called from ErrorHook */
//        }
//    }

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
/** @req OS511 OS provides the service GetISRID() */
/** @req OS515 GetISRID() available in all Scalability Classes. */
ISRType GetISRID( void ) {

      /** @req OS264 */
      if(OS_SYS_PTR->intNestCnt == 0 ) {
            return INVALID_ISR;
      }

      if( Os_SysIsrGetCurr()->constPtr->type == ISR_TYPE_1 ) {
          return INVALID_ISR;
      }

      /** @req OS263 */
      return (ISRType)Os_SysIsrGetCurr()->id;
}

#if !defined(CFG_GNULINUX)
static inline void Os_Arc_SetCleanContext( OsTaskVarType *pcb ) {
    if (pcb->constPtr->proc_type == PROC_EXTENDED) {
        /** @req OSEK ActivateTask Cleanup events
         * OSEK,ActivateTask, When an extended task is transferred from suspended
         * state into ready state all its events are cleared.*/
        pcb->ev_set = 0;
        pcb->ev_wait = 0;
    }
    Os_StackSetup(pcb);
    Os_ArchSetupContext(pcb);
}
#endif /* !defined(CFG_GNULINUX) */

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
    StatusType rv = E_OK;

#if defined(CFG_GNULINUX)
    OS_DEBUG(D_TASK,"# ActivateTask %d\n", TaskID);

    /* ActivateTask on GNU/Linux is to restart a task that ended/exited. */

    rv = start_thread(TaskID);

    return rv;
#else
    imask_t flags;
    OsTaskVarType *currPcbPtr = Os_SysTaskGetCurr();
    OsTaskVarType *destPcbPtr;

    OS_DEBUG(D_TASK,"# ActivateTask %s\n",currPcbPtr->constPtr->name);
    TASK_CHECK_ID(TaskID);

    destPcbPtr = Os_TaskGet(TaskID);

#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)

    if( destPcbPtr->constPtr->applOwnerId != currPcbPtr->constPtr->applOwnerId ) {
        ApplicationType appId;
        OS_VALIDATE_STD_1( (Os_ApplCheckState(destPcbPtr->constPtr->applOwnerId) != E_OS_ACCESS) , E_OS_ACCESS,
                           OSServiceId_ActivateTask,TaskID);
        /* Do we have access to the task we are activating */
        if(OS_SYS_PTR->intNestCnt == 0 ) {
            appId = currPcbPtr->constPtr->applOwnerId;
        } else {
            appId = Os_SysIsrGetCurr()->constPtr->appOwner;
        }
        OS_VALIDATE_STD_1( (Os_ApplCheckAccess(appId , destPcbPtr->constPtr->accessingApplMask) != E_OS_ACCESS) , E_OS_ACCESS,
                           OSServiceId_ActivateTask,TaskID);

#if (OS_NUM_CORES > 1)

        if (Os_ApplGetCore(destPcbPtr->constPtr->applOwnerId) != GetCoreID()) {
            StatusType status = Os_NotifyCore(Os_ApplGetCore(destPcbPtr->constPtr->applOwnerId),
                                              OSServiceId_ActivateTask,
                                              TaskID,
                                              (uint32_t)NULL,
                                              (uint32_t)NULL);
            return status;
        }
#endif
    }
#endif

    /* @req OS093 ActivateTask */
    if( Os_SysIntAnyDisabled() ) {
        /* Standard */
        rv = E_OS_DISABLEDINT;
        OS_STD_ERR_1(OSServiceId_ActivateTask,TaskID);
    }

    Irq_Save(flags);

    destPcbPtr->activations++;
    if( os_pcb_get_state(destPcbPtr) != ST_SUSPENDED ) {
        /** @req OSEK_? Too many task activations */
        if( destPcbPtr->activations >= (destPcbPtr->constPtr->activationLimit + 1) ) {
            /* Standard */
            rv=E_OS_LIMIT; /* OSEK Standard Status */
            Irq_Restore(flags);
            --destPcbPtr->activations;
            OS_STD_ERR_1(OSServiceId_ActivateTask,TaskID);
        }
    } else {
        /* We have a suspended task, make it ready for use */
        ASSERT( destPcbPtr->activations == 1 );
        Os_Arc_SetCleanContext(destPcbPtr);
        Os_TaskMakeReady(destPcbPtr);
    }

    /* Preempt only if we are preemptable and target has higher prio than us */
    //lint -e{9007} MISRA False positive. No side effects of Os_SchedulerResourceIsFree
    if(    (OS_SYS_PTR->intNestCnt == 0) &&
        (currPcbPtr->constPtr->scheduling == FULL) &&
        (destPcbPtr->activePriority > Os_SysTaskGetCurr()->activePriority) &&
        (Os_SchedulerResourceIsFree())) {
        Os_Dispatch(OP_ACTIVATE_TASK);
    }

    Irq_Restore(flags);

    return rv;
#endif /* else of #ifdef CFG_GNULINUX*/
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

#if !defined(CFG_GNULINUX)
StatusType TerminateTask( void ) {
    StatusType rv = E_OK;
    OsTaskVarType *currPcbPtr = Os_SysTaskGetCurr();

    OS_DEBUG(D_TASK,"# TerminateTask %s\n",OS_SYS_PTR->currTaskPtr->constPtr->name);

    OS_VALIDATE_STD( OS_SYS_PTR->intNestCnt == 0, E_OS_CALLEVEL , OSServiceId_TerminateTask);
    OS_VALIDATE_STD( !Os_TaskOccupiesResources(currPcbPtr), E_OS_RESOURCE , OSServiceId_TerminateTask);
#if (OS_NUM_CORES > 1)
    OS_VALIDATE_STD( !Os_TaskOccupiesSpinlocks(currPcbPtr), E_OS_SPINLOCK , OSServiceId_TerminateTask);    /* @req 4.1.2/SWS_Os_00612 */
#endif

    Irq_Disable();

    /* Force the dispatcher to find something, even if its us */
    Os_Dispatch(OP_TERMINATE_TASK);

    ASSERT(0);

    return rv;
}
#else /* defined(CFG_GNULINUX) */
StatusType TerminateTask(){

        TaskType CurrentTaskId;

        CurrentTaskId = GetTaskId();

        if ( CurrentTaskId > -1 ) {
        /* TerminateTask is called by the running thread. (self) */
        	pthread_mutex_lock(&ThreadTasks[CurrentTaskId].mutex_lock);
        	int thread_is_started = ( ThreadTasks[CurrentTaskId].pthread_status == GNULINUX_PTHREAD_FUNCTION_STARTED );
        	if (thread_is_started)
            {
        		ThreadTasks[CurrentTaskId].pthread_status = GNULINUX_PTHREAD_FUNCTION_TERMINATED;
        	}
        	pthread_mutex_unlock(&ThreadTasks[CurrentTaskId].mutex_lock);

        	if (!thread_is_started)
        	{
                logger(LOG_ERR, "TerminateTask PTHREAD_FUNCTION_NOT_STARTED %s Id (%d) %d.", ThreadTasks[CurrentTaskId].name, CurrentTaskId, ThreadTasks[CurrentTaskId].pthread_status);
        	}
        	else
        	{
                logger_mod((LOGGER_MOD_LINOS|LOGGER_SUB_OS_TASK), LOG_INFO, "TerminateTask called for %s Id (%d).", ThreadTasks[CurrentTaskId].name, CurrentTaskId);
        	}

        	return E_OK;
        } else {
            logger(LOG_ERR, "TerminateTask ERROR. Couldn't identify calling Task ID.");
            return E_NOT_OK;
        }

        return E_OK;
}
#endif


#if !defined(CFG_GNULINUX)
StatusType ChainTask( TaskType TaskId ) {
    imask_t flags;
    StatusType rv = E_OK;
    OsTaskVarType *currPcbPtr = Os_SysTaskGetCurr();
    OsTaskVarType *destPcbPtr;

    OS_VALIDATE_STD_1( TASK_CHECK_ID(TaskId) , E_OS_ID, 
                OSServiceId_ChainTask,TaskId); 
    destPcbPtr = Os_TaskGet(TaskId);

    OS_DEBUG(D_TASK,"# ChainTask %s\n",destPcbPtr->constPtr->name);

    OS_VALIDATE_STD_1( OS_SYS_PTR->intNestCnt == 0, E_OS_CALLEVEL , 
                OSServiceId_ChainTask,TaskId); 
    OS_VALIDATE_STD_1( !Os_TaskOccupiesResources(currPcbPtr), E_OS_RESOURCE , 
                OSServiceId_ChainTask,TaskId); 
#if (OS_NUM_CORES > 1)
    OS_VALIDATE_STD_1( !Os_TaskOccupiesSpinlocks(currPcbPtr), E_OS_SPINLOCK , 
                OSServiceId_ChainTask,TaskId);    /* @req 4.1.2/SWS_Os_00612 */ 
#endif

    Irq_Save(flags);

    if (currPcbPtr != destPcbPtr) {
        /** @req OSEK_? Too many task activations */

        if( (destPcbPtr->activations + 1) >  destPcbPtr->constPtr->activationLimit ) {
            rv = E_OS_LIMIT;    /* OSEK Standard Status */
            Irq_Restore(flags);
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


#if    (OS_APPLICATION_CNT > 1) && (OS_NUM_CORES > 1)
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

    Os_Dispatch(OP_CHAIN_TASK);

    ASSERT( 0 );

    return rv;
}
#endif /* !defined(CFG_GNULINUX) */

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
#if !defined(CFG_GNULINUX)
StatusType Schedule( void ) {
    StatusType rv = E_OK;
    imask_t flags;
    OsTaskVarType *currPcbPtr = Os_SysTaskGetCurr();

    OS_DEBUG(D_TASK,"# Schedule %s\n",currPcbPtr->constPtr->name);

    /* Check that we are not calling from interrupt context */
    OS_VALIDATE_STD( OS_SYS_PTR->intNestCnt == 0, E_OS_CALLEVEL , OSServiceId_Schedule);              /* @req 4.1.2/SWS_Os_00088 */
    OS_VALIDATE_STD( !Os_TaskOccupiesResources(currPcbPtr), E_OS_RESOURCE , OSServiceId_Schedule);    /* OSEK */
#if (OS_NUM_CORES > 1)
    OS_VALIDATE_STD( !Os_TaskOccupiesSpinlocks(currPcbPtr), E_OS_SPINLOCK , OSServiceId_Schedule);    /* @req 4.1.2/SWS_Os_00624 */
#endif


    ASSERT( Os_SysTaskGetCurr()->state & ST_RUNNING );

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

    return rv;
}
#endif /* !defined(CFG_GNULINUX) */

void Os_Arc_GetTaskInfo( Arc_PcbType *pcbPtr, TaskType taskId, uint32 flags ) {
    OsTaskVarType *taskPtr = Os_TaskGet(taskId);

    if( flags & OS_ARC_F_TASK_BASIC) {
        strncpy(pcbPtr->name,taskPtr->constPtr->name,OS_ARC_PCB_NAME_SIZE);
        pcbPtr->tasktype = (taskPtr->constPtr->proc_type == PROC_EXTENDED ) ? ARC_TASKTYPE_EXENDED : ARC_TASKTYPE_BASIC;
    }

    if( flags & OS_ARC_F_TASK_STACK) {
        Os_Arc_GetStackInfo(taskId, &pcbPtr->stack );
    }
}



