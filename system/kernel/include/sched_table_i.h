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

#ifndef SCHED_TABLE_I_H_
#define SCHED_TABLE_I_H_

/* Should hold the internal API used by the schedule tables */

#define SCHEDULE_ACTION_ACTIVATETASK	0
#define SCHEDULE_ACTION_SETEVENT	1

#include "alist_i.h"
#include "counter_i.h"

struct OsCounter;

#define SINGLE_SHOT 	0
#define REPEATING		1

#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
enum OsScheduleTableSyncStrategy {
	/* Support for sync, this is same as no OS */
	NONE,
	/* synchronize with "external" counter */
	EXPLICIT,
	/* sync internal */
	IMPLICIT,
};
#endif

enum OsScheduleTableAutostartType {
	// Start with StartScheduleTableAbs()
	SCHTBL_AUTOSTART_ABSOLUTE,
	// Start with StartScheduleTableRel()
	SCHTBL_AUTOSTART_RELATIVE,
	// Start with StartScheduleTableSyncon()
	SCHTBL_AUTOSTART_SYNCHRONE,
};


/* STD container: OsScheduleTableEventSetting
 * OsScheduleTableSetEventRef: 		1
 * OsScheduleTableSetEventTaskRef:	1
*/
typedef struct OsScheduleTableEventSetting {
	EventMaskType 	event;
	TaskType     task;
} OsScheduleTableEventSettingType;

/* OsScheduleTableTaskActivation */

/* STD container: OsScheduleTableExpiryPoint
 * OsScheduleTblExpPointOffset: 	1    Int
 * OsScheduleTableEventSetting:		0..*
 * OsScheduleTableTaskActivation:	0..*
 * OsScheduleTblAdjustableExpPoint:	0..1
 * */

/** @req OS402 */
/** @req OS403 */
typedef struct OsScheduleTableExpiryPoint {
  	/* The expiry point offset, OsScheduleTblExpPointOffset */
	/** @req OS404 */
	uint64 			offset;
   	// delta to next action

	//uint64    		delta;

	/* List of events to activate */
	const TaskType 		*taskList;
	uint8_t 		taskListCnt;

	/* List of events to activate */
	const OsScheduleTableEventSettingType *eventList;
	uint8_t 		eventListCnt;
} OsScheduleTableExpiryPointType;


#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
typedef struct OsScheduleTableSync {

/* SPEC */
	enum OsScheduleTableSyncStrategy syncStrategy;
	// from spec. (only if syncStrategy==EXPLICIT )
	int explicitPrecision;

/* OWN */
	// This counter is advanced by the driver counter but is synchronized
	// by SyncScheduleTable()
	GlobalTimeTickType syncCounter;

	// This is the deviation from the sync counter to the drive counter.
	// (set by SyncScheduleTable())
	// Calculated as 'driver count' - 'global time count from SyncScheduleTable()'
	int deviation;

} OsScheduleTableSyncType;


/* SPEC */
typedef struct OsSchTblAdjExpPoint {
	uint8_t	maxAdvance;
	uint8_t	maxRetard;
} OsSchTblAdjExpPointType;
#endif

/* STD container: OsScheduleTableAutostart
 * OsScheduleTableAbsValue  		1    Int
 * OsScheduleTableAutostartType     1    Enum
 * OsScheduleTableRelOffset         1    Int
 * OsScheduleTableAppModeRef 		1..* Ref to OsAppMode
 */
struct OsSchTblAutostart {
	enum OsScheduleTableAutostartType type;
	/* offset applies to both rel and abs */
	TickType offset;
	uint32_t appMode;	// TODO
};


/* STD container: OsScheduleTable
 * OsScheduleTableDuration: 		1    Int
 * OsScheduleTableRepeating: 		1	 Bool
 * OsSchTblAccessingApplication		0..* Ref
 * OsScheduleTableCounterRef:		1	 Ref
 * OsScheduleTableAutostart[C]		0..1
 * OsScheduleTableExpiryPoint[C]	1..*
 * OsScheduleTableSync 				0..1
 */

typedef struct OsSchTbl {
	/* OsScheduleTableDuration */
	TickType duration;

	char *name;
	/* If true, the schedule is periodic, OS009
	 * OsScheduleTableRepeating , 0 - SINGLE_SHOT */
	/** @req OS413 */
	_Bool repeating;

	// pointer to this tables counter
	// OsScheduleTableCounterRef
	/** @req OS409 */
	struct OsCounter *counter;

	/* OsScheduleTableAutostart[C] */
	const struct OsSchTblAutostart *autostartPtr;

	/* NULL if NONE, and non-NULL if EXPLICIT and IMPLICIT */
	struct OsScheduleTableSync *sync;

#if (OS_SC3 == STD_ON ) || (OS_SC4 == STD_ON )
	uint32 app_mask;
#endif

#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
	struct OsSchTblAdjExpPoint adjExpPoint;
#endif
// RAM

	uint32 id;

	/* The current index into the expire list
	 * The value is updated at each expire point. */
	int expire_curr_index;

	/* When this table expires the next time
	 * This value should be compared to the counter that drives
	 * the counter to check if it has expired.
	 * The value is updated at each expire point. */
	TickType expire_val;

	ScheduleTableStatusType state;

	/* Pointer to next schedule table, if any
	 * (don't use normal lists here since we have no list head) */
	struct OsSchTbl *next;

	/* Head of static expire point list
	 * OsScheduleTableExpiryPoint[C] */
	SA_LIST_HEAD(alist,OsScheduleTableExpiryPoint) expirePointList;

	/* Entry in the list of schedule tables connected to a specific
	 * counter  */
	SLIST_ENTRY(OsSchTbl) sched_list;

} OsSchTblType;

/*
#define os_stbl_get_action(x) 		SA_LIST_GET(&(x)->expirePointList,(x)->expire_curr_index)
#define os_stbl_get_action_type(x) os_stbl_get_action(x)->type
#define os_stbl_get_action_offset(x) os_stbl_get_action(x)->offset
#define os_stbl_get_action_pid(x) os_stbl_get_action(x)->pid
#define os_stbl_get_action_event(x) os_stbl_get_action(x)->event
*/

void Os_SchTblInit( void );
void Os_SchTblAutostart( void );
void Os_SchTblCalcExpire( OsSchTblType *stbl );
void Os_SchTblCheck(OsCounterType *c_p);
void Os_SchTblAutostart( void );


static inline TickType Os_SchTblGetInitialOffset( OsSchTblType *sPtr ) {
	return SA_LIST_GET(&sPtr->expirePointList,0)->offset;
}

static inline TickType Os_SchTblGetFinalOffset( OsSchTblType *sPtr ) {
	return (sPtr->duration -
			SA_LIST_GET(&sPtr->expirePointList, SA_LIST_CNT(&sPtr->expirePointList)-1)->offset);
}



#endif /*SCHED_TABLE_I_H_*/
