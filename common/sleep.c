/*
 * sleep.c
 *
 *  Created on: 23 jun 2010
 *      Author: jcar
 */
#include "sleep.h"
#include "Mcu.h"
#include "Os.h"

struct timeoutlist_t{
	uint32_t timeout;
	boolean active;
	EventMaskType mask;
};
static struct timeoutlist_t timeoutlist[OS_TASK_CNT];

static uint32_t ticks = 0;

void SleepInit()
{
	uint32_t i;

	for(i=0;i<OS_TASK_CNT;i++)
	{
		timeoutlist[i].active = FALSE;
	}
}

void Sleep(uint32_t nofTicks, TaskType TaskID, EventMaskType Mask )
{
	uint32 pval = McuE_EnterCriticalSection();
	if(nofTicks == 0){
		nofTicks=1;
	}
    if(TaskID < OS_TASK_CNT){
        timeoutlist[TaskID].timeout = ticks + nofTicks;
        timeoutlist[TaskID].active = TRUE;
        timeoutlist[TaskID].mask = Mask;
    }else{
        /* Error */
    	ErrorHook(E_OS_LIMIT);
    }
	McuE_ExitCriticalSection(pval);
}


void SleepTask(void)
{
	uint32_t i;
	for(;;) {
		// Alarms every tick
		WaitEvent(EVENT_MASK_SLEEP_ALARM_TASK);
		ClearEvent(EVENT_MASK_SLEEP_ALARM_TASK);

		ticks++;

		for(i=0;i<OS_TASK_CNT;i++)
		{
			if((timeoutlist[i].active == TRUE) && (timeoutlist[i].timeout == ticks))
			{
				timeoutlist[i].active = FALSE;
				SetEvent(i,timeoutlist[i].mask);
			}
		}
	}

}
