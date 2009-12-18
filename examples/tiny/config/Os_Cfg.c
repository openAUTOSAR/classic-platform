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

#include <stdlib.h>
#include <stdint.h>
#include "os_config_macros.h"
#include "Platform_Types.h"
#include "Os.h"				// includes Os_Cfg.h
#include "kernel.h"
#include "kernel_offset.h"
#include "alist_i.h"
#include "Mcu.h"

extern void dec_exception( void );

OsTickType OsTickFreq = 1000;

// atleast 1
#define SERVICE_CNT 1

GEN_TRUSTEDFUNCTIONS_LIST

//--- APPLICATIONS ----

GEN_APPLICATION_HEAD {

	GEN_APPLICATON(	0,
					"application_1",
					true,
					NULL,NULL,NULL , 0,0,0,0,0,0 )
};

// --- RESOURCES ---

GEN_RESOURCE_HEAD {
	GEN_RESOURCE(RES_SCHEDULER,RESOURCE_TYPE_STANDARD,0,0,0),		// Standard resource..
};

//--- TASKS ----

DECLARE_STACK(OsIdle,PRIO_STACK_SIZE);
DECLARE_STACK(etask_1,PRIO_STACK_SIZE);
DECLARE_STACK(etask_2,PRIO_STACK_SIZE);
DECLARE_STACK(btask_3,PRIO_STACK_SIZE);

GEN_TASK_HEAD {
	GEN_ETASK(	OsIdle,
				0,
				true/*auto*/,
				NULL/*tm*/,
				APPLICATION_ID_application_1/*app*/,
				NULL/*rsrc*/),

	GEN_ETASK(	etask_1,
				1,
				true/*auto*/,
				NULL/*tm*/,
				APPLICATION_ID_application_1/*app*/,
				NULL/*rsrc*/),

	GEN_ETASK(	etask_2,
				2,
				true/*auto*/,
				NULL/*tm*/,
				APPLICATION_ID_application_1/*app*/,
				NULL/*rsrc*/),


	GEN_BTASK(	btask_3,
				3,
				false/*auto*/,
				NULL/*tm*/,
				APPLICATION_ID_application_1/*app*/,
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

// --- MESSAGES ---

// --- ALARMS ---
#define ALARM_USE

GEN_ALARM_HEAD {
	GEN_ALARM(	0,"Alarm_4ms",COUNTER_ID_OsTick,
				1,100,2,0,		/*active,start,cycle,app_mask */
				ALARM_ACTION_SETEVENT, TASK_ID_etask_1, 2, 0 ),
};

// --- SCHEDULETABLES ---

// --- HOOKS ---

GEN_HOOKS( StartupHook, ProtectionHook, ShutdownHook, ErrorHook, PreTaskHook, PostTaskHook )



// --- MISC ---

uint32 os_dbg_mask = \
	D_MASTER_PRINT |\
	D_ISR_MASTER_PRINT |\
	D_RAMLOG |\
    D_ISR_RAMLOG | D_TASK | D_ALARM;


// | D_ALARM | D_TASK;


#include "os_config_funcs.h"









