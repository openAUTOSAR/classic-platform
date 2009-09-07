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
#include "pcb.h"
#include "sched_table_i.h"
#include "ext_config.h"
#include "hooks.h"
#include <stdlib.h>
#include "alist_i.h"
#include <assert.h>

/*
 * How Autosar sees the scheduletable
 *
 * duration
 * repeating
 * accessionApplication
 * counterRef
 * autostart [0..1]
 * |--- absValue   (only if type==ABSOLUTE )
 * |--- relOffset  (only if type==RELATIVE )
 * |--- type       (ABSOLUTE, RELATIVE, SYNCHRON )
 * |--- modeRef
 * |
 * expiryPoint [1..*]
 * |--- offset
 * |--- EventSetting [0..*]
 * |    |--- SetEvent
 * |    `--- SetEventTaskRef
 * |
 * |--- TaskActivation [0..*]
 * |    `- TaskRef
 * |
 * |--- AdjustableExpPoint [0..1] (only if syncStrategy!=NONE)
 * |    |--- maxAdvance
 * |    `--- macRetard
 * |
 * sync
 * |--- explicitPrecision (only if syncStrategy==EXPLICIT )
 * |--- syncStrategy  	  (NONE,EXPLICIT,IMPLICIT )
 *
 */


// Cancel

#define SCHED_CHECK_ID(x) 				\
	if( (x) > Oil_GetSchedCnt()) { \
		rv = E_OS_ID;					\
		goto err; 						\
	}

#define SCHED_STD_END 	\
		return rv;		\
	err:				\
		ERRORHOOK(rv);  \
		return rv;

extern TickType GetCountValue( counter_obj_t *counter );

static TickType os_calc_modulo( TickType curr, TickType max, TickType add ) {
	TickType diff = max - curr;
	return (diff >= add ) ? (curr + add) :
							(add - curr);
}

enum OsScheduleTableSyncStrategy getSyncStrategy( sched_table_t *stblPtr ) {
	return stblPtr->sync.syncStrategy;
}


/**
 * Consistency checks for scheduletables. This should really be checked by
 * the generator.
 *
 * See chapter 11.2.
 *
 * @return
 */
static void ScheduleTableConsistenyCheck( sched_table_t *s_p ) {

	// OS440
	if( s_p->sync.syncStrategy == IMPLICIT ) {
		assert( s_p->duration == (s_p->counter->alarm_base.maxallowedvalue +1) );
	}

	// OS431
	if( s_p->sync.syncStrategy == EXPLICIT ) {
		assert( s_p->duration <= (s_p->counter->alarm_base.maxallowedvalue +1) );
	}
}


// TODO: OS452,OS278
StatusType StartScheduleTableRel(ScheduleTableType sid, TickType offset) {
	StatusType rv = E_OK;
	sched_table_t *s_tbl;
	TickType max_offset;


	(void)offset;
	// OS275
	SCHED_CHECK_ID(sid);
	s_tbl = Oil_GetSched(sid);
	// OS276
	max_offset = s_tbl->counter->alarm_base.maxallowedvalue;
	if( (offset == 0) || (offset > max_offset )) {
		rv = E_OS_VALUE;
		goto err;
	}

	// OS277
	if( s_tbl->state != SCHEDULETABLE_STOPPED ) {
		rv = E_OS_STATE; goto err;
	}

	s_tbl->state = SCHEDULETABLE_RUNNING;
	// calculate the expire value..
	s_tbl->expire_val = os_calc_modulo( GetCounterValue_(s_tbl->counter), max_offset, offset );
//	s_tbl->expire_val = offset + SA_LIST_GET(&s_tbl->action_list,0)->offset;
//	ALIST_RESET(&s_tbl->action_list);
	s_tbl->state = SCHEDULETABLE_RUNNING;

	//	s_tbl->action_list_index = 0;

	SCHED_STD_END;
}

StatusType StartScheduleTableAbs(ScheduleTableType sid, TickType val ){
	StatusType rv = E_OK;
	(void)val;
	SCHED_CHECK_ID(sid);

	SCHED_STD_END;
}

/**
 *
 * @param sid
 * @return
 */

StatusType StartScheduleTableSynchron(ScheduleTableType sid ){
	sched_table_t *s_p;
	StatusType rv = E_OK;

	DisableAllInterrupts();

	SCHED_CHECK_ID(sid);

	// OS387
	if( s_p->sync.syncStrategy != EXPLICIT ) {
		rv = E_OS_ID;
		goto err;
	}

	// OS388
	if( s_p->state != SCHEDULETABLE_STOPPED ) {
		rv = E_OS_STATE;
		goto err;
	}

	// OS389
	s_p->state = SCHEDULETABLE_WAITING;

	EnableAllInterrupts();

	SCHED_STD_END;
}



StatusType StopScheduleTable(ScheduleTableType sid) {
	StatusType rv = E_OK;
	sched_table_t *s_tbl;
	SCHED_CHECK_ID(sid);
	s_tbl = Oil_GetSched(sid);

	s_tbl->state = SCHEDULETABLE_STOPPED;

	SCHED_STD_END;
}

StatusType NextScheduleTable( ScheduleTableType sid_curr, ScheduleTableType sid_next) {
	StatusType rv = E_OK;
	(void)sid_curr;
	(void)sid_next;

	sched_table_t *s_curr;
	sched_table_t *s_next;

	SCHED_CHECK_ID(sid_curr);
	SCHED_CHECK_ID(sid_next);

	s_curr = Oil_GetSched(sid_curr);
	s_next = Oil_GetSched(sid_curr);

	// OS330
	if( s_curr->counter != s_next->counter) {
		rv = E_OS_ID;
		goto err;
	}


	DisableAllInterrupts();

	// OS283
	if( s_curr->state == SCHEDULETABLE_STOPPED ||
		s_curr->state == SCHEDULETABLE_NEXT ||
		s_next->state == SCHEDULETABLE_STOPPED ||
		s_next->state == SCHEDULETABLE_NEXT )
	{
		rv = E_OS_NOFUNC;
		goto err;
	}

	// OS309
	if( s_next->state != SCHEDULETABLE_STOPPED ) {
		rv = E_OS_STATE;
		goto err;
	}

	// OS324
	if( s_curr->next != NULL ) {
		// Stop the scheduletable that was to be next.
		s_curr->next->state = SCHEDULETABLE_STOPPED;
	}

	s_curr->next = s_next;
	s_next->state = SCHEDULETABLE_NEXT;

	EnableAllInterrupts();

	SCHED_STD_END;
}



/**
 *
 * @param sid
 * @param globalTime
 * @return
 */
StatusType SyncScheduleTable( ScheduleTableType sid, GlobalTimeTickType globalTime  ) {
	StatusType rv = E_OK;
	sched_table_t *s_p =  Oil_GetSched(sid);

	SCHED_CHECK_ID(sid);

	// OS454
	if( s_p->sync.syncStrategy != EXPLICIT ) {
		rv =  E_OS_ID;
		goto err;
	}

	// OS455
	if( globalTime > s_p->duration ) {
		rv = E_OS_VALUE;
		goto err;
	}

	DisableAllInterrupts();

	// OS456
	if( (s_p->state == SCHEDULETABLE_STOPPED) ||
		(s_p->state == SCHEDULETABLE_NEXT)	) {
		rv = E_OS_STATE;
		goto err;
	}

	switch(s_p->state) {
	case  SCHEDULETABLE_WAITING:
		// First time we called since started. Set the sync counter to
		// the value provided.
		s_p->sync.syncCounter = globalTime;
		s_p->state = SCHEDULETABLE_RUNNING_AND_SYNCHRONOUS;
		break;

	case SCHEDULETABLE_RUNNING:
	case SCHEDULETABLE_RUNNING_AND_SYNCHRONOUS:
		s_p->sync.deviation = s_p->sync.syncCounter - globalTime;
		if( s_p->sync.deviation != 0 ) {
			// We are not at sync any more...
			s_p->state = SCHEDULETABLE_RUNNING;
		}
		break;

	default:
		assert(0);
		break;
	}

	EnableAllInterrupts();

	SCHED_STD_END;
}

/**
 *
 * @param sid
 * @param status
 * @return
 */
StatusType GetScheduleTableStatus( ScheduleTableType sid, ScheduleTableStatusRefType status ) {
	StatusType rv = E_OK;
	sched_table_t *s_p;
	(void)status;
	SCHED_CHECK_ID(sid);

	s_p = Oil_GetSched(sid);
	DisableAllInterrupts();

	switch(s_p->state) {
	case SCHEDULETABLE_STOPPED:					// OS289
	case SCHEDULETABLE_NEXT:					// OS353
	case SCHEDULETABLE_RUNNING_AND_SYNCHRONOUS:	// OS290
	case SCHEDULETABLE_WAITING:					// OS354
	case SCHEDULETABLE_RUNNING:					// OS291
		*status = s_p->state;
		break;
	default:
		assert(0);

	}

	EnableAllInterrupts();

	SCHED_STD_END;
}


/**
 *
 * @param sid
 * @return
 */
StatusType SetScheduleTableAsync( ScheduleTableType sid ) {
	StatusType rv = E_OK;
	sched_table_t *s_p = Oil_GetSched(sid);

	SCHED_CHECK_ID(sid);

	// OS458
	if( s_p->sync.syncStrategy != EXPLICIT ) {
		rv = E_OS_ID;
		goto err;
	}

	DisableAllInterrupts();

	// TODO: check OS362, OS323

	// OS300
	s_p->state = SCHEDULETABLE_RUNNING;

	EnableAllInterrupts();

	SCHED_STD_END;
}

/*
 *start e   e       e  delta stop
 * |----|---|-------|---------|
 *
 *
 *  |   s   e    e   cm   e
 *  |---|---|----|---|----|----------|
 *      1   2    3   4    5
 *  e-expiry point
 * cm-counter max( restart from 0)
 *  s-call to StartScheduleTableRel()
 */


/* TODO: Remove when we have a stable generator. The reason for this
 * funcion is that I'm afraid of if I change the maxallowedvalue for the
 * counter I will miss to update the delta values
 */
static void os_stbl_action_calc_delta( sched_table_t *stbl ) {
	sched_action_t * first;
	sched_action_t * second;
//	ALIST_DECL_ITER(iter);
	int iter;

	// calculate the delta to next action

	for(iter=1; iter <  SA_LIST_CNT(&stbl->action_list) ;iter++) {
		first = SA_LIST_GET(&stbl->action_list,iter-1);
		second = SA_LIST_GET(&stbl->action_list,iter);
		first->delta = second->offset - first->offset;
	}
	// calculate the last delta( to countes max value )
	first = SA_LIST_GET(&stbl->action_list, SA_LIST_CNT(&stbl->action_list)-1);
	first->delta = stbl->counter->alarm_base.maxallowedvalue - first->offset;
}

/**
 *
 */
void os_stbl_init( void ) {
	sched_table_t *s_p;
	for( int i=0; i < Oil_GetSchedCnt();i++ ) {
		s_p = Oil_GetSched(i);
		os_stbl_action_calc_delta(s_p);

		ScheduleTableConsistenyCheck(s_p);
	}
}

/**
 *
 * @param stbl
 */
void os_stbl_calc_expire( sched_table_t *stbl) {

	TickType old_delta;

	/* Any more actions in the action list?*/
	if( (stbl->expire_curr_index+1) >= SA_LIST_CNT(&stbl->action_list) ) {

		// TODO: final offset
		if( stbl->next != NULL ) {
			assert(stbl->state == SCHEDULETABLE_RUNNING);
		}

		if( !stbl->repeating ) {
			stbl->state = SCHEDULETABLE_STOPPED;
			stbl->expire_curr_index = 0;
			goto end;
		}
	}

	old_delta = SA_LIST_GET(&stbl->action_list,stbl->expire_curr_index)->delta;
	stbl->expire_curr_index++;
//	ALIST_INC(&stbl->action_list);

	stbl->expire_val =
 os_calc_modulo(	stbl->expire_val,
					stbl->counter->alarm_base.maxallowedvalue,
					old_delta);


#if 0
	TickType old_delta;
	if( ALIST_LAST(&stbl->action_list)) {
		if( !stbl->repeating ) {
			stbl->active = 0;
			ALIST_RESET(&stbl->action_list);
			goto end;
		}
	}

	old_delta = ALIST_GET_DATA(&stbl->action_list)->delta;
	ALIST_INC(&stbl->action_list);

	stbl->expire_val =
 os_calc_modulo(	stbl->expire_val,
					stbl->counter_id->alarm_base.maxallowedvalue,
					old_delta);
//					stbl->action_list[stbl->action_list_index]->delta);
#endif

end:
	return;
}


