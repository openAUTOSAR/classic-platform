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

#ifndef _OS_MAIN_H
#define _OS_MAIN_H

#ifndef _WIN32
/* Not POSIX, but needed for debugging using pthread_setname_np */
#define __USE_GNU
#include <pthread.h>
#endif

#include <setjmp.h>
#ifdef _WIN32
#include "os_stubs.h"
#endif
/*
 * Main header 
 *
*/


Os_SysType Os_Sys[OS_NUM_CORES];
Os_IntCounterType Os_IntDisableAllCnt;
Os_IntCounterType Os_IntSuspendAllCnt;
Os_IntCounterType Os_IntSuspendOsCnt;

OsTaskVarType Os_TaskVarList[OS_TASK_CNT];


/* GNULINUX tasks/threads */
/* Trying not to crash with normal tasks. Take the next free task */
#define TASK_ID_GNULINUX_timer      ((TaskType)OS_TASK_CNT+0)
#define TASK_ID_GNULINUX_pmc_sync_status_reader TASK_ID_GNULINUX_timer+1
#define GNULINUX_TASK_CNT 2  /* Number of GNULINUX tasks. */
/*			*/

#define BIT_EVENT_FILE_NAME_SUFFIX "_event.txt"


/* pthread status */
#define GNULINUX_PTHREAD_EMPTY			0
#define GNULINUX_PTHREAD_STARTING 		1
#define GNULINUX_PTHREAD_FUNCTION_STARTED	2
#define GNULINUX_PTHREAD_FUNCTION_TERMINATED	3

/*	*/

#define PTP_DEVICE       "/dev/ptp0"

#ifndef CLOCK_INVALID
#define CLOCK_INVALID -1
#endif


/* Task meta data, the stuff we need anyway. Bits from config/stm32_stm3210c/Os_Cfg.c GEN_TASK_HEAD */
typedef struct {
    OsTaskidType    pid;
    pthread_t tid;	/* Linux pThread id type */
    pthread_mutex_t mutex_lock; /* Mutex lock for locking a thread running a WaitEvent */
    pthread_cond_t cond;  /* Condition variable for notifing changes to EventState */
    jmp_buf stack_env; /* pthread stack environment */
    uint8 pthread_status; /* Only start new thread if old thread has ended. */
    OsPriorityType  prio;
    const char *name;
    void (*entry)( void ); /* Function pointer */
    uint8 autostart;
    EventMaskType EventState; /* Something to hold Event bits for each task */
} ThreadTaskType;

#define handle_error_en(en, msg) \
    do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)


/*					*/

void gnulinux_timer(void);
struct timespec timesdiff(struct timespec *start, struct timespec *end);
int start_thread( int threadId );

void gnulinux_pmc_sync_status_reader(void);

#endif /* _OS_MAIN_H */
