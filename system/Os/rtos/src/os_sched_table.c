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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.3.0 */

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
 /** @req SWS_Os_00007  OS shall permit multiple schdeule table */
 /** @req SWS_Os_00410  At least one schedule table per counter */
 /** @req SWS_Os_00411  One tick on counter corresponds to one tick on schedule table */
 /** @req SWS_Os_00428  If schedule table processing has been cancelled before reaching the
  * 					Final Expiry Point and is subsequently restarted then means that the
  * 					re-start occurs from the start of the schedule table. */


/* ----------------------------[includes]------------------------------------*/

#include "os_i.h"


/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/

/* Initial check is to silence some compilers when OS_SCHTBL_CNT == 0 */
#define SCHED_CHECK_ID(x)       ( (OS_SCHTBL_CNT != 0UL) &&  ((x) < OS_SCHTBL_CNT) )

/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
#if (OS_SCHTBL_CNT != 0)
StatusType Os_StartScheduleTableRelStartup(ScheduleTableType sid, TickType offset);
StatusType Os_StartScheduleTableAbsStartup(ScheduleTableType sid, TickType start );
#endif
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
    _Bool handleLast = 0; /*lint !e970 MISRA:OTHER:type _Bool declared outside:[MISRA 2012 Directive 4.6, advisory] */

    if( (stbl->expire_curr_index) == (SA_LIST_CNT(&stbl->expirePointList) - 1) ) {
        /* We are at the last expiry point */
        finalOffset = Os_SchTblGetFinalOffset(stbl);

        if (finalOffset != 0) {
            stbl->expire_val =	Os_CounterAdd(
                            Os_CounterGetValue(stbl->counter),
                            Os_CounterGetMaxValue(stbl->counter),
                            finalOffset );

            stbl->expire_curr_index++;
            return;/*lint !e904 MISRA:OTHER:Return statement is necessary in case of reporting a DET error:[MISRA 2012 Rule 15.5, advisory]*/
        } else {
            /* Only single shot may have an offset of 0 */
            ASSERT(stbl->repeating == SINGLE_SHOT);
            handleLast = 1;
        }
    }

    if( handleLast ||
        ( (stbl->expire_curr_index) == SA_LIST_CNT(&stbl->expirePointList) ) )
    {
        /* At final offset */
        /** @req SWS_Os_00194 */
        if( (stbl->repeating == REPEATING) || (stbl->next != NULL) ) {
            if( stbl->next != NULL ) {
                /** @req SWS_Os_00284 */
                nextStblPtr = stbl->next;
                /* NextScheduleTable() have been called */
                ASSERT( nextStblPtr->state==SCHEDULETABLE_NEXT );

                /* We don't care about REPEATING or SINGLE_SHOT here */
                initalOffset = Os_SchTblGetInitialOffset(nextStblPtr);
                stbl->state = SCHEDULETABLE_STOPPED;
                nextStblPtr->state = SCHEDULETABLE_RUNNING;

                nextStblPtr->expire_val =	Os_CounterAdd(
                                Os_CounterGetValue(nextStblPtr->counter),
                                Os_CounterGetMaxValue(nextStblPtr->counter),
                                initalOffset );

            } else {
                /** @req SWS_Os_00414 */

                /* REPEATING */
                ASSERT( stbl->repeating == REPEATING );
                initalOffset = Os_SchTblGetInitialOffset(stbl);

                stbl->expire_val =	Os_CounterAdd(
                                Os_CounterGetValue(stbl->counter),
                                Os_CounterGetMaxValue(stbl->counter),
                                initalOffset );
            }


        } else {
            ASSERT( stbl->repeating == SINGLE_SHOT );
            /** @req SWS_Os_00009 */
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
#if (OS_SCHTBL_CNT != 0)
/*lint -e818 MISRA:OTHER:sTblPtr is not pointing to constant:[MISRA 2012 Rule 8.13, advisory] */
static void ScheduleTableConsistenyCheck( OsSchTblType *sTblPtr ){

#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
    /** @req SWS_Os_00440 */
    if( sTblPtr->sync.syncStrategy == IMPLICIT ) {
        ASSERT( sTblPtr->duration == (sTblPtr->counter->alarm_base.maxallowedvalue +1) );
    }

    /** OS431 */
    if( sTblPtr->sync.syncStrategy == EXPLICIT ) {
        ASSERT( sTblPtr->duration <= (sTblPtr->counter->alarm_base.maxallowedvalue +1) );
    }
#endif

    /** @req SWS_Os_00401 */
    ASSERT(SA_LIST_CNT(&sTblPtr->expirePointList)>=1);



    {
        int iter;/*lint !e970 MISRA:OTHER:iterator variable :[MISRA 2012 Directive 4.6, advisory] */
        TickType delta = 0;
        TickType minCycle = Os_CounterGetMinCycle(sTblPtr->counter);
        TickType maxValue =  Os_CounterGetMaxValue(sTblPtr->counter);

        /* - start at offset=0
         * - X expiry points = SA_LIST_CNT
         * - Final offset.
         */
        /** @req SWS_Os_00443 */
        /** @req SWS_Os_00408 */
        for(iter=0; iter  <  SA_LIST_CNT(&sTblPtr->expirePointList) ; iter++) {
            delta = SA_LIST_GET(&sTblPtr->expirePointList,iter)->offset - delta;
            /* initial offset may be zero (OS443) */
            if(iter!=0) {
                ASSERT( delta >=  minCycle );
            }
            ASSERT( delta <=  maxValue );
        }

        /* Final */
        /** @req SWS_Os_00444 */ /** !req SWS_Os_00427 */
        delta = sTblPtr->duration - SA_LIST_GET(&sTblPtr->expirePointList,iter-1)->offset;
        ASSERT( delta >=  minCycle );
        ASSERT( delta <=  maxValue );
        // if assert is defined not to access the argument(s) lint warnings about the arguments not used
        //lint -save -e438 -e529
    }
    //lint -restore  Restore the inhibit messages within block above

}
#endif

/** @req SWS_Os_00347 StartScheduleTableRel API */
/** @req SWS_Os_00521 StartScheduleTableRel available in all Scalability Classes. */
StatusType StartScheduleTableRel(ScheduleTableType sid, TickType offset) {
    StatusType rv = E_OK;
    OsSchTblType *sPtr;
    TickType max_offset;
    imask_t state;

    /* Validation of parameters, if failure, function will return */
    /* OS_STD_ERR_2: Function will return after calling ErrorHook */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_2((Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT,
    		          OSServiceId_StartScheduleTableRel,sid, offset);   /* @req SWS_Os_00093 */

/** @req SWS_BSW_00029 */
#if (OS_STATUS_EXTENDED == STD_ON )
    /** @req SWS_Os_00275 */
    /*lint -e685 MISRA:CONFIGURATION:argument check:[MISRA 2012 Rule 14.3, required] */
    /*lint -e568 MISRA:CONFIGURATION:argument check:[MISRA 2004 Info, advisory] */
    OS_VALIDATE_STD_2(SCHED_CHECK_ID(sid) , E_OS_ID,
    		          OSServiceId_StartScheduleTableRel,sid, offset);
#endif

    sPtr = Os_SchTblGet(sid);

#if	(OS_APPLICATION_CNT > 1)

    rv = Os_ApplHaveAccess( sPtr->accessingApplMask );
    if( rv != E_OK ) {
    	OS_STD_ERR_2(OSServiceId_StartScheduleTableRel,sid, offset);
    }

#endif

#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
    if( sPtr->sync != NULL ) {
        /* EXPLICIT or IMPLICIT */

        /** OS452 */ /** OS430 */
    	 OS_VALIDATE_STD_2((sPtr->sync->syncStrategy != IMPLICIT) , E_OS_ID,
    	    		          OSServiceId_StartScheduleTableRel,sid, offset);
    }
#endif

    max_offset = Os_CounterGetMaxValue(sPtr->counter);
	
/** @req SWS_BSW_00029 */
#if (OS_STATUS_EXTENDED == STD_ON )
    /** @req SWS_Os_00276 */
    /** @req SWS_Os_00332 */
    /*lint -e{9007} MISRA:FALSE_POSITIVE:No side effects of Os_SchTblGetInitialOffset:[MISRA 2012 Rule 13.5, required]*/
    if( (offset == 0) || ((offset + Os_SchTblGetInitialOffset(sPtr)) > max_offset ) ) {
        rv = E_OS_VALUE;
        OS_STD_ERR_2(OSServiceId_StartScheduleTableRel,sid, offset);
    }
#endif

    /** @req SWS_Os_00277 */
    /*lint -e539 MISRA:CONFIGURATION:argument check:[MISRA 2004 Info, advisory] */
    OS_VALIDATE_STD_2((sPtr->state == SCHEDULETABLE_STOPPED) , E_OS_STATE,
	    		      OSServiceId_StartScheduleTableRel,sid, offset);
    Irq_Save(state);
    /* calculate the expire value.. */
    /** @req SWS_Os_00278 */
    sPtr->expire_val = Os_CounterAdd(
                            Os_CounterGetValue(sPtr->counter),
                            max_offset,
                            offset + Os_SchTblGetInitialOffset(sPtr) );
    sPtr->state = SCHEDULETABLE_RUNNING;
    Irq_Restore(state);

    return rv;
}

/** @req SWS_Os_00358 StartScheduleTableAbs API */
/** @req SWS_Os_00522 StartScheduleTableAbs available in all Scalability Classes. */
StatusType StartScheduleTableAbs(ScheduleTableType sid, TickType start ){
    StatusType rv = E_OK;
    OsSchTblType *sTblPtr;
    imask_t state;

    /* Validation of parameters, if failure, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_2((Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT,
    		          OSServiceId_StartScheduleTableAbs,sid, start);   /* @req SWS_Os_00093 */
    /** @req SWS_Os_00348 */
    OS_VALIDATE_STD_2(SCHED_CHECK_ID(sid) , E_OS_ID,
    		          OSServiceId_StartScheduleTableAbs,sid, start);

    sTblPtr =  Os_SchTblGet(sid);

#if	(OS_APPLICATION_CNT > 1)

    rv = Os_ApplHaveAccess( sTblPtr->accessingApplMask );
    if( rv != E_OK ) {
        /* OS_STD_ERR_2: Function will return after calling ErrorHook */
        /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    	OS_STD_ERR_2(OSServiceId_StartScheduleTableAbs,sid, start);
    }

#endif

    /** @req SWS_Os_00349 */
    OS_VALIDATE_STD_2((start <= Os_CounterGetMaxValue(sTblPtr->counter)) , E_OS_VALUE,
    		          OSServiceId_StartScheduleTableAbs,sid, start);

    /** @req SWS_Os_00350 */
    OS_VALIDATE_STD_2((sTblPtr->state == SCHEDULETABLE_STOPPED) , E_OS_STATE,
    		          OSServiceId_StartScheduleTableAbs,sid, start);

    Irq_Save(state);
    /** !req SWS_Os_00351 (SCHEDULETABLE_RUNNING_AND_SYNCHRONOUS not supported) */
    sTblPtr->expire_val = start + Os_SchTblGetInitialOffset(sTblPtr);
    sTblPtr->state = SCHEDULETABLE_RUNNING;
    Irq_Restore(state);

    return rv;
}

/**
 *
 * @param sid
 * @return
 */
/** OS201 OS provides the service StartScheduleTableSynchron() */
/** OS525 StartScheduleTableSynchron() available in Scalability Classes 2 and 4. */
#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
StatusType StartScheduleTableSynchron(ScheduleTableType sid ){
    OsSchTblType *s_p;
    StatusType rv = E_OK;
    imask_t state;

    /* Validation of parameters, if failure, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_1((Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT,
    		          OSServiceId_StartScheduleTableSynchron,sid);   /* @req SWS_Os_00093 */

    Irq_Save(state);
    OS_VALIDATE_STD_1(SCHED_CHECK_ID(sid) , E_OS_ID,
    		          OSServiceId_StartScheduleTableSynchron,sid);

    /** OS387 */
    OS_VALIDATE_STD_1(( s_p->sync.syncStrategy == EXPLICIT ) , E_OS_ID,
    		          OSServiceId_StartScheduleTableSynchron,sid);
    /** OS388 */
    OS_VALIDATE_STD_1(( s_p->state == SCHEDULETABLE_STOPPED ) , E_OS_STATE,
    		          OSServiceId_StartScheduleTableSynchron,sid);

    /** OS389 */ /** OS435 */
    s_p->state = SCHEDULETABLE_WAITING;

    Irq_Restore(state);

    return rv;
}
#endif



/** @req SWS_Os_00006 */
/** @req SWS_Os_00523 StopScheduleTable available in all Scalability Classes. */
/* IMPROVEMENT: Implement StopScheduleTable */
StatusType StopScheduleTable(ScheduleTableType sid) {
    StatusType rv = E_OK;
    OsSchTblType *sPtr;

    /* Validation of parameters, if failure, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_1( (Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT,
    		           OSServiceId_StopScheduleTable,sid);   /* @req SWS_Os_00093 */
    /** @req SWS_Os_00279 */
    OS_VALIDATE_STD_1( SCHED_CHECK_ID(sid) , E_OS_ID,
    		           OSServiceId_StopScheduleTable,sid);
    sPtr = Os_SchTblGet(sid);

    /** @req SWS_Os_00280 */
    OS_VALIDATE_STD_1( (sPtr->state != SCHEDULETABLE_STOPPED), E_OS_NOFUNC,
    		           OSServiceId_StopScheduleTable,sid);

    /** @req SWS_Os_00281 */
    sPtr->state = SCHEDULETABLE_STOPPED;

    return rv;
}

/** @req SWS_Os_00191 */
/** @req SWS_Os_00524 NextScheduleTable available in all Scalability Classes. */
StatusType NextScheduleTable( ScheduleTableType sid_curr, ScheduleTableType sid_next) {
    StatusType rv = E_OK;
    (void)sid_curr;
    (void)sid_next;

    OsSchTblType *sFromPtr;
    OsSchTblType *sToPtr;

    imask_t state;

    /* Validation of parameters, if failure, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_2( (Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT,
    		           OSServiceId_NextScheduleTable,sid_curr, sid_next);   /* @req SWS_Os_00093 */

    /** @req SWS_Os_00282 */
    OS_VALIDATE_STD_2(SCHED_CHECK_ID(sid_curr) , E_OS_ID,
    		          OSServiceId_NextScheduleTable,sid_curr, sid_next);
    OS_VALIDATE_STD_2(SCHED_CHECK_ID(sid_next) , E_OS_ID,
    		          OSServiceId_NextScheduleTable,sid_curr, sid_next);

    sFromPtr = Os_SchTblGet(sid_curr);
    sToPtr = Os_SchTblGet(sid_next);

    /** @req SWS_Os_00330 */
    OS_VALIDATE_STD_2( (sFromPtr->counter == sToPtr->counter), E_OS_ID ,
    		           OSServiceId_NextScheduleTable,sid_curr, sid_next);


    /** @req SWS_Os_00283 */
    if( (sFromPtr->state == SCHEDULETABLE_STOPPED) ||
        (sFromPtr->state == SCHEDULETABLE_NEXT) )
    {
        rv = E_OS_NOFUNC;
        /* OS_STD_ERR_2: Function will return after calling ErrorHook */
        /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
        OS_STD_ERR_2(OSServiceId_NextScheduleTable,sid_curr, sid_next);
    }

    /** @req SWS_Os_00309 */
    OS_VALIDATE_STD_2( (sToPtr->state == SCHEDULETABLE_STOPPED ), E_OS_STATE,
    		           OSServiceId_NextScheduleTable,sid_curr, sid_next);

    Irq_Save(state);

    /** @req SWS_Os_00453 */
    if( sFromPtr->state == SCHEDULETABLE_STOPPED ) {
        sFromPtr->next->state = SCHEDULETABLE_STOPPED;
    } else {
        /** @req SWS_Os_00324 */
        if( sFromPtr->next != NULL ) {
            // Stop the schedule-table that was to be next.
            sFromPtr->next->state = SCHEDULETABLE_STOPPED;
        }

        sFromPtr->next = sToPtr;
        sToPtr->state = SCHEDULETABLE_NEXT;
        sToPtr->expire_curr_index = 0;
    }

    Irq_Restore(state);

    return rv;
}



/**
 *
 * @param sid
 * @param globalTime
 * @return
 */
/** OS199 OS provides the service SyncScheduleTable() */
/** OS526 SyncScheduleTable() available in Scalability Classes 2 and 4. */
#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
StatusType SyncScheduleTable( ScheduleTableType ScheduleTableID, TickType Value  ) {
    StatusType rv = E_OK;
    OsSchTblType *s_p;
    imask_t state;
    /* Validation of parameters, if failure, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_2( (Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT,
    		           OSServiceId_SyncScheduleTable,ScheduleTableID, Value);   /* @req SWS_Os_00093 */
    OS_VALIDATE_STD_2(SCHED_CHECK_ID(ScheduleTableID) , E_OS_ID,
    		          OSServiceId_SyncScheduleTable,ScheduleTableID, Value);

    s_p = Os_SchTblGet(ScheduleTableID);

    /** OS454 */
    OS_VALIDATE_STD_2( ( s_p->sync.syncStrategy == EXPLICIT ) , E_OS_ID,
    		           OSServiceId_SyncScheduleTable,ScheduleTableID, Value);

    /** OS455 */
    OS_VALIDATE_STD_2( ( Value <= s_p->duration ) , E_OS_VALUE,
    		           OSServiceId_SyncScheduleTable,ScheduleTableID, Value);

    Irq_Save(state);

    /** OS456 */
    if( (s_p->state == SCHEDULETABLE_STOPPED) ||
        (s_p->state == SCHEDULETABLE_NEXT)	) {
        rv = E_OS_STATE;
        /* OS_STD_ERR_2: Function will return after calling ErrorHook */
        /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
        OS_STD_ERR_2(OSServiceId_SyncScheduleTable,ScheduleTableID, Value);
    }

    switch(s_p->state) {
    case  SCHEDULETABLE_WAITING:
        // First time we called since started. Set the sync counter to
        // the value provided.
        s_p->sync.syncCounter = Value;
        s_p->state = SCHEDULETABLE_RUNNING_AND_SYNCHRONOUS;
        break;

    case SCHEDULETABLE_RUNNING:
    case SCHEDULETABLE_RUNNING_AND_SYNCHRONOUS:
        s_p->sync.deviation = s_p->sync.syncCounter - Value;
        if( s_p->sync.deviation != 0 ) {
            // We are not at sync any more...
            /** OS419 */
            s_p->state = SCHEDULETABLE_RUNNING;
        }
        break;

    default:
        ASSERT(0);
        break;
    }

    Irq_Restore(state);

    return rv;
}
#endif


/**
 *
 * @param sid
 * @param status
 * @return
 */

/** @req SWS_Os_00528 GetScheduleTableStatus() available in all Scalability Classes. */
/** @req SWS_Os_00227 */
StatusType GetScheduleTableStatus( ScheduleTableType sid, ScheduleTableStatusRefType status ) {
    StatusType rv = E_OK;
    /*lint -e954 MISRA:OTHER:need not be const pointer:[MISRA 2012 Rule 8.13, advisory] */
    OsSchTblType *s_p;
    (void)status; /*lint !e920 MISRA:FALSE_POSITIVE:Allowed to cast pointer to void here:[MISRA 2012 Rule 1.3, required]*/
    imask_t state;

    /* Validation of parameters, if failure, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_2( (Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT ,
    		           OSServiceId_GetScheduleTableStatus,sid, status);   /* @req SWS_Os_00093 */
    /** @req SWS_Os_00293 */
    OS_VALIDATE_STD_2(SCHED_CHECK_ID(sid) , E_OS_ID,
    		          OSServiceId_GetScheduleTableStatus,sid, status);

    s_p = Os_SchTblGet(sid);
    Irq_Save(state);

    switch(s_p->state) {
        /** @req SWS_Os_00289 */
    case SCHEDULETABLE_STOPPED:
        /** @req SWS_Os_00353 */
    case SCHEDULETABLE_NEXT:
        /** OS290 */
    case SCHEDULETABLE_RUNNING_AND_SYNCHRONOUS:
        /** OS354 */
    case SCHEDULETABLE_WAITING:
        /** OS291 */
    case SCHEDULETABLE_RUNNING:
        *status = s_p->state;
        break;
    default:
        ASSERT(0);

    }

    Irq_Restore(state);

    return rv;
}


/**
 *
 * @param sid
 * @return
 */
/** OS422 OS provides the service SetScheduleTableAsync() */
/** OS527 SetScheduleTableAsync() available in Scalability Classes 2 and 4. */
#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
StatusType SetScheduleTableAsync( ScheduleTableType sid ) {
    StatusType rv = E_OK;
    OsSchTblType *s_p;
    imask_t state;

    /* Validation of parameters, if failure, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_1( (Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT ,
    		           OSServiceId_SetScheduleTableAsync,sid );   /* @req SWS_Os_00093 */

    OS_VALIDATE_STD_1(SCHED_CHECK_ID(sid) , E_OS_ID,
    		          OSServiceId_SetScheduleTableAsync,sid);
    s_p = Os_SchTblGet(sid);

    /** OS458 */
    OS_VALIDATE_STD_1( ( s_p->sync.syncStrategy == EXPLICIT ) , E_OS_ID ,
    		           OSServiceId_SetScheduleTableAsync,sid );

    Irq_Save(state);

    /** !req SWS_Os_00362 */ /** !req SWS_Os_00323 */ /** !req SWS_Os_00483 */

    /** OS300 */
    s_p->state = SCHEDULETABLE_RUNNING;

    Irq_Restore(state);

    return rv;
}
#endif



/**
 * Go through the schedule tables connected to this counter
 *
 * @param c_p Pointer to counter object
 */
void Os_SchTblCheck(OsCounterType *c_p) {
    /** @req SWS_Os_00002 */
    /** @req SWS_Os_00007 */

    OsSchTblType *sched_obj;

    /* Iterate through the schedule tables */
    /*lint -e9036 -e818 MISRA:PERFORMANCE:this is a continuous loop:[MISRA 2012 Rule 14.4, required] */
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
        if( ((sched_obj->state == SCHEDULETABLE_RUNNING) ||
            (sched_obj->state == SCHEDULETABLE_RUNNING_AND_SYNCHRONOUS) ) &&
            (c_p->val == sched_obj->expire_val) )
        {
            if ( sched_obj->expire_curr_index < SA_LIST_CNT(&sched_obj->expirePointList) ) {
                OsScheduleTableExpiryPointType * action;
                uint32_t i;

                action = SA_LIST_GET(&sched_obj->expirePointList,sched_obj->expire_curr_index);

                /** @req SWS_Os_00407 */
                /** @req SWS_Os_00412 */

                /* According to OS412 activate tasks before events */
                for(i=0; i< (uint32_t)action->taskListCnt;i++ ) {
                    (void)ActivateTask(action->taskList[i]);
                }

                for(i=0; i< (uint32_t)action->eventListCnt;i++ ) {
                    (void)SetEvent( action->eventList[i].task, action->eventList[i].event);
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
#if (OS_SCHTBL_CNT != 0)
    OsSchTblType *s_p;
    /*lint -e{681, 685, 568} MISRA:CONFIGURATION:Allow MISRA violations depending on configuration:[MISRA 2004 Info,advisory]*/

    for( ScheduleTableType i=0; i < OS_SCHTBL_CNT;i++ ) {
        s_p = Os_SchTblGet(i);

        ScheduleTableConsistenyCheck(s_p);
    }
#endif
}

void Os_SchTblAutostart( void ) {
#if (OS_SCHTBL_CNT != 0)
    /*lint -e{681, 685, 568} MISRA:CONFIGURATION:Allow MISRA violations depending on configuration:[MISRA 2004 Info,advisory]*/

    for(ScheduleTableType j=0; j < OS_SCHTBL_CNT; j++ ) {
        OsSchTblType *sPtr;
        sPtr = Os_SchTblGet(j);

        if( sPtr->autostartPtr != NULL ) {
            const struct OsSchTblAutostart *autoPtr = sPtr->autostartPtr;

            /* Check appmode */
            if( OS_SYS_PTR->appMode & autoPtr->appMode ) {

                /* Start the schedule table */
                switch(autoPtr->type) {
                case SCHTBL_AUTOSTART_ABSOLUTE:
                    (void)Os_StartScheduleTableAbsStartup(j,autoPtr->offset);
                    break;
                case SCHTBL_AUTOSTART_RELATIVE:
                    (void)Os_StartScheduleTableRelStartup(j,autoPtr->offset);
                    break;
    #if defined(OS_SC2) || defined(OS_SC4)
                case SCHTBL_AUTOSTART_SYNCHRONE:
                    /* IMPROVEMENT: Add support  */
                    break;
    #endif
                default:
                    ASSERT(0); 		// Illegal value
                    break;
                }
            }
        }
    }
#endif
}
#if (OS_SCHTBL_CNT != 0)
/*
 * Os_StartScheduleTableRelStartup: Start the relative scheduletable during shartup phase.
 * Difference between StartScheduleTableRel and Os_StartScheduleTableRelStartup is, no interrup check is not available (i.e. SWS_Os_00093)
 */
StatusType Os_StartScheduleTableRelStartup(ScheduleTableType sid, TickType offset) {
    StatusType rv = E_OK;
    OsSchTblType *sPtr;
    TickType max_offset;
    imask_t state;

    /* OS_STD_ERR_2: Function will return after calling ErrorHook */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
/** @req SWS_BSW_00029 */
#if (OS_STATUS_EXTENDED == STD_ON )
    /** @req SWS_Os_00275 */
    OS_VALIDATE_STD_2(SCHED_CHECK_ID(sid) , E_OS_ID,
    		          OSServiceId_StartScheduleTableRel,sid, offset);
#endif

    sPtr = Os_SchTblGet(sid);

#if	(OS_APPLICATION_CNT > 1)

    rv = Os_ApplHaveAccess( sPtr->accessingApplMask );
    if( rv != E_OK ) {
    	OS_STD_ERR_2(OSServiceId_StartScheduleTableRel,sid, offset);
    }

#endif

#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
    if( sPtr->sync != NULL ) {
        /* EXPLICIT or IMPLICIT */

        /** OS452 */ /** OS430 */
        if( sPtr->sync->syncStrategy == IMPLICIT ) {
            rv = E_OS_ID;
            OS_STD_ERR_2(OSServiceId_StartScheduleTableRel,sid, offset);
        }
    }
#endif

    max_offset = Os_CounterGetMaxValue(sPtr->counter);
/** @req SWS_BSW_00029 */
#if (OS_STATUS_EXTENDED == STD_ON )
    /** @req SWS_Os_00276 */
    /** @req SWS_Os_00332 */
    /*lint -e{9007} MISRA:FALSE_POSITIVE:No side effects of Os_SchTblGetInitialOffset:[MISRA 2012 Rule 13.5, required]*/
    if( (offset == 0) || ((offset + Os_SchTblGetInitialOffset(sPtr)) > max_offset ) ) {
        rv = E_OS_VALUE;
        OS_STD_ERR_2(OSServiceId_StartScheduleTableRel,sid, offset);
    }
#endif

    /** @req SWS_Os_00277 */
    if( sPtr->state != SCHEDULETABLE_STOPPED ) {
        rv = E_OS_STATE;
        OS_STD_ERR_2(OSServiceId_StartScheduleTableRel,sid, offset);
    }

    Irq_Save(state);
    /* calculate the expire value.. */
    /** @req SWS_Os_00278 */
    sPtr->expire_val = Os_CounterAdd(
                            Os_CounterGetValue(sPtr->counter),
                            max_offset,
                            offset + Os_SchTblGetInitialOffset(sPtr) );
    sPtr->state = SCHEDULETABLE_RUNNING;
    Irq_Restore(state);

    return rv;
}

/*
 * Os_StartScheduleTableAbsStartup: Start the absolute scheduletable during shartup phase.
 * Difference between StartScheduleTableAbs and Os_StartScheduleTableAbsStartup is, no interrup check is not available (i.e. SWS_Os_00093)
 */
StatusType Os_StartScheduleTableAbsStartup(ScheduleTableType sid, TickType start ){
    StatusType rv = E_OK;
    OsSchTblType *sTblPtr;
    imask_t state;

    /** @req SWS_Os_00348 */
    OS_VALIDATE_STD_2(SCHED_CHECK_ID(sid) , E_OS_ID,
    		          OSServiceId_StartScheduleTableAbs,sid, start);
    sTblPtr =  Os_SchTblGet(sid);

    /* OS_STD_ERR_2: Function will return after calling ErrorHook */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
#if	(OS_APPLICATION_CNT > 1)

    rv = Os_ApplHaveAccess( sTblPtr->accessingApplMask );
    if( rv != E_OK ) {
    	OS_STD_ERR_2(OSServiceId_StartScheduleTableAbs,sid, start);
    }

#endif

    /** @req SWS_Os_00349 */
    if( start > Os_CounterGetMaxValue(sTblPtr->counter) ) {
        rv = E_OS_VALUE;
        OS_STD_ERR_2(OSServiceId_StartScheduleTableAbs,sid, start);
    }

    /** @req SWS_Os_00350 */
    if( sTblPtr->state != SCHEDULETABLE_STOPPED ) {
        rv = E_OS_STATE;
        OS_STD_ERR_2(OSServiceId_StartScheduleTableAbs,sid, start);
    }


    Irq_Save(state);
    /** !req SWS_Os_00351 (SCHEDULETABLE_RUNNING_AND_SYNCHRONOUS not supported) */
    sTblPtr->expire_val = start + Os_SchTblGetInitialOffset(sTblPtr);
    sTblPtr->state = SCHEDULETABLE_RUNNING;
    Irq_Restore(state);

    return rv;
}
#endif

