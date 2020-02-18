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

/**
 * @brief   Semaphores
 *
 * @addtogroup OS
 * @details Implements semaphores and binary semaphores.
 *
 * @{
 */

/* ----------------------------[includes]------------------------------------*/
#include "os_i.h"
#include "os_internal.h"
#include "logger.h"
#include "sys/queue.h"

#if defined(CFG_LOG) && (LOG_OS_SEM)
#define _LOG_NAME_ "os_sem"
#endif
#include "log.h"


/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
/* ----------------------------[private functions]---------------------------*/



/**
 * @brief   Add a task, tP, last in a list of tasks waiting on
 *          a semaphore
 *
 * @param[in] sP Pointer to semaphore
 * @param[in] tP Pointer to task
 */
 static void Os_SemQAddTail(OsSemType *sP, OsTaskVarType *tP) {
    STAILQ_INSERT_TAIL(&sP->taskHead, tP, semEntry);
}

 /**
 * @brief  return the first task waiting on a semaphore
 *
 * @param[in] sP        Pointer to semaphore
 * @return    Pointer to the first task
 */
 static OsTaskVarType * Os_SemQFirst(OsSemType *sP) {
    return STAILQ_FIRST(&sP->taskHead);
}

 /**
  * @brief   Remove the first task waiting on the semaphore
  *
  * @param[in] sP       Pointer to semaphore
  */
static void Os_SemQRemoveHead(OsSemType *sP) {
    STAILQ_REMOVE_HEAD(&sP->taskHead, semEntry);
}

/**
 * @brief   Remove a task in the list of tasks waiting for semaphore sP
 *
 * @param[in] sP        Pointer to semaphore
 * @param[in] tP        Pointer to task
 */
static void Os_SemQRemove(OsSemType *sP, OsTaskVarType *tP) {
    STAILQ_REMOVE(&sP->taskHead, tP, OsTaskVar, semEntry);
}

/**
 * @brief   Function to check if there is any waiting for the semaphore
 *
 * @param[in]  sP       Pointer to semaphore
 * @return
 */
static boolean Os_SemQIsEmpty(OsSemType *sP) {
    return STAILQ_EMPTY( &sP->taskHead );
}


/**
 * @brief  Perform sanity checking on a task
 *         were a semaphore service is called
 *
 * @param[in] tP        Pointer to task
 * @return
 */
static StatusType SemWaitChecks( OsTaskVarType *tP ) {
	StatusType rv = E_OK;
    ASSERT(OS_SYS_PTR->intNestCnt == 0);

    if (tP->constPtr->proc_type != PROC_EXTENDED) {
        rv = E_OS_ACCESS;
        OS_STD_ERR_1(OSServiceId_SemWaitChecks,rv);
    }
	else if (Os_TaskOccupiesResources(tP)) {
		rv = E_OS_RESOURCE;
        OS_STD_ERR_1(OSServiceId_SemWaitChecks,rv);
	}

    return rv;
}

/* ----------------------------[public functions]----------------------------*/


/**
 * @brief   Function to initialize a semaphore
 * @details
 * @param[in] semPtr        Pointer to semaphore to initialize
 * @param[in] initialCount  Initial count of the semaohore
 * @return
 */
StatusType SemInit(OsSemType *semPtr, sint32 initialCount ) {

    StatusType rv = E_OK;

    if( semPtr != NULL_PTR ) {
        /* Check for empty */
        ASSERT( Os_SemQIsEmpty( semPtr ));

        semPtr->count = initialCount;
        STAILQ_INIT(&semPtr->taskHead);
    } else {
        /* NULL_PTR pointer check */
        Os_CallErrorHook(E_OS_VALUE);
        rv = E_OS_VALUE;
    }

    return rv;
}


StatusType SemWait(OsSemType *semPtr) {
    return SemWaitTmo(semPtr, TMO_INFINITE );
}

StatusType SemWaitTmo(OsSemType *semPtr, TickType tmo) {
    OsTaskVarType *pcbPtr = Os_SysTaskGetCurr();
    uint32_t flags;
    StatusType rv = E_OK;

    Irq_Save(flags);

    LOG_S_U32(__func__,(uint32)semPtr);

    if( semPtr != NULL_PTR ) {
        /* Someone is waiting at the semaphore, so it can't be empty  OR
         * Counter with values 0,1 must have an empty queue*/
        ASSERT( ((semPtr->count < 0) && !Os_SemQIsEmpty(semPtr)) ||
                ((semPtr->count >= 0) && Os_SemQIsEmpty(semPtr)) );

        if( (rv = SemWaitChecks(pcbPtr)) == E_OK ) {

            semPtr->count--;
            if( semPtr->count < 0 ) {
                /* It's locked */
                if (tmo == TMO_MIN ) {
                    /* Restore sem value and return timeout */
                    semPtr->count++;
                    Irq_Restore(flags);
                    return E_OS_TIMEOUT;
                }

                LOG_S_U32("WAIT",(uint32)semPtr);

                /* Add us to the current task */
                OS_SYS_PTR->currTaskPtr->semPtr = semPtr;
                /* Add this task to the semaphore */
                Os_SemQAddTail( semPtr, pcbPtr);

                rv = Os_DispatchToSleepWithTmo(OP_WAIT_SEMAPHORE, tmo);

                /* Return values here are:
                 * E_OK             We dispatched
                 * E_OS_RESOURCE    Dispatcher locked..
                 * E_OS_TIMEOUT     Timeout
                 */

                if( rv == E_OS_TIMEOUT ) {
                    LOG_S_U32("REMOVE",(uint32)semPtr);

                    /* Remove us from the semaphore queue.
                     * We have already removed from the timer queue and put into ready state here  */
                    Os_SemQRemove( semPtr, OS_SYS_PTR->currTaskPtr);
                    /* Restore sem value */
                    semPtr->count++;
               }

            } else {
                /* It's unlocked */
                LOG_S_U32("UNLOCKED",(uint32)semPtr);
            }
        }
    } else {
        /* NULL_PTR pointer check */
        Os_CallErrorHook(E_OS_VALUE);
        rv = E_OS_VALUE;
    }

    Irq_Restore(flags);

    return rv;
}

StatusType SemSignal(OsSemType *semPtr) {
    uint32_t flags;
    OsTaskVarType *taskPtr;
    StatusType rv = E_OK;

    Irq_Save(flags);

    LOG_S_U32(__func__,(uint32)semPtr);

    ASSERT( ((semPtr->count < 0) && !Os_SemQIsEmpty(semPtr)) ||
            ((semPtr->count >= 0) && Os_SemQIsEmpty(semPtr)) );

    if (semPtr != NULL_PTR) {
        semPtr->count++;
        if (semPtr->count <= 0) {

            LOG_S_U32("RM_FIRST",(uint32)semPtr);

            /* Remove the first task that waits at the semaphore */
            taskPtr = Os_SemQFirst(semPtr);

            ASSERT(taskPtr != NULL_PTR);

            /* Release the first task in queue */
            Os_SemQRemoveHead(semPtr);

            if( taskPtr->state == ST_SUSPENDED) {
                /* We tried to signal semaphore that have a suspended task */
                Os_CallErrorHook(E_OS_STATE);
                rv = E_OS_VALUE;
            } else {
                LOG_S_U32("WAKE",(uint32)semPtr);

                /* Remove us from the timerQ */
                if (taskPtr->tmoVal != TMO_INFINITE ) {
                    Os_TimerQRemove(taskPtr);
                }

                taskPtr->rv = E_OK;
                Os_TaskMakeReady(taskPtr);

                if(  (OS_SYS_PTR->intNestCnt == 0) &&
                     (Os_SysTaskGetCurr()->constPtr->scheduling == FULL) &&
                     (taskPtr->activePriority > Os_SysTaskGetCurr()->activePriority) &&
                     (Os_SysIntAnyDisabled() == FALSE ) &&
                     (Os_SchedulerResourceIsFree())) {

                     Os_Dispatch(OP_SIGNAL_SEMAPHORE);
                }
            }
        }
        else {
            /* We do nothing */
            /* IMPROVEMENT: We could do a counter wrap check here */
        }
    }
    else {
        /* NULL_PTR pointer check */
        Os_CallErrorHook(E_OS_VALUE);
        rv = E_OS_VALUE;
    }

    Irq_Restore(flags);

    return rv;
}

/**
 * @brief   Function to signal a binary semaphore.
 *
 * @param[in] semPtr        Pointer to a semaphore.
 *                          .
 * @retval E_OS_VALUE       Semaphore count was already 1.
 * @return See SemSignal for more return values.
 */
StatusType BSemSignal( OsSemType *semPtr ) {
    StatusType rv;

    /* semPtr is also checked in SemSignal */
    if (semPtr != NULL_PTR) {
        /* Binary semaphores may not grow above 1 */
        if( semPtr->count == BSEM_UNLOCKED ) {
            Os_CallErrorHook(E_OS_VALUE);
            rv = E_OS_VALUE;
        } else {
            rv = SemSignal(semPtr);
        }
    } else {
        /* NULL_PTR pointer check */
        Os_CallErrorHook(E_OS_VALUE);
        rv = E_OS_VALUE;
    }

    return rv;
}

/** @} */
