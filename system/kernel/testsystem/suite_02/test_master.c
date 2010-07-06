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
 *   Autostart bananza..
 *   1. Autostart of tasks
 *   2. Autostart of alarms
 *   3. Autostart of scheduletables.
 */





#include <stdlib.h>
#include "Os.h"
#include "test_framework.h"
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


static _Bool started[OS_TASK_CNT];

static TaskType prioList[OS_TASK_CNT] = {
		TASK_ID_etask_master,
		TASK_ID_etask_5,
		TASK_ID_btask_4,
		TASK_ID_etask_3,
		TASK_ID_btask_2,
		TASK_ID_OsIdle };

static void setStarted( void ) {
	TaskType taskId;
	GetTaskID(&taskId);
	for(int i=0;i<OS_TASK_CNT;i++) {
		if( prioList[i] == taskId ) {
			started[i] ^= 1;
			break;
		}
	}
}

static _Bool checkStarted( void ) {
	TaskType taskId;
	_Bool after = 0;

	GetTaskID(&taskId);

	for(int i=0;i<OS_TASK_CNT;i++) {

		if( after) {
			if( started[i] == 1 ) {
				return 0;
			}
		} else {
			if( started[i] == 0 ) {
				return 0;
			}
		}

		// Should be less priority until we hit it.
		if( prioList[i] == taskId ) {
			after = 1;
		}

	}
	return 1;
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
	StatusType rv;
	uint32_t mask;
	TEST_INIT();

	TEST_START("Autostart, priority tasks",test_nr);
	setStarted();
	TEST_ASSERT(checkStarted());
	WaitEvent(EVENT_MASK_Event1);
	ClearEvent(EVENT_MASK_Event1);

	TEST_NEXT("Autostart, Alarms and Scheduletables",++test_nr);


	/* Autostart
	 * SetRelAlarm 				Increment: Must != 0
	 * SetAbsAlarm 				No limit
	 * StartScheduleTableAbs
	 * ..
	 * ...
	 *
	 */


	rv = IncrementCounter(COUNTER_ID_Counter2);

#if 0
	validateErrorHook(	0,					/* backlog */
							E_OS_LIMIT,			/* error */
							OSServiceId_ActivateTask,	/* Service Id */
							COUNTER_ID_Counter2, /* param1 */
							TEST_VALUE_NC, 	/* param2 */
							TEST_VALUE_NC, 	/* param3 */
							TEST_VALUE_NC,		/* API id */
							TEST_VALUE_NC );	/* Module id */
#endif

	validateErrorHook(	0,0,0,0,					/* backlog */
							TEST_VALUE_NC, 	/* param2 */
							TEST_VALUE_NC, 	/* param3 */
							TEST_VALUE_NC,		/* API id */
							TEST_VALUE_NC );	/* Module id */

	mask = EVENT_MASK_Alarm_1 | EVENT_MASK_Alarm_2 | EVENT_MASK_SchTbl_1 | EVENT_MASK_SchTbl_2;
	WaitEvent( mask );

	TaskType currTask;
	EventMaskType evMask;

	GetTaskID(&currTask);
	GetEvent(currTask,&evMask);
	TEST_ASSERT( evMask == mask);

	TestExit(0);
}

//--------------------------------------------------------------------

void etask_3( void )
{
	setStarted();
	TEST_ASSERT(checkStarted());
	TerminateTask();
}

void etask_5( void )
{
	setStarted();
	TEST_ASSERT(checkStarted());
	TerminateTask();
}


//--------------------------------------------------------------------

void btask_2( void ) {
	setStarted();
	TEST_ASSERT(checkStarted());
	TerminateTask();
}

void btask_4( void ) {
	setStarted();
	TEST_ASSERT(checkStarted());
	TerminateTask();
}

void OsIdle(void ) {
	setStarted();
	TEST_ASSERT(checkStarted());
	SetEvent(TASK_ID_etask_master, EVENT_MASK_Event1);
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

