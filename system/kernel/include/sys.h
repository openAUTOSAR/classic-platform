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
	OP_SLEEP = 128,

} OpType ;

/*
 * Global system structure
 */
typedef struct Os_Sys {
//	OsApplicationType *curr_application;
	/* Current running task*/
	OsTaskVarType *currTaskPtr;

	/* List of all tasks */
	OsTaskVarType *pcb_list;

	OsTaskVarType *chainedPcbPtr;
	/* Interrupt nested count */
	uint32 intNestCnt;
	/* The current operation */
	uint8_t op;
	/* Ptr to the interrupt stack */
	void *intStack;
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

#if	(OS_USE_APPLICATIONS == STD_ON)
	ApplicationStateType currApplState;
	ApplicationType currApplId;
#endif

	uint32_t task_cnt;

	uint32_t isrCnt;
#if defined(USE_KERNEL_EXTRA)

/* List of PCB's to be put in ready list when timeout */
	TAILQ_HEAD(,OsPcb) timerHead;		// TASK
#endif

	/* List of all pcb's,
	 * Only needed for non-static configuration of the kernel
	 */
//	TAILQ_HEAD(,OsTaskVar) pcb_head;
	/* Ready queue */
	TAILQ_HEAD(,OsTaskVar) ready_head;

//	TAILQ_HEAD(,OsIsrVar) isrHead;

	/* Occording to OSEK 8.3 RES_SCHEDULER is accessible to all tasks */
	OsResourceType resScheduler;
} Os_SysType;

extern Os_SysType Os_Sys;

static inline OsTaskVarType *Os_TaskGetCurrent(  void ) {
	return Os_Sys.currTaskPtr;
}

#if 0
static uint32_t OSErrorGetServiceId( void ) {
	return Os_Sys.serviceId;
}
#endif


#endif /*SYS_H_*/
