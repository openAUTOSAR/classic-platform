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









#include "os_config_macros.h"
#include "kernel.h"
#include "Os_Cfg.h"
#include "Mcu.h"
#include "kernel_offset.h"
//#include "Hooks.h"

extern void dec_exception( void );

OsTickType OsTickFreq = 1000;

GEN_APPLICATION_HEAD {
	GEN_APPLICATON(BLINKER_APP_ID,"PwmApp",true,NULL,NULL,NULL , 0,0,0,0,0,0 )
};

GEN_RESOURCE_HEAD {
	GEN_RESOURCE(RES_SCHEDULER,RESOURCE_TYPE_STANDARD,0,0,0),		// Standard resource..
};

//--- TASKS ----
DECLARE_STACK(OsIdle,PRIO_STACK_SIZE);
DECLARE_STACK(bTask25,PRIO_STACK_SIZE);
DECLARE_STACK(bTask100,PRIO_STACK_SIZE);
DECLARE_STACK(Startup,PRIO_STACK_SIZE);

GEN_TASK_HEAD {
	GEN_ETASK(	OsIdle,
				TASK_ID_OsIdle,
				true/*auto*/,
				NULL/*tm*/,
				BLINKER_APP_ID/*app*/,
				NULL/*rsrc*/),

	GEN_BTASK(	bTask25,
				TASK_ID_bTask25,
				false/*auto*/,
				NULL/*tm*/,
				BLINKER_APP_ID/*app*/,
				NULL/*rsrc*/),

	GEN_BTASK(	bTask100,
				TASK_ID_bTask100,
				false/*auto*/,
				NULL/*tm*/,
				BLINKER_APP_ID/*app*/,
				NULL/*rsrc*/),

	GEN_BTASK(	Startup,
				TASK_ID_Startup,
				true/*auto*/,
				NULL/*tm*/,
				BLINKER_APP_ID/*app*/,
				NULL/*rsrc*/),
};

GEN_PCB_LIST()


// --- INTERRUPTS ---
uint8_t os_interrupt_stack[OS_INTERRUPT_STACK_SIZE] __attribute__ ((aligned (0x10)));

GEN_IRQ_VECTOR_TABLE_HEAD {};
GEN_IRQ_ISR_TYPE_TABLE_HEAD {};
GEN_IRQ_PRIORITY_TABLE_HEAD {};

// --- COUNTERS ---
GEN_COUNTER_HEAD {
	GEN_COUNTER(	COUNTER_ID_OsTick,
					"COUNTER_ID_OsTick",
					COUNTER_TYPE_HARD,
					COUNTER_UNIT_NANO,
					0xffff,1,1,0 ),
};

CounterType Os_Arc_OsTickCounter = COUNTER_ID_OsTick;

// --- ALARMS ---
GEN_ALARM_HEAD {
	{
		.expire_val	= 25,
		.active		= FALSE,
		.counter = &counter_list[COUNTER_ID_OsTick],
		.counter_id = COUNTER_ID_OsTick,
		.action =
		{
				.type = ALARM_ACTION_ACTIVATETASK,
				.task_id = TASK_ID_bTask25,
				.event_id = 0,
		}
	},
	{
		.expire_val	= 100,
		.active		= FALSE,
		.counter = &counter_list[COUNTER_ID_OsTick],
		.counter_id = COUNTER_ID_OsTick,
		.action =
		{
				.type = ALARM_ACTION_ACTIVATETASK,
				.task_id = TASK_ID_bTask100,
				.event_id = 0,
		}
	},
};

// --- HOOKS ---
GEN_HOOKS( StartupHook, ProtectionHook, ShutdownHook, ErrorHook, PreTaskHook, PostTaskHook )

// --- MISC ---
uint32 os_dbg_mask = 0;
/*
	D_MASTER_PRINT |\
	D_ISR_MASTER_PRINT |\
	D_STDOUT |\
	D_ISR_STDOUT |
	D_ALARM | D_TASK;
*/

#include "os_config_funcs.h"


