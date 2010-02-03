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
	ABSOLUTE,
	// Start with StartScheduleTableRel()
	RELATIVE,
	// Start with StartScheduleTableSyncon()
	SYNCHRONE,
};


typedef struct OsScheduleTableAction {
	// 0 - activate task, 1 - event
	/** @req OS402 */
	/** @req OS403 */
	int   			type;
  	// for debug only???
	uint64 			offset;
   	// delta to next action
	/** @req OS404 */
	uint64    		delta;
	TaskType 		task_id;
	// used only if event..
	EventMaskType event_id;
} OsScheduleTableActionType;

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
	_Bool active;
	enum OsScheduleTableAutostartType type;
	uint32_t relOffset;
	uint32_t appModeRef;	// TODO
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

// Configuration

	// OsScheduleTableDuration
	int duration;

	// If true, the schedule is periodic, OS009
	// OsScheduleTableRepeating , 0 - SINGLE_SHOT
	_Bool repeating;

	// Application mask
	uint32 app_mask;

	// pointer to this tables counter
	// OsScheduleTableCounterRef
	/** @req OS409 */
	struct OsCounter *counter;

	struct OsSchTblAutostart autostart;

	/* NULL if NONE, and non-NULL if EXPLICIT and IMPLICIT */
	struct OsScheduleTableSync *sync;

#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
	struct OsSchTblAdjExpPoint adjExpPoint;
#endif

// Private stuff

	uint32_t finalOffset;

	uint32_t initialOffset;
	// Name...
	char *name;

	// ??
// RAM
	uint64 length;

	uint32 id;

	/* The current index into the expire list
	 * The value is updated at each expire point.
	 * */
	int expire_curr_index;

	/* When this table expires the next time
	 * This value should be compared to the counter that drives
	 * the counter to check if it has expired.
	 * The value is updated at each expire point.
	 */
	TickType expire_val;

	// if true, the table is active
	//_Bool active;
	ScheduleTableStatusType state;

	// Pointer to next schedule table, if any
	// (don't use normal lists here since we have no list head)
	struct OsSchTbl *next;

	/* Head of static action list */
	SA_LIST_HEAD(alist,OsScheduleTableAction) action_list;

	/* Entry in the list of schedule tables connected to a specfic
	 * counter  */
	SLIST_ENTRY(OsSchTbl) sched_list;

	// TableDuration
	//

} OsSchTblType;

/*
#define os_stbl_get_action(x) 		SA_LIST_GET(&(x)->action_list,(x)->expire_curr_index)
#define os_stbl_get_action_type(x) os_stbl_get_action(x)->type
#define os_stbl_get_action_offset(x) os_stbl_get_action(x)->offset
#define os_stbl_get_action_pid(x) os_stbl_get_action(x)->pid
#define os_stbl_get_action_event(x) os_stbl_get_action(x)->event
*/

void Os_SchTblInit( void );
void Os_SchTblCalcExpire( OsSchTblType *stbl );

static inline TickType Os_SchTblGetInitialOffset( OsSchTblType *sPtr ) {
	return SA_LIST_GET(&sPtr->action_list,0)->offset;
}

static inline TickType Os_SchTblGetFinalOffset( OsSchTblType *sPtr ) {
	return (sPtr->duration -
			SA_LIST_GET(&sPtr->action_list, SA_LIST_CNT(&sPtr->action_list))->offset);
}



#endif /*SCHED_TABLE_I_H_*/
