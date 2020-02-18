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

#include "Os.h"
#include "timer.h"
#include "arc.h"
#if defined(CFG_SHELL)
#include "shell.h"
#endif

#include <stdio.h>
#include <string.h>

//#define USE_LDEBUG_PRINTF
#include "debug.h"

/* ----------------------------[private define]------------------------------*/
#define MAX_FUNCTION_CNT    16uL // has to be less that 255
#define MAX_NAME_CHAR_LEN   32uL
#define NO_ACTIVE_FUNCTION 0xFFu
#define MAX_VAL_INIT 0xFFFFFFFF
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/


#if defined CFG_PERF_FUNC
typedef struct StatTick_Type {
	TickType avg;
	TickType total;
	TickType min;
	TickType max;
}StatTickType;
#endif

typedef struct Perf_Info_S {
    /* Calculated load, 0% to 100% */
    uint8_t load;
    /* Number of times the function have been called */
    uint32_t invokedCnt;

    /* Max time in us that this task/isr have executed */
    TickType timeMax_us;
    TickType timeMin_us;
    /* The total time in us this task/isr have executed */
    TickType timeTotal_us;

    char name[MAX_NAME_CHAR_LEN];
//    char *name;

    /*
     * INTERNAL
     */

    TickType timePeriodTotal_us;

    /* For each entry 1 is added. For each exit 1 is subtracted
     * Normally this should be 0 or 1 */
    int8_t called;
	TickType timeStart;

#if defined CFG_PERF_FUNC
	/* Active PerfFuncIdx in the Task
	 * Default activeFunc=0xFF -> No active Function * */
	uint8_t activeFunc;

	TickType invokeTime; // Tick at which Perf_Prefunction is Called
	TickType prev_invokeTime; // Last time Tick at which Perf_Prefunction is Called
	TickType timeActive;
	uint32_t invokedcnt_trig;

	StatTickType activePeriod_us; // Period between pre_func - post_func: ONLY active period (when task was running)
	StatTickType invokePeriod_us; // Period between consecutive invocations of pre_func
	StatTickType entryExitPeriod_us; // Period between pre_func and post_func
#endif

} Perf_InfoType;

struct Perf_Sum {
    uint8_t loadIsr;
    uint8_t loadTask;
    uint8_t loadTotal;
    uint32  idleTaskTime_us;    /* Special for OS idle */
    uint32  taskTime_us;
    uint32  isrTime_us;
};


struct Perf_Total {
    struct Perf_Sum sinceStart;
    struct Perf_Sum lastPeriod;
    TickType timeTotal_us;      /* Total runtime of perf */
    TickType timeLastPeriod_us;
};





/* ----------------------------[private function prototypes]-----------------*/

#if defined(CFG_SHELL)
static int shellCmdTop(int argc, char *argv[] );
#endif

#if defined CFG_PERF_FUNC
static int shellCmdtop_func(int argc, char *argv[] );
#endif

/* ----------------------------[private variables]---------------------------*/

Perf_InfoType Perf_TaskTimers[OS_TASK_CNT];
Perf_InfoType Perf_IsrTimers[OS_ISR_MAX_CNT];
struct Perf_Total Perf_Stats;

#if defined CFG_PERF_FUNC
Perf_InfoType Perf_FunctionTimers[MAX_FUNCTION_CNT];
uint8 PERF_FUNCTION_CNT = 0;
#endif




uint32 nesting = 0;
static boolean initCalled = false;


#if defined CFG_PERF_FUNC
static ShellCmdT top_funcCmdInfo = {
		shellCmdtop_func,
		0,1,
		"top_func",
		"Stat about Function measurements (alpha version)\n",
	    " (*) Use Perf_PreFunctionHook; <Function>; Perf_PostFunctionHook; \n"
		"     Active Time[us]: Time for which function was active (excludes time for which function is preempted) \n"
		"     Invoke Period[us]: Time between two consecutive invocations\n"
		"     Entry-Exit Period[us]: Time between function entry and function exit (includes inactive time)\n"
		"     Global Variable \"Perf_FunctionTimers\" for complete stats \n"
		"     Average and Load are calculated over trigger period, and Min,Max over entire time\n"
	    "     Warning: No nested calls for Perf_FunctionHook\n"
		"     Warning: Higher CPU load\n",
		"     Warning: Supports functions in tasks only (no ISR)\n",
		{NULL}
};
#endif

#if defined(CFG_SHELL)
static ShellCmdT topCmdInfo = {
    shellCmdTop,
    0,1,
    "top",
    "top",
    "List CPU load\n"
    " (*) The total will never be 100% since calculation times\n"
    "     are floored and time between measuring points is not\n"
    "     measured.\n",
    {    NULL,NULL}
};
#endif

/* ----------------------------[private functions]---------------------------*/

#if defined(CFG_SHELL)
/*
 *
 * @param argc
 * @param argv
 * @return
 */
static int shellCmdTop(int argc, char *argv[] ) {
    Arc_PcbType pcb;
    uint32 totalPerc = 0;

    if(argc == 1 ) {
        puts("                  load                  stack\n");
        puts("Task name          [%]   invCnt max[us]  [%] \n");
        puts("--------------------------------------------------\n");
        for (int i = 0; i < OS_TASK_CNT; i++) {

            Os_Arc_GetTaskInfo(&pcb,i,OS_ARC_F_TASK_STACK);

            totalPerc += Perf_TaskTimers[i].load;
            printf("  %-16s %3d %8d %6d  %3d\n",Perf_TaskTimers[i].name,
                    Perf_TaskTimers[i].load,
                    Perf_TaskTimers[i].invokedCnt,
                    Perf_TaskTimers[i].timeMax_us,
                    pcb.stack.usageInPercent);
        }

        puts("\n");
        puts("                  load                   \n");
        puts("ISR name           [%]   invCnt  max[us] \n");
        puts("--------------------------------------------------\n");
        for (int i = 0; i < Os_Arc_GetIsrCount(); i++) {

            if( Perf_IsrTimers[i].name[0] == '\0' ) {
                Os_Arc_GetIsrInfo(&pcb,i);
                memcpy(Perf_IsrTimers[i].name,pcb.name,MAX_NAME_CHAR_LEN);
            }
            totalPerc += Perf_IsrTimers[i].load;
            printf("  %-16s %3d %8d %6d\n",Perf_IsrTimers[i].name,
                    Perf_IsrTimers[i].load,
                    Perf_IsrTimers[i].invokedCnt,
                    Perf_IsrTimers[i].timeMax_us );

        }

        printf("\nTotal%%: %d (*) See help\n\n",totalPerc);

    } else {
    }

    return 0;
}
#endif



#if defined CFG_PERF_FUNC
static int shellCmdtop_func(int argc, char *argv[] ){

	if(argc == 1 ) {
		puts("               		Active Time[uS] |  Invoke Period[uS]|  Entry-Exit Period[uS] | Load \n");
		puts("Function name  		avg   min   max |  avg   min   max  |  avg     min     max   | [%]  \n");
		puts("--------------------------------------------------------------------------------------\n");

		for (int i = 0; i < PERF_FUNCTION_CNT; i++) {

			printf("  %-16s     %5d %5d %5d| %5d %5d %5d | %5d  %5d  %5d    | %2d\n",Perf_FunctionTimers[i].name,
					Perf_FunctionTimers[i].activePeriod_us.avg,
					Perf_FunctionTimers[i].activePeriod_us.min,
					Perf_FunctionTimers[i].activePeriod_us.max,
					Perf_FunctionTimers[i].invokePeriod_us.avg,
					Perf_FunctionTimers[i].invokePeriod_us.min,
					Perf_FunctionTimers[i].invokePeriod_us.max,
					Perf_FunctionTimers[i].entryExitPeriod_us.avg,
					Perf_FunctionTimers[i].entryExitPeriod_us.min,
					Perf_FunctionTimers[i].entryExitPeriod_us.max,
					Perf_FunctionTimers[i].load);
		}


	} else {
	}

	return 0;
}


#endif
/* ----------------------------[public functions]----------------------------*/

/*
 *
 */
void Perf_Init(void) {
    Arc_PcbType pcb;
    if (initCalled) {
        return;
    }
#if defined(CFG_SHELL)
    SHELL_AddCmd(&topCmdInfo);
#endif
#if defined CFG_PERF_FUNC
	SHELL_AddCmd(&top_funcCmdInfo);
#endif
	Timer_Init();
	initCalled = true;

	for (int i = 0; i < OS_TASK_CNT; i++) {
		Os_Arc_GetTaskInfo(&pcb,i,OS_ARC_F_TASK_ALL);
		memcpy(Perf_TaskTimers[i].name,pcb.name,16);
#if defined CFG_PERF_FUNC
		Perf_TaskTimers[i].activeFunc = NO_ACTIVE_FUNCTION;
		PERF_FUNCTION_CNT = 0;
#endif
	}
	for (int i = 0; i < Os_Arc_GetIsrCount(); i++) {
		Os_Arc_GetIsrInfo(&pcb,i);
		memcpy(Perf_IsrTimers[i].name,pcb.name,16);
	}
}

/*
 *
 */
void Perf_Trigger(void) {
    TickType perfDiff_us;
    static TickType perfTimerLast = 0;

    /* Ignore first time */
    if (perfTimerLast == 0) {
        perfTimerLast = Timer_GetTicks();

        /* Check for late installed interrupts */
        Arc_PcbType pcb;

        for (int i = 0; i < Os_Arc_GetIsrCount(); i++) {
            Os_Arc_GetIsrInfo(&pcb,i);
            if( (Perf_IsrTimers[i].name[0] == '\0') && (pcb.name[0] != '\0') ) {
                memcpy(Perf_IsrTimers[i].name,pcb.name,16);
            }
        }

        return;
    }

    SuspendAllInterrupts();

    /* Diff from previous run */
    perfDiff_us = TIMER_TICK2US(Timer_GetTicks() - perfTimerLast);
    Perf_Stats.timeTotal_us += perfDiff_us;
    Perf_Stats.timeLastPeriod_us = perfDiff_us;

    perfTimerLast = Timer_GetTicks();

    /* Clear all the timers */
    Perf_Stats.lastPeriod.taskTime_us = 0;
    Perf_Stats.lastPeriod.isrTime_us = 0;
    Perf_Stats.sinceStart.taskTime_us = 0;
    Perf_Stats.sinceStart.isrTime_us = 0;

    Perf_Stats.lastPeriod.idleTaskTime_us = Perf_TaskTimers[0].timePeriodTotal_us;

    for (int i = 0; i < OS_TASK_CNT; i++) {
        /* Add period time to total time */
        Perf_TaskTimers[i].timeTotal_us += Perf_TaskTimers[i].timePeriodTotal_us;
        Perf_TaskTimers[i].load = Perf_TaskTimers[i].timePeriodTotal_us * 100 / perfDiff_us;

        /* Add the times for all tasks but the IDLE task */
        if( i != 0 ) {
            Perf_Stats.sinceStart.taskTime_us += Perf_TaskTimers[i].timeTotal_us;
            Perf_Stats.lastPeriod.taskTime_us += Perf_TaskTimers[i].timePeriodTotal_us;
        }

        Perf_TaskTimers[i].timePeriodTotal_us = 0;
    }

    Perf_Stats.sinceStart.idleTaskTime_us = Perf_TaskTimers[0].timeTotal_us;

    for (int i = 0; i < Os_Arc_GetIsrCount(); i++) {
        /* Add period time to total time */
        Perf_IsrTimers[i].timeTotal_us += Perf_IsrTimers[i].timePeriodTotal_us;
        Perf_IsrTimers[i].load = Perf_IsrTimers[i].timePeriodTotal_us * 100 / perfDiff_us;

        /* Add the times for all Isrs but the IDLE Isr */
        Perf_Stats.sinceStart.isrTime_us += Perf_IsrTimers[i].timeTotal_us;
        Perf_Stats.lastPeriod.isrTime_us += Perf_IsrTimers[i].timePeriodTotal_us;
        Perf_IsrTimers[i].timePeriodTotal_us = 0;

    }

    Perf_Stats.sinceStart.loadTask = Perf_Stats.sinceStart.taskTime_us * 100 / Perf_Stats.timeTotal_us;
    Perf_Stats.sinceStart.loadIsr = Perf_Stats.sinceStart.isrTime_us * 100 / Perf_Stats.timeTotal_us;
    Perf_Stats.sinceStart.loadTotal = Perf_Stats.sinceStart.loadTask + Perf_Stats.sinceStart.loadIsr;

    /* Update load this period */
    Perf_Stats.lastPeriod.loadTask = Perf_Stats.lastPeriod.taskTime_us * 100 / Perf_Stats.timeLastPeriod_us;
    Perf_Stats.lastPeriod.loadIsr = Perf_Stats.lastPeriod.isrTime_us * 100 / Perf_Stats.timeLastPeriod_us;
    Perf_Stats.lastPeriod.loadTotal = Perf_Stats.lastPeriod.loadTask + Perf_Stats.lastPeriod.loadIsr;


#if defined CFG_PERF_FUNC
	for (int i = 0; i < PERF_FUNCTION_CNT; i++){


		Perf_FunctionTimers[i].invokePeriod_us.avg = (Perf_FunctionTimers[i].invokePeriod_us.total/
						(Perf_FunctionTimers[i].invokedcnt_trig-1)); // -1 to compenset for not calculating for 1st cnt
		/* Subtract number of time pre_func was called but not the post_func for
		 * correct number of times the period was measured*/
		Perf_FunctionTimers[i].activePeriod_us.avg = Perf_FunctionTimers[i].activePeriod_us.total/
				(Perf_FunctionTimers[i].invokedcnt_trig-Perf_FunctionTimers[i].called);
		Perf_FunctionTimers[i].entryExitPeriod_us.avg = Perf_FunctionTimers[i].entryExitPeriod_us.total/
				(Perf_FunctionTimers[i].invokedcnt_trig-Perf_FunctionTimers[i].called);

		/* Load for this period		 */
		Perf_FunctionTimers[i].load = Perf_FunctionTimers[i].activePeriod_us.total*100 / Perf_Stats.timeLastPeriod_us;

		/*Clear Totals for new trigger*/
		Perf_FunctionTimers[i].activePeriod_us.total = 0;
		Perf_FunctionTimers[i].invokePeriod_us.total = 0;
		Perf_FunctionTimers[i].entryExitPeriod_us.total = 0;
		Perf_FunctionTimers[i].invokedcnt_trig = 0;
		Perf_FunctionTimers[i].prev_invokeTime = 0;
		Perf_FunctionTimers[i].timePeriodTotal_us=0;
		Perf_FunctionTimers[i].timeActive=0;
	}

#endif

	ResumeAllInterrupts();

}

#if !defined(CFG_T1_ENABLE)
/*
 *
 * @param isr
 */
void Os_PreIsrHook(ISRType isr) {
    Perf_IsrTimers[isr].invokedCnt++;
    Perf_IsrTimers[isr].called++;
    Perf_IsrTimers[isr].timeStart = Timer_GetTicks();

}

/*
 *
 * @param isr
 */
void Os_PostIsrHook(ISRType isr) {
    TickType diff;

    diff = TIMER_TICK2US(Timer_GetTicks() - Perf_IsrTimers[isr].timeStart);
    if (diff > Perf_IsrTimers[isr].timeMax_us) {
        Perf_IsrTimers[isr].timeMax_us = diff;
    }
    Perf_IsrTimers[isr].timePeriodTotal_us += diff;
    Perf_IsrTimers[isr].called--;
}
#else
#warning Perf.c cannot trace the interrupts when T1 is used.
#endif
/*
 *
 */
void PreTaskHook(void) {
    TaskType task;

    GetTaskID(&task);

    /* Task */
    Perf_TaskTimers[task].invokedCnt++;
    Perf_TaskTimers[task].called++;
    Perf_TaskTimers[task].timeStart = Timer_GetTicks();

#if defined CFG_PERF_FUNC
	/*Perf_Func
	 * If switching task during execution of perf_function	 */
	if ( Perf_TaskTimers[task].activeFunc < MAX_FUNCTION_CNT){
		Perf_FunctionTimers[Perf_TaskTimers[task].activeFunc].timeStart = Timer_GetTicks();
	}
#endif

}

/*
 *
 */
void PostTaskHook(void) {
    TaskType task;
    TickType diff;

    GetTaskID(&task);
    Perf_TaskTimers[task].called--;
    diff = TIMER_TICK2US(Timer_GetTicks() - Perf_TaskTimers[task].timeStart);
    if (diff > Perf_TaskTimers[task].timeMax_us) {
        Perf_TaskTimers[task].timeMax_us = diff;
    }
    Perf_TaskTimers[task].timePeriodTotal_us += diff;

#if defined CFG_PERF_FUNC
	/*Perf_Func
	 * If switching task during execution of perf_function	 */
	uint8 PerfFuncIdx;
	PerfFuncIdx = Perf_TaskTimers[task].activeFunc;
	if ( PerfFuncIdx < MAX_FUNCTION_CNT){
		TickType diff;
		/* */
		diff = (Timer_GetTicks() - Perf_FunctionTimers[PerfFuncIdx].timeStart);
		Perf_FunctionTimers[PerfFuncIdx].timeActive += diff;   		// Time since start in ticks

	}
#endif
}

#if defined CFG_PERF_FUNC
/* Logic: measurement of active time in function measurements
 * start timer when pre_func_hook called. reset timeActive (timeActive = active time between pre_func and post func)
 * when task is preemted by another task, in posttaskhook check if there is any function measurement active.
 * If function measurement is active when task is switched, then add time diff to timeActive (this is active time).
 * when this task is switched back to, check if there was any active function when this was switched from.
 * If there there was function active then, start time measurement for that function.
 * Note: No nested function hook calls allowed.
 * **/


/*
 * Optional API, add a name to the Perf_InfoType for usability.
 * @param PerfFuncIdx
 * @param PerfNamePtr
 * @param PerfNameLen
 */
void Perf_InstallFunctionName(uint8 PerfFuncIdx, char *PerfNamePtr, uint8 PerfNameLen) {

    if((PerfNameLen < MAX_NAME_CHAR_LEN) &&
            (PerfFuncIdx < MAX_FUNCTION_CNT))
    {
        for (uint8 i = 0; i < PerfNameLen; i++) {
            Perf_FunctionTimers[PerfFuncIdx].name[i] = PerfNamePtr[i];
        }
        /*Init default min value*/
        Perf_FunctionTimers[PerfFuncIdx].activePeriod_us.min = MAX_VAL_INIT;
        Perf_FunctionTimers[PerfFuncIdx].invokePeriod_us.min = MAX_VAL_INIT;
        Perf_FunctionTimers[PerfFuncIdx].entryExitPeriod_us.min = MAX_VAL_INIT;
        PERF_FUNCTION_CNT++;
    }
}

/*
 * Call before the functions for measurement is called.
 * @param PerfFuncIdx
 */
void Perf_PreFunctionHook(uint8 PerfFuncIdx) {
	TaskType task;
	TickType diff;

	GetTaskID(&task);

    if (PerfFuncIdx < MAX_FUNCTION_CNT) {
        Perf_FunctionTimers[PerfFuncIdx].invokedCnt++;
        Perf_FunctionTimers[PerfFuncIdx].invokedcnt_trig++;
        Perf_FunctionTimers[PerfFuncIdx].called++;
        Perf_FunctionTimers[PerfFuncIdx].timeStart = Timer_GetTicks(); // Called every preFunc and preTask(if active task)
		Perf_FunctionTimers[PerfFuncIdx].timeActive = 0; // Reset time spent in function active only!

		/*invoked_Period */
		Perf_FunctionTimers[PerfFuncIdx].invokeTime = Perf_FunctionTimers[PerfFuncIdx].timeStart; // Called at preFunc in OS_TICK
		if (Perf_FunctionTimers[PerfFuncIdx].invokedcnt_trig > 1){ // dont have prev value at 1st cnt
			diff= TIMER_TICK2US(Perf_FunctionTimers[PerfFuncIdx].invokeTime - Perf_FunctionTimers[PerfFuncIdx].prev_invokeTime);
			Perf_FunctionTimers[PerfFuncIdx].invokePeriod_us.total += diff;
			Perf_FunctionTimers[PerfFuncIdx].invokePeriod_us.min = MIN(diff,Perf_FunctionTimers[PerfFuncIdx].invokePeriod_us.min);
			Perf_FunctionTimers[PerfFuncIdx].invokePeriod_us.max = MAX(diff,Perf_FunctionTimers[PerfFuncIdx].invokePeriod_us.max);
		}
		Perf_FunctionTimers[PerfFuncIdx].prev_invokeTime = Perf_FunctionTimers[PerfFuncIdx].invokeTime;

		/* Active function*/
		if (Perf_TaskTimers[task].activeFunc == NO_ACTIVE_FUNCTION){
			Perf_TaskTimers[task].activeFunc = PerfFuncIdx; // Set current function as active function
		}
		else {
			/* ERROR!!
			 * If already an active function -> ignore this active function. We don't support nested function*/
		}
    }
}


/*
 * Call after the function for measurement has been executed.
 * @param PerfFuncIdx
 * No nested calls for perf_function
 */
void Perf_PostFunctionHook(uint8 PerfFuncIdx) {
    TickType diff;
	TaskType task;

	GetTaskID(&task);

    if (PerfFuncIdx < MAX_FUNCTION_CNT) {

    	/* Time spent during in function when active only. in ticks*/
        Perf_FunctionTimers[PerfFuncIdx].timeActive += Timer_GetTicks() - Perf_FunctionTimers[PerfFuncIdx].timeStart;
        diff = TIMER_TICK2US(Perf_FunctionTimers[PerfFuncIdx].timeActive);

        Perf_FunctionTimers[PerfFuncIdx].timePeriodTotal_us += diff; 	//Time since Last trigger in uS
        Perf_FunctionTimers[PerfFuncIdx].timeTotal_us += diff; 	//Time since begining in uS
        Perf_FunctionTimers[PerfFuncIdx].called--;

		/* time spent in function active period only*/
		Perf_FunctionTimers[PerfFuncIdx].activePeriod_us.total+= diff;
		Perf_FunctionTimers[PerfFuncIdx].activePeriod_us.min = MIN(diff, Perf_FunctionTimers[PerfFuncIdx].activePeriod_us.min);
		Perf_FunctionTimers[PerfFuncIdx].activePeriod_us.max = MAX(diff, Perf_FunctionTimers[PerfFuncIdx].activePeriod_us.max);

		/* entry_exit_period*/
		diff = TIMER_TICK2US(Timer_GetTicks() - Perf_FunctionTimers[PerfFuncIdx].invokeTime);
		Perf_FunctionTimers[PerfFuncIdx].entryExitPeriod_us.total+= diff;
		Perf_FunctionTimers[PerfFuncIdx].entryExitPeriod_us.min = MIN(diff, Perf_FunctionTimers[PerfFuncIdx].entryExitPeriod_us.min);
		Perf_FunctionTimers[PerfFuncIdx].entryExitPeriod_us.max = MAX(diff, Perf_FunctionTimers[PerfFuncIdx].entryExitPeriod_us.max);

		Perf_TaskTimers[task].activeFunc = NO_ACTIVE_FUNCTION; // Clear active function from task
    }
}
#endif
