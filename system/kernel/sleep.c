
#include "Os.h"
#include "internal.h"
#include <assert.h>
#include <string.h>

/**
 * Will sleep for sleep ticks. This works only for extended
 * tasks. If sleep is 0 it will just call the dispatcher to
 * see if there is anything with higher priority to run.
 *
 *
 *
 * @param sleep
 * @return E_OS_ACCESS if called from a basic task
 *         E_OS_RESOURCE called while holding a resource
 *
 */

StatusType Sleep( TickType sleep ) {
	OsPcbType *pcbPtr = Os_TaskGetCurrent();
	uint32_t flags;

	Irq_Save(flags);


	if (pcbPtr->proc_type != PROC_EXTENDED) {
		return E_OS_ACCESS;
	}

	if ( Os_TaskOccupiesResources(pcbPtr) ) {
		return E_OS_RESOURCE;
	}

	if ( Os_SchedulerResourceIsFree() ) {
		if( sleep != 0 ) {
			TAILQ_INSERT_TAIL(&os_sys.timerHead,pcbPtr,timerEntry);
			pcbPtr->timerDec = sleep;
			Os_Dispatch(OP_SLEEP);
		} else {
			Os_Dispatch(OP_SCHEDULE);
		}

	}

	Irq_Restore(flags);
	return E_OK;
}
