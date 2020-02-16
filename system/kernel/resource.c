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

#include "Os.h"
#include "application.h"
#include "internal.h"
#include "task_i.h"
#include "resource_i.h"
#include "sys.h"
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
  resource can be released (see chapter 8.2, �LIFO principle�).
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

#define valid_standard_id() (rPtr->nr < OS_RESOURCE_CNT) //&& !(rPtr->type == RESOURCE_TYPE_INTERNAL) )
#define valid_internal_id() (rPtr->nr < OS_RESOURCE_CNT) //&& (rPtr->type == RESOURCE_TYPE_INTERNAL) )



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
	OsResourceType *rPtr;
	uint32_t flags;

#if	(OS_APPLICATION_CNT > 1)

	if (ResID != RES_SCHEDULER) {
		rv = Os_ApplHaveAccess( Os_ResourceGet(ResID)->accessingApplMask );
		if( rv != E_OK ) {
			goto err;
		}
	}

#endif

	Irq_Save(flags);

	if( OS_SYS_PTR->intNestCnt != 0 ) {

		/* For interrupts to the scheduler resource seems just dumb to get */
		OsIsrVarType *isrPtr = Os_SysIsrGetCurr();

		/* Check we can access it */
		if( ((isrPtr->constPtr->resourceMask & (1<< ResID)) == 0) ||
			( ResID == RES_SCHEDULER )	) {
			rv = E_OS_ID;
			goto err;
		}

		rPtr = Os_ResourceGet(ResID);

		/* ceiling prio for ISR seems strange...so no */
		if( rPtr->owner != NO_TASK_OWNER ) {
			rv = E_OS_ACCESS;
			Irq_Restore(flags);
			goto err;
		}
		/* Add the resource to the list of resources held by this isr */
		Os_IsrResourceAdd(rPtr,isrPtr);

	} else {
		OsTaskVarType *taskPtr = Os_SysTaskGetCurr();

		if( ResID == RES_SCHEDULER ) {
			rPtr = &OS_SYS_PTR->resScheduler;
		} else {
			/* Check we can access it */
			if( (taskPtr->constPtr->resourceAccess & (1<< ResID)) == 0 ) {
				rv = E_OS_ID;
				goto err;
			}

			rPtr = Os_ResourceGet(ResID);
		}

		/* Check for invalid configuration */
		if( (rPtr->owner != NO_TASK_OWNER) ||
			(taskPtr->activePriority > rPtr->ceiling_priority) )
		{
			rv = E_OS_ACCESS;
			Irq_Restore(flags);
			goto err;
		}
		/* Add the resource to the list of resources held by this task */
		Os_TaskResourceAdd(rPtr,taskPtr);
	}

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
	OsTaskVarType *pcbPtr = Os_SysTaskGetCurr();
	OsResourceType *rPtr;
	uint32_t flags;

	Irq_Save(flags);
	if( ResID == RES_SCHEDULER ) {
		rPtr = &OS_SYS_PTR->resScheduler;
	} else {
		/* Check we can access it */
		if( (pcbPtr->constPtr->resourceAccess & (1<< ResID)) == 0 ) {
			rv = E_OS_ID;
			goto err;
		}
		rPtr = Os_ResourceGet(ResID);
	}

	/* Check for invalid configuration */
	if( rPtr->owner == NO_TASK_OWNER)
	{
		rv = E_OS_NOFUNC;
		Irq_Restore(flags);
		goto err;
	}

	if( (pcbPtr->activePriority < rPtr->ceiling_priority))
	{
		rv = E_OS_ACCESS;
		Irq_Restore(flags);
		goto err;
	}

	Os_TaskResourceRemove(rPtr,pcbPtr);

	/* do a rescheduling (in some cases) (see OSEK OS 4.6.1) */
	if ( (pcbPtr->constPtr->scheduling == FULL) &&
		 (OS_SYS_PTR->intNestCnt == 0) &&
		 (Os_SchedulerResourceIsFree()) ) {

		OsTaskVarType* top_pcb = Os_TaskGetTop();

		/* only dispatch if some other ready task has higher prio */
		if (top_pcb->activePriority > Os_SysTaskGetCurr()->activePriority) {
			Os_Dispatch(OP_RELEASE_RESOURCE);
		}
	}
	Irq_Restore(flags);

	OS_STD_END_1(OSServiceId_ReleaseResource,ResID);
}


void Os_ResourceGetInternal( void ) {
	OsTaskVarType *pcbPtr = Os_SysTaskGetCurr();
	OsResourceType *rt = pcbPtr->constPtr->resourceIntPtr;

	if( rt != NULL ) {
		OS_DEBUG(D_RESOURCE,"Get IR proc:%s prio:%u old_task_prio:%u\n",
				Os_SysTaskGetCurr()->constPtr->name,
				(unsigned)rt->ceiling_priority,
				(unsigned)rt->old_task_prio);
		Os_TaskResourceAdd(rt,pcbPtr);
	}
}

void Os_ResourceReleaseInternal( void ) {
	OsTaskVarType *pcbPtr = Os_SysTaskGetCurr();
	OsResourceType *rt = pcbPtr->constPtr->resourceIntPtr;

	if(  rt != NULL ) {
		OS_DEBUG(D_RESOURCE,"Rel IR proc:%s prio:%u old_task_prio:%u\n",
				Os_SysTaskGetCurr()->constPtr->name,
				(unsigned)rt->ceiling_priority,
				(unsigned)rt->old_task_prio);
		Os_TaskResourceRemove(rt,pcbPtr);
	}
}



/**
 *
 * @param pcb_p
 * @return
 */
void Os_ResourceInit( void ) {
	//TAILQ_INIT(&pcb_p->resourceHead);
	OsTaskVarType *pcb_p;
	OsResourceType *rsrc_p;
	int topPrio;


	/* For now, assign the scheduler resource here */
	OS_SYS_PTR->resScheduler.ceiling_priority = OS_RES_SCHEDULER_PRIO;
	strcpy(OS_SYS_PTR->resScheduler.id,"RES_SCHEDULER");
	OS_SYS_PTR->resScheduler.nr = RES_SCHEDULER;
	OS_SYS_PTR->resScheduler.owner = NO_TASK_OWNER;

	/* Calculate ceiling priority
	 * We make this as simple as possible. The ceiling priority
	 * is set to the same priority as the highest priority task that
	 * access it.
	 *
	 * Note that this applies both internal and standard resources.
	 * */
	for( int i=0; i < OS_RESOURCE_CNT; i++) {
		rsrc_p = Os_ResourceGet(i);
		topPrio = 0;

		for( int pi = 0; pi < OS_TASK_CNT; pi++) {

			pcb_p = Os_TaskGet(pi);


			if(pcb_p->constPtr->resourceAccess & (1<<i) ) {
				topPrio = MAX(topPrio,pcb_p->constPtr->prio);
			}

			/* Generator fix, add RES_SCHEDULER */
//			pcb_p->constPtr->resourceAccess |= (1 << RES_SCHEDULER) ;
		}
		rsrc_p->ceiling_priority = topPrio;
	}
}

