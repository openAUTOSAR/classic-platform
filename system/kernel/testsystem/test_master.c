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

/* CONFIGURATION
 * - The "etask_master" should be the only autostarted task.
 * - It can hold at most 3 extended task and 3 basic tasks. They must be called
 *   etask_sup_l, etask_sup_m, etask_sup_h,
 *   btask_sup_l, btask_sup_m, btask_sup_h
 */





#include <stdlib.h>
#include "Os.h"
#include "os_test.h"
#include "Mcu.h"
#if defined(USE_GPT)
#include "Gpt.h"
#endif


//#define USE_LDEBUG_PRINTF
#include "debug.h"
#include "arc.h"


typedef struct {
	 uint32 nr;
	 uint32 sub_nr;
	 uint32 failed;
} test_master_cfg_t;

TaskType test_activate_pid_list[] =
{
/* 01*/	TASK_ID_etask_sup_l,
/* 02*/	TASK_ID_etask_sup_l,
/* 03*/	TASK_ID_etask_sup_l,
/* 04*/	TASK_ID_etask_sup_l,
};

static int test_case = 0;

/*
 * Master test process, everything is controlled from here.
 */
void etask_master( void ) {
	TaskType pid;

	for( ; test_case < sizeof(test_activate_pid_list)/sizeof(TaskType); test_case++)
	{
		test_nr = 1;
		printf("-----> Test Suite %02d\n",test_suite);
		pid = test_activate_pid_list[test_case];
		ActivateTask(pid);
		/* We are lowest prio task in the system (apart from idle) so
		 * all tasks in the test are now terminated...
		 */
		test_suite++;
	}

	test_done();

	// Test complete..
	while(1);

}

extern test_func_t etask_sup_matrix[][3];
extern test_func_t btask_sup_matrix[][3];

//--------------------------------------------------------------------
//--------------------------------------------------------------------

void etask_sup_l( void )
{
	test_func_t func;
	func = etask_sup_matrix[test_case][0];
	if( func != NULL )
		func();

	TerminateTask();
}

void etask_sup_m( void )
{
	test_func_t func;
	func = 	etask_sup_matrix[test_case][1];
	if( func != NULL )
		func();

	TerminateTask();
}

void etask_sup_h( void )
{
	test_func_t func;
	func = 	etask_sup_matrix[test_case][2];
	if( func != NULL )
		func();

	TerminateTask();
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------

void btask_sup_l( void ) {
	test_func_t func;
	func = 	btask_sup_matrix[test_case][0];
	if( func != NULL )
		func();
}
void btask_sup_m( void ) {
	test_func_t func;
	func = 	btask_sup_matrix[test_case][1];
	if( func != NULL )
		func();
}
void btask_sup_h( void ) {
	test_func_t func;
	func = 	btask_sup_matrix[test_case][2];
	if( func != NULL )
		func();
}


void OsIdle(void ) {
	for(;;);
}


/* Global hooks */
ProtectionReturnType ProtectionHook( StatusType FatalError ) {
	printf("## ProtectionHook\n");
	return PRO_KILLAPPL;
}

void StartupHook( void ) {
//	LDEBUG_PRINTF("## StartupHook\n");

#ifdef USE_MCU
	uint32_t sys_freq = McuE_GetSystemClock();
	(void)sys_freq;
	LDEBUG_PRINTF("Sys clock %d Hz\n",sys_freq);
#endif
}

void ShutdownHook( StatusType Error ) {
	LDEBUG_PRINTF("## ShutdownHook\n");
	const char *err;
	err = Arc_StatusToString(Error);
	while(1) {
		err = err;
	}
}

void ErrorHook( StatusType Error ) {
	LDEBUG_PRINTF("## ErrorHook err=%d\n",Error);
	const char *err;
	err = Arc_StatusToString(Error);
//	while(1);
}

void PreTaskHook( void ) {
	TaskType task;
	GetTaskID(&task);
	if( task > 10 ) {
		while(1);
	}
	LDEBUG_PRINTF("## PreTaskHook, taskid=%d\n",task);
}

void PostTaskHook( void ) {
	TaskType task;
	GetTaskID(&task);
	if( task > 10 ) {
		while(1);
	}

	LDEBUG_PRINTF("## PostTaskHook, taskid=%d\n",task);
#if 0
	{
		StackInfoType si;
		Os_Arc_GetStackInfo(task,&si);
//		LDEBUG_PRINTF("Stack usage %d%% (this=%08x, top=%08x, size=%08x,usage=%08x )\n",OS_STACK_USAGE(&si),si.curr, si.top,si.size,si.usage);
	}
#endif
}

