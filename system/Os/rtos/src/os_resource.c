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

#include "os_i.h"

#if !defined(MAX)
#define MAX(_x,_y) (((_x) > (_y)) ? (_x) : (_y))
#endif


/* INFO
 * - If OsTaskSchedule = NON, Task it not preemptable, no internal resource may be assigned to a task
 *                       (cause it already have one of prio 32)
 *                       FULL, Task is preemptable
 * - On Schedule() .... This service has no influence on tasks with no internal resource
 *                      assigned (preemptable tasks).
 *
 * OSEK on internal resources:
 * - Non preemptable tasks are a special group with an internal resource of the
 *   same priority as RES_SCHEDULER assigned
 *
 *
 * Assign RES_SCHEDULER with prio 32.
 * Assign internal resources to NON preemptable task.
 *
 * So that leaves us with:
 * - NON
 *   - Cannot assign internal resource.
 *     It automatically gets internal resource with same prio as RES_SCHEDULER
 *
 * - FULL
 *   - Assigned. Used for grouping tasks.
 *   - No assigned.
 *
 * What does that mean?
 * - It's probably OK to do a GetResource(RES_SCHEDULER) from a NON task (although pointless)
 * - GetResource(<any>) from a NON task is wrong
 *
 * Generation/Implementation:
 * - Resources to 32. Alloc with .resourceAlloc = ((1<<RES_1) |(1<<RES_2));
 * - Keep allocated resources as stack to comply with LIFO order.
 * - A linked resource is just another name for an existing resource. See OsResource in Autosar SWS OS.
 *   This means that no resource object should be generated, just the define in Os_Cfg.h
 * - A task with Scheduling=NON have priority (although it's internal priority is 32)
 *
 */

struct OsResource resScheduler;

/**
 * This call serves to enter critical sections in the code that are
 * assigned to the resource referenced by <ResID>. A critical
 * section shall always be left using ReleaseResource.
 *
 * The OSEK priority ceiling protocol for resource management is described
 * in chapter 8.5. Nested resource occupation is only allowed if the inner
 * critical sections are completely executed within the surrounding critical
 * section (strictly stacked, see chapter 8.2, Restrictions when using
 * resources). Nested occupation of one and the same resource is also
 * forbidden! It is recommended that corresponding calls to GetResource and
 * ReleaseResource appear within the same function.
 *
 * It is not allowed to use services which are points of rescheduling for
 * non preemptable tasks  (TerminateTask,ChainTask,  Schedule  and  WaitEvent,
 * see  chapter  4.6.2)  in critical  sections.
 * Additionally,  critical  sections  are  to  be  left before completion of
 * an interrupt service routine.
 * Generally speaking, critical sections should be short.
 * The service may be called from an from task level (see Figure 12-1).
 * Resource access in ISR is not supported.
 *
 * @param ResID
 * @return
 */
/* @req OSEK_SWS_RM_00010 */
StatusType GetResource( ResourceType ResID ) {
    StatusType rv = E_OK;
    OsResourceType *rPtr;
    imask_t flags;

    /* Validation of parameters, if failure, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_1((Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT,
    		          OSServiceId_GetResource,ResID);   /* @req SWS_Os_00093 */

    /* RES_SCHEDULER is assigned OS_RESOURCE_CNT, so OS_RESOURCE_CNT is a valid Id */
    OS_VALIDATE_STD_1((ResID <= OS_RESOURCE_CNT), E_OS_ID,
    		          OSServiceId_GetResource,ResID);  /* @req OSEK_SWS_RM_00001 */ /*lint !e775 MISRA:CONFIGURATION:argument check:[MISRA 2004 Info, advisory] */

#if	(OS_APPLICATION_CNT > 1)

    if (ResID != RES_SCHEDULER) {
        rv = Os_ApplHaveAccess( Os_ResourceGet(ResID)->accessingApplMask );
        if( rv != E_OK ) {
            /* OS_STD_ERR_1: Function will return after calling ErrorHook */
            /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
        	OS_STD_ERR_1(OSServiceId_GetResource,ResID);
        }
    }

#endif

    Irq_Save(flags);

    if( OS_SYS_PTR->intNestCnt != 0 ) {
        /* Interrupt requesting for resource is not supported */
        rv = E_OS_ISR_RESOURCE;
        Irq_Restore(flags);
        /* OS_STD_ERR_1: Function will return after calling ErrorHook */
        /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
        OS_STD_ERR_1(OSServiceId_GetResource,ResID);
    } else {
        OsTaskVarType *taskPtr = Os_SysTaskGetCurr();

        if( ResID == RES_SCHEDULER ) {
            rPtr = OS_SYS_PTR->resScheduler;
        } else {

            /* Check we can access it:
             * 1. we don't have access to the resource
             * 2. resource already taken
             * 3. We don't have priority less than ceiling priority
             */
            rPtr = Os_ResourceGet(ResID);

            if( ((taskPtr->constPtr->resourceAccess & (1UL<< ResID)) == 0) ||	/* 1 */
                (rPtr->owner != NO_TASK_OWNER) ||							    /* 2 */
                (taskPtr->activePriority > rPtr->ceiling_priority) ) {		    /* 3 */
                rv = E_OS_ACCESS; /* @req OSEK_SWS_RM_00003 */
                Irq_Restore(flags);
                /* OS_STD_ERR_1: Function will return after calling ErrorHook */
                /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
                OS_STD_ERR_1(OSServiceId_GetResource,ResID);
            }
        }
        /* Add the resource to the list of resources held by this task */
        Os_TaskResourceAdd(rPtr,taskPtr);
    }

    Irq_Restore(flags);

    return rv;
}

/**
 * ReleaseResource   is   the   counterpart   of   GetResource   and
 * serves to leave critical sections in the code that are assigned to
 * the resource referenced by <ResID>.
 *
 * For  information  on  nesting  conditions,  see  particularities  of
 * GetResource. The service may be called from an task level.
 * resource access from ISR is not supported.
 *
 * @param ResID
 * @return
 */
/* @req OSEK_SWS_RM_00011*/
StatusType ReleaseResource( ResourceType ResID) {
    StatusType rv = E_OK;
    OsTaskVarType *pcbPtr = Os_SysTaskGetCurr();
    OsResourceType *rPtr;
    imask_t flags;

    /* Validation of parameters, if failure, function will return */
    /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */
    OS_VALIDATE_STD_1((Os_SysIntAnyDisabled() == FALSE) , E_OS_DISABLEDINT,
    		          OSServiceId_ReleaseResource,ResID);   /* @req SWS_Os_00093 */

    /* RES_SCHEDULER is assigned OS_RESOURCE_CNT, so OS_RESOURCE_CNT is a valid Id */
    OS_VALIDATE_STD_1((ResID <= OS_RESOURCE_CNT), E_OS_ID,
    		          OSServiceId_ReleaseResource,ResID); /* @req OSEK_SWS_RM_00005*/ /*lint !e775 MISRA:CONFIGURATION:argument check:[MISRA 2004 Info, advisory] */

    Irq_Save(flags);
    if( OS_SYS_PTR->intNestCnt != 0 ) {
    	/* Interrupt's release request for resource is not supported */
		rv = E_OS_ISR_RESOURCE;
		Irq_Restore(flags);
		OS_STD_ERR_1(OSServiceId_ReleaseResource,ResID);
    }
    else
    {
        /* OS_STD_ERR_1: Function will return after calling ErrorHook */
        /* This is not inline with Table 8, ISO26262-6:2011, Req 1a and 1h */

        if( ResID == RES_SCHEDULER ) {
            rPtr = OS_SYS_PTR->resScheduler;
        } else {
            /* Check we can access it */
            if( (pcbPtr->constPtr->resourceAccess & (1UL<< ResID)) == 0 ) {
                rv = E_OS_ID;
                OS_STD_ERR_1(OSServiceId_ReleaseResource,ResID);
            }
            rPtr = Os_ResourceGet(ResID);
        }

        /* Check for invalid configuration */
        if( rPtr->owner == NO_TASK_OWNER)
        {
            rv = E_OS_NOFUNC; /* @req OSEK_SWS_RM_00008 */
            Irq_Restore(flags);
            OS_STD_ERR_1(OSServiceId_ReleaseResource,ResID);
        }

        ASSERT(pcbPtr->activePriority >= rPtr->ceiling_priority);

        Os_TaskResourceRemove(rPtr,pcbPtr);

        /* do a rescheduling (in some cases) (see OSEK OS 4.6.1) */
        if ( (pcbPtr->constPtr->scheduling == FULL) ) {

            const OsTaskVarType* top_pcb = Os_TaskGetTop();

            /* only dispatch if some other ready task has higher prio */
            if (top_pcb->activePriority > Os_SysTaskGetCurr()->activePriority) {
                Os_Dispatch(OP_RELEASE_RESOURCE);
            }
        }
    }
    Irq_Restore(flags);

    return rv;
}


void Os_ResourceGetInternal( void ) {
    OsTaskVarType *pcbPtr = Os_SysTaskGetCurr();
    OsResourceType *rPtr = pcbPtr->constPtr->resourceIntPtr;

    if( rPtr != NULL_PTR ) {
        OS_DEBUG(D_RESOURCE,"Get IR proc:%s prio:%u old_task_prio:%u\n",
                Os_SysTaskGetCurr()->constPtr->name,
                (unsigned)rPtr->ceiling_priority,
                (unsigned)rPtr->old_task_prio);
        Os_TaskResourceAdd(rPtr,pcbPtr);
    }
}

void Os_ResourceReleaseInternal( void ) {
    OsTaskVarType *pcbPtr = Os_SysTaskGetCurr();
    OsResourceType *rPtr = pcbPtr->constPtr->resourceIntPtr;

    if(  rPtr != NULL_PTR ) {
        OS_DEBUG(D_RESOURCE,"Rel IR proc:%s prio:%u old_task_prio:%u\n",
                Os_SysTaskGetCurr()->constPtr->name,
                (unsigned)rPtr->ceiling_priority,
                (unsigned)rPtr->old_task_prio);
        Os_TaskResourceRemove(rPtr,pcbPtr);
    }
}



/**
 *
 * @param pcb_p
 * @return
 */
void Os_ResourceInit( void ) {

#if (OS_RESOURCE_CNT > 0)
    const OsTaskVarType *pcb_p;
    OsResourceType *rsrc_p;
    OsPriorityType topPrio;
#endif

    /* For now, assign the scheduler resource here */
    OS_SYS_PTR->resScheduler->ceiling_priority = OS_RES_SCHEDULER_PRIO;
    strcpy(OS_SYS_PTR->resScheduler->id,"RES_SCHEDULER");
    OS_SYS_PTR->resScheduler->nr = RES_SCHEDULER;
    OS_SYS_PTR->resScheduler->owner = NO_TASK_OWNER;

    /* Calculate ceiling priority
     * We make this as simple as possible. The ceiling priority
     * is set to the same priority as the highest priority task that
     * access it.
     *
     * Note that this applies both internal and standard resources.
     * */

#if (OS_RESOURCE_CNT > 0)
    for( uint32 i=0; i < OS_RESOURCE_CNT; i++) {
        rsrc_p = Os_ResourceGet((ResourceType)i);
        topPrio = 0;

        for( TaskType pi = 0; pi < OS_TASK_CNT; pi++) {
            pcb_p = Os_TaskGet(pi);

            if((pcb_p->constPtr->resourceAccess & (1UL<<i)) != FALSE) {
                topPrio = MAX(topPrio,pcb_p->constPtr->prio);
            }
        }
        rsrc_p->ceiling_priority = topPrio;
    }
#endif
}

