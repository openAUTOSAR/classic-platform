
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

#include "linos_logger.h" /* Logger functions */
#include "Os.h"
#include <os_sys.h>  /* OS_SYS_PTR */
#include <os_i.h>    /* GEN_TASK_HEAD  */

extern CounterType Os_Arc_OsTickCounter;

/* Stuff here is taken from ./core/system/Os/os_counter.c */

StatusType IncrementCounter( CounterType counter_id ) {
    StatusType rv = E_OK;
    //logger(LOG_INFO, "                    IncrementCounter (%d)", counter_id);
    OS_SYS_PTR->tick++;

    // if not used, os_tick_counter < 0
    if( Os_Arc_OsTickCounter != -1UL ) {
        OsCounterType *cPtr = Os_CounterGet(Os_Arc_OsTickCounter);
#if defined(CFG_KERNEL_EXTRA)
        OsTaskVarType *pcbPtr;
#endif


        cPtr->val = Os_CounterAdd( cPtr->val, Os_CounterGetMaxValue(cPtr), 1 );

#if defined(CFG_KERNEL_EXTRA)
        /* Check tasks in the timer queue (here from Sleep() or WaitSemaphore() ) */
        TAILQ_FOREACH(pcbPtr, &OS_SYS_PTR->timerHead, timerEntry ) {
            --pcbPtr->timerDec;
            if( pcbPtr->timerDec <= 0 ) {
                /* Remove from the timer queue */
                TAILQ_REMOVE(&OS_SYS_PTR->timerHead, pcbPtr, timerEntry);
                /* ... and add to the ready queue */
                //Os_TaskMakeReady(pcbPtr);
                logger(LOG_INFO, "gnulinux_timer Task ready");
            }
        }
#endif
#if OS_ALARM_CNT!=0
        Os_AlarmCheck(cPtr);
#endif
#if OS_SCHTBL_CNT!=0
        Os_SchTblCheck(cPtr);
#endif
    }

    // logger(LOG_INFO, "*******************************    Timer test %d", OS_SYS_PTR->tick);
    return rv;
}


/**
 *  * Initialize alarms and schedule-tables for the counters
 *   */
void Os_CounterInit( void ) {
#if OS_ALARM_CNT!=0
        {
                OsCounterType *cPtr;
                OsAlarmType *aPtr;

                /* Add the alarms to counters */
                for (AlarmType i = 0; i < OS_ALARM_CNT; i++) {
                        aPtr = Os_AlarmGet(i);
                        cPtr = aPtr->counter;
                        SLIST_INSERT_HEAD(&cPtr->alarm_head, aPtr, alarm_list);
            logger(LOG_INFO, "Os_CounterInit adding alarm %d (%s)", 
            i, aPtr->name);
                }
        }
#endif

#if OS_SCHTBL_CNT!=0
        {
                OsCounterType *cPtr;
                OsSchTblType *sPtr;

                /* Add the schedule tables to counters */
                for(ScheduleTableType i = 0; i < OS_SCHTBL_CNT; i++ ) {
                        sPtr = Os_SchTblGet(i);
                        cPtr = sPtr->counter;
                        SLIST_INSERT_HEAD(&cPtr->sched_head, sPtr, sched_list);
            logger(LOG_INFO, "Os_CounterInit adding Schedule table %d", i);
                }
        }
#endif
}
