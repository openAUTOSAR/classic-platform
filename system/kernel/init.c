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
#include "Trace.h"
#include "arch.h"

extern void Oil_GetInterruptStackInfo( OsStackType *stack );
extern uint32_t McuE_GetSystemClock( void );
extern OsTickType OsTickFreq;

sys_t os_sys;

Os_IntCounterType Os_IntDisableAllCnt;
Os_IntCounterType Os_IntSuspendAllCnt;
Os_IntCounterType Os_IntSuspendOsCnt;

/**
 * Initialize alarms and schedule-tables for the counters
 */
static void os_counter_init( void ) {
	OsCounterType *counter;
	OsAlarmType *alarm_obj;
	OsSchTblType *sched_obj;
	/* Create a list from the counter to the alarms */
	for(int i=0; i < Oil_GetCounterCnt() ; i++) {
		counter = Oil_GetCounter(i);
		// Alarms
		SLIST_INIT(&counter->alarm_head);
		for(int j=0; j < Oil_GetAlarmCnt(); j++ ) {
			alarm_obj = Oil_GetAlarmObj(j);
			// Add the alarms
			SLIST_INSERT_HEAD(&counter->alarm_head,alarm_obj, alarm_list);
		}
		// Schedule tables
		SLIST_INIT(&counter->sched_head);
		for(int j=0; j < Oil_GetSchedCnt(); j++ ) {
			sched_obj = Oil_GetSched(j);
			// Add the alarms
			SLIST_INSERT_HEAD(&counter->sched_head,
								sched_obj,
								sched_list);
		}


	}
}

/**
 *
 * @param pcb_p
 * @return
 */
static void os_resource_init( void ) {
	//TAILQ_INIT(&pcb_p->resource_head);
	OsPcbType *pcb_p;
	OsResourceType *rsrc_p;
	int topPrio;

	/* Calculate ceiling priority
	 * We make this as simple as possible. The ceiling priority
	 * is set to the same priority as the highest priority task that
	 * access it.
	 * */
	for( int i=0; i < Oil_GetResourceCnt(); i++) {
		rsrc_p = Oil_GetResource(i);
		topPrio = 0;

		for( int pi; pi < Oil_GetTaskCnt(); pi++) {

			pcb_p = os_get_pcb(pi);
			if(pcb_p->resourceAccess & (1<<i) ) {
				topPrio = MAX(topPrio,pcb_p->prio);
			}
		}
		rsrc_p->ceiling_priority = topPrio;
	}

	/* Assign an internal resource with prio 32 to the tasks
	 * with scheduling=NON
	 */
	for( int i; i < Oil_GetTaskCnt(); i++) {
		pcb_p = os_get_pcb(i);
		if(pcb_p->scheduling == NON ) {
			pcb_p->prio = OS_RES_SCHEDULER_PRIO;
		}
	}
}


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
#if ( OS_SC1 == STD_ON ) || ( OS_SC4 == STD_ON )
	pcb->application = Oil_GetApplObj(r_pcb->application_id);
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

	os_arch_init();

	// Assign pcb list and init ready queue
	os_sys.pcb_list = pcb_list;
	TAILQ_INIT(& os_sys.ready_head);
	TAILQ_INIT(& os_sys.pcb_head);

	// Calc interrupt stack
	Oil_GetInterruptStackInfo(&int_stack);
	os_sys.int_stack = int_stack.top + int_stack.size - 16;		// TODO: 16 is arch dependent

	// Init counter.. with alarms and schedule tables
	os_counter_init();
	Os_SchTblInit();
	os_resource_init();

	// Put all tasks in the pcb list
	// Put the one that belong in the ready queue there
	// TODO: we should really hash on priority here to get speed, but I don't care for the moment
	// TODO: Isn't this just EXTENED tasks ???
	for( i=0; i < Oil_GetTaskCnt(); i++) {
		tmp_pcb = os_get_pcb(i);

		assert(tmp_pcb->prio<=OS_TASK_PRIORITY_MAX);

		os_pcb_rom_copy(tmp_pcb,os_get_rom_pcb(i));
		if( !(tmp_pcb->proc_type & PROC_ISR) ) {
			Os_ContextInit(tmp_pcb);
		}

		Os_AddTask(tmp_pcb);

		DEBUG(DEBUG_LOW,"pid:%d name:%s prio:%d\n",tmp_pcb->pid,tmp_pcb->name,tmp_pcb->prio);
	}

	// Now all tasks should be created.
}

static void os_start( void ) {
	OsPcbType *tmp_pcb;

	assert(init_os_called);

	/* TODO: fix ugly */
	/* Call the startup hook */
	extern struct OsHooks os_conf_global_hooks;
	os_sys.hooks = &os_conf_global_hooks;
	if( os_sys.hooks->StartupHook!=NULL ) {
		os_sys.hooks->StartupHook();
	}

	/* handle autostart */
	for(int j=0; j < Oil_GetAlarmCnt(); j++ ) {
		OsAlarmType *alarmPtr;
		alarmPtr = Oil_GetAlarmObj(j);
		if(alarmPtr->autostartPtr != NULL ) {
			OsAlarmAutostartType *autoPtr = alarmPtr->autostartPtr;

			if( autoPtr->autostartType == ALARM_AUTOSTART_ABSOLUTE ) {
				SetAbsAlarm(j,autoPtr->alarmTime, autoPtr->cycleTime);
			} else {
				SetRelAlarm(j,autoPtr->alarmTime, autoPtr->cycleTime);
			}
		}
	}

	// Activate the systick interrupt
	{
		uint32_t sys_freq = McuE_GetSystemClock();
		Os_SysTickInit();
		Os_SysTickStart(sys_freq/OsTickFreq);
	}

	/* Find highest Autostart task */
	{
		OsPcbType *iterPcbPtr;
		OsPriorityType topPrio = 0;

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



