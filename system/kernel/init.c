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
#include "task_i.h"
#include "sys.h"
#include "isr.h"
#include "counter_i.h"
#include "application.h"
#include "sched_table_i.h"
#include "alarm_i.h"
#include "arch.h"

/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/
#define OS_VALIDATE(_a,_b)   if((_a)!=(_b) ) { \
								assert(#_a  #_b); \
							  }
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
Os_SysType Os_Sys;

Os_IntCounterType Os_IntDisableAllCnt;
Os_IntCounterType Os_IntSuspendAllCnt;
Os_IntCounterType Os_IntSuspendOsCnt;

OsErrorType os_error;

/* ----------------------------[private functions]---------------------------*/


static void Os_CfgValidate(void ) {
#if (OS_COUNTER_CNT!=0)
	OS_VALIDATE(OS_COUNTER_CNT,ARRAY_SIZE(counter_list));
#endif
#if (OS_RESOURCE_CNT!=0)
	OS_VALIDATE(OS_RESOURCE_CNT,ARRAY_SIZE(resource_list));
#endif
	OS_VALIDATE(OS_TASK_CNT ,ARRAY_SIZE( Os_TaskConstList));
#if (OS_ALARM_CNT!=0)
	OS_VALIDATE(OS_ALARM_CNT,ARRAY_SIZE(alarm_list));
#endif
#if (OS_SCHTBL_CNT!=0)
	OS_VALIDATE(OS_SCHTBL_CNT, ARRAY_SIZE(sched_list));
#endif
}

/* ----------------------------[public functions]----------------------------*/

extern uint32_t McuE_GetSystemClock( void );


/**
 * Copy rom pcb data(r_pcb) to ram data
 *
 * @param 	pcb		ram data
 * @param 	r_pcb	rom data
 */

static void copyPcbParts( OsTaskVarType *pcb, const OsTaskConstType *r_pcb ) {
	assert(r_pcb->prio<=OS_TASK_PRIORITY_MAX);
	pcb->activePriority = r_pcb->prio;
	pcb->stack= r_pcb->stack;
	pcb->constPtr = r_pcb;
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

	Os_CfgValidate();

	DEBUG(DEBUG_LOW,"os_init");

	/* Clear sys */
	memset(&Os_Sys,0,sizeof(Os_SysType));

	Os_ArchInit();

	/* Get the numbers defined in the editor */
	Os_Sys.isrCnt = OS_ISR_CNT;

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
		Os_TaskContextInit(tmpPcbPtr);
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

#define TEST_DATA  0x12345
#define TEST_SDATA2	0x3344
volatile uint32_t test_data = TEST_DATA;
volatile uint32_t test_bss = 0;
volatile uint32_t test_bss_array[3];
volatile uint32_t test_data_array[3] = {TEST_DATA,TEST_DATA,TEST_DATA};

/* Define if compiler is set to use small data section */
/* #define CC_USE_SMALL_DATA */

//#define CC_USE_SMALL_DATA 1

#if defined(CFG_PPC) && defined(__CWCC__)
/* Note! It does not matter if the data is initialized to 0,
 * it still sbss2.
 */
#if defined(CFG_PPC) && defined(__CWCC__)
volatile const int test_sbss2;
#endif
/* Initialized small data */
volatile const int test_sdata2 = TEST_SDATA2;

#endif


#define BAD_LINK_FILE() 	while(1) {}

extern void EcuM_Init(void);

#if defined(__CWCC__)
extern char __SDATA2[];
extern char __SDATA2_RAM[];
extern char __SDATA2_END[];
extern char __SBSS2_START[];
extern char __SBSS2_END[];
#endif

#if defined(__DCC__)
extern void __init( void );
#endif

int main( void )
{
	/* TODO: Move to arch specific part */
#if defined(CFG_PPC) && defined(__CWCC__)
	memcpy(__SDATA2_RAM, __SDATA2, __SDATA2_END - __SDATA2_RAM );
	memset(__SBSS2_START,0,  __SBSS2_END -__SBSS2_START );
#endif

	/* Check link file */

	/* .data */
	for(int i=0;i<3;i++ ) {
		if( test_data_array[i] != TEST_DATA) {
			BAD_LINK_FILE();
		}
	}

	/* .sdata */
	if (TEST_DATA != test_data) {
		BAD_LINK_FILE();
	}

	/* .bss */
	for(int i=0;i<3;i++ ) {
		if( test_bss_array[i] != 0) {
			BAD_LINK_FILE();
		}
	}

	/* .sbss */
	if (test_bss != 0) {
		BAD_LINK_FILE();
	}

#if defined(CFG_PPC) && defined(__CWCC__)
	/* check .sdata2 */
	if (test_sdata2 != TEST_SDATA2) {
		BAD_LINK_FILE();
	}
#endif

#if defined(CFG_PPC) && defined(__CWCC__)
	/* check .sbss */
	if (test_sbss2 != 0) {
		BAD_LINK_FILE();
	}
#endif

#if defined(__DCC__)
	/* Runtime init */
	__init();
#endif

	EcuM_Init();

}

/**
 * Starts the OS
 *
 * @param Mode - Application mode to start in
 *
 */
void StartOS(AppModeType Mode) {

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



