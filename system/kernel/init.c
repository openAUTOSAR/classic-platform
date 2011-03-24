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

/* ----------------------------[includes]------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "Os.h"
#include "internal.h"
#include "arc.h"
#include "debug.h"
#include "arch.h"

/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
Os_SysType Os_Sys;

Os_IntCounterType Os_IntDisableAllCnt;
Os_IntCounterType Os_IntSuspendAllCnt;
Os_IntCounterType Os_IntSuspendOsCnt;

/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/

extern uint32_t McuE_GetSystemClock( void );
extern OsTickType OsTickFreq;


/**
 * Copy rom pcb data(r_pcb) to ram data
 *
 * @param 	pcb		ram data
 * @param 	r_pcb	rom data
 */

static void copyPcbParts( OsTaskVarType *pcb, const OsTaskConstType *r_pcb ) {

	/* Copy VAR stuff first */




#if 0 //?????
	// Check to that the memory is ok
	{
		int cnt = sizeof(OsTaskVarType);
		for(int i=0;i<cnt;i++) {
			if( *((unsigned char *)pcb) != 0 ) {
				while(1);
			}
		}
	}
#endif

//	memset(pcb,sizeof(OsTaskVarType),0);
//	pcb->pid = r_pcb->pid;
	assert(r_pcb->prio<=OS_TASK_PRIORITY_MAX);
	pcb->activePriority = r_pcb->prio;
#if	(OS_USE_APPLICATIONS == STD_ON)
//	pcb->accessingApp = Os_CfgGetApplObj(r_pcb->application_id);
#endif
//	pcb->entry = r_pcb->entry;
//	pcb->proc_type = r_pcb->proc_type;
//	pcb->autostart =  r_pcb->autostart;
	pcb->stack= r_pcb->stack;
	pcb->constPtr = r_pcb;
//	pcb->resourceIntPtr = r_pcb->resourceIntPtr;
//	pcb->scheduling = r_pcb->scheduling;
//	pcb->resourceAccess = r_pcb->resourceAccess;
//	pcb->activationLimit = r_pcb->activationLimit;
//	pcb->app = &app_list[r_pcb->app];
//	pcb->app_mask = app_mask[r_pcb->app];
//	strncpy(pcb->name,r_pcb->name,16);
//	pcb->name[15] = '\0';
}

static _Bool init_os_called = 0;

/**
 * Initialization of kernel structures and start of the first
 * task.
 */

void InitOS( void ) {
	int i;
	OsTaskVarType *tmpPcbPtr;
	OsIsrStackType intStack;

	init_os_called = 1;

	DEBUG(DEBUG_LOW,"os_init");

	/* Clear sys */
	memset(&Os_Sys,0,sizeof(Os_SysType));

	Os_ArchInit();

	// Assign pcb list and init ready queue
	Os_Sys.pcb_list = Os_TaskVarList;
	TAILQ_INIT(& Os_Sys.ready_head);
//	TAILQ_INIT(& Os_Sys.pcb_head);
#if defined(USE_KERNEL_EXTRA)
	TAILQ_INIT(& Os_Sys.timerHead);
#endif

	// Calc interrupt stack
	Os_IsrGetStackInfo(&intStack);
	// TODO: 16 is arch dependent
	Os_Sys.intStack = (void *)((size_t)intStack.top + (size_t)intStack.size - 16);

	// Init counter.. with alarms and schedule tables
#if OS_COUNTER_CNT!=0
	Os_CounterInit();
#endif
#if OS_SCHTBL_CNT!=0
	Os_SchTblInit();
#endif

	// Put all tasks in the pcb list
	// Put the one that belong in the ready queue there
	// TODO: we should really hash on priority here to get speed, but I don't care for the moment
	// TODO: Isn't this just EXTENED tasks ???
	for( i=0; i < OS_TASK_CNT; i++) {
		tmpPcbPtr = Os_TaskGet(i);

		copyPcbParts(tmpPcbPtr,&Os_TaskConstList[i]);

#if 1
		Os_TaskContextInit(tmpPcbPtr);
#else
		if( !(tmpPcbPtr->constPtr->proc_type & PROC_ISR) ) {
			Os_TaskContextInit(tmpPcbPtr);
		}
#endif

		TAILQ_INIT(&tmpPcbPtr->resourceHead);

#if 0
		Os_AddTask(tmpPcbPtr);
#endif

		DEBUG(DEBUG_LOW,"pid:%d name:%s prio:%d\n",tmpPcbPtr->pid,tmpPcbPtr->name,tmpPcbPtr->prio);
	}

	Os_ResourceInit();

	// Now all tasks should be created.
}

static void os_start( void ) {
	uint16_t i;
	OsTaskVarType *tmpPcbPtr = NULL;

	// We will be setting up interrupts,
	// but we don't want them to fire just yet
	Irq_Disable();

	assert(init_os_called);

	/* TODO: fix ugly */
	/* Call the startup hook */
	extern struct OsHooks os_conf_global_hooks;
	Os_Sys.hooks = &os_conf_global_hooks;
	if( Os_Sys.hooks->StartupHook!=NULL ) {
		Os_Sys.hooks->StartupHook();
	}


#if	(OS_USE_APPLICATIONS == STD_ON)
	/* Start applications */
	Os_ApplStart();
#endif


	/* Alarm autostart */
#if OS_ALARM_CNT!=0
	Os_AlarmAutostart();
#endif

#if OS_SCHTBL_CNT!=0
	Os_SchTblAutostart();
#endif

	// Set up the systick interrupt
	{
		uint32_t sys_freq = McuE_GetSystemClock();
		Os_SysTickInit();
		Os_SysTickStart(sys_freq/OsTickFreq);
	}

	/* Find highest Autostart task */
	{
		OsTaskVarType *iterPcbPtr;
		OsPriorityType topPrio = -1;

		for(i=0;i<OS_TASK_CNT;i++) {
			iterPcbPtr = Os_TaskGet(i);
			if(	iterPcbPtr->constPtr->autostart ) {
				if( iterPcbPtr->activePriority > topPrio ) {
					tmpPcbPtr = iterPcbPtr;
					topPrio = iterPcbPtr->activePriority;
				}
			}
		}
#if 0
		TAILQ_FOREACH(iterPcbPtr,& Os_Sys.pcb_head,pcb_list) {
			if(	iterPcbPtr->constPtr->autostart ) {
				if( iterPcbPtr->activePriority > topPrio ) {
					tmpPcbPtr = iterPcbPtr;
					topPrio = iterPcbPtr->activePriority;
				}
			}
 		}
#endif
	}

	// Swap in prio proc.
	{
		// FIXME: Do this in a more structured way.. setting Os_Sys.currTaskPtr manually is not the way to go..
		Os_Sys.currTaskPtr = tmpPcbPtr;
#if	(OS_USE_APPLICATIONS == STD_ON)
		/* Set current application */
		Os_Sys.currApplId = tmpPcbPtr->constPtr->applOwnerId;
#endif

		// register this auto-start activation
		assert(tmpPcbPtr->activations < tmpPcbPtr->constPtr->activationLimit);
		tmpPcbPtr->activations++;

		// NOTE! We don't go for os_swap_context() here..
		// first arg(NULL) is dummy only
		Os_TaskSwapContextTo(NULL,tmpPcbPtr);
		// We should not return here
		assert(0);
	}
}
#if 0
static void os_start( void ) {

}
#endif

#define TEST_DATA  12345
int test_data = TEST_DATA;
int test_bss = 0;


void noooo( void ) {
	while(1);
}

extern void EcuM_Init();
int main( void )
{
	EcuM_Init();

}

/**
 * Starts the OS
 *
 * @param Mode - Application mode to start in
 *
 */
void StartOS(AppModeType Mode) {

	/* Check link file */
	if (TEST_DATA != test_data) {
		noooo();
	}

	if (test_bss != 0) {
		noooo();
	}

	Os_Sys.appMode = Mode;

	Os_CfgValidate();

	os_start();

	/** @req OS424 */
	assert(0);
}

/**
 * OS shutdown
 *
 * @param Error - Reason for shutdown
 */

/** @req OS071 */
void ShutdownOS( StatusType Error ) {

	if( Os_Sys.hooks->ShutdownHook != NULL ) {
		Os_Sys.hooks->ShutdownHook(Error);
	}

	Irq_Disable();
	/** @req OS425 */
	while(1) {	}

}



