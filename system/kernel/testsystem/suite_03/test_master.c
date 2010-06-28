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

/* Tests
 *    Scheduling tests FULL/NONE using SetEvent(), ActivateTask() and ChainTask()
 *    TODO: ChainTask()
 *    TODO: Should we add the GetResource(RES_SCHEDULER) here also?
 *
 *    SetEvent()
 *      etask_m_full: SetEvent() to  etask_l_full, etask_h_full
 *      etask_m_full: SetEvent() to  etask_l_none, etask_h_none
 *      etask_m_none: SetEvent() to  etask_l_full, etask_h_full
 *      etask_m_none: SetEvent() to  etask_l_none, etask_h_none
 *
 *      btask_m_full: SetEvent() to  etask_l_full, etask_h_full
 *      btask_m_full: SetEvent() to  etask_l_none, etask_h_none
 *      btask_m_none: SetEvent() to  etask_l_full, etask_h_full
 *      btask_m_none: SetEvent() to  etask_l_none, etask_h_none
 *
 *    ActivateTask()
 *      etask_m_full: ActivateTask() to  etask_l_full, etask_h_full
 *      etask_m_full: ActivateTask() to  etask_l_none, etask_h_none
 *      etask_m_none: ActivateTask() to  etask_l_full, etask_h_full
 *      etask_m_none: ActivateTask() to  etask_l_none, etask_h_none
 *
 *      etask_m_full: ActivateTask() to  btask_l_full, btask_h_full
 *      etask_m_full: ActivateTask() to  btask_l_none, btask_h_none
 *      etask_m_none: ActivateTask() to  btask_l_full, btask_h_full
 *      etask_m_none: ActivateTask() to  btask_l_none, btask_h_none
 *
 *      btask_m_full: ActivateTask() to  btask_l_full, etask_h_full
 *      btask_m_full: ActivateTask() to  btask_l_none, etask_h_none
 *      btask_m_none: ActivateTask() to  btask_l_full, etask_h_full
 *      btask_m_none: ActivateTask() to  btask_l_none, etask_h_none
 *
 *      btask_m_full: ActivateTask() to  btask_l_full, etask_h_full
 *      btask_m_full: ActivateTask() to  btask_l_none, etask_h_none
 *      btask_m_none: ActivateTask() to  btask_l_full, etask_h_full
 *      btask_m_none: ActivateTask() to  btask_l_none, etask_h_none
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
#include "test_framework.h"


#define ERROR_LOG_SIZE	1

typedef struct ErrorEntry {
	StatusType 		error;
	OsErrorType     info;
	TaskType    	taskId;
	OsServiceIdType serviceId;
} ErrorEntryType;


typedef struct ErrorLog {
	int 			index;
	ErrorEntryType 	log[ERROR_LOG_SIZE];
} ErrorLogType;

ErrorLogType ErrorLog;

ErrorEntryType *errorLogGetEntry( int backlog ) {


	int index = ErrorLog.index - backlog;

	if( index < 0 ) {
		index = ERROR_LOG_SIZE + index;
	}
	return &ErrorLog.log[index];
}


void validateErrorHook(int backlog, int error, int serviceId,
							uint32_t param1, uint32_t param2, uint32_t param3,
							int apiId, int modId ) {
	ErrorEntryType *entry = errorLogGetEntry(backlog);
	TEST_ASSERT( error == entry->error );
	if(param1 != TEST_VALUE_NC ) {
		TEST_ASSERT(param1 == entry->info.param1 );
	}
	if(param2 != TEST_VALUE_NC ) {
		TEST_ASSERT(param2 == entry->info.param2 );
	}
	if(param2 != TEST_VALUE_NC ) {
		TEST_ASSERT(param3 == entry->info.param3 );
	}
}

#define TEST_VALIDATE_ERROR_HOOK( _backlog,_error,_service_id,_param1, \
								   _param2,_param3,_api_id,_mod_id)     \
do { 																	\
	ErrorEntryType *entry = errorLogGetEntry(_backlog);  			\
	TEST_ASSERT(_error != entry->error );							\
	if(_param1 != TEST_VALUE_NC ) {									\
		TEST_ASSERT(_param1 == entry->info.param1 );				\
	}																\
	if(_param2 != TEST_VALUE_NC ) {									\
		TEST_ASSERT(_param2 == entry->info.param2 );				\
	}																\
	if(_param2 != TEST_VALUE_NC ) {									\
		TEST_ASSERT(_param3 == entry->info.param3 );				\
	}																\
} while(0)

/*
 * Master test process, everything is controlled from here.
 */
void etask_master( void ) {
	TEST_INIT();
	test_nr = 1;

	ActivateTask(TASK_ID_etask_m_full);
	ActivateTask(TASK_ID_etask_m_none);

	testExit(0);
}

//--------------------------------------------------------------------

// Tasks
void btask_h_full( void ) {

	TerminateTask();
}

void btask_h_none( void ) {

	TerminateTask();
}

void btask_l_full( void ) {

	TerminateTask();
}

void btask_l_none( void ) {

	TerminateTask();
}

void btask_m_full( void ) {

	TerminateTask();
}

void btask_m_none( void ) {

	TerminateTask();
}

void etask_h_full( void ) {
	for(;;) {
		WaitEvent(EVENT_MASK_test);
		ClearEvent(EVENT_MASK_test);
		switch(test_nr) {
		case 2:
			SetEvent(TASK_ID_etask_m_full,EVENT_MASK_test);
			break;
		case 100:
			TerminateTask();
		default:
			TEST_ASSERT(0);
			break;
		}

	}
}

void etask_h_none( void ) {


}

void etask_l_full( void ) {
	for(;;) {
		WaitEvent(EVENT_MASK_test);
		ClearEvent(EVENT_MASK_test);

		switch(test_nr) {
		case 1:
			SetEvent(TASK_ID_etask_m_full,EVENT_MASK_test);
			break;
		case 100:
			TerminateTask();
		default:
			TEST_ASSERT(0);
			break;
		}

	}
}

void etask_l_none( void ) {


}

void etask_m_full( void ) {
	EventMaskType mask;

	ActivateTask(TASK_ID_etask_l_full);
	ActivateTask(TASK_ID_etask_h_full);

	for(;;) {
		switch(test_nr) {
		case 1:
			TEST_START("SetEvent() to low",test_nr);
			/** req OS?? */
			GetEvent(TASK_ID_etask_l_full,&mask);
			TEST_ASSERT( !(mask & EVENT_MASK_test) );
			/* After SetEvent(), NO dispatch */
			SetEvent(TASK_ID_etask_l_full, EVENT_MASK_test);
			GetEvent(TASK_ID_etask_l_full,&mask);
			TEST_ASSERT( mask & EVENT_MASK_test )

			/* Let the Low prio task run */
			WaitEvent(EVENT_MASK_test);
			ClearEvent(EVENT_MASK_test);
			TEST_END();
			test_nr++;
			break;
		case 2:
			TEST_START("SetEvent() to high",test_nr);
			/** req OS?? */
			GetEvent(TASK_ID_etask_h_full,&mask);
			TEST_ASSERT( !(mask & EVENT_MASK_test) );
			/* After SetEvent(), dispatch */
			SetEvent(TASK_ID_etask_h_full, EVENT_MASK_test);
			GetEvent(TASK_ID_etask_h_full,&mask);
			TEST_ASSERT( !(mask & EVENT_MASK_test) );
			TEST_END();
			test_nr = 100;
			break;
		case 100:
			/* Kill tasks */
			SetEvent(TASK_ID_etask_l_full, EVENT_MASK_test);
			SetEvent(TASK_ID_etask_h_full, EVENT_MASK_test);
			TerminateTask();
		default:
			TEST_ASSERT(0);
			break;
		}

	}
}

void etask_m_none( void ) {


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


void ErrorHook( StatusType error ) {

	TaskType task;
	ErrorEntryType *errEntry;

	GetTaskID(&task);

	OsServiceIdType service = OSErrorGetServiceId();

	/* Grab the arguments to the functions
	 * This is the standard way, see 11.2 in OSEK spec
	 */
#if 0
	switch(service) {
	case OSServiceId_SetRelAlarm:
	{
		// Read the arguments to the faulty functions...
		AlarmType alarm_id = OSError_SetRelAlarm_AlarmId;
		TickType increment = OSError_SetRelAlarm_Increment;
		TickType cycle = OSError_SetRelAlarm_Cycle;
		(void)alarm_id;
		(void)increment;
		(void)cycle;

		// ... Handle this some way.
		break;
	case OSServiceId_ActivateTask:


		break;
	}
	/*
	 * The same pattern as above applies for all other OS functions.
	 * See Os.h for names and definitions.
	 */

	default:
		break;
	}
#endif

	LDEBUG_PRINTF("## ErrorHook err=%u\n",Error);

	/* Log the errors in a buffer for later review */
	errEntry = &ErrorLog.log[ErrorLog.index];

	errEntry->info.param1 = os_error.param1;
	errEntry->info.param2 = os_error.param2;
	errEntry->info.param3 = os_error.param3;
	errEntry->info.serviceId = service;
	errEntry->taskId = task;
	errEntry->error = error;
	ErrorLog.index = (ErrorLog.index + 1) % ERROR_LOG_SIZE ;
}

#if 0
void ErrorHook( StatusType Error ) {

	LDEBUG_PRINTF("## ErrorHook err=%d\n",Error);
	const char *err;
	err = Arc_StatusToString(Error);
//	while(1);
}
#endif

void PreTaskHook( void ) {
	TaskType task;
	GetTaskID(&task);
	LDEBUG_PRINTF("## PreTaskHook, taskid=%d\n",task);
}

void PostTaskHook( void ) {
	TaskType task;
	GetTaskID(&task);
	LDEBUG_PRINTF("## PostTaskHook, taskid=%d\n",task);
#if 0
	{
		StackInfoType si;
		Os_Arc_GetStackInfo(task,&si);
//		LDEBUG_PRINTF("Stack usage %d%% (this=%08x, top=%08x, size=%08x,usage=%08x )\n",OS_STACK_USAGE(&si),si.curr, si.top,si.size,si.usage);
	}
#endif
}

