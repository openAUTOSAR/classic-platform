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


#ifndef OS_BSEM_
#define OS_BSEM_

#include "Std_Types.h"
#include <sys/queue.h>

#define TICK_MAX 	UINT_MAX

typedef struct OsSem {
    sint32 count;
    STAILQ_HEAD(,OsTaskVar) taskHead;
} OsSemType;

#define SEM_LOCKED          0
#define SEM_UNLOCKED        1

#define BSEM_LOCKED         0       /* WaitSem() will go into WAITING state.*/
#define BSEM_UNLOCKED       1


StatusType Sleep( TickType ticks );

/**
 * @brief   Function to initialize a semaphore
 * @details
 * @param[in] semPtr        Pointer to semaphore to initialize
 * @param[in] initialCount  Initial count of the semaohore
 * @return
 */

StatusType  SemInit( OsSemType *semPtr, sint32 initialCount);
/**
 * @brief   Function that waits for a semaphore.
 * @details The count of the semaphore is decremented. If the count < 0 the
 *          caller is put in WAITING state. If the scheduler is locked
 *          (hold RES_SCHEDULER) the call will fail and returns E_NOT_OK.
 *
 * @param[in] semPtr    Pointer to the semaphore to wait on.
 * @param[in] tmo       Timeout in OS ticks.
 *                      A timeout of 0 can be used for polling the semaphore.
 *                      .
 * @retval E_OS_TIMEOUT
 */

StatusType  SemWaitTmo( OsSemType *semPtr, TickType tmo );


/**
 * @brief   Function that waits for a semaphore.
 * @details The count of the semaphore is decremented. If the count < 0 the
 *          caller is put in WAITING state. If the scheduler is locked
 *          (hold RES_SCHEDULER) the call will fail and returns E_NOT_OK.
 *
 * @param[in] semPtr    Pointer to the semaphore to wait on.
 *                      .
 * @retval E_OS_OK
 */
StatusType SemWait(OsSemType *semPtr);

/**
 * @brief   Function to signal a semaphore.
 *
 * @param[in] semPtr        Pointer to a semaphore.
 *                          .
 * @retval E_OS_VALUE       The first task that waits on the semaphore
 *                          is in suspended state.
 * @retval E_OS_VALUE       The semPtr passed was NULL_PTR.
 *
 */
StatusType  SemSignal(OsSemType *semPtr);

/**
 * @brief   Function to initialize a binary semaphore.
 *
 * @param[in] semPtr        Pointer to the semaphore to initialize
 * @param[in] initialCount  BSEM_LOCKED or BSEM_UNLOCKED
 * @return
 */
static inline StatusType BSemInit( OsSemType *semPtr, sint32 initialCount) {
	/* 0 - locked, WaitSem() will go into WAITING state.
	 * 1 - unlocked, WaitSem() will NOT go into WAITING state.
	 */
	ASSERT( (initialCount == BSEM_LOCKED) || (initialCount == BSEM_UNLOCKED) );

	return SemInit(semPtr, initialCount );
}


/**
 * @brief   Function to wait on a binary semaphore.
 *
 * @param[in] semPtr        Pointer to a semaphore
 * @param[in] tmo           Timeout in OS ticks
 *                          .
 * @retval E_OK             OK
 * @retval E_OS_TIMEOUT     Timeout
 */
static inline StatusType BSemWaitTmo( OsSemType *semPtr, TickType tmo ) {
	return SemWaitTmo(semPtr,tmo);
}

/**
 * @brief   Function that waits on a binray semaphore
 *
 * @param[in] semPtr        Pointer to a semaphore
 *                          .
 * @retval E_OK             OK
 * @retval E_OS_TIMEOUT     Timeout
 */

static inline StatusType BSemWait( OsSemType *semPtr ) {
	return	SemWait(semPtr);
}

StatusType BSemSignal( OsSemType *semPtr );

#endif /*OS_BSEM_*/
