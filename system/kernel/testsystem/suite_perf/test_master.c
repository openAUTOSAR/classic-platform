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


#include <stdlib.h>
#include "Os.h"
#include "test_framework.h"
#include "Mcu.h"
#if defined(USE_GPT)
#include "Gpt.h"
#endif


//#define USE_LDEBUG_PRINTF
#include "debug.h"
#include "arc.h"
#include "irq.h"


#define ERROR_LOG_SIZE	1


#define MAKE_TEST(_x)		TEST_ ## (_x)
#define TEST_INC()			test_nr++
//#define TEST_START2()		printf("%s\n",TestFixure[test_nr-1].description)
#define TEST_START2()		TestStart(TestFixure[test_nr-1].description, TestFixure[test_nr-1].nr );
#define TEST_GETCASE()		TestFixure[test_nr-1].nr
#define TEST_LAST()		(TestFixure[test_nr-1].nr == TEST_LAST)


enum testCase {
	TEST_1 = 1,
	TEST_2,
	TEST_3,
	TEST_LAST,
};



static TestFixtureType TestFixure[] = {
/* 0 */
		{"Interrupt latency, 0% load (Tick running)",TEST_1},
		{"Interrupt latency, 0% load",TEST_2},
		{"Nested interrupt, (Tick Running)",TEST_3},
		{"",TEST_LAST},
};


static TickType start;
static TickType diff;
static _Bool kill =  0;

static void isrSoftInt0( void ) {
	diff = Os_SysTickGetElapsedValue(start);
}

static void isrSoftInt1( void ) {
	Irq_GenerateSoftInt( IRQ_SOFTINT_0 );
	diff = Os_SysTickGetElapsedValue(start);
}


void Task2( void  ) {

}

void Task3( void  ) {

}

/*
 * Master test process, everything is controlled from here.
 */
void etask_master( void ) {
	TEST_INIT();
	test_nr = 1;
	TaskType isrTask;

	/* Create an ISR2 */
	isrTask = Os_Arc_CreateIsr( isrSoftInt0, 8/*prio*/,"soft_0");
	Irq_AttachIsr2(isrTask,NULL,IRQ_SOFTINT_0);
	/* Create an ISR2, lower prio  */
	isrTask = Os_Arc_CreateIsr( isrSoftInt1, 4/*prio*/,"soft_0");
	Irq_AttachIsr2(isrTask,NULL,IRQ_SOFTINT_1);

	while (!TEST_LAST()) {
		if( kill == 1) {
			/* Kill tasks */
//			SetEvent(TASK_ID_etask_master, EVENT_MASK_kill);
//			TerminateTask();
		}

		TEST_START2();
		switch(TEST_GETCASE()) {
		case TEST_1:
		{
			int i;
			for(i=0;i<100;i++) {
				start = Os_SysTickGetValue();
				Irq_GenerateSoftInt( IRQ_SOFTINT_0 );
				printf("Counter diff is %d HW ticks\n",diff);
			}
			break;
		}
		case TEST_2:
		{
			TEST_NOT_IMPLEMENTED();
			break;
		}
		case TEST_3:
		{
			Irq_GenerateSoftInt( IRQ_SOFTINT_1 );
			break;
		}
		default:
			TEST_ASSERT(0);
			break;
		}
		TEST_END();
		TEST_INC();
	}

	TestExit(0);
}

//--------------------------------------------------------------------


void OsIdle(void ) {
	for(;;);
}


