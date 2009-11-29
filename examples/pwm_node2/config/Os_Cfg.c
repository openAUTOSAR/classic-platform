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
* Module vendor:  Autocore
* Module version: 1.0.0
* Specification: Autosar v3.0.1, Final
*
*/


// File generated on Tue Aug 18 13:49:03 CEST 2009

#include <stdlib.h>
#include <stdint.h>
#include "os_config_macros.h"
#include "Platform_Types.h"
#include "Os.h"				// includes Os_Cfg.h
#include "Kernel.h"
#include "Kernel_Offset.h"
#include "alist_i.h"
#include "Mcu.h"

OsTickType OsTickFreq = 1000;

// ##################################    DEBUG     #################################
// All output on as standard
uint32 os_dbg_mask =
	D_MASTER_PRINT ;
//	D_ISR_MASTER_PRINT |
//	D_STDOUT |
//	D_ISR_STDOUT | D_TASK | D_ALARM;

// ###############################    APPLICATION     ##############################
// A single, non-configurable application for now
rom_app_t rom_app_list[] = {
	{
		.application_id = APPLICATION_ID_application_1,
		.name = "application_1",
		.trusted = true,
		.StartupHook = NULL,
		.ShutdownHook = NULL,
		.ErrorHook = NULL,
		.isr_mask = 0,
		.scheduletable_mask  = 0,
		.alarm_mask  = 0,
		.counter_mask  = 0,
		.resource_mask  = 0,
		.message_mask  = 0,
	}
};


// #################################    COUNTERS     ###############################
counter_obj_t counter_list[] = {
	{
		.name = "OsTick",
		.type = COUNTER_TYPE_HARD,
		.unit = COUNTER_UNIT_NANO,
		.alarm_base.maxallowedvalue = 65535,
		.alarm_base.tickperbase = 1,
		.alarm_base.mincycle = 0,
	},
};

CounterType Os_Arc_OsTickCounter = COUNTER_ID_OsTick;

// ##################################    ALARMS     ################################
alarm_obj_t alarm_list[] = {
	{
		.name = "ComAlarm",
		.counter = &counter_list[COUNTER_ID_OsTick],
		.counter_id = COUNTER_ID_OsTick,
		.autostart = {
			.active = TRUE,
			.alarmtime = 5,
			.cycletime = 20,
			.appmode_mask = APPLICATION_ID_application_1, //___ARCTICSTUDIO_GENERATOR_TODO___,
		},
		.action = {
			.type = ALARM_ACTION_ACTIVATETASK,
			.task_id = TASK_ID_ComTask,
		},
	},
};


// ################################    RESOURCES     ###############################
resource_obj_t resource_list[] = {
	{
		.nr = RES_SCHEDULER,
		.type = RESOURCE_TYPE_STANDARD,
		.ceiling_priority = 0,
		.application_owner_id = 0,
		.task_mask = 0,
		.owner = (-1),
	},
};

// ##############################    STACKS (TASKS)     ############################
uint8_t stack_ComTask[PRIO_STACK_SIZE];
uint8_t stack_OsIdle[PRIO_STACK_SIZE];
uint8_t stack_StartupTask[PRIO_STACK_SIZE];

// #####################    TIMING PROTECTIONS (TASKS, ISRS)     ###################
timing_protection_t timing_protection_list[] = {
};

rom_pcb_t rom_pcb_list[] = {
// ##################################    TASKS     #################################
	{
		.pid = TASK_ID_ComTask,
		.name = "ComTask",
		.entry = ComTask,
		.prio = 10,
		.proc_type = PROC_BASIC,
		.stack.size = sizeof stack_ComTask,
		.stack.top = stack_ComTask,
		.autostart = false,
		.application_id = APPLICATION_ID_application_1,
	},
	{
		.pid = TASK_ID_OsIdle,
		.name = "OsIdle",
		.entry = OsIdle,
		.prio = 1,
		.proc_type = PROC_EXTENDED,
		.stack.size = sizeof stack_OsIdle,
		.stack.top = stack_OsIdle,
		.autostart = true,
		.application_id = APPLICATION_ID_application_1,
	},
	{
		.pid = TASK_ID_StartupTask,
		.name = "StartupTask",
		.entry = StartupTask,
		.prio = 100,
		.proc_type = PROC_BASIC,
		.stack.size = sizeof stack_StartupTask,
		.stack.top = stack_StartupTask,
		.autostart = true,
		.application_id = APPLICATION_ID_application_1,
	},
};

uint8_t pcb_list[PCB_T_SIZE*ARRAY_SIZE(rom_pcb_list)];

uint8_t os_interrupt_stack[OS_INTERRUPT_STACK_SIZE] __attribute__ ((aligned (0x10)));

GEN_IRQ_VECTOR_TABLE_HEAD {};
GEN_IRQ_ISR_TYPE_TABLE_HEAD {};
GEN_IRQ_PRIORITY_TABLE_HEAD {};


// ##################################    HOOKS    ##################################
struct os_conf_global_hooks_s os_conf_global_hooks = {
		.StartupHook = StartupHook,
		.ProtectionHook = ProtectionHook,
		.ShutdownHook = ShutdownHook,
		.ErrorHook = ErrorHook,
		.PreTaskHook = PreTaskHook,
		.PostTaskHook = PostTaskHook,
};

#include "os_config_funcs.h"
