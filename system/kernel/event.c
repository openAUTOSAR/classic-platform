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









#include "Os.h"
#include "pcb.h"
#include "sys/queue.h"
#include "sys.h"
#include <stdlib.h>
#include "internal.h"
#include "swap.h"
#include "task_i.h"
#include "hooks.h"

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

	pcb_t *curr_pcb = get_curr_pcb();
	StatusType rv = E_OK;

	/* Remove from ready queue */
	Irq_Disable();

	// Reschedule if mask not set already
	if( !(curr_pcb->ev_set & Mask) ) {

		curr_pcb->ev_wait = Mask;
		os_pcb_make_waiting(curr_pcb);
		{
			pcb_t *pcb;
			pcb = os_find_top_prio_proc();
			assert(pcb!=NULL);
			os_swap_context(curr_pcb,pcb);
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
	pcb_t *dest_pcb;

	dest_pcb = os_get_pcb(TaskID);
	Irq_Disable();

	if( (dest_pcb->state & ST_SUSPENDED ) ) {
		rv = E_OS_STATE;
		goto err;
	}

	if( dest_pcb->proc_type != PROC_EXTENDED ) {
		rv = E_OS_ACCESS;
		goto err;
	}

	// If at least one of the events match, taskID from waiting to ready
	// (if not already)
	if( Mask & dest_pcb->ev_wait ) {
		os_pcb_make_ready(dest_pcb);
	}

	dest_pcb->ev_set |= Mask;
	Irq_Enable();

	OS_STD_END_2(OSServiceId_SetEvent,TaskID, Mask);
}

StatusType GetEvent( TaskType TaskId, EventMaskRefType Mask) {

	pcb_t *dest_pcb;
	StatusType rv = E_OK;

	dest_pcb = os_get_pcb(TaskId);

	VALIDATE_W_RV(dest_pcb->state & ST_SUSPENDED,E_OS_STATE);
	VALIDATE_W_RV(dest_pcb->proc_type != PROC_EXTENDED,E_OS_ACCESS);

	*Mask = dest_pcb->ev_set;

	if (0) goto err;

	OS_STD_END_2(OSServiceId_GetEvent,TaskId, Mask);
}


StatusType ClearEvent( EventMaskType Mask) {
    StatusType rv = E_OK;
	pcb_t *pcb;
	pcb = get_curr_pcb();
	pcb->ev_set &= ~Mask;

	if (0) goto err;

	OS_STD_END_1(OSServiceId_ClearEvent,Mask);
}





