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

	NONE,	   	/* Support for sync, this is same as no OS */
	EXPLICIT,  	/* synchronize with "external" counter */
	IMPLICIT,  	/* sync internal */
};
#endif

enum OsScheduleTableAutostartType {
	SCHTBL_AUTOSTART_ABSOLUTE,	/* Start with StartScheduleTableAbs() */
	SCHTBL_AUTOSTART_RELATIVE,	/* Start with StartScheduleTableRel() */
	SCHTBL_AUTOSTART_SYNCHRONE	/* StartScheduleTableSyncon() */
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
	/** @req OS404 */
	uint64 			  	offset;
	const TaskType 	* 	taskList;		/* List of events to activate */
	uint8_t 		  	taskListCnt;
	uint8_t 			eventListCnt;
	const OsScheduleTableEventSettingType *eventList;	/* List of events to activate */
} OsScheduleTableExpiryPointType;

#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
typedef struct OsScheduleTableSync {

/* SPEC */
	enum OsScheduleTableSyncStrategy syncStrategy;
	int explicitPrecision;		/* from spec. (only if syncStrategy==EXPLICIT ) */

/* OWN */
	GlobalTimeTickType syncCounter;	/* This counter is advanced by the driver counter but is
									 * synchronized by SyncScheduleTable() */

	int deviation;					/* This is the deviation from the sync counter to the drive counter.
	 	 	 	 	 	 	 	 	 * (set by SyncScheduleTable())
	 	 	 	 	 	 	 	 	 * Calculated as 'driver count' - 'global time count from SyncScheduleTable()' */
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
	char *	 name;
	TickType duration;			/* OsScheduleTableDuration */
	/** @req OS413 */
	_Bool repeating;			/* If true, the schedule is periodic, OS009
	 	 	 	 	 	 	 	 * OsScheduleTableRepeating , 0 - SINGLE_SHOT */
#if (OS_USE_APPLICATIONS == STD_ON)
	ApplicationType applOwnerId;
	uint32 			accessingApplMask;
#endif

	/** @req OS409 */
	struct OsCounter *				 counter;		/* pointer to this tables counter, OsScheduleTableCounterRef */
	const struct OsSchTblAutostart * autostartPtr;	/* OsScheduleTableAutostart[C] */
	struct OsScheduleTableSync *	 sync;			/* NULL if NONE, and non-NULL if EXPLICIT and IMPLICIT */

#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
	struct OsSchTblAdjExpPoint adjExpPoint;
#endif

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


void Os_SchTblInit( void );
void Os_SchTblCalcExpire( OsSchTblType *stbl );
void Os_SchTblCheck(OsCounterType *c_p);
void Os_SchTblAutostart( void );

#if (OS_SCHTBL_CNT!=0)
extern GEN_SCHTBL_HEAD;
#endif

static inline OsSchTblType *Os_SchTblGet( ScheduleTableType sched_id ) {
#if (OS_SCHTBL_CNT!=0)
	if(sched_id < OS_SCHTBL_CNT) {
		return &sched_list[sched_id];
	} else {
		return NULL;
	}
#else
	(void)sched_id;
	return NULL;
#endif
}

static inline TickType Os_SchTblGetInitialOffset( OsSchTblType *sPtr ) {
	return SA_LIST_GET(&sPtr->expirePointList,0)->offset;
}

static inline TickType Os_SchTblGetFinalOffset( OsSchTblType *sPtr ) {
	return (sPtr->duration -
			SA_LIST_GET(&sPtr->expirePointList, SA_LIST_CNT(&sPtr->expirePointList)-1)->offset);
}

static inline ApplicationType Os_SchTblGetApplicationOwner( ScheduleTableType id ) {
	ApplicationType rv = INVALID_OSAPPLICATION;
#if (OS_SCHTBL_CNT!=0)

	if( id < OS_SCHTBL_CNT ) {
		rv = Os_SchTblGet(id)->applOwnerId;
	}
#else
	(void)id;
#endif
	return rv;
}


/* Accessor functions */
#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
static inline OsSchTblAdjExpPointType *getAdjExpPoint( OsSchTblType *stblPtr ) {
	return &stblPtr->adjExpPoint;
}
#endif


#endif /*SCHED_TABLE_I_H_*/
