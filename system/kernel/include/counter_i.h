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

#ifndef COUNTER_I_H_
#define COUNTER_I_H_

#include "Os.h"
#include "internal.h"
#include "os_config_macros.h"
#include <sys/queue.h>

#define COUNTER_TYPE_HARD 	0
#define COUNTER_TYPE_SOFT	1

#define COUNTER_UNIT_TICKS	0
#define COUNTER_UNIT_NANO	1

/* STD container : OsCounter
 * OsCounterMaxAllowedValue: 		1    Integer
 * OsCounterMinCycle:				1	 Integer
 * OsCounterTicksPerBase:			1 	 Integer
 * OsCounterType:               	1    Enum HARDWARE/SOFTWARE
 * OsSecondsPerTick: 				0..1 Float
 * OsCounterAccessingApplication: 	0..* Ref
 * OsDriver[C]:						0..1
 * OsTimeConstant[C]:				0..*
 */

typedef struct OsCounter {
	char 	name[16];
	_Bool 	type;			/* hardware or software counter, SWS OS255 */
	_Bool 	unit;			/* Ticks or nano, SWS OS331 */

	uint32_t val;			/* The counter value ( if software counter ) */
#if (OS_USE_APPLICATIONS == STD_ON)
	ApplicationType applOwnerId;
	uint32 			accessingApplMask;		/* Application mask, SWS OS317 */
#endif

	AlarmBaseType alarm_base;
	OsDriver *	driver;						/* Used only if we configure a GPT timer as os timer */

	SLIST_HEAD(slist,OsAlarm) alarm_head;	/* List of alarms this counter is connected to
											 * Overkill ??? Could have list of id's here,
											 * but easier to debug this way */

	SLIST_HEAD(sclist,OsSchTbl) sched_head;	/* List of schedule-table connected to this counter */
} OsCounterType;


#if OS_COUNTER_CNT!=0
extern GEN_COUNTER_HEAD;
#endif

static inline TickType Os_CounterGetMaxValue(OsCounterType *cPtr ) {
	return cPtr->alarm_base.maxallowedvalue;
}

static inline TickType Os_CounterGetMinCycle(OsCounterType *cPtr ) {
	return cPtr->alarm_base.mincycle;
}

static inline TickType Os_CounterGetValue( OsCounterType *cPtr ) {
	return cPtr->val;
}

static inline OsCounterType *Os_CounterGet(CounterType id) {
#if OS_COUNTER_CNT!=0
	return &counter_list[id];
#else
	(void)id;
	return NULL;
#endif
}

static inline ApplicationType Os_CounterGetApplicationOwner( CounterType id ) {
	ApplicationType rv;
	if( id < OS_COUNTER_CNT ) {
		rv = Os_CounterGet(id)->applOwnerId;
	} else {
		rv = INVALID_OSAPPLICATION;
	}
	return rv;
}


static inline TickType Os_CounterDiff( TickType curr, TickType old, TickType max ) {
	/* + 1 here because it do diff one between OSMAXALLOWEDVALUE and 0.
	 * That is, if curr = 0, max = 9 and old = 0, then the diff should be 1.
	 */
	return (curr >= old ) ? (curr - old) : (curr + (max - old) + 1);
}

/**
 * Add value to a counter (that have a max value )
 *
 * @param curr	The current counter value
 * @param max	The max value of the counter
 * @param add	The value do add
 * @return
 */
static inline TickType Os_CounterAdd( TickType curr, TickType max, TickType add ) {
	TickType diff = max - curr;
	TickType result;
	if( add <= diff  ) {
		result = curr + add;
	} else {
		result = add - (max - curr ) - 1;
	}

	return result;
}

void Os_CounterInit( void );

#endif /*COUNTER_I_H_*/
