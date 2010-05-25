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
#include "internal.h"


#define COUNTER_MAX(x) 			(x)->counter->alarm_base.maxallowedvalue
#define COUNTER_MIN_CYCLE(x) 	(x)->counter->alarm_base.mincycle
#define ALARM_CHECK_ID(x) 				\
	if( (x) > Os_CfgGetAlarmCnt()) { \
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
    StatusType rv = Os_CfgGetAlarmBase(AlarmId,Info);
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
    aPtr = Os_CfgGetAlarmObj(AlarmId);

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

	ALARM_CHECK_ID(AlarmId);

	aPtr = Os_CfgGetAlarmObj(AlarmId);

	OS_DEBUG(D_ALARM,"SetRelAlarm id:%d inc:%u cycle:%u\n",
					AlarmId,
					(unsigned)Increment,
					(unsigned)Cycle);


	if( (Increment == 0) || (Increment > COUNTER_MAX(aPtr)) ) {
		/** @req OS304 */
		rv =  E_OS_VALUE;
		goto err;
	} else {
		if(  Cycle == 0 ||
			(Cycle >= COUNTER_MIN_CYCLE(aPtr)) ||
			(Cycle <= COUNTER_MAX(aPtr)) ) {
			/* OK */
		} else {
			/** @req OS304 */
			rv =  E_OS_VALUE;
			goto err;
		}
	}

	{
		Irq_Disable();
		if( aPtr->active == 1 ) {
			Irq_Enable();
			rv = E_OS_STATE;
			goto err;
		}

		aPtr->active = 1;

		aPtr->expire_val = Os_CounterAdd(
								Os_CounterGetValue(aPtr->counter),
								COUNTER_MAX(aPtr),
								Increment);
		aPtr->cycletime = Cycle;

		Irq_Enable();
		OS_DEBUG(D_ALARM,"  expire:%u cycle:%u\n",
				(unsigned)aPtr->expire_val,
				(unsigned)aPtr->cycletime);
	}

	OS_STD_END_3(OSServiceId_SetRelAlarm,AlarmId, Increment, Cycle);
}

StatusType SetAbsAlarm(AlarmType AlarmId, TickType Start, TickType Cycle) {

	OsAlarmType *aPtr;
	long flags;
	StatusType rv = E_OK;

	ALARM_CHECK_ID(AlarmId);

	aPtr = Os_CfgGetAlarmObj(AlarmId);

	if( Start > COUNTER_MAX(aPtr) ) {
		/** @req OS304 */
		rv =  E_OS_VALUE;
		goto err;
	} else {
		if(  Cycle == 0 ||
			(Cycle >= COUNTER_MIN_CYCLE(aPtr)) ||
			(Cycle <= COUNTER_MAX(aPtr)) ) {
			/* OK */
		} else {
			/** @req OS304 */
			rv =  E_OS_VALUE;
			goto err;
		}
	}

	Irq_Save(flags);
	if( aPtr->active == 1 ) {
		rv = E_OS_STATE;
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

	aPtr = Os_CfgGetAlarmObj(AlarmId);

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



