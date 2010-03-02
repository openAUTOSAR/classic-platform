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
 * Tests tasks:
 *
 * OSEK
 * - ActivateTask   E_OS_ID, E_OS_LIMIT
 * - TerminateTask  E_OS_RESOURCE, E_OS_CALLEVEL
 * - ChainTask      E_OS_ID, E_OS_LIMIT, E_OS_RESOURCE, E_OS_CALLLEVEL
 * - Schedule       E_OS_CALLEVEL, E_OS_RESOURCE
 * - GetTaskID
 * - GetTaskState   E_OS_ID
 *
 *
 * Limitations:
 * - No way to detect if in ISR1 or hooks
 * - NON tasks are not tested.
 * - ActivateTask() with own taskid?
 */

#include "Os.h"
#include "os_test.h"
#include "arc.h"

int btaskRunCnt = 0;

void isr_l(void ) {
	StatusType rv;
	switch(test_nr) {
	case 11:
		rv = TerminateTask();
		TEST_ASSERT(rv=E_OS_CALLEVEL);
		break;
	case 12:
		break;
	case 13:
		break;
	default:
		while(1);
	}
}

void etask_sup_l_01(void) {
	_Bool done = 0;
	StatusType rv;

	while (!done) {
		switch (test_nr) {
		case 1:
			/*@req E_OS_ID ActivateTask */
			rv = ActivateTask(TASK_ID_ILL);
			TEST_ASSERT(rv == E_OS_ID);
			test_nr = 2;
			break;
		case 2:
			/*@req E_OS_LIMIT ActivateTask
			 * Test the activation limit and check that activated task is run
			 * the activation limit times */

			/* Activate higher prio task to do the test */
			rv = ActivateTask(TASK_ID_btask_sup_h);
			TEST_ASSERT(rv == E_OK);

			/* Was it run activation times? */
			TEST_ASSERT(btaskRunCnt == Os_ArcTest_GetTaskActivationLimit(TASK_ID_btask_sup_m));
			test_nr = 10;
			break;
		case 10:
			/*@req E_OS_RESOURCE TerminateTask
			 * Terminate a task that still holds resources
			 * */
			rv = ActivateTask(TASK_ID_btask_sup_h);
			TEST_ASSERT(rv == E_OK);
			break;
		case 11:
			/*@req E_OS_CALLEVEL TerminateTask */
			Os_ArcTest_SetIrqNestLevel(1);
			isr_l();
			Os_ArcTest_SetIrqNestLevel(0);
			test_nr=20;
			break;

		case 20:
			/*@req E_OS_ID ChainTask */
			rv = ChainTask(TASK_ID_ILL);
			TEST_ASSERT(rv == E_OS_ID);
			test_nr++;
		case 21:
			/*@req E_OS_LIMIT ChainTask */
		case 22:
			/*@req E_OS_RESOURCE ChainTask */
		case 23:
			/*@req E_OS_CALLLEVEL ChainTask */
			break;

		case 30:
			/*@req E_OS_CALLEVEL Schedule */
		case 31:
			/*@req E_OS_RESOURCE Schedule */
			break;

		case 100:
			/*@req Scheduler test
			 * The first task(oldest) task of the same priority should be scheduled first
			 * E.g. From M task do ActivateTask()
			 */
			/* Change to higher prio */
			btaskRunCnt = 0;
			rv = ActivateTask(TASK_ID_btask_sup_m);
			/* We got back from M, btask_l is now ready in queue */
			TEST_ASSERT(btaskRunCnt==0);
			/* Terminate ourselves, to be activated later */
			TerminateTask();
			break;
		case 101:
			TEST_ASSERT(btaskRunCnt==1);
			break;
		default:
			while(1);
		}
	}
}

void btask_sup_l_01( void ) {
	switch(test_nr){
	case 100:
		btaskRunCnt++;
		/* Make it go up again */
		test_nr = 101;
		ActivateTask(TASK_ID_etask_sup_l);
		break;
	default:
		break;
	}

}

void btask_sup_m_01( void ) {
	int rv;
	switch(test_nr){
	case 2:
		btaskRunCnt++;
		break;
	case 10:
		rv = GetResource(RES_ID_ext_prio_3);
		TEST_ASSERT(rv == E_OK);

		rv = TerminateTask();
		TEST_ASSERT(rv==E_OS_RESOURCE);

		rv = ReleaseResource(RES_ID_ext_prio_3);
		TerminateTask();
		break;
	case 100:
		/* We got here from etask_l, so it should be oldest */
		rv = ActivateTask(TASK_ID_btask_sup_l);
		break;
	default:
		while(1);
	}

}

void btask_sup_h_01(void) {
	StatusType rv;
	int limit;

	switch (test_nr) {
	case 2:
		/* Activate a basic task to many times */
		limit = Os_ArcTest_GetTaskActivationLimit(TASK_ID_btask_sup_m);
		for (int i = 0; i < limit; i++) {
			rv = ActivateTask(TASK_ID_btask_sup_m);
			TEST_ASSERT(rv == E_OK);
		}
		rv = ActivateTask(TASK_ID_btask_sup_m);
		TEST_ASSERT(rv == E_OS_LIMIT);
		/* Let the us terminate and count the number of times the tasks
		 * was actually activated
		 *
		 * TODO:@req Since the etask and btask have the same priority, who gets scheduled?
		 *           Oldest is scheduled first..
		 * */
		break;
	default:
		while(1);

	}
}

__attribute__ ((section (".test_btask"))) const test_func_t btask_sup_matrix_01[3] = { btask_sup_l_01, btask_sup_m_01, btask_sup_h_01 };
__attribute__ ((section (".test_etask"))) const test_func_t etask_sup_matrix_01[3] = { etask_sup_l_01, NULL, NULL };

