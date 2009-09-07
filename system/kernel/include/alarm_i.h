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

struct counter_obj_s;

typedef enum alarm_action_type_e {
	ALARM_ACTION_ACTIVATETASK=0,
	ALARM_ACTION_SETEVENT,
	ALARM_ACTION_ALARMCALLBACK,		/* Only class 1 */
	ALARM_ACTION_INCREMENTCOUNTER, /* SWS OS302 */
} alarm_action_type_t;


typedef struct alarm_action_s {
	alarm_action_type_t type;
	TaskType 			task_id;
	EventMaskType 		event_id;
	CounterType 		counter_id;
} alarm_action_t;

typedef struct alarm_autostart_s {
	_Bool  active;
	uint32 alarmtime;
	uint32 cycletime;
	uint32 appmode_mask;
} alarm_autostart_t;

typedef struct alarm_obj_s {

	char name[16];
	/* Reference to counter */
	struct counter_obj_s *counter;

	CounterType counter_id;
	/* cycle, 0 = no cycle */
	uint32 alarmtime;
	uint32 cycletime;
	uint32 appmode_mask;

	alarm_autostart_t autostart;

	uint32 app_mask;

	/* if the alarm is active or not */
	_Bool active;
	/* expire value */
	uint32 expire_val;


	// Action attributes when alarm expires.
	alarm_action_t action;
/*
	alarm_action_type_t action_type;
	TaskType 			action_pid;
	EventMaskType 		action_event;
	CounterType 		action_counter;
*/
	/* List of alarms connected to the same counter */
	SLIST_ENTRY(alarm_obj_s) alarm_list;
	/* TODO: OS242, callback in scalability class 1 only..*/
#if 0
	void (*cb)(void);
#endif
} alarm_obj_t;



#endif /*ALARM_I_H_*/
