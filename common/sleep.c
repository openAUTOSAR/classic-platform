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
	imask_t state;
    Irq_Save(state);
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
    Irq_Restore(state);
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
