/*
 * test.c
 *
 *  Created on: 4 aug 2010
 *      Author: mahi
 */

#include "os.h"
#include "test_framework.h"
#include "arc.h"
#include "irq.h"

/*
 * This file tests test requirements OSEK_TM_XX for non-
 *
 *
 * Priorities:
 * ll - low low prio (used for scheduling only)
 * l  - low prio
 * m  - medium
 * h  - high prio
 */


enum OsekFixtureNr {
	OSEK_TM_01 = 1,
	OSEK_TM_02,
	OSEK_TM_03,
	OSEK_TM_04,
	OSEK_TM_05,
	OSEK_TM_06,
	OSEK_TM_07,
	OSEK_TM_08,
	OSEK_TM_09,
	OSEK_TM_10,
	OSEK_TM_11,
	OSEK_TM_12,
	OSEK_TM_13,
	OSEK_TM_14,
	OSEK_TM_15,
	OSEK_TM_16,
	OSEK_TM_17,
	OSEK_TM_18,
	OSEK_TM_19,
	OSEK_TM_20,
	OSEK_TM_21,
	OSEK_TM_22,
	OSEK_TM_23,
	OSEK_TM_24,
	OSEK_TM_25,
	OSEK_TM_26,
	OSEK_TM_27,
	OSEK_TM_28,
	OSEK_TM_29,
	OSEK_TM_30,
	OSEK_TM_31,
	OSEK_TM_32,
	OSEK_TM_33,
	OSEK_TM_34,
	OSEK_TM_35,
	OSEK_TM_36,
	OSEK_TM_37,
	OSEK_TM_38,
	OSEK_TM_39,
	OSEK_TM_40,
	OSEK_TM_41,
	OSEK_TM_42,
	OSEK_TM_43,
};

TestWorldType TestWorld = {0};

uint32_t testNrNon = 0;

static uint8_t activations = 0;


static void isrSoftInt0( void ) {
	StatusType rv;
	TaskType taskId;

	switch ( TestWorld.fixtureNr ) {
	case OSEK_TM_20:
		TEST_SET_FIXTURE(OSEK_TM_20, SEQ_NR_02);
		rv = TerminateTask();
		TEST_ASSERT( rv == E_OS_CALLEVEL );
		TEST_SET_FIXTURE(OSEK_TM_20, SEQ_NR_03);
		break;
	case OSEK_TM_25:
		TEST_SET_FIXTURE(OSEK_TM_25, SEQ_NR_02);
		rv = ChainTask(TASK_ID_OsIdle);
		TEST_ASSERT( rv == E_OS_CALLEVEL );
		TEST_SET_FIXTURE(OSEK_TM_25, SEQ_NR_03);
		break;
	case OSEK_TM_35:
		TEST_SET_FIXTURE(OSEK_TM_35, SEQ_NR_02);
		rv = Schedule();
		TEST_ASSERT( rv == E_OS_CALLEVEL );
		TEST_SET_FIXTURE(OSEK_TM_35, SEQ_NR_03);
		break;
	case OSEK_TM_37:
		TEST_SET_FIXTURE(OSEK_TM_37, SEQ_NR_02);
		rv = GetTaskID(&taskId);
		TEST_ASSERT( rv == E_OS_CALLEVEL );
		TEST_SET_FIXTURE(OSEK_TM_37, SEQ_NR_03);
		break;
	default:
		TEST_ASSERT( 0 );
	}

}

void OsIdle(void) {
	while(1);
}

void etask_m_none ( void ) {
	StatusType rv;
	// 2. Call ActivateTask() from non-preemptive task on basic task
	rv = ActivateTask(TASK_ID_btask_m_full);
	TEST_ASSERT( rv == E_OK);
}

/*
 * ActivationLimit = 2
 */
void btask_l_full(void) {
	StatusType rv;

	switch ( TestWorld.fixtureNr ) {
	case OSEK_TM_04:
		TEST_SET_FIXTURE(OSEK_TM_04, SEQ_NR_03);
		break;
	case OSEK_TM_10:
		activations++;
		switch( activations ) {
		case 1:
			TEST_SET_FIXTURE(OSEK_TM_10, SEQ_NR_02);
			break;
		case 2:
			TEST_SET_FIXTURE(OSEK_TM_10, SEQ_NR_03);
			break;
		default:
			TEST_ASSERT( 0 );
		}
		break;
	case OSEK_TM_13:
	{	/* 2 activations */
		activations++;
		break;
	}
	case OSEK_TM_28:
		TEST_SET_FIXTURE(OSEK_TM_28, SEQ_NR_05 );
		rv = SetEvent( TASK_ID_etask_m_full,EVENT_MASK_go );
		break;
	case OSEK_TM_30:
		activations++;
		if( activations == 1) {
			TEST_SET_FIXTURE(OSEK_TM_30, SEQ_NR_02);
		} else if( activations == 2) {
			TEST_SET_FIXTURE(OSEK_TM_30, SEQ_NR_03);
		} else {
			TEST_ASSERT( 0 );
		}
		break;
	case OSEK_TM_32:
		activations++;
		if( activations == 1) {
			TEST_SET_FIXTURE(OSEK_TM_32, SEQ_NR_05);
		} else if( activations == 2) {
			TEST_SET_FIXTURE(OSEK_TM_32, SEQ_NR_06);
		} else {
			TEST_ASSERT( 0 );
		}
		break;
	case OSEK_TM_33:
		TEST_SET_FIXTURE(OSEK_TM_33, SEQ_NR_02);
		rv = ChainTask( TASK_ID_etask_m_full );
		TEST_ASSERT( rv == E_OS_LIMIT );
		TEST_SET_FIXTURE(OSEK_TM_33, SEQ_NR_03);
		rv = SetEvent( TASK_ID_etask_m_full, EVENT_MASK_go);
		TEST_ASSERT( rv == E_OK );
		break;
	default:
		TEST_ASSERT( 0 );
	}
	TerminateTask();
}

/*
 * ActivationLimit = 2
 */
void btask_m_full( void ) {
	StatusType rv;
	TaskType taskId;

	switch ( TestWorld.fixtureNr ) {
	case OSEK_TM_05:
		TEST_SET_FIXTURE(OSEK_TM_05, SEQ_NR_03);
		break;

	case OSEK_TM_14:
		activations++;
		switch(activations) {
		case 1:
			TEST_SET_FIXTURE(OSEK_TM_14, SEQ_NR_03 );
			break;
		case 2:
			TEST_SET_FIXTURE(OSEK_TM_14, SEQ_NR_04 );
			break;
		default:
			TEST_ASSERT( 0 );
		}
		break;
	case OSEK_TM_15:
		activations++;
		switch(activations) {
		case 1:
			rv = GetTaskID(&taskId);
			TEST_ASSERT(rv == E_OK);
			/* Activate ourself */
			rv = ActivateTask(taskId);
			TEST_ASSERT(rv == E_OK);

			rv = ActivateTask(taskId);
			TEST_ASSERT(rv == E_OS_LIMIT);
			TEST_SET_FIXTURE(OSEK_TM_15, SEQ_NR_02);
			break;

		case 2:
			TEST_SET_FIXTURE(OSEK_TM_15, SEQ_NR_03);

			/* back to test task */
			rv = SetEvent(TASK_ID_etask_m_full, EVENT_MASK_go );
			TEST_ASSERT( rv == E_OK );
			break;

		default:
			TEST_ASSERT( 0 );
			break;
		}
		break;
	case OSEK_TM_28:
		TEST_SET_FIXTURE(OSEK_TM_28, SEQ_NR_04 );
		break;
	default:
		TEST_ASSERT( 0 );
	}
	TerminateTask();
}

void etask_h_full( void ) {
	StatusType rv;
	switch(TestWorld.fixtureNr) {
	case OSEK_TM_06:
		TEST_SET_FIXTURE(OSEK_TM_06, SEQ_NR_02 );
		break;
	case OSEK_TM_07:
		TEST_SET_FIXTURE(OSEK_TM_07, SEQ_NR_02 );
		break;
	case OSEK_TM_19:
		TEST_SET_FIXTURE(OSEK_TM_19, SEQ_NR_02 );
		rv = WaitEvent(EVENT_MASK_go);
		TEST_ASSERT( rv == E_OK );
		rv = ClearEvent(EVENT_MASK_go);
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_TM_19, SEQ_NR_04 );
		break;
	default:
		TEST_ASSERT( 0 );
		break;
	}
	TerminateTask();
}

void etask_l_full( void ) {

	switch(TestWorld.fixtureNr) {
	case OSEK_TM_08:
		TEST_SET_FIXTURE(OSEK_TM_08, SEQ_NR_03 );
		break;
	case OSEK_TM_11:
		TEST_SET_FIXTURE(OSEK_TM_11, SEQ_NR_03 );
		break;
	case OSEK_TM_31:
		TEST_SET_FIXTURE(OSEK_TM_31, SEQ_NR_03 );
		break;
	default:
		TEST_ASSERT( 0 );
		break;
	}
	TerminateTask();
}

void btask_h_full( void ) {
	StatusType rv;
	TaskType taskId;

	switch(TestWorld.fixtureNr) {
	case OSEK_TM_02:
		TEST_SET_FIXTURE(OSEK_TM_02, SEQ_NR_02 );
		break;
	case OSEK_TM_03:
		TEST_SET_FIXTURE(OSEK_TM_03, SEQ_NR_02 );
		break;
	case OSEK_TM_10:
		TEST_SET_FIXTURE(OSEK_TM_10, SEQ_NR_02 );
		break;
	case OSEK_TM_12:
		TEST_SET_FIXTURE(OSEK_TM_12, SEQ_NR_03 );
		break;
	case OSEK_TM_28:
		TEST_SET_FIXTURE(OSEK_TM_28, SEQ_NR_02 );
		rv = ActivateTask( TASK_ID_btask_m_full);
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_TM_28, SEQ_NR_03 );
		/* Terminate this task an:
		 * 1. run m task
		 * 2. run l task
		 */
		rv = ChainTask( TASK_ID_btask_l_full );
		TEST_ASSERT( 0 );
		break;
	case OSEK_TM_29:
		activations++;
		if( activations == 1) {
			TEST_SET_FIXTURE(OSEK_TM_29, SEQ_NR_02 );
			rv = GetTaskID(&taskId);
			TEST_ASSERT( rv == E_OK );
			rv = ChainTask( taskId );
			TEST_ASSERT( 0 );
		} else if(activations == 2) {
			TEST_SET_FIXTURE(OSEK_TM_29, SEQ_NR_03 );
		} else {
			TEST_ASSERT( 0 );
		}
		break;
	case OSEK_TM_32:
		TEST_SET_FIXTURE(OSEK_TM_32, SEQ_NR_04 );
		break;
	default:
		TEST_ASSERT( 0 );
	}
	TerminateTask();
}

void etask_m_full_2( void ) {

	switch(TestWorld.fixtureNr) {
	case OSEK_TM_09:
		TEST_SET_FIXTURE(OSEK_TM_09, SEQ_NR_03 );
		break;
	default:
		TEST_ASSERT( 0 );
		break;
	}

	TerminateTask();
}


/**
 * Task used for scheduling only.
 */
void btask_ll_non( void ) {
	StatusType rv;
	/* Used for scheduling ONLY */
	rv = SetEvent(TASK_ID_etask_m_full, EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );
}

/* Terminate a lower-priority task.
 *
 *
 * */
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

void btask_m_non( void ) {
	StatusType rv;
	EventMaskType mask;
	testNrNon = TestWorld.testNr;
	TaskType taskId;

	switch ( TestWorld.fixtureNr ) {
	case OSEK_TM_02:

		/** @treq OSEK_TM_02
		 *
		 * Call ActivateTask() from non-preemptive task on suspended basic task
		 *
		 * No preemption of running task. Activated task becomes ready.
		 * Service returns E_OK
		 */
		rv = ActivateTask(TASK_ID_btask_h_full);
		TEST_ASSERT( rv == E_OK );
		/* Run the higher prio task */
		rv = Schedule();
		TEST_ASSERT( rv == E_OK );
		break;

	case OSEK_TM_06:
		/** @treq OSEK_TM_06
		 *
		 * Call ActivateTask() from non-preemptive task on suspended extended task
		 *
		 * No preemption of running task. Activated task becomes ready and its
		 * events are cleared. Service returns E_OK
		 */

		rv = ActivateTask(TASK_ID_etask_h_full);
		rv = GetEvent(TASK_ID_etask_h_full,&mask);
		TEST_ASSERT( rv == E_OK );
		TEST_ASSERT( mask == 0);
		/* Run the higher prio task */
		rv = Schedule();
		TEST_ASSERT( rv == E_OK );
		break;

	case OSEK_TM_12:
		/** @treq OSEK_TM_12
		 *
		 * Call ActivateTask() from non-preemptive task on ready basic task
		 * which has not reached max number of activations
		 *
		 * No preemption of running task. Activation request is queued in ready
		 * list. Service returns E_OK
		 */
		rv = ActivateTask(TASK_ID_btask_h_full);
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_TM_12, SEQ_NR_02 );
		/* Run the higher prio task */
		rv = Schedule();
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_TM_12, SEQ_NR_04 );
		break;

	case OSEK_TM_17:
	{

		/** @treq OSEK_TM_17
		 *
		 * Call ActivateTask() from non-preemptive task on running basic
		 * task which has not reached max number of activations
		 *
		 * No preemption of running task. Activation request is queued in ready list.
		 * Service returns E_OK
		 */
		activations++;
		switch( activations ) {
		case 1:
			rv = GetTaskID(&taskId);
			TEST_ASSERT(rv == E_OK);
			/* Run us again */
			rv = ActivateTask(taskId);
			TEST_ASSERT( rv == E_OK );
			TEST_SET_FIXTURE(OSEK_TM_17, SEQ_NR_02 );
			TerminateTask();
			break;
		case 2:
			TEST_SET_FIXTURE(OSEK_TM_17, SEQ_NR_03 );
			break;
		default:
			TEST_ASSERT( 0 );
		}

	}
		break;

	case OSEK_TM_32:

		/** @treq OSEK_TM_32
		 *
		 * Call ChainTask() from non-preemptive task on ready basic task
		 * which has not reached max. Number of activations
		 *
		 * Running task is terminated, activation request is queued in
		 * ready list and ready task with highest priority is executed
		 */
		activations = 0;
		TEST_SET_FIXTURE(OSEK_TM_32, SEQ_NR_02 );
		/* Activate higher prio task */
		rv = SetEvent(TASK_ID_etask_m_full,EVENT_MASK_go);
		TEST_ASSERT( rv == E_OK );

		TEST_SET_FIXTURE(OSEK_TM_32, SEQ_NR_03 );
		/* Make it READY */
		rv = ActivateTask( TASK_ID_btask_l_full );
		TEST_ASSERT( rv == E_OK );

		rv = ChainTask( TASK_ID_btask_l_full );
		TEST_ASSERT( 0 );
		break;
	default:
		TEST_ASSERT( 0 );
	}

	/* Back to etask_m_full */
	rv = SetEvent(TASK_ID_etask_m_full,EVENT_MASK_go);
	TEST_ASSERT( rv == E_OK );
	TerminateTask();
	TEST_ASSERT( 0 );
}

/**
 * The master in the tests
 */
void etask_m_full( void ) {

	StatusType rv;
	TaskType taskId;
	TaskType isrTask;
	TaskStateType taskState;

	/** @treq OSEK_TM_01
	 *
	 * Call ActivateTask() from task-level with invalid task ID
	 * (task does not exist)
	 *
	 * Service returns E_OS_ID
	 */
	TEST_SET_FIXTURE(OSEK_TM_01, SEQ_NR_01 );
	rv = ActivateTask(TASK_ID_ILL);
	TEST_ASSERT( rv == E_OS_ID);

	/* OSEK_TM_02 is in another task */
	TEST_SET_FIXTURE(OSEK_TM_02, SEQ_NR_01 );
	TestActivateAndWait(TASK_ID_btask_m_non);

	/** @treq OSEK_TM_03
	 *
	 * Call ActivateTask() from	preemptive task on basic task which
	 * has higher priority than running task.
	 *
	 * Running task is preempted. Activated task becomes running.
	 * Service returns E_OK
	 * */
	TEST_SET_FIXTURE(OSEK_TM_03, SEQ_NR_01 );
	rv = ActivateTask(TASK_ID_btask_h_full);
	TEST_ASSERT( rv == E_OK);

	/* The higher prio task is run prior to getting here */
	TEST_SET_FIXTURE(OSEK_TM_03, SEQ_NR_03 );

	/** @treq OSEK_TM_04
	 *
	 * Call ActivateTask() from preemptive task on suspended basic
     * task which has lower priority than running task.
     *
     * No preemption of running task. Activated task becomes ready.
     * Service returns E_OK */

	TEST_SET_FIXTURE(OSEK_TM_04, SEQ_NR_01 );
	rv = ActivateTask(TASK_ID_btask_l_full);
	TEST_ASSERT( rv == E_OK);
	TEST_SET_FIXTURE(OSEK_TM_04, SEQ_NR_02 );

	/* Cleanup, let btask_l_full run */
	TestTaskRunLowerPrio();
	TEST_SET_FIXTURE(OSEK_TM_04, SEQ_NR_04 );

	/** @treq OSEK_TM_05
	 *  Call ActivateTask() from preemptive task on suspended basic
	 * task which has equal priority as running task.
	 *
	 * No preemption of running task. Activated task becomes ready.
     * Service returns E_OK
     */
	TEST_SET_FIXTURE(OSEK_TM_05, SEQ_NR_01 );
	rv = ActivateTask(TASK_ID_btask_m_full);
	TEST_ASSERT( rv == E_OK);
	TEST_SET_FIXTURE(OSEK_TM_05, SEQ_NR_02 );

	/* Cleanup, let btask_m_full run */
	TestTaskRunLowerPrio();
	TEST_SET_FIXTURE(OSEK_TM_05, SEQ_NR_04 );


	/* OSEK_TM_06 is in another task */
	TEST_SET_FIXTURE(OSEK_TM_06, SEQ_NR_01 );
	TestActivateAndWait(TASK_ID_btask_m_non);

	/** @treq OSEK_TM_07
	 *
	 * Call ActivateTask() from preemptive task on suspended
     * extended task which has higher priority than running task.
     *
     * Running task is preempted. Activated task becomes running and
     * its events are cleared. Service returns E_OK
	 */
	TEST_SET_FIXTURE(OSEK_TM_07, SEQ_NR_01 );
	rv =  ActivateTask(TASK_ID_etask_h_full);
	TEST_ASSERT( rv == E_OK);
	TEST_SET_FIXTURE(OSEK_TM_07, SEQ_NR_03 );

	/** @treq OSEK_TM_08
	 * Call ActivateTask() from preemptive task on suspended
     * extended task which has lower priority than running task.
	 *
	 * No preemption of running task. Activated task becomes ready
	 * and its events are cleared. Service returns E_OK
	 */
	TEST_SET_FIXTURE(OSEK_TM_08, SEQ_NR_01 );
	rv =  ActivateTask(TASK_ID_etask_l_full);
	TEST_SET_FIXTURE(OSEK_TM_08, SEQ_NR_02 );
	TestTaskRunLowerPrio();
	TEST_SET_FIXTURE(OSEK_TM_08, SEQ_NR_04 );

	/** @treq OSEK_TM_09
	 *
	 * Call ActivateTask() from preemptive task on suspended
	 * extended task which has equal priority as running task.
	 *
	 * No preemption of running task. Activated task becomes
	 * ready and its events are cleared. Service returns E_OK.
	 */

	TEST_SET_FIXTURE(OSEK_TM_09, SEQ_NR_01 );
	rv =  ActivateTask(TASK_ID_etask_m_full_2);
	TEST_ASSERT( rv == E_OK);
	TEST_SET_FIXTURE(OSEK_TM_09, SEQ_NR_02 );
	TestTaskRunLowerPrio();
	TEST_SET_FIXTURE(OSEK_TM_09, SEQ_NR_04 );

	/** @treq OSEK_TM_10
	 *
	 * Call ActivateTask() on ready basic task which has reached max
	 * number of activations
	 *
	 * Service returns E_OS_LIMIT
	 */
	TEST_SET_FIXTURE(OSEK_TM_10, SEQ_NR_01 );
	activations = 0;
	rv =  ActivateTask(TASK_ID_btask_l_full);
	TEST_ASSERT( rv == E_OK);
	rv =  ActivateTask(TASK_ID_btask_l_full);
	TEST_ASSERT( rv == E_OK);
	rv =  ActivateTask(TASK_ID_btask_l_full);
	TEST_ASSERT( rv == E_OS_LIMIT);
	TestTaskRunLowerPrio();
	TEST_SET_FIXTURE(OSEK_TM_10, SEQ_NR_04);

	/** @treq OSEK_TM_11
	 *
	 * Call ActivateTask() on ready extended task
	 *
	 * Service returns E_OS_LIMIT
	 */
	TEST_SET_FIXTURE(OSEK_TM_11, SEQ_NR_01 );
	/* Make ready */
	rv =  ActivateTask(TASK_ID_etask_l_full);
	TEST_ASSERT( rv == E_OK);
	TEST_SET_FIXTURE(OSEK_TM_11, SEQ_NR_02 );
	rv =  ActivateTask(TASK_ID_etask_l_full);
	TEST_ASSERT( rv == E_OS_LIMIT);
	TestTaskRunLowerPrio();
	TEST_SET_FIXTURE(OSEK_TM_11, SEQ_NR_04 );

	/* OSEK_TM_12 is in another task */
	TEST_SET_FIXTURE(OSEK_TM_12, SEQ_NR_01 );
	TestActivateAndWait(TASK_ID_btask_m_non);


	/** @treq OSEK_TM_13
	 *
	 * Call ActivateTask() from preemptive task on ready basic task which has
	 * not reached max number of activations and has lower priority than
	 * running task
	 *
	 * No preemption of running task. Activation request is queued in ready
	 * list. Service returns E_OK
	 */
	activations = 0;
	TEST_SET_FIXTURE(OSEK_TM_13, SEQ_NR_01 );
	/* make ready */
	rv =  ActivateTask(TASK_ID_btask_l_full);
	TEST_ASSERT( rv == E_OK);
	/* Activate ready basic task */
	rv =  ActivateTask(TASK_ID_btask_l_full);
	TEST_ASSERT( rv == E_OK);
	TestTaskRunLowerPrio();
	TEST_ASSERT( activations == 2 );


	/** @treq OSEK_TM_14
	 *
	 * Call ActivateTask() from preemptive task on ready basic task
	 * which has not reached max number of activations and has equal
	 * priority as running task
	 *
	 * No preemption of running task.Activation request is queued in ready
	 * list. Service returns E_OK
	 */
	activations = 0;
	TEST_SET_FIXTURE(OSEK_TM_14, SEQ_NR_01 );
	/* make ready */
	rv =  ActivateTask(TASK_ID_btask_m_full);
	TEST_ASSERT( rv == E_OK);
	/* Activate ready basic task */
	rv =  ActivateTask(TASK_ID_btask_m_full);
	TEST_ASSERT( rv == E_OK);
	TEST_SET_FIXTURE(OSEK_TM_14, SEQ_NR_02 );
	TestTaskRunLowerPrio();
	TEST_ASSERT( activations == 2 );
	TEST_SET_FIXTURE(OSEK_TM_14, SEQ_NR_05 );


	/** @treq OSEK_TM_15
	 *
	 * Call ActivateTask() on running basic task which has reached max
	 * number of activations
	 *
	 * Service returns E_OS_LIMIT
	 */
	/* OSEK_TM_15 is in another task */
	activations = 0;
	TEST_SET_FIXTURE(OSEK_TM_15, SEQ_NR_01 );
	TestActivateAndWait(TASK_ID_btask_m_full);

	/** @treq OSEK_TM_16
	 *
	 * Call ActivateTask() on running extended task
	 *
	 * Service returns E_OS_LIMIT
	 */
	TEST_SET_FIXTURE(OSEK_TM_16, SEQ_NR_01 );
	rv = GetTaskID(&taskId);
	TEST_ASSERT(rv == E_OK);
	/* Activate ourself */
	rv = ActivateTask(taskId);
	TEST_ASSERT(rv == E_OS_LIMIT);

	/* OSEK_TM_17 is in another task */
	activations = 0;
	TEST_SET_FIXTURE(OSEK_TM_17, SEQ_NR_01 );
	TestActivateAndWait(TASK_ID_btask_m_non);
	TEST_SET_FIXTURE(OSEK_TM_17, SEQ_NR_04 );

	/** @treq OSEK_TM_18
	 *
	 * Call ActivateTask() from preemptive task on running basic task
	 * which has not reached max number of activations
	 *
	 * No preemption of running task. Activation request is queued in
	 * ready list. Service returns E_OK
	 */

	/* This is done in OSEK_TM_15, just record that that the test is OK */
	TEST_SET_FIXTURE(OSEK_TM_18, SEQ_NR_01 );

	/** @treq OSEK_TM_19
	 *
	 * Call ActivateTask() on waiting extended task
	 *
	 * Service returns E_OS_LIMIT
	 */
	TEST_SET_FIXTURE(OSEK_TM_19, SEQ_NR_01 );
	rv = ActivateTask( TASK_ID_etask_h_full );
	/* etask_h_full should now be waiting */
	TEST_SET_FIXTURE(OSEK_TM_19, SEQ_NR_03 );

	rv = ActivateTask( TASK_ID_etask_h_full );
	TEST_ASSERT( rv == E_OS_LIMIT );

	/* Make it terminate */
	rv = SetEvent( TASK_ID_etask_h_full,EVENT_MASK_go);
	TEST_ASSERT( rv == E_OK);

	TEST_SET_FIXTURE(OSEK_TM_19, SEQ_NR_05 );

	/** @treq OSEK_TM_20
	 *
	 * Call TerminateTask() from ISR category 2
	 *
	 * Service returns E_OS_CALLEVEL
	 *
	 */

	TEST_SET_FIXTURE(OSEK_TM_20, SEQ_NR_01 );

	/* Create an ISR2 */
	isrTask = Os_Arc_CreateIsr( isrSoftInt0, 8/*prio*/,"soft_0");
	Irq_AttachIsr2(isrTask,NULL, IRQ_SOFTINT_0);

	Irq_GenerateSoftInt( IRQ_SOFTINT_0 );

	TEST_SET_FIXTURE(OSEK_TM_20, SEQ_NR_04);

	/* OSEK_TM_21 not in Autosar */
	TEST_SET_FIXTURE(OSEK_TM_21, SEQ_NR_01 );

	/** @treq OSEK_TM_22
	 *
	 * Call TerminateTask() while still occupying a resource
	 *
	 * Running task is not terminated.
	 * Service returns E_OS_RESOURCE
	 */

	TEST_SET_FIXTURE(OSEK_TM_22, SEQ_NR_01 );
	rv = GetResource( RES_ID_std_1 );
	TEST_ASSERT( rv == E_OK );
	rv = TerminateTask();
	TEST_ASSERT( rv == E_OS_RESOURCE );
	rv = ReleaseResource( RES_ID_std_1 );
	TEST_SET_FIXTURE(OSEK_TM_22, SEQ_NR_02 );

	/** @treq OSEK_TM_23
	 *
	 * Call TerminateTask()
	 *
	 * Running task is terminated and ready task with highest priority
	 * is executed
	 */

	/* This is already verified by the test-system */
	TEST_SET_FIXTURE(OSEK_TM_23, SEQ_NR_01 );

	/** @treq OSEK_TM_24
	 *
	 * Call ChainTask() from task-level. Task-ID is invalid (does not exist).
	 *
	 * Service returns E_OS_ID
	 */

	TEST_SET_FIXTURE(OSEK_TM_24, SEQ_NR_01 );
	rv = ChainTask(TASK_ID_ILL);
	TEST_ASSERT( rv == E_OS_ID );
	TEST_SET_FIXTURE(OSEK_TM_24, SEQ_NR_02 );

	/** @treq OSEK_TM_25
	 *
	 * Call ChainTask() from ISR category 2
	 *
	 * Service returns E_OS_CALLEVEL
	 */
	TEST_SET_FIXTURE(OSEK_TM_25, SEQ_NR_01 );
	Irq_GenerateSoftInt( IRQ_SOFTINT_0 );
	TEST_SET_FIXTURE(OSEK_TM_25, SEQ_NR_04 );

	/* OSEK_TM_26 not in Autosar */
	TEST_SET_FIXTURE(OSEK_TM_26, SEQ_NR_01 );

	/** @treq OSEK_TM_27
	 *
	 * Call ChainTask() while still occupying a resource
	 *
	 * Running task is not terminated. Service returns E_OS_RESOURCE
	 */

	TEST_SET_FIXTURE(OSEK_TM_27, SEQ_NR_01 );
	rv = GetResource( RES_ID_std_1 );
	TEST_ASSERT( rv == E_OK );
	rv = ChainTask(TASK_ID_OsIdle);
	TEST_ASSERT( rv == E_OS_RESOURCE );
	rv = ReleaseResource( RES_ID_std_1 );
	TEST_SET_FIXTURE(OSEK_TM_27, SEQ_NR_02 );


	/** @treq OSEK_TM_28
	 *
	 * Call ChainTask() on suspended task
	 *
	 * Running task is terminated, chained task becomes ready and ready task
	 * with highest priority is executed
	 */
	TEST_SET_FIXTURE(OSEK_TM_28, SEQ_NR_01 );
	TestActivateAndWait(TASK_ID_btask_h_full);
	TEST_SET_FIXTURE(OSEK_TM_28, SEQ_NR_06 );
	/* Cleanup */
	TestTaskRunLowerPrio();

	/** @treq OSEK_TM_29
	 *
	 * Call ChainTask() on running task
	 *
	 * Running task is terminated, chained task becomes ready and ready task
	 * with highest priority is executed
	 */
	activations = 0;
	TEST_SET_FIXTURE(OSEK_TM_29, SEQ_NR_01 );
	rv = ActivateTask(TASK_ID_btask_h_full);
	TEST_ASSERT( rv == E_OK );
	TEST_SET_FIXTURE(OSEK_TM_29, SEQ_NR_04 );

	/** @treq OSEK_TM_30
	 *
	 * Call ChainTask() on ready basic task which has reached max number of
	 * activations.
	 *
	 * Running task is not terminated. Service returns E_OS_LIMIT
	 */

	activations = 0;
	TEST_SET_FIXTURE(OSEK_TM_30, SEQ_NR_01 );
	/* First activation */
	rv = ActivateTask(TASK_ID_btask_l_full);
	TEST_ASSERT( rv == E_OK );
	/* second activation */
	rv = ActivateTask(TASK_ID_btask_l_full);
	TEST_ASSERT( rv == E_OK );

	rv = ChainTask(TASK_ID_btask_l_full);
	TEST_ASSERT( rv == E_OS_LIMIT );
	/* Cleanup */
	TestTaskRunLowerPrio();
	TEST_SET_FIXTURE(OSEK_TM_30, SEQ_NR_04);

	/** @treq OSEK_TM_31
	 *
	 * Call ChainTask() on ready extended task
	 *
	 * Running task is not terminated. Service returns E_OS_LIMIT
	 */
	TEST_SET_FIXTURE(OSEK_TM_31, SEQ_NR_01 );
	rv = ActivateTask( TASK_ID_etask_l_full );
	TEST_ASSERT( rv == E_OK );
	rv = ChainTask( TASK_ID_etask_l_full );
	TEST_ASSERT( rv == E_OS_LIMIT );
	TEST_SET_FIXTURE(OSEK_TM_31, SEQ_NR_02 );
	/* Cleanup */
	TestTaskRunLowerPrio();
	TEST_SET_FIXTURE(OSEK_TM_31, SEQ_NR_04);

	/** @treq OSEK_TM_32
	 *
	 * Call ChainTask() from non-preemptive task on ready basic task which
	 * has not reached max  number of activations
	 *
	 * Running task is terminated, activation request is queued in ready list
	 * and ready task with highest priority is executed
	 */
	TEST_SET_FIXTURE(OSEK_TM_32, SEQ_NR_01 );
	TestActivateAndWait(TASK_ID_btask_m_non);
	TEST_SET_FIXTURE(OSEK_TM_32, SEQ_NR_04 );
	TestTaskRunLowerPrio();
	TEST_SET_FIXTURE(OSEK_TM_32, SEQ_NR_07);

	/** @treq OSEK_TM_33
	 *
	 * Call ChainTask() on waiting extended task
	 *
	 * Service returns E_OS_LIMIT
	 */
	TEST_SET_FIXTURE(OSEK_TM_33, SEQ_NR_01 );
	TestActivateAndWait(TASK_ID_btask_l_full);
	TEST_SET_FIXTURE(OSEK_TM_33, SEQ_NR_04);
	TestTaskRunLowerPrio();

	/** @treq OSEK_TM_34
	 *
	 * Call Schedule() from task.
	 *
	 * Ready task with highest priority is executed. Service returns E_OK
	 */
	/* Already tested in OSEK_TM_02 */
	TEST_SET_FIXTURE(OSEK_TM_34, SEQ_NR_01 );

	/** @treq OSEK_TM_35
	 *
	 * Call Schedule() from ISR category 2
	 *
	 * Service returns E_OS_CALLEVEL
	 */
	TEST_SET_FIXTURE(OSEK_TM_35, SEQ_NR_01 );
	Irq_GenerateSoftInt( IRQ_SOFTINT_0 );
	TEST_SET_FIXTURE(OSEK_TM_35, SEQ_NR_04 );

	/* OSEK_TM_36 not an OSEK/Autosar requirement */
	TEST_SET_FIXTURE(OSEK_TM_36, SEQ_NR_01 );

	/** @treq OSEK_TM_37
	 *
	 * Call GetTaskID() from ISR category 2
	 *
	 * Service returns E_OS_CALLEVEL
	 */
	TEST_SET_FIXTURE(OSEK_TM_37, SEQ_NR_01 );
	Irq_GenerateSoftInt( IRQ_SOFTINT_0 );
	TEST_SET_FIXTURE(OSEK_TM_37, SEQ_NR_04 );

	/* @treq OSEK_TM_38 not an OSEK/Autosar requirement */
	TEST_SET_FIXTURE(OSEK_TM_38, SEQ_NR_01 );

	/** @treq OSEK_TM_39
	 *
	 * Call GetTaskID() from task
	 *
	 * Return task ID of currently running task. Service returns E_OK
	 */
	/* Already tested in numerous earlier testcases */
	TEST_SET_FIXTURE(OSEK_TM_39, SEQ_NR_01 );


	/** @treq OSEK_TM_40
	 *
	 * Call GetTaskState() with invalid task ID (task does not exist)
	 *
	 * Service returns E_OS_ID
	 */
	TEST_SET_FIXTURE(OSEK_TM_40, SEQ_NR_01 );
	rv = GetTaskState(TASK_ID_ILL,&taskState);
	TEST_ASSERT( rv == E_OS_ID );
	TEST_SET_FIXTURE(OSEK_TM_40, SEQ_NR_02 );


	/** @treq OSEK_TM_41
	 *
	 * Call GetTaskState()
	 *
	 *  Return state of queried task. Service returns E_OK
	 */

	TEST_SET_FIXTURE(OSEK_TM_41, SEQ_NR_01 );
	rv = GetTaskState(TASK_ID_etask_h_full,&taskState);
	TEST_ASSERT( rv == E_OK );
	TEST_ASSERT( taskState  == TASK_STATE_SUSPENDED );
	TEST_SET_FIXTURE(OSEK_TM_41, SEQ_NR_02 );

	rv = GetTaskState(TASK_ID_etask_l_full,&taskState);
	TEST_ASSERT( rv == E_OK );
	TEST_ASSERT( taskState  == TASK_STATE_SUSPENDED );
	TEST_SET_FIXTURE(OSEK_TM_41, SEQ_NR_03 );

	rv = GetTaskState(taskId,&taskState);
	TEST_ASSERT( rv == E_OK );
	TEST_ASSERT( taskState  == TASK_STATE_RUNNING );
	TEST_SET_FIXTURE(OSEK_TM_41, SEQ_NR_04 );

//	TestActivateAndWait( TASK_ID_btask_m_non );

	TestExit(0);
}
