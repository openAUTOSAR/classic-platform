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
#include "internal.h"
#include "alist_i.h"

/*
 * Generic requirements this module can handle
 */
 /** @req OS007 */
 /** @req OS410 */
 /** @req OS411 */
 /** @req OS347 */
 /** @req OS358 */

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

extern TickType GetCountValue( OsCounterType *counter );


#if 0
enum OsScheduleTableSyncStrategy getSyncStrategy( OsSchTblType *stblPtr ) {
	return stblPtr->sync.syncStrategy;
}
#endif


/**
 * Consistency checks for scheduletables. This should really be checked by
 * the generator.
 *
 * See chapter 11.2.
 *
 * @return
 */
static void ScheduleTableConsistenyCheck( OsSchTblType *sTblPtr ) {

#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
	/** @req OS440 */
	if( sTblPtr->sync.syncStrategy == IMPLICIT ) {
		assert( sTblPtr->duration == (sTblPtr->counter->alarm_base.maxallowedvalue +1) );
	}

	/** @req OS431 */
	if( sTblPtr->sync.syncStrategy == EXPLICIT ) {
		assert( sTblPtr->duration <= (sTblPtr->counter->alarm_base.maxallowedvalue +1) );
	}
#endif

	/** @req OS401 */
	assert(SA_LIST_CNT(&sTblPtr->expirePointList)>=1);



	{
		int iter;
		TickType delta = 0;
		TickType minCycle = Os_CounterGetMinCycle(sTblPtr->counter);
		TickType maxValue =  Os_CounterGetMaxValue(sTblPtr->counter);

		/* - start at offset=0
		 * - X expiry points = SA_LIST_CNT
		 * - Final offset.
		 */
		/** @req OS443 */
		/** @req OS408 */
		for(iter=0; iter  <  SA_LIST_CNT(&sTblPtr->expirePointList) ; iter++) {
			delta = SA_LIST_GET(&sTblPtr->expirePointList,iter)->offset - delta;
			assert( delta >=  minCycle );
			assert( delta <=  maxValue );
		}

		/* Final */
		/** @req OS444 */
		delta = sTblPtr->duration - SA_LIST_GET(&sTblPtr->expirePointList,iter)->offset;
		assert( delta >=  minCycle );
		assert( delta <=  maxValue );
	}

}

StatusType StartScheduleTableRel(ScheduleTableType sid, TickType offset) {
	StatusType rv = E_OK;
	OsSchTblType *s_tbl;
	TickType max_offset;


#if (OS_STATUS_EXTENDED == STD_ON )
	/** @req OS275 */
	SCHED_CHECK_ID(sid);
#endif

	s_tbl = Oil_GetSched(sid);

#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
	if( s_tbl->sync != NULL ) {
		/* EXPLICIT or IMPLICIT */

		/** @req OS452 */
		if( s_tbl->sync->syncStrategy == IMPLICIT ) {
			rv = E_OS_ID;
			goto err;
		}
	}
#endif

#if (OS_STATUS_EXTENDED == STD_ON )

	max_offset = Os_CounterGetMaxValue(s_tbl->counter);
	/** @req OS276 */
	/** @req OS332 */
	if( (offset == 0) || ((offset + Os_SchTblGetInitialOffset()) > max_offset ) ) {
		rv = E_OS_VALUE;
		goto err;
	}
#endif

	/** @req OS277 */
	if( s_tbl->state != SCHEDULETABLE_STOPPED ) {
		rv = E_OS_STATE;
		goto err;
	}

	Irq_Disable();
	/* calculate the expire value.. */
	/** @req OS278 */
	s_tbl->expire_val = Os_CounterCalcModulo( Os_CounterGetValue(s_tbl->counter), max_offset, offset );
	s_tbl->state = SCHEDULETABLE_RUNNING;
	Irq_Enable();

	SCHED_STD_END;
}

StatusType StartScheduleTableAbs(ScheduleTableType sid, TickType start ){
	StatusType rv = E_OK;
	OsSchTblType *sTblPtr;

	/** @req OS348 */
	SCHED_CHECK_ID(sid);

	/** @req OS349 */
	if( start > Os_CounterGetMaxValue(sTblPtr->counter) ) {
		rv = E_OS_VALUE;
		goto err;
	}

	/** @req OS350 */
	if( sTblPtr->state != SCHEDULETABLE_STOPPED ) {
		rv = E_OS_STATE;
		goto err;
	}


	Irq_Disable();
	/** @req OS351 */
	sTblPtr->expire_val = start + Os_SchTblGetInitialOffset(sTblPtr);
	sTblPtr->state = SCHEDULETABLE_RUNNING;
	Irq_Enable();

	SCHED_STD_END;
}

/**
 *
 * @param sid
 * @return
 */
#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )

StatusType StartScheduleTableSynchron(ScheduleTableType sid ){
	OsSchTblType *s_p;
	StatusType rv = E_OK;

	Irq_Disable();

	SCHED_CHECK_ID(sid);

	/** @req OS387 */
	if( s_p->sync.syncStrategy != EXPLICIT ) {
		rv = E_OS_ID;
		goto err;
	}

	/** @req OS388 */
	if( s_p->state != SCHEDULETABLE_STOPPED ) {
		rv = E_OS_STATE;
		goto err;
	}

	/** @req OS389 */
	s_p->state = SCHEDULETABLE_WAITING;

	Irq_Enable();

	SCHED_STD_END;
}
#endif



/** @req OS006 */
/* TODO: Implement StopScheduleTable */
StatusType StopScheduleTable(ScheduleTableType sid) {
	StatusType rv = E_OK;
	OsSchTblType *s_tbl;
	/** @req OS279 */
	SCHED_CHECK_ID(sid);
	s_tbl = Oil_GetSched(sid);

	/** @req OS280 */
	if(s_tbl->state == SCHEDULETABLE_STOPPED ) {
		rv = E_OS_NOFUNC;
		goto err;
	}

	/** @req OS281 */
	s_tbl->state = SCHEDULETABLE_STOPPED;

	SCHED_STD_END;
}

/** @req OS191 */
StatusType NextScheduleTable( ScheduleTableType sid_curr, ScheduleTableType sid_next) {
	StatusType rv = E_OK;
	(void)sid_curr;
	(void)sid_next;

	OsSchTblType *sFromPtr;
	OsSchTblType *sToPtr;

	/** @req OS282 */
	SCHED_CHECK_ID(sid_curr);
	SCHED_CHECK_ID(sid_next);

	sFromPtr = Oil_GetSched(sid_curr);
	sToPtr = Oil_GetSched(sid_curr);

	/** @req OS330 */
	if( sFromPtr->counter != sToPtr->counter) {
		rv = E_OS_ID;
		goto err;
	}

	/** @req OS283 */
	if( sFromPtr->state == SCHEDULETABLE_STOPPED ||
		sFromPtr->state == SCHEDULETABLE_NEXT )
	{
		rv = E_OS_NOFUNC;
		goto err;
	}

	/** @req OS309 */
	if( sToPtr->state != SCHEDULETABLE_STOPPED ) {
		rv = E_OS_STATE;
		goto err;
	}

	Irq_Disable();

	/** @req OS453 */
	if( sFromPtr->state == SCHEDULETABLE_STOPPED ) {
		sFromPtr->next->state = SCHEDULETABLE_STOPPED;
	} else {
		/** @req OS324 */
		if( sFromPtr->next != NULL ) {
			// Stop the schedule-table that was to be next.
			sFromPtr->next->state = SCHEDULETABLE_STOPPED;
		}

		sFromPtr->next = sToPtr;
		sToPtr->state = SCHEDULETABLE_NEXT;
		sToPtr->expire_curr_index = 0;
	}

	Irq_Enable();

	SCHED_STD_END;
}



/**
 *
 * @param sid
 * @param globalTime
 * @return
 */
#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
StatusType SyncScheduleTable( ScheduleTableType sid, GlobalTimeTickType globalTime  ) {
	StatusType rv = E_OK;
	OsSchTblType *s_p =  Oil_GetSched(sid);

	SCHED_CHECK_ID(sid);


	/** @req OS454 */
	if( s_p->sync.syncStrategy != EXPLICIT ) {
		rv =  E_OS_ID;
		goto err;
	}

	/** @req OS455 */
	if( globalTime > s_p->duration ) {
		rv = E_OS_VALUE;
		goto err;
	}

	Irq_Disable();

	/** @req OS456 */
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

	Irq_Enable();

	SCHED_STD_END;
}
#endif


/**
 *
 * @param sid
 * @param status
 * @return
 */

/** @req OS359 */
/** @req OS227 */
StatusType GetScheduleTableStatus( ScheduleTableType sid, ScheduleTableStatusRefType status ) {
	StatusType rv = E_OK;
	OsSchTblType *s_p;
	(void)status;
	/** @req OS293 */
	SCHED_CHECK_ID(sid);

	s_p = Oil_GetSched(sid);
	Irq_Disable();

	switch(s_p->state) {
		/** @req OS289 */
	case SCHEDULETABLE_STOPPED:
		/** @req OS353 */
	case SCHEDULETABLE_NEXT:
		/** @req OS290 */
	case SCHEDULETABLE_RUNNING_AND_SYNCHRONOUS:
		/** @req OS354 */
	case SCHEDULETABLE_WAITING:
		/** @req OS291 */
	case SCHEDULETABLE_RUNNING:
		*status = s_p->state;
		break;
	default:
		assert(0);

	}

	Irq_Enable();

	SCHED_STD_END;
}


/**
 *
 * @param sid
 * @return
 */
#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
StatusType SetScheduleTableAsync( ScheduleTableType sid ) {
	StatusType rv = E_OK;
	OsSchTblType *s_p = Oil_GetSched(sid);

	SCHED_CHECK_ID(sid);

	/** @req OS458 */
	if( s_p->sync.syncStrategy != EXPLICIT ) {
		rv = E_OS_ID;
		goto err;
	}

	Irq_Disable();

	/** @req_todo OS362 */
	/** @req_todo OS323 */

	/** @req OS300 */
	s_p->state = SCHEDULETABLE_RUNNING;

	Irq_Enable();

	SCHED_STD_END;
}
#endif



/**
 * Go through the schedule tables connected to this counter
 *
 * @param c_p Pointer to counter object
 */
void Os_SchTblCheck(OsCounterType *c_p) {
	/** @req OS002 */
	/** @req OS007 */

	OsSchTblType *sched_obj;

	/* Iterate through the schedule tables */
	SLIST_FOREACH(sched_obj,&c_p->sched_head,sched_list) {

		if( sched_obj->state == SCHEDULETABLE_STOPPED ) {
			continue;
		}

#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
		if( sched_obj->sync.syncStrategy == IMPLICIT ) {
			// ....

		} else {
			int adj;
			// Handle EXPLICIT
			if( sched_obj->sync.deviation > 0 ) {
				// The sync counter was set back ==
				// we have more time to complete the table
				adj = MIN(sched_obj->sync.deviation, getAdjExpPoint(sched_obj)->maxAdvance );
				sched_obj->sync.deviation -= adj;

			} else if( sched_obj->sync.deviation < 0 ) {
				// The sync counter was set forward ==
				// we have less time to complete the table
				adj = MIN((-sched_obj->sync.deviation), getAdjExpPoint(sched_obj)->maxRetard);
				sched_obj->sync.deviation -= adj;

			} else {
				// all is well
				sched_obj->state = SCHEDULETABLE_RUNNING_AND_SYNCHRONOUS;
			}
		}
#endif

		/* Check if the expire point have been hit */
		if( (sched_obj->state == SCHEDULETABLE_RUNNING ||
				SCHEDULETABLE_RUNNING_AND_SYNCHRONOUS ) &&
				(c_p->val >= sched_obj->expire_val) )
		{
			OsScheduleTableExpiryPointType * action;
			int i;

			action = SA_LIST_GET(&sched_obj->expirePointList,sched_obj->expire_curr_index);

			/** @req OS407 */
			/** @req OS412 */

			/* According to OS412 activate tasks before events */
			for(i=0; i< action->taskListCnt;i++ ) {
				ActivateTask(action->taskList[i]);
			}

			for(i=0; i< action->eventListCnt;i++ ) {
				SetEvent( action->eventList[i].task, action->eventList[i].event);
			}
			// Calc new expire val
			Os_SchTblCalcExpire(sched_obj);
		}

	}
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
static void os_stbl_action_calc_delta( OsSchTblType *stbl ) {
	OsScheduleTableExpiryPointType * first;
	OsScheduleTableExpiryPointType * second;
//	ALIST_DECL_ITER(iter);
	int iter;

	// calculate the delta to next action

	for(iter=1; iter <  SA_LIST_CNT(&stbl->expirePointList) ;iter++) {
		first = SA_LIST_GET(&stbl->expirePointList,iter-1);
		second = SA_LIST_GET(&stbl->expirePointList,iter);
		first->delta = second->offset - first->offset;
	}
	// calculate the last delta( to countes max value )
	first = SA_LIST_GET(&stbl->expirePointList, SA_LIST_CNT(&stbl->expirePointList)-1);
	first->delta = stbl->counter->alarm_base.maxallowedvalue - first->offset;
}

/**
 *
 */
void Os_SchTblInit( void ) {
	OsSchTblType *s_p;
	for( int i=0; i < Oil_GetSchedCnt();i++ ) {
		s_p = Oil_GetSched(i);
		os_stbl_action_calc_delta(s_p);

		ScheduleTableConsistenyCheck(s_p);
	}
}

/**
 * Calculates expire value and changes state depending it's state.
 *
 * Note!
 * We can't cheat with the final + initial expire-point, instead we
 * must setup trigger after the final delay and set the "previous"
 * table to SCHEDULETABLE_STOPPED and the new to SCHEDULETABLE_RUNNING.
 *
 * @param stbl Ptr to a Schedule Table.
 */
void Os_SchTblCalcExpire( OsSchTblType *stbl ) {

	TickType delta;
	TickType initalOffset;
	TickType finalOffset;
	OsSchTblType *nextStblPtr;


	if( (stbl->expire_curr_index) == SA_LIST_CNT(&stbl->expirePointList) ) {
		/* We are at the last expiry point */
		finalOffset = Os_SchTblGetFinalOffset(stbl);

		stbl->expire_val =	Os_CounterCalcModulo(
						Os_CounterGetValue(stbl->counter),
						Os_CounterGetMaxValue(stbl->counter),
						finalOffset );

		stbl->expire_curr_index++;

	} else if( (stbl->expire_curr_index) > SA_LIST_CNT(&stbl->expirePointList) ) {
		/* At final offset */

		/** @req OS194 */
		if( (stbl->repeating == REPEATING) || (stbl->next != NULL) ) {
			if( stbl->next != NULL ) {
				/** @req OS284 */
				nextStblPtr = stbl->next;
				/* NextScheduleTable() have been called */
				assert( nextStblPtr->state==SCHEDULETABLE_NEXT );

				/* We don't care about REPEATING or SINGLE_SHOT here */
				initalOffset = Os_SchTblGetInitialOffset(nextStblPtr);
				stbl->state = SCHEDULETABLE_STOPPED;
				nextStblPtr->state = SCHEDULETABLE_RUNNING;
			} else {
				/** @req OS414 */

				/* REPEATING */
				assert( stbl->repeating == REPEATING );
				initalOffset = Os_SchTblGetInitialOffset(stbl);
			}

			stbl->expire_val =	Os_CounterCalcModulo(
							Os_CounterGetValue(stbl->counter),
							Os_CounterGetMaxValue(stbl->counter),
							initalOffset );

		} else {
			assert( stbl->repeating == SINGLE_SHOT );
			/** @req OS009 */
			stbl->state = SCHEDULETABLE_STOPPED;
			stbl->expire_curr_index = 0;
		}
	} else {

		delta = SA_LIST_GET(&stbl->expirePointList,stbl->expire_curr_index)->delta;

		stbl->expire_val =	Os_CounterCalcModulo(
						Os_CounterGetValue(stbl->counter),
						Os_CounterGetMaxValue(stbl->counter),
						delta );

		stbl->expire_curr_index++;

	}

	return;
}


