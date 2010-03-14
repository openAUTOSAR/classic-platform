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
 * From Autosar 3.1
 *
 * Tests:
 * - StartScheduleTableRel          E_OS_ID, E_OS_VALUE, E_OS_STATE
 * - StartScheduleTableAbs          E_OS_ID, E_OS_VALUE, E_OS_STATE
 * - StopScheduleTable				E_OS_ID, E_OS_NO_FUNC
 * - NextScheduleTable				E_OS_ID, E_OS_NO_FUNC, E_OS_STATE
 * - GetScheduleTableStatus			E_OS_ID,
 *
 *   Class 2 and 4
 * - StartScheduleTableSynchrone    E_OS_ID, E_OS_STATE
 * - SyncScheduleTable              E_OS_ID, E_OS_VALUE, E_OS_STATE
 * - SetScheduleTableAsync			E_OS_ID,
 *
 * More tests here:
 *
 * Limitations:
 */

#include "Os.h"
#include "os_test.h"
#include "arc.h"


static 	int subTest = 0;

void etask_sup_l_04(void) {
	_Bool done = 0;
	StatusType rv;
	ScheduleTableStatusType status;

	while (!done) {
		TEST_RUN();
		switch (test_nr) {
		case 1:
			/* StartScheduleTableRel E_OS_ID, E_OS_VALUE, E_OS_STATE */
			rv = StartScheduleTableRel(SCHEDULE_TABLE_ID_ILL,1);
			TEST_ASSERT(rv==E_OS_ID);
			rv = StartScheduleTableRel(SCHEDULE_TABLE_0,0);
			TEST_ASSERT(rv==E_OS_VALUE);
			rv = StartScheduleTableRel(SCHEDULE_TABLE_0,1);
			TEST_ASSERT(rv==E_OK);
			rv = StartScheduleTableRel(SCHEDULE_TABLE_0,1);
			TEST_ASSERT(rv==E_OS_STATE);
			rv = StopScheduleTable(SCHEDULE_TABLE_0);
			test_nr++;
			break;
		case 2:
			/* StartScheduleTableAbs E_OS_ID, E_OS_VALUE, E_OS_STATE */
			rv = StartScheduleTableAbs(SCHEDULE_TABLE_ID_ILL,1);
			TEST_ASSERT(rv==E_OS_ID);
#if 0
			/* TODO:Must supply an offset bigger than OsCounterMaxAllowedValue */
			StartScheduleTableAbs(SCHEDULE_TABLE_0,0);
			TEST_ASSERT(rv==E_OS_VALUE);
#endif
			rv = StartScheduleTableAbs(SCHEDULE_TABLE_0,1);
			TEST_ASSERT(rv==E_OK);
			rv = StartScheduleTableAbs(SCHEDULE_TABLE_0,1);
			TEST_ASSERT(rv==E_OS_STATE);
			rv = StopScheduleTable(SCHEDULE_TABLE_0);
			test_nr++;
			break;
		case 3:
			/* StopScheduleTable E_OS_ID, E_OS_NO_FUNC */
			rv = StopScheduleTable(SCHEDULE_TABLE_ID_ILL);
			TEST_ASSERT(rv==E_OS_ID);
			rv = StopScheduleTable(SCHEDULE_TABLE_0);
			TEST_ASSERT(rv==E_OS_NOFUNC);
			test_nr++;
			break;
		case 4:
			/* NextScheduleTable	E_OS_ID, E_OS_NO_FUNC, E_OS_STATE */
			rv = NextScheduleTable(SCHEDULE_TABLE_ID_ILL, SCHEDULE_TABLE_ID_ILL);
			TEST_ASSERT(rv==E_OS_ID);
			rv = NextScheduleTable(SCHEDULE_TABLE_ID_ILL, SCHEDULE_TABLE_0);
			TEST_ASSERT(rv==E_OS_ID);
			rv = NextScheduleTable(SCHEDULE_TABLE_0, SCHEDULE_TABLE_ID_ILL);
			TEST_ASSERT(rv==E_OS_ID);

			rv = NextScheduleTable(SCHEDULE_TABLE_0,SCHEDULE_TABLE_0);
			TEST_ASSERT(rv==E_OS_NOFUNC);

			rv = StartScheduleTableRel(SCHEDULE_TABLE_0,1);
			rv = StartScheduleTableRel(SCHEDULE_TABLE_1,1);
			rv = NextScheduleTable(SCHEDULE_TABLE_0,SCHEDULE_TABLE_1);
			TEST_ASSERT(rv==E_OS_STATE);

			/* Cleanup */
			rv = StopScheduleTable(SCHEDULE_TABLE_0);
			TEST_ASSERT(rv==E_OK);
			rv = StopScheduleTable(SCHEDULE_TABLE_1);
			TEST_ASSERT(rv==E_OK);
			test_nr = 10;
			break;
		/*--------------------------------------------------------------------
		 * Functional tests, ScheduleTables
		 *--------------------------------------------------------------------
		 * - StartScheduleTableRel
		 * - StartScheduleTableAbs
		 * - StopScheduleTable
		 * - NextScheduleTable
		 * - GetScheduleTableStatus
		 */
		case 10:
			subTest = 0;
			rv = ActivateTask(TASK_ID_etask_sup_m);
			TEST_ASSERT( rv == E_OK );

			rv = GetScheduleTableStatus(SCHEDULE_TABLE_0,&status);
			TEST_ASSERT(rv==E_OK);
			TEST_ASSERT( status == SCHEDULETABLE_STOPPED );
			rv = StartScheduleTableRel(SCHEDULE_TABLE_0,2);
			TEST_ASSERT(rv==E_OK);

			rv = GetScheduleTableStatus(SCHEDULE_TABLE_0,&status);
			TEST_ASSERT(rv==E_OK);
			TEST_ASSERT( status == SCHEDULETABLE_RUNNING );

			/* 2+ 5 */
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			subTest = 5;
			rv = IncrementCounter(COUNTER_ID_soft_1);

			/* 2 more for next*/
			rv = IncrementCounter(COUNTER_ID_soft_1);
			subTest = 7;
			rv = IncrementCounter(COUNTER_ID_soft_1);

			/* 4 more for next*/
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			subTest = 11;
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = GetScheduleTableStatus(SCHEDULE_TABLE_0,&status);
			TEST_ASSERT(rv==E_OK);
			TEST_ASSERT( status == SCHEDULETABLE_RUNNING );

			/* and the final offset */
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = GetScheduleTableStatus(SCHEDULE_TABLE_0,&status);
			TEST_ASSERT(rv==E_OK);
			TEST_ASSERT( status == SCHEDULETABLE_RUNNING );

			/* First offset, again. */
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);

			/* TODO: More here */

			test_nr = 100;
			break;


#if 0 /* Working single shot case */
		case 10:
			subTest = 0;
			rv = ActivateTask(TASK_ID_etask_sup_m);
			TEST_ASSERT( rv == E_OK );

			rv = GetScheduleTableStatus(SCHEDULE_TABLE_0,&status);
			TEST_ASSERT(rv==E_OK);
			TEST_ASSERT( status == SCHEDULETABLE_STOPPED );
			rv = StartScheduleTableRel(SCHEDULE_TABLE_0,2);
			TEST_ASSERT(rv==E_OK);

			rv = GetScheduleTableStatus(SCHEDULE_TABLE_0,&status);
			TEST_ASSERT(rv==E_OK);
			TEST_ASSERT( status == SCHEDULETABLE_RUNNING );

			/* 2+ 5 */
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			subTest = 5;
			rv = IncrementCounter(COUNTER_ID_soft_1);

			/* 2 more for next*/
			rv = IncrementCounter(COUNTER_ID_soft_1);
			subTest = 7;
			rv = IncrementCounter(COUNTER_ID_soft_1);

			/* 4 more for next*/
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			subTest = 11;
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = GetScheduleTableStatus(SCHEDULE_TABLE_0,&status);
			TEST_ASSERT(rv==E_OK);
			TEST_ASSERT( status == SCHEDULETABLE_RUNNING );

			/* and the final offset */
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = IncrementCounter(COUNTER_ID_soft_1);
			rv = GetScheduleTableStatus(SCHEDULE_TABLE_0,&status);
			TEST_ASSERT(rv==E_OK);
			TEST_ASSERT( status == SCHEDULETABLE_STOPPED );
			test_nr = 100;

			break;
#endif


		case 100:
			rv = SetEvent(TASK_ID_etask_sup_m, EVENT_KILL);
			TEST_ASSERT( rv == E_OK );
			TerminateTask();
			break;
		}
	}
}

void etask_sup_m_04(void) {

	TaskType currTask;
	EventMaskType eventMask;
	GetTaskID(&currTask);
	StatusType rv;

	for(;;) {
		WaitEvent(EVENT_1 | EVENT_KILL );
		switch(test_nr) {
		case 10:
			switch(subTest) {
			case 7:
			case 11:
				printf("etask\n");
				rv = GetEvent(currTask,&eventMask);
				TEST_ASSERT( rv == E_OK );
				TEST_ASSERT(eventMask == EVENT_1 );
				ClearEvent(EVENT_1);
				break;

			default:
				TEST_ASSERT(0);
				break;

			}
			break;
		case 100:
			rv = TerminateTask();
			TEST_ASSERT( rv == E_OK );
			break;

		default:
			TEST_ASSERT(0);
			break;
		}
	}
}


void btask_sup_l_04(void) {



}

void btask_sup_m_04(void) {

	switch(test_nr) {
	case 10:
		switch(subTest) {
		case 5:
		case 11:
			printf("btask\n");
			break;

		default:
			TEST_ASSERT(0);
			break;
		}

		break;
	}

}


DECLARE_TEST_ETASK(04, etask_sup_l_04, etask_sup_m_04, NULL );
DECLARE_TEST_BTASK(04, btask_sup_l_04, btask_sup_m_04, NULL );


