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

#ifndef ALARM_I_H_
#define ALARM_I_H_

#include "counter_i.h"

/* STD container : OsAlarmSetEvent
 * OsAlarmSetEventRef:			1
 * OsAlarmSetEventTaskRef:		1
 * */

/* STD container : OsAlarmActivateTask
 * OsAlarmActivateTaskRef:		1
 * */

/* STD container : OsAlarmCallback
 * OsAlarmCallbackName:			1    Function Ptr
 * */

/* STD container : OsAlarmIncrementCounter
 * OsAlarmIncrementCounterRef:	1    Ref to counter
 * */


struct OsCounter;

typedef enum alarm_action_type_e {
	ALARM_ACTION_ACTIVATETASK=0,
	ALARM_ACTION_SETEVENT,
	ALARM_ACTION_ALARMCALLBACK,		/* Only class 1 */
	ALARM_ACTION_INCREMENTCOUNTER/* SWS OS302 */
} alarm_action_type_t;


/* STD container : OsAlarmAction
 * OsAlarmActivateTask:			0..1
 * OsAlarmCallback:				0..1
 * OsAlarmIncrementCounter:		0..1
 * OsAlarmSetEvent:				0..1
 * */
typedef struct OsAlarmAction {
	alarm_action_type_t type;
	TaskType 			task_id;
	EventMaskType 		event_id;
	CounterType 		counter_id;
} OsAlarmActionType;


enum OsAlarmAutostartTypeType {
	// Start with SetAbsAlarm()
	ALARM_AUTOSTART_ABSOLUTE,
	// Start with SetRelAlarm()
	ALARM_AUTOSTART_RELATIVE
};


/* STD container : OsAlarmAutostart
 * OsAlarmAlarmTime: 	 		1    Int
 * OsAlarmAutoStartType: 		1    Int, ABSOLUTE, RELATIVE
 * OsAlarmCycleTime: 	 		1    Int
 * OsAlarmAppModeRef: 	 		1..*
 */
typedef struct OsAlarmAutostart {
	uint32 alarmTime;
	enum OsAlarmAutostartTypeType autostartType;
	uint32 cycleTime;
	AppModeType appModeRef;
} OsAlarmAutostartType;

/* STD container : OsAlarm
 * OsAlarmAccessionApplication: 0..* Ref to OS application
 * OsAlamCounterRef:            1    Ref to counter
 * OsAlarmAction[C]             1    Action when alarm expires
 * OsAlarmAutostart[C]          0..1 Autostart
 */
typedef struct OsAlarm {
	char 	name[16];
	/* Reference to counter */
	struct OsCounter *counter;

	CounterType counter_id;
	/* cycle, 0 = no cycle */
	uint32 alarmtime;
	uint32 cycletime;
//	uint32 appmode_mask;

	uint32 app_mask;

	/* if the alarm is active or not */
	_Bool 	active;
	/* expire value */
	uint32 	expire_val;
	/* Action attributes when alarm expires. */
	OsAlarmActionType action;

	/* Autostart */
	const struct OsAlarmAutostart *autostartPtr;

	/* List of alarms connected to the same counter */
	SLIST_ENTRY(OsAlarm) alarm_list;

#if (OS_SC1 == STD_ON)
	/* TODO: OS242, callback in scalability class 1 only..*/
	/** Missing req OS242 */
#endif

} OsAlarmType;


void Os_AlarmCheck(OsCounterType *c_p);
void Os_AlarmAutostart(void);

#endif /*ALARM_I_H_*/
