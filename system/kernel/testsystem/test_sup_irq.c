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
 * Tested: Irq and nesting of interrupts
 */

#include "Os.h"
#include "Trace.h"
//#include <stdio.h>
#include <assert.h>
#include "os_test.h"
#include "int_ctrl.h"

void isr_l(void ) {
	switch(test_nr) {
	case 2:
		TEST_OK();
		test_nr++;
	case 5:
		test_nr++;
		/* Trigger higher prio interrupt */
		IntCtrl_GenerateSoftInt(EXTI1_IRQn);
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
		IntCtrl_GenerateSoftInt(EXTI2_IRQn);
		TEST_ASSERT(test_nr==7);
		test_nr++;
		break;
	default:
		assert(0);
	}

}

void isr_h(void ) {
	switch(test_nr) {
	case 4:
		TEST_OK();
		test_nr++;
	case 7:
		TEST_OK();
		test_nr++;
		break;
	default:
		assert(0);
	}
}


void etask_sup_l_04( void ) {
	test_nr = 1;
	TaskType isr1;
	TaskType isr2;
	TaskType isr3;

	for(;;) {
		switch(test_nr){
		case 1:
			// test 01 ===============================================
			isr1 = Os_CreateIsr(isr_l,6/*prio*/,"ISR_L");
			IntCtrl_AttachIsr2(isr1,NULL, EXTI0_IRQn);

			isr2 = Os_CreateIsr(isr_m,7/*prio*/,"ISR_M");
			IntCtrl_AttachIsr2(isr2,NULL, EXTI1_IRQn);

			isr3 = Os_CreateIsr(isr_h,8/*prio*/,"ISR_H");
			IntCtrl_AttachIsr2(isr3,NULL, EXTI2_IRQn);
			TEST_OK();
			test_nr++;
			break;

		case 2:
			// test 02 ===============================================
			IntCtrl_GenerateSoftInt(EXTI0_IRQn);
			TEST_ASSERT(test_nr==3);
			break;
		case 3:
			// test 03 ===============================================
			IntCtrl_GenerateSoftInt(EXTI1_IRQn);
			TEST_ASSERT(test_nr==4);
			break;
		case 4:
			// test 04 ===============================================
			IntCtrl_GenerateSoftInt(EXTI2_IRQn);
			TEST_ASSERT(test_nr==5);
			break;
		case 5:
			// test 05 ===============================================
			IntCtrl_GenerateSoftInt(EXTI2_IRQn);
			TEST_ASSERT(test_nr==5);
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
