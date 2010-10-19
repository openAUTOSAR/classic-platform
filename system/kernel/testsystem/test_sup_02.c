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
 * Tested: alarms and events
 *
 * COUNTER_ID_soft_1 drives alarms:
 *   ALARM_ID_c_soft_1_setevent_etask_m
 *   ALARM_ID_c_soft_1_inc_counter_2
 */

#include "Os.h"
#include "debug.h"
#include <assert.h>
#include "test_framework.h"


void etask_sup_l_02( void ) {

	for(;;) {
		switch(test_nr){
		case 1:
			ChainTask(TASK_ID_btask_sup_l);
			break;
		case 2:
			/*
			 * Check GetAlarmBase()
			 */
			TEST_OK();
			{
				AlarmBaseType alarm;
				TickType tick;
				GetAlarmBase(ALARM_ID_c_soft_1_setevent_etask_m,&alarm);
				LDEBUG_PRINTF("Alarm %d,%d,%d\n",	alarm.maxallowedvalue,
												alarm.tickperbase,
												alarm.mincycle);

				/* Get ticks before alarm expires */
				GetAlarm(TASK_ID_btask_sup_l,&tick);
				test_nr = 3;
				break;
			}
		case 3:
			// Make cyclic alarm
			SetRelAlarm(ALARM_ID_c_soft_1_setevent_etask_m, 2, 5);
			// Let sup_m wait for the event.
			ActivateTask(TASK_ID_etask_sup_m);
			IncrementCounter(COUNTER_ID_soft_1);    // 1
			IncrementCounter(COUNTER_ID_soft_1); 	// 2
			// Should trigger trigger after here, since higher prio it should swap
			break;
		case 4:
			IncrementCounter(COUNTER_ID_soft_1);	// 3
			IncrementCounter(COUNTER_ID_soft_1);	// 4
			IncrementCounter(COUNTER_ID_soft_1);	// 5
			IncrementCounter(COUNTER_ID_soft_1);	// 1
			Schedule();
			IncrementCounter(COUNTER_ID_soft_1);	// 2
			Schedule();
			CancelAlarm(ALARM_ID_c_soft_1_setevent_etask_m );
			test_nr = 100;
			break;
		case 100:
			TerminateTask();
			break;
		default:
			assert(0);
			while(1);
		}
	}
//SetAbsAlarm(TASK_ID_btask_sup_l,1000,0);

	while(1);
}

void etask_sup_m_02( void ) {
	while(1) {
		switch(test_nr) {
		case 3:
			WaitEvent(EVENT_1);
			ClearEvent(EVENT_1);
			TEST_OK();
			test_nr = 4;
			break;
		case 4:
			// Back to SUP_L
			WaitEvent(EVENT_1);
			TEST_OK();
			test_nr++;
			TerminateTask();
			break;
		default:
			assert(0);
			while(1);
		}
	}
}

void etask_sup_h_02( void ) {
	while(1);
}


#if 0
void test_sup_m_02( void ) {
	switch(test){
	case 1:
		TEST_OK();
		test_nr++;
 		ChainTask(SUP_L_BASIC);
		break;
	case 3:
		TEST_OK();
	default:
		break;
	}
}
#endif

void btest_sup_l_02( void ) {
	switch(test_nr){
	case 1:
		TEST_OK();
		test_nr++;
		ChainTask(TASK_ID_etask_sup_l);
		break;
	default:
		while(1);
	}

}

void btest_sup_m_02( void ) {
}

void btest_sup_h_02( void ) {
}


