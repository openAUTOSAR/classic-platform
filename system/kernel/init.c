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


#include <stdlib.h>
#include <string.h>
#include "Os.h"
#include "internal.h"
#include "arc.h"
#include "debug.h"
#include "arch.h"

extern void Os_CfgGetInterruptStackInfo( OsStackType *stack );
extern uint32_t McuE_GetSystemClock( void );
extern OsTickType OsTickFreq;

sys_t os_sys;

Os_IntCounterType Os_IntDisableAllCnt;
Os_IntCounterType Os_IntSuspendAllCnt;
Os_IntCounterType Os_IntSuspendOsCnt;


/**
 * Copy rom pcb data(r_pcb) to ram data
 *
 * @param 	pcb		ram data
 * @param 	r_pcb	rom data
 */

static void os_pcb_rom_copy( OsPcbType *pcb, const OsRomPcbType *r_pcb ) {

#if 0 //?????
	// Check to that the memory is ok
	{
		int cnt = sizeof(OsPcbType);
		for(int i=0;i<cnt;i++) {
			if( *((unsigned char *)pcb) != 0 ) {
				while(1);
			}
		}
	}
#endif

//	memset(pcb,sizeof(OsPcbType),0);
	pcb->pid = r_pcb->pid;
	pcb->prio = r_pcb->prio;
#if ( OS_SC3 == STD_ON ) || ( OS_SC4 == STD_ON )
	pcb->application = Os_CfgGetApplObj(r_pcb->application_id);
#endif
	pcb->entry = r_pcb->entry;
	pcb->proc_type = r_pcb->proc_type;
	pcb->autostart =  r_pcb->autostart;
	pcb->stack= r_pcb->stack;
	pcb->pcb_rom_p = r_pcb;
	pcb->resource_int_p = r_pcb->resource_int_p;
	pcb->scheduling = r_pcb->scheduling;
	pcb->resourceAccess = r_pcb->resourceAccess;
	pcb->activationLimit = r_pcb->activationLimit;
//	pcb->app = &app_list[r_pcb->app];
//	pcb->app_mask = app_mask[r_pcb->app];
	strncpy(pcb->name,r_pcb->name,16);
}

static _Bool init_os_called = 0;

/**
 * Initialization of kernel structures and start of the first
 * task.
 */

void InitOS( void ) {
	int i;
	OsPcbType *tmp_pcb;
	OsStackType int_stack;

	init_os_called = 1;

	DEBUG(DEBUG_LOW,"os_init");

	/* Clear sys */
	memset(&os_sys,0,sizeof(sys_t));

	Os_ArchInit();

	// Assign pcb list and init ready queue
	os_sys.pcb_list = pcb_list;
	TAILQ_INIT(& os_sys.ready_head);
	TAILQ_INIT(& os_sys.pcb_head);

	// Calc interrupt stack
	Os_CfgGetInterruptStackInfo(&int_stack);
	// TODO: 16 is arch dependent
	os_sys.int_stack = int_stack.top + int_stack.size - 16;

	// Init counter.. with alarms and schedule tables
	Os_CounterInit();
	Os_SchTblInit();

	// Put all tasks in the pcb list
	// Put the one that belong in the ready queue there
	// TODO: we should really hash on priority here to get speed, but I don't care for the moment
	// TODO: Isn't this just EXTENED tasks ???
	for( i=0; i < Os_CfgGetTaskCnt(); i++) {
		tmp_pcb = os_get_pcb(i);

		assert(tmp_pcb->prio<=OS_TASK_PRIORITY_MAX);

		os_pcb_rom_copy(tmp_pcb,os_get_rom_pcb(i));
		if( !(tmp_pcb->proc_type & PROC_ISR) ) {
			Os_ContextInit(tmp_pcb);
		}

		TAILQ_INIT(&tmp_pcb->resource_head);

		Os_AddTask(tmp_pcb);

		DEBUG(DEBUG_LOW,"pid:%d name:%s prio:%d\n",tmp_pcb->pid,tmp_pcb->name,tmp_pcb->prio);
	}

	Os_ResourceInit();

	// Now all tasks should be created.
}

static void os_start( void ) {
	OsPcbType *tmp_pcb;

	// We will be setting up interrupts,
	// but we don't want them to fire just yet
	Irq_Disable();

	assert(init_os_called);

	/* TODO: fix ugly */
	/* Call the startup hook */
	extern struct OsHooks os_conf_global_hooks;
	os_sys.hooks = &os_conf_global_hooks;
	if( os_sys.hooks->StartupHook!=NULL ) {
		os_sys.hooks->StartupHook();
	}

	/* handle autostart */
	for(int j=0; j < Os_CfgGetAlarmCnt(); j++ ) {
		OsAlarmType *alarmPtr;
		alarmPtr = Os_CfgGetAlarmObj(j);
		if(alarmPtr->autostartPtr != NULL ) {
			const OsAlarmAutostartType *autoPtr = alarmPtr->autostartPtr;

			if( autoPtr->autostartType == ALARM_AUTOSTART_ABSOLUTE ) {
				SetAbsAlarm(j,autoPtr->alarmTime, autoPtr->cycleTime);
			} else {
				SetRelAlarm(j,autoPtr->alarmTime, autoPtr->cycleTime);
			}
		}
	}

	// Set up the systick interrupt
	{
		uint32_t sys_freq = McuE_GetSystemClock();
		Os_SysTickInit();
		Os_SysTickStart(sys_freq/OsTickFreq);
	}

	/* Find highest Autostart task */
	{
		OsPcbType *iterPcbPtr;
		OsPriorityType topPrio = -1;

		TAILQ_FOREACH(iterPcbPtr,& os_sys.pcb_head,pcb_list) {
			if(	iterPcbPtr->autostart ) {
				if( iterPcbPtr->prio > topPrio ) {
					tmp_pcb = iterPcbPtr;
				}
			}
 		}
	}

	// Swap in prio proc.
	{
		// FIXME: Do this in a more structured way.. setting os_sys.curr_pcb manually is not the way to go..
		os_sys.curr_pcb = tmp_pcb;
		// NOTE! We don't go for os_swap_context() here..
		// first arg(NULL) is dummy only
		Os_TaskSwapContextTo(NULL,tmp_pcb);
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

	if( os_sys.hooks->ShutdownHook != NULL ) {
		os_sys.hooks->ShutdownHook(Error);
	}

	Irq_Disable();
	/** @req OS425 */
	while(1) {	}

}



