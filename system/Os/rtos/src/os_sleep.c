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


/**
 * Will sleep for sleep ticks. This works only for extended
 * tasks. If sleep is 0 it will just call the dispatcher to
 * see if there is anything with higher or equal priority to run.
 *
 * @param sleep
 * @return E_OS_ACCESS if called from a basic task
 *         E_OS_RESOURCE called while holding a resource
 *         E_OS_CALLEVEL if called from interrupt context
 *         E_OK if called from a FULL task
 */

StatusType Sleep( TickType sleep ) {
    StatusType rv = E_OK;
    OsTaskVarType *pcbPtr;
    uint32_t flags;

    /* Validation of parameters, if failure, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_1( (Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT ,
    		     OSServiceId_Sleep,sleep);   /* @req SWS_Os_00093 */

    pcbPtr = Os_SysTaskGetCurr();

    OS_VALIDATE_STD_1( (pcbPtr->constPtr->proc_type == PROC_EXTENDED) , E_OS_ACCESS ,
    		           OSServiceId_Sleep,sleep);

    /* Check that we are not calling from interrupt context */
    OS_VALIDATE_STD_1( ( OS_SYS_PTR->intNestCnt == 0 ) , E_OS_CALLEVEL ,
    		           OSServiceId_Sleep,sleep);

    if ( Os_TaskOccupiesResources(pcbPtr) ) {
        rv =  E_OS_RESOURCE;
        /* OS_STD_ERR_1: Function will return after calling ErrorHook */
        /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
        OS_STD_ERR_1(OSServiceId_Sleep,sleep);
    }

    Irq_Save(flags);

    if ( Os_SchedulerResourceIsFree() ) {
        if( sleep != TMO_MIN ) {
            rv  = Os_DispatchToSleepWithTmo(OP_SLEEP,sleep);
            ASSERT(rv == E_OS_TIMEOUT);
            rv = E_OK;
        } else {

            /* Put us last in the ready list, by first removing us and
             * then adding us again */
            TAILQ_REMOVE(&OS_SYS_PTR->ready_head,pcbPtr,ready_list);

            TAILQ_INSERT_TAIL(& OS_SYS_PTR->ready_head,pcbPtr,ready_list);

            OsTaskVarType *topTask = Os_TaskGetTop();
            if( topTask != pcbPtr ) {
                Os_Dispatch(OP_SCHEDULE);
            }
        }
    }

    Irq_Restore(flags);

    return rv;
}
