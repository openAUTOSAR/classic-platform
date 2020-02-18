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

#include "os_i.h"


/**
 * The state of the calling task is set to waiting, unless at least one
 * of the events specified in <Mask> has already been set.
 *
 * This call enforces rescheduling, if the wait condition occurs. If
 * rescheduling takes place, the internal resource of the task is
 * released while the task is in the waiting state.
 * This service shall only be called from the extended task owning
 * the event.
 *
 * From 7.6.1 in Autosar OS 4.0
 * An event is accessible if the task for which the event can be set
 * is accessible. Access means that these Operating System objects are
 * allowed as parameters to API services.
 *
 * @param Mask Mask of the events waited for
 * @return
 */
/* @req OSEK_SWS_EV_00001 */
StatusType WaitEvent( EventMaskType Mask ) {

    OsTaskVarType *curr_pcb = Os_SysTaskGetCurr();
    StatusType rv = E_OK;
    imask_t state;

    OS_DEBUG(D_EVENT,"# WaitEvent %s\n",Os_SysTaskGetCurr()->constPtr->name);
    /* Validation of parameters, if failure, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_1((Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT,
    		          OSServiceId_WaitEvent,Mask);   /* @req SWS_Os_00093 */
    OS_VALIDATE_STD_1(OS_SYS_PTR->intNestCnt == 0, E_OS_CALLEVEL,
    		          OSServiceId_WaitEvent,Mask);  /* @req OSEK_SWS_EV_00004 */
    OS_VALIDATE_STD_1(curr_pcb->constPtr->proc_type == PROC_EXTENDED,E_OS_ACCESS,
    		          OSServiceId_WaitEvent,Mask); /* @req OSEK_SWS_EV_00002 */
    OS_VALIDATE_STD_1(!Os_TaskOccupiesResources(curr_pcb),E_OS_RESOURCE,
    		          OSServiceId_WaitEvent,Mask); /* @req OSEK_SWS_EV_00003 */
#if	(OS_NUM_CORES > 1)
    OS_VALIDATE_STD_1(!Os_TaskOccupiesSpinlocks(curr_pcb) != 0,E_OS_SPINLOCK,
    		          OSServiceId_WaitEvent,Mask);       /* SWS_Os_00622 */
#endif

    /* Remove from ready queue */
    Irq_Save(state);

    // OSEK/VDX footnote 5. The call of WaitEvent does not lead to a waiting state if one of the events passed in the event mask to
    // WaitEvent is already set. In this case WaitEvent does not lead to a rescheduling.
    /* @req OSEK_SWS_EV_00004 */
    if( (curr_pcb->ev_set & Mask) == 0 ) {

        curr_pcb->ev_wait = Mask;

        Os_Dispatch(OP_WAIT_EVENT);
        ASSERT( curr_pcb->state & ST_RUNNING );
    }

    Irq_Restore(state);

    return rv;
}

/**
 * The events of task <TaskID> are set according to the event
 * mask <Mask>. Calling SetEvent causes the task <TaskID> to
 * be transferred to the  ready state, if it was waiting for at least
 * one of the events specified in <Mask>.
 *
 * @param TaskID - Reference to the task for which one or several events are to be set.
 * @param Mask - Mask of the events to be set
 * @return
 */

StatusType SetEvent( TaskType TaskID, EventMaskType Mask ) {
    StatusType rv = E_OK;
    OsTaskVarType *destPcbPtr;
    const OsTaskVarType *currPcbPtr;
    imask_t flags;

    OS_DEBUG(D_EVENT,"# SetEvent %s\n",Os_SysTaskGetCurr()->constPtr->name);
	/* Validation of parameters, if failure, function will return */
	/* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_2( TASK_CHECK_ID(TaskID) , E_OS_ID,
    		           OSServiceId_SetEvent,TaskID, Mask); /*@req OSEK_SWS_EV_00007 */

    destPcbPtr = Os_TaskGet(TaskID);
    currPcbPtr = Os_SysTaskGetCurr();

#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)

    if( destPcbPtr->constPtr->applOwnerId != OS_SYS_PTR->currApplId ) {
        ApplicationType appId;
        /* If the application state is not APPLICATION_ACCESSIBLE then control flow will
         * reach in OS_STD_ERR_2 and ErrorHook will be called  */
        OS_VALIDATE_STD_2( (Os_ApplCheckState(destPcbPtr->constPtr->applOwnerId) != E_OS_ACCESS) , E_OS_ACCESS,
        		           OSServiceId_SetEvent,TaskID, Mask);
        /* getting the OwnerId of Task/ISR */
        appId = Os_GetCurrTaskISROwnerId();
        /* Do we have access to the task we are activating */
        /* If the application does not have access then control flow will
         * reach in OS_STD_ERR_2 and ErrorHook will be called  */
        OS_VALIDATE_STD_2( (Os_ApplCheckAccess(appId , destPcbPtr->constPtr->accessingApplMask) != E_OS_ACCESS) , E_OS_ACCESS,
        		           OSServiceId_SetEvent,TaskID, Mask);

#if (OS_NUM_CORES > 1)
        if (Os_ApplGetCore(destPcbPtr->constPtr->applOwnerId) != GetCoreID()) {
                    StatusType status = Os_NotifyCore(Os_ApplGetCore(destPcbPtr->constPtr->applOwnerId),
                                                      OSServiceId_SetEvent,
                                                      TaskID,
                                                      Mask,
                                                      0);
                    return status;
#endif

    }
#endif

    OS_VALIDATE_STD_2(destPcbPtr->constPtr->proc_type == PROC_EXTENDED, E_OS_ACCESS,
    		          OSServiceId_SetEvent,TaskID, Mask);  /*@req OSEK_SWS_EV_00008 */
    OS_VALIDATE_STD_2((destPcbPtr->state & ST_SUSPENDED) == 0, E_OS_STATE,
    		          OSServiceId_SetEvent,TaskID, Mask); /*@req OSEK_SWS_EV_00009 */

    Irq_Save(flags);

    /* Calling  SetEvent  causes  the  task  <TaskID>  to be  transferred
     * to  the  ready  state,  if  it  was  waiting  for  at  least one of the
     * events specified in <Mask>.
     *
     * OSEK/VDX 4.6.1,  rescheduling is performed in all of the following cases:
     * ..
     * Setting an event to a waiting task at task level (e.g. system service SetEvent,
     * see chapter 13.5.3.1, message notification mechanism, alarm expiration, if event setting
     * defined, see chapter 9.2)
     * ... */

    destPcbPtr->ev_set |= Mask;

    if( (Mask & destPcbPtr->ev_wait) != 0) {
        /* We have an event match */
        /*lint -e{9036} MISRA:OTHER: bit level comparison is required:[MISRA 2012 Rule 14.4, required]*/
        if( destPcbPtr->state & ST_WAIT_EVENT){
#if defined(CFG_T1_ENABLE)
            Os_ReleaseEventHook(destPcbPtr->constPtr->pid);
#endif
            Os_TaskMakeReady(destPcbPtr);

            currPcbPtr = Os_SysTaskGetCurr();
            /* Checking "4.6.2  Non preemptive scheduling" it does not dispatch if NON  */
            if( (OS_SYS_PTR->intNestCnt == 0) &&
                (currPcbPtr->constPtr->scheduling == FULL) &&
                (destPcbPtr->activePriority > currPcbPtr->activePriority) &&
                (Os_SysIntAnyDisabled() == FALSE ) ) /*lint !e9007, OK side effects */
            {
                Os_Dispatch(OP_SET_EVENT); /* @req  OSEK_SWS_EV_00018 */
            }
          /*lint -e{9036} MISRA:OTHER: bit level comparison is required:[MISRA 2012 Rule 14.4, required]*/
        __CODE_COVERAGE_IGNORE__
        } else if(destPcbPtr->state & (ST_READY | ST_RUNNING | ST_SLEEPING | ST_WAIT_SEM) ) {
            /* Do nothing */
        } else {
            /* @CODECOV:DEFAULT_CASE: Default statement is required for defensive programming. See above...*/
            ASSERT( 0 );
        }
    }

    Irq_Restore(flags);

    return rv;
}


/**
 * This service returns the current state of all event bits of the task
 * <TaskID>, not the events that the task is waiting for.
 * The service may be called from interrupt service routines, task
 * level and some hook routines (see Figure 12-1).
 *  The current status of the event mask of task <TaskID> is copied
 * to <Event>.
 *
 * @param TaskId Task whose event mask is to be returned.
 * @param Mask   Reference to the memory of the return data.
 * @return
 */
StatusType GetEvent( TaskType TaskId, EventMaskRefType Mask) {

    const OsTaskVarType *destPcbPtr;
    StatusType rv = E_OK;
#if (OS_SC1==STD_ON) || (OS_SC2==STD_ON)
    imask_t flags;
#endif

	/* Validation of parameters, if failure, function will return */
	/* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_2( (Mask != NULL), E_OS_PARAM_POINTER,
    		           OSServiceId_GetEvent,TaskId, Mask);   /* @req SWS_Os_00566 */
#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
    OS_VALIDATE_STD_2( (OS_VALIDATE_ADDRESS_RANGE(Mask,sizeof(EventMaskType)) == TRUE ) , E_OS_ILLEGAL_ADDRESS ,
    		           OSServiceId_GetEvent,TaskId, Mask);/*@req SWS_Os_00051 */
#endif
    OS_VALIDATE_STD_2( (Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT,
    		          OSServiceId_GetEvent,TaskId, Mask);   /* @req SWS_Os_00093 */

    OS_VALIDATE_STD_2( TASK_CHECK_ID(TaskId) , E_OS_ID,
    		           OSServiceId_GetEvent,TaskId, Mask); /* @req OSEK_SWS_EV_00011 */

    /* @req OSEK_SWS_EV_00020 */
    destPcbPtr = Os_TaskGet(TaskId); /* @req  OSEK_SWS_EV_00014 */

    OS_VALIDATE_STD_2( (destPcbPtr->constPtr->proc_type == PROC_EXTENDED), E_OS_ACCESS,
    		           OSServiceId_GetEvent,TaskId, Mask);  /* @req OSEK_SWS_EV_00012 */
    OS_VALIDATE_STD_2( (destPcbPtr->state & ST_SUSPENDED) == 0, E_OS_STATE,
    		           OSServiceId_GetEvent,TaskId, Mask);  /* @req OSEK_SWS_EV_00013 */
/* @req OSEK_SWS_EV_00021 */
#if (OS_SC1==STD_ON) || (OS_SC2==STD_ON)
    /* For SC1 and SC2 EventMaskType is 64bit,
     * the atomicity of read operation is depends on compiler (even though arch's ISR has load double inst)
     * and in most case this 64bit operation is not atomic. Hence making this as atomic.
     */
    Irq_Save(flags);
    *Mask = destPcbPtr->ev_set;
    Irq_Restore(flags);
#else
    *Mask = destPcbPtr->ev_set;
#endif

    return rv;
}


/**
 * The events of the extended task calling ClearEvent are cleared
 * according to the event mask <Mask>.
 *
 *
 * @param Mask
 * @return
 */
StatusType ClearEvent( EventMaskType Mask) {
    StatusType rv = E_OK;
    OsTaskVarType *pcb = Os_SysTaskGetCurr();
    imask_t flags;

	/* Validation of parameters, if failure, function will return */
	/* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_1((Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT,
    		          OSServiceId_ClearEvent,Mask);   /* @req SWS_Os_00093 */
    OS_VALIDATE_STD_1(OS_SYS_PTR->intNestCnt == 0,E_OS_CALLEVEL,
    		          OSServiceId_ClearEvent,Mask); /* @req OSEK_SWS_EV_00016 */
    OS_VALIDATE_STD_1(pcb->constPtr->proc_type == PROC_EXTENDED,E_OS_ACCESS,
    		          OSServiceId_ClearEvent,Mask); /* @req OSEK_SWS_EV_00015 */

    Irq_Save(flags);
    pcb->ev_set &= ~Mask; /* @req OSEK_SWS_EV_00017 */
    Irq_Restore(flags);

    return rv;
}





