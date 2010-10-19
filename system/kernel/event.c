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

#include <sys/queue.h>
#include <stdlib.h>
#include "Os.h"
#include "internal.h"

#define VALIDATE_W_RV(_exp,_rv) \
	if( (_exp) ) { \
		ERRORHOOK(_rv); \
		return _rv; \
	}


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
 * @param Mask Mask of the events waited for
 * @return
 */

StatusType WaitEvent( EventMaskType Mask ) {

	OsPcbType *curr_pcb = get_curr_pcb();
	StatusType rv = E_OK;

	OS_DEBUG(D_EVENT,"# WaitEvent %s\n",Os_TaskGetCurrent()->name);

	if( os_sys.int_nest_cnt != 0 ) {
		rv =  E_OS_CALLEVEL;
		goto err;
	}

	if (curr_pcb->proc_type != PROC_EXTENDED) {
		rv = E_OS_ACCESS;
		goto err;
	}

	if ( Os_TaskOccupiesResources(curr_pcb) ) {
		rv = E_OS_RESOURCE;
		goto err;
	}

	/* Remove from ready queue */
	Irq_Disable();

	// OSEK/VDX footnote 5. The call of WaitEvent does not lead to a waiting state if one of the events passed in the event mask to
    // WaitEvent is already set. In this case WaitEvent does not lead to a rescheduling.
	if( !(curr_pcb->ev_set & Mask) ) {

		curr_pcb->ev_wait = Mask;

		if ( Os_SchedulerResourceIsFree() ) {
			// Os_TaskMakeWaiting(curr_pcb);
			Os_Dispatch(OP_WAIT_EVENT);
			assert( curr_pcb->state & ST_RUNNING );
		} else {
			Os_TaskMakeWaiting(curr_pcb);
		}
	}

	Irq_Enable();

	// The following line disables the unused label warning. Remove when
	// proper error handling is implemented.
	if (0) goto err;

	OS_STD_END_1(OSServiceId_WaitEvent,Mask);
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
	OsPcbType *dest_pcb;
	OsPcbType *currPcbPtr;
	uint32_t flags;

	OS_DEBUG(D_EVENT,"# SetEvent %s\n",Os_TaskGetCurrent()->name);

	if( TaskID  >= Os_CfgGetTaskCnt() ) {
		rv = E_OS_ID;
		goto err;
	}

	dest_pcb = os_get_pcb(TaskID);

#if (OS_STATUS_EXTENDED == STD_ON )
	if( dest_pcb->proc_type != PROC_EXTENDED ) {
		rv = E_OS_ACCESS;
		goto err;
	}

	if( (dest_pcb->state & ST_SUSPENDED ) ) {
		rv = E_OS_STATE;
		goto err;
	}
#endif

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

	dest_pcb->ev_set |= Mask;

	if( (Mask & dest_pcb->ev_wait) ) {
		/* We have an event match */
		if( dest_pcb->state & ST_WAITING) {
			Os_TaskMakeReady(dest_pcb);

			currPcbPtr = Os_TaskGetCurrent();
			/* Checking "4.6.2  Non preemptive scheduling" it does not dispatch if NON  */
			if( (os_sys.int_nest_cnt == 0) &&
				(currPcbPtr->scheduling == FULL) &&
				(dest_pcb->prio > currPcbPtr->prio) &&
				(Os_SchedulerResourceIsFree()) )
			{
				Os_Dispatch(OP_SET_EVENT);
			}

		}  else if(dest_pcb->state & (ST_READY|ST_RUNNING) ) {
			/* Hmm, we do nothing */
		} else {
			assert( 0 );
		}
	}

	Irq_Restore(flags);

	OS_STD_END_2(OSServiceId_SetEvent,TaskID, Mask);
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

	OsPcbType *dest_pcb;
	StatusType rv = E_OK;

	if( TaskId  >= Os_CfgGetTaskCnt() ) {
		rv = E_OS_ID;
		goto err;
	}

	dest_pcb = os_get_pcb(TaskId);

	VALIDATE_W_RV(dest_pcb->proc_type != PROC_EXTENDED,E_OS_ACCESS);
	VALIDATE_W_RV(dest_pcb->state & ST_SUSPENDED,E_OS_STATE);

	*Mask = dest_pcb->ev_set;

	if (0) goto err;

	OS_STD_END_2(OSServiceId_GetEvent,TaskId, Mask);
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
	OsPcbType *pcb;

	if( os_sys.int_nest_cnt != 0 ) {
		rv =  E_OS_CALLEVEL;
		goto err;
	}

	pcb = get_curr_pcb();

	if (pcb->proc_type != PROC_EXTENDED) {
		rv = E_OS_ACCESS;
		goto err;
	}

	pcb->ev_set &= ~Mask;

	if (0) goto err;

	OS_STD_END_1(OSServiceId_ClearEvent,Mask);
}





