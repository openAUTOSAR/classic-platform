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

/* ----------------------------[includes]------------------------------------*/
#include "os_i.h"

#if defined(CFG_LOG) && defined(LOG_OS_COUNTER)
#define _LOG_NAME_ "os_cnt"
#endif
#include "log.h"

/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
/* ----------------------------[private functions]---------------------------*/
static inline boolean IsCounterValid(CounterType counterId) {
    /*lint -e685 -e568 MISRA:ARGUMENT_CHECK:check counter id:[MISRA 2012 Rule 14.3, required] */
#if (OS_COUNTER_CNT != 0 )
    return (boolean)((CounterType)(counterId) < (CounterType)OS_COUNTER_CNT);
#else
    return FALSE;
#endif
}   

/* ----------------------------[public functions]----------------------------*/
#ifdef USE_RTE
Std_ReturnType Os_GetCounterValue(CounterType counterId, TickType * value) {
    return (Std_ReturnType)GetCounterValue( counterId , (TickRefType)value);  /*lint !e929 MISRA:STANDARDIZED_INTERFACE:pointer cast to correct type:[MISRA 2012 Rule 11.3, required] */
}


Std_ReturnType Os_GetElapsedValue(CounterType counterId, TickType * value, TickType * elapsedValue) {
    return (Std_ReturnType)GetElapsedValue( counterId , (TickRefType)value, (TickRefType)elapsedValue); /*lint !e929 MISRA:STANDARDIZED_INTERFACE:pointer cast to correct type:[MISRA 2012 Rule 11.3, required] */
}

#endif

/**
 *
 * @param counter_id
 * @return
 */

/** @req SWS_Os_00530 IncrementCounter() available in all Scalability Classes. */
/** @req SWS_Os_00399 */
/* @req SWS_Os_00529
 * Caveats of IncrementCounter(): If called from a task, rescheduling may take place.
 */
StatusType IncrementCounter( CounterType counter_id ) {
    StatusType rv = E_OK;
    OsCounterType *cPtr;
    imask_t flags;
#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
    const OsTaskVarType *currPcbPtr;
#endif

    /* Validation of parameters, if failure, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_1((Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT,
    		          OSServiceId_IncrementCounter,counter_id);   /* @req SWS_Os_00093 */
    /*lint -e{9027} MISRA:FALSE_POSITIVE:Not allowed for string, but operand is boolean:[MISRA 2012 Rule 10.1, required] */
    OS_VALIDATE_STD_1(IsCounterValid(counter_id), E_OS_ID,
    		          OSServiceId_IncrementCounter,counter_id);    /* @req SWS_Os_00285 */
    cPtr = Os_CounterGet(counter_id);
    OS_VALIDATE_STD_1(COUNTER_TYPE_SOFT == cPtr->type, E_OS_ID,
    		          OSServiceId_IncrementCounter,counter_id);    /* @req SWS_Os_00285 */

#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
    currPcbPtr = Os_SysTaskGetCurr();

    if( currPcbPtr->constPtr->applOwnerId != cPtr->applOwnerId ) {
    	ApplicationType appId;
        /* @req SWS_Os_00056 */
        OS_VALIDATE_STD_1( (Os_ApplCheckState(cPtr->applOwnerId) != E_OS_ACCESS) , E_OS_ACCESS,
        		           OSServiceId_IncrementCounter,counter_id);
        appId = Os_GetCurrTaskISROwnerId();
        OS_VALIDATE_STD_1( (Os_ApplCheckAccess(appId, cPtr->accessingApplMask) != E_OS_ACCESS) , E_OS_ACCESS,
		                   OSServiceId_IncrementCounter,counter_id);

#if (OS_NUM_CORES > 1)
        OS_EXT_VALIDATE( Os_ApplGetCore(cPtr->applOwnerId) != GetCoreID(), E_OS_ACCESS );
#endif
    }
#endif

    Irq_Save(flags);

    /** @req SWS_Os_00286 */
    cPtr->val = Os_CounterAdd( cPtr->val, Os_CounterGetMaxValue(cPtr), 1 );

#if OS_ALARM_CNT!=0
    Os_AlarmCheck(cPtr);
#endif
#if OS_SCHTBL_CNT!=0
    Os_SchTblCheck(cPtr);
#endif

    Irq_Restore(flags);

    /** @req SWS_Os_00321 */
    return rv;
}


/** @req SWS_Os_00532 GetCounterValue() available in all Scalability Classes. */
/** @req SWS_Os_00383 */

StatusType GetCounterValue( CounterType counter_id , TickRefType tick_ref)
{
    StatusType rv = E_OK;
    const OsCounterType *cPtr;
#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
    const OsTaskVarType *currPcbPtr;
#endif
    /* Validation of parameters, if failure, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_2((tick_ref != NULL), E_OS_PARAM_POINTER,
    		          OSServiceId_GetCounterValue,counter_id,tick_ref);   /* @req SWS_Os_00566 */
    OS_VALIDATE_STD_2((Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT,
    		          OSServiceId_GetCounterValue,counter_id,tick_ref);   /* @req SWS_Os_00093 */
    /*lint -e{9027} MISRA:FALSE_POSITIVE:Not allowed for string, but operand is boolean:[MISRA 2012 Rule 10.1, required] */
    OS_VALIDATE_STD_2(IsCounterValid(counter_id),E_OS_ID,
    		          OSServiceId_GetCounterValue,counter_id,tick_ref);    /* @req SWS_Os_00376 */
#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
    OS_VALIDATE_STD_2( (OS_VALIDATE_ADDRESS_RANGE(tick_ref,sizeof(TickType)) ==  TRUE ) , E_OS_ILLEGAL_ADDRESS ,
    		           OSServiceId_GetCounterValue,counter_id,tick_ref);/*@req SWS_Os_00051 */
#endif


    cPtr = Os_CounterGet(counter_id);


#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
    currPcbPtr = Os_SysTaskGetCurr();
    if( currPcbPtr->constPtr->applOwnerId != cPtr->applOwnerId ) {
    	ApplicationType appId;
        /* @req SWS_Os_00056 */
        /* Validation of parameters, if it failure, function will return */
        /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
        OS_VALIDATE_STD_2( (Os_ApplCheckState(cPtr->applOwnerId) != E_OS_ACCESS) , E_OS_ACCESS,
        		           OSServiceId_GetCounterValue,counter_id,tick_ref);
        appId = Os_GetCurrTaskISROwnerId();
        OS_VALIDATE_STD_2( (Os_ApplCheckAccess(appId, cPtr->accessingApplMask) != E_OS_ACCESS) , E_OS_ACCESS,
        		           OSServiceId_GetCounterValue,counter_id,tick_ref);

#if (OS_NUM_CORES > 1)
        if (Os_ApplGetCore(cPtr->applOwnerId) != GetCoreID()) {
            StatusType status = Os_NotifyCore(Os_ApplGetCore(cPtr->applOwnerId),
                                              OSServiceId_GetCounterValue,
                                              counter_id,
                                              (uint32_t)tick_ref,
                                              0);
            return status;
        }
#endif
    }
#endif

    /** @req SWS_Os_00377 */
    if( cPtr->type == COUNTER_TYPE_HARD ) {
        /* It's OSINTERNAL */
        *tick_ref = OS_SYS_PTR->tick;
    } else {
        *tick_ref = cPtr->val;
    }

    return rv;
}

/**
 *
 * @param counter_id        The counter to be read
 * @param val[in,out]       in,  The previously read tick value of the counter
 *                          out, Contains the current tick value of the counter.
 * @param elapsed_val[out]  The difference
 * @return
 */

/** @req SWS_Os_00392 OS provides the service GetElapsedValue() */
/** @req SWS_Os_00534 GetElapsedValue() available in all Scalability Classes. */
StatusType GetElapsedValue ( CounterType counter_id, TickRefType val, TickRefType elapsed_val)
{
    StatusType rv = E_OK;
    OsCounterType *cPtr;
    TickType currTick = 0;
    TickType max;

    /* @req SWS_Os_00566 */
    /* Validation of parameters, if failure, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_3( (val != NULL), E_OS_PARAM_POINTER,
    		           OSServiceId_GetElapsedValue, counter_id, val,elapsed_val);
    OS_VALIDATE_STD_3( (elapsed_val != NULL), E_OS_PARAM_POINTER,
    		           OSServiceId_GetElapsedValue, counter_id, val,elapsed_val);
#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
    OS_VALIDATE_STD_3( (OS_VALIDATE_ADDRESS_RANGE(val, sizeof(TickType)) == TRUE ) , E_OS_ILLEGAL_ADDRESS ,
    		           OSServiceId_GetElapsedValue, counter_id, val,elapsed_val);/*@req SWS_Os_00051 */
    OS_VALIDATE_STD_3( (OS_VALIDATE_ADDRESS_RANGE(elapsed_val, sizeof(TickType)) == TRUE ) , E_OS_ILLEGAL_ADDRESS ,
    		           OSServiceId_GetElapsedValue, counter_id, val,elapsed_val);/*@req SWS_Os_00051 */
#endif
    OS_VALIDATE_STD_3( (Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT,
    		           OSServiceId_GetElapsedValue, counter_id, val,elapsed_val);   /* @req SWS_Os_00093 */
    /*lint -e{9027} MISRA:FALSE_POSITIVE:Not allowed for string, but operand is boolean:[MISRA 2012 Rule 10.1, required] */
    OS_VALIDATE_STD_3( IsCounterValid(counter_id),E_OS_ID,
    		           OSServiceId_GetElapsedValue, counter_id, val,elapsed_val);    /* @req SWS_Os_00381 */
    cPtr = Os_CounterGet(counter_id);
    OS_VALIDATE_STD_3( (cPtr != NULL), E_OS_PARAM_POINTER,
    		           OSServiceId_GetElapsedValue, counter_id, val,elapsed_val);
    max = Os_CounterGetMaxValue(cPtr);

    /** @req SWS_Os_00391 */
    OS_VALIDATE_STD_3( *val <= max,E_OS_VALUE,
    		           OSServiceId_GetElapsedValue, counter_id, val,elapsed_val);

#if (OS_APPLICATION_CNT > 1) && (OS_NUM_CORES > 1)
    if (Os_ApplGetCore(cPtr->applOwnerId) != GetCoreID()) {
        StatusType status = Os_NotifyCore(Os_ApplGetCore(cPtr->applOwnerId),
                                          OSServiceId_GetElapsedValue,
                                          counter_id,
                                          (int32_t)val,
                                          (int32_t)elapsed_val);
        return status;
    }
#endif

    (void)GetCounterValue(counter_id,&currTick);

    /** @req SWS_Os_00382 */
    *elapsed_val = Os_CounterDiff(currTick,*val,max);

    /** @req SWS_Os_00460 */
    *val = currTick;

    return rv;
}

/*
 * The OsTick():
 * 1. The Decrementer is setup by Os_SysTickStart(period_ticks)
 * 2. Os_SysTickInit() setup INTC[7] to trigger OsTick
 * 3. OsTick() then increment counter os_tick_counter if used
 */
ISR(OsTick) {
    /* if not used, Os_Arc_OsTickCounter < 0 and ARC_OSTICK is STD_OFF*/
#if (ARC_OSTICK == STD_ON)
        OsCounterType *cPtr = Os_CounterGet(Os_Arc_OsTickCounter);
#if defined(CFG_KERNEL_EXTRA)
        OsTaskVarType *pcbPtr;
        imask_t flags;
#endif

        OS_SYS_PTR->tick++;
        /* @req SWS_Os_00374
         * The Operating System module shall handle all the initialization and
         * configuration of timers used directly by the Operating System module and
         * not handled by the GPT driver
         */
        cPtr->val = Os_CounterAdd( cPtr->val, Os_CounterGetMaxValue(cPtr), 1 );

#if defined(CFG_KERNEL_EXTRA)
        Irq_Save(flags);
        /* Check tasks in the timer queue for API calls with timeout */
        LOG_S_S(__func__,"CheckTQ");
        if( !Os_TimerQIsEmpty() ) {
            /* We have timeouts */
            pcbPtr = Os_TimerQFirst();
            if( pcbPtr->tmo == OS_SYS_PTR->tick ) {
                LOG_S_S("TMO, rel:", pcbPtr->constPtr->name);

                ASSERT( (pcbPtr->state & (ST_SLEEPING|ST_WAIT_SEM)) != 0  );

                /* We have timeout, so remove the task from the timer queue and make ready   */
                pcbPtr->rv = E_OS_TIMEOUT;
                Os_TimerQRemove(pcbPtr);
                /* ... and add to the ready queue */
                Os_TaskMakeReady(pcbPtr);
            }
        }
        Irq_Restore(flags);
#endif
#if OS_ALARM_CNT!=0
        Os_AlarmCheck(cPtr);
#endif
#if OS_SCHTBL_CNT!=0
        Os_SchTblCheck(cPtr);
#endif
#endif /* ARC_OSTICK == STD_ON */
}

TickType GetOsTick( void ) {
    return OS_SYS_PTR->tick;
}

/**
 * Initialize alarms and schedule-tables for the counters
 */
void Os_CounterInit( void ) {
#if OS_ALARM_CNT!=0
    {
        OsCounterType *cPtr;
        OsAlarmType *aPtr;

        aPtr = Os_AlarmGet(0);
        cPtr = aPtr->counter;
        /*lint -e{9036} MISRA:EXTERNAL_FILE:while(0) allowed to be used in macros:[MISRA 2012 Rule 14.4, required] */
        SLIST_INIT(&cPtr->alarm_head);
        /* Add the alarms to counters */
        for (AlarmType i = 0; i < OS_ALARM_CNT; i++) {
            aPtr = Os_AlarmGet(i);
            cPtr = aPtr->counter;
            /*lint -e{9036} MISRA:EXTERNAL_FILE:while(0) allowed to be used in macros:[MISRA 2012 Rule 14.4, required] */
            SLIST_INSERT_HEAD(&cPtr->alarm_head, aPtr, alarm_list);
        }
    }
#endif

#if OS_SCHTBL_CNT!=0
    {
        OsCounterType *cPtr;
        OsSchTblType *sPtr;

        /* Add the schedule tables to counters */
        sPtr = Os_SchTblGet(0);
        cPtr = sPtr->counter;
        /*lint -e{9036} MISRA:EXTERNAL_FILE:while(0) allowed to be used in macros:[MISRA 2012 Rule 14.4, required] */
        SLIST_INIT(&cPtr->alarm_head);
        for(ScheduleTableType i = 0; i < OS_SCHTBL_CNT; i++ ) {

            sPtr = Os_SchTblGet(i);
            cPtr = sPtr->counter;
            /*lint -e{9036} MISRA:EXTERNAL_FILE:while(0) allowed to be used in macros:[MISRA 2012 Rule 14.4, required] */
            SLIST_INSERT_HEAD(&cPtr->sched_head, sPtr, sched_list);
        }
    }
#endif
}
