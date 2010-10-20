/*
 *  Created on: 4 aug 2010
 *      Author: mahi
 */
/*
 * This file tests test requirements OSEK_AL_XX.
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
	OSEK_AL_01 = 1,
	OSEK_AL_02,
	OSEK_AL_03,
	OSEK_AL_04,
	OSEK_AL_05,
	OSEK_AL_06,
	OSEK_AL_07,
	OSEK_AL_08,
	OSEK_AL_09,
	OSEK_AL_10,
	OSEK_AL_11,
	OSEK_AL_12,
	OSEK_AL_13,
	OSEK_AL_14,
	OSEK_AL_15,
	OSEK_AL_16,
	OSEK_AL_17,
	OSEK_AL_18,
	OSEK_AL_19,
	OSEK_AL_20,
	OSEK_AL_21,
	OSEK_AL_22,
	OSEK_AL_23,
	OSEK_AL_24,
	OSEK_AL_25,
	OSEK_AL_26,
	OSEK_AL_27,
	OSEK_AL_28,
	OSEK_AL_29,
	OSEK_AL_30,
	OSEK_AL_31,
	OSEK_AL_32,
	OSEK_AL_33,
	OSEK_AL_34,
	OSEK_AL_35,
	OSEK_AL_36,
};

TestWorldType TestWorld = {0};

uint32_t testNrNon = 0;

static uint8_t activations = 0;

#if 0
static void isrSoftInt1( void ) {
	switch ( TestWorld.fixtureNr ) {
	case OSEK_TM_07:
		TEST_SET_FIXTURE(OSEK_AL_07, SEQ_NR_03 );
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
		TEST_SET_FIXTURE(OSEK_AL_07, SEQ_NR_02 );
		Irq_GenerateSoftInt( IRQ_SOFTINT_1 );
		break;
	case OSEK_TM_09:
		TEST_SET_FIXTURE(OSEK_AL_09, SEQ_NR_04 );
		break;
	default:
		TEST_ASSERT( 0 );
	}

}

#endif

void OsIdle(void) {
	while(1);
}

#if 0
void btask_l_non ( void ) {
	StatusType rv;
	switch ( TestWorld.fixtureNr ) {
	case OSEK_TM_09:
		TEST_SET_FIXTURE(OSEK_AL_09, SEQ_NR_02 );
		/* Make higher prio task ready */
		rv = SetEvent( TASK_ID_etask_m_full,EVENT_MASK_go);
		TEST_ASSERT( rv == E_OK ) ;
		TEST_SET_FIXTURE(OSEK_AL_09, SEQ_NR_03 );
		Irq_GenerateSoftInt( IRQ_SOFTINT_0 );
		TEST_SET_FIXTURE(OSEK_AL_09, SEQ_NR_05 );
		break;
	default:
		TEST_ASSERT( 0 );
	}

}

void etask_m_non ( void ) {
	StatusType rv;
	TaskStateType taskState;
	switch ( TestWorld.fixtureNr ) {
	case OSEK_AL_04:
		TEST_SET_FIXTURE(OSEK_AL_04, SEQ_NR_03 );
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
		TEST_SET_FIXTURE(OSEK_AL_04, SEQ_NR_04 );
		break;
	case OSEK_AL_04:
		TEST_SET_FIXTURE(OSEK_AL_04, SEQ_NR_03 );
		rv = SetEvent( etask_m_full, EVENT_MASK_go2 );
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_AL_04, SEQ_NR_04 );
		break;
	default:
		TEST_ASSERT( 0 );
	}

	rv = TerminateTask();
	TEST_ASSERT( rv == E_OK );
}
#endif

void etask_l_non( void ) {

	StatusType rv;
	TaskStateType taskState;

	switch ( TestWorld.fixtureNr ) {
	case OSEK_AL_30:
		TEST_SET_FIXTURE(OSEK_AL_30, SEQ_NR_02 );
		rv = IncrementCounter(COUNTER_ID_soft1);
		TEST_ASSERT( rv == E_OK );
		rv = IncrementCounter(COUNTER_ID_soft1);
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_AL_30, SEQ_NR_03 );
		break;
	case OSEK_AL_32:
		TEST_SET_FIXTURE(OSEK_AL_32, SEQ_NR_03 );
		rv = SetEvent( TASK_ID_etask_m_full, EVENT_MASK_go );
		TEST_ASSERT( rv == E_OK );
		break;
	case OSEK_AL_33:
		TEST_SET_FIXTURE(OSEK_AL_33, SEQ_NR_03 );
		rv = IncrementCounter(COUNTER_ID_soft1);
		TEST_ASSERT( rv == E_OK );
		/* Set event go in etask_h_full, nothing should happen */
		rv = IncrementCounter(COUNTER_ID_soft1);
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_AL_33, SEQ_NR_04 );

		rv = GetTaskState(TASK_ID_etask_h_full,&taskState);
		TEST_ASSERT( rv == E_OK );
		TEST_ASSERT( taskState == TASK_STATE_WAITING );

		/* Wake up etask_m_full for cleanup */
		rv = SetEvent( TASK_ID_etask_m_full, EVENT_MASK_go );
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_AL_33, SEQ_NR_05 );
		break;
	case OSEK_AL_34:
		TEST_SET_FIXTURE(OSEK_AL_34, SEQ_NR_03 );
		rv = IncrementCounter(COUNTER_ID_soft1);
		TEST_ASSERT( rv == E_OK );
		/* Set event go in etask_h_full  */
		rv = IncrementCounter(COUNTER_ID_soft1);
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_AL_34, SEQ_NR_04 );

		rv = GetTaskState(TASK_ID_etask_h_full,&taskState);
		TEST_ASSERT( rv == E_OK );
		TEST_ASSERT( taskState == TASK_STATE_READY );

		/* Wake up etask_m_full for cleanup */
		rv = SetEvent( TASK_ID_etask_m_full, EVENT_MASK_go );
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_AL_34, SEQ_NR_05 );
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
	case OSEK_AL_14:
		TEST_SET_FIXTURE(OSEK_AL_14, SEQ_NR_03 );
		break;
	case OSEK_AL_15:
		TEST_SET_FIXTURE(OSEK_AL_15, SEQ_NR_02 );
		rv = WaitEvent(EVENT_MASK_go);
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_AL_15, SEQ_NR_04 );
		rv = ClearEvent( EVENT_MASK_go );
		TEST_ASSERT( rv == E_OK );
		break;
	case OSEK_AL_23:
		TEST_SET_FIXTURE(OSEK_AL_23, SEQ_NR_03 );
		break;
	case OSEK_AL_24:
		TEST_SET_FIXTURE(OSEK_AL_24, SEQ_NR_02 );
		rv = WaitEvent(EVENT_MASK_go);
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_AL_24, SEQ_NR_04 );
		rv = ClearEvent( EVENT_MASK_go );
		TEST_ASSERT( rv == E_OK );
		break;
	case OSEK_AL_29:
		TEST_SET_FIXTURE(OSEK_AL_29, SEQ_NR_03 );
		rv = SetEvent( TASK_ID_etask_m_full, EVENT_MASK_go );
		TEST_ASSERT( rv == E_OK );
		break;
	case OSEK_AL_30:
		TEST_SET_FIXTURE(OSEK_AL_30, SEQ_NR_04 );
		rv = SetEvent( TASK_ID_etask_m_full, EVENT_MASK_go );
		TEST_ASSERT( rv == E_OK );
		break;
	case OSEK_AL_31:
		TEST_SET_FIXTURE(OSEK_AL_31, SEQ_NR_02 );
		break;
	case OSEK_AL_33:
		TEST_SET_FIXTURE(OSEK_AL_33, SEQ_NR_02 );
		rv = WaitEvent( EVENT_MASK_go2 );
		TEST_ASSERT( rv == E_OK );
		rv = ClearEvent( EVENT_MASK_go2 | EVENT_MASK_go );
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_AL_33, SEQ_NR_07 );
		break;
	case OSEK_AL_34:
		TEST_SET_FIXTURE(OSEK_AL_34, SEQ_NR_02 );
		rv = WaitEvent( EVENT_MASK_go );
		TEST_ASSERT( rv == E_OK );
		rv = ClearEvent( EVENT_MASK_go );
		TEST_ASSERT( rv == E_OK );
		TEST_SET_FIXTURE(OSEK_AL_34, SEQ_NR_06 );
		break;
	case OSEK_AL_35:
		TEST_SET_FIXTURE(OSEK_AL_35, SEQ_NR_02 );
		rv = WaitEvent( EVENT_MASK_go2 );
		TEST_ASSERT( rv == E_OK );
		rv = ClearEvent( EVENT_MASK_go2 | EVENT_MASK_go );
		break;
	case OSEK_AL_36:
		TEST_SET_FIXTURE(OSEK_AL_36, SEQ_NR_02 );
		rv = WaitEvent( EVENT_MASK_go );
		TEST_ASSERT( rv == E_OK );
		rv = ClearEvent( EVENT_MASK_go );
		TEST_SET_FIXTURE(OSEK_AL_36, SEQ_NR_04 );
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
	TickType tick;
	AlarmBaseType alarmBase;
	EventMaskType eventMask;

	/** @treq OSEK_AL_01
	 *
	 * Call GetAlarmBase() with invalid alarm ID
	 *
	 * Service returns E_OS_ID
	 */
	TEST_SET_FIXTURE(OSEK_AL_01, SEQ_NR_01 );
	rv = GetAlarmBase(ALARM_ID_ILL,&alarmBase);
	TEST_ASSERT( rv == E_OS_ID);

	/** @treq OSEK_AL_02
	 *
	 * Call GetAlarmBase()
	 *
	 * Return alarm base characteristics. Service returns E_OK
	 */
	TEST_SET_FIXTURE(OSEK_AL_02, SEQ_NR_01 );
	rv = GetAlarmBase(ALARM_ID_act_etask_h_full,&alarmBase);
	TEST_ASSERT( rv == E_OK);
	TEST_ASSERT( alarmBase.maxallowedvalue == 65535 );
	TEST_ASSERT( alarmBase.mincycle == 2 );
	TEST_ASSERT( alarmBase.tickperbase == 1 );

	/** @treq OSEK_AL_03
	 *
	 * Call GetAlarm() with invalid alarm ID
	 *
	 * Service returns E_OS_ID
	 */
	TEST_SET_FIXTURE(OSEK_AL_03, SEQ_NR_01 );
	rv = GetAlarm(ALARM_ID_ILL,&tick);
	TEST_ASSERT( rv == E_OS_ID);

	/** @treq OSEK_AL_04
	 *
	 * Call GetAlarm() for alarm which is currently not in use
	 *
	 * Service returns E_OS_NOFUNC
	 */
	TEST_SET_FIXTURE(OSEK_AL_04, SEQ_NR_01 );
	rv = GetAlarm(ALARM_ID_act_etask_h_full,&tick);
	TEST_ASSERT( rv == E_OS_NOFUNC);

	/** @treq OSEK_AL_05
	 *
	 * Call GetAlarm() for alarm which will activate a task on expiration
	 *
	 * Returns number of ticks until expiration. Service returns E_OK
	 */
	TEST_SET_FIXTURE(OSEK_AL_05, SEQ_NR_01 );
	rv = SetRelAlarm(ALARM_ID_act_etask_h_full,10,0);
	TEST_ASSERT( rv == E_OK);
	rv = GetAlarm(ALARM_ID_act_etask_h_full,&tick);
	TEST_ASSERT( rv == E_OK);
	TEST_ASSERT( tick == 10 );
	rv = CancelAlarm(ALARM_ID_act_etask_h_full);
	TEST_ASSERT( rv == E_OK);

	/** @treq OSEK_AL_06
	 *
	 * Call GetAlarm() for alarm which will set an event on expiration
	 *
	 * Returns number of ticks until expiration. Service returns E_OK
	 */
	TEST_SET_FIXTURE(OSEK_AL_06, SEQ_NR_01 );
	rv = SetRelAlarm(ALARM_ID_setev_go_etask_h_full,10,0);
	TEST_ASSERT( rv == E_OK);
	rv = GetAlarm(ALARM_ID_setev_go_etask_h_full,&tick);
	TEST_ASSERT( rv == E_OK);
	TEST_ASSERT( tick == 10 );
	rv = CancelAlarm(ALARM_ID_setev_go_etask_h_full);
	TEST_ASSERT( rv == E_OK);

	/** @treq OSEK_AL_07
	 *
	 * 	Call SetRelAlarm() with invalid alarm ID
	 *
	 * 	Service returns E_OS_ID
	 */
	TEST_SET_FIXTURE(OSEK_AL_07, SEQ_NR_01 );
	rv = SetRelAlarm(ALARM_ID_ILL,10,0);
	TEST_ASSERT( rv == E_OS_ID );

	/** @treq OSEK_AL_08
	 *
	 * Call SetRelAlarm() for already activated alarm which will activate a task on expiration
	 *
	 * Service returns E_OS_STATE
	 */
	TEST_SET_FIXTURE(OSEK_AL_08, SEQ_NR_01 );
	rv = SetRelAlarm(ALARM_ID_act_etask_h_full,10,0);
	TEST_ASSERT( rv == E_OK);
	rv = SetRelAlarm(ALARM_ID_act_etask_h_full,10,0);
	TEST_ASSERT( rv == E_OS_STATE);
	rv = CancelAlarm(ALARM_ID_act_etask_h_full);
	TEST_ASSERT( rv == E_OK);

	/** @treq OSEK_AL_09
	 *
	 * Call SetRelAlarm() for already activated alarm which will set an
	 * event on expiration
	 *
	 * Service returns E_OS_STATE
	 */
	TEST_SET_FIXTURE(OSEK_AL_09, SEQ_NR_01 );
	rv = SetRelAlarm(ALARM_ID_setev_go_etask_h_full,10,0);
	TEST_ASSERT( rv == E_OK);
	rv = SetRelAlarm(ALARM_ID_setev_go_etask_h_full,10,0);
	TEST_ASSERT( rv == E_OS_STATE);
	rv = CancelAlarm(ALARM_ID_setev_go_etask_h_full);
	TEST_ASSERT( rv == E_OK);


	/** @treq OSEK_AL_10
	 *
	 * Call SetRelAlarm() with increment value lower than zero
	 *
	 * Service returns E_OS_VALUE
	 */
	TEST_SET_FIXTURE(OSEK_AL_10, SEQ_NR_01 );
	rv = SetRelAlarm(ALARM_ID_setev_go_etask_h_full,-1,0);
	TEST_ASSERT( rv == E_OS_VALUE);

	/** @treq OSEK_AL_11
	 *
	 * Call SetRelAlarm() with increment value greater than maxallowedvalue
	 *
	 * Service returns E_OS_VALUE
	 */
	TEST_SET_FIXTURE(OSEK_AL_11, SEQ_NR_01 );
	rv = SetRelAlarm(ALARM_ID_setev_go_etask_h_full,UINT16_MAX+1,0);
	TEST_ASSERT( rv == E_OS_VALUE);

	/** @treq OSEK_AL_12
	 *
	 * Call SetRelAlarm() with cycle value lower than mincycle
	 *
	 * Service returns E_OS_VALUE
	 */
	TEST_SET_FIXTURE(OSEK_AL_12, SEQ_NR_01 );
	rv = SetRelAlarm(ALARM_ID_setev_go_etask_h_full,10,1);
	TEST_ASSERT( rv == E_OS_VALUE);


	/** @treq OSEK_AL_13
	 *
	 * Call SetRelAlarm() with cycle value greater than maxallowedvalue
	 *
	 * Service returns E_OS_VALUE
	 */
	TEST_SET_FIXTURE(OSEK_AL_13, SEQ_NR_01 );
	rv = SetRelAlarm(ALARM_ID_setev_go_etask_h_full,10,UINT16_MAX+1);
	TEST_ASSERT( rv == E_OS_VALUE);

	/** @treq OSEK_AL_14
	 *
	 * Call SetRelAlarm() for alarm which will activate a task on expiration
	 *
	 * Alarm is activated. Service returns	E_OK
	 */
	TEST_SET_FIXTURE(OSEK_AL_14, SEQ_NR_01 );
	rv = SetRelAlarm(ALARM_ID_act_etask_h_full,2,0);
	TEST_ASSERT( rv == E_OK);
	rv = IncrementCounter(COUNTER_ID_soft1);
	TEST_ASSERT( rv == E_OK);
	TEST_SET_FIXTURE(OSEK_AL_14, SEQ_NR_02 );
	rv = IncrementCounter(COUNTER_ID_soft1);
	TEST_ASSERT( rv == E_OK);
	TEST_SET_FIXTURE(OSEK_AL_14, SEQ_NR_04 );

	/** @treq OSEK_AL_15
	 *
	 * Call SetRelAlarm() for alarm which will set an event on expiration
	 *
	 * Alarm is activated. Service returns E_OK
	 */

	TEST_SET_FIXTURE(OSEK_AL_15, SEQ_NR_01 );
	rv = SetRelAlarm(ALARM_ID_setev_go_etask_h_full,2,0);
	TEST_ASSERT( rv == E_OK);
	/* The task may not be suspended */
	rv = ActivateTask(TASK_ID_etask_h_full);
	TEST_ASSERT( rv == E_OK);
	rv = IncrementCounter(COUNTER_ID_soft1);
	TEST_ASSERT( rv == E_OK);
	TEST_SET_FIXTURE(OSEK_AL_15, SEQ_NR_03 );
	rv = IncrementCounter(COUNTER_ID_soft1);
	TEST_ASSERT( rv == E_OK);
	TEST_SET_FIXTURE(OSEK_AL_15, SEQ_NR_05 );

	/** @treq OSEK_AL_16
	 *
	 * Call SetAbsAlarm() with invalid alarm ID
	 *
	 * Service returns E_OS_ID
	 */
	TEST_SET_FIXTURE(OSEK_AL_16, SEQ_NR_01 );
	rv = SetAbsAlarm(ALARM_ID_ILL,2,0);
	TEST_ASSERT( rv == E_OS_ID);

	/** @treq OSEK_AL_17
	 *
	 * Call SetAbsAlarm() for already activated alarm which will activate a task on expiration
	 *
	 * Service returns E_OS_STATE
	 */
	TEST_SET_FIXTURE(OSEK_AL_17, SEQ_NR_01 );
	rv = SetAbsAlarm(ALARM_ID_act_etask_h_full,2,0);
	TEST_ASSERT( rv == E_OK);
	rv = SetAbsAlarm(ALARM_ID_act_etask_h_full,2,0);
	TEST_ASSERT( rv == E_OS_STATE);
	rv = CancelAlarm(ALARM_ID_act_etask_h_full);
	TEST_ASSERT( rv == E_OK);

	/** @treq OSEK_AL_18
	 *
	 * Call SetAbsAlarm() for already activated alarm which will set an
	 * event on expiration
	 *
	 * Service returns E_OS_STATE
	 */
	TEST_SET_FIXTURE(OSEK_AL_18, SEQ_NR_01 );
	rv = SetAbsAlarm(ALARM_ID_setev_go_etask_h_full,2,0);
	TEST_ASSERT( rv == E_OK);
	rv = SetAbsAlarm(ALARM_ID_setev_go_etask_h_full,2,0);
	TEST_ASSERT( rv == E_OS_STATE);
	rv = CancelAlarm(ALARM_ID_setev_go_etask_h_full);
	TEST_ASSERT( rv == E_OK);

	/** @treq OSEK_AL_19
	 *
	 * Call SetAbsAlarm() with increment value lower than zero
	 *
	 * 	Service returns E_OS_VALUE
	 */
	TEST_SET_FIXTURE(OSEK_AL_19, SEQ_NR_01 );
	rv = SetAbsAlarm(ALARM_ID_setev_go_etask_h_full,-1,0);
	TEST_ASSERT( rv == E_OS_VALUE);

	/** @treq OSEK_AL_20
	 *
	 * Call SetAbsAlarm() with increment value greater than maxallowedvalue
	 *
	 * Service returns E_OS_VALUE
	 */
	TEST_SET_FIXTURE(OSEK_AL_20, SEQ_NR_01 );
	rv = SetAbsAlarm(ALARM_ID_setev_go_etask_h_full,UINT16_MAX+1,0);
	TEST_ASSERT( rv == E_OS_VALUE);

	 /** @treq OSEK_AL_21
	  *
	  * Call SetAbsAlarm() with cycle value lower than mincycle
	  *
	  * Service returns E_OS_VALUE
	  */
	TEST_SET_FIXTURE(OSEK_AL_21, SEQ_NR_01 );
	rv = SetAbsAlarm(ALARM_ID_setev_go_etask_h_full,10,1);
	TEST_ASSERT( rv == E_OS_VALUE);

	 /** @treq OSEK_AL_22
	  *
	  * Call SetAbsAlarm() with cycle value greater than maxallowedvalue
	  * Service returns E_OS_VALUE
	  */
	TEST_SET_FIXTURE(OSEK_AL_22, SEQ_NR_01 );
	rv = SetAbsAlarm(ALARM_ID_setev_go_etask_h_full,10,UINT16_MAX+1);
	TEST_ASSERT( rv == E_OS_VALUE);

	 /** @treq OSEK_AL_23
	  *
	  * Call SetAbsAlarm() for alarm which will activate a task on expiration
	  *
	  * Alarm is activated. Service returns E_OK
	  */
	TEST_SET_FIXTURE(OSEK_AL_23, SEQ_NR_01 );
	rv = GetCounterValue(COUNTER_ID_soft1,&tick);
	TEST_ASSERT( rv == E_OK );

	rv = SetAbsAlarm(ALARM_ID_act_etask_h_full,tick+2,0);
	TEST_ASSERT( rv == E_OK);
	rv = IncrementCounter(COUNTER_ID_soft1);
	TEST_ASSERT( rv == E_OK);
	TEST_SET_FIXTURE(OSEK_AL_23, SEQ_NR_02 );
	rv = IncrementCounter(COUNTER_ID_soft1);
	TEST_ASSERT( rv == E_OK);
	TEST_SET_FIXTURE(OSEK_AL_23, SEQ_NR_04 );


	 /** @treq OSEK_AL_24
	  *
	  * Call SetAbsAlarm() for alarm which will set an event on expiration
	  *
	  * Alarm is activated. Service returns E_OK
	  */

	TEST_SET_FIXTURE(OSEK_AL_24, SEQ_NR_01 );

	rv = GetCounterValue(COUNTER_ID_soft1,&tick);
	TEST_ASSERT( rv == E_OK );

	rv = SetAbsAlarm(ALARM_ID_setev_go_etask_h_full,tick+2,0);
	TEST_ASSERT( rv == E_OK);
	/* The task may not be suspended */
	rv = ActivateTask(TASK_ID_etask_h_full);
	TEST_ASSERT( rv == E_OK);
	rv = IncrementCounter(COUNTER_ID_soft1);
	TEST_ASSERT( rv == E_OK);
	TEST_SET_FIXTURE(OSEK_AL_24, SEQ_NR_03 );
	rv = IncrementCounter(COUNTER_ID_soft1);
	TEST_ASSERT( rv == E_OK);
	TEST_SET_FIXTURE(OSEK_AL_24, SEQ_NR_05 );

	/** @treq OSEK_AL_25
	 *
	 * Call CancelAlarm() with invalid alarm ID
	 *
	 * Service returns E_OS_ID
	 */
	TEST_SET_FIXTURE(OSEK_AL_25, SEQ_NR_01 );
	rv = CancelAlarm(ALARM_ID_ILL);
	TEST_ASSERT( rv == E_OS_ID );

	 /** @treq OSEK_AL_26
	  *
	  * Call CancelAlarm() for alarm which is currently not in use
	  *
	  * Service returns E_OS_NOFUNC
	  */
	TEST_SET_FIXTURE(OSEK_AL_26, SEQ_NR_01 );
	rv = CancelAlarm(ALARM_ID_act_etask_h_full);
	TEST_ASSERT( rv == E_OS_NOFUNC );

	/** @treq OSEK_AL_27
	 *
	 * Call CancelAlarm() for already activated alarm which will activate a task on expiration
	 *
	 * Alarm is cancelled. Service returns E_OK
	 */
	TEST_SET_FIXTURE(OSEK_AL_27, SEQ_NR_01 );
	/* Already covered by OSEK_AL_05 */

	 /** @treq OSEK_AL_28
	  *
	  * Call CancelAlarm() for already activated alarm which will set an event on expiration
	  *
	  * Alarm is cancelled. Service returns E_OK
	  */
	TEST_SET_FIXTURE(OSEK_AL_28, SEQ_NR_01 );
	/* Already covered by OSEK_AL_06 */

	/** @treq OSEK_AL_29
	 *
	 *  Expiration of alarm which activates a task while no tasks are currently running
	 *
	 *  Task is activated
	 */
	TEST_SET_FIXTURE(OSEK_AL_29, SEQ_NR_01 );
	rv = SetRelAlarm(ALARM_ID_sys_tick_act_etask_h_full,2,0);
	TEST_ASSERT( rv == E_OK );
	TEST_SET_FIXTURE(OSEK_AL_29, SEQ_NR_02 );
	rv = WaitEvent( EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );
	rv = ClearEvent( EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );
	TEST_SET_FIXTURE(OSEK_AL_29, SEQ_NR_04 );

	/** @treq OSEK_AL_30
	 *
	 * Expiration of alarm which activates a task while running task is non-preemptive
	 *
	 * Task is activated. No preemption of running task
	 */
	TEST_SET_FIXTURE(OSEK_AL_30, SEQ_NR_01 );
	rv = SetRelAlarm(ALARM_ID_act_etask_h_full,2,0);
	TEST_ASSERT( rv == E_OK );
	rv = ActivateTask(TASK_ID_etask_l_non);
	TEST_ASSERT( rv == E_OK );
	/* Swap to NON task */
	rv = WaitEvent( EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );
	rv = ClearEvent( EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );

	/** @treq OSEK_AL_31
	 *
	 * Expiration of alarm which activates a task with higher priority than running task while running task is preemptive
	 *
	 * Task is activated. Task with highest priority is executed
	 */
	TEST_SET_FIXTURE(OSEK_AL_31, SEQ_NR_01 );
	rv = SetRelAlarm(ALARM_ID_act_etask_h_full,2,0);
	TEST_ASSERT( rv == E_OK );
	rv = IncrementCounter(COUNTER_ID_soft1);
	TEST_ASSERT( rv == E_OK );
	rv = IncrementCounter(COUNTER_ID_soft1);
	TEST_ASSERT( rv == E_OK );
	TEST_SET_FIXTURE(OSEK_AL_31, SEQ_NR_03 );

	/** @treq OSEK_AL_32
	 *
	 * Expiration of alarm which activates a task with lower priority than running task while running task is preemptive
	 *
	 * Task is activated. No preemption of running task.
	 */
	TEST_SET_FIXTURE(OSEK_AL_32, SEQ_NR_01 );
	rv = SetRelAlarm(ALARM_ID_act_etask_l_non,2,0);
	TEST_ASSERT( rv == E_OK );
	rv = IncrementCounter(COUNTER_ID_soft1);
	TEST_ASSERT( rv == E_OK );
	rv = IncrementCounter(COUNTER_ID_soft1);
	TEST_ASSERT( rv == E_OK );
	TEST_SET_FIXTURE(OSEK_AL_32, SEQ_NR_02 );
	/* Swap to alarm task */
	rv = WaitEvent( EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );
	rv = ClearEvent( EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );
	TEST_SET_FIXTURE(OSEK_AL_32, SEQ_NR_04 );

	/** @treq OSEK_AL_33
	 *
	 * Expiration of alarm which sets an event while running task is non-preemptive.
	 *
	 * Task which owns the event is not waiting for this event and not suspended.Event is set
	 */
	TEST_SET_FIXTURE(OSEK_AL_33, SEQ_NR_01 );
	rv = SetRelAlarm(ALARM_ID_setev_go_etask_h_full,2,0);
	TEST_ASSERT( rv == E_OK );

	/* Activate task that waits for go2 */
	rv = ActivateTask(TASK_ID_etask_h_full);
	TEST_ASSERT( rv == E_OK );

	/* Activate the NON task */
	rv = ActivateTask(TASK_ID_etask_l_non);
	TEST_ASSERT( rv == E_OK );

	/* Swap to NON task */
	rv = WaitEvent( EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );
	rv = ClearEvent( EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );

	TEST_SET_FIXTURE(OSEK_AL_33, SEQ_NR_06 );

	/* Cleanup by etask_h_full */
	rv = SetEvent( TASK_ID_etask_h_full, EVENT_MASK_go2 );
	TEST_ASSERT( rv == E_OK );
	TEST_SET_FIXTURE(OSEK_AL_33, SEQ_NR_08 );
	/** @treq OSEK_AL_34
	 *
	 * Expiration of alarm which sets an event while running task is non-preemptive.
	 * Task which owns the event is waiting for this event.
	 *
	 * Event is set. Task which is owner of the event becomes ready.
	 * No preemption of running task
	 */
	TEST_SET_FIXTURE(OSEK_AL_34, SEQ_NR_01 );
	rv = SetRelAlarm(ALARM_ID_setev_go_etask_h_full,2,0);
	TEST_ASSERT( rv == E_OK );

	/* Activate task that waits for go */
	rv = ActivateTask(TASK_ID_etask_h_full);
	TEST_ASSERT( rv == E_OK );

	/* Activate the NON task */
	rv = ActivateTask(TASK_ID_etask_l_non);
	TEST_ASSERT( rv == E_OK );

	/* Swap to NON task */
	rv = WaitEvent( EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );
	rv = ClearEvent( EVENT_MASK_go );
	TEST_ASSERT( rv == E_OK );

	TEST_SET_FIXTURE(OSEK_AL_34, SEQ_NR_07 );

	/** @treq OSEK_AL_35
	 *
	 * Expiration of alarm which sets an event while running task is preemptive.
	 * Task which owns the event is not waiting for this event and not suspended.
	 *
	 * Event is set
	 */
	TEST_SET_FIXTURE(OSEK_AL_35, SEQ_NR_01 );
	rv = SetRelAlarm(ALARM_ID_setev_go_etask_h_full,2,0);

	/* Activate task that waits for go */
	rv = ActivateTask(TASK_ID_etask_h_full);
	TEST_ASSERT( rv == E_OK );

	TEST_SET_FIXTURE(OSEK_AL_35, SEQ_NR_03 );
	/* Set go event */
	rv = IncrementCounter(COUNTER_ID_soft1);
	TEST_ASSERT( rv == E_OK );
	rv = IncrementCounter(COUNTER_ID_soft1);
	TEST_ASSERT( rv == E_OK );

	TEST_SET_FIXTURE(OSEK_AL_35, SEQ_NR_04 );

	rv = GetEvent(TASK_ID_etask_h_full,&eventMask);
	TEST_ASSERT( rv == E_OK );
	TEST_ASSERT( eventMask == EVENT_MASK_go );

	/* Cleanup */
	rv = SetEvent(TASK_ID_etask_h_full,EVENT_MASK_go2);
	TEST_ASSERT( rv == E_OK );

	/** @treq OSEK_AL_36
	 *
	 * Expiration of alarm which sets an event while running task is preemptive.
	 * Task which owns the event is waiting for this event.
	 *
	 * Event is set. Task which is owner of the event becomes ready.
	 * Task with highest priority is executed(Rescheduling)
	 */
	TEST_SET_FIXTURE(OSEK_AL_36, SEQ_NR_01 );

	rv = SetRelAlarm(ALARM_ID_setev_go_etask_h_full,2,0);
	TEST_ASSERT( rv == E_OK );
	rv = ActivateTask(TASK_ID_etask_h_full);
	TEST_ASSERT( rv == E_OK );

	TEST_SET_FIXTURE(OSEK_AL_36, SEQ_NR_03 );
	rv = IncrementCounter(COUNTER_ID_soft1);
	TEST_ASSERT( rv == E_OK );
	rv = IncrementCounter(COUNTER_ID_soft1);
	TEST_ASSERT( rv == E_OK );
	TEST_SET_FIXTURE(OSEK_AL_36, SEQ_NR_05 );

	TestExit(0);
}
