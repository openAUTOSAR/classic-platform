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
#include <assert.h>
#include <string.h>


#if !defined(MAX)
#define MAX(_x,_y) (((_x) > (_y)) ? (_x) : (_y))
#endif


/*
Resource management at interrupt level is NOT supported


Testing
RM:
1. Priority ceiling: Call GetResource() from preemtive
   task and activate a task with higher priority than the ceiling protocol.
   The higher priority task should be swapped in.
2. Verify that you cannot allocate an internal resource with
   a) GetResource()
   b) ReleaseResource()
3. Internal resource. Allocate 1 internal resource to 3 tasks of different
   priorities. Verify that
   a) Higher priority tasks than the group can preement
   b) For tasks which have the same or lower priority as the highest priority within a group,
      the tasks within the group behave like non preemptable tasks ( OSEK 4.6.3)
4. Attempt to release a resource which has a lower ceiling priority
   than the statically assigned priority of the calling task or
   interrupt routine, E_OS_ACCESS
5. The  general  restriction  on  some  system  calls  that  they  are  not  to  be  called  with  resources
  occupied (chapter 8.2) does not apply to internal resources, as internal resources are handled
  within  those  calls.  However,  all  standard  resources  have  to  be  released  before  the  internal
  resource can be released (see chapter 8.2, “LIFO principle”).
6. Check LIFO order. Return E_OS_ACCESS if not in LIFO order..
7. Test Os_IsrAddResource().


task
- GetResource(RES_SCHEDULER) will lock the scheduler even for ISR2

TODO:
1. task.resourceAccess is already calculated by BSW builder. This is the bitmask
   of what resources is accessable by the task.
2.

  task.rsrcAccessMask & (1 << RES_SCHEDULER)

 *
 */

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


void Os_ResourceAlloc( OsResourceType *rPtr, OsPcbType *pcbPtr) {
	/* Save old task prio in resource and set new task prio */
	rPtr->owner = pcbPtr->pid;
	rPtr->old_task_prio = pcbPtr->prio;
	pcbPtr->prio = rPtr->ceiling_priority;

	if( rPtr->type != RESOURCE_TYPE_INTERNAL ) {
		TAILQ_INSERT_TAIL(&pcbPtr->resource_head, rPtr, listEntry);
	}
}

void Os_ResourceFree( OsResourceType *rPtr , OsPcbType *pcbPtr) {
	assert( rPtr->owner == pcbPtr->pid );
	rPtr->owner = NO_TASK_OWNER;
	pcbPtr->prio = rPtr->old_task_prio;

	if( rPtr->type != RESOURCE_TYPE_INTERNAL ) {
		/* The list can't be empty here */
		assert( !TAILQ_EMPTY(&pcbPtr->resource_head) );

		/* The list should be popped in LIFO order */
		assert( TAILQ_LAST(&pcbPtr->resource_head, head) == rPtr );

		/* Remove the entry */
		TAILQ_REMOVE(&pcbPtr->resource_head, rPtr, listEntry);
	}
}

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
	OsPcbType *pcbPtr = Os_TaskGetCurrent();
	OsResourceType *rPtr;
	uint32_t flags;

	Irq_Save(flags);

	if( ResID == RES_SCHEDULER ) {

		rPtr = &os_sys.resScheduler;
	} else {
		/* Check we can access it */
		if( (pcbPtr->resourceAccess & (1<< ResID)) == 0 ) {
			rv = E_OS_ID;
			goto err;
		}

		rPtr = Os_CfgGetResource(ResID);
	}

	/* Check for invalid configuration */
	if( (rPtr->owner != NO_TASK_OWNER) ||
		(pcbPtr->prio > rPtr->ceiling_priority) )
	{
		rv = E_OS_ACCESS;
		Irq_Restore(flags);
		goto err;
	}

	Os_ResourceAlloc(rPtr,pcbPtr);
	Irq_Restore(flags);

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
	OsPcbType *pcbPtr = Os_TaskGetCurrent();
	OsResourceType *rPtr;
	uint32_t flags;

	Irq_Save(flags);
	if( ResID == RES_SCHEDULER ) {
		rPtr = &os_sys.resScheduler;
	} else {
		/* Check we can access it */
		if( (pcbPtr->resourceAccess & (1<< ResID)) == 0 ) {
			rv = E_OS_ID;
			goto err;
		}
		rPtr = Os_CfgGetResource(ResID);
	}

	/* Check for invalid configuration */
	if( rPtr->owner == NO_TASK_OWNER)
	{
		rv = E_OS_NOFUNC;
		Irq_Restore(flags);
		goto err;
	}

	if( (pcbPtr->prio < rPtr->ceiling_priority))
	{
		rv = E_OS_ACCESS;
		Irq_Restore(flags);
		goto err;
	}

	Os_ResourceFree(rPtr,pcbPtr);

	/* do a rescheduling (in some cases) (see OSEK OS 4.6.1) */
	if ( (pcbPtr->scheduling == FULL) &&
		 (os_sys.int_nest_cnt == 0) &&
		 (Os_SchedulerResourceIsFree()) ) {

		OsPcbType* top_pcb = Os_TaskGetTop();

		/* only dispatch if some other ready task has higher prio */
		if (top_pcb->prio > Os_TaskGetCurrent()->prio) {
			Os_Dispatch(OP_RELEASE_RESOURCE);
		}
	}
	Irq_Restore(flags);

	OS_STD_END_1(OSServiceId_ReleaseResource,ResID);
}


void Os_ResourceGetInternal( void ) {
	OsPcbType *pcbPtr = Os_TaskGetCurrent();
	OsResourceType *rt = pcbPtr->resource_int_p;

	if( rt != NULL ) {
		OS_DEBUG(D_RESOURCE,"Get IR proc:%s prio:%u old_task_prio:%u\n",
				get_curr_pcb()->name,
				(unsigned)rt->ceiling_priority,
				(unsigned)rt->old_task_prio);
		Os_ResourceAlloc(rt,pcbPtr);
	}
}

void Os_ResourceReleaseInternal( void ) {
	OsPcbType *pcbPtr = Os_TaskGetCurrent();
	OsResourceType *rt = pcbPtr->resource_int_p;

	if(  rt != NULL ) {
		OS_DEBUG(D_RESOURCE,"Rel IR proc:%s prio:%u old_task_prio:%u\n",
				get_curr_pcb()->name,
				(unsigned)rt->ceiling_priority,
				(unsigned)rt->old_task_prio);
		Os_ResourceFree(rt,pcbPtr);
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


	/* For now, assign the scheduler resource here */
	os_sys.resScheduler.ceiling_priority = OS_RES_SCHEDULER_PRIO;
	strcpy(os_sys.resScheduler.id,"RES_SCHEDULER");
	os_sys.resScheduler.nr = RES_SCHEDULER;
	os_sys.resScheduler.owner = NO_TASK_OWNER;

	/* Calculate ceiling priority
	 * We make this as simple as possible. The ceiling priority
	 * is set to the same priority as the highest priority task that
	 * access it.
	 *
	 * Note that this applies both internal and standard resources.
	 * */
	for( int i=0; i < Os_CfgGetResourceCnt(); i++) {
		rsrc_p = Os_CfgGetResource(i);
		topPrio = 0;

		for( int pi = 0; pi < Os_CfgGetTaskCnt(); pi++) {

			pcb_p = os_get_pcb(pi);


			if(pcb_p->resourceAccess & (1<<i) ) {
				topPrio = MAX(topPrio,pcb_p->prio);
			}

			/* Generator fix, add RES_SCHEDULER */
			pcb_p->resourceAccess |= (1 << RES_SCHEDULER) ;
		}
		rsrc_p->ceiling_priority = topPrio;
	}
}

