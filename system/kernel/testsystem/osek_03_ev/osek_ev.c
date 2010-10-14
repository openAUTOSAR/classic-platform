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
	OSEK_EV_25,
	OSEK_EV_26,
};

TestWorldType TestWorld = {0};

uint32_t testNrNon = 0;

static uint8_t activations = 0;

static void isrSoftInt1( void ) {
	switch ( TestWorld.fixtureNr ) {
	case OSEK_EV_07:
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
	case OSEK_EV_12:
		TEST_SET_FIXTURE(OSEK_EV_12, SEQ_NR_02 );
		rv = ClearEvent( EVENT_MASK_go );
		TEST_ASSERT( rv == E_OS_CALLEVEL );
		break;
	case OSEK_EV_23:
		TEST_SET_FIXTURE(OSEK_EV_23, SEQ_NR_02 );
		rv = WaitEvent( EVENT_MASK_go );
		TEST_ASSERT( rv == E_OS_CALLEVEL );
		break;
	default:
		TEST_ASSERT( 0 );
	}

}

void TestTaskRunLowerPrio( void ) {
	StatusType rv;
	/* Activate lowest prioriy task */
	rv = ActivateTask(TASK_ID_btask_ll_non);
	TEST_ASSERT( rv == E_OK );
	/* Let the low prio task run */
	rv = WaitEvent(EVENT_MASK_go);
	rv = ClearEvent(EVENT_MASK_go);
	TEST_ASSERT( rv == E_OK );
}

void TestActivateAndWait( TaskType task ) {
	StatusType rv;
	rv = ActivateTask(task);
	TEST_ASSERT( rv == E_OK );
	/* Let the low prio task run */
	rv = WaitEvent(EVENT_MASK_go);
	TEST_ASSERT( rv == E_OK );
	rv = ClearEvent(EVENT_MASK_go);
	TEST_ASSERT( rv == E_OK );
}


void btask_h_full( void ) {
	StatusType rv;
	switch ( TestWorld.fixtureNr ) {
	case OSEK_EV_11:
		TEST_SET_FIXTURE(OSEK_EV_11, SEQ_NR_02 );
		rv = ClearEvent(EVENT_MASK_go);
		TEST_ASSERT( rv == E_OS_ACCESS );
		break;
	case OSEK_EV_21:
		TEST_SET_FIXTURE(OSEK_EV_21, SEQ_NR_02 );
		rv = WaitEvent(EVENT_MASK_go);
		TEST_ASSERT( rv == E_OS_ACCESS );
		TEST_SET_FIXTURE(OSEK_EV_21, SEQ_NR_03 );
		break;
	default:
		TEST_ASSERT( 0 );
	}
	rv = TerminateTask();
	TEST_ASSERT( rv == E_OK );
}

void etask_h_full( void ) {
	StatusType rv;
	switch ( TestWorld.fixtureNr ) {
	case OSEK_EV_08:
		TEST_SET_FIXTURE(OSEK_EV_08, SEQ_NR_02 );
		rv = WaitEvent( EVENT_MASK_go );
		TEST_ASSERT( rv == E_OK );
		rv = ClearEvent( EVENT_MASK_go | EVENT_MASK_go2 );
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_EV_08, SEQ_NR_05 );
		break;
	case OSEK_EV_09:
		TEST_SET_FIXTURE(OSEK_EV_09, SEQ_NR_04 );
		rv = SetEvent( TASK_ID_etask_m_full, EVENT_MASK_go);
		TEST_ASSERT( rv == E_OK );
		break;
	default:
		TEST_ASSERT( 0 );
	}
	rv = TerminateTask();
	TEST_ASSERT( rv == E_OK );
}

void btask_m_full( void ) {

}

void btask_ll_non( void ) {
	StatusType rv;
	/* Used for scheduling ONLY */
	rv = SetEvent(TASK_ID_etask_m_full, EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );
}


void OsIdle(void) {
	while(1);
}


void btask_l_non ( void ) {
	StatusType rv;
	switch ( TestWorld.fixtureNr ) {
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
		rv = GetTaskState(TASK_ID_etask_m_full,&taskState);
		TEST_ASSERT( rv == E_OK );
		TEST_ASSERT( taskState == TASK_STATE_WAITING);
		/* Set the Event */
		rv = SetEvent( TASK_ID_etask_m_full, EVENT_MASK_go );
		TEST_ASSERT( rv == E_OK );
		/* Verify state, again */
		rv = GetTaskState(TASK_ID_etask_m_full,&taskState);
		TEST_ASSERT( rv == E_OK );
		TEST_ASSERT( taskState == TASK_STATE_READY);
		TEST_SET_FIXTURE(OSEK_EV_04, SEQ_NR_04 );
		break;
	case OSEK_EV_05:
		TEST_SET_FIXTURE(OSEK_EV_05, SEQ_NR_03 );
		/* Set event in etask_m_full that it is NOT waiting for */
		rv = SetEvent( TASK_ID_etask_m_full, EVENT_MASK_go2 );
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_EV_05, SEQ_NR_04 );
		/* Cleanup, and let it run */
		rv = SetEvent( TASK_ID_etask_m_full, EVENT_MASK_go );
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_EV_05, SEQ_NR_05 );
		break;
	case OSEK_EV_09:
		TEST_SET_FIXTURE(OSEK_EV_09, SEQ_NR_02 );
		/* make higher prio task ready */
		rv = ActivateTask( TASK_ID_etask_h_full );
		TEST_ASSERT( rv == E_OK );
		/* Do the actual SetEvent() test */
		rv = SetEvent( TASK_ID_etask_h_full, EVENT_MASK_go );
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_EV_09, SEQ_NR_03 );
		break;
	default:
		TEST_ASSERT( 0 );
	}

	rv = TerminateTask();
	TEST_ASSERT( rv == E_OK );
}

void btask_l_full ( void ) {
	StatusType rv;
	switch ( TestWorld.fixtureNr ) {
	case OSEK_EV_09:
		break;
	default:
		break;
	}
}

void etask_l_full ( void ) {
	StatusType rv;
	EventMaskType mask;
	switch ( TestWorld.fixtureNr ) {
	case OSEK_EV_06:
		TEST_SET_FIXTURE(OSEK_EV_06, SEQ_NR_03 );
		/* Set event preempt us */
		rv = SetEvent(TASK_ID_etask_m_full,EVENT_MASK_go);
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_EV_06, SEQ_NR_05 );
		break;
	case OSEK_EV_07:
		TEST_SET_FIXTURE(OSEK_EV_07, SEQ_NR_03 );
		rv = WaitEvent(EVENT_MASK_go);
		TEST_ASSERT( rv == E_OK );
		rv = ClearEvent(EVENT_MASK_go);
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_EV_07, SEQ_NR_06 );
		break;
	case OSEK_EV_10:
		TEST_SET_FIXTURE(OSEK_EV_10, SEQ_NR_03 );
		break;
	case OSEK_EV_19:
		TEST_SET_FIXTURE(OSEK_EV_19, SEQ_NR_03 );
		rv = ClearEvent( EVENT_MASK_go2 );
		break;
	case OSEK_EV_20:
		TEST_SET_FIXTURE(OSEK_EV_20, SEQ_NR_02 );
		rv = SetEvent( TASK_ID_etask_m_full, EVENT_MASK_go2 );
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_EV_20, SEQ_NR_03 );
		rv = GetEvent( TASK_ID_etask_m_full, &mask );
		TEST_ASSERT( rv == E_OK );
		TEST_ASSERT( mask == EVENT_MASK_go2 );
		rv = SetEvent( TASK_ID_etask_m_full, EVENT_MASK_go );
		TEST_ASSERT( rv == E_OK );
	default:
		break;
	}
	TerminateTask();
}


/**
 * The master in the tests
 */
void etask_m_full( void ) {

	StatusType rv;
	TaskType taskId;
	TaskType isrTask;
	TaskStateType taskState;
	EventMaskType mask;

	/** @treq OSEK_EV_01
	 *
	 * Call SetEvent() with invalid Task ID
	 *
	 * Service returns E_OS_ID
	 */

	TEST_SET_FIXTURE(OSEK_EV_01, SEQ_NR_01 );
	rv = SetEvent(TASK_ID_ILL,EVENT_MASK_go);
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
	rv = ClearEvent( EVENT_MASK_go );
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
	TEST_SET_FIXTURE(OSEK_EV_05, SEQ_NR_02 );
	rv = WaitEvent( EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );
	rv = ClearEvent( EVENT_MASK_go | EVENT_MASK_go2 );
	TEST_ASSERT( rv == E_OK );
	TEST_SET_FIXTURE(OSEK_EV_05, SEQ_NR_06 );

	/** @treq OSEK_EV_06
	 *
	 * Call SetEvent() from preemptive task on waiting extended task which
	 * is waiting for at least one of the requested events and has higher
	 * priority than running task
	 *
	 * Requested events are set. Running task becomes ready (is preempted)
	 * Waiting task becomes running. Service returns E_OK
	 */
	TEST_SET_FIXTURE(OSEK_EV_06, SEQ_NR_01 );
	rv = ActivateTask( TASK_ID_etask_l_full );
	TEST_ASSERT( rv == E_OK );
	TEST_SET_FIXTURE(OSEK_EV_06, SEQ_NR_02 );
	/* Let etask__l_full */
	rv = WaitEvent( EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );
	rv = ClearEvent( EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );
	TEST_SET_FIXTURE(OSEK_EV_06, SEQ_NR_04 );
	TestTaskRunLowerPrio();
	TEST_SET_FIXTURE(OSEK_EV_06, SEQ_NR_06 );


	/** @treq OSEK_EV_07
	 *
	 * Call SetEvent() from preemptive task on waiting extended
	 * task which is waiting for at least one of the requested
	 * events and has equal or lower priority than running task
	 *
	 * Requested events are set. Running task is not preempted.
	 * Waiting task becomes ready. Service returns E_OK
	 */
	TEST_SET_FIXTURE(OSEK_EV_07, SEQ_NR_01 );
	rv = ActivateTask( TASK_ID_etask_l_full );
	TEST_ASSERT( rv == E_OK );
	TEST_SET_FIXTURE(OSEK_EV_07, SEQ_NR_02 );
	/* Let the lowerprio task hit WaitEvent() */
	TestTaskRunLowerPrio();
	TEST_SET_FIXTURE(OSEK_EV_07, SEQ_NR_04 );
	/* Set the Event, no preempt */
	rv = SetEvent( TASK_ID_etask_l_full, EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );
	TEST_SET_FIXTURE(OSEK_EV_07, SEQ_NR_05 );
	TestTaskRunLowerPrio();
	TEST_SET_FIXTURE(OSEK_EV_07, SEQ_NR_07 );

	/** @treq OSEK_EV_08
	 *
	 * Call SetEvent() from preemptive task on waiting extended
	 * task which is not waiting for any of the requested events
	 *
	 * Requested events are set. Running task is not preempted.
	 * Waiting task doesn’t become ready. Service returns E_OK
	 */
	TEST_SET_FIXTURE(OSEK_EV_08, SEQ_NR_01 );
	rv = ActivateTask( TASK_ID_etask_h_full );
	TEST_ASSERT( rv == E_OK );
	TEST_SET_FIXTURE(OSEK_EV_08, SEQ_NR_03 );
	rv = SetEvent( TASK_ID_etask_h_full, EVENT_MASK_go2 );
	TEST_ASSERT( rv == E_OK );
	TEST_SET_FIXTURE(OSEK_EV_08, SEQ_NR_04 );
	/* Cleanup, Let it run */
	rv = SetEvent( TASK_ID_etask_h_full, EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );
	TEST_SET_FIXTURE(OSEK_EV_08, SEQ_NR_06 );

	/** @treq OSEK_EV_09
	 *
	 * Call SetEvent() from non-preemptive task on ready extended
	 * task.
	 *
	 * Requested events are set. Running task is not preempted.
	 * Service returns E_OK
	 */

	TEST_SET_FIXTURE(OSEK_EV_09, SEQ_NR_01 );
	TestActivateAndWait(TASK_ID_etask_m_non);
	TEST_SET_FIXTURE(OSEK_EV_09, SEQ_NR_05 );


	/** @treq OSEK_EV_10
	 *
	 * Call SetEvent() from preemptive task on ready extended task
	 *
	 * Requested events are set. Running task is not  preempted.
	 * Service returns E_OK
	 */
	TEST_SET_FIXTURE(OSEK_EV_10, SEQ_NR_01 );
	rv = ActivateTask( TASK_ID_etask_l_full );
	TEST_ASSERT( rv == E_OK );
	rv = SetEvent( TASK_ID_etask_l_full, EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );
	TEST_SET_FIXTURE(OSEK_EV_10, SEQ_NR_02 );
	TestTaskRunLowerPrio();
	TEST_SET_FIXTURE(OSEK_EV_10, SEQ_NR_04 );

	/** @treq OSEK_EV_11
	 *
	 * Call ClearEvent() from basic task
	 *
	 * Service returns E_OS_ACCESS
	 */
	TEST_SET_FIXTURE(OSEK_EV_11, SEQ_NR_01 );
	rv = ActivateTask( TASK_ID_btask_h_full );
	TEST_SET_FIXTURE(OSEK_EV_11, SEQ_NR_03 );


	/** @treq OSEK_EV_12
	 *
	 * Call ClearEvent() from ISR2
	 *
	 *  Service returns E_OS_CALLEVEL
	 */
	/* Create an ISR2 */
	TEST_SET_FIXTURE(OSEK_EV_12, SEQ_NR_01 );
	isrTask = Os_Arc_CreateIsr( isrSoftInt0, 8/*prio*/,"soft_0");
	Irq_AttachIsr2(isrTask,NULL, IRQ_SOFTINT_0);
	Irq_GenerateSoftInt( IRQ_SOFTINT_0 );
	TEST_SET_FIXTURE(OSEK_EV_12, SEQ_NR_03 );


	/** @treq OSEK_EV_13
	 *
	 * Call ClearEvent() from ISR3
	 *
	 * Service returns E_OS_CALLEVEL
	 *
	 */
	TEST_SET_FIXTURE(OSEK_EV_13, SEQ_NR_01 );
	/* Not applicatable */

	/** @treq OSEK_EV_14
	 *
	 * Call ClearEvent() from extended task
	 *
	 * Requested events are cleared. Service returns E_OK
	 */
	TEST_SET_FIXTURE(OSEK_EV_14, SEQ_NR_01 );
	/* This is already covered in this testsystem */

	/** @treq OSEK_EV_15
	 *
	 * Call GetEvent() with invalid Task ID
	 *
	 * Service returns E_OS_ID
	 */
	TEST_SET_FIXTURE(OSEK_EV_15, SEQ_NR_01 );
	rv = GetEvent(TASK_ID_ILL, &mask);
	TEST_ASSERT( rv == E_OS_ID );

	/** @treq OSEK_EV_16
	 *
	 * Call GetEvent() for basic task
	 *
	 * Service returns E_OS_ACCESS
	 */
	TEST_SET_FIXTURE(OSEK_EV_16, SEQ_NR_01 );
	rv = GetEvent(TASK_ID_btask_h_full, &mask);
	TEST_ASSERT( rv == E_OS_ACCESS);

	/** @treq OSEK_EV_17
	 *
	 * Call GetEvent() for suspended extended task
	 *
	 * Service returns E_OS_STATE
	 */
	TEST_SET_FIXTURE(OSEK_EV_17, SEQ_NR_01 );
	rv = GetEvent(TASK_ID_etask_h_full, &mask);
	TEST_ASSERT( rv == E_OS_STATE);

	/** @treq OSEK_EV_18
	 *
	 * Call GetEvent() for running extended task
	 *
	 * Return current state of all event bits. Service returns E_OK
	 */
	TEST_SET_FIXTURE(OSEK_EV_18, SEQ_NR_01 );
	rv = GetTaskID(&taskId);
	TEST_ASSERT( rv == E_OK);
	rv = GetEvent(taskId, &mask);
	TEST_ASSERT( rv == E_OK);
	TEST_ASSERT( mask == 0 );

	/** @treq OSEK_EV_19
	 *
	 * Call GetEvent() for ready extended task
	 *
	 * Return current state of all event bits. Service returns E_OK
	 */
	TEST_SET_FIXTURE(OSEK_EV_19, SEQ_NR_01 );
	/* Make it ready */
	rv = ActivateTask(TASK_ID_etask_l_full);
	TEST_ASSERT( rv == E_OK);
	/* Set an event */
	rv = SetEvent( TASK_ID_etask_l_full, EVENT_MASK_go2 );
	TEST_ASSERT( rv == E_OK);

	rv = GetEvent( TASK_ID_etask_l_full, &mask );
	TEST_ASSERT( rv == E_OK);
	TEST_ASSERT( mask == EVENT_MASK_go2 );

	TEST_SET_FIXTURE(OSEK_EV_19, SEQ_NR_02 );
	TestTaskRunLowerPrio();
	TEST_SET_FIXTURE(OSEK_EV_19, SEQ_NR_04 );

	/** @treq OSEK_EV_20
	 *
	 * Call GetEvent() for waiting extended task
	 *
	 * Return current state of all event bits. Service returns E_OK
	 */
	TEST_SET_FIXTURE(OSEK_EV_20, SEQ_NR_01 );
	TestActivateAndWait(TASK_ID_etask_l_full);
	TEST_SET_FIXTURE(OSEK_EV_20, SEQ_NR_04 );
	TestTaskRunLowerPrio();
	TEST_SET_FIXTURE(OSEK_EV_20, SEQ_NR_05 );


	/** @treq OSEK_EV_21
	 *
	 * Call WaitEvent() from basic task
	 *
	 * Service returns E_OS_ACCESS
	 */
	TEST_SET_FIXTURE(OSEK_EV_21, SEQ_NR_01 );
	rv = ActivateTask( TASK_ID_btask_h_full );
	TEST_ASSERT( rv == E_OK);
	TEST_SET_FIXTURE(OSEK_EV_21, SEQ_NR_04 );

	/** @treq OSEK_EV_22
	 *
	 * Call WaitEvent() from extended task which occupies a resource
	 *
	 * Service returns E_OS_RESOURCE
	 */
	TEST_SET_FIXTURE(OSEK_EV_22, SEQ_NR_01 );
	rv = GetResource(RES_ID_std_1);
	TEST_ASSERT( rv == E_OK);
	rv = WaitEvent( EVENT_MASK_go );
	TEST_ASSERT( rv == E_OS_RESOURCE);
	rv = ReleaseResource(RES_ID_std_1);
	TEST_ASSERT( rv == E_OK);

	/** @treq OSEK_EV_23
	 *
	 * Call WaitEvent() from ISR2
	 *
	 * Service returns E_OS_CALLEVEL
	 */
	TEST_SET_FIXTURE(OSEK_EV_23, SEQ_NR_01 );
	Irq_GenerateSoftInt( IRQ_SOFTINT_0 );
	TEST_SET_FIXTURE(OSEK_EV_23, SEQ_NR_03 );


	/** @treq OSEK_EV_24
	 * Call WaitEvent() from ISR3
	 *
	 * Service returns E_OS_CALLEVEL
	 */
	TEST_SET_FIXTURE(OSEK_EV_24, SEQ_NR_01 );
	/* Not applicable */

	/** @treq OSEK_EV_25
	 *
	 * Call WaitEvent() from extended task. None of the events waited for is set
	 *
	 * Running task becomes waiting and ready task with highest priority is executed  Service returns E_OK
	 */
	TEST_SET_FIXTURE(OSEK_EV_25, SEQ_NR_01 );
	/* Already by using TestActivateAndWait()  */

	/** @treq OSEK_EV_26
	 *
	 * Call WaitEvent() from extended task. At least one event waited for is already set
	 *
	 * No preemption of running task Service returns E_OK
	 */
	TEST_SET_FIXTURE(OSEK_EV_26, SEQ_NR_01 );
	rv = SetEvent( taskId, EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK);
	rv = WaitEvent( EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK);
	rv = ClearEvent( EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK);
	TEST_SET_FIXTURE(OSEK_EV_26, SEQ_NR_02 );

	TestExit(0);
}
