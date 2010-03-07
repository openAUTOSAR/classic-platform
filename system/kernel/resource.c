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

#define valid_standard_id() (rPtr->nr < Oil_GetResourceCnt()) //&& !(rPtr->type == RESOURCE_TYPE_INTERNAL) )
#define valid_internal_id() (rPtr->nr < Oil_GetResourceCnt()) //&& (rPtr->type == RESOURCE_TYPE_INTERNAL) )


static StatusType GetResource_( OsResourceType * );
StatusType ReleaseResource_( OsResourceType * );

StatusType GetResource( ResourceType ResID ) {
	OsResourceType *rPtr = Oil_GetResource(ResID);
	StatusType rv = GetResource_(rPtr);

	if (rv != E_OK)
	    goto err;

	OS_STD_END_1(OSServiceId_GetResource,ResID);
}

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

StatusType ReleaseResource( ResourceType ResID) {
    StatusType rv = E_OK;
	if( ResID == RES_SCHEDULER ) {
		os_sys.scheduler_lock=0;
	} else {
	    OsResourceType *rPtr = Oil_GetResource(ResID);
	    rv = ReleaseResource_(rPtr);
	}

	if (rv != E_OK)
	    goto err;

	OS_STD_END_1(OSServiceId_ReleaseResource,ResID);
}

StatusType ReleaseResource_( OsResourceType * rPtr ) {
	if (!valid_standard_id()) {
		return E_OS_ID;
	} else {

        // Release it...
        rPtr->owner = (TaskType) (-1);
        TAILQ_REMOVE(&Os_TaskGetCurrent()->resource_head, rPtr, listEntry);
        os_pcb_set_prio(Os_TaskGetCurrent(), rPtr->old_task_prio);
        return E_OK;
	}
}


void Os_ResourceReleaseAll( uint32_t mask ) {

}

//
void Os_ResourceGetInternal( void ) {
	OsResourceType *rt = os_get_resource_int_p();

	if( rt != NULL ) {
		//simple_printf("Get IR proc:%s prio:%d old_task_prio:%d\n",get_curr_pcb()->name, rt->ceiling_priority,rt->old_task_prio);
		GetResource_(rt);
	}
	//GetResourceInternal(Os_TaskGetCurrent()->resource_internal);
}

void Os_ResourceReleaseInternal( void ) {
	OsResourceType *rt = os_get_resource_int_p();

	if(  rt != NULL ) {
		//simple_printf("Rel IR proc:%s prio:%d old_task_prio:%d\n",get_curr_pcb()->name,rt->ceiling_priority,rt->old_task_prio);
		ReleaseResource_(rt);
	}
	//ReleaseResource(Os_TaskGetCurrent()->resource_internal);
}
