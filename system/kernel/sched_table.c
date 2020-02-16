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

/*
 * Generic requirements this module can handle
 */
 /** @req OS007 */
 /** @req OS410 */
 /** @req OS411 */
 /** @req OS347 */
 /** @req OS358 */
 /** @req OS428 */


/* ----------------------------[includes]------------------------------------*/

#include "Os.h"
#include "sched_table_i.h"
#include "sys.h"
#include "alist_i.h"
#include "application.h"


/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/

#define SCHED_CHECK_ID(x) 		\
	if( (x) > OS_SCHTBL_CNT) { 	\
		rv = E_OS_ID;			\
		goto err; 				\
	}

#define SCHED_STD_END 	\
		return rv;		\
	err:				\
		ERRORHOOK(rv);  \
		return rv;

/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
/* ----------------------------[private functions]---------------------------*/


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
static void Os_SchTblUpdateState( OsSchTblType *stbl ) {

	TickType delta;
	TickType initalOffset;
	TickType finalOffset;
	OsSchTblType *nextStblPtr;
	_Bool handleLast = 0;

	if( (stbl->expire_curr_index) == (SA_LIST_CNT(&stbl->expirePointList) - 1) ) {
		/* We are at the last expiry point */
		finalOffset = Os_SchTblGetFinalOffset(stbl);

		if (finalOffset != 0) {
			stbl->expire_val =	Os_CounterAdd(
							Os_CounterGetValue(stbl->counter),
							Os_CounterGetMaxValue(stbl->counter),
							finalOffset );

			stbl->expire_curr_index++;
			return;
		} else {
			/* Only single shot may have an offset of 0 */
			assert(stbl->repeating == SINGLE_SHOT);
			handleLast = 1;
		}
	}

	if( handleLast ||
		( (stbl->expire_curr_index) == SA_LIST_CNT(&stbl->expirePointList) ) )
	{
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

			stbl->expire_val =	Os_CounterAdd(
							Os_CounterGetValue(stbl->counter),
							Os_CounterGetMaxValue(stbl->counter),
							initalOffset );

		} else {
			assert( stbl->repeating == SINGLE_SHOT );
			/** @req OS009 */
			stbl->state = SCHEDULETABLE_STOPPED;
		}
		stbl->expire_curr_index = 0;

	} else {

		delta = SA_LIST_GET(&stbl->expirePointList,stbl->expire_curr_index+1)->offset -
				SA_LIST_GET(&stbl->expirePointList,stbl->expire_curr_index)->offset ;

		stbl->expire_val =	Os_CounterAdd(
						Os_CounterGetValue(stbl->counter),
						Os_CounterGetMaxValue(stbl->counter),
						delta );

		stbl->expire_curr_index++;

	}

	return;
}


/* ----------------------------[public functions]----------------------------*/


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
			/* initial offset may be zero (OS443) */
			if(iter!=0) {
				assert( delta >=  minCycle );
			}
			assert( delta <=  maxValue );
		}

		/* Final */
		/** @req OS444 */
		delta = sTblPtr->duration - SA_LIST_GET(&sTblPtr->expirePointList,iter-1)->offset;
		assert( delta >=  minCycle );
		assert( delta <=  maxValue );
	}

}

StatusType StartScheduleTableRel(ScheduleTableType sid, TickType offset) {
	StatusType rv = E_OK;
	OsSchTblType *sPtr;
	TickType max_offset;
	imask_t state;


#if (OS_STATUS_EXTENDED == STD_ON )
	/** @req OS275 */
	SCHED_CHECK_ID(sid);
#endif

	sPtr = Os_SchTblGet(sid);

#if	(OS_APPLICATION_CNT > 1)

	rv = Os_ApplHaveAccess( sPtr->accessingApplMask );
	if( rv != E_OK ) {
		goto err;
	}

#endif

#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
	if( sPtr->sync != NULL ) {
		/* EXPLICIT or IMPLICIT */

		/** @req OS452 */
		if( sPtr->sync->syncStrategy == IMPLICIT ) {
			rv = E_OS_ID;
			goto err;
		}
	}
#endif

	max_offset = Os_CounterGetMaxValue(sPtr->counter);
#if (OS_STATUS_EXTENDED == STD_ON )
	/** @req OS276 */
	/** @req OS332 */
	if( (offset == 0) || ((offset + Os_SchTblGetInitialOffset(sPtr)) > max_offset ) ) {
		rv = E_OS_VALUE;
		goto err;
	}
#endif

	/** @req OS277 */
	if( sPtr->state != SCHEDULETABLE_STOPPED ) {
		rv = E_OS_STATE;
		goto err;
	}

	Irq_Save(state);
	/* calculate the expire value.. */
	/** @req OS278 */
	sPtr->expire_val = Os_CounterAdd(
							Os_CounterGetValue(sPtr->counter),
							max_offset,
							offset + Os_SchTblGetInitialOffset(sPtr) );
	sPtr->state = SCHEDULETABLE_RUNNING;
	Irq_Restore(state);

	SCHED_STD_END;
}

StatusType StartScheduleTableAbs(ScheduleTableType sid, TickType start ){
	StatusType rv = E_OK;
	OsSchTblType *sTblPtr;
	imask_t state;

	/** @req OS348 */
	SCHED_CHECK_ID(sid);
	sTblPtr =  Os_SchTblGet(sid);

#if	(OS_APPLICATION_CNT > 1)

	rv = Os_ApplHaveAccess( sTblPtr->accessingApplMask );
	if( rv != E_OK ) {
		goto err;
	}

#endif

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


	Irq_Save(state);
	/** @req OS351 */
	sTblPtr->expire_val = start + Os_SchTblGetInitialOffset(sTblPtr);
	sTblPtr->state = SCHEDULETABLE_RUNNING;
	Irq_Restore(state);

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
	imask_t state;

	Irq_Save(state);

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

	Irq_Restore(state);

	SCHED_STD_END;
}
#endif



/** @req OS006 */
/* TODO: Implement StopScheduleTable */
StatusType StopScheduleTable(ScheduleTableType sid) {
	StatusType rv = E_OK;
	OsSchTblType *sPtr;
	/** @req OS279 */
	SCHED_CHECK_ID(sid);
	sPtr = Os_SchTblGet(sid);

	/** @req OS280 */
	if(sPtr->state == SCHEDULETABLE_STOPPED ) {
		rv = E_OS_NOFUNC;
		goto err;
	}

	/** @req OS281 */
	sPtr->state = SCHEDULETABLE_STOPPED;

	SCHED_STD_END;
}

/** @req OS191 */
StatusType NextScheduleTable( ScheduleTableType sid_curr, ScheduleTableType sid_next) {
	StatusType rv = E_OK;
	(void)sid_curr;
	(void)sid_next;

	OsSchTblType *sFromPtr;
	OsSchTblType *sToPtr;

	imask_t state;

	/** @req OS282 */
	SCHED_CHECK_ID(sid_curr);
	SCHED_CHECK_ID(sid_next);

	sFromPtr = Os_SchTblGet(sid_curr);
	sToPtr = Os_SchTblGet(sid_curr);

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

	Irq_Save(state);

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

	Irq_Restore(state);

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
	OsSchTblType *s_p =  Os_SchTblGet(sid);
	imask_t state;

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

	Irq_Save(state);

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

	Irq_Restore(state);

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
	imask_t state;

	/** @req OS293 */
	SCHED_CHECK_ID(sid);

	s_p = Os_SchTblGet(sid);
	Irq_Save(state);

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

	Irq_Restore(state);

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
	OsSchTblType *s_p = Os_SchTblGet(sid);
	imask_t state;

	SCHED_CHECK_ID(sid);

	/** @req OS458 */
	if( s_p->sync.syncStrategy != EXPLICIT ) {
		rv = E_OS_ID;
		goto err;
	}

	Irq_Save(state);

	/** @req_todo OS362 */
	/** @req_todo OS323 */

	/** @req OS300 */
	s_p->state = SCHEDULETABLE_RUNNING;

	Irq_Restore(state);

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
			sched_obj->state == SCHEDULETABLE_RUNNING_AND_SYNCHRONOUS ) &&
			(c_p->val == sched_obj->expire_val) )
		{
			if ( sched_obj->expire_curr_index < SA_LIST_CNT(&sched_obj->expirePointList) ) {
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
			}
			// Calc new expire val and state
			Os_SchTblUpdateState(sched_obj);
		}

	}
}

/**
 *
 */
void Os_SchTblInit( void ) {
	OsSchTblType *s_p;
	for( int i=0; i < OS_SCHTBL_CNT;i++ ) {
		s_p = Os_SchTblGet(i);

		ScheduleTableConsistenyCheck(s_p);
	}
}

void Os_SchTblAutostart( void ) {

	for(int j=0; j < OS_SCHTBL_CNT; j++ ) {
		OsSchTblType *sPtr;
		sPtr = Os_SchTblGet(j);

		if( sPtr->autostartPtr != NULL ) {
			const struct OsSchTblAutostart *autoPtr = sPtr->autostartPtr;

			/* Check appmode */
			if( OS_SYS_PTR->appMode & autoPtr->appMode ) {

				/* Start the schedule table */
				switch(autoPtr->type) {
				case SCHTBL_AUTOSTART_ABSOLUTE:
					StartScheduleTableAbs(j,autoPtr->offset);
					break;
				case SCHTBL_AUTOSTART_RELATIVE:
					StartScheduleTableRel(j,autoPtr->offset);
					break;
	#if defined(OS_SC2) || defined(OS_SC4)
				case SCHTBL_AUTOSTART_SYNCHRONE:
					/* TODO: */
					break;
	#endif
				default:
					assert(0); 		// Illegal value
					break;
				}
			}
		}
	}
}


