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


#ifndef KERNEL_H_
#define KERNEL_H_

#include <sys/queue.h>
#include "Os.h"
#include "counter_i.h"
#include "alarm_i.h"
#include "sched_table_i.h"

typedef void ( * trusted_func_t)( TrustedFunctionIndexType , TrustedFunctionParameterRefType );

/*-----------------------------------------------------------------*/
/* Global Hooks( non-application specific ) */

typedef struct os_conf_global_hooks_s {
	ProtectionHookType ProtectionHook;
	StartupHookType StartupHook;
	ShutdownHookType ShutdownHook;
	ErrorHookType ErrorHook;
	PreTaskHookType PreTaskHook;
	PostTaskHookType PostTaskHook;
} os_conf_global_hooks_t;

/*-----------------------------------------------------------------*/

/*
 * The only information about the COM that is valid is
 * in the COM specification ..SWS_COM.pdf.
 *
 * The most important requirements are COM010 and COM013
 *
 * Com_Init()
 * Com_DeInit()
 *
 * No error hooks..
 * No. GetMessageStatus()
 * No. SendZeroMessage()
 * No. SendDynamicMessage(), RecieveDynamicMessage()
 *
 * See http://www.altium.com/files/AltiumDesigner6/LearningGuides/GU0102%20TSK51x%20TSK52x%20RTOS.pdf
 *
 * Yes. SendMessage()
 *
 * */

typedef enum message_property_e {
	// ???
	SEND_STATIC_INTERNAL,
	// messages are not consumed during read
	RECEIVE_UNQUEUED_INTERNAL,
	// We have an internal queue
	RECEIVE_QUEUE_INTERNAL,
} message_property_t;



typedef enum message_notification_action_e {
	MESSAGE_NOTIFICATION_ACTION_NONE=0,
	MESSAGE_NOTIFICATION_ACTION_ACTIVATETASK,
	MESSAGE_NOTIFICATION_ACTION_SETEVENT,
} message_notification_action_t;

typedef struct message_notification_s {
	message_notification_action_t type;
	TaskType 			task_id;
	EventMaskType 		event_id;
} message_notification_t;


typedef struct message_obj_s {
	message_property_t		property;		// send/recieve...
	int 					q_size; 		// 0-Not queued
	message_notification_t 	notification;
	// TODO: This is not a good solution but it will have to do for now
	void *data;
	int data_size;
} message_obj_t;


/* OsTask/OsTaskSchedule */
enum OsTaskSchedule {
	FULL,
	NON
};

/*-----------------------------------------------------------------*/

typedef uint8_t proc_type_t;

#define PROC_PRIO		0x1
#define PROC_BASIC		0x1
#define PROC_EXTENDED	0x3

#define PROC_ISR		0x4
#define PROC_ISR1		0x4
#define PROC_ISR2		0xc


typedef struct {
	void   		*curr;	// Current stack ptr( at swap time )
	void   		*top;	// Top of the stack( low address )
	uint32		size;	// The size of the stack
} stack_t;

typedef struct rom_app_s {
	uint32 	application_id;
	char 	name[16];
	uint8	trusted;

	/* hooks */
	void (*StartupHook)( void );
	void (*ShutdownHook)( Std_ReturnType Error );
	void (*ErrorHook)( Std_ReturnType Error );

	uint32 isr_mask;
	uint32 scheduletable_mask;
	uint32 alarm_mask;
	uint32 counter_mask;
	uint32 resource_mask;
	uint32 message_mask;

} rom_app_t;

/*-----------------------------------------------------------------*/

typedef struct lockingtime_obj_s {
	char id[16];
	int resource;
	int resource_clock_time;
	int all_interrupt_clock_time;
	int os_interrupt_clock_time;
	int locking_type;
} lockingtime_obj_t;


typedef enum {
	// the normal behaviour
	RESOURCE_TYPE_STANDARD,
	// ??
	RESOURCE_TYPE_LINKED,
	// Used for grouping tasks
	RESOURCE_TYPE_INTERNAL
} resource_type_t;

typedef struct  {
	resource_type_t type;
	// used only if type is RESOURCE_TYPE_LINKED
	ResourceType    linked_resource;
} resource_property_t;

/*-----------------------------------------------------------------*/
typedef struct resource_obj_s {
	char id[16];
	// The running number, starting at RES_SCHEDULER=0
	int nr;
	// The calculated ceiling prio
	uint32 ceiling_priority;
	// Stored prio of the owner oi the resource
	uint32 old_task_prio;

	// What application may access this resource. A resource may only be
	// accessed by one application
	uint32 application_owner_id;
	// What tasks may access this resource. A resource may be be shared
	// several tasks.
	uint32 task_mask;
	// Owner of the resource...
	TaskType owner;

	resource_type_t type;
	// used only if type is RESOURCE_TYPE_LINKED
	ResourceType    linked_resource;

	/* List of resources for each task. */
	TAILQ_ENTRY(resource_obj_s) listEntry;

} resource_obj_t;

typedef enum {
	LOCK_TYPE_RESOURCE,
	LOCK_TYPE_INTERRUPT,
} lock_type_t;

/*
typedef struct {
	ResourceType resource;
	uint64 locktime;
} resource_locktime_t;

typedef struct {
	uint64 all;
	uint64 os;
} interrupt_locktime_t;
*/

typedef struct lockingtime_s {
//	lock_type_t type;
	lock_type_t type;
	union {
		struct {
			ResourceType id;
			uint64 time;
		} resource;

		struct {
			uint64 all;
			uint64 os;
		} interrupt;
	} u;
//		resource_locktime_t resource;
//		interrupt_locktime_t interrupt;
//	} locktime;
} lockingtime_t;

typedef struct timing_protection_s {
	// ROM, worst case execution budget in ns
	uint64	execution_budget;
	// ROM, the frame in ns that timelimit may execute in.
	uint64 timeframe;
	// ROM, time in ns that the task/isr may with a timeframe.
	uint64 timelimit;
	// ROM, resource/interrupt locktimes
	lockingtime_t *lockingtime;

//	interrupt_locktime_t interrupt_locktime;
	// ROM, resource lock times
//	const resource_locktime_t *resource_locktime_list;
//	lockingtime_t *lockingtime;
} timing_protection_t;


/*-----------------------------------------------------------------*/

typedef struct rom_pcb_s {
	TaskType	 	pid;
	uint8		 	prio;
	uint32			app_mask;
	void 			(*entry)();
	proc_type_t  	proc_type;
	uint8	 	 	autostart;
	stack_t 	 	stack;
	int				vector; 				// ISR
	ApplicationType application_id;
	char 		 	name[16];
	enum OsTaskSchedule scheduling;
	uint32_t 		resourceAccess;
//	uint64			execution_budget;
//	uint32			count_limit;
//	uint64			time_limit;
	// pointer to internal resource
	// NULL if none
	resource_obj_t	*resource_int_p;
	timing_protection_t	*timing_protection;
//	lockingtime_obj_t
} rom_pcb_t;


/*-----------------------------------------------------------------*/

typedef struct sched_action_s {
	int   			type;  		// 0 - activate task, 1 - event
	uint64 			offset;  	// for debug only???
	uint64    		delta;   	// delta to next action
	TaskType 		task_id;
	EventMaskType event_id;		// used only if event..
} sched_action_t;

/*-----------------------------------------------------------------*/


/*

typedef enum message_type_e {
	// ???
	SEND_STATIC_INTERNAL,
	// messages are not consumed during read
	RECEIVE_UNQUEUED_INTERNAL,
	// We have an internal queue
	RECEIVE_QUEUE_INTERNAL,
} message_type_t;
*/

#if 0
typedef struct message_obj_s {
	char name[16];
	message_type_t type;
	void *
	char name[16];
	uint32 	accessingapplications_mask;
	// TODO:  Below types are NOT OK !!!!!!!
	void*	cdatatype;
	void*	initialvalue;
	uint32 	queuesize;
	message_property_t messageproperty;
	void *	callbackroutine;
	uint32 	callbackMessage;
	void*	flagname;
	uint32 	notification;

} message_obj_t;
#endif



/*-----------------------------------------------------------------*/


typedef struct memory_s {
	uint32_t flags;
	/* ptr to start of memory region */
	void  	*start;
	/* size in bytes */
	uint32_t size;
} memory_t;

/*-----------------------------------------------------------------*/
/*
 * debug settings for os_debug_mask
 *
 */

#define OS_DBG_MASTER_PRINT			(1<<0)
#define OS_DBG_ISR_MASTER_PRINT		(1<<1)
#define OS_DBG_STDOUT				(1<<2)
#define OS_DBG_ISR_STDOUT			(1<<3)

// Enable print dbg_XXXX (not dbg_isr_XXX though)
#define D_MASTER_PRINT			(1<<0)
// Enable print for all dbg_isr_XXX
#define D_ISR_MASTER_PRINT		(1<<1)
// print to STDOUT. If not set it prints to ramlog
#define D_STDOUT				(1<<2)
#define D_RAMLOG				0
// print to STDOUT, If not set print to ramlog
#define D_ISR_STDOUT			(1<<3)
#define D_ISR_RAMLOG			0

#define D_TASK						(1<<16)
#define D_ALARM						(1<<18)

#define OS_DBG_TASK					(1<<16)
#define OS_DBG_ALARM				(1<<18)



#endif /* KERNEL_H_ */
