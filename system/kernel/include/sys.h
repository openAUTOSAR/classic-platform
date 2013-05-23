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

#include "task_i.h"
#include "isr.h"

/* STD container : OsOs. OSEK properties
 * Class: ALL
 *
 * OsScalabilityClass:			0..1 SC1,SC2,SC3,SC4
 * OsStackMonitoring:			1    Stack monitoring of tasks/category 2
 * OsStatus                 	1    EXTENDED or STANDARD status
 * OsUseGetServiceId			1    We can use the  OSErrorGetServiceId() function
 * OsUseParameterAccess			1    We save the parameters in OSError_XX_YY()
 * OsUseResScheduler			1
 * OsHooks[C]               	1
 *
 * From OSEK/VDX oil:
 *
 * OS ExampleOS {
 *   STATUS = STANDARD;
 *   STARTUPHOOK = TRUE;
 *   ERRORHOOK = TRUE;
 *   SHUTDOWNHOOK = TRUE;
 *   PRETASKHOOK = FALSE;
 *   POSTTASKHOOK = FALSE;
 *   USEGETSERVICEID = FALSE;
 *   USEPARAMETERACCESS = FALSE;
 *   USERESSCHEDULER = TRUE;
 * };
 *
 * OS_SC1 | OS_SC2 | OS_SC3 | OS_SC4
 * OS_STACK_MONITORING
 * OS_STATUS_EXTENDED  / OS_STATUS_STANDARD
 * OS_USE_GET_SERVICE_ID
 * OS_USE_PARAMETER_ACCESS
 * OS_RES_SCHEDULER
 * */

struct os_conf_global_hook_s;

typedef enum  {
	OP_SET_EVENT = 1,
	OP_WAIT_EVENT = 2,
	OP_ACTIVATE_TASK = 4,
	OP_TERMINATE_TASK = 8,
	OP_SCHEDULE = 16,
	OP_CHAIN_TASK = 32,
	OP_RELEASE_RESOURCE = 64,
	OP_SLEEP = 128,
	OP_WAIT_SEMAPHORE = 256,
	OP_SIGNAL_SEMAPHORE = 512
} OpType ;

/*
 * Global system structure
 */
typedef struct Os_Sys {
//	OsApplicationType *curr_application;
	/* Current running task*/
	OsTaskVarType *currTaskPtr;

	OsIsrVarType *currIsrPtr;

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
	TAILQ_HEAD(,OsTaskVar) timerHead;		// TASK
#endif

	/* List of all pcb's,
	 * Only needed for non-static configuration of the kernel
	 */
//	TAILQ_HEAD(,OsTaskVar) pcb_head;
	/* Ready queue */
	TAILQ_HEAD(,OsTaskVar) ready_head;

//	TAILQ_HEAD(,OsIsrVar) isrHead;

	/* According to OSEK 8.3 RES_SCHEDULER is accessible to all tasks */
	OsResourceType resScheduler;
} Os_SysType;

extern Os_SysType Os_Sys;

static inline _Bool Os_SchedulerResourceIsFree( void ) {
	return (Os_Sys.resScheduler.owner == NO_TASK_OWNER );
}

static inline void Os_SysTaskSetCurr( OsTaskVarType *pcb ) {
	Os_Sys.currTaskPtr = pcb;
}

static inline OsTaskVarType *Os_SysTaskGetCurr( void ) {
	return Os_Sys.currTaskPtr;
}

static inline OsIsrVarType *Os_SysIsrGetCurr( void ) {
	return Os_Sys.currIsrPtr;
}

/**
 * Check if any of the interrupt disable/suspends counters is != 0
 *
 * @return 1 if there are outstanding disable/suspends
 */
static inline _Bool Os_SysIntAnyDisabled( void ) {
	return ((Os_IntDisableAllCnt | Os_IntSuspendAllCnt | Os_IntSuspendOsCnt) != 0);
}

/**
 * Clear all disable/system interrupts
 */
static inline void Os_SysIntClearAll( void ) {
	Os_IntDisableAllCnt = 0;
	Os_IntSuspendAllCnt = 0;
	Os_IntSuspendOsCnt = 0;
}



#endif /*SYS_H_*/
