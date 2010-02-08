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
#include "arc.h"

#define COUNTER_STD_END 	\
		goto ok;		\
	err:				\
		ERRORHOOK(rv);	\
	ok:					\
		return rv;


/* Accessor functions */
#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
static inline OsSchTblAdjExpPointType *getAdjExpPoint( OsSchTblType *stblPtr ) {
	return &stblPtr->adjExpPoint;
}
#endif


static inline struct OsSchTblAutostart *getAutoStart( OsSchTblType *stblPtr ) {
	return &stblPtr->autostart;
}

#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
static inline struct OsScheduleTableSync *getSync( OsSchTblType *stblPtr ) {
	return &stblPtr->sync;
}
#endif


/**
 *
 * @param a_obj
 */
static void AlarmProcess( OsAlarmType *a_obj ) {
	if( a_obj->cycletime == 0 ) {
		a_obj->active = 0;
	} else {
		// Calc new expire value..
		a_obj->expire_val = Os_CounterCalcModulo( a_obj->expire_val,
											Os_CounterGetMaxValue(a_obj->counter),
											a_obj->cycletime);
	}
}

static void check_alarms( OsCounterType *c_p ) {
	OsAlarmType *a_obj;

	SLIST_FOREACH(a_obj,&c_p->alarm_head,alarm_list) {
		if( a_obj->active && (c_p->val == a_obj->expire_val) ) {
			/* Check if the alarms have expired */
			os_isr_printf(D_ALARM,"expired %s id:%d val:%d\n",
											a_obj->name,
											a_obj->counter_id,
											a_obj->expire_val);

			switch( a_obj->action.type ) {
			case ALARM_ACTION_ACTIVATETASK:
				if( ActivateTask(a_obj->action.task_id) != E_OK ) {
					/* We actually do thing here, See 0S321 */
				}
				AlarmProcess(a_obj);
				break;
			case ALARM_ACTION_SETEVENT:
				if( SetEvent(a_obj->action.task_id,a_obj->action.event_id) != E_OK ) {
					// TODO: Check what to do here..
					assert(0);
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

/**
 * Go through the schedule tables connected to this counter
 *
 * @param c_p Pointer to counter object
 */

/** @req OS002 */
/** @req OS007 */
static void check_stbl(OsCounterType *c_p) {
	OsSchTblType *sched_obj;

	/* Iterate through the schedule tables */
	SLIST_FOREACH(sched_obj,&c_p->sched_head,sched_list) {

		if( sched_obj->state == SCHEDULETABLE_STOPPED ) {
			continue;
		}

#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
		if( sched_obj->sync.syncStrategy == IMPLICIT ) {
			// ....

		} else {
			int adj;
			// Handle EXPLICIT
			if( sched_obj->sync.deviation > 0 ) {
				// The sync counter was set back ==
				// we have more time to complete the table
				adj = MIN(sched_obj->sync.deviation, getAdjExpPoint(sched_obj)->maxAdvance );
				sched_obj->sync.deviation -= adj;

			} else if( sched_obj->sync.deviation < 0 ) {
				// The sync counter was set forward ==
				// we have less time to complete the table
				adj = MIN((-sched_obj->sync.deviation), getAdjExpPoint(sched_obj)->maxRetard);
				sched_obj->sync.deviation -= adj;

			} else {
				// all is well
				sched_obj->state = SCHEDULETABLE_RUNNING_AND_SYNCHRONOUS;
			}
		}
#endif

		/* Check if the expire point have been hit */
		if( (sched_obj->state == SCHEDULETABLE_RUNNING ||
				SCHEDULETABLE_RUNNING_AND_SYNCHRONOUS ) &&
				(c_p->val >= sched_obj->expire_val) )
		{
			OsScheduleTableActionType * action;

			action = SA_LIST_GET(&sched_obj->action_list,sched_obj->expire_curr_index);

			switch( action->type ) {
			case SCHEDULE_ACTION_ACTIVATETASK:
				ActivateTask(action->task_id);
				break;

			case SCHEDULE_ACTION_SETEVENT:
				SetEvent( action->task_id, action->event_id);
				break;

			default:
				/** @req OS407 */
				assert(0);
		}
			// Calc new expire val
			Os_SchTblCalcExpire(sched_obj);
		}

	}
}


/**
 * Increment a counter. Checks for wraps.
 *
 * @param counter Ptr to a counter object
 */
static void IncCounter( OsCounterType *counter ) {
	// Check for wrap of type
	if( (counter->val+1) < (counter->val) ) {
		counter->val = 0;		// This wraps
	} else {
		if( counter->val > counter->alarm_base.maxallowedvalue ) {
			counter->val = 0;
		} else {
			counter->val++;
		}
	}
}


#define IsCounterValid(_counterId)   ((_counterId) <= Oil_GetCounterCnt())

/**
 *
 * @param counter_id
 * @return
 */

/** @req OS399 */
StatusType IncrementCounter( CounterType counter_id ) {
	StatusType rv = E_OK;
	OsCounterType *counter;
	counter = Oil_GetCounter(counter_id);

	/** @req OS376 */
	if( !IsCounterValid(counter_id) ) {
		rv = E_OS_ID;
		goto err;
	}

	/* Check param */
	/** @req OS285 */
	if( ( counter->type != COUNTER_TYPE_SOFT ) ||
		( counter_id >= Oil_GetCounterCnt() ) ) {
		rv =  E_OS_ID;
		goto err;
	}

	/** @req OS286 */
	IncCounter(counter);

	check_alarms(counter);
	check_stbl(counter);

	/** @req OS321 */
	COUNTER_STD_END;
}


StatusType GetCounterValue( CounterType counter_id , TickRefType tick_ref)
{
	StatusType rv = E_OK;
	OsCounterType *cPtr;
	cPtr = Oil_GetCounter(counter_id);

	/** @req OS376 */
	if( !IsCounterValid(counter_id) ) {
		rv = E_OS_ID;
		goto err;
	}

	/** @req OS377 */
	if( cPtr->type == COUNTER_TYPE_HARD ) {
		if( cPtr->driver == NULL ) {
			/* It's OSINTERNAL */
			*tick_ref = os_sys.tick;
		} else {
#if 0
		/* We support only GPT for now */
		*tick_ref  = (TickType)Gpt_GetTimeElapsed(cPtr->driver.OsGptChannelRef);
#endif

		}
	} else {
		*tick_ref = cPtr->val;
	}

	COUNTER_STD_END;
}

StatusType GetElapsedCounterValue( CounterType counter_id, TickRefType val, TickRefType elapsed_val)
{
	StatusType rv = E_OK;
	OsCounterType *cPtr;
	TickType tick;

	cPtr = Oil_GetCounter(counter_id);

	/** @req OS381 */
	if( !IsCounterValid(counter_id) ) {
		rv = E_OS_ID;
		goto err;
	}

	/** @req OS391 */
	if( *val > Os_CounterGetMaxValue(cPtr) ) {
		rv = E_OS_VALUE;
		goto err;
	}

	GetCounterValue(counter_id,&tick);

#warning missing....OS382

	COUNTER_STD_END;
}

/*
 * The OsTick():
 * 1. The Decrementer is setup by Os_SysTickStart(period_ticks)
 * 2. Os_SysTickInit() setup INTC[7] to trigger OsTick
 * 3. OsTick() then increment counter os_tick_counter if used
 */

/*
 * Non-Autosar stuff
 */

/* The id of the counter driven by the os tick, or -1 if not used.
 * Using weak linking to set default value -1 if not set by config.
 */
CounterType Os_Arc_OsTickCounter __attribute__((weak)) = -1;

void OsTick( void ) {
	// if not used, os_tick_counter < 0
	if (Os_Arc_OsTickCounter >= 0) {

		OsCounterType *c_p = Oil_GetCounter(Os_Arc_OsTickCounter);

		os_sys.tick++;

		IncCounter(c_p);

	//	os_sys.tick = c_p->val;

		check_alarms(c_p);
		check_stbl(c_p);
	}
}

TickType GetOsTick( void ) {
	return get_os_tick();
}

