/*
 *  Created on: 4 aug 2010
 *      Author: mahi
 */
/*
 * This file tests test requirements OSEK_RM_XX.
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
	OSEK_RM_01 = 1,
	OSEK_RM_02,
	OSEK_RM_03,
	OSEK_RM_04,
	OSEK_RM_05,
	OSEK_RM_06,
	OSEK_RM_07,
	OSEK_RM_08,
	OSEK_RM_09,
	OSEK_RM_10,
	OSEK_RM_11,
	OSEK_RM_12,
	OSEK_RM_13,
	OSEK_RM_14,
	OSEK_RM_15,
	OSEK_RM_16,
};

TestWorldType TestWorld = {0};

uint32_t testNrNon = 0;


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

#if 0
static void isrSoftInt1( void ) {
	switch ( TestWorld.fixtureNr ) {
	default:
		TEST_ASSERT( 0 );
		break;
	}
}

static void isrSoftInt0( void ) {
	StatusType rv;
	TaskType taskId;

	switch ( TestWorld.fixtureNr ) {
	default:
		TEST_ASSERT( 0 );
	}

}
#endif

void OsIdle(void) {
	while(1);
}


void btask_l_non ( void ) {
	StatusType rv;
	switch ( TestWorld.fixtureNr ) {
	case OSEK_RM_06:
		TEST_SET_FIXTURE(OSEK_RM_06, SEQ_NR_02 );
		rv = GetResource(RES_ID_std_h);
		TEST_ASSERT( rv == E_OK);
		rv = ActivateTask( TASK_ID_btask_m_non );
		TEST_ASSERT( rv == E_OK);
		rv = Schedule();
		TEST_ASSERT( rv == E_OS_RESOURCE);
		TEST_SET_FIXTURE(OSEK_RM_06, SEQ_NR_03 );
		rv = ReleaseResource(RES_ID_std_h);
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_RM_06, SEQ_NR_04 );
		break;
	case OSEK_RM_15:
		TEST_SET_FIXTURE(OSEK_RM_15, SEQ_NR_02 );
		rv = GetResource(RES_SCHEDULER);
		TEST_ASSERT( rv == E_OK);
		rv = ActivateTask( TASK_ID_btask_m_non );
		TEST_ASSERT( rv == E_OK);
		rv = Schedule();
		TEST_ASSERT( rv == E_OS_RESOURCE);
		TEST_SET_FIXTURE(OSEK_RM_15, SEQ_NR_03 );
		rv = ReleaseResource(RES_SCHEDULER);
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_RM_15, SEQ_NR_04 );
		break;
	default:
		TEST_ASSERT( 0 );
	}
	rv = TerminateTask();
	TEST_ASSERT( rv == E_OK );
}

void btask_m_non ( void ) {
	StatusType rv;
	switch ( TestWorld.fixtureNr ) {
	case OSEK_RM_06:
		TEST_SET_FIXTURE(OSEK_RM_06, SEQ_NR_05 );
		rv = SetEvent( TASK_ID_etask_m_full, EVENT_MASK_go );
		TEST_ASSERT( rv == E_OK);
		break;
	case OSEK_RM_07:
		TEST_SET_FIXTURE(OSEK_RM_07, SEQ_NR_04 );
		rv = SetEvent( TASK_ID_etask_m_full, EVENT_MASK_go );
		TEST_ASSERT( rv == E_OK);
		break;
	case OSEK_RM_15:
		TEST_SET_FIXTURE(OSEK_RM_15, SEQ_NR_05 );
		rv = SetEvent( TASK_ID_etask_m_full, EVENT_MASK_go );
		TEST_ASSERT( rv == E_OK);
		break;
	default:
		TEST_ASSERT( 0 );
	}

	rv = TerminateTask();
	TEST_ASSERT( rv == E_OK );
}


void btask_h_full ( void ) {
	StatusType rv;
	switch ( TestWorld.fixtureNr ) {
	case OSEK_RM_08:
		TEST_SET_FIXTURE(OSEK_RM_08, SEQ_NR_03 );
		break;
	default:
		TEST_ASSERT( 0 );
	}

	rv = TerminateTask();
	TEST_ASSERT( rv == E_OK );

}

void btask_l_full ( void ) {

}

void btask_ll_non( void ) {
	StatusType rv;
	/* Used for scheduling ONLY */
	rv = SetEvent(TASK_ID_etask_m_full, EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );
}

void btask_m_full ( void ) {

}

void etask_h_full( void ) {

}

void etask_l_full( void ) {
	StatusType rv;
	switch ( TestWorld.fixtureNr ) {
	case OSEK_RM_07:
		TEST_SET_FIXTURE(OSEK_RM_07, SEQ_NR_02 );
		rv = GetResource(RES_ID_std_h);
		TEST_ASSERT( rv == E_OK);
		rv = ActivateTask( TASK_ID_btask_m_non );
		TEST_ASSERT( rv == E_OK);
		TEST_SET_FIXTURE(OSEK_RM_07, SEQ_NR_03 );
		/* reshedule to btask_m_non */
		rv = ReleaseResource(RES_ID_std_h);
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_RM_07, SEQ_NR_06 );
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

	/** @treq OSEK_RM_01
	 *
	 * Call GetResource() from task which has no access to this resource
	 *
	 * Service returns E_OS_ACCESS
	 */
	TEST_SET_FIXTURE(OSEK_RM_01, SEQ_NR_01 );
	rv = GetResource( RES_ID_std_h);
	TEST_ASSERT( rv = E_OS_ACCESS )

	/** @treq OSEK_RM_02
	 *
	 * Call GetResource() from task with invalid resource ID
	 *
	 * Service returns E_OS_ID
	 */
	TEST_SET_FIXTURE(OSEK_RM_02, SEQ_NR_01 );
	rv = GetResource( RES_ID_ILL);
	TEST_ASSERT( rv = E_OS_ID );

	/** @treq OSEK_RM_03
	 *
	 * Call GetResource() from ISR2
	 *
	 * Service returns E_OS_CALLEVEL
	 */
	TEST_SET_FIXTURE(OSEK_RM_03, SEQ_NR_01 );
	/* This test is not applicable since it is allowed in OSEK
	 * to do GetResource() from ISR2
	 */

	/** @treq OSEK_RM_04
	 *
	 * Call GetResource() from ISR3
	 *
	 * Service returns E_OS_CALLEVEL
	 */
	TEST_SET_FIXTURE(OSEK_RM_04, SEQ_NR_01 );
	/* Not applicable */

	/** @treq OSEK_RM_05
	 *
	 * Call GetResource() from task with too many resources occupied in parallel
	 *
	 * Service returns E_OS_LIMIT
	 */
	TEST_SET_FIXTURE(OSEK_RM_05, SEQ_NR_01 );
	/* Not applicable */

	/** @treq OSEK_RM_06
	 *
	 * Test Priority Ceiling Protocol:
	 * Call GetResource() from non-preemptive task, activate task with priority
	 * higher than running task but lower than ceiling priority, and force
	 * rescheduling
	 *
	 * Resource is occupied and running task’s priority is set to resource’s
	 * ceiling priority. Service returns E_OK. No preemption occurs after
	 * activating the task with higher priority and rescheduling
	 */
	TEST_SET_FIXTURE(OSEK_RM_06, SEQ_NR_01 );
	TestActivateAndWait(TASK_ID_btask_l_non);
	TEST_SET_FIXTURE(OSEK_RM_06, SEQ_NR_06 );

	/** @treq OSEK_RM_07
	 *
	 * Test Priority Ceiling Protocol:
	 * Call GetResource()from preemptive task, and activate task with priority
	 * higher than running task but lower than ceiling priority
	 *
	 * Resource is occupied and running task’s priority is set to resource’s
	 * ceiling priority. Service returns E_OK. No preemption occurs after
	 * activating the task with higher priority
	 */
	TEST_SET_FIXTURE(OSEK_RM_07, SEQ_NR_01 );
	TestActivateAndWait(TASK_ID_etask_l_full);
	TEST_SET_FIXTURE(OSEK_RM_07, SEQ_NR_05 );
	TestTaskRunLowerPrio();
	TEST_SET_FIXTURE(OSEK_RM_07, SEQ_NR_07 );

	/** @treq OSEK_RM_08
	 *
	 * 	Call GetResource() for resource RES_SCHEDULER
	 *
	 * Resource is occupied and running task’s priority is set to resource’s
	 * ceiling priority. Service returns E_OK
	 */
	TEST_SET_FIXTURE(OSEK_RM_08, SEQ_NR_01 );
	rv = GetResource( RES_SCHEDULER );
	TEST_ASSERT( rv == E_OK );
	rv = ActivateTask( TASK_ID_btask_h_full);
	TEST_ASSERT( rv == E_OK );
	TEST_SET_FIXTURE(OSEK_RM_08, SEQ_NR_02 );
	rv = ReleaseResource( RES_SCHEDULER);
	TEST_ASSERT( rv == E_OK );
	TEST_SET_FIXTURE(OSEK_RM_08, SEQ_NR_04 );

	/** @treq OSEK_RM_09
	 *
	 * Call ReleaseResource() from task with invalid resource ID
	 *
	 * Service returns E_OS_ID
	 */
	TEST_SET_FIXTURE(OSEK_RM_09, SEQ_NR_01 );
	rv = ReleaseResource( RES_ID_ILL);
	TEST_ASSERT( rv == E_OS_ID);

	/** @treq OSEK_RM_10
	 *
	 * Call ReleaseResource() from ISR2
	 *
	 * Service returns E_OS_CALLEVEL
	 */
	TEST_SET_FIXTURE(OSEK_RM_10, SEQ_NR_01 );
	/* This test is not applicable since it is allowed in OSEK
	 * to do GetResource() from ISR2
	 */

	/** @treq OSEK_RM_11
	 *
	 * Call ReleaseResource() from ISR3
	 *
	 * Service returns E_OS_CALLEVEL
	 */

	TEST_SET_FIXTURE(OSEK_RM_11, SEQ_NR_01 );
	/* Not applicable */

	/** @treq OSEK_RM_12
	 *
	 * Call ReleaseResource() from task with resource which is not occupied
	 *
	 * Service returns E_OS_NOFUNC
	 */
	TEST_SET_FIXTURE(OSEK_RM_12, SEQ_NR_01 );
	rv = ReleaseResource( RES_ID_std_1 );
	TEST_ASSERT( rv == E_OS_NOFUNC );

	/** @treq OSEK_RM_13
	 *
	 * Call ReleaseResource() from non-preemptive task
	 *
	 * Resource is released and running task’s priority is reset.
	 * No preemption of running task. Service returns E_OK
	 */

	TEST_SET_FIXTURE(OSEK_RM_13, SEQ_NR_01 );
	/* Already covered in OSEK_RM_06 */

	/** @treq OSEK_RM_14
	 *
	 * Call ReleaseResource() from preemptive task
	 *
	 * Resource is released and running task’s priority is reset.
	 * Ready task with highest priority is executed(Rescheduling).
	 * Service returns E_OK"
	 */
	TEST_SET_FIXTURE(OSEK_RM_14, SEQ_NR_01 );
	/* Already covered in OSEK_RM_07 */

	/** @treq OSEK_RM_15
	 *
	 * Call ReleaseResource()from non-preemptive task for resource
	 * RES_SCHEDULER
	 *
	 * Resource is released and running task’s priority is reset.
	 * No preemption of running task. Service returns E_OK
	 *
	 */
	TEST_SET_FIXTURE(OSEK_RM_15, SEQ_NR_01 );
	TestActivateAndWait(TASK_ID_btask_l_non);
	TEST_SET_FIXTURE(OSEK_RM_15, SEQ_NR_06 );

	/** @treq OSEK_RM_16
	 *
	 * Call ReleaseResource()from preemptive task for resource
	 * RES_SCHEDULER
	 *
	 * Resource is released and running task’s priority is reset.
	 * Ready task with highest priority is executed (Rescheduling).
	 * Service returns E_OK"
	 */
	TEST_SET_FIXTURE(OSEK_RM_16, SEQ_NR_01 );
	/* Already covered in OSEK_RM_08 */

	TestExit(0);
}
