/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/

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
#include "multicore_i.h"

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

	/* Disallow incrementing a counter that is bound to an application
	 * running on a different core than the current */
	if (Os_ApplGetCore(cPtr->applOwnerId) != GetCoreID()) {
		rv = E_OS_ACCESS; /* ASR spec does not say what error to return */
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

#if	(OS_APPLICATION_CNT > 1) && (OS_NUM_CORES > 1)
	if (Os_ApplGetCore(cPtr->applOwnerId) != GetCoreID()) {
		StatusType status = Os_NotifyCore(Os_ApplGetCore(cPtr->applOwnerId),
		                                  OSServiceId_GetCounterValue,
		                                  counter_id,
		                                  (uint32_t)tick_ref,
		                                  0);
		return status;
	}
#endif

	/** @req OS376 */
	if( !IsCounterValid(counter_id) ) {
		rv = E_OS_ID;
		goto err;
	}

	/** @req OS377 */
	if( cPtr->type == COUNTER_TYPE_HARD ) {
		if( cPtr->driver == NULL ) {
			/* It's OSINTERNAL */
			*tick_ref = OS_SYS_PTR->tick;
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

#if	(OS_APPLICATION_CNT > 1) && (OS_NUM_CORES > 1)
	if (Os_ApplGetCore(cPtr->applOwnerId) != GetCoreID()) {
		StatusType status = Os_NotifyCore(Os_ApplGetCore(cPtr->applOwnerId),
		                                  OSServiceId_GetElapsedValue,
		                                  counter_id,
		                                  (int32_t)val,
		                                  (int32_t)elapsed_val);
		return status;
	}
#endif

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

#if 0
/* TODO: Fix this once the whole compiler / multicore combo gets figured out */

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
#endif

extern const CounterType Os_Arc_OsTickCounter[];

void OsTick( void ) {
	// if not used, os_tick_counter < 0
	if (Os_Arc_OsTickCounter[GetCoreID()] >= 0) {

		OsCounterType *cPtr = Os_CounterGet(Os_Arc_OsTickCounter[GetCoreID()]);
#if defined(USE_KERNEL_EXTRA)
		OsTaskVarType *pcbPtr;
#endif

		OS_SYS_PTR->tick++;

		cPtr->val = Os_CounterAdd( cPtr->val, Os_CounterGetMaxValue(cPtr), 1 );

#if defined(USE_KERNEL_EXTRA)
		/* Check tasks in the timer queue (here from Sleep() or WaitSemaphore() ) */
		TAILQ_FOREACH(pcbPtr, &OS_SYS_PTR->timerHead, timerEntry ) {
			--pcbPtr->timerDec;
			if( pcbPtr->timerDec <= 0 ) {
				/* Remove from the timer queue */
				TAILQ_REMOVE(&OS_SYS_PTR->timerHead, pcbPtr, timerEntry);
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
	return OS_SYS_PTR->tick;
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

