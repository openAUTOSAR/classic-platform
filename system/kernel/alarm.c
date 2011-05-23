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

#include <assert.h>
#include <stdlib.h>
#include "Os.h"
#include "application.h"
#include "internal.h"
#include "alarm_i.h"
#include "sys.h"

#if (OS_ALARM_CNT!=0)
#define COUNTER_MAX(x) 			(x)->counter->alarm_base.maxallowedvalue
#define COUNTER_MIN_CYCLE(x) 	(x)->counter->alarm_base.mincycle
#define ALARM_CHECK_ID(x) 				\
	if( (x) > OS_ALARM_CNT) { \
		rv = E_OS_ID;					\
		goto err; 						\
	}


/**
 * The system service  GetAlarmBase  reads the alarm base
 * characteristics. The return value <Info> is a structure in which
 * the information of data type AlarmBaseType is stored.
 *
 * @param alarm_id  Reference to alarm
 * @param info Reference to structure with constants of the alarm base.
 * @return
 */
StatusType GetAlarmBase( AlarmType AlarmId, AlarmBaseRefType Info ) {
    StatusType rv = Os_AlarmGetBase(AlarmId,Info);
    if (rv != E_OK) {
        goto err;
    }
    OS_STD_END_2(OSServiceId_GetAlarmBase,AlarmId, Info);
}


/**
 * The system service GetAlarm returns the relative value in ticks
 * before the alarm <AlarmID> expires.

 * @param AlarmId	Reference to an alarm
 * @param Tick[out]	Relative value in ticks before the alarm <AlarmID> expires.
 * @return
 */
StatusType GetAlarm(AlarmType AlarmId, TickRefType Tick) {
    StatusType rv = E_OK;
    OsAlarmType *aPtr;
    long flags;

    ALARM_CHECK_ID(AlarmId);
    aPtr = Os_AlarmGet(AlarmId);

	Irq_Save(flags);
	if( aPtr->active == 0 ) {
		rv = E_OS_NOFUNC;
		Irq_Restore(flags);
		goto err;
	}

	*Tick = Os_CounterDiff( 	aPtr->expire_val,
								Os_CounterGetValue(aPtr->counter),
								Os_CounterGetMaxValue(aPtr->counter) );

	Irq_Restore(flags);

	OS_STD_END_2(OSServiceId_GetAlarm,AlarmId, Tick);
}




/**
 * The system service occupies the alarm <AlarmID> element.
 * After <increment> ticks have elapsed, the task assigned to the
 * alarm <AlarmID> is activated or the assigned event (only for
 * extended tasks) is set or the alarm-callback routine is called.
 *
 * @param alarm_id Reference to the alarm element
 * @param increment Relative value in ticks
 * @param cycle Cycle value in case of cyclic alarm. In case of single alarms, cycle shall be zero.
 * @return
 */

StatusType SetRelAlarm(AlarmType AlarmId, TickType Increment, TickType Cycle){
	StatusType rv = E_OK;
	OsAlarmType *aPtr;
	long flags;

	ALARM_CHECK_ID(AlarmId);

	aPtr = Os_AlarmGet(AlarmId);

	OS_DEBUG(D_ALARM,"SetRelAlarm id:%d inc:%u cycle:%u\n",
					AlarmId,
					(unsigned)Increment,
					(unsigned)Cycle);

#if	(OS_APPLICATION_CNT > 1)

	rv = Os_ApplHaveAccess( Os_AlarmGet(AlarmId)->accessingApplMask );
	if( rv != E_OK ) {
		goto err;
	}

#endif

	if( (Increment == 0) || (Increment > COUNTER_MAX(aPtr)) ) {
		/** @req OS304 */
		rv =  E_OS_VALUE;
		goto err;
	} else {
		if( Cycle != 0 &&
			( (Cycle < COUNTER_MIN_CYCLE(aPtr)) ||
			  (Cycle > COUNTER_MAX(aPtr)) ) ) {
			/** @req OS304 */
			rv =  E_OS_VALUE;
			goto err;
		}
	}

	{
		Irq_Save(flags);
		if( aPtr->active == 1 ) {
		        Irq_Restore(flags);
			rv = E_OS_STATE;
			goto err;
		}

		aPtr->active = 1;

		aPtr->expire_val = Os_CounterAdd(
								Os_CounterGetValue(aPtr->counter),
								COUNTER_MAX(aPtr),
								Increment);
		aPtr->cycletime = Cycle;

		Irq_Restore(flags);
		OS_DEBUG(D_ALARM,"  expire:%u cycle:%u\n",
				(unsigned)aPtr->expire_val,
				(unsigned)aPtr->cycletime);
	}

	OS_STD_END_3(OSServiceId_SetRelAlarm,AlarmId, Increment, Cycle);
}

/**
 * The  system  service  occupies  the  alarm  <AlarmID>  element.
 * When <start> ticks are reached, the task assigned to the alarm
 *
 * If the absolute value <start> is very close to the current counter
 * value, the alarm may expire, and the task may become ready or
 * the  alarm-callback  may  be  called  before  the  system  service
 * returns to the user.
 * If  the  absolute  value  <start>  already  was  reached  before  the
 * system call, the alarm shall only expire when the absolute value
 * <start>  is  reached  again,  i.e.  after  the  next  overrun  of  the
 * counter.
 *
 * If <cycle> is unequal zero, the alarm element is logged on again
 * immediately after expiry with the relative value <cycle>.
 *
 * The alarm <AlarmID> shall not already be in use.
 * To  change  values  of  alarms  already  in  use  the  alarm  shall  be
 * cancelled first.
 *
 * If  the  alarm  is  already  in  use,  this  call  will  be  ignored  and  the
 * error E_OS_STATE is returned.
 *
 * Allowed on task level and in ISR, but not in hook routines.
 *
 * @param AlarmId
 * @param Start
 * @param Cycle
 * @return
 */

StatusType SetAbsAlarm(AlarmType AlarmId, TickType Start, TickType Cycle) {

	OsAlarmType *aPtr;
	long flags;
	StatusType rv = E_OK;

	ALARM_CHECK_ID(AlarmId);

	aPtr = Os_AlarmGet(AlarmId);

#if	(OS_APPLICATION_CNT > 1)

	rv = Os_ApplHaveAccess( aPtr->accessingApplMask );
	if( rv != E_OK ) {
		goto err;
	}

#endif

	if( Start > COUNTER_MAX(aPtr) ) {
		/** @req OS304 */
		rv =  E_OS_VALUE;
		goto err;
	}

	if( Cycle != 0 &&
		( (Cycle < COUNTER_MIN_CYCLE(aPtr)) ||
		  (Cycle > COUNTER_MAX(aPtr)) ) ) {
		/** @req OS304 */
		rv =  E_OS_VALUE;
		goto err;
	}

	Irq_Save(flags);
	if( aPtr->active == 1 ) {
		rv = E_OS_STATE;
		Irq_Restore(flags);
		goto err;
	}

	aPtr->active = 1;

	aPtr->expire_val = Start;
	aPtr->cycletime = Cycle;

	Irq_Restore(flags);

	OS_DEBUG(D_ALARM,"  expire:%u cycle:%u\n",
					(unsigned)aPtr->expire_val,
					(unsigned)aPtr->cycletime);

	OS_STD_END_3(OSServiceId_SetAbsAlarm,AlarmId, Start, Cycle);
}

StatusType CancelAlarm(AlarmType AlarmId) {
	StatusType rv = E_OK;
	OsAlarmType *aPtr;
	long flags;

	ALARM_CHECK_ID(AlarmId);

	aPtr = Os_AlarmGet(AlarmId);

	Irq_Save(flags);
	if( aPtr->active == 0 ) {
		rv = E_OS_NOFUNC;
		Irq_Restore(flags);
		goto err;
	}

	aPtr->active = 0;

	Irq_Restore(flags);

	OS_STD_END_1(OSServiceId_CancelAlarm,AlarmId);
}



/**
 *
 * @param a_obj
 */
static void AlarmProcess( OsAlarmType *aPtr ) {
	if( aPtr->cycletime == 0 ) {
		aPtr->active = 0;
	} else {
		// Calc new expire value..
		aPtr->expire_val = Os_CounterAdd( Os_CounterGetValue(aPtr->counter),
											Os_CounterGetMaxValue(aPtr->counter),
											aPtr->cycletime);
	}
}

void Os_AlarmCheck( OsCounterType *c_p ) {
	OsAlarmType *a_obj;
	StatusType rv;

	SLIST_FOREACH(a_obj,&c_p->alarm_head,alarm_list) {
		if( a_obj->active && (c_p->val == a_obj->expire_val) ) {
			/* Check if the alarms have expired */
			OS_DEBUG(D_ALARM,"expired %s id:%u val:%u\n",
											a_obj->name,
											(unsigned)a_obj->counter_id,
											(unsigned)a_obj->expire_val);

			switch( a_obj->action.type ) {
			case ALARM_ACTION_ACTIVATETASK:
				if( ActivateTask(a_obj->action.task_id) != E_OK ) {
					/* We actually do thing here, See 0S321 */
				}
				AlarmProcess(a_obj);
				break;
			case ALARM_ACTION_SETEVENT:
				rv =  SetEvent(a_obj->action.task_id,a_obj->action.event_id);
				if( rv != E_OK ) {
					ERRORHOOK(rv);
				}
				AlarmProcess(a_obj);
				break;
			case ALARM_ACTION_ALARMCALLBACK:
				/* TODO: not done */
				break;

			case ALARM_ACTION_INCREMENTCOUNTER:
				/** @req OS301 */
				/* Huh,, recursive....*/
				IncrementCounter(a_obj->action.counter_id);
				break;
			default:
				assert(0);
			}
		}
	}
}

void Os_AlarmAutostart(void) {
	int j;
	for (j = 0; j < OS_ALARM_CNT; j++) {
		OsAlarmType *alarmPtr;
		alarmPtr = Os_AlarmGet(j);
		if (alarmPtr->autostartPtr != NULL) {
			const OsAlarmAutostartType *autoPtr = alarmPtr->autostartPtr;

			if (Os_Sys.appMode & autoPtr->appModeRef) {
				if (autoPtr->autostartType == ALARM_AUTOSTART_ABSOLUTE) {
					SetAbsAlarm(j, autoPtr->alarmTime, autoPtr->cycleTime);
				} else {
					SetRelAlarm(j, autoPtr->alarmTime, autoPtr->cycleTime);
				}
			}
		}
	}
}
#endif


