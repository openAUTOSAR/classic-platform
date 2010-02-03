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

/** @req OS327 */
#if (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)
#if (OS_STATUS_STANDARD == STD_ON )
#error req OS327 not fullfilled
#endif
#endif


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
#if (  OS_SC3 == STD_ON) || (  OS_SC4==STD_ON)
int Oil_GetApplCnt(void) {
	return APPLICATION_CNT;
}

OsRomApplicationType *Oil_GetApplObj( ApplicationType application_id ) {
	return &rom_app_list[application_id];
}
#endif

/*-----------------------------------------------------------------*/
int Oil_GetTaskCnt(void) {
	return TASK_CNT;
}
/*-----------------------------------------------------------------*/

OsResourceType *Oil_GetResource( ResourceType resource ) {
	return &resource_list[resource];
}

int Oil_GetResourceCnt() {
	return RESOURCE_CNT;
}

/*-----------------------------------------------------------------*/

OsCounterType *Oil_GetCounter(CounterType count_id) {
	return &counter_list[count_id];
}

uint32 Oil_GetCounterCnt(void ) {
	return COUNTER_CNT;
//	return sizeof(counter_list)/sizeof(OsCounterType);
}
/*-----------------------------------------------------------------*/

uint32 Oil_GetSchedCnt(  void ) {
	return SCHEDULETABLE_CNT;
}

OsSchTblType *Oil_GetSched( ScheduleTableType sched_id ) {
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

OsAlarmType *Oil_GetAlarmObj( AlarmType alarm_id ) {
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

#if 0
OsMessageType *Oil_GetMessage(MessageType message_id) {
#if MESSAGE_CNT!=0
	return &message_list[message_id];
#else
	return NULL;
#endif
}

uint32 Oil_GetMessageCnt(void ) {
	return MESSAGE_CNT;
}
#endif

/*-----------------------------------------------------------------*/

#if (  OS_SC3 == STD_ON) || (  OS_SC4 == STD_ON)
uint32 Oil_GetServiceCnt( void ) {
	return SERVICE_CNT;
}
#endif


/*-----------------------------------------------------------------*/

void Oil_GetInterruptStackInfo( OsStackType *stack ) {
	stack->top = os_interrupt_stack;
	stack->size = sizeof(os_interrupt_stack);
}





