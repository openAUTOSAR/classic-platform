/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/


#include "Os.h"
#include "Mcu.h"
#include "arc.h"

//#define USE_LDEBUG_PRINTF // Uncomment this to turn debug statements on.
#include "debug.h"

// How many errors to keep in error log.
#define ERROR_LOG_SIZE 20


/**
 * Just an example of a basic task.
 */

void bTask3( void ) {
	StackInfoType si;
	TaskType currTask;
	LDEBUG_PRINTF("[%08u] bTask3 start\n", (unsigned)GetOsTick() );

	GetTaskID(&currTask);
	Os_Arc_GetStackInfo(currTask,&si);
	LDEBUG_PRINTF("bTask3: %u%% stack usage\n",
			(unsigned)OS_STACK_USAGE(&si));

	TerminateTask();
}

/**
 * An extended task is auto-started and is also triggered by an alarm
 * that sets event 2.
 */

void eTask1( void ) {
	volatile float tryFloatingPoint = 0.0F;
	StackInfoType si;
	TaskType currTask;

	LDEBUG_FPUTS("eTask1 start\n");

	ActivateTask(TASK_ID_eTask2);
	for(;;) {
		SetEvent(TASK_ID_eTask2,EVENT_MASK_Event2);
		WaitEvent(EVENT_MASK_Event1);
		ClearEvent(EVENT_MASK_Event1);
		tryFloatingPoint += 1.0F;
		GetTaskID(&currTask);
		Os_Arc_GetStackInfo(currTask,&si);
		LDEBUG_PRINTF("eTask1: %u%% stack usage\n",
				(unsigned)OS_STACK_USAGE(&si));

	}
}

/**
 * An extended task that receives events from someone
 * and activates task: bTask3.
 */
void eTask2( void ) {
	LDEBUG_FPUTS("eTask2 start\n");

	for(;;) {
		WaitEvent(EVENT_MASK_Event2);
		ClearEvent(EVENT_MASK_Event2);
		ActivateTask(TASK_ID_bTask3);
		{
			StackInfoType si;
			TaskType currTask;
			GetTaskID(&currTask);
			Os_Arc_GetStackInfo(currTask,&si);
			LDEBUG_PRINTF("eTask2: %u%% stack usage\n",
					(unsigned)OS_STACK_USAGE(&si));
		}
	}
}


/*
 * Functions that must be supplied by the example
 */

void OsIdle( void ) {
	for(;;) {}
}
