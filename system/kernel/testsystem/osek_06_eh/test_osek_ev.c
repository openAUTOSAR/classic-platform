/*
 *  Created on: 4 aug 2010
 *      Author: mahi
 */
/*
 * This file tests test requirements OSEK_EV_XX.
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
	OSEK_EV_01 = 1,
	OSEK_EV_02,
	OSEK_EV_03,
	OSEK_EV_04,
	OSEK_EV_05,
	OSEK_EV_06,
	OSEK_EV_07,
	OSEK_EV_08,
	OSEK_EV_09,
	OSEK_EV_10,
	OSEK_EV_11,
	OSEK_EV_12,
	OSEK_EV_13,
	OSEK_EV_14,
	OSEK_EV_15,
	OSEK_EV_16,
	OSEK_EV_17,
	OSEK_EV_18,
	OSEK_EV_19,
	OSEK_EV_20,
	OSEK_EV_21,
	OSEK_EV_22,
	OSEK_EV_23,
	OSEK_EV_24,
};

TestWorldType TestWorld = {0};

uint32_t testNrNon = 0;

static uint8_t activations = 0;

static void isrSoftInt1( void ) {
	switch ( TestWorld.fixtureNr ) {
	case OSEK_TM_07:
		TEST_SET_FIXTURE(OSEK_EV_07, SEQ_NR_03 );
		break;
	default:
		TEST_ASSERT( 0 );
		break;
	}
}

static void isrSoftInt0( void ) {
	StatusType rv;
	TaskType taskId;

	switch ( TestWorld.fixtureNr ) {
	case OSEK_TM_07:
		TEST_SET_FIXTURE(OSEK_EV_07, SEQ_NR_02 );
		Irq_GenerateSoftInt( IRQ_SOFTINT_1 );
		break;
	case OSEK_TM_09:
		TEST_SET_FIXTURE(OSEK_EV_09, SEQ_NR_04 );
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
	case OSEK_TM_09:
		TEST_SET_FIXTURE(OSEK_EV_09, SEQ_NR_02 );
		/* Make higher prio task ready */
		rv = SetEvent( TASK_ID_etask_m_full,EVENT_MASK_go);
		TEST_ASSERT( rv == E_OK ) ;
		TEST_SET_FIXTURE(OSEK_EV_09, SEQ_NR_03 );
		Irq_GenerateSoftInt( IRQ_SOFTINT_0 );
		TEST_SET_FIXTURE(OSEK_EV_09, SEQ_NR_05 );
		break;
	default:
		TEST_ASSERT( 0 );
	}

}

void etask_m_non ( void ) {
	StatusType rv;
	TaskStateType taskState;
	switch ( TestWorld.fixtureNr ) {
	case OSEK_EV_04:
		TEST_SET_FIXTURE(OSEK_EV_04, SEQ_NR_03 );
		/* Verify that task is in waiting */
		rv = GetTaskState(etask_m_full,&taskState);
		TEST_ASSERT( rv == E_OK );
		TEST_ASSERT( taskState == TASK_STATE_WAITING);
		/* Set the Event */
		rv = SetEvent( etask_m_full, EVENT_MASK_go );
		TEST_ASSERT( rv == E_OK );
		/* Verify state, again */
		rv = GetTaskState(etask_m_full,&taskState);
		TEST_ASSERT( rv == E_OK );
		TEST_ASSERT( taskState == TASK_STATE_READY);
		TEST_SET_FIXTURE(OSEK_EV_04, SEQ_NR_04 );
		break;
	case OSEK_EV_04:
		TEST_SET_FIXTURE(OSEK_EV_04, SEQ_NR_03 );
		rv = SetEvent( etask_m_full, EVENT_MASK_go2 );
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_EV_04, SEQ_NR_04 );
		break;
	default:
		TEST_ASSERT( 0 );
	}

	rv = TerminateTask();
	TEST_ASSERT( rv == E_OK );
}

/**
 * The master in the tests
 */
void etask_m_full( void ) {

	StatusType rv;
	TaskType taskId;
	TaskType isrTask;
	TaskStateType taskState;

	/** @treq OSEK_EV_01
	 *
	 * Call SetEvent() with invalid Task ID
	 *
	 * Service returns E_OS_ID
	 */

	TEST_SET_FIXTURE(OSEK_EV_01, SEQ_NR_01 );
	rv = SetEvent(TASK_ID_ILL);
	TEST_ASSERT( rv == E_OS_ID );

	/** @treq OSEK_EV_02
	 *
	 * Call SetEvent() for basic task
	 *
	 * Service returns E_OS_ACCESS
	 */

	TEST_SET_FIXTURE(OSEK_EV_02, SEQ_NR_01 );
	rv = SetEvent( TASK_ID_btask_l_full, EVENT_MASK_go );
	TEST_ASSERT( rv == E_OS_ACCESS );


	// --------------------------------------------------------------------

	/** @treq OSEK_EV_03
	 *
	 * Call SetEvent() for suspended extended task
	 *
	 * Service returns E_OS_STATE
	 */
	TEST_SET_FIXTURE(OSEK_EV_03, SEQ_NR_01 );
	rv = SetEvent( TASK_ID_etask_l_full, EVENT_MASK_go );
	TEST_ASSERT( rv == E_OS_STATE );

	/** @treq OSEK_EV_04
	 *
	 * Call SetEvent() from non-preemptive task on waiting extended
     * task which is waiting for at least one of the requested events
	 *
	 * Requested events are set. Running task is not preempted.
	 * Waiting task becomes ready  Service returns E_OK
	 */
	TEST_SET_FIXTURE(OSEK_EV_04, SEQ_NR_01 );
	rv = ActivateTask( TASK_ID_etask_m_non );
	TEST_ASSERT( rv == E_OK );
	TEST_SET_FIXTURE(OSEK_EV_04, SEQ_NR_02 );
	rv = WaitEvent( EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );
	TEST_SET_FIXTURE(OSEK_EV_04, SEQ_NR_05 );

	/** @treq OSEK_EV_05
	 *
	 * Call SetEvent() from non-preemptive task on waiting extended
	 * task which is not waiting for any of the requested events
	 *
	 * Requested events are set. Running task is not preempted.
	 * Waiting task doesn’t become ready. Service returns E_OK
	 */
	TEST_SET_FIXTURE(OSEK_EV_05, SEQ_NR_01 );
	rv = ActivateTask( TASK_ID_etask_m_non );
	TEST_ASSERT( rv == E_OK );

	/** @treq OSEK_EV_06
	 *
	 * Interruption of running task
	 *
	 * Interrupt is executed
	 */
	TEST_SET_FIXTURE(OSEK_EV_06, SEQ_NR_01 );
	/* Already tested in a number of cases in tm suite */

	/** @treq OSEK_EV_07
	 *
	 * Interruption of ISR2
	 *
	 * Interrupt is executed
	 */
	TEST_SET_FIXTURE(OSEK_EV_07, SEQ_NR_01 );
	/* Create an ISR2 */
	isrTask = Os_Arc_CreateIsr( isrSoftInt0, 8/*prio*/,"soft_0");
	Irq_AttachIsr2(isrTask,NULL, IRQ_SOFTINT_0);

	/* Create an ISR2 */
	isrTask = Os_Arc_CreateIsr( isrSoftInt0, 9/*prio*/,"soft_1");
	Irq_AttachIsr2(isrTask,NULL, IRQ_SOFTINT_1);

	Irq_GenerateSoftInt( IRQ_SOFTINT_0 );
	TEST_SET_FIXTURE(OSEK_EV_07, SEQ_NR_04 );


	/** @treq OSEK_EV_08
	 *
	 * Interruption of ISR3
	 *
	 * Interrupt is executed
	 */
	TEST_SET_FIXTURE(OSEK_EV_08, SEQ_NR_01 );
	/* ISR3 is not applicable */

	/** @treq OSEK_EV_09
	 *
	 * Return from ISR2. Interrupted task is non-preemptive
	 *
	 * Execution of interrupted task is continued
	 */
	TEST_SET_FIXTURE(OSEK_EV_09, SEQ_NR_01 );
	rv = ActivateTask(TASK_ID_btask_l_non );
	TEST_ASSERT( rv == E_OK );
	rv = WaitEvent( EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );
	rv = ClearEvent( EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );
	TEST_SET_FIXTURE(OSEK_EV_09, SEQ_NR_06 );

	/** @treq OSEK_EV_10
	 *
	 * Return from ISR3. Interrupted task is non-preemptive
     *
	 * Execution of interrupted task is continued
	 */
	TEST_SET_FIXTURE(OSEK_EV_10, SEQ_NR_01 );
	/* ISR3 is not applicable */

	/** @treq OSEK_EV_11
	 *
	 * Return from ISR2. Interrupted task is preemptive
     *
	 * Ready task with highest priority is executed (Rescheduling)
	 */
	TEST_SET_FIXTURE(OSEK_EV_11, SEQ_NR_01 );



	/** @treq OSEK_EV_12
	 *
	 * Return from ISR3. Interrupted task is preemptive
     *
	 * Ready task with highest priority is executed (Rescheduling)
	 */
	TEST_SET_FIXTURE(OSEK_EV_12, SEQ_NR_01 );
	/* ISR3 is not applicable */


	TestExit(0);
}
