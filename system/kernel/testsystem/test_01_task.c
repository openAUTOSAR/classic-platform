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

static int status_100 = 0;

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
		TEST_RUN();

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
			 * This is done in the TASK_ID_btask_sup_m task..
			 */
			rv = ActivateTask(TASK_ID_btask_sup_m);
			TEST_ASSERT(rv == E_OK);
			test_nr++;
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
			test_nr = 100;
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
			/* Check that tasks as run in priority order and that the oldest task
			 * of the same priority should be scheduled first.
			 * 1. esup_l: Activate(sup_m)
			 * 2. bsup_m: Activate(sup_l)  (should not be taken)
			 * 3. bsup_m: Activate(sup_h)  (taken)
			 * 4. bsup_h: Terminate()
			 *   (We should now have bsup_m, esup_l, bsup_l )
			 * 5. bsup_m: Terminate()
			 * 6. esup_l : Terminate()
			 * 7. bsup_l : Activate(esup_l)  (found by dispatcher)
			 * 8. esup_l:  Back again!!!!
			 *
			 * */
			if(status_100==6) {
				test_nr=101;
				break;
			}
			btaskRunCnt = 0;
			rv = ActivateTask(TASK_ID_btask_sup_m);
			TEST_ASSERT(status_100=3);
			status_100=4;
			TerminateTask();  // Step 6.

			// Should never get here since we have restarded ourselves.
			assert(0);
			break;
		case 101:
			/* End Testing of this module */
			TerminateTask();
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
		TEST_ASSERT(status_100=4);
		status_100=5;
		ActivateTask(TASK_ID_etask_sup_l);  // Step 7.
		TEST_ASSERT(status_100=5);
		status_100=6;
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
		TerminateTask();
		break;
	case 10:
		rv = GetResource(RES_ID_std_prio_3);
		TEST_ASSERT(rv == E_OK);

		rv = TerminateTask();
		TEST_ASSERT(rv==E_OS_RESOURCE);

		rv = ReleaseResource(RES_ID_std_prio_3);
		TEST_ASSERT(rv==E_OK);
		TerminateTask();
		break;
	case 100:
		/* We got here from etask_l, so it should be oldest */
		rv = ActivateTask(TASK_ID_btask_sup_l);
		TEST_ASSERT(rv==E_OK);
		TEST_ASSERT(status_100==0);
		status_100=1;
		rv = ActivateTask(TASK_ID_btask_sup_h);
		TEST_ASSERT(rv==E_OK);
		TEST_ASSERT(status_100==2);
		status_100=3;
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
	case 100:
		TEST_ASSERT(status_100==1);
		status_100=2;
		break;
	default:
		while(1);

	}
}

DECLARE_TEST_ETASK(01, etask_sup_l_01, NULL, NULL );
DECLARE_TEST_BTASK(01, btask_sup_l_01, btask_sup_m_01, btask_sup_h_01);

