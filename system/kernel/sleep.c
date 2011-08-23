
#include "Os.h"
#include "application.h"
#include "internal.h"
#include "task_i.h"
#include "sys.h"
#include <assert.h>
#include <string.h>

/**
 * Will sleep for sleep ticks. This works only for extended
 * tasks. If sleep is 0 it will just call the dispatcher to
 * see if there is anything with higher priority to run.
 *
 * Sleep(0) only makes sense from a NON task (since they highest
 * prio task is always running). SetEvent() nor ActivateTask() will
 * activate a sleeping task.
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


	pcbPtr = Os_SysTaskGetCurr();

	if (pcbPtr->constPtr->proc_type != PROC_EXTENDED) {
		rv = E_OS_ACCESS;
		goto err;
	}

	/* Check that we are not calling from interrupt context */
	if( Os_Sys.intNestCnt != 0 ) {
		rv =  E_OS_CALLEVEL;
		goto err;
	}

	if ( Os_TaskOccupiesResources(pcbPtr) ) {
		rv =  E_OS_RESOURCE;
		goto err;
	}

	Irq_Save(flags);

	if ( Os_SchedulerResourceIsFree() ) {
		if( sleep != 0 ) {
			TAILQ_INSERT_TAIL(&Os_Sys.timerHead,pcbPtr,timerEntry);
			pcbPtr->timerDec = sleep;
			Os_Dispatch(OP_SLEEP);
		} else {

			if( Os_SysTaskGetCurr()->constPtr->scheduling != NON ) {
				return E_OK;
			}

			OsTaskVarType *topTask = Os_TaskGetTop();
			if( topTask->activePriority  != pcbPtr->activePriority ) {
				Os_Dispatch(OP_SCHEDULE);
			}
		}
	}

	Irq_Restore(flags);

	OS_STD_END_1(OSServiceId_Sleep,sleep);
}
