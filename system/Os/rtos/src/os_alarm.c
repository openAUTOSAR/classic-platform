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

/* ----------------------------[private define]------------------------------*/
#if (OS_ALARM_CNT!=0)

/* ----------------------------[private macro]-------------------------------*/
#define COUNTER_MAX(x)          (x)->counter->alarm_base.maxallowedvalue
#define COUNTER_MIN_CYCLE(x)    (x)->counter->alarm_base.mincycle

static inline StatusType Os_AlarmCheckCycle(TickType cycle, const OsAlarmType *aPtr) {
    StatusType status = E_OK;
    if( (cycle != 0) && ( (cycle < COUNTER_MIN_CYCLE(aPtr)) || (cycle > COUNTER_MAX(aPtr)) ) ) {
    	status =  E_OS_VALUE;
    }
    return status;
}

#define ALARM_CHECK_ID(x) 	((x) < OS_ALARM_CNT)

/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
/* ----------------------------[private functions]---------------------------*/

/*
 * Checks if the given alarm has expired, if so sets it to inactive.
 * Otherwise calculate a new expiration time
 */
static void AlarmProcess( OsAlarmType *aPtr ) {
    if( aPtr->cycletime == 0 ) {
        aPtr->active = 0;
    } else {
        // Calc new expire value..
        aPtr->expire_val = Os_CounterAdd( Os_CounterGetValue(aPtr->counter),
                                            Os_CounterGetMaxValue(aPtr->counter),
                                            aPtr->cycletime);
    }
}

/**
 * Os_internalSetRelAlarm: Handles setting of relative alarm both for API and at startup
 * Difference between SetRelAlarm API and SetRelAlarm at startup; no
 * interrupt check is available (i.e. SWS_Os_00093)
 *
 * @param AlarmId Reference to alarm
 * @param Increment Relative value in ticks
 * @param Cycle Referenced alarm's cycle time in ticks
 * @param isStartup is FALSE when called from API, TRUE at startup
 * @return Error code
 */
static StatusType Os_internalSetRelAlarm(AlarmType AlarmId, TickType Increment, TickType Cycle, boolean isStartup){
    StatusType rv = E_OK;
    imask_t flags;
    OsAlarmType *aPtr;

    /* Validation of parameters, if it fails, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    if ((boolean)FALSE == isStartup) {
        OS_VALIDATE_STD_3( (Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT,
                       OSServiceId_SetRelAlarm,AlarmId, Increment, Cycle);   /* @req SWS_Os_00093 */
    }
    OS_VALIDATE_STD_3( ALARM_CHECK_ID(AlarmId), E_OS_ID,
                       OSServiceId_SetRelAlarm,AlarmId, Increment, Cycle);  /* @req OSEK_SWS_AL_00009 */

    aPtr =  Os_AlarmGet(AlarmId); /* @req OSEK_SWS_AL_00013 */

    OS_DEBUG(D_ALARM,"SetRelAlarm id:%d inc:%u cycle:%u\n",
                    AlarmId,
                    (unsigned)Increment,
                    (unsigned)Cycle);

#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)

    const OsTaskVarType *currPcbPtr = Os_SysTaskGetCurr();

    // Checking state and access of alarm only done when called from API, not startup.
    if( ((boolean)FALSE == isStartup) && (currPcbPtr->constPtr->applOwnerId != aPtr->applOwnerId) ) {
        ApplicationType appId;
        OS_VALIDATE_STD_3( (Os_ApplCheckState(aPtr->applOwnerId) != E_OS_ACCESS) , E_OS_ACCESS,
                           OSServiceId_SetRelAlarm,AlarmId, Increment, Cycle);
        appId = Os_GetCurrTaskISROwnerId();
        OS_VALIDATE_STD_3( (Os_ApplCheckAccess(appId, aPtr->accessingApplMask) != E_OS_ACCESS) , E_OS_ACCESS,
                           OSServiceId_SetRelAlarm,AlarmId, Increment, Cycle);

#if (OS_NUM_CORES > 1)
        if (Os_ApplGetCore(aPtr->applOwnerId) != GetCoreID()) {
            StatusType status = Os_NotifyCore(Os_ApplGetCore(aPtr->applOwnerId),
                                              OSServiceId_SetRelAlarm,
                                              AlarmId,
                                              Increment,
                                              Cycle);
            return status;
        }
#endif
    }
#endif

    /** @req SWS_Os_00304 */
    OS_VALIDATE_STD_3( !((Increment == 0) || (Increment > COUNTER_MAX(aPtr))), E_OS_VALUE,
                       OSServiceId_SetRelAlarm,AlarmId, Increment, Cycle); /* @req OSEK_SWS_AL_00011 */
    OS_VALIDATE_STD_3( (Os_AlarmCheckCycle(Cycle, aPtr) != E_OS_VALUE) , E_OS_VALUE,
                       OSServiceId_SetRelAlarm,AlarmId, Increment, Cycle); /* @req OSEK_SWS_AL_00012 */

    Irq_Save(flags);
    if( aPtr->active == 1 ) {
            Irq_Restore(flags);
        rv = E_OS_STATE; /* @req OSEK_SWS_AL_00010 */
        /* OS_STD_ERR_3: Function will return after calling ErrorHook */
        /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
        OS_STD_ERR_3(OSServiceId_SetRelAlarm,AlarmId, Increment, Cycle);
    }

    aPtr->active = 1;
    aPtr->expire_val = Os_CounterAdd(
                            Os_CounterGetValue(aPtr->counter),
                            COUNTER_MAX(aPtr),
                            Increment);
    aPtr->cycletime = Cycle;

    Irq_Restore(flags);
    OS_DEBUG(D_ALARM,"  expire:%u cycle:%u\n",
            (unsigned)aPtr->expire_val,
            (unsigned)aPtr->cycletime);

    return rv;
}


/**
 * Os_internalSetAbsAlarm: Handles setting of absolute alarm both for API and at startup
 * Difference between SetAbsAlarm API and SetAbsAlarm at startup; no
 * interrupt check is available (i.e. SWS_Os_00093)
 *
 * @param AlarmId Reference to alarm
 * @param Start Tick value when the alarm expires for the first time
 * @param Cycle Referenced alarm's cycle time in ticks
 * @param isStartup is FALSE when called from API, TRUE at startup
 * @return Error code
 */
static StatusType Os_internalSetAbsAlarm(AlarmType AlarmId, TickType Start, TickType Cycle, boolean isStartup) {
    imask_t flags;
    StatusType rv = E_OK;
    OsAlarmType *aPtr;

    /* Validation of parameters, if it fails, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    if ((boolean)FALSE == isStartup) {
        OS_VALIDATE_STD_3( (Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT,
                       OSServiceId_SetAbsAlarm,AlarmId, Start, Cycle);   /* @req SWS_Os_00093 */
    }
    OS_VALIDATE_STD_3( ALARM_CHECK_ID(AlarmId), E_OS_ID,
                       OSServiceId_SetAbsAlarm,AlarmId, Start, Cycle); /* @req OSEK_SWS_AL_00015 */

    aPtr =  Os_AlarmGet(AlarmId); /* @req OSEK_SWS_AL_00019 */

#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
    const OsTaskVarType *currPcbPtr = Os_SysTaskGetCurr();

    // Checking state and access of alarm only done when called from API, not startup.
    if( (((boolean)FALSE == isStartup)) && (currPcbPtr->constPtr->applOwnerId != aPtr->applOwnerId)) {
        ApplicationType appId;
        OS_VALIDATE_STD_3( (Os_ApplCheckState(aPtr->applOwnerId) != E_OS_ACCESS) , E_OS_ACCESS,
                           OSServiceId_SetAbsAlarm,AlarmId, Start, Cycle);
        appId = Os_GetCurrTaskISROwnerId();
        OS_VALIDATE_STD_3( (Os_ApplCheckAccess(appId, aPtr->accessingApplMask) != E_OS_ACCESS) , E_OS_ACCESS,
                           OSServiceId_SetAbsAlarm,AlarmId, Start, Cycle);

#if (OS_NUM_CORES > 1)
        if (Os_ApplGetCore(aPtr->applOwnerId) != GetCoreID()) {
            StatusType status = Os_NotifyCore(Os_ApplGetCore(aPtr->applOwnerId),
                                              OSServiceId_SetAbsAlarm,
                                              AlarmId,
                                              Start,
                                              Cycle);
            return status;
        }
#endif
    }
#endif

    /** @req SWS_Os_00304 */
    OS_VALIDATE_STD_3( !(Start > COUNTER_MAX(aPtr)), E_OS_VALUE,
                       OSServiceId_SetAbsAlarm,AlarmId, Start, Cycle); /* @req OSEK_SWS_AL_00017 */
    OS_VALIDATE_STD_3( (Os_AlarmCheckCycle(Cycle, aPtr) != E_OS_VALUE) , E_OS_VALUE,
                       OSServiceId_SetAbsAlarm,AlarmId, Start, Cycle); /* @req OSEK_SWS_AL_00018 */

    Irq_Save(flags);
    if( aPtr->active == 1 ) {
        rv = E_OS_STATE; /* @req OSEK_SWS_AL_00016 */
        Irq_Restore(flags);
        /* OS_STD_ERR_3: Function will return after calling ErrorHook */
        /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
        OS_STD_ERR_3(OSServiceId_SetAbsAlarm,AlarmId, Start, Cycle);
    }

    aPtr->active = 1;

    aPtr->expire_val = Start;
    aPtr->cycletime = Cycle;

    Irq_Restore(flags);

    OS_DEBUG(D_ALARM,"  expire:%u cycle:%u\n",
                    (unsigned)aPtr->expire_val,
                    (unsigned)aPtr->cycletime);

    return rv;
}


/* ----------------------------[public functions]----------------------------*/

/**
 * The system service  GetAlarmBase  reads the alarm base
 * characteristics. The return value <Info> is a structure in which
 * the information of data type AlarmBaseType is stored.
 *
 * @param alarm_id  Reference to alarm
 * @param info Reference to structure with constants of the alarm base.
 * @return
 */
/*@req OSEK_SWS_AL_00001*/
StatusType GetAlarmBase( AlarmType AlarmId, AlarmBaseRefType Info ) {
    StatusType rv = E_OK;

    /* Validation of parameters, if it fails, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_2( (Info != NULL_PTR), E_OS_PARAM_POINTER,
                       OSServiceId_GetAlarmBase,AlarmId, Info);   /* @req SWS_Os_00566 */
#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
    OS_VALIDATE_STD_2( (OS_VALIDATE_ADDRESS_RANGE(Info,sizeof(AlarmBaseType)) == TRUE ) , E_OS_ILLEGAL_ADDRESS ,
    		            OSServiceId_GetAlarmBase,AlarmId, Info); /*@req SWS_Os_00051 */
#endif
    OS_VALIDATE_STD_2( (Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT,
    		           OSServiceId_GetAlarmBase,AlarmId, Info);   /* @req SWS_Os_00093 */
    OS_VALIDATE_STD_2( ALARM_CHECK_ID(AlarmId), E_OS_ID,
    		           OSServiceId_GetAlarmBase,AlarmId, Info);   /* @req OSEK_SWS_AL_00002 */

#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
    const OsTaskVarType *currPcbPtr = Os_SysTaskGetCurr();
    const OsAlarmType *aPtr =  Os_AlarmGet(AlarmId);

    if( currPcbPtr->constPtr->applOwnerId != aPtr->applOwnerId ) {
        ApplicationType appId;

        OS_VALIDATE_STD_2( (Os_ApplCheckState(aPtr->applOwnerId) != E_OS_ACCESS) , E_OS_ACCESS,
        		           OSServiceId_GetAlarmBase,AlarmId, Info);
        appId = Os_GetCurrTaskISROwnerId();
        OS_VALIDATE_STD_2( (Os_ApplCheckAccess(appId, aPtr->accessingApplMask) != E_OS_ACCESS) , E_OS_ACCESS,
        		           OSServiceId_GetAlarmBase,AlarmId, Info);

#if	(OS_NUM_CORES > 1)
        if (Os_ApplGetCore(aPtr->applOwnerId) != GetCoreID()) {
            StatusType status = Os_NotifyCore(Os_ApplGetCore(aPtr->applOwnerId),
                                              OSServiceId_GetAlarmBase,
                                              AlarmId,
                                              (uint32)Info,
                                              0);
            return status;
        }
#endif
    }
#endif
    /* @req OSEK_SWS_AL_00003 */
    *Info = alarm_list[AlarmId].counter->alarm_base;

    return rv;
}

/**
 * The system service GetAlarm returns the relative value in ticks
 * before the alarm <AlarmID> expires.

 * @param AlarmId	Reference to an alarm
 * @param Tick[out]	Relative value in ticks before the alarm <AlarmID> expires.
 * @return
 */
/* @req OSEK_SWS_AL_00004 */
StatusType GetAlarm(AlarmType AlarmId, TickRefType Tick) {
    StatusType rv = E_OK;
    imask_t flags;
    const OsAlarmType *aPtr;

    /* Validation of parameters, if it fails, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_2( (Tick != NULL_PTR), E_OS_PARAM_POINTER,
    		           OSServiceId_GetAlarm,AlarmId, Tick);   /* @req SWS_Os_00566 */
#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
    OS_VALIDATE_STD_2( (OS_VALIDATE_ADDRESS_RANGE(Tick, sizeof(TickType) ) == TRUE ) , E_OS_ILLEGAL_ADDRESS ,
    		           OSServiceId_GetAlarm,AlarmId, Tick);/*@req SWS_Os_00051 */
#endif
    OS_VALIDATE_STD_2( (Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT,
    		           OSServiceId_GetAlarm,AlarmId, Tick);   /* @req SWS_Os_00093 */
    OS_VALIDATE_STD_2( ALARM_CHECK_ID(AlarmId), E_OS_ID,
    		           OSServiceId_GetAlarm,AlarmId, Tick); /* @req OSEK_SWS_AL_00005 */

    aPtr =  Os_AlarmGet(AlarmId); /* @req OSEK_SWS_AL_00007 */

#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
    const OsTaskVarType *currPcbPtr = Os_SysTaskGetCurr();

    if( currPcbPtr->constPtr->applOwnerId != aPtr->applOwnerId ) {
    	ApplicationType appId;
        OS_VALIDATE_STD_2( (Os_ApplCheckState(aPtr->applOwnerId) != E_OS_ACCESS) , E_OS_ACCESS,
        		           OSServiceId_GetAlarm,AlarmId, Tick);
        appId = Os_GetCurrTaskISROwnerId();
        OS_VALIDATE_STD_2( (Os_ApplCheckAccess(appId, aPtr->accessingApplMask) != E_OS_ACCESS) , E_OS_ACCESS,
		                   OSServiceId_GetAlarm,AlarmId, Tick);

#if	(OS_NUM_CORES > 1)
        if (Os_ApplGetCore(aPtr->applOwnerId) != GetCoreID()) {
            StatusType status = Os_NotifyCore(Os_ApplGetCore(aPtr->applOwnerId),
                                              OSServiceId_GetAlarm,
                                              AlarmId,
                                              (uint32)Tick,
                                              0);
            return status;
        }
#endif
    }
#endif

    Irq_Save(flags);
    if( aPtr->active == 0 ) {
        rv = E_OS_NOFUNC; /* @req OSEK_SWS_AL_00006 */
        Irq_Restore(flags);
        /* OS_STD_ERR_2: Function will return after calling ErrorHook */
        /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
        OS_STD_ERR_2(OSServiceId_GetAlarm,AlarmId, Tick);
    }

    *Tick = Os_CounterDiff( 	aPtr->expire_val,
                                Os_CounterGetValue(aPtr->counter),
                                Os_CounterGetMaxValue(aPtr->counter) );

    Irq_Restore(flags);

    return rv;
}

/**
 * The system service occupies the alarm <AlarmID> element.
 * After <increment> ticks have elapsed, the task assigned to the
 * alarm <AlarmID> is activated or the assigned event (only for
 * extended tasks) is set or the alarm-callback routine is called.
 *
 * @param AlarmId Reference to the alarm element
 * @param increment Relative value in ticks
 * @param cycle Cycle value in case of cyclic alarm. In case of single alarms, cycle shall be zero.
 * @return
 */
/* @req OSEK_SWS_AL_00008 */
StatusType SetRelAlarm(AlarmType AlarmId, TickType Increment, TickType Cycle) {
    return Os_internalSetRelAlarm(AlarmId, Increment, Cycle, FALSE);
}

/**
 * The  system  service  occupies  the  alarm  <AlarmID>  element.
 * When <start> ticks are reached, the task assigned to the alarm
 *
 * If the absolute value <start> is very close to the current counter
 * value, the alarm may expire, and the task may become ready or
 * the  alarm-callback  may  be  called  before  the  system  service
 * returns to the user.
 * If  the  absolute  value  <start>  already  was  reached  before  the
 * system call, the alarm shall only expire when the absolute value
 * <start>  is  reached  again,  i.e.  after  the  next  overrun  of  the
 * counter.
 *
 * If <cycle> is unequal zero, the alarm element is logged on again
 * immediately after expiry with the relative value <cycle>.
 *
 * The alarm <AlarmID> shall not already be in use.
 * To  change  values  of  alarms  already  in  use  the  alarm  shall  be
 * cancelled first.
 *
 * If  the  alarm  is  already  in  use,  this  call  will  be  ignored  and  the
 * error E_OS_STATE is returned.
 *
 * Allowed on task level and in ISR, but not in hook routines.
 *
 * @param AlarmId
 * @param Start
 * @param Cycle
 * @return
 */
/* @req OSEK_SWS_AL_00014 */
StatusType SetAbsAlarm(AlarmType AlarmId, TickType Start, TickType Cycle) {
    return Os_internalSetAbsAlarm(AlarmId, Start, Cycle, FALSE);
}

/*
 * The system service CancelAlarm cancels the alarm <AlarmID>.
 */
/* @req OSEK_SWS_AL_00020 */
StatusType CancelAlarm(AlarmType AlarmId) {
    StatusType rv = E_OK;
    OsAlarmType *aPtr;
    imask_t flags;

    /* Validation of parameters, if it fails, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_1( (Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT,
    		           OSServiceId_CancelAlarm,AlarmId);   /* @req SWS_Os_00093 */
    OS_VALIDATE_STD_1( ALARM_CHECK_ID(AlarmId), E_OS_ID,
    		           OSServiceId_CancelAlarm,AlarmId); /* @req OSEK_SWS_AL_00021 */

    aPtr = Os_AlarmGet(AlarmId); /* @req OSEK_SWS_AL_00023 */

#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
    const OsTaskVarType *currPcbPtr = Os_SysTaskGetCurr();

    if( currPcbPtr->constPtr->applOwnerId != aPtr->applOwnerId ) {
    	ApplicationType appId;
        OS_VALIDATE_STD_1( (Os_ApplCheckState(aPtr->applOwnerId) != E_OS_ACCESS) , E_OS_ACCESS,
        		           OSServiceId_CancelAlarm,AlarmId);
        appId = Os_GetCurrTaskISROwnerId();
        OS_VALIDATE_STD_1( (Os_ApplCheckAccess(appId, aPtr->accessingApplMask) != E_OS_ACCESS) , E_OS_ACCESS,
		                   OSServiceId_CancelAlarm,AlarmId);
    }
#if	(OS_NUM_CORES > 1)
    if (Os_ApplGetCore(aPtr->applOwnerId) != GetCoreID()) {
        StatusType status = Os_NotifyCore(Os_ApplGetCore(aPtr->applOwnerId),
                                          OSServiceId_CancelAlarm,
                                          AlarmId,
                                          0,
                                          0);
        return status;
    }
#endif
#endif

    Irq_Save(flags);
    if( aPtr->active == 0 ) {
        rv = E_OS_NOFUNC; /* @req OSEK_SWS_AL_00022 */
        Irq_Restore(flags);
        /* OS_STD_ERR_1: Function will return after calling ErrorHook */
        /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
        OS_STD_ERR_1(OSServiceId_CancelAlarm,AlarmId);
    }

    aPtr->active = 0;

    Irq_Restore(flags);

    return rv;
}

/*
 * Check the action of the alarm and progress accordingly;
 * either activate a task, set an event or increment the counter
 * */
void Os_AlarmCheck( const OsCounterType *c_p ) {
    OsAlarmType *aPtr;
    StatusType rv;

    /*lint -e{9036} MISRA:EXTERNAL_FILE::[MISRA 2012 Rule 14.4, required] */
    SLIST_FOREACH(aPtr,&c_p->alarm_head,alarm_list) {
        if( aPtr->active && (c_p->val == aPtr->expire_val) ) {
            /* Check if the alarms have expired */
            OS_DEBUG(D_ALARM,"expired %s id:%u val:%u\n",
                                            aPtr->name,
                                            (unsigned)aPtr->counter_id,
                                            (unsigned)aPtr->expire_val);

            switch( aPtr->action.type ) {
            case ALARM_ACTION_ACTIVATETASK:
                /* @req SWS_Os_00321 */
                if( ActivateTask(aPtr->action.task_id) != E_OK ) {
                    /* We ignore return value since handling of error occurs in ActivateTask */
                }
                AlarmProcess(aPtr);
                break;
            case ALARM_ACTION_SETEVENT:
                rv =  SetEvent(aPtr->action.task_id,aPtr->action.event_id);
                if( rv != E_OK ) {
                	Os_CallErrorHook(rv);
                }
                AlarmProcess(aPtr);
                break;
#if (OS_SC1 == STD_ON)
            case ALARM_ACTION_ALARMCALLBACK:
                break;
#endif
            case ALARM_ACTION_INCREMENTCOUNTER:
                /** @req SWS_Os_00301 */
                (void)IncrementCounter(aPtr->action.counter_id);
                AlarmProcess(aPtr);
                break;
            __CODE_COVERAGE_IGNORE__
            default:
                /* @CODECOV:DEFAULT_CASE:Default statement is required for defensive programming. */
                ASSERT(0);
            }
        }
    }
}

/* @req SWS_Os_00476
 * The Operating System module shall allow to automatically start
 * preconfigured absolute alarms during the start of the Operating System.
 */
void Os_AlarmAutostart(void) {

	AlarmType j;

    for (j = 0; j < OS_ALARM_CNT; j++) {
#if (OS_NUM_CORES > 1)
        if (Os_OnRunningCore(OBJECT_ALARM,j)) {
#endif
            const OsAlarmType *aPtr;
            aPtr = Os_AlarmGet(j);
            if (aPtr->autostartPtr != NULL_PTR) {
                const OsAlarmAutostartType *autoPtr = aPtr->autostartPtr;

                /*lint -e{9036} MISRA:STANDARDIZED_INTERFACE:Type defined by AUTOSAR:[MISRA 2012 Rule 14.4, required] */
                /* @CODECOV:OTHER_TEST_EXIST: We support only one appmode */
                __CODE_COVERAGE_IGNORE__
                if (OS_SYS_PTR->appMode & autoPtr->appModeRef) {
                    if (autoPtr->autostartType == ALARM_AUTOSTART_ABSOLUTE) {
                        (void)Os_internalSetAbsAlarm(j, autoPtr->alarmTime, autoPtr->cycleTime, TRUE);
                    } else {
                        (void)Os_internalSetRelAlarm(j, autoPtr->alarmTime, autoPtr->cycleTime, TRUE);
                    }
                }
            }
#if (OS_NUM_CORES > 1)
        }
#endif
    }
}

#endif
