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
 * Tested: scheduletables
 */

#include "Os.h"
#include "debug.h"
//#include <stdio.h>
#include <assert.h>
#include "os_test.h"


//static int test = 1;

/*
 * Table 0: period 10
 * 5 - Activate task sup_m
 * 7 - Setevent, EVENT_2 in sup_m
 *
 * Table 1: period 5
 * 2 - activate task_sup_m
 */

void etask_sup_l_03( void ) {
	ScheduleTableStatusType status;
	test_nr = 1;
	for(;;) {
		switch(test_nr){

		case 1:

			GetScheduleTableStatus(SCHTBL_ID_0,&status);
			TEST_ASSERT(status==SCHEDULETABLE_STOPPED);

			// Start a schedule with expire points at 2 and 5
			StartScheduleTableRel(SCHTBL_ID_0,SCHEDULETABLE_DURATION_1*2/10);
			GetScheduleTableStatus(SCHTBL_ID_0,&status);
			TEST_ASSERT(status==SCHEDULETABLE_RUNNING);

			IncrementCounter(COUNTER_ID_soft_2);
			// This one should trigger SCHEDULE_ACTION_ACTIVATETASK, TASK_SUP_M
			IncrementCounter(COUNTER_ID_soft_2);

			TEST_ASSERT(test_nr==2);
			break;

		case 2:
			// test 02 ===============================================
			// back from trigger, go to next expire point (5 more)
			IncrementCounter(COUNTER_ID_soft_2);	// 3
			IncrementCounter(COUNTER_ID_soft_2);
			IncrementCounter(COUNTER_ID_soft_2);
			IncrementCounter(COUNTER_ID_soft_2);	// 6
			// Trigger the SCHEDULE_ACTION_SETEVENT, TASK_SUP_M
			IncrementCounter(COUNTER_ID_soft_2);	// 7
			// Schedule to check if the is an event in TASK_SUP_M queue
			Schedule();

			TEST_ASSERT(test_nr==3);

			break;
		case 3:

			// test 03 ===============================================
			// back from trigger, period is 10 so another 2 should start all over again.
			IncrementCounter(COUNTER_ID_soft_2);	// 8
			IncrementCounter(COUNTER_ID_soft_2);	// 9
			IncrementCounter(COUNTER_ID_soft_2);	// 10

			// Trigger SCHEDULE_ACTION_ACTIVATETASK, TASK_SUP_M
			Schedule();

			TEST_ASSERT(test_nr==4);

		case 4:

			// Go with the next schedule table, while 0 is running
			// (The current table must complete first)
			NextScheduleTable(SCHTBL_ID_0, SCHTBL_ID_1);

			GetScheduleTableStatus(SCHTBL_ID_1,&status);
			TEST_ASSERT(status==SCHEDULETABLE_NEXT);

			IncrementCounter(COUNTER_ID_soft_2); // 1
			IncrementCounter(COUNTER_ID_soft_2); // 2
			IncrementCounter(COUNTER_ID_soft_2); // 3
			IncrementCounter(COUNTER_ID_soft_2); // 4
			IncrementCounter(COUNTER_ID_soft_2); // 5, ActivateTask

			IncrementCounter(COUNTER_ID_soft_2); // 6
			IncrementCounter(COUNTER_ID_soft_2); // 7, SetEvent
			IncrementCounter(COUNTER_ID_soft_2); // 8
			IncrementCounter(COUNTER_ID_soft_2); // 9
			IncrementCounter(COUNTER_ID_soft_2); // 10

			GetScheduleTableStatus(SCHTBL_ID_0,&status);
			TEST_ASSERT(status==SCHEDULETABLE_STOPPED);
			GetScheduleTableStatus(SCHTBL_ID_1,&status);
			TEST_ASSERT(status==SCHEDULETABLE_RUNNING);

			IncrementCounter(COUNTER_ID_soft_2); // 0 On table 1


//			StartScheduleTableRel(SCHTBL_ID_0,SCHEDULETABLE_DURATION_1*3/10);

			// Done
			break;
		default:
			assert(0);
			while(1);
		}
	}
}

void etask_sup_m_03( void ) {
	for(;;) {
		switch(test_nr) {
		case 1:
			TEST_OK();
			test_nr++;
			break;
		case 2:
			// back to TASK_SUP_L
			WaitEvent(EVENT_2);
			TEST_OK()
			test_nr++;
			break;
		case 3:
			// we have started again
			TEST_OK();
			test_nr++;
			break;
		case 4:
			break;
		default:
			assert(0);
			break;
		}
	}
}
