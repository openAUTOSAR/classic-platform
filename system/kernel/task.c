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
#include "sys.h"
#include "internal.h"
#include "hooks.h"
#include <stdlib.h>
#include "swap.h"
#include "task_i.h"
#include "Ramlog.h"

StatusType GetTaskState(TaskType TaskId, TaskStateRefType State) {
	state_t curr_state = os_pcb_get_state(os_get_pcb(TaskId));
	StatusType rv = E_OK;

	// TODO: Lazy impl. for now */
	switch(curr_state) {
	case ST_RUNNING: *State = TASK_STATE_RUNNING;  break;
	case ST_WAITING: *State = TASK_STATE_WAITING;  break;
	case ST_SUSPENDED: *State = TASK_STATE_SUSPENDED;  break;
	case ST_READY: *State = TASK_STATE_READY;  break;
	}

	// Prevent label warning. Remove when proper error handling is implemented.
	if (0) goto err;

	OS_STD_END_2(OSServiceId_GetTaskState,TaskId, State);
}

StatusType GetTaskID( TaskRefType task_id ) {
	*task_id = os_sys.curr_pcb->pid;
	return E_OK;
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
 * @param pid
 * @return
 */

StatusType ActivateTask( TaskType TaskID ) {
	long msr;
	pcb_t *pcb = os_get_pcb(TaskID);
	StatusType rv = E_OK;

	os_isr_printf(D_TASK,"ActivateTask %s\n",pcb->name);

	Irq_Save(msr);

	// TODO: It doesn't say anything about if we do this
	//       on a waiting task!
	if( !os_pcb_pid_valid(pcb) ) {
		ramlog_str("E_OS_ID\n");
		ramlog_str(pcb->name);
		ramlog_hex(pcb->pid);
		rv = E_OS_ID;
		goto err;
	}

	if( os_pcb_get_state(pcb) != ST_SUSPENDED ) {
		ramlog_str("#E_OS_LIMIT\n");
		ramlog_str(pcb->name);
		ramlog_hex(pcb->pid);
		ramlog_str(" ");
		ramlog_hex(pcb->state);
		ramlog_str(" ");
		rv = E_OS_LIMIT;
		goto err;

	} else {
		/* TODO: This makes some things double.. cleanup is needed */
		os_pcb_make_virgin(pcb);
	}

	os_pcb_make_ready(pcb);
	Irq_Restore(msr);

	// Following chapter 4.6.1 in OSEK/VDX here it seems we should re-schedule.
	if( (pcb->scheduling == SCHEDULING_FULL) &&  (os_sys.int_nest_cnt == 0) ) {
		Schedule();
	}

	OS_STD_END_1(OSServiceId_ActivateTask,TaskID);
}

extern void os_pcb_make_virgin(pcb_t *pcb);

StatusType TerminateTask( void ) {
	pcb_t *curr_pcb = os_get_curr_pcb();
	pcb_t *new_pcb;
	StatusType rv = E_OK;
	uint32_t flags;

	os_std_printf(D_TASK,"TerminateTask %s\n",curr_pcb->name);

	Irq_Save(flags);

	os_pcb_make_suspended(curr_pcb);

	// Schedule any process
	new_pcb = os_find_top_prio_proc();
	assert(new_pcb!=NULL);
	os_swap_context(curr_pcb,new_pcb);

	Irq_Restore(flags);
	// It must find something here...otherwise something is very wrong..
	assert(0);

	rv = E_NOT_OK;
	goto err;


	OS_STD_END(OSServiceId_TerminateTask);
}

StatusType ChainTask( TaskType TaskId ) {
	StatusType rv;
	uint32_t flags;

	Irq_Save(flags);
	rv = ActivateTask(TaskId);
	/* TODO: more more here..*/
	TerminateTask();
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
 */
StatusType Schedule( void ) {
	pcb_t *pcb;
	pcb_t *curr_pcb = get_curr_pcb();
	StatusType rv = E_OK;
	uint32_t flags;

	Irq_Save(flags);
	/* Try to find higher prio task that is ready, if none, continue */
	pcb = os_find_top_prio_proc();

	/* Swap if we found any process */
	if( pcb != curr_pcb ) {
		/* Add us to the ready list */
		os_pcb_running_to_ready(curr_pcb);
		os_swap_context(curr_pcb,pcb);
	}
	Irq_Restore(flags);

	// Prevent label warning. Remove this when proper error handling is implemented.
	if (0) goto err;

	OS_STD_END(OSServiceId_Schedule);
}

