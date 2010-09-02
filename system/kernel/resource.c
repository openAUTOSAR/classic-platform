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

#define valid_standard_id() (rPtr->nr < Os_CfgGetResourceCnt()) //&& !(rPtr->type == RESOURCE_TYPE_INTERNAL) )
#define valid_internal_id() (rPtr->nr < Os_CfgGetResourceCnt()) //&& (rPtr->type == RESOURCE_TYPE_INTERNAL) )


static StatusType GetResource_( OsResourceType * );
StatusType ReleaseResource_( OsResourceType * );

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
 * The service may be called from an ISR and from task level (see Figure 12-1).
 *
 * @param ResID
 * @return
 */
StatusType GetResource( ResourceType ResID ) {
	StatusType rv = E_OK;

	if( ResID == RES_SCHEDULER ) {
		if ( Os_SchedulerResourceIsOccupied() ) {
			rv = E_OS_ACCESS;
			goto err;
		} else {
			Os_GetSchedulerResource();
		}
	} else {
		if (ResID >= Os_CfgGetResourceCnt()) {
			rv = E_OS_ID;
			goto err;
		}

		OsResourceType *rPtr = Os_CfgGetResource(ResID);
		rv = GetResource_(rPtr);
	}

	if (rv != E_OK)
		goto err;

	OS_STD_END_1(OSServiceId_GetResource,ResID);
}

/**
 * ReleaseResource   is   the   counterpart   of   GetResource   and
 * serves to leave critical sections in the code that are assigned to
 * the resource referenced by <ResID>.
 *
 * For  information  on  nesting  conditions,  see  particularities  of
 * GetResource. The service may be called from an ISR and from task level (see
 * Figure 12-1).
 *
 * @param ResID
 * @return
 */

StatusType ReleaseResource( ResourceType ResID) {
    StatusType rv = E_OK;

	if( ResID == RES_SCHEDULER ) {
		if ( Os_SchedulerResourceIsFree() ) {
			rv = E_OS_NOFUNC;
			goto err;
		} else {
			Os_ReleaseSchedulerResource();
		}
	} else {
		if (ResID >= Os_CfgGetResourceCnt()) {
			rv = E_OS_ID;
			goto err;
		}

	    OsResourceType *rPtr = Os_CfgGetResource(ResID);
	    rv = ReleaseResource_(rPtr);
	}

	if (rv != E_OK)
	    goto err;

	/* do a rescheduling (in some cases) (see OSEK OS 4.6.1) */
	if ( (Os_TaskGetCurrent()->scheduling == FULL) &&
		 (os_sys.int_nest_cnt == 0) &&
		 (Os_SchedulerResourceIsFree()) ) {

		OsPcbType* top_pcb = Os_TaskGetTop();

		/* only dispatch if some other ready task has higher prio */
		if (top_pcb->prio > Os_TaskGetCurrent()->prio) {
			long flags;
			Irq_Save(flags);
			Os_Dispatch(0);
			Irq_Restore(flags);
		}
	}

	OS_STD_END_1(OSServiceId_ReleaseResource,ResID);
}


/**
 * Internal GetResource function...
 *
 * @param rPtr
 * @return
 */

static StatusType GetResource_( OsResourceType * rPtr ) {
	StatusType rv = E_OK;

	if( rPtr->nr == RES_SCHEDULER ) {
		// Lock the scheduler
		os_sys.scheduler_lock = 1;
	}

	/* Check if valid resource */
	if( !valid_standard_id() ) {
		rv = E_OS_ID;
		goto err;
	}

	/* check if we have access
	 * TODO: This gives access to all resources for ISR2s but we should respect the OsIsrResourceRef [0..*] here.
	 */
	if ( Os_TaskGetCurrent()->proc_type != PROC_ISR2) {
		if ( !(Os_TaskGetCurrent()->resourceAccess & (1 << rPtr->nr)) ) {
			rv = E_OS_ACCESS;
			goto err;
		}
	}

	/* @req OSEK
	 * Attempt to get a resource which is already occupied by any task
     * or ISR, or the statically assigned priority of the calling task or
     * interrupt routine is higher than the calculated ceiling priority,
     * E_OS_ACCESS
	 */
	if( (Os_TaskGetCurrent()->prio > rPtr->ceiling_priority )
#if ( OS_SC3 == STD_ON ) || ( OS_SC4 == STD_ON )
		|| ( get_curr_application_id() !=  rPtr->application_owner_id)
#endif
		|| ( rPtr->owner != (TaskType)(-1)))
	{
		rv = E_OS_ACCESS;
		goto err;
	}

	rPtr->owner = get_curr_pid();
	rPtr->old_task_prio = os_pcb_set_prio(Os_TaskGetCurrent() ,rPtr->ceiling_priority);

	if( rPtr->type != RESOURCE_TYPE_INTERNAL ) {
		TAILQ_INSERT_TAIL(&Os_TaskGetCurrent()->resource_head, rPtr, listEntry);
	}

	goto ok;
err:
	ERRORHOOK(rv);
ok:
	return rv;
}


/**
 * Internal release resource..
 * @param rPtr
 * @return
 */
StatusType ReleaseResource_( OsResourceType * rPtr ) {
	if (!valid_standard_id()) {
		return E_OS_ID;
	}

	/* check if we have access
	 * TODO: This gives access to all resources for ISR2s but we should respect the OsIsrResourceRef [0..*] here.
	 */
	if ( Os_TaskGetCurrent()->proc_type != PROC_ISR2) {
		if ( !(Os_TaskGetCurrent()->resourceAccess & (1 << rPtr->nr)) ) {
			return E_OS_ACCESS;
		}
	}

	/* if we are not holding this resource */
	if (rPtr->owner != Os_TaskGetCurrent()->pid) {
		return E_OS_NOFUNC;
	}

	// Release it...
	rPtr->owner = (TaskType) (-1);
	TAILQ_REMOVE(&Os_TaskGetCurrent()->resource_head, rPtr, listEntry);
	os_pcb_set_prio(Os_TaskGetCurrent(), rPtr->old_task_prio);
	return E_OK;
}


void Os_ResourceGetInternal( void ) {
	OsResourceType *rt = os_get_resource_int_p();

	if( rt != NULL ) {
		OS_DEBUG(D_RESOURCE,"Get IR proc:%s prio:%u old_task_prio:%u\n",
				get_curr_pcb()->name,
				(unsigned)rt->ceiling_priority,
				(unsigned)rt->old_task_prio);
		GetResource_(rt);
	}
}

void Os_ResourceReleaseInternal( void ) {
	OsResourceType *rt = os_get_resource_int_p();

	if(  rt != NULL ) {
		OS_DEBUG(D_RESOURCE,"Rel IR proc:%s prio:%u old_task_prio:%u\n",
				get_curr_pcb()->name,
				(unsigned)rt->ceiling_priority,
				(unsigned)rt->old_task_prio);
		ReleaseResource_(rt);
	}
}



/**
 *
 * @param pcb_p
 * @return
 */
void Os_ResourceInit( void ) {
	//TAILQ_INIT(&pcb_p->resource_head);
	OsPcbType *pcb_p;
	OsResourceType *rsrc_p;
	int topPrio;

	/* Calculate ceiling priority
	 * We make this as simple as possible. The ceiling priority
	 * is set to the same priority as the highest priority task that
	 * access it.
	 * */
	for( int i=0; i < Os_CfgGetResourceCnt(); i++) {
		rsrc_p = Os_CfgGetResource(i);
		topPrio = 0;

		for( int pi = 0; pi < Os_CfgGetTaskCnt(); pi++) {

			pcb_p = os_get_pcb(pi);
			if(pcb_p->resourceAccess & (1<<i) ) {
				topPrio = MAX(topPrio,pcb_p->prio);
			}
		}
		rsrc_p->ceiling_priority = topPrio;
	}



	/* From OSEK:
	 * Non preemptable tasks are the most common usage of the concept
	 * of internal resources; they are tasks with a special internal
	 * resource of highest task priority assigned.
	 * --> Interpret this as we can set the priority to 32.
	 *
	 * Assign an internal resource with prio 32 to the tasks
	 * with scheduling=NON
	 *
	 *
	 */
#if 0
	for( int i=0; i < Os_CfgGetTaskCnt(); i++) {
		pcb_p = os_get_pcb(i);
		if(pcb_p->scheduling == NON ) {
			pcb_p->prio = OS_RES_SCHEDULER_PRIO;
		}
	}
#endif
}

