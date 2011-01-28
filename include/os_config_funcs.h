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

#define OS_VALIDATE(_a,_b)   if((_a)!=(_b) ) { \
								assert(#_a  #_b); \
							  }




void Os_CfgValidate(void ) {
	OS_VALIDATE(OS_COUNTER_CNT,ARRAY_SIZE(counter_list));
#if (RESOURCE_CNT!=0)
	OS_VALIDATE(OS_RESOURCE_CNT,ARRAY_SIZE(resource_list));
#endif
	OS_VALIDATE(OS_TASK_CNT ,ARRAY_SIZE(rom_pcb_list));
#if (RESOURCE_CNT!=0)
	OS_VALIDATE(OS_ALARM_CNT,ARRAY_SIZE(alarm_list));
#endif
#if (OS_SCHTBL_CNT!=0)
	OS_VALIDATE(OS_SCHTBL_CNT, ARRAY_SIZE(sched_list));
#endif
}

OsErrorType os_error;

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
int Os_CfgGetApplCnt(void) {
	return APPLICATION_CNT;
}

OsRomApplicationType *Os_CfgGetApplObj( ApplicationType application_id ) {
	return &rom_app_list[application_id];
}
#endif

/*-----------------------------------------------------------------*/

OsResourceType *Os_CfgGetResource( ResourceType resource ) {
#if OS_RESOURCE_CNT!=0
	return &resource_list[resource];
#else
	return NULL;
#endif
}

/*-----------------------------------------------------------------*/

OsCounterType *Os_CfgGetCounter(CounterType count_id) {
	return &counter_list[count_id];
}

/*-----------------------------------------------------------------*/

OsSchTblType *Os_CfgGetSched( ScheduleTableType sched_id ) {
#if (OS_SCHTBL_CNT!=0)
	if(sched_id < OS_SCHTBL_CNT) {
		return &sched_list[sched_id];
	} else {
		return NULL;
	}
#else
	return NULL;
#endif
}

/*-----------------------------------------------------------------*/

OsAlarmType *Os_CfgGetAlarmObj( AlarmType alarm_id ) {
#if (OS_ALARM_CNT!=0)
	if( alarm_id < OS_ALARM_CNT) {
	  return &alarm_list[alarm_id];
	} else {
		return NULL;
	}
#else
	return NULL;
#endif
}

StatusType Os_CfgGetAlarmBase(AlarmType alarm_id, AlarmBaseRefType info) {

	StatusType rv = E_OK;

	if( alarm_id >= OS_ALARM_CNT ) {
		rv = E_OS_ID;
	} else {
#if (OS_ALARM_CNT!=0)
		*info = alarm_list[alarm_id].counter->alarm_base;
#endif
	}
	return rv;
}


/*-----------------------------------------------------------------*/

#if 0
OsMessageType *Os_CfgGetMessage(MessageType message_id) {
#if (OS_MESSAGE_CNT!=0)
	return &message_list[message_id];
#else
	return NULL;
#endif
}

uint32 Os_CfgGetMessageCnt(void ) {
	return OS_MESSAGE_CNT;
}
#endif

/*-----------------------------------------------------------------*/

#if (  OS_SC3 == STD_ON) || (  OS_SC4 == STD_ON)
uint32 Os_CfgGetServiceCnt( void ) {
	return SERVICE_CNT;
}
#endif


/*-----------------------------------------------------------------*/

void Os_CfgGetInterruptStackInfo( OsStackType *stack ) {
	stack->top = os_interrupt_stack;
	stack->size = sizeof(os_interrupt_stack);
}





