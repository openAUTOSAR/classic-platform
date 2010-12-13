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
#ifndef PCB_H
#define PCB_H

struct OsApplication;
struct OsRomPcb;

#define PID_IDLE			0
#define PRIO_IDLE			0

#define ST_READY 			1
#define ST_WAITING			(1<<1)
#define ST_SUSPENDED		(1<<2)
#define ST_RUNNING			(1<<3)
#define ST_NOT_STARTED  	(1<<4)

typedef uint16_t state_t;

/* from Os.h types */
typedef TaskType		OsTaskidType;
typedef EventMaskType 	OsEventType;
/* Lets have 32 priority levels
 * 0 - Highest prio
 * 31- Lowest
 */
typedef sint8 OsPriorityType;

#define TASK_NAME_SIZE		16


/* STD container : OsIsr
 * Class: ALL
 *
 * OsIsrCategory:				1    CATEGORY_1 or CATEGORY_2
 * OsIsrResourceRef:			0..1 Reference to OsResource
 * OsIsrTimingProtection[C] 	0..1
 * */

/* STD container : OsIsrResourceLock
 * Class: 2 and 4
 *
 * OsIsrResourceLockBudget  	1    Float in seconds (MAXRESOURCELOCKINGTIME)
 * OsIsrResourceLockResourceRef 1    Ref to OsResource
 * */

/* STD container : OsIsrTimingProtection
 * Class: 2 and 4
 *
 * OsIsrAllInterruptLockBudget  0..1 float
 * OsIsrExecutionBudget 		0..1 float
 * OsIsrOsInterruptLockBudget 	0..1 float
 * OsIsrTimeFrame 				0..1 float
 * OsIsrResourceLock[C] 		0..1
 * */



/* STD container : OsHooks
 * OsErrorHooks:			1
 * OsPostTaskHook:			1
 * OsPreTaskHook:			1
 * OsProtectionHook:		0..1 Class 2,3,4 it's 1 instance
 * OsShutdownHook:			1
 * OsStartupkHook:			1
 * */

typedef struct OsHooks {
#if (  OS_SC2 == STD_ON ) || ( OS_SC3 == STD_ON ) || ( OS_SC4 == STD_ON )
	ProtectionHookType 	ProtectionHook;
#endif
	StartupHookType 	StartupHook;
	ShutdownHookType 	ShutdownHook;
	ErrorHookType 		ErrorHook;
	PreTaskHookType 	PreTaskHook;
	PostTaskHookType 	PostTaskHook;
} OsHooksType;

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
} OsStackType;


#define SYS_FLAG_HOOK_STATE_EXPECTING_PRE	0
#define SYS_FLAG_HOOK_STATE_EXPECTING_POST   1

/* We do ISR and TASK the same struct for now */
typedef struct OsPcb {
	OsTaskidType 	pid;					// TASK
	OsPriorityType  prio;
#if ( OS_SC1 == STD_ON ) || ( OS_SC4 == STD_ON )
	ApplicationType application_id;
	uint32			app_mask;
#endif
	void 			(*entry)();
	proc_type_t 	proc_type;
	int 			autostart:1;			// TASK
	OsStackType		stack;					// TASK

	int				vector; 				// ISR
	char 			name[TASK_NAME_SIZE];
#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
	OsTimingProtectionType	*timing_protection;
#endif

	state_t 		state;					// TASK
	OsEventType 	ev_wait;				// TASK
	OsEventType 	ev_set;					// TASK

	uint32_t 		flags;

	enum OsTaskSchedule 	scheduling;	// TASK
	/* belongs to this application */
	struct OsApplication	*application;

	/* OsTaskActivation
	 * The limit from OsTaskActivation. This is 1 for extended tasks */
	uint8_t activationLimit;
	// The number of queued activation of a task
	int8_t activations;

	// A task can hold only one internal resource and only i f
	// OsTaskSchedule == FULL
	OsResourceType *resource_int_p;		// TASK

    // OsTaskResourceRef
	// What resources this task have access to
	// Typically (1<<RES_xxx) | (1<<RES_yyy)
	uint32_t resourceAccess;


	// What resource that are currently held by this task
	// Typically (1<<RES_xxx) | (1<<RES_yyy)
	uint32_t resourceMaskTaken;

	TAILQ_HEAD(head,OsResource) resource_head; // TASK

	const struct OsRomPcb *pcb_rom_p;

	/* TODO: Arch specific regs .. make space for them later...*/
	uint32_t	regs[16]; 				// TASK
	/* List of PCB's */
	TAILQ_ENTRY(OsPcb) pcb_list;		// TASK
	/* ready list */
	TAILQ_ENTRY(OsPcb) ready_list;		// TASK
} OsPcbType;

/*-----------------------------------------------------------------*/

typedef struct OsRomPcb {
	OsTaskidType	pid;
	OsPriorityType	prio;
	uint32			app_mask;
	void 			(*entry)();
	proc_type_t  	proc_type;
	uint8	 	 	autostart;
	OsStackType 	stack;
	int				vector; 				// ISR
	ApplicationType application_id;
	char 		 	name[16];
	enum OsTaskSchedule scheduling;
	uint32_t 		resourceAccess;
	// pointer to internal resource
	// NULL if none
	OsResourceType	*resource_int_p;
	OsTimingProtectionType	*timing_protection;
	uint8_t          activationLimit;
//	lockingtime_obj_t
} OsRomPcbType;

#endif

