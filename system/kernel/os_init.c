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

/* ----------------------------[includes]------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "Os.h"
#include "internal.h"
#include "arc.h"
#include "debug.h"
#include "task_i.h"
#include "sys.h"
#include "isr.h"
#include "counter_i.h"
#include "application.h"
#include "sched_table_i.h"
#if (OS_NUM_CORES > 1)
#include "spinlock_i.h"
#endif
#include "alarm_i.h"
#include "arch.h"
#include "multicore_i.h"
#include "Mcu.h"
/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/
#define OS_VALIDATE(_a,_b)   if((_a)!=(_b) ) { \
								assert(#_a  #_b); \
							  }
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
Os_SysType Os_Sys[OS_NUM_CORES];

Os_IntCounterType Os_IntDisableAllCnt;
Os_IntCounterType Os_IntSuspendAllCnt;
Os_IntCounterType Os_IntSuspendOsCnt;

OsErrorType os_error;

/* ----------------------------[private functions]---------------------------*/


static void Os_CfgValidate(void ) {
#if (OS_COUNTER_CNT!=0)
	OS_VALIDATE(OS_COUNTER_CNT,ARRAY_SIZE(counter_list));
#endif
#if (OS_RESOURCE_CNT!=0)
	OS_VALIDATE(OS_RESOURCE_CNT,ARRAY_SIZE(resource_list));
#endif
	OS_VALIDATE(OS_TASK_CNT ,ARRAY_SIZE( Os_TaskConstList));
#if (OS_ALARM_CNT!=0)
	OS_VALIDATE(OS_ALARM_CNT,ARRAY_SIZE(alarm_list));
#endif
#if (OS_SCHTBL_CNT!=0)
	OS_VALIDATE(OS_SCHTBL_CNT, ARRAY_SIZE(sched_list));
#endif
#if (OS_SPINLOCK_CNT!=0)
#if (OS_NUM_CORES > 1)
	OS_VALIDATE(OS_SPINLOCK_CNT, ARRAY_SIZE(spinlock_list));
#endif
#endif
}

/* ----------------------------[public functions]----------------------------*/

extern uint32_t McuE_GetSystemClock( void );


/**
 * Copy rom pcb data(r_pcb) to ram data
 *
 * @param 	pcb		ram data
 * @param 	r_pcb	rom data
 */

static void copyPcbParts( OsTaskVarType *pcb, const OsTaskConstType *r_pcb ) {
	assert(r_pcb->prio<=OS_TASK_PRIORITY_MAX);
	pcb->activePriority = r_pcb->prio;
	pcb->stack= r_pcb->stack;
	pcb->constPtr = r_pcb;
}

//static _Bool init_os_called = 0;

/**
 * Initialization of kernel structures and start of the first
 * task.
 */

void InitOS( void ) {
	int i;
	OsTaskVarType *tmpPcbPtr;
	OsIsrStackType intStack;

	Os_CfgValidate();

	DEBUG(DEBUG_LOW,"os_init");

	/* Clear sys */
	memset(OS_SYS_PTR,0,sizeof(Os_SysType));

	OS_SYS_PTR->status.init_os_called = true;

	Os_ArchInit();

	/* Get the numbers defined in the editor */
	OS_SYS_PTR->isrCnt = OS_ISR_CNT;

	// Assign pcb list and init ready queue
	OS_SYS_PTR->pcb_list = Os_TaskVarList;
	TAILQ_INIT(& (OS_SYS_PTR->ready_head));
//	TAILQ_INIT(& OS_SYS_PTR->pcb_head);
#if defined(USE_KERNEL_EXTRA)
	TAILQ_INIT(& OS_SYS_PTR->timerHead);
#endif

	// Calc interrupt stack
	Os_IsrGetStackInfo(&intStack);
	// TODO: 16 is arch dependent
	OS_SYS_PTR->intStack = (void *)((size_t)intStack.top + (size_t)intStack.size - 16);

	// Init counter.. with alarms and schedule tables
	// Master core will initialize for all cores
	if (GetCoreID() == OS_CORE_ID_MASTER) {
#if OS_COUNTER_CNT!=0
		Os_CounterInit();
#endif
#if OS_SCHTBL_CNT!=0
		Os_SchTblInit();
#endif
#if (OS_NUM_CORES > 1)
		IocInit();
#endif
	}

	// Put all tasks in the pcb list
	// Put the one that belong in the ready queue there
	// TODO: we should really hash on priority here to get speed, but I don't care for the moment
	// TODO: Isn't this just EXTENED tasks ???
	for( i=0; i < OS_TASK_CNT; i++) {
		tmpPcbPtr = Os_TaskGet(i);
#if (OS_NUM_CORES > 1)
		if (Os_OnRunningCore(OBJECT_TASK,i)) {
#endif
			copyPcbParts(tmpPcbPtr,&Os_TaskConstList[i]);
			Os_TaskContextInit(tmpPcbPtr);
			TAILQ_INIT(&tmpPcbPtr->resourceHead);
			TAILQ_INIT(&tmpPcbPtr->spinlockHead);
			DEBUG(DEBUG_LOW,"pid:%d name:%s prio:%d\n",tmpPcbPtr->constPtr->pid,tmpPcbPtr->constPtr->name,tmpPcbPtr->activePriority);
#if (OS_NUM_CORES > 1)
		}
#endif

#if 0
		Os_AddTask(tmpPcbPtr);
#endif

		DEBUG(DEBUG_LOW,"pid:%d name:%s prio:%d\n",tmpPcbPtr->constPtr->pid,tmpPcbPtr->constPtr->name,tmpPcbPtr->activePriority);
	}

	Os_ResourceInit();

	// Now all tasks should be created.
}
volatile boolean beforeStartHooks[OS_NUM_CORES] = {false};
volatile boolean afterStartHooks[OS_NUM_CORES] = {false};
volatile boolean afterIsrSetup[OS_NUM_CORES] = {false};


#if (OS_NUM_CORES > 1)
/**
 * TODO: This will of course not work in a cached
 * system at all....
 *
 *
 * @param syncedCores
 */
static void syncCores(volatile boolean syncedCores[]) {
	CoreIDType coreId = GetCoreID();
	assert(coreId < OS_NUM_CORES && coreId >= 0);
	boolean syncOk = false;
	assert(syncedCores[coreId] == false);
	syncedCores[coreId] = true;
	while (!syncOk) {
		syncOk = true;
		for (int i = 0; i < OS_NUM_CORES; i++) {
			if (syncedCores[i] == false) {
				syncOk = false;
			}
		}
	}
}
#endif

static void os_start( void ) {
	uint16_t i;
	OsTaskVarType *tmpPcbPtr = NULL;

	// We will be setting up interrupts,
	// but we don't want them to fire just yet
	Irq_Disable();
	assert(OS_SYS_PTR->status.init_os_called);

	/* TODO: fix ugly */
	/* Call the startup hook */
#if (OS_NUM_CORES > 1)
	syncCores(beforeStartHooks);
#endif
	extern struct OsHooks os_conf_global_hooks;
	OS_SYS_PTR->hooks = &os_conf_global_hooks;
	if( OS_SYS_PTR->hooks->StartupHook!=NULL ) {
		OS_SYS_PTR->hooks->StartupHook();
	}
#if (OS_NUM_CORES > 1)
	syncCores(afterStartHooks);
#endif

#if	(OS_USE_APPLICATIONS == STD_ON)
	/* Start applications */
	Os_ApplStart();
#endif


	/* Alarm autostart */
#if OS_ALARM_CNT!=0
	Os_AlarmAutostart();
#endif

#if OS_SCHTBL_CNT!=0
	Os_SchTblAutostart();
#endif

	// Set up the systick interrupt
	{
		Os_SysTickInit();
#if (OS_NUM_CORES > 1)
		Os_CoreNotificationInit();
#endif
		if (GetCoreID() == OS_CORE_ID_MASTER) {
			uint32_t sys_freq = McuE_GetSystemClock();
			Os_SysTickStart(sys_freq/OsTickFreq);
		}
	}
#if (OS_NUM_CORES > 1)
	/* This is not an autosar req, but cores need to be synchronnized here because
	 * the isr handler uses shared data*/
	syncCores(afterIsrSetup);
	/* Find highest Autostart task */
#endif
	{
		OsTaskVarType *iterPcbPtr;
		OsPriorityType topPrio = -1;
		// TODO_MC: only the master core has one idle task, we need one for each core
		for(i=0;i<OS_TASK_CNT;i++) {
			iterPcbPtr = Os_TaskGet(i);
			if (Os_OnRunningCore(OBJECT_TASK,iterPcbPtr->constPtr->pid)) {
				if(	iterPcbPtr->constPtr->autostart ) {
					if( iterPcbPtr->activePriority > topPrio ) {
						tmpPcbPtr = iterPcbPtr;
						topPrio = iterPcbPtr->activePriority;
					}
				}
			}
		}
#if 0
		OsTaskVarType *iterPcbPtr;
		TAILQ_FOREACH(iterPcbPtr,& OS_SYS_PTR->pcb_head,pcb_list) {
			if(	iterPcbPtr->constPtr->autostart ) {
				if( iterPcbPtr->activePriority > topPrio ) {
					tmpPcbPtr = iterPcbPtr;
					topPrio = iterPcbPtr->activePriority;
				}
			}
 		}
#endif
	}

	// Swap in prio proc.
	{
		// FIXME: Do this in a more structured way.. setting OS_SYS_PTR->currTaskPtr manually is not the way to go..
		OS_SYS_PTR->currTaskPtr = tmpPcbPtr;
#if	(OS_USE_APPLICATIONS == STD_ON)
		/* Set current application */
		OS_SYS_PTR->currApplId = tmpPcbPtr->constPtr->applOwnerId;
#endif

		// register this auto-start activation
		assert(tmpPcbPtr->activations < tmpPcbPtr->constPtr->activationLimit);
		tmpPcbPtr->activations++;

		// NOTE! We don't go for os_swap_context() here..
		// first arg(NULL) is dummy only
		Os_TaskSwapContextTo(NULL,tmpPcbPtr);
		// We should not return here
		assert(0);
	}
}


extern void EcuM_Init(void);


int main( void )
{
	EcuM_Init();
}

/**
 * Starts the OS
 *
 * @param Mode - Application mode to start in
 *
 */
void StartOS(AppModeType Mode) {
	OS_SYS_PTR->status.os_started = true;

	OS_SYS_PTR->appMode = Mode;

	Os_CfgValidate();

	os_start();

	/** @req OS424 */
	assert(0);
}

/**
 * OS shutdown
 *
 * @param Error - Reason for shutdown
 */

/** @req OS071 */
void ShutdownOS( StatusType Error ) {

	if( OS_SYS_PTR->hooks->ShutdownHook != NULL ) {
		OS_SYS_PTR->hooks->ShutdownHook(Error);
	}

	Irq_Disable();
	/** @req OS425 */
	while(1) {	}

}



