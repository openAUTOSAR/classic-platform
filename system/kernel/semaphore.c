/*
 * semaphore.c
 *
 *  Created on: 13 okt 2010
 *      Author: mahi
 *
 * Mutex usage:
 * - Serialized access to a global resource. The same task
 *   ALWAYS first use Os_WaitMutex() and the Os_ReleaseMutex()
 *   Should mutex be as GetResource()/ReleaseResource() be able to
 *   tinker with priotities..
 *
 *
 *   Task1:
 *     Os_WaitMutex(my_mutex)
 *     ..
 *     Os_ReleaseMutex(my_mutex)

 *   Task2
 *     Os_WaitMutex(my_mutex)
 *     ..
 *     Os_ReleaseMutex(my_mutex)
 *
 * Semaphore usage:
 * - Producer and consumer problems.
 *   Os_WaitSemaphore() and Os_SignalSemaphore().
 *
 *   ISR:
 *     Os_SignalSemaphore(my_semaphore)

 *   Task:
 *     Os_WaitSemaphore(my_semaphore)
 *
 */

StatusType WaitSemaphore( OsSemaphore *semPtr ) {
	uint32_t flags;

	Irq_Save(flags);

	--semPtr->val;

	if(semPtr->val < 0 ) {
		/* Wait for the semaphore to be signaled */
		Os_Dispatch(OP_WAIT_SEMAPHORE);
	}

	Irq_Restore(flags);

	return E_OK;
}

/**
 * Increate the semaphore value by 1.
 *
 * @param semPtr
 */
void SignalSemaphore( OsSemaphore *semPtr ) {
	uint32_t flags;

	Irq_Save(flags);

	assert( semPtr != NULL );

	++semPtr->val;

	if(semPtr->val <= 0 ) {
		Os_Dispatch(OP_SIGNAL_SEMAPHORE);
	}

	Irq_Restore(flags);

	return E_OK;
}


/*
 * Usage:
 */
StatusType CreateSemaphore( OsSemaphore *, int initialCount  ) {


}


/* With priority inheretance */
StatusType CreateMutex( OsMutex *mutexPtr ) {


}

StatusType WaitMutex( OsMutex *mutexPtr ) {


}

StatusType Os_ReleaseMutex( OsMutex *mutexPtr ) {


}









