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

#ifndef SPINLOCK_I_H_
#define SPINLOCK_I_H_

#include "Os.h"
#include "internal.h"
#include "os_config_macros.h"

typedef struct OsSpinlock {
	SpinlockIdType id;
	char name[32];
	volatile unsigned int lock;
#if (OS_USE_APPLICATIONS == STD_ON)
	uint32 accessingApplMask;
#endif
	TAILQ_ENTRY(OsSpinlock) spinlockEntry;
} OsSpinlockType;


#if OS_SPINLOCK_CNT!=0
extern GEN_SPINLOCK_HEAD;
#endif

static inline OsSpinlockType *Os_SpinlockGet(SpinlockIdType id) {
#if OS_SPINLOCK_CNT!=0
	return &spinlock_list[id];
#else
	(void)id;
	return NULL;
#endif
}


void Os_ArchGetSpinlock( volatile unsigned int *);
TryToGetSpinlockType Os_ArchTryToGetSpinlock( volatile unsigned int *);
void Os_ArchReleaseSpinlock( volatile unsigned int *);

static inline void Os_GetSpinlock(OsSpinlockType *sPtr) {
	Os_ArchGetSpinlock(&sPtr->lock);
}

static inline TryToGetSpinlockType Os_TryToGetSpinlock(OsSpinlockType *sPtr) {
	return (TryToGetSpinlockType)Os_ArchTryToGetSpinlock(&sPtr->lock);
}

static inline void Os_ReleaseSpinlock(OsSpinlockType *sPtr) {
	Os_ArchReleaseSpinlock(&sPtr->lock);
}

#endif /*SPINLOCK_I_H_*/
