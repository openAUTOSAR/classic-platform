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
 * This file only be use with the Os_Cfg.c file and os_config_macros.h
 */
#include "kernel.h"
#include "Os_Cfg.h"

// COUNTER, RESOURCE, TSAK, must be at least 1
#define COUNTER_CNT				ARRAY_SIZE(counter_list)
#define RESOURCE_CNT			ARRAY_SIZE(resource_list)
#define TASK_CNT				ARRAY_SIZE(rom_pcb_list)

#if defined(ALARM_USE)
#define ALARM_CNT				ARRAY_SIZE(alarm_list)
#else
#define ALARM_CNT				0
#endif

#if defined(SCHEDULETABLE_USE)
#define SCHEDULETABLE_CNT		ARRAY_SIZE(sched_list)
#else
#define SCHEDULETABLE_CNT		0
#endif

#if defined(MESSAGE_CNT)
#undef MESSAGE_CNT
#define MESSAGE_CNT				ARRAY_SIZE(message_list)
#else
#define MESSAGE_CNT				0
#endif

#if defined(EVENT_CNT)
#undef EVENT_CNT
#define EVENT_CNT				ARRAY_SIZE(event_list)
#else
#define EVENT_CNT				0
#endif

#if defined(SERVICE_CNT)
#undef SERVICE_CNT
#define SERVICE_CNT				ARRAY_SIZE(os_cfg_trusted_list)
#else
#define SERVICE_CNT				0
#endif

os_error_t os_error;

//-------------------------------------------------------------------


/*
 * Accessor functions for os_config.c
 */

#if 0
#if SERVICE_CNT!=0
trusted_func_t oil_trusted_func_list[SERVICE_CNT];
#endif
#endif

/*-----------------------------------------------------------------*/
int Oil_GetApplCnt(void) {
	return APPLICATION_CNT;
}

rom_app_t *Oil_GetApplObj( ApplicationType application_id ) {
	return &rom_app_list[application_id];
}

/*-----------------------------------------------------------------*/
int Oil_GetTaskCnt(void) {
	return TASK_CNT;
}
/*-----------------------------------------------------------------*/

resource_obj_t *Oil_GetResource( ResourceType resource ) {
	return &resource_list[resource];
}

int Oil_GetResourceCnt() {
	return RESOURCE_CNT;
}

/*-----------------------------------------------------------------*/

counter_obj_t *Oil_GetCounter(CounterType count_id) {
	return &counter_list[count_id];
}

uint32 Oil_GetCounterCnt(void ) {
	return COUNTER_CNT;
//	return sizeof(counter_list)/sizeof(counter_obj_t);
}
/*-----------------------------------------------------------------*/

uint32 Oil_GetSchedCnt(  void ) {
	return SCHEDULETABLE_CNT;
}

sched_table_t *Oil_GetSched( ScheduleTableType sched_id ) {
#if defined(SCHEDULETABLE_USE)
	if(sched_id < SCHEDULETABLE_CNT) {
		return &sched_list[sched_id];
	} else {
		return NULL;
	}
#else
	return NULL;
#endif
}

/*-----------------------------------------------------------------*/

uint32 Oil_GetAlarmCnt(void) {
	return ALARM_CNT;
}

alarm_obj_t *Oil_GetAlarmObj( AlarmType alarm_id ) {
#if defined(ALARM_USE)
	if( alarm_id < ALARM_CNT) {
	  return &alarm_list[alarm_id];
	} else {
		return NULL;
	}
#else
	return NULL;
#endif
}

StatusType Oil_GetAlarmBase(AlarmType alarm_id, AlarmBaseRefType info) {

	StatusType rv = E_OK;

	if( alarm_id >= Oil_GetAlarmCnt() ) {
		rv = E_OS_ID;
	}
#if defined(ALARM_USE)
	*info = alarm_list[alarm_id].counter->alarm_base;
#endif
	return rv;
}


/*-----------------------------------------------------------------*/

message_obj_t *Oil_GetMessage(MessageType message_id) {
#if MESSAGE_CNT!=0
	return &message_list[message_id];
#else
	return NULL;
#endif
}

uint32 Oil_GetMessageCnt(void ) {
	return MESSAGE_CNT;
}

/*-----------------------------------------------------------------*/

uint32 Oil_GetServiceCnt( void ) {
	return SERVICE_CNT;
}


/*-----------------------------------------------------------------*/

void Oil_GetInterruptStackInfo( stack_t *stack ) {
	stack->top = os_interrupt_stack;
	stack->size = sizeof(os_interrupt_stack);
}





