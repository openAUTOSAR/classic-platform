
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


#include <stdint.h>
#include "Os.h"
#include <assert.h>


#include "test_framework.h"
#if defined( USE_MCU )
#include "Mcu.h"
#endif
#include "arc.h"

//#define USE_LDEBUG_PRINTF	1
#include "debug.h"

#define ERROR_LOG_SIZE 1

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

//	LDEBUG_PRINTF("## ErrorHook err=%u\n",error);

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
	StatusType rv;
	TaskType task;
	TaskStateType state;

	rv = GetTaskID(&task);
	assert( rv == E_OK );
	LDEBUG_PRINTF("## PreTaskHook, taskid=%d\n",task);
	rv = GetTaskState(task,&state);
	assert( rv == E_OK );
	assert( state == TASK_STATE_RUNNING );
}

void PostTaskHook( void ) {
	StatusType rv;
	TaskType task;
	TaskStateType state;

	rv = GetTaskID(&task);
	assert( rv == E_OK );
	LDEBUG_PRINTF("## PostTaskHook, taskid=%d\n",task);
	rv = GetTaskState(task,&state);
	assert( rv == E_OK );
	assert( state == TASK_STATE_RUNNING );

#if 0
	{
		StackInfoType si;
		Os_Arc_GetStackInfo(task,&si);
//		LDEBUG_PRINTF("Stack usage %d%% (this=%08x, top=%08x, size=%08x,usage=%08x )\n",OS_STACK_USAGE(&si),si.curr, si.top,si.size,si.usage);
	}
#endif
}

