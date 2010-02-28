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

#define valid_standard_id() (rid->nr < Oil_GetResourceCnt()) //&& !(rid->type == RESOURCE_TYPE_INTERNAL) )
#define valid_internal_id() (rid->nr < Oil_GetResourceCnt()) //&& (rid->type == RESOURCE_TYPE_INTERNAL) )


static StatusType GetResource_( OsResourceType * );
StatusType ReleaseResource_( OsResourceType * );

StatusType GetResource( ResourceType ResID ) {
	OsResourceType *rid = Oil_GetResource(ResID);
	StatusType rv = GetResource_(rid);

	if (rv != E_OK)
	    goto err;

	OS_STD_END_1(OSServiceId_GetResource,ResID);
}

#if 0
StatusType GetResourceInternal( ResourceType ResID ) {
	return GetResource_(ResID,1);
}
#endif

static StatusType GetResource_( OsResourceType * rid ) {
	StatusType rv = E_OK;

	if( rid->nr == RES_SCHEDULER ) {
		// Lock the scheduler
		os_sys.scheduler_lock = 1;
	}

	/* Check if valid resource */
	if( !valid_standard_id() ) {
		rv = E_OS_ID;
		goto err;
	}
	// Check that the resource does not belong to another application or task
	if(	( (os_task_nr_to_mask(get_curr_pid()) & rid->task_mask ) == 0 )
#if ( OS_SC3 == STD_ON ) || ( OS_SC4 == STD_ON )
		|| ( get_curr_application_id() !=  rid->application_owner_id)
#endif
		|| ( rid->owner != (TaskType)(-1)))
	{
		rv = E_OS_ACCESS;
		goto err;
	}

	rid->owner = get_curr_pid();
	rid->old_task_prio = os_pcb_set_prio(Os_TaskGetCurrent() ,rid->ceiling_priority);

	if( rid->type != RESOURCE_TYPE_INTERNAL ) {
		TAILQ_INSERT_TAIL(&Os_TaskGetCurrent()->resource_head, rid, listEntry);
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
	    OsResourceType *rid = Oil_GetResource(ResID);
	    rv = ReleaseResource_(rid);
	}

	if (rv != E_OK)
	    goto err;

	OS_STD_END_1(OSServiceId_ReleaseResource,ResID);
}

StatusType ReleaseResource_( OsResourceType * rid ) {
	if (!valid_standard_id()) {
		return E_OS_ID;
	} else {

        // Release it...
        rid->owner = (TaskType) (-1);
        TAILQ_REMOVE(&Os_TaskGetCurrent()->resource_head, rid, listEntry);
        os_pcb_set_prio(Os_TaskGetCurrent(), rid->old_task_prio);
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
