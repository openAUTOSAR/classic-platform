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

struct counter_obj_s;

enum OsScheduleTableSyncStrategy {
	/* Support for sync */
	NONE,
	/* synchronize with "external" counter */
	EXPLICIT,
	/* sync internal */
	IMPLICIT,
};

enum OsScheduleTableAutostartType {
	// Start with StartScheduleTableAbs()
	ABSOLUTE,
	// Start with StartScheduleTableRel()
	RELATIVE,
	// Start with StartScheduleTableSyncon()
	SYNCHRONE,
};


typedef struct sched_table_sync_s {

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

} sched_table_sync_t;

/* SPEC */
typedef struct Stbl_AdjustableExpPoint {
	uint8_t	maxAdvance;
	uint8_t	maxRetard;
} Stbl_AdjustableExpPointType;

struct sched_table_autostart_s {
	_Bool active;
	enum OsScheduleTableAutostartType type;
	uint32_t relOffset;
	uint32_t appModeRef;	// TODO
};

typedef struct sched_table_s {

// Configuration

	// OsScheduleTableDuration
	int duration;

	// If true, the schedule is periodic, OS009
	// OsScheduleTableRepeating
	_Bool repeating;

	// Application mask
	uint32 app_mask;

	// pointer to this tables counter
	// OsScheduleTableCounterRef
	struct counter_obj_s *counter;

	struct sched_table_autostart_s autostart;

	struct sched_table_sync_s sync;

	struct Stbl_AdjustableExpPoint adjExpPoint;

// Private stuff

	uint32_t final_offset;   // used?
	uint32_t init_offset;	 // used?
	// Name...
	char *name;

	// ??
// RAM
	uint64 length;

	uint32 id;

	int expire_curr_index;
	// When this table expires the next time
	TickType expire_val;
	// if true, the table is active
	//_Bool active;
	ScheduleTableStatusType state;

	// Pointer to next schedule table, if any
	// (don't use normal lists here since we have no list head)
	struct sched_table_s *next;

	/* Head of static action list */
	SA_LIST_HEAD(alist,sched_action_s) action_list;

	/* Entry in the list of schedule tables connected to a specfic
	 * counter  */
	SLIST_ENTRY(sched_table_s) sched_list;

	// TableDuration
	//

} sched_table_t;

/*
#define os_stbl_get_action(x) 		SA_LIST_GET(&(x)->action_list,(x)->expire_curr_index)
#define os_stbl_get_action_type(x) os_stbl_get_action(x)->type
#define os_stbl_get_action_offset(x) os_stbl_get_action(x)->offset
#define os_stbl_get_action_pid(x) os_stbl_get_action(x)->pid
#define os_stbl_get_action_event(x) os_stbl_get_action(x)->event
*/

void os_stbl_init( void );
void os_stbl_calc_expire( sched_table_t *stbl);

#endif /*SCHED_TABLE_I_H_*/
