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
 * Os_Cfg.h
 *
 *  Created on: 2008-dec-22
 *      Author: mahi
 */

#ifndef OS_CFG_H_
#define OS_CFG_H_

/* os_config.h */

#define APPLICATION_ID_application_1	0
#define APPLICATION_CNT					1

#define TASK_ID_OsIdle					0
#define TASK_ID_etask_master			1
#define TASK_ID_etask_sup_l				2
#define TASK_ID_etask_sup_m				3
#define TASK_ID_etask_sup_h				4
#define TASK_ID_btask_sup_l				5
#define TASK_ID_btask_sup_m				6
#define TASK_ID_btask_sup_h				7

#define TASK_ID_os_tick					8

#if defined(USE_SIMPLE_PRINTF)
#define PRIO_STACK_SIZE		1024
#else
#define PRIO_STACK_SIZE		330
#endif

#define STACK_SIZE_OsIdle					ALIGN_16(PRIO_STACK_SIZE)
#define STACK_SIZE_etask_master				ALIGN_16(PRIO_STACK_SIZE)
#define STACK_SIZE_etask_sup_l				ALIGN_16(PRIO_STACK_SIZE)
#define STACK_SIZE_etask_sup_m				ALIGN_16(PRIO_STACK_SIZE)
#define STACK_SIZE_etask_sup_h				ALIGN_16(PRIO_STACK_SIZE)
#define STACK_SIZE_btask_sup_l				ALIGN_16(PRIO_STACK_SIZE)
#define STACK_SIZE_btask_sup_m				ALIGN_16(PRIO_STACK_SIZE)
#define STACK_SIZE_btask_sup_h				ALIGN_16(PRIO_STACK_SIZE)


//#define TASK_ID_isr_dec					3
//#define TASK_ID_isr_soft7				4

#define RES_ID_INT_1		1
#define RES_ID_EXT_1		2

#define COUNTER_ID_OsTick		0
// Driver all alarms
#define COUNTER_ID_soft_1		1
// Drives scheduletable 0
#define COUNTER_ID_soft_2		2

//
// ScheduleTables
//
#define SCHEDULE_TABLE_0		0
#define SCHEDULE_TABLE_1 		1


// NOT GENERATED( for test system only )
#define SYSTEM_COUNTER_PERIOD		100
#define SOFT_COUNTER_1_PERIOD		10
#define SOFT_COUNTER_2_PERIOD		100

#define 	SCHEDULETABLE_DURATION_1		10
#define 	SCHEDULETABLE_DURATION_2		5

#define ALARM_ID_c_sys_activate_btask_h				0
#define ALARM_ID_c_sys_1_setevent_etask_m			1
#define ALARM_ID_c_soft_1_setevent_etask_m			2
#define ALARM_ID_c_soft_1_inc_counter_2				3

// Don't start IDLE task...
#undef USE_IDLE_TASK

#define OS_INTERRUPT_STACK_SIZE				1024

// The object that can actually be 0 is here..rest in os_config_funcs.h

// Just define them if you want to use them.
#define ALARM_USE				1
#define SCHEDULETABLE_USE		1
#undef  MESSAGE_USE
#define EVENT_USE				1
#undef SERVICE_USE


#define EVENT_0	(1<<0)
#define EVENT_1	(1<<1)
#define EVENT_2 (1<<2)



#endif /* OS_CFG_H_ */
