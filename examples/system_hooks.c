
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
/*
 * DESCRIPTION
 *   Hold all OSEK system hooks.
 */

/* ----------------------------[includes]------------------------------------*/

#include <stdint.h>
#include "Os.h"
#include <assert.h>

#include "Mcu.h"

/* ----------------------------[private define]------------------------------*/
#define ERROR_LOG_SIZE 1

//#define USE_LDEBUG_PRINTF	1
#include "debug.h"

/* ----------------------------[private macro]-------------------------------*/


/* ----------------------------[private typedef]-----------------------------*/

typedef struct ErrorEntry {
	StatusType 		error;
	OsErrorType     info;
	TaskType    	taskId;
	OsServiceIdType serviceId;
} ErrorEntryType;


typedef struct ErrorLog {
	int 			index;
	ErrorEntryType 	log[ERROR_LOG_SIZE];
} ErrorLogType;

struct LogBad {
	uint32_t param1;
	uint32_t param2;
	uint32_t param3;
	TaskType taskId;
	OsServiceIdType serviceId;
	StatusType error;
} LogBadType;


/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
ErrorLogType ErrorLog;
/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/

/**
 *
 * @param FatalError
 * @return
 */
ProtectionReturnType ProtectionHook( StatusType FatalError ) {
	printf("## ProtectionHook\n");
	return PRO_KILLAPPL;
}

/**
 *
 */
void StartupHook( void ) {
//	LDEBUG_PRINTF("## StartupHook\n");

	uint32_t sys_freq = McuE_GetSystemClock();
	(void)sys_freq;
	LDEBUG_PRINTF("Sys clock %d Hz\n",sys_freq);
}

/**
 *
 * @param Error
 */
void ShutdownHook( StatusType error ) {
	LDEBUG_FPUTS("## ShutdownHook\n");
	(void)error;
	while(1) {
		//err = err;
		;
	}
}

/**
 *
 * @param error
 */
void ErrorHook( StatusType error ) {

	TaskType task;
	static struct LogBad LogBad[ERROR_LOG_SIZE];
	static uint8_t ErrorCount = 0;

	GetTaskID(&task);


	OsServiceIdType service = OSErrorGetServiceId();

	/* Grab the arguments to the functions
	 * This is the standard way, see 11.2 in OSEK spec
	 */
	switch(service) {
	case OSServiceId_SetRelAlarm:
	{
		// Read the arguments to the faulty functions...
		AlarmType alarm_id = OSError_SetRelAlarm_AlarmId;
		TickType increment = OSError_SetRelAlarm_Increment;
		TickType cycle = OSError_SetRelAlarm_Cycle;
		(void)alarm_id;
		(void)increment;
		(void)cycle;

		// ... Handle this some way.
		break;
	}
	/*
	 * The same pattern as above applies for all other OS functions.
	 * See Os.h for names and definitions.
	 */

	default:
		break;
	}

	LDEBUG_PRINTF("## ErrorHook err=%u\n",Error);

	/* Log the errors in a buffer for later review */
	LogBad[ErrorCount].param1 = os_error.param1;
	LogBad[ErrorCount].param2 = os_error.param2;
	LogBad[ErrorCount].param3 = os_error.param3;
	LogBad[ErrorCount].serviceId = service;
	LogBad[ErrorCount].taskId = task;
	LogBad[ErrorCount].error = error;

	ErrorCount++;

	/* Keep compiler silent */
	(void)LogBad[ErrorCount].param1;

	// Stall if buffer is full.
	while(ErrorCount >= ERROR_LOG_SIZE);
}


/**
 *
 */
void PreTaskHook( void ) {
	StatusType rv;
	TaskType task;
	TaskStateType state;

	rv = GetTaskID(&task);
	assert( rv == E_OK );
	LDEBUG_PRINTF("## PreTaskHook, taskid=%d\n",task);
	rv = GetTaskState(task,&state);
	assert( rv == E_OK );
	assert( state == TASK_STATE_RUNNING );
}

/**
 *
 */
void PostTaskHook( void ) {
	StatusType rv;
	TaskType task;
	TaskStateType state;

	rv = GetTaskID(&task);
	assert( rv == E_OK );
	LDEBUG_PRINTF("## PostTaskHook, taskid=%d\n",task);
	rv = GetTaskState(task,&state);
	assert( rv == E_OK );
	assert( state == TASK_STATE_RUNNING );

}

