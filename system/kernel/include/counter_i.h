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
	char name[16];
	// hardware or software counter, SWS OS255
	_Bool type;
	// Ticks or nano, SWS OS331
	_Bool unit;
	// The counter value ( if software counter )
	uint32_t val;
	// Application mask, SWS OS317
#if (OS_USE_APPLICATIONS == STD_ON)
	ApplicationType applOwnerId;
	uint32 accessingApplMask;
#endif
	//  hmm, strange to call it alarm base.... but see spec.
	AlarmBaseType alarm_base;
	/* Used only if we configure a GPT timer as os timer */
	OsDriver *driver;
	/* List of alarms this counter is connected to
	 * Overkill ??? Could have list of id's here, but easier to debug this way*/
	SLIST_HEAD(slist,OsAlarm) alarm_head;
	/* List of scheduletable connected to this counter */
	SLIST_HEAD(sclist,OsSchTbl) sched_head;
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
