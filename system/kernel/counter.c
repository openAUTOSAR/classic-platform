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









#include "Os.h"
#include "types.h"
#include "counter_i.h"
#include "ext_config.h"
#include "alarm_i.h"
#include <assert.h>
#include <stdlib.h>
#include "hooks.h"
#include "sched_table_i.h"
#include "internal.h"

#define MIN(_x,_y) (((_x) < (_y)) ? (_x) : (_y))
#define MAX(_x,_y) (((_x) > (_y)) ? (_x) : (_y))


#define COUNTER_MAX(x) 			(x)->counter->alarm_base.maxallowedvalue

#define COUNTER_STD_END 	\
		goto ok;		\
	err:				\
		ERRORHOOK(rv);	\
	ok:					\
		return rv;


/* Accessor functions */
static inline Stbl_AdjustableExpPointType *getAdjExpPoint( sched_table_t *stblPtr ) {
	return &stblPtr->adjExpPoint;
}

static inline struct sched_table_autostart_s *getAutoStart( sched_table_t *stblPtr ) {
	return &stblPtr->autostart;
}

static inline struct sched_table_sync_s *getSync( sched_table_t *stblPtr ) {
	return &stblPtr->sync;
}


/**
 *
 * @param curr
 * @param max
 * @param add
 * @return
 */
static TickType os_calc_modulo( TickType curr, TickType max, TickType add ) {
	TickType diff = max - curr;
//	return (diff > add ) ? (curr + add) :
//							(add - curr);
	return (add>diff) ? (add-diff) : (curr+add);
}

/**
 *
 * @param a_obj
 */
static void AlarmProcess( alarm_obj_t *a_obj ) {
	if( a_obj->cycletime == 0 ) {
		a_obj->active = 0;
	} else {
		// Calc new expire value..
		a_obj->expire_val = os_calc_modulo(	a_obj->expire_val,
											COUNTER_MAX(a_obj),
											a_obj->cycletime);
	}
}

static void check_alarms( counter_obj_t *c_p ) {
	alarm_obj_t *a_obj;

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
					assert(0);
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
static void check_stbl( counter_obj_t *c_p ) {
	sched_table_t *sched_obj;

	/* Iterate through the schedule tables */
	SLIST_FOREACH(sched_obj,&c_p->sched_head,sched_list) {

		if( sched_obj->state == SCHEDULETABLE_STOPPED ) {
			continue;
		}

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

		/* Check if the expire point have been hit */
		if( (sched_obj->state == SCHEDULETABLE_RUNNING ||
				SCHEDULETABLE_RUNNING_AND_SYNCHRONOUS ) &&
				(c_p->val >= sched_obj->expire_val) ) {
			sched_action_t * action;

			action = SA_LIST_GET(&sched_obj->action_list,sched_obj->expire_curr_index);

			switch( action->type ) {
				case SCHEDULE_ACTION_ACTIVATETASK:
				ActivateTask(action->task_id);
				break;

				case SCHEDULE_ACTION_SETEVENT:
				SetEvent( action->task_id, action->event_id);
				break;

				default:
				assert(0);
			}
			// Calc new expire val
			os_stbl_calc_expire(sched_obj);
		}

	}
}


/**
 * Increment a counter. Checks for wraps.
 *
 * @param counter Ptr to a counter object
 */
static void IncCounter( counter_obj_t *counter ) {
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

/**
 *
 * @param counter_id
 * @return
 */

StatusType IncrementCounter( CounterType counter_id ) {
	StatusType rv = E_OK;

	/* Check the alarms associated with this timer */
	counter_obj_t *counter;
	counter = Oil_GetCounter(counter_id);

	/* Check param */

	if( ( counter->type != COUNTER_TYPE_SOFT ) ||
		( counter_id >= Oil_GetCounterCnt() ) ) {
		rv =  E_OS_ID;
		goto err;
	}

	IncCounter(counter);

	/* TODO: the other alarm_base.xxx values are still not done */
	check_alarms(counter);
	check_stbl(counter);

	COUNTER_STD_END;
}


TickType GetCounterValue_( counter_obj_t *c_p ) {
	if( c_p->type == COUNTER_TYPE_HARD ) {
		/* Grab the GPT */
#if 0
		// TODO: Move this outside the OS??
		// Is this the HW value??
		// No good way to check if something went wrong here. Can check for != 0
		// but that can really happen
		return (TickType)Gpt_GetTimeElapsed(c_p->driver.OsGptChannelRef );
#else
		return Frt_GetTimeElapsed();
#endif
	} else {
		return c_p->val;
	}
}


StatusType GetCounterValue( CounterType counter_id , TickRefType tick_ref)
{
	counter_obj_t *counter;
	counter = Oil_GetCounter(counter_id);

	*tick_ref = GetCounterValue_(counter);
	return E_OK;
}

StatusType GetElapsedCounterValue( CounterType counter_id, TickRefType val, TickRefType elapsed_val)
{
	return E_OK;
}

/*
 * The OsTick():
 * 1. The Decrementer is setup by Frt_Start(period_ticks)
 * 2. Frt_Init() setup INTC[7] to trigger OsTick
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

		counter_obj_t *c_p = Oil_GetCounter(Os_Arc_OsTickCounter);

		os_sys.tick++;

		IncCounter(c_p);

	//	os_sys.tick = c_p->val;

		check_alarms(c_p);
		check_stbl(c_p);
	}
}

#if 0
void OsIdle( void ) {
	for(;;);
}
#endif

TickType GetOsTick( void ) {
	return get_os_tick();
}

#if 0
StatusType InitCounter(AlarmType alarm_id ) {

	return E_OK;
}

StatusType StartCounter( AlarmType alarm_id ) {
	return E_OK;
}
#endif
