/*
 *  Created on: 4 aug 2010
 *      Author: mahi
 */
/*
 * This file tests test requirements OSEK_IP_XX.
 * We need only 2 tasks, 1 non-preemtive
 *
 *
 * Priorities:
 * ll - low low prio (used for scheduling only)
 * l  - low prio
 * m  - medium
 * h  - high prio
 */


#include "os.h"
#include "test_framework.h"
#include "arc.h"
#include "irq.h"



enum OsekFixtureNr {
	OSEK_IP_01 = 1,
	OSEK_IP_02,
	OSEK_IP_03,
	OSEK_IP_04,
	OSEK_IP_05,
	OSEK_IP_06,
	OSEK_IP_07,
	OSEK_IP_08,
	OSEK_IP_09,
	OSEK_IP_10,
	OSEK_IP_11,
	OSEK_IP_12,
};

TestWorldType TestWorld = {0};

uint32_t testNrNon = 0;

static void isrSoftInt1( void ) {
	switch ( TestWorld.fixtureNr ) {
	case OSEK_IP_07:
		TEST_SET_FIXTURE(OSEK_IP_07, SEQ_NR_03 );
		break;
	default:
		TEST_ASSERT( 0 );
		break;
	}
}

static void isrSoftInt0( void ) {

	switch ( TestWorld.fixtureNr ) {
	case OSEK_IP_07:
		TEST_SET_FIXTURE(OSEK_IP_07, SEQ_NR_02 );
		Irq_GenerateSoftInt( IRQ_SOFTINT_1 );
		break;
	case OSEK_IP_09:
		TEST_SET_FIXTURE(OSEK_IP_09, SEQ_NR_04 );
		break;
	default:
		TEST_ASSERT( 0 );
	}

}



void OsIdle(void) {
	while(1);
}


void btask_l_non ( void ) {
	StatusType rv;
	switch ( TestWorld.fixtureNr ) {
	case OSEK_IP_09:
		TEST_SET_FIXTURE(OSEK_IP_09, SEQ_NR_02 );
		/* Make higher prio task ready */
		rv = SetEvent( TASK_ID_etask_m_full,EVENT_MASK_go);
		TEST_ASSERT( rv == E_OK ) ;
		TEST_SET_FIXTURE(OSEK_IP_09, SEQ_NR_03 );
		Irq_GenerateSoftInt( IRQ_SOFTINT_0 );
		TEST_SET_FIXTURE(OSEK_IP_09, SEQ_NR_05 );
		break;
	default:
		TEST_ASSERT( 0 );
	}

}

/**
 * The master in the tests
 */
void etask_m_full( void ) {

	StatusType rv;
	TaskType isrTask;

	/** @treq OSEK_IP_01
	 *
	 * Call EnableInterrupt(). All requested interrupts are disabled
	 *
	 * Enable interrupts. Service returns E_OK
	 */

	TEST_SET_FIXTURE(OSEK_IP_01, SEQ_NR_01 );
	/* The API is no longer available in Autosar or OSEK */

	/** @treq OSEK_IP_02
	 *
	 * Call EnableInterrupt(). At least one of the requested interrupts is already enabled.
	 *
	 * Enable interrupts. Service returns E_OS_NOFUNC
	 */

	TEST_SET_FIXTURE(OSEK_IP_02, SEQ_NR_01 );
	/* The API is no longer available in Autosar or OSEK */

	/** @treq OSEK_IP_03
	 *
	 * Call DisableInterrupt(). All requested interrupts are enabled
	 *
	 * Disable interrupts. Service returns E_OK
	 */
	TEST_SET_FIXTURE(OSEK_IP_03, SEQ_NR_01 );
	/* The API is no longer available in Autosar or OSEK */

	/** @treq OSEK_IP_04
	 *
	 * "Call DisableInterrupt(). At least one of the requested interrupts
	 * is already disabled"
	 *
	 * Disable interrupts. Service returns E_OS_NOFUNC
	 */
	TEST_SET_FIXTURE(OSEK_IP_04, SEQ_NR_01 );
	/* The API is no longer available in Autosar or OSEK */

	/** @treq OSEK_IP_05
	 *
	 * Call GetInterruptDescriptor()
	 *
	 * "Returns current interrupt descriptor. Service returns E_OK"
	 */
	TEST_SET_FIXTURE(OSEK_IP_05, SEQ_NR_01 );
	/* The API is no longer available in Autosar or OSEK */

	/** @treq OSEK_IP_06
	 *
	 * Interruption of running task
	 *
	 * Interrupt is executed
	 */
	TEST_SET_FIXTURE(OSEK_IP_06, SEQ_NR_01 );
	/* Already tested in a number of cases in tm suite */

	/** @treq OSEK_IP_07
	 *
	 * Interruption of ISR2
	 *
	 * Interrupt is executed
	 */
	TEST_SET_FIXTURE(OSEK_IP_07, SEQ_NR_01 );
	/* Create an ISR2 */
	isrTask = Os_Arc_CreateIsr( isrSoftInt0, 8/*prio*/,"soft_0");
	Irq_AttachIsr2(isrTask,NULL, IRQ_SOFTINT_0);

	/* Create an ISR2 */
	isrTask = Os_Arc_CreateIsr( isrSoftInt1, 9/*prio*/,"soft_1");
	Irq_AttachIsr2(isrTask,NULL, IRQ_SOFTINT_1);

	Irq_GenerateSoftInt( IRQ_SOFTINT_0 );
	TEST_SET_FIXTURE(OSEK_IP_07, SEQ_NR_04 );


	/** @treq OSEK_IP_08
	 *
	 * Interruption of ISR3
	 *
	 * Interrupt is executed
	 */
	TEST_SET_FIXTURE(OSEK_IP_08, SEQ_NR_01 );
	/* ISR3 is not applicable */

	/** @treq OSEK_IP_09
	 *
	 * Return from ISR2. Interrupted task is non-preemptive
	 *
	 * Execution of interrupted task is continued
	 */
	TEST_SET_FIXTURE(OSEK_IP_09, SEQ_NR_01 );
	rv = ActivateTask(TASK_ID_btask_l_non );
	TEST_ASSERT( rv == E_OK );
	rv = WaitEvent( EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );
	rv = ClearEvent( EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );
	TEST_SET_FIXTURE(OSEK_IP_09, SEQ_NR_06 );

	/** @treq OSEK_IP_10
	 *
	 * Return from ISR3. Interrupted task is non-preemptive
     *
	 * Execution of interrupted task is continued
	 */
	TEST_SET_FIXTURE(OSEK_IP_10, SEQ_NR_01 );
	/* ISR3 is not applicable */

	/** @treq OSEK_IP_11
	 *
	 * Return from ISR2. Interrupted task is preemptive
     *
	 * Ready task with highest priority is executed (Rescheduling)
	 */
	TEST_SET_FIXTURE(OSEK_IP_11, SEQ_NR_01 );



	/** @treq OSEK_IP_12
	 *
	 * Return from ISR3. Interrupted task is preemptive
     *
	 * Ready task with highest priority is executed (Rescheduling)
	 */
	TEST_SET_FIXTURE(OSEK_IP_12, SEQ_NR_01 );
	/* ISR3 is not applicable */


	TestExit(0);
}
