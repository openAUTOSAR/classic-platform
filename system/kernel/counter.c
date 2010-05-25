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


static inline const struct OsSchTblAutostart *getAutoStart( OsSchTblType *stblPtr ) {
	return stblPtr->autostartPtr;
}

#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
static inline struct OsScheduleTableSync *getSync( OsSchTblType *stblPtr ) {
	return &stblPtr->sync;
}
#endif


#define IsCounterValid(_counterId)   ((_counterId) <= Os_CfgGetCounterCnt())

/**
 *
 * @param counter_id
 * @return
 */

/** @req OS399 */
StatusType IncrementCounter( CounterType counter_id ) {
	StatusType rv = E_OK;
	OsCounterType *cPtr;
	cPtr = Os_CfgGetCounter(counter_id);

	/** @req OS376 */
	if( !IsCounterValid(counter_id) ) {
		rv = E_OS_ID;
		goto err;
	}

	/* Check param */
	/** @req OS285 */
	if( ( cPtr->type != COUNTER_TYPE_SOFT ) ||
		( counter_id >= Os_CfgGetCounterCnt() ) ) {
		rv =  E_OS_ID;
		goto err;
	}

	/** @req OS286 */
	cPtr->val = Os_CounterAdd( cPtr->val, Os_CounterGetMaxValue(cPtr), 1 );

	Os_AlarmCheck(cPtr);
	Os_SchTblCheck(cPtr);

	/** @req OS321 */
	COUNTER_STD_END;
}


/** @req OS383 */
StatusType GetCounterValue( CounterType counter_id , TickRefType tick_ref)
{
	StatusType rv = E_OK;
	OsCounterType *cPtr;
	cPtr = Os_CfgGetCounter(counter_id);

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

/**
 *
 * @param counter_id		The counter to be read
 * @param val[in,out]		in,  The previously read tick value of the counter
 * 							out, Contains the current tick value of the counter.
 * @param elapsed_val[out]  The difference
 * @return
 */

/** @req OS392 */
StatusType GetElapsedCounterValue( CounterType counter_id, TickRefType val, TickRefType elapsed_val)
{
	StatusType rv = E_OK;
	OsCounterType *cPtr;
	TickType currTick = 0;
	TickType max;

	cPtr = Os_CfgGetCounter(counter_id);

	/** @req OS381 */
	if( !IsCounterValid(counter_id) ) {
		rv = E_OS_ID;
		goto err;
	}

	max = Os_CounterGetMaxValue(cPtr);
	/** @req OS391 */
	if( *val > max ) {
		rv = E_OS_VALUE;
		goto err;
	}

	GetCounterValue(counter_id,&currTick);

	/** @req OS382 */
	*elapsed_val = Os_CounterDiff(currTick,*val,max);

	/** @req OS460 */
	*val = currTick;

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

		OsCounterType *cPtr = Os_CfgGetCounter(Os_Arc_OsTickCounter);

		os_sys.tick++;

		cPtr->val = Os_CounterAdd( cPtr->val, Os_CounterGetMaxValue(cPtr), 1 );

	//	os_sys.tick = cPtr->val;

		Os_AlarmCheck(cPtr);
		Os_SchTblCheck(cPtr);
	}
}

TickType GetOsTick( void ) {
	return get_os_tick();
}


/**
 * Initialize alarms and schedule-tables for the counters
 */
void Os_CounterInit( void ) {
	OsCounterType *counter;
	OsAlarmType *alarm_obj;
	OsSchTblType *sched_obj;
	/* Create a list from the counter to the alarms */
	for(int i=0; i < Os_CfgGetCounterCnt() ; i++) {
		counter = Os_CfgGetCounter(i);
		// Alarms
		SLIST_INIT(&counter->alarm_head);
		for(int j=0; j < Os_CfgGetAlarmCnt(); j++ ) {
			alarm_obj = Os_CfgGetAlarmObj(j);
			// Add the alarms
			SLIST_INSERT_HEAD(&counter->alarm_head,alarm_obj, alarm_list);
		}
		// Schedule tables
		SLIST_INIT(&counter->sched_head);
		for(int j=0; j < Os_CfgGetSchedCnt(); j++ ) {
			sched_obj = Os_CfgGetSched(j);
			// Add the alarms
			SLIST_INSERT_HEAD(&counter->sched_head,
								sched_obj,
								sched_list);
		}


	}
}

