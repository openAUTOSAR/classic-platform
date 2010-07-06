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
 * OSEK:
 * - GetResource     			E_OS_ID, E_OS_ACCESS
 * - ReleaseResource 			E_OS_ID, E_OS_ACCESS, E_OS_NOFUNC
 *
 * More tests here:
 * - GetResource(RES_SCHEDULER)
 * - Check the ceiling protocol
 * - Linked resources
 * - Nested allocation of the same resource is forbidden.
 *
 *
 * Limitations:
 * - Internal resources.
 */

#include "Os.h"
#include "test_framework.h"
#include "arc.h"
#include "irq.h"


static int isr2RunCnt = 0;
static int taskRunCnt = 0;

static void isr2( void ) {
	isr2RunCnt++;
	ActivateTask(TASK_ID_btask_sup_m);
#if 0
	{
		TaskType task;
		GetTaskID(&task);
		printf("ISR2: %d\n",task);
	}
#endif
}

static void isr1( void ) {

}

void etask_sup_l_02(void) {
	_Bool done = 0;
	StatusType rv;
	TaskType isrTask;
	TaskType task;

	while (!done) {
		TEST_RUN();
		switch (test_nr) {
		case 1:
			rv = GetResource(RES_ID_std_prio_3);
			TEST_ASSERT(rv == E_OK);
			rv = ReleaseResource(RES_ID_std_prio_3);
			TEST_ASSERT(rv == E_OK);
			test_nr++;
			break;
		case 2:
			/* Get the same resource twice */
			rv = GetResource(RES_ID_std_prio_3);
			TEST_ASSERT(rv == E_OK);
			rv = GetResource(RES_ID_std_prio_3);
			TEST_ASSERT(rv == E_OS_ACCESS);
			test_nr = 10;
			break;
		case 10:
			/* Test RES_SCHEDULER.
			 * Since GetResource(RES_SCHEDULER) only locks the
			 * scheduler (interrupts are still working) we the interrupt
			 * task should trigger a higher prio task, but that should
			 * not be swapped in.
			 */

			/* Create an ISR2 */
			isrTask = Os_Arc_CreateIsr( isr2, 4/*prio*/,"soft_0");
			Irq_AttachIsr2(isrTask,NULL,INTC_SSCIR0_CLR0);
			/* Create an ISR1 */
			Irq_AttachIsr1(isr1,NULL,INTC_SSCIR0_CLR1,6);

			GetTaskID(&task);
			TEST_ASSERT( task == TASK_ID_etask_sup_l );
			/* Lock the scheduler */
			rv = GetResource(RES_SCHEDULER);
			Irq_GenerateSoftInt( INTC_SSCIR0_CLR0 );

			GetTaskID(&task);

			TEST_ASSERT( task == TASK_ID_etask_sup_l );

			/* TODO: Should we re-schedule here? */
			rv = ReleaseResource(RES_SCHEDULER);
			// TODO: ReleaseResource should re-schedule

			TEST_ASSERT( task == TASK_ID_etask_sup_l );

//			TEST_ASSERT( taskRunCnt == 1 );
			// TODO:

			test_nr = 100;
			break;
		case 100:
			TerminateTask();
			break;
		}
	}
}

void btask_sup_l_02(void) {
}

void btask_sup_m_02(void) {
	TaskType task;
	GetTaskID(&task);

	taskRunCnt++;
	TEST_ASSERT( task == TASK_ID_btask_sup_m );
}


DECLARE_TEST_ETASK(02, etask_sup_l_02, NULL, NULL );
DECLARE_TEST_BTASK(02, btask_sup_l_02, btask_sup_m_02, NULL );
