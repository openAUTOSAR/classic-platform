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
#include "counter_i.h"
#include "sys.h"
#include "alarm_i.h"
#include "sched_table_i.h"
#include "application.h"
#include "arc.h"

#define COUNTER_STD_END 	\
		goto ok;		\
	err:				\
		ERRORHOOK(rv);	\
	ok:					\
		return rv;



#define IsCounterValid(_counterId)   ((_counterId) <= OS_COUNTER_CNT)

/**
 *
 * @param counter_id
 * @return
 */

/** @req OS399 */
StatusType IncrementCounter( CounterType counter_id ) {
	StatusType rv = E_OK;
	OsCounterType *cPtr;
	uint32_t flags;
	cPtr = Os_CounterGet(counter_id);

#if	(OS_APPLICATION_CNT > 1)

	rv = Os_ApplHaveAccess( cPtr->accessingApplMask );
	if( rv != E_OK ) {
		goto err;
	}

#endif


	Irq_Save(flags);
	/** @req OS376 */
	if( !IsCounterValid(counter_id) ) {
		rv = E_OS_ID;
		Irq_Restore(flags);
		goto err;
	}

	/* Check param */
	/** @req OS285 */
	if( ( cPtr->type != COUNTER_TYPE_SOFT ) ||
		( counter_id >= OS_COUNTER_CNT ) ) {
		rv =  E_OS_ID;
		Irq_Restore(flags);
		goto err;
	}

	/** @req OS286 */
	cPtr->val = Os_CounterAdd( cPtr->val, Os_CounterGetMaxValue(cPtr), 1 );

#if OS_ALARM_CNT!=0
	Os_AlarmCheck(cPtr);
#endif
#if OS_SCHTBL_CNT!=0
	Os_SchTblCheck(cPtr);
#endif

	Irq_Restore(flags);

	/** @req OS321 */
	COUNTER_STD_END;
}


/** @req OS383 */
StatusType GetCounterValue( CounterType counter_id , TickRefType tick_ref)
{
	StatusType rv = E_OK;
	OsCounterType *cPtr;
	cPtr = Os_CounterGet(counter_id);

	/** @req OS376 */
	if( !IsCounterValid(counter_id) ) {
		rv = E_OS_ID;
		goto err;
	}

	/** @req OS377 */
	if( cPtr->type == COUNTER_TYPE_HARD ) {
		if( cPtr->driver == NULL ) {
			/* It's OSINTERNAL */
			*tick_ref = Os_Sys.tick;
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

	cPtr = Os_CounterGet(counter_id);

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
#if defined(__DCC__)
#pragma weak Os_Arc_OsTickCounter
#endif
#if defined(__ICCHCS12__)
extern CounterType Os_Arc_OsTickCounter;
#else
CounterType Os_Arc_OsTickCounter __attribute__((weak)) = -1;
#endif

void OsTick( void ) {
	// if not used, os_tick_counter < 0
	if (Os_Arc_OsTickCounter >= 0) {

		OsCounterType *cPtr = Os_CounterGet(Os_Arc_OsTickCounter);
#if defined(USE_KERNEL_EXTRA)
		OsTaskVarType *pcbPtr;
#endif

		Os_Sys.tick++;

		cPtr->val = Os_CounterAdd( cPtr->val, Os_CounterGetMaxValue(cPtr), 1 );

#if defined(USE_KERNEL_EXTRA)
		/* Check tasks in the timer queue (here from Sleep() or WaitSemaphore() ) */
		TAILQ_FOREACH(pcbPtr, &Os_Sys.timerHead, timerEntry ) {
			--pcbPtr->timerDec;
			if( pcbPtr->timerDec <= 0 ) {
				/* Remove from the timer queue */
				TAILQ_REMOVE(&Os_Sys.timerHead, pcbPtr, timerEntry);
				/* ... and add to the ready queue */
				Os_TaskMakeReady(pcbPtr);
			}
		}
#endif
#if OS_ALARM_CNT!=0
		Os_AlarmCheck(cPtr);
#endif
#if OS_SCHTBL_CNT!=0
		Os_SchTblCheck(cPtr);
#endif
	}
}

TickType GetOsTick( void ) {
	return Os_Sys.tick;
}


/**
 * Initialize alarms and schedule-tables for the counters
 */
void Os_CounterInit( void ) {
#if OS_ALARM_CNT!=0
	{
		OsCounterType *cPtr;
		OsAlarmType *aPtr;

		/* Add the alarms to counters */
		for (int i = 0; i < OS_ALARM_CNT; i++) {
			aPtr = Os_AlarmGet(i);
			cPtr = aPtr->counter;
			SLIST_INSERT_HEAD(&cPtr->alarm_head, aPtr, alarm_list);
		}
	}
#endif

#if OS_SCHTBL_CNT!=0
	{
		OsCounterType *cPtr;
		OsSchTblType *sPtr;

		/* Add the schedule tables to counters */
		for(int i=0; i < OS_SCHTBL_CNT; i++ ) {

			sPtr = Os_SchTblGet(i);
			cPtr = sPtr->counter;
			SLIST_INSERT_HEAD(&cPtr->sched_head, sPtr, sched_list);
		}
	}
#endif
}

