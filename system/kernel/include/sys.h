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

#ifndef SYS_H_
#define SYS_H_

struct os_conf_global_hook_s;

typedef enum  {
	OP_SET_EVENT = 1,
	OP_WAIT_EVENT = 2,
	OP_ACTIVATE_TASK = 4,
	OP_TERMINATE_TASK = 8,
	OP_SCHEDULE = 16,
	OP_CHAIN_TASK = 32,
	OP_RELEASE_RESOURCE = 64
} OpType ;

typedef struct sys_s {
//	OsApplicationType *curr_application;
	/* Current running task*/
	OsPcbType *curr_pcb;
	/* List of all tasks */
	OsPcbType *pcb_list;

	OsPcbType *chainedPcbPtr;
	/* Interrupt nested count */
	uint32 int_nest_cnt;
	/* The current operation */
	uint8_t op;
	/* Ptr to the interrupt stack */
	void *int_stack;
	// The os tick
	TickType tick;
	// 1-The scheduler is locked (by GetResource() or something else)
//	int scheduler_lock;
	/* Hooks */
	struct OsHooks *hooks;

	// parameters for functions, used by OSErrorXXX()
	uint32_t param1;
	uint32_t param2;
	uint32_t param3;
	uint32_t serviceId;

	/* Current Application mode */
	AppModeType appMode;

//	uint32_t flags;

	uint32_t task_cnt;
	/* List of all pcb's,
	 * Only needed for non-static configuration of the kernel
	 */
	TAILQ_HEAD(,OsPcb) pcb_head;
	/* Ready queue */
	TAILQ_HEAD(,OsPcb) ready_head;

	/* Occording to OSEK 8.3 RES_SCHEDULER is accessible to all tasks */
	OsResourceType resScheduler;
} sys_t;

extern sys_t os_sys;

static inline OsPcbType *Os_TaskGetCurrent(  void ) {
	return os_sys.curr_pcb;
}

#if 0
static uint32_t OSErrorGetServiceId( void ) {
	return os_sys.serviceId;
}
#endif


#endif /*SYS_H_*/
