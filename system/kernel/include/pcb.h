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
struct OsTaskConst;

#define PID_IDLE			0
#define PRIO_IDLE			0

#define ST_READY 			1
#define ST_WAITING			(1<<1)
#define ST_SUSPENDED		(1<<2)
#define ST_RUNNING			(1<<3)
#define ST_NOT_STARTED  	(1<<4)
#define ST_SLEEPING		(1<<5)

#define ST_ISR_RUNNING			1
#define ST_ISR_NOT_RUNNING 		2

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



/* STD container : OsHooks
 * OsErrorHooks:			1
 * OsPostTaskHook:			1
 * OsPreTaskHook:			1
 * OsProtectionHook:		0..1 Class 2,3,4 it's 1 instance
 * OsShutdownHook:			1
 * OsStartupkHook:			1
 * */

typedef struct OsHooks {
#if	(OS_USE_APPLICATIONS == STD_ON)
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

#if 0
#define PROC_ISR		0x4
#define PROC_ISR1		0x4
#define PROC_ISR2		0xc
#endif


typedef struct {
	void   		*curr;	// Current stack ptr( at swap time )
	void   		*top;	// Top of the stack( low address )
	uint32		size;	// The size of the stack
} OsStackType;


#define SYS_FLAG_HOOK_STATE_EXPECTING_PRE	0
#define SYS_FLAG_HOOK_STATE_EXPECTING_POST   1

/* We do ISR and TASK the same struct for now */
typedef struct OsTaskVar {
	OsStackType		stack;					// TASK

#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
	OsTimingProtectionType	*timing_protection;
#endif

	state_t 		state;					// TASK

	/* Events the task wait for ( what events WaitEvent() was called with) */
	OsEventType 	ev_wait;
	/* Events that are set by SetEvent() on the task */
	OsEventType 	ev_set;
	/* The events the task may react on */
	OsEventType     ev_react;

	uint32_t 		flags;

	/* Priority of the task, this can be different depening on if the
	 * task hold resources. Related to priority inversion */
	OsPriorityType  activePriority;
#if 0
	void 			(*entry)();

	char 			name[TASK_NAME_SIZE];

	OsTaskidType 	pid;					// TASK
	OsPriorityType  prio;

	proc_type_t 	proc_type;
	int 			autostart;				// TASK

	enum OsTaskSchedule 	scheduling;	// TASK
	/* belongs to this application */
	struct OsApplication	*application;
	/* OsTaskActivation
	 * The limit from OsTaskActivation. This is 1 for extended tasks */
	uint8_t activationLimit;

	// A task can hold only one internal resource and only i f
	// OsTaskSchedule == FULL
	OsResourceType *resourceIntPtr;		// TASK

    // OsTaskResourceRef
	// What resources this task have access to
	// Typically (1<<RES_xxx) | (1<<RES_yyy)
	uint32_t resourceAccess;

#endif

	// The number of queued activation of a task
	int8_t activations;

	// What resource that are currently held by this task
	// Typically (1<<RES_xxx) | (1<<RES_yyy)
	uint32_t resourceMaskTaken;

	TAILQ_HEAD(head,OsResource) resourceHead; // TASK

	const struct OsTaskConst *constPtr;

	/* TODO: Arch specific regs .. make space for them later...*/
	uint32_t	regs[16]; 				// TASK
#if defined(USE_KERNEL_EXTRA)
	TAILQ_ENTRY(OsTaskVar) timerEntry;		// TASK
	int32_t		   timerDec;
#endif
	/* List of PCB's */
//	TAILQ_ENTRY(OsTaskVar) pcb_list;		// TASK
	/* ready list */
	TAILQ_ENTRY(OsTaskVar) ready_list;		// TASK
} OsTaskVarType;

/*-----------------------------------------------------------------*/


/*-----------------------------------------------------------------*/

/* STD container : OsTask
 * OsTaskActivation:		    1
 * OsTaskPriority:			    1
 * OsTaskSchedule:			    1
 * OsTaskAccessingApplication:	0..*
 * OsTaskEventRef:				0..*
 * OsTaskResourceRef:			0..*
 * OsTaskAutoStart[C]			0..1
 * OsTaskTimingProtection[C]	0..1
 * */


typedef struct OsTaskConst {
	OsTaskidType	pid;
	OsPriorityType	prio;
	uint32			app_mask;
	void 			(*entry)();
	proc_type_t  	proc_type;
	uint8	 	 	autostart;
	OsStackType 	stack;
	int				vector; 		// ISR

#if	(OS_USE_APPLICATIONS == STD_ON)
	/* Application that owns this task */
	ApplicationType applOwnerId;
	/* Applications that may access task when state is APPLICATION_ACCESSIBLE */
	uint32			accessingApplMask;
#endif

	char 		 	name[16];
	enum OsTaskSchedule scheduling;
	uint32_t 		resourceAccess;
	// pointer to internal resource
	// NULL if none
	OsResourceType	*resourceIntPtr;
	OsTimingProtectionType	*timing_protection;
	uint8_t          activationLimit;
//	lockingtime_obj_t
} OsTaskConstType;

#endif

