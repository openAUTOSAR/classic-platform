
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
#ifndef _WIN32
/* Not POSIX, but needed for debugging using pthread_setname_np */
#define __USE_GNU
#include <pthread.h>
#include <time.h>
#include <unistd.h> /* sleep */
#endif
#include "linos_logger.h" /* Logger functions */
#include "Os.h"
#include <os_sys.h>  /* OS_SYS_PTR */
#include <os_i.h>    /* GEN_TASK_HEAD  */
#include "os_main.h"

extern ThreadTaskType ThreadTasks[(OS_TASK_CNT+GNULINUX_TASK_CNT)]; /* Normal tasks and special GNULinux tasks. Declared in os_main.c  */

extern int GetTaskId (void); /* Declared in os_task_linos.c */

StatusType WaitEvent ( EventMaskType Mask ) {

    unsigned int usecs;
    unsigned int start_ticks, end_ticks, diff_ticks;
    TaskType CurrentTaskId;
#ifndef _WIN32
    struct timespec t_start, t_end, t_diff;
#endif
    CurrentTaskId = GetTaskId();

    if ( CurrentTaskId > -1)  {
        logger_mod((LOGGER_MOD_LINOS | LOGGER_SUB_OS_EVENT), LOG_INFO,
                "Started WaitEvent TaskId %s(%d)  Mask [0x%.8X] 32bits.",
                ThreadTasks[CurrentTaskId].name, CurrentTaskId, Mask);

#ifndef _WIN32

        clock_gettime(CLOCK_REALTIME, &t_start); // Get current time.

        start_ticks = OS_SYS_PTR->tick;

        pthread_mutex_lock(&ThreadTasks[CurrentTaskId].mutex_lock);

        int event_already_existed = 1;
        while ((ThreadTasks[CurrentTaskId].EventState & Mask) == 0) {
            event_already_existed = 0;
            pthread_cond_wait(&ThreadTasks[CurrentTaskId].cond, &ThreadTasks[CurrentTaskId].mutex_lock);
        }

        pthread_mutex_unlock(&ThreadTasks[CurrentTaskId].mutex_lock);

        clock_gettime(CLOCK_REALTIME, &t_end); // Get current time.
        end_ticks = OS_SYS_PTR->tick;
        t_diff = timesdiff(&t_start, &t_end);

        usecs = (t_diff.tv_nsec / 1000) + (t_diff.tv_sec * 1000000);

        diff_ticks = end_ticks - start_ticks;
        if (event_already_existed) {
            logger_mod((LOGGER_MOD_LINOS | LOGGER_SUB_OS_EVENT), LOG_INFO,
                    "Finished WaitEvent TaskId %s(%d). The event already existed.",
                    ThreadTasks[CurrentTaskId].name, CurrentTaskId);
        } else {
            logger_mod((LOGGER_MOD_LINOS | LOGGER_SUB_OS_EVENT), LOG_INFO,
                    "Finished WaitEvent TaskId %s(%d). Got event after %d ticks, %d ms.",
                    ThreadTasks[CurrentTaskId].name, CurrentTaskId, diff_ticks,
                    ((usecs) / 1000));
        }
#endif
    } else {
        logger(LOG_ERR, "WaitEvent. ERROR. Couldn't identify calling Task ID %d.", CurrentTaskId);
        return E_NOT_OK;
    }

    return E_OK;
}


StatusType GetEvent( TaskType TaskId, EventMaskRefType Mask){

    logger_mod((LOGGER_MOD_LINOS | LOGGER_SUB_OS_EVENT), LOG_INFO,
            "Started GetEvent TaskId %s(%d) [0x%.8X] 32bits",
            ThreadTasks[TaskId].name, TaskId, *Mask);
#ifndef _WIN32
    pthread_mutex_lock(&ThreadTasks[TaskId].mutex_lock);

    *Mask = ThreadTasks[TaskId].EventState;

    pthread_mutex_unlock(&ThreadTasks[TaskId].mutex_lock);
#endif
    logger_mod((LOGGER_MOD_LINOS | LOGGER_SUB_OS_EVENT), LOG_INFO,
            "Finished GetEvent TaskId %s(%d) [0x%.8X] 32bits",
            ThreadTasks[TaskId].name, TaskId, *Mask);

    return E_OK;
}

StatusType ClearEvent ( EventMaskType Mask ) {

    TaskType CurrentTaskId;

    const char same[] = "*** NO CHANGE ***";
    const char different[] = "";

    char *info;

    CurrentTaskId = GetTaskId();

    if ( CurrentTaskId > -1 ) {
        logger_mod((LOGGER_MOD_LINOS | LOGGER_SUB_OS_EVENT), LOG_INFO,
                    "Started ClearEvent TaskId %s(%d),  Mask [0x%.8X] 32bits.",
                    ThreadTasks[CurrentTaskId].name, CurrentTaskId, Mask);
#ifndef _WIN32
        pthread_mutex_lock(&ThreadTasks[CurrentTaskId].mutex_lock);
        EventMaskType OldMask = ThreadTasks[CurrentTaskId].EventState;

        ThreadTasks[CurrentTaskId].EventState &= ~(Mask);

        /* Get the newMask to avoid locking the EventState while locking. */
        EventMaskType newMask = ThreadTasks[CurrentTaskId].EventState;

        pthread_mutex_unlock(&ThreadTasks[CurrentTaskId].mutex_lock);


        if ( OldMask == newMask)
            info = (char*)&same;
        else
            info = (char*)&different;

        logger_mod((LOGGER_MOD_LINOS | LOGGER_SUB_OS_EVENT), LOG_INFO,
                "Finished ClearEvent TaskId %s(%d) Mask [0x%.8X]. [0x%.8X] -> [0x%.8X] 32bits. %s",
                ThreadTasks[CurrentTaskId].name, CurrentTaskId, Mask, OldMask,
                newMask, info);
#endif
    } else {
        logger(LOG_ERR, "ClearEvent. ERROR. Couldn't identify calling Task ID.");
        return E_NOT_OK;
    }

    return E_OK;

}


StatusType SetEvent( TaskType TaskId, EventMaskType Mask ){

#ifndef _WIN32
    logger_mod((LOGGER_MOD_LINOS | LOGGER_SUB_OS_EVENT), LOG_INFO,
            "Started SetEvent TaskId %s(%d),  Mask [0x%.8X] 32bits.",
            ThreadTasks[TaskId].name, TaskId, Mask);

    pthread_mutex_lock(&ThreadTasks[TaskId].mutex_lock);

    ThreadTasks[TaskId].EventState |= Mask;

    /* Notify the waiting thread (WaitEvent) */
    pthread_cond_signal(&ThreadTasks[TaskId].cond);
    pthread_mutex_unlock(&ThreadTasks[TaskId].mutex_lock);

    logger_mod((LOGGER_MOD_LINOS | LOGGER_SUB_OS_EVENT), LOG_INFO,
            "Finished SetEvent TaskId %s(%d),  Mask [0x%.8X] 32bits.",
            ThreadTasks[TaskId].name, TaskId, Mask);
#endif
    return E_OK;
}

