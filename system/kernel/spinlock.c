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
#include "internal.h"
#include "spinlock_i.h"
#include "application.h"
#include "sys.h"

#define SPINLOCK_STD_END	\
		goto ok;        	\
	err:                	\
		ERRORHOOK(rv);  	\
	ok:                 	\
		return rv;

#define IsSpinlockValid(_spinlockId)   (((_spinlockId) < OS_SPINLOCK_CNT) )

static StatusType GetSpinlockDeadlockCheck(SpinlockIdType spinlock_id)
{
	StatusType rv = E_OK;
	OsSpinlockType *sPtr;
	OsTaskVarType *pcbPtr = Os_SysTaskGetCurr();

	/** @req OS696 */
	/* Check if the same task has already occupied the spinlock */
	if (pcbPtr != NULL) { // When the OS itself is running pcbPtr is NULL
		TAILQ_FOREACH(sPtr, &pcbPtr->spinlockHead, spinlockEntry ) {
			if (sPtr->id == spinlock_id)
				rv = E_OS_INTERFERENCE_DEADLOCK;
		}
	}

	/** !req OS690, OS708 */
	/* TODO: Check if other tasks or ISRs on the same core have already occupied the spinlock */

	/** !req OS691, OS709 */
	/* TODO: Check that occupying this spinlock is correct according to the nesting list */

	return rv;
}


static StatusType ReleaseSpinlockAccessCheck(SpinlockIdType spinlock_id)
{
	StatusType rv = E_OK;
	OsSpinlockType *sPtr;
	OsTaskVarType *pcbPtr = Os_SysTaskGetCurr();

	/** !req OS699 */
	/* Check if the task actually occupies the spinlock */
	if (pcbPtr != NULL) { // When the OS itself is running pcbPtr is NULL
		rv = E_OS_ACCESS;
		TAILQ_FOREACH(sPtr, &pcbPtr->spinlockHead, spinlockEntry ) {
			if (sPtr->id == spinlock_id)
				rv = E_OK;
		}
	}

	/** !req OS701 */
	/* TODO: Check that no other spinlock has to be released before this one */

	/** !req OS702 */
	/* TODO: Check that LIFO order is correct in relation to RESOURCES */

	return rv;
}


StatusType GetSpinlock(SpinlockIdType spinlock_id)
{
	StatusType rv = E_OK;
	OsSpinlockType *sPtr;
	OsTaskVarType *pcbPtr = Os_SysTaskGetCurr();

	/** @req OS689 */
	if( !IsSpinlockValid(spinlock_id) ) {
		rv = E_OS_ID;
		goto err;
	}

	sPtr = Os_SpinlockGet(spinlock_id);

#if	(OS_APPLICATION_CNT > 1)
	/** @req OS692 */
	rv = Os_ApplHaveAccess( sPtr->accessingApplMask );
	if( rv != E_OK ) {
		goto err;
	}
#endif

	rv = GetSpinlockDeadlockCheck(spinlock_id);
	if (rv != E_OK)
		goto err;

	/** @req OS687 */
	Os_GetSpinlock(sPtr);

	if (pcbPtr != NULL)
		TAILQ_INSERT_TAIL(&pcbPtr->spinlockHead, sPtr, spinlockEntry);

	/** @req OS688 */
	SPINLOCK_STD_END;
}


StatusType ReleaseSpinlock(SpinlockIdType spinlock_id)
{
	StatusType rv = E_OK;
	OsSpinlockType *sPtr;
	OsTaskVarType *pcbPtr = Os_SysTaskGetCurr();

	/** @req OS698 */
	if( !IsSpinlockValid(spinlock_id) ) {
		rv = E_OS_ID;
		goto err;
	}

	sPtr = Os_SpinlockGet(spinlock_id);

#if	(OS_APPLICATION_CNT > 1)
	/** @req OS700 */
	rv = Os_ApplHaveAccess( sPtr->accessingApplMask );
	if( rv != E_OK ) {
		goto err;
	}
#endif

	rv = ReleaseSpinlockAccessCheck(spinlock_id);
	if (rv != E_OK)
		goto err;

	if (pcbPtr != NULL)
		TAILQ_REMOVE(&pcbPtr->spinlockHead, sPtr, spinlockEntry);

	Os_ReleaseSpinlock(sPtr);

	/** @req OS697 */
	SPINLOCK_STD_END;
}


StatusType TryToGetSpinlock(SpinlockIdType spinlock_id, TryToGetSpinlockType* success)
{
	StatusType rv = E_OK;
	OsSpinlockType *sPtr;
	OsTaskVarType *pcbPtr;

	/** @req OS707 */
	if( !IsSpinlockValid(spinlock_id) ) {
		rv = E_OS_ID;
		goto err;
	}

	sPtr = Os_SpinlockGet(spinlock_id);

#if	(OS_APPLICATION_CNT > 1)
	/** @req OS710 */
	rv = Os_ApplHaveAccess( sPtr->accessingApplMask );
	if( rv != E_OK ) {
		goto err;
	}
#endif

	rv = GetSpinlockDeadlockCheck(spinlock_id);
	if (rv != E_OK)
		goto err;

	/** @req OS704, OS705 */
	*success = Os_TryToGetSpinlock(sPtr);

	if (*success == TRYTOGETSPINLOCK_SUCCESS) {
		pcbPtr = Os_SysTaskGetCurr();
		if (pcbPtr != NULL) {
			TAILQ_INSERT_TAIL(&pcbPtr->spinlockHead, sPtr, spinlockEntry);
		}
	}

	/** @req OS706 */
	SPINLOCK_STD_END;
}
