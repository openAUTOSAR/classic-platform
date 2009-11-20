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
#include <stdlib.h>
#include "os_test.h"
#include "Mcu.h"
#if defined(USE_GPT)
#include "Gpt.h"
#endif
#include "simple_printf.h"

#define USE_DEBUG
#include "Trace.h"
#include "arc.h"

extern void etask_sup_l_basic_02( void );
extern void etask_sup_m_basic_02( void );
extern void etask_sup_h_basic_02( void );


#define DECLARE_BASIC(_nr) \
	extern void btest_sup_l_##_nr(void);\
	extern void btest_sup_m_##_nr(void);\
	extern void btest_sup_h_##_nr(void);

DECLARE_BASIC(02);

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
		dbg_printf("-----> Test Suite %02d\n",test_suite);
		pid = test_activate_pid_list[test_case];
		ActivateTask(pid);
		// All test tasks are higher prio than we.. so this triggers them..
		Schedule();
		// All tasks in the test are now terminated...
		// Start new tests..
		test_suite++;
	}

	// Test complete..
	while(1);

}

test_func_t etask_sup_matrix[][3] = {
/* 01*/	{ etask_sup_l_01, etask_sup_m_01, etask_sup_h_01},
/* 02*/	{ etask_sup_l_02, etask_sup_m_02, etask_sup_h_02},
/* 03*/	{ etask_sup_l_03, etask_sup_m_03, NULL},
}; // __attribute__ ((section(".data_app_2")));

#define TEST_BASIC(nr) \
	{ btest_sup_l_##nr , btest_sup_m_##nr , btest_sup_h_## nr }

test_func_t btask_sup_matrix[][3] = {
/* 01*/	{ NULL, NULL, NULL},
		TEST_BASIC(02),
#if 0
#endif
}; // __attribute__ ((section(".data_app_2")));


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
	dbg_printf("## ProtectionHook\n");
	return PRO_KILLAPPL;
}

void StartupHook( void ) {
//	dbg_printf("## StartupHook\n");

#ifdef USE_MCU
	uint32_t sys_freq = McuE_GetSystemClock();
	dbg_printf("Sys clock %d Hz\n",sys_freq);
#endif
}

void ShutdownHook( StatusType Error ) {
//	dbg_printf("## ShutdownHook\n");
	const char *err;
	err = Arc_StatusToString(Error);
	while(1) {
		err = err;
	}
}

void ErrorHook( StatusType Error ) {
//	dbg_printf("## ErrorHook err=%d\n",Error);
	const char *err;
	err = Arc_StatusToString(Error);
	while(1);
}

void PreTaskHook( void ) {
	TaskType task;
	GetTaskID(&task);
	if( task > 10 ) {
		while(1);
	}
// 	dbg_printf("## PreTaskHook, taskid=%d\n",task);
}

void PostTaskHook( void ) {
	TaskType task;
	GetTaskID(&task);
	if( task > 10 ) {
		while(1);
	}

//	dbg_printf("## PostTaskHook, taskid=%d\n",task);
	{
		StackInfoType si;
		Os_GetStackInfo(task,&si);
//		dbg_printf("Stack usage %d%% (this=%08x, top=%08x, size=%08x,usage=%08x )\n",OS_STACK_USAGE(&si),si.curr, si.top,si.size,si.usage);
	}
}

