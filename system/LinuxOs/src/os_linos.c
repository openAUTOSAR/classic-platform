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

#include "os_i.h"
#include "linos_logger.h" /* Logger functions */
#include <stdlib.h>
#include <stdio.h>

/* Stuff here is taken from core/system/Os/os_init.c */
OsErrorType os_error;


/* Stuff here is taken from ./core/system/Os/include/os_internal.h */

#define SG_OS_STD_ERR_3(_service_id,_p1,_p2,_p3) \
		Os_CallErrorHook(rv);  \
        return rv // Expecting calling function to provide the ending semicolon


/* Stuff here is taken from ./core/system/Os/os_alarm.c */

#define COUNTER_MAX(x)                  (x)->counter->alarm_base.maxallowedvalue
#define COUNTER_MIN_CYCLE(x)    (x)->counter->alarm_base.mincycle
#define ALARM_CHECK_ID(x)   ((x) < OS_ALARM_CNT)


StatusType SetAbsAlarm(AlarmType AlarmId, TickType Start, TickType Cycle) {
        imask_t flags;
        StatusType rv = E_OK;
        OsAlarmType *aPtr;

        OS_VALIDATE( ALARM_CHECK_ID(AlarmId), E_OS_ID);

        aPtr =  Os_AlarmGet(AlarmId);

#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
    OsTaskVarType *currPcbPtr = Os_SysTaskGetCurr();

    if( currPcbPtr->constPtr->applOwnerId != aPtr->applOwnerId ) {
    	OS_VALIDATE( (Os_ApplCheckState(aPtr->applOwnerId) != E_OS_ACCESS) , E_OS_ACCESS);
    	OS_VALIDATE( (Os_ApplCheckAccess(currPcbPtr->constPtr->applOwnerId, aPtr->accessingApplMask) != E_OS_ACCESS) , E_OS_ACCESS);

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

        OS_VALIDATE( ! (Start > COUNTER_MAX(aPtr)), E_OS_VALUE );    /** @req OS304 */

        if( Cycle != 0 &&
                ( (Cycle < COUNTER_MIN_CYCLE(aPtr)) ||
                  (Cycle > COUNTER_MAX(aPtr)) ) ) {
                /** @req OS304 */
                rv =  E_OS_VALUE;
                SG_OS_STD_ERR_3(OSServiceId_SetAbsAlarm,AlarmId, Start, Cycle);
        }

        Irq_Save(flags);
        if( aPtr->active == 1 ) {
                rv = E_OS_STATE;
                Irq_Restore(flags);
                SG_OS_STD_ERR_3(OSServiceId_SetAbsAlarm,AlarmId, Start, Cycle);
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


/**
 * The system service occupies the alarm <AlarmID> element.
 * After <increment> ticks have elapsed, the task assigned to the
 * alarm <AlarmID> is activated or the assigned event (only for
 * extended tasks) is set or the alarm-callback routine is called.
 *
 * @param alarm_id Reference to the alarm element
 * @param increment Relative value in ticks
 * @param cycle Cycle value in case of cyclic alarm. In case of single alarms, cycle shall be zero.
 * @return
 */

StatusType SetRelAlarm(AlarmType AlarmId, TickType Increment, TickType Cycle){
        StatusType rv = E_OK;
        imask_t flags;
        OsAlarmType *aPtr;

        OS_VALIDATE( ALARM_CHECK_ID(AlarmId), E_OS_ID);

        aPtr =  Os_AlarmGet(AlarmId);

        logger(LOG_INFO,"SetRelAlarm id:%d inc:%u cycle:%u",
                                        AlarmId,
                                        (unsigned)Increment,
                                        (unsigned)Cycle);

#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)

    OsTaskVarType *currPcbPtr = Os_SysTaskGetCurr();

    if( currPcbPtr->constPtr->applOwnerId != aPtr->applOwnerId ) {
    	OS_VALIDATE( (Os_ApplCheckState(aPtr->applOwnerId) != E_OS_ACCESS) , E_OS_ACCESS);
    	OS_VALIDATE( (Os_ApplCheckAccess(currPcbPtr->constPtr->applOwnerId, aPtr->accessingApplMask) != E_OS_ACCESS) , E_OS_ACCESS);

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
        if( (Increment == 0) || (Increment > COUNTER_MAX(aPtr)) ) {
                /** @req OS304 */
                rv =  E_OS_VALUE;
                SG_OS_STD_ERR_3(OSServiceId_SetRelAlarm,AlarmId, Increment, Cycle);
        } else {
                if( Cycle != 0 &&
                        ( (Cycle < COUNTER_MIN_CYCLE(aPtr)) ||
                          (Cycle > COUNTER_MAX(aPtr)) ) ) {
                        /** @req OS304 */
                        rv =  E_OS_VALUE;
                        SG_OS_STD_ERR_3(OSServiceId_SetRelAlarm,AlarmId, Increment, Cycle);
                }
        }

        {
                Irq_Save(flags);
                if( aPtr->active == 1 ) {
                        Irq_Restore(flags);
                        rv = E_OS_STATE;
                        SG_OS_STD_ERR_3(OSServiceId_SetRelAlarm,AlarmId, Increment, Cycle);
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
        }

        return rv;
}



void Os_IsrInit( void ) {

}

void StartOS( AppModeType Mode ) {

    OS_SYS_PTR->status.os_started = true;

    OS_SYS_PTR->appMode = Mode;

    //os_start();
    /* Originally in os_start core/system/Os/os_init.c */
        /* Alarm autostart */
#if OS_ALARM_CNT!=0
        Os_AlarmAutostart();
#endif

    /** @req OS424 */
    //ASSERT(0);


}

void ShutdownOS( StatusType Error ) {
	(void)Error;
	printf("Shutting down!");
	exit(Error);
}

StatusType GetResource( ResourceType ResID ) {
    return E_OK;
}
StatusType ReleaseResource( ResourceType ResID) {
    return E_OK;
}
