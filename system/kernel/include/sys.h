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

#ifndef SYS_H_
#define SYS_H_

#include "task_i.h"
#include "isr.h"


#define OS_SYS_PTR			(&Os_Sys[GetCoreID()])


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

typedef struct Os_CoreStatus {
	boolean activated;
	boolean os_started;
	boolean init_os_called;
} Os_CoreStatusType;
/*
 * Global system structure
 */
typedef struct Os_Sys {

	OsTaskVarType 	*currTaskPtr;		/* Current running task*/
	OsIsrVarType 	*currIsrPtr;
	OsTaskVarType 	*pcb_list;			/* List of all tasks */

	OsTaskVarType 	*chainedPcbPtr;
	uint32_t 		intNestCnt;			/* Interrupt nested count, 0 if no interrupt active */
	TickType 		tick;				/* The OS Tick counter */
	uint8_t 		op;					/* The current operation */
	void *			intStack;			/* Ptr to the interrupt stack */
	struct OsHooks *hooks;

	/* parameters for functions, used by OSErrorXXX() */
	uint32_t param1;
	uint32_t param2;
	uint32_t param3;
	uint32_t serviceId;

	AppModeType appMode;	/* According to OSEK 8.3 RES_SCHEDULER is accessible to all tasks */

#if	(OS_USE_APPLICATIONS == STD_ON)
	ApplicationStateType currApplState;
	ApplicationType 	 currApplId;
#endif

	uint32_t task_cnt;
	uint32_t isrCnt;

#if defined(USE_KERNEL_EXTRA)
	/* List of PCB's to be put in ready list when timeout */
	TAILQ_HEAD(,OsTaskVar) timerHead;		// TASK
#endif

	TAILQ_HEAD(,OsTaskVar) ready_head;	/* Ready queue */
	OsResourceType resScheduler;		/* According to OSEK 8.3 RES_SCHEDULER is accessible to all tasks */
	Os_CoreStatusType status;
} Os_SysType;

extern Os_SysType Os_Sys[OS_NUM_CORES];

static inline _Bool Os_SchedulerResourceIsFree( void ) {
	return (OS_SYS_PTR->resScheduler.owner == NO_TASK_OWNER );
}

static inline void Os_SysTaskSetCurr( OsTaskVarType *pcb ) {
	OS_SYS_PTR->currTaskPtr = pcb;
}

static inline OsTaskVarType *Os_SysTaskGetCurr( void ) {
	return OS_SYS_PTR->currTaskPtr;
}

static inline OsIsrVarType *Os_SysIsrGetCurr( void ) {
	return OS_SYS_PTR->currIsrPtr;
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
