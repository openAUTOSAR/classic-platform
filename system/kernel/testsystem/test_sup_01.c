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

/*
 * Tested: Tasks
 */

//#include "Platform_Types.h"
#include "Os.h"
#include "debug.h"
#include "test_framework.h"
//#include "test_cfg.h"

#define EVENT_NR	1
#define TASK_1		1
#define TASK_2		2

#define E_TEST_OUT_OF_SYNC		100


//#define EVENT_1		1
//#define EVENT_2		2

DeclareResource(RES_1);

//static int test = 1;

#define TASK_ID_ILL			99
#define RES_ID_ILL			99
#define ALARM_ID_ILL		99

static void taskApiTests( void ) {
	StatusType 		rv;
	TaskStateType 	taskState;
	EventMaskType 	eventMask;
	AlarmBaseType 	alarmInfo;
	TickType 		tick;

	/*@req E_OS_ID */
	rv = ActivateTask(TASK_ID_ILL);
	TEST_ASSERT(rv==E_OS_ID );
	rv = ChainTask(TASK_ID_ILL);
	TEST_ASSERT(rv==E_OS_ID );

	rv = GetTaskState(TASK_ID_ILL,&taskState);
	TEST_ASSERT(rv==E_OS_ID );

	rv = GetResource(RES_ID_ILL);
	TEST_ASSERT(rv==E_OS_ID );
	rv = ReleaseResource(RES_ID_ILL);
	TEST_ASSERT(rv==E_OS_ID );

	rv = SetEvent(TASK_ID_ILL,1);
	TEST_ASSERT(rv==E_OS_ID );
	rv = GetEvent(TASK_ID_ILL,&eventMask);
	TEST_ASSERT(rv==E_OS_ID );

	rv = GetAlarmBase(ALARM_ID_ILL,&alarmInfo);
	TEST_ASSERT(rv==E_OS_ID );
	rv = GetAlarm(ALARM_ID_ILL,&tick);
	TEST_ASSERT(rv==E_OS_ID );
	rv = SetRelAlarm(ALARM_ID_ILL,1,2);
	TEST_ASSERT(rv==E_OS_ID );
	rv = SetAbsAlarm(ALARM_ID_ILL,1,2);
	TEST_ASSERT(rv==E_OS_ID );
	rv = CancelAlarm(ALARM_ID_ILL);
	TEST_ASSERT(rv==E_OS_ID );
}

void etask_sup_l_01( void ){

	_Bool done = 0;

	while(!done) {
		switch( test_nr ) {
		case 1:
			/*@req OSEK_TASK_1 */
			ActivateTask(TASK_ID_etask_sup_m);
			/* Switch to higher prio */
			Schedule();
			break;
		case 4:
			// From WaitEvent() in sup_m
			TEST_OK();
			++test_nr;
			ClearEvent(EVENT_1);
			// trigger sup_m to ready state(since it's waiting for the event)
			SetEvent(TASK_ID_etask_sup_m,EVENT_1);
			// Let SUP_M take the event
#warning Something is very wrong here...
			WaitEvent(EVENT_1);
			break;
		case 7:
			// We get scheduled again from sup_m
			TEST_OK();
			test_nr = 100;
			break;

		case 100:
			SetEvent(TASK_ID_etask_sup_m,EVENT_1);
			// Let SUP_M finish...
			// Schedule();
			// The test case is done! Kill ourselves
			TerminateTask();
//			done = 1;
			break;
		default:
			TEST_FAIL("default in switch\n");
//			ErrorHook(E_TEST_OUT_OF_SYNC);
			break;
		}
	}
	// Cleanup


}

void etask_sup_m_01( void ){
	_Bool done = 0;

	while( !done ) {
		switch( test_nr ) {
		case	1:
			TEST_OK();
			++test_nr;
			break;
		case 	2:
			Schedule();
			// We should stay here
			TEST_OK();
			++test_nr;
			break;
		case	3:
			SetEvent(TASK_ID_etask_sup_l,EVENT_1);
			// We should stay here since we are higher prio
			TEST_OK();
			++test_nr;
			break;
		case 	4:
			// No event waiting so this should trigger sup_l
			WaitEvent(EVENT_1);
			break;
		case 5:
			SetEvent(TASK_ID_etask_sup_l,EVENT_1);
			TEST_OK();
			++test_nr;
			break;
		case 6:
			// The event isn't cleared yet, so we should return right away
			WaitEvent(EVENT_1);
			TEST_OK();
			++test_nr;
			break;
		case 7:
			ClearEvent(EVENT_1);
			// Switch to sup_l
			WaitEvent(EVENT_1);
			break;
		case 100:
			TerminateTask();

#if 0
			ClearEvent(1);
			done = 1;
#endif
			break;
		default:
			TEST_FAIL("default in switch");
			ErrorHook(E_TEST_OUT_OF_SYNC);
		}
	}
}

void etask_sup_h_01( void ){

}

#if 0
void test_sup_master( void ) {

	for(;;) {
 os_printf("test1_1,msr=%x\n",get_msr());

		GetResource(RES_1);
		ReleaseResource(RES_1);
		SetEvent(TASK_2,EVENT_NR);
		WaitEvent(EVENT_NR);
		ClearEvent(EVENT_NR);
	}
}

void test1_2( void ) {
	for(;;) {
 os_printf("test1_2,msr=%x\n",get_msr());
		WaitEvent(EVENT_NR);
		ClearEvent(EVENT_NR);
		SetEvent(TASK_1,EVENT_NR);
	}
}
#endif

#if 0
void idle_proc( void )
{
	for(;;) {}
}
#endif


