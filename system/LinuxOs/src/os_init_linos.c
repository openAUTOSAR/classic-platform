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

#include "linos_logger.h" /* Logger functions */
#include "Os.h"
#include <os_sys.h>  /* OS_SYS_PTR */
#include <os_i.h>    /* GEN_TASK_HEAD  */
#include "os_main.h"

extern ThreadTaskType ThreadTasks[(OS_TASK_CNT+GNULINUX_TASK_CNT)]; /* Normal tasks and special GNULinux tasks. Declared in os_main.c  */

static void copyPcbParts( OsTaskVarType *pcb, const OsTaskConstType *r_pcb ) {
        pcb->activePriority = r_pcb->prio;
        pcb->stack= r_pcb->stack;
        pcb->constPtr = r_pcb;
}


void InitOS( void ) {

    uint32_t i;
    OsTaskVarType *tmpPcbPtr = NULL;

    logger(LOG_INFO, "InitOS called.\n");

#if OS_COUNTER_CNT!=0
                Os_CounterInit();
#endif
#if OS_SCHTBL_CNT!=0
                Os_SchTblInit(); // IMPROVEMENT SimonG ??
#endif
#if (OS_NUM_CORES > 1)
                IocInit(); // IMPROVEMENT SimonG ??
#endif


    for( i=0; i < OS_TASK_CNT; i++) {
                tmpPcbPtr = Os_TaskGet((TaskType)i);
#if (OS_NUM_CORES > 1)
                if (Os_OnRunningCore(OBJECT_TASK,i)) {
#endif
                    copyPcbParts(tmpPcbPtr,&Os_TaskConstList[i]);
                    logger(LOG_INFO,"InitOS adding task pid:%d name:%s prio:%d",tmpPcbPtr->constPtr->pid,tmpPcbPtr->constPtr->name,tmpPcbPtr->activePriority);

                    // Fill Simons struct !!
                    ThreadTasks[i].pid = tmpPcbPtr->constPtr->pid;
                    ThreadTasks[i].entry = tmpPcbPtr->constPtr->entry;
                    ThreadTasks[i].name = tmpPcbPtr->constPtr->name;
                    ThreadTasks[i].prio = tmpPcbPtr->activePriority;
                    ThreadTasks[i].autostart = tmpPcbPtr->constPtr->autostart;
                    //

#if (OS_NUM_CORES > 1)
                }
#endif

#if 0
                Os_AddTask(tmpPcbPtr);
#endif
        }

    /* GNULINUX  tasks GNULINUX_TASK_CNT */

    i = TASK_ID_GNULINUX_timer;
    //ThreadTasks[i].pid = TASK_ID_GNULINUX_timer;
    ThreadTasks[i].entry = gnulinux_timer;
    ThreadTasks[i].name = "TickDriveTimer";
    ThreadTasks[i].prio = 1;
    ThreadTasks[i].autostart = TRUE;

    i = TASK_ID_GNULINUX_pmc_sync_status_reader;
    //ThreadTasks[i].pid = TASK_ID_GNULINUX_pmc_sync_status_reader;
    ThreadTasks[i].entry = gnulinux_pmc_sync_status_reader;
    ThreadTasks[i].name = "SyncStatusReader";
    ThreadTasks[i].prio = 1;
    ThreadTasks[i].autostart = TRUE;
}
