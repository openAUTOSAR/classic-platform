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
 * Tested: Resources
 *
 * Calls:  GetResource()
 *         ReleaseResource()
 *         Schedule()
 *
 */

#include "Os.h"
#include "debug.h"
//#include <stdio.h>
#include <assert.h>
#include "os_test.h"
#include "irq.h"

#if 0
void isr_l(void ) {
	switch(test_nr) {
	case 2:
		TEST_OK();
		test_nr++;
	case 5:
		test_nr++;
		/* Trigger higher prio interrupt */
		Irq_GenerateSoftInt(EXTI1_IRQn);
		TEST_ASSERT(test_nr==8);
		break;
	default:
		assert(0);
	}
}

void isr_m(void ) {
	switch(test_nr) {
	case 3:
		TEST_OK();
		test_nr++;
	case 6:
		test_nr++;
		/* Trigger higher prio interrupt */
		Irq_GenerateSoftInt(EXTI2_IRQn);
		TEST_ASSERT(test_nr==7);
		test_nr++;
		break;
	default:
		assert(0);
	}

}
#endif


void etask_sup_l_04( void ) {
	test_nr = 1;
	TaskType isr1;
	TaskType isr2;
	TaskType isr3;

	for(;;) {
		switch(test_nr){
		case 1:
			// test 01 ===============================================
			// Test: Grab and release the one and only standard resource.
			GetResource(RES_SCHEDULER);
			ReleaseResource(RES_SCHEDULER);
			TEST_OK();
			test_nr++;
			break;

		case 2:
			// test 02 ===============================================
			// Test: Grab resource twice. Should result in E_OS_ACCESS
			GetResource(RES_SCHEDULER);
			GetResource(RES_SCHEDULER);
			ReleaseResource(RES_SCHEDULER);
			// ??
			TEST_OK();
			test_nr++;

			break;
		case 3:
			// test 03 ===============================================
			// Test: Check that the scheduler is locked when doing this.
			//       SetEvent() on higher prio process.
			GetResource(RES_SCHEDULER);
			// TODO:
			ReleaseResource(RES_SCHEDULER);
			TEST_OK();
			test_nr++;
			break;
		case 4:
			// test 04 ===============================================
			// Test: Check that priority inversion works.
			//       Allocate resource and check priority.
			GetResource(RES_ID_ext_prio_3);
			ReleaseResource(RES_ID_ext_prio_3);
			TEST_OK();
			test_nr++;
			break;
		case 5:
			// test 05 ===============================================
			// Test: Attempt to get a resource which the statically assigned
			//       priority of the calling task or interrupt routine is
			//       higher than the calculated ceiling priority, E_OS_ACCESS
			TEST_ASSERT(test_nr==5);
			break;
		case 6:
			// test 06 ===============================================
			// Test: Nest resources
			GetResource(RES_ID_ext_prio_3);
			GetResource(RES_ID_ext_prio_4);
			ReleaseResource(RES_ID_ext_prio_4);
			ReleaseResource(RES_ID_ext_prio_3);
			TEST_OK();
			test_nr++;
			break;
		case 100:
			TerminateTask();
		default:
			assert(0);
			while(1);
		}
	}
}

void etask_sup_m_04( void ) {
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

/* In the OS you can queue activations for a task.
 * Example: Two tasks, main with prio 2 and slave with prio 1,activate 2.
 */

void ActivationQueue(void) {
	{
	  StatusType rv;
	  /* Queue up */
	  for(int i=0;i<2;i++) {
	    rv=ActivateTask(slave);
	    assert(rv==E_OK);
	  }

	  /* Try to queue up more....that will not succeed */
	  for(int i=0;i<2;i++) {
	    rv=ActivateTask(slave);
	    assert(rv==E_OS_LIMIT);
	  }
	  /* When we terminate slave will be run twice */
	  TerminateTask();
	}

}




