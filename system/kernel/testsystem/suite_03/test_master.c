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
 *    1 etask_m_full: SetEvent() to  etask_l_full, etask_h_full
 *    1 etask_m_full: SetEvent() to  etask_l_non, etask_h_non
 *    4 etask_m_non: SetEvent() to  etask_l_full, etask_h_full
 *    4 etask_m_non: SetEvent() to  etask_l_non, etask_h_non
 *
 *      btask_m_full: SetEvent() to  etask_l_full, etask_h_full
 *      btask_m_full: SetEvent() to  etask_l_non, etask_h_non
 *      btask_m_non: SetEvent() to  etask_l_full, etask_h_full
 *      btask_m_non: SetEvent() to  etask_l_non, etask_h_non
 *
 *    ActivateTask()
 *    2 etask_m_full: ActivateTask() to  etask_l_full, etask_h_full
 *    2 etask_m_full: ActivateTask() to  etask_l_non, etask_h_non
 *      etask_m_non: ActivateTask() to  etask_l_full, etask_h_full
 *      etask_m_non: ActivateTask() to  etask_l_non, etask_h_non
 *
 *    3 etask_m_full: ActivateTask() to  btask_l_full, btask_h_full
 *    3 etask_m_full: ActivateTask() to  btask_l_non, btask_h_non
 *      etask_m_non: ActivateTask() to  btask_l_full, btask_h_full
 *      etask_m_non: ActivateTask() to  btask_l_non, btask_h_non
 *
 *      btask_m_full: ActivateTask() to  btask_l_full, etask_h_full
 *      btask_m_full: ActivateTask() to  btask_l_non, etask_h_non
 *      btask_m_non: ActivateTask() to  btask_l_full, etask_h_full
 *      btask_m_non: ActivateTask() to  btask_l_non, etask_h_non
 *
 *      btask_m_full: ActivateTask() to  btask_l_full, etask_h_full
 *      btask_m_full: ActivateTask() to  btask_l_non, etask_h_non
 *      btask_m_non: ActivateTask() to  btask_l_full, etask_h_full
 *      btask_m_non: ActivateTask() to  btask_l_non, etask_h_non
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


#define ERROR_LOG_SIZE	1

#define NOT_EXPECTED	0
#define EXPECTED 		1
#define TASK_BASIC		0
#define TASK_EXT		1



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

#define MAKE_TEST(_x)		TEST_ ## (_x)
#define TEST_INC()			test_nr++
//#define TEST_START2()		printf("%s\n",TestFixure[test_nr-1].description)
#define TEST_START2()		TestStart(TestFixure[test_nr-1].description, TestFixure[test_nr-1].nr );
#define TEST_GETCASE()		TestFixure[test_nr-1].nr


enum testCase {
	TEST_1 = 1,
	TEST_2,
	TEST_3,
	TEST_4,
	TEST_5,
	TEST_6,
	TEST_7,
	TEST_8,
	TEST_9,
	TEST_10,
	TEST_11,
	TEST_12,
	TEST_13,
	TEST_14,
	TEST_15,
	TEST_16,
	TEST_17,
	TEST_18,
	TEST_19,
	TEST_20,
	TEST_LAST,
};



static TestFixtureType TestFixure[] = {
/* 0 */
		{"SetEvent()/E/FULL to self",TEST_1},
		{"ActivateTask()/E/FULL to self",TEST_2},

/* 1 */
		{"SetEvent()/E/FULL to E/Lo/FULL",TEST_3},
		{"SetEvent()/E/FULL to E/Hi/FULL",TEST_4},
		{"SetEvent()/E/FULL to E/Lo/NON",TEST_5},
		{"SetEvent()/E/FULL to E/Hi/NON",TEST_6},

/* 2 */
		{"ActivateTask()/E/FULL to E/Lo/FULL",TEST_7},
		{"ActivateTask()/E/FULL to E/Hi/FULL",TEST_8},
		{"ActivateTask()/E/FULL to E/Lo/NON",TEST_9},
		{"ActivateTask()/E/FULL to E/Hi/NON",TEST_10},

/* 3 */
		{"ActivateTask()/E/FULL to B/Lo/FULL",TEST_11},
		{"ActivateTask()/E/FULL to B/Hi/FULL",TEST_12},
		{"ActivateTask()/E/FULL to B/Lo/NON",TEST_13},
		{"ActivateTask()/E/FULL to B/Hi/NON",TEST_14},

		{"SetEvent()/E/NON to self",TEST_15},
		{"ActivateTask()/E/NON to self",TEST_16},

/* 4 */
		{"SetEvent()/E/NON to E/Lo/FULL",TEST_17},
		{"SetEvent()/E/NON to E/Hi/FULL",TEST_18},
		{"SetEvent()/E/NON to E/Lo/NON",TEST_19},
		{"SetEvent()/E/NON to E/Hi/NON",TEST_20},

		{"",TEST_LAST},
};


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

	/* Activate ext. task "slaves" */
	ActivateTask(TASK_ID_etask_l_full);
	ActivateTask(TASK_ID_etask_h_full);
	ActivateTask(TASK_ID_etask_l_non);
	ActivateTask(TASK_ID_etask_h_non);

	/* Do the m_full -> XX tests first */
	ActivateTask(TASK_ID_etask_m_full);

	/* Do the next test */
	WaitEvent(EVENT_MASK_next);
	ClearEvent(EVENT_MASK_next);

	/* Do the m_non -> XX  */
	ActivateTask(TASK_ID_etask_m_non);


	TestExit(0);
}

//--------------------------------------------------------------------

// Tasks
void btask_h_full( void ) {

	TerminateTask();
}

void btask_h_non( void ) {

	TerminateTask();
}

void btask_l_full( void ) {

	TerminateTask();
}

void btask_l_non( void ) {

	TerminateTask();
}

void btask_m_full( void ) {

	TerminateTask();
}

void btask_m_non( void ) {

	TerminateTask();
}


static void waitKillAndTest( void ) {
	TaskType currTask;
	EventMaskType mask;
	GetTaskID(&currTask);

	WaitEvent( EVENT_MASK_test | EVENT_MASK_kill);
	GetEvent(currTask,&mask);
	if( EVENT_MASK_kill & mask ) {
		TerminateTask();
	}
	ClearEvent(EVENT_MASK_test);
}

void etask_h_full( void ) {
	for(;;) {
		waitKillAndTest();
		switch(test_nr) {
		case TEST_4:
		case TEST_8:
			SetEvent(TASK_ID_etask_m_full,EVENT_MASK_test);
			break;
		default:
			TEST_ASSERT(0);
			break;
		}

	}
}

void etask_h_non( void ) {
	for(;;) {
		waitKillAndTest();
		switch(test_nr) {
		case TEST_6:
		case TEST_9:
			SetEvent(TASK_ID_etask_m_full,EVENT_MASK_test);
			break;
		default:
			TEST_ASSERT(0);
			break;
		}
	}
}

void etask_l_full( void ) {
	for(;;) {
		waitKillAndTest();
		switch(test_nr) {
		case TEST_3:
		case TEST_7:
			SetEvent(TASK_ID_etask_m_full,EVENT_MASK_test);
			break;
		default:
			TEST_ASSERT(0);
			break;
		}

	}
}

void etask_l_non( void ) {
	for(;;) {
		waitKillAndTest();
		switch(test_nr) {
		case TEST_5:
		case TEST_9:
			SetEvent(TASK_ID_etask_m_full,EVENT_MASK_test);
			break;
		default:
			TEST_ASSERT(0);
			break;
		}

	}
}


static void taskDispatchCheck( TaskType task,_Bool expected, _Bool etask ) {
	EventMaskType mask;
	StatusType rv;
	TaskType currTask;
	TaskStateType taskState;

	GetTaskID(&currTask);

	/* 1. Kill the task */
	TEST_ASSERT(taskState == TASK_STATE_WAITING);
	rv = SetEvent(task,EVENT_MASK_kill);
	TEST_ASSERT( rv == E_OK );

	/* 2. Grab task state */
	GetTaskState(task, &taskState);
	TEST_ASSERT(taskState == TASK_STATE_SUSPENDED);

	/* 3. Activate */
	ActivateTask(task);

	GetEvent(currTask,&mask);
	if( expected ) {
		TEST_ASSERT( mask & EVENT_MASK_test );
	} else {
		TEST_ASSERT( (mask & EVENT_MASK_test) == 0 );
	}

	/* Let task task run */
	WaitEvent(EVENT_MASK_test);
	ClearEvent(EVENT_MASK_test);
}


static void eventExpectNoDispatch( TaskType task ) {
	EventMaskType mask;
	StatusType rv;
	/** req OS?? */
	rv = GetEvent(task,&mask);
	TEST_ASSERT(rv == E_OK);
	TEST_ASSERT( !(mask & EVENT_MASK_test) );
	/* After SetEvent(), NO dispatch */
	SetEvent(task, EVENT_MASK_test);
	GetEvent(task,&mask);
	TEST_ASSERT( mask & EVENT_MASK_test );

	/* Let the Low prio task run */
	WaitEvent(EVENT_MASK_test);
	ClearEvent(EVENT_MASK_test);
}

static void eventExpectDispatch( TaskType task ) {
	EventMaskType mask;
	StatusType rv;
	/** req OS?? */
	rv = GetEvent(task,&mask);
	TEST_ASSERT(rv == E_OK);
	TEST_ASSERT( !(mask & EVENT_MASK_test) );
	/* After SetEvent(), dispatch */
	SetEvent(task, EVENT_MASK_test);
	GetEvent(task,&mask);
	TEST_ASSERT( !(mask & EVENT_MASK_test) );
}



void etask_m_full( void ) {
	StatusType	rv;
	TaskType task;
	EventMaskType mask;
	static int m_full_starts = 0;
	_Bool kill = 0;

	(void)rv;
	m_full_starts++;
	GetTaskID(&task);

	for(;;) {
		if( kill == 1) {
			/* Kill tasks */
#if 0
			SetEvent(TASK_ID_etask_l_full, EVENT_MASK_kill);
			SetEvent(TASK_ID_etask_h_full, EVENT_MASK_kill);
			SetEvent(TASK_ID_etask_l_non, EVENT_MASK_kill);
			SetEvent(TASK_ID_etask_h_non, EVENT_MASK_kill);
#endif
			SetEvent(TASK_ID_etask_master, EVENT_MASK_next);
			TerminateTask();
		}

		TEST_START2();
		switch(TEST_GETCASE()) {
		case TEST_1:
			GetTaskID(&task);
			GetEvent(task,&mask);
			TEST_ASSERT( (mask & EVENT_MASK_test) == 0 );
			TEST_ASSERT( m_full_starts == 1 );
			SetEvent(task,EVENT_MASK_test);
			GetEvent(task,&mask);
			TEST_ASSERT( (mask & EVENT_MASK_test) );
			TEST_ASSERT( m_full_starts == 1 );
			ClearEvent(EVENT_MASK_test);
			break;
		case TEST_2:
			rv = ActivateTask(task);
			TEST_ASSERT( rv == E_OS_LIMIT);
			break;
		case TEST_3:
			eventExpectNoDispatch(TASK_ID_etask_l_full);
			break;
		case TEST_4:
			eventExpectDispatch(TASK_ID_etask_h_full);
			break;
		case TEST_5:
			eventExpectNoDispatch(TASK_ID_etask_l_non);
			break;
		case TEST_6:
			eventExpectDispatch(TASK_ID_etask_h_non);
			break;
		case TEST_7:
			taskDispatchCheck(TASK_ID_etask_l_full, EXPECTED, TASK_EXT );
			break;
		case TEST_8:
			taskDispatchCheck(TASK_ID_etask_h_full, NOT_EXPECTED, TASK_EXT );
			break;
		case TEST_9:
			taskDispatchCheck(TASK_ID_etask_l_non, EXPECTED, TASK_EXT );
			break;
		case TEST_10:
			taskDispatchCheck(TASK_ID_etask_h_full, NOT_EXPECTED, TASK_EXT );
			break;
		case TEST_11:
			taskDispatchCheck(TASK_ID_btask_l_full, EXPECTED, TASK_BASIC );
			break;
		case TEST_12:
			taskDispatchCheck(TASK_ID_btask_h_full, NOT_EXPECTED, TASK_BASIC );
			break;
		case TEST_13:
			taskDispatchCheck(TASK_ID_btask_l_non, EXPECTED, TASK_BASIC );
			break;
		case TEST_14:
			taskDispatchCheck(TASK_ID_btask_h_full, NOT_EXPECTED, TASK_BASIC );
			kill = 1;
			break;
		default:
			TEST_ASSERT(0);
			break;
		}
		TEST_END();
		TEST_INC();
	}
}

void etask_m_non( void ) {


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

