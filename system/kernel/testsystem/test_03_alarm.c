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
 * Tests:
 *
 * OSEK
 * - GetAlarmBase				E_OS_ID
 * - GetAlarm      				E_OS_ID, E_OS_NOFUNC
 * - SetRelAlarm                E_OS_ID, E_OS_STATE, E_OS_VALUE
 * - SetAbsAlarm                E_OS_ID, E_OS_STATE, E_OS_VALUE
 * - CancelAlarm                E_OS_ID, E_OS_NOFUNC
 *
 * Autosar
 * - IncrementCounter			E_OS_ID
 * - GetCounterValue            E_OS_ID
 * - GetElapsedCounterValue     E_OS_ID, E_OS_VALUE
 *
 * More tests here:
 *
 * Limitations:
 */


#include "Os.h"
#include "os_test.h"
#include "arc.h"


void etask_sup_l_03(void) {
	_Bool done = 0;
	StatusType rv;
	AlarmBaseType alarmBase;
	TickType tick;
	TickType tickElapsed;

	int i;

	rv = ActivateTask(TASK_ID_etask_sup_m);
	assert( rv == E_OK );

	while (!done) {
		TEST_RUN();
		switch (test_nr) {
		/*--------------------------------------------------------------------
		 * API tests
		 *--------------------------------------------------------------------
		 */
		case 1:
			/* GetAlarmBase E_OS_ID */
			rv = GetAlarmBase(ALARM_ID_c_sys_activate_btask_h, &alarmBase);
			TEST_ASSERT(rv == E_OK);
			rv = GetAlarmBase(ALARM_ID_ILL, &alarmBase);
			TEST_ASSERT(rv == E_OS_ID);
			test_nr++;
			break;
		case 2:
			/* GetAlarm E_OS_ID and E_OS_NO_FUNC */
			rv = GetAlarm(ALARM_ID_c_sys_activate_btask_h,&tick);
			TEST_ASSERT(rv == E_OS_NOFUNC);
			rv = GetAlarm(ALARM_ID_ILL,&tick);
			TEST_ASSERT(rv == E_OS_ID);
			test_nr++;
			break;
		case 3:
			 /* SetRelAlarm  E_OS_ID, E_OS_STATE, E_OS_VALUE */
			rv = SetRelAlarm(ALARM_ID_ILL,1,10);
			TEST_ASSERT(rv == E_OS_ID);
			rv = SetRelAlarm(ALARM_ID_c_soft_1_setevent_etask_m,0,10);
			TEST_ASSERT(rv == E_OS_VALUE);
			rv = SetRelAlarm(ALARM_ID_c_soft_1_setevent_etask_m,1,10);
			TEST_ASSERT(rv == E_OK);
			rv = SetRelAlarm(ALARM_ID_c_soft_1_setevent_etask_m,1,10);
			TEST_ASSERT(rv == E_OS_STATE);
			rv = CancelAlarm(ALARM_ID_c_soft_1_setevent_etask_m);
			TEST_ASSERT(rv == E_OK);
			test_nr++;
			break;
		case 4:
			 /* SetAbsAlarm  E_OS_ID, E_OS_STATE, E_OS_VALUE */
			rv = SetAbsAlarm(ALARM_ID_ILL,1,10);
			TEST_ASSERT(rv == E_OS_ID);
			rv = SetAbsAlarm(ALARM_ID_c_soft_1_setevent_etask_m,
					OSMAXALLOWEDVALUE_soft_1 + 1, 10);
			TEST_ASSERT(rv == E_OS_VALUE);
			rv = SetAbsAlarm(ALARM_ID_c_soft_1_setevent_etask_m,1,10);
			TEST_ASSERT(rv == E_OK);
			rv = SetAbsAlarm(ALARM_ID_c_soft_1_setevent_etask_m,1,10);
			TEST_ASSERT(rv == E_OS_STATE);
			rv = CancelAlarm(ALARM_ID_c_soft_1_setevent_etask_m);
			TEST_ASSERT(rv == E_OK);
			test_nr++;
			break;
		case 5:
			 /* CancelAlarm  E_OS_ID, E_OS_NOFUNC */
			rv = CancelAlarm(ALARM_ID_ILL);
			TEST_ASSERT(rv == E_OS_ID);
			rv = CancelAlarm(ALARM_ID_c_soft_1_setevent_etask_m);
			TEST_ASSERT(rv == E_OS_NOFUNC);
			test_nr++;
			break;
		case 6:
			 /* IncrementCounter	E_OS_ID */
			rv = IncrementCounter(COUNTER_ID_ILL);
			TEST_ASSERT(rv == E_OS_ID);
			test_nr++;
			break;
		case 7:
			 /* GetCounterValue     E_OS_ID */
			rv = GetCounterValue(COUNTER_ID_ILL,&tick);
			TEST_ASSERT(rv == E_OS_ID);
			test_nr++;
			break;
		case 8:
			 /* GetElapsedCounterValue	E_OS_ID, E_OS_VALUE */
			rv = GetElapsedCounterValue(COUNTER_ID_ILL,&tick,&tick);
			TEST_ASSERT(rv == E_OS_ID);
			test_nr = 10;
			break;

		/*--------------------------------------------------------------------
		 * Functional tests, Counters
		 *--------------------------------------------------------------------
		 * IncrementCounter
		 * GetCounterValue
		 * GetElapsedCounterValue
		 */
		case 10:
			rv = IncrementCounter(COUNTER_ID_soft_1);
			TEST_ASSERT(rv == E_OK);
			rv = GetCounterValue(COUNTER_ID_soft_1,&tick);
			TEST_ASSERT(rv == E_OK);
			TEST_ASSERT(tick == 1);
			rv = GetElapsedCounterValue(	COUNTER_ID_soft_1,
											&tick,
											&tickElapsed);
			TEST_ASSERT(tick == 1);
			TEST_ASSERT(tickElapsed == 0);

			rv = IncrementCounter(COUNTER_ID_soft_1);
			TEST_ASSERT(rv == E_OK);
			rv = GetElapsedCounterValue(	COUNTER_ID_soft_1,
											&tick,
											&tickElapsed);
			TEST_ASSERT(tick == 2);
			TEST_ASSERT(tickElapsed == 1);

			/* Test max value */
			for( i=0; i< OSMAXALLOWEDVALUE_soft_1 - 2; i++ ) {
				rv = IncrementCounter(COUNTER_ID_soft_1);
				TEST_ASSERT(rv == E_OK);
			}

			rv = GetCounterValue(COUNTER_ID_soft_1,&tick);
			TEST_ASSERT(tick == OSMAXALLOWEDVALUE_soft_1);

			tick = 0;
			rv = GetElapsedCounterValue(	COUNTER_ID_soft_1,
											&tick,
											&tickElapsed);

			TEST_ASSERT(tick == OSMAXALLOWEDVALUE_soft_1 );
			TEST_ASSERT(tickElapsed == OSMAXALLOWEDVALUE_soft_1);

			/* Check that wrapping calculation works */
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = GetElapsedCounterValue(	COUNTER_ID_soft_1,
											&tick,
											&tickElapsed);

			TEST_ASSERT(tick == 1);
			TEST_ASSERT(tickElapsed == 2);

			test_nr++;
			break;

		/*--------------------------------------------------------------------
		 * Functional tests, Alarm
		 *--------------------------------------------------------------------
		 * GetAlarmBase
		 * GetAlarm
		 * SetRelAlarm
		 * SetAbsAlarm
		 * CancelAlarm
		 */

		case 11:
			rv = GetAlarmBase(ALARM_ID_c_soft_1_setevent_etask_m, &alarmBase);
			TEST_ASSERT(alarmBase.maxallowedvalue == OSMAXALLOWEDVALUE_soft_1 );
			/* TODO: Fix this in the editor */
			TEST_ASSERT(alarmBase.mincycle == 1 );
			TEST_ASSERT(alarmBase.tickperbase == 1 );
			test_nr++;
			break;

		case 12:
			/* Test single-shot alarm */
			rv = SetRelAlarm(ALARM_ID_c_soft_1_setevent_etask_m,2,0);
			TEST_ASSERT( rv == E_OK );
			rv = GetAlarm(ALARM_ID_c_soft_1_setevent_etask_m,&tick);
			TEST_ASSERT( rv == E_OK );
			TEST_ASSERT(tick == 2 );

			IncrementCounter(COUNTER_ID_soft_1);
			rv = GetAlarm(ALARM_ID_c_soft_1_setevent_etask_m,&tick);
			TEST_ASSERT( rv == E_OK );
			TEST_ASSERT(tick == 1 );

			IncrementCounter(COUNTER_ID_soft_1);
			rv = GetAlarm(ALARM_ID_c_soft_1_setevent_etask_m,&tick);
			TEST_ASSERT( rv == E_OS_NOFUNC );
			test_nr++;
			break;
		case 13:
			/* Test cyclic alarm */
			rv = SetRelAlarm(ALARM_ID_c_soft_1_setevent_etask_m,2,5);
			TEST_ASSERT( rv == E_OK );
			rv = GetAlarm(ALARM_ID_c_soft_1_setevent_etask_m,&tick);
			TEST_ASSERT( rv == E_OK );
			TEST_ASSERT(tick == 2 );

			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = GetAlarm(ALARM_ID_c_soft_1_setevent_etask_m,&tick);
			TEST_ASSERT( rv == E_OK );
			TEST_ASSERT(tick == 1 );

			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = GetAlarm(ALARM_ID_c_soft_1_setevent_etask_m,&tick);
			TEST_ASSERT( rv == E_OK );
			TEST_ASSERT(tick == 5 );

			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = GetAlarm(ALARM_ID_c_soft_1_setevent_etask_m,&tick);
			TEST_ASSERT(tick == 4 );

			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = GetAlarm(ALARM_ID_c_soft_1_setevent_etask_m,&tick);
			TEST_ASSERT(tick == 1 );

			rv = IncrementCounter(COUNTER_ID_soft_1);
			TEST_ASSERT( rv == E_OK );
			rv = GetAlarm(ALARM_ID_c_soft_1_setevent_etask_m,&tick);
			TEST_ASSERT( rv == E_OK );
			TEST_ASSERT(tick == 5 );

			rv = CancelAlarm(ALARM_ID_c_soft_1_setevent_etask_m);
			TEST_ASSERT( rv == E_OK );
			test_nr++;
			break;
		case 14:
			/*
			 * SetAbsAlarm
			 */

			/* Start from 0 */
			rv = GetCounterValue(COUNTER_ID_soft_1,&tick);
			for(i=0;i<OSMAXALLOWEDVALUE_soft_1 - tick + 1 ;i++) {
				IncrementCounter(COUNTER_ID_soft_1);
			}

			rv = GetCounterValue(COUNTER_ID_soft_1,&tick);
			TEST_ASSERT(tick == 0);

			/* Test single-shot alarm */
			rv = SetAbsAlarm(ALARM_ID_c_soft_1_setevent_etask_m,2,0);
			TEST_ASSERT( rv == E_OK );
			rv = GetAlarm(ALARM_ID_c_soft_1_setevent_etask_m,&tick);
			TEST_ASSERT( rv == E_OK );
			TEST_ASSERT(tick == 2 );

			IncrementCounter(COUNTER_ID_soft_1);
			rv = GetAlarm(ALARM_ID_c_soft_1_setevent_etask_m,&tick);
			TEST_ASSERT( rv == E_OK );
			TEST_ASSERT(tick == 1 );

			IncrementCounter(COUNTER_ID_soft_1);
			rv = GetAlarm(ALARM_ID_c_soft_1_setevent_etask_m,&tick);
			TEST_ASSERT( rv == E_OS_NOFUNC );
			test_nr++;
			break;
		case 15:

			/* Start from 1 */
			rv = GetCounterValue(COUNTER_ID_soft_1,&tick);
			for(i=0;i<OSMAXALLOWEDVALUE_soft_1 - tick + 1 ;i++) {
				IncrementCounter(COUNTER_ID_soft_1);
			}
			IncrementCounter(COUNTER_ID_soft_1);

			/* Test cyclic alarm */
			rv = SetAbsAlarm(ALARM_ID_c_soft_1_setevent_etask_m,3,5);
			TEST_ASSERT( rv == E_OK );
			rv = GetAlarm(ALARM_ID_c_soft_1_setevent_etask_m,&tick);
			TEST_ASSERT( rv == E_OK );
			TEST_ASSERT(tick == 2 );

			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = GetAlarm(ALARM_ID_c_soft_1_setevent_etask_m,&tick);
			TEST_ASSERT( rv == E_OK );
			TEST_ASSERT(tick == 1 );

			/* Trigger the alarm */
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = GetAlarm(ALARM_ID_c_soft_1_setevent_etask_m,&tick);
			TEST_ASSERT( rv == E_OK );
			/* The abs alarm is now a relative alarm cith cycle */
			TEST_ASSERT(tick == 5 );

			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = GetAlarm(ALARM_ID_c_soft_1_setevent_etask_m,&tick);
			TEST_ASSERT(tick == 4 );

			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = GetAlarm(ALARM_ID_c_soft_1_setevent_etask_m,&tick);
			TEST_ASSERT(tick == 1 );

			rv = IncrementCounter(COUNTER_ID_soft_1);
			TEST_ASSERT( rv == E_OK );
			rv = GetAlarm(ALARM_ID_c_soft_1_setevent_etask_m,&tick);
			TEST_ASSERT( rv == E_OK );
			TEST_ASSERT(tick == 5 );

			rv = CancelAlarm(ALARM_ID_c_soft_1_setevent_etask_m);
			TEST_ASSERT( rv == E_OK );
			test_nr++;
			break;
		case 16:
			test_nr = 100;
			break;
		case 100:
			SetEvent(TASK_ID_etask_sup_m, EVENT_MASK_kill);
			TerminateTask();
			break;
		}
	}
}

void etask_sup_m_03(void) {

	for(;;) {

		WaitEvent(EVENT_MASK_notif | EVENT_MASK_kill );
		switch (test_nr) {
		case 12:
		case 13:
		case 14:
		case 15:
			ClearEvent(EVENT_MASK_notif);
			break;
		case 100:
			TerminateTask();
			break;
		default:
			assert(0);
		}
	}

}

void btask_sup_l_03(void) {
}


DECLARE_TEST_ETASK(03, etask_sup_l_03, etask_sup_m_03, NULL );
DECLARE_TEST_BTASK(03, btask_sup_l_03, NULL, NULL );
