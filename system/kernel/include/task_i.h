/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/

#ifndef TASK_I_H_
#define TASK_I_H_

/* ----------------------------[includes]------------------------------------*/



#include <stdlib.h>
#include <assert.h>
#include "Os.h"
#include "resource_i.h"
#include "internal.h"

/* ----------------------------[define]--------------------------------------*/
/* ----------------------------[macro]---------------------------------------*/


struct OsApplication;
struct OsTaskConst;

#define NO_TASK_OWNER 	(TaskType)(~0)


#define PID_IDLE			0
#define PRIO_IDLE			0

#define ST_READY 			1
#define ST_WAITING			(1<<1)
#define ST_SUSPENDED		(1<<2)
#define ST_RUNNING			(1<<3)
#define ST_NOT_STARTED  	(1<<4)
#define ST_SLEEPING			(1<<5)
#define ST_WAITING_SEM		(1<<6)

#define ST_ISR_RUNNING		1
#define ST_ISR_NOT_RUNNING 	2

#define TASK_NAME_SIZE		16


#define STACK_PATTERN		0x42		/* Pattern that the stack that unused stack space is filled with */

#define PRIO_ILLEGAL	-100


/* ----------------------------[typedef]-------------------------------------*/

typedef uint16_t state_t;

/* from Os.h types */
typedef TaskType		OsTaskidType;
typedef EventMaskType 	OsEventType;
/* Lets have 32 priority levels
 * 0 - Highest prio
 * 31- Lowest
 */
typedef sint8 OsPriorityType;


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

#define PROC_PRIO				0x1
#define PROC_BASIC				0x1
#define PROC_EXTENDED			0x3

typedef struct {
	void   		*curr;	/* Current stack ptr( at swap time ) */
	void   		*top;	/* Top of the stack( low address )   */
	uint32		size;	/* The size of the stack             */
} OsStackType;


#define SYS_FLAG_HOOK_STATE_EXPECTING_PRE		0
#define SYS_FLAG_HOOK_STATE_EXPECTING_POST   	1

/* We do ISR and TASK the same struct for now */
typedef struct OsTaskVar {
	OsStackType		stack;

#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
	OsTimingProtectionType	*timing_protection;
#endif

	state_t 		state;
	OsEventType 	ev_wait;			/* Events the task wait for
										 * ( what events WaitEvent() was called with) */
	OsEventType 	ev_set;				/* Events that are set by SetEvent() on the task */
	OsEventType     ev_react;			/* The events the task may react on */
	uint32_t 		flags;

	OsPriorityType  activePriority;  	/* Priority of the task, this can be different depening on if the
									  	 * task hold resources. Related to priority inversion */
	int8_t 			activations;		/* The number of queued activation of a task */
	uint32_t 		resourceMaskTaken;	/* What resource that are currently held by this task
										 * Typically (1<<RES_xxx) | (1<<RES_yyy) */

	TAILQ_HEAD(head,OsResource) resourceHead;
	TAILQ_HEAD(shead,OsSpinlock) spinlockHead;	/* Occupied spinlocks list */

	const struct OsTaskConst *constPtr;
	uint32_t	regs[16]; 					/* TODO: Arch specific regs .. make space for them later...*/
#if defined(USE_KERNEL_EXTRA)
	TAILQ_ENTRY(OsTaskVar) timerEntry;
	int32_t		   timerDec;

	/* Semaphore list */
	STAILQ_ENTRY(OsTaskVar) semEntry;
#endif
	TAILQ_ENTRY(OsTaskVar) ready_list;
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
	void 			(*entry)( void );
	proc_type_t  	proc_type;
	uint8	 	 	autostart;
	OsStackType 	stack;
#if	(OS_USE_APPLICATIONS == STD_ON)
	ApplicationType applOwnerId;		/* Application that owns this task */
	uint32			accessingApplMask;	/* Applications that may access task
										 * when state is APPLICATION_ACCESSIBLE */
#endif
	char 		 	name[16];
	enum OsTaskSchedule scheduling;
	uint32_t 		resourceAccess;
	uint32_t 		eventMask;
	OsResourceType	*resourceIntPtr;	/* pointer to internal resource, NULL if none */
	OsTimingProtectionType	*timing_protection;
	uint8_t          activationLimit;
} OsTaskConstType;


/* ----------------------------[function prototypes]-------------------------*/

extern OsTaskVarType Os_TaskVarList[OS_TASK_CNT];
extern GEN_TASK_HEAD;

void Os_Dispatch( uint32_t op );

void Os_TaskMakeReady( 	OsTaskVarType *pcb );
void Os_TaskMakeWaiting( OsTaskVarType *pcb );

void Os_TaskSwapContext  ( OsTaskVarType *old_pcb, OsTaskVarType *new_pcb );
void Os_TaskSwapContextTo( OsTaskVarType *old_pcb, OsTaskVarType *new_pcb );

void Os_TaskStartExtended( void );
void Os_TaskStartBasic( void );
void Os_TaskContextInit( OsTaskVarType *pcb );
TaskType 		Os_AddTask( OsTaskVarType *pcb );
OsTaskVarType * Os_TaskGetTop( void );



/**
 * Set the task to running state and remove from ready list
 *
 * @params pcb Ptr to pcb
 */
static inline void Os_TaskMakeRunning( OsTaskVarType *pcb ) {
	pcb->state = ST_RUNNING;
}


#if 0 // Not used any more
OsTaskVarType  *os_find_higher_priority_task( OsPriorityType prio );
#endif

static inline OsTaskVarType * Os_TaskGet( TaskType pid ) {
	return &Os_TaskVarList[pid];
}

static inline ApplicationType Os_TaskGetApplicationOwner( TaskType id ) {
	ApplicationType rv;
	if( id < OS_TASK_CNT ) {
		rv = Os_TaskConstList[id].applOwnerId;
	} else {
		rv = INVALID_OSAPPLICATION;
	}
	return rv;
}



/**
 * Add a resource to a list of resources held by pcbPtr
 *
 * @param rPtr   Ptr to the resource to add to the task
 * @param pcbPtr Ptr to the task
 */
static inline void Os_TaskResourceAdd( OsResourceType *rPtr, OsTaskVarType *pcbPtr) {
	/* Save old task prio in resource and set new task prio */
	rPtr->owner = pcbPtr->constPtr->pid;
	rPtr->old_task_prio = pcbPtr->activePriority;
	pcbPtr->activePriority = rPtr->ceiling_priority;

	if( rPtr->type != RESOURCE_TYPE_INTERNAL ) {
		TAILQ_INSERT_TAIL(&pcbPtr->resourceHead, rPtr, listEntry);
	}
}

/**
 * Remove a resource from the list of resources held by pcbPtr
 * @param rPtr   Ptr to the resource to remove from the task
 * @param pcbPtr Ptr to the task
 */
static inline  void Os_TaskResourceRemove( OsResourceType *rPtr , OsTaskVarType *pcbPtr) {
	assert( rPtr->owner == pcbPtr->constPtr->pid );
	rPtr->owner = NO_TASK_OWNER;
	pcbPtr->activePriority = rPtr->old_task_prio;

	if( rPtr->type != RESOURCE_TYPE_INTERNAL ) {
		/* The list can't be empty here */
		assert( !TAILQ_EMPTY(&pcbPtr->resourceHead) );

		/* The list should be popped in LIFO order */
		assert( TAILQ_LAST(&pcbPtr->resourceHead, head) == rPtr );

		/* Remove the entry */
		TAILQ_REMOVE(&pcbPtr->resourceHead, rPtr, listEntry);
	}
}

/**
 * Free all resource held by a task.
 *
 * @param pcbPtr Ptr to the task
 */
static inline void Os_TaskResourceFreeAll( OsTaskVarType *pcbPtr ) {
	OsResourceType *rPtr;

	/* Pop the queue */
	TAILQ_FOREACH(rPtr, &pcbPtr->resourceHead, listEntry ) {
		Os_TaskResourceRemove(rPtr,pcbPtr);
	}
}


#define os_pcb_get_state(pcb) ((pcb)->state)


static inline void *Os_StackGetUsage( OsTaskVarType *pcb ) {

	uint8_t *p = pcb->stack.curr;
	uint8_t *end = pcb->stack.top;

	while( (*end == STACK_PATTERN) && (end<p)) {
			end++;
		}
	return (void *)end;
}

static inline void Os_StackSetEndmark( OsTaskVarType *pcbPtr ) {
	uint8_t *end = pcbPtr->stack.top;
	*end = STACK_PATTERN;
}

static inline _Bool Os_StackIsEndmarkOk( OsTaskVarType *pcbPtr ) {
	_Bool rv;
	uint8_t *end = pcbPtr->stack.top;
	rv =  ( *end == STACK_PATTERN);
	if( !rv ) {
		OS_DEBUG(D_TASK,"Stack End Mark is bad for %s curr: %p curr: %p\n",
				pcbPtr->constPtr->name,
				pcbPtr->stack.curr,
				pcbPtr->stack.top );
	}
	return rv;
}

static inline void Os_StackPerformCheck( OsTaskVarType *pcbPtr ) {
#if (OS_STACK_MONITORING == 1)
		if( !Os_StackIsEndmarkOk(pcbPtr) ) {
#if (OS_SC1 == STD_ON) || (OS_SC2 == STD_ON)
			/** @req OS068 */
			ShutdownOS(E_OS_STACKFAULT);
#elif (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)
			/** @req OS396 */
			PROTECTIONHOOK(E_OS_STACKFAULT);
#endif
		}
#endif
}

static inline _Bool Os_TaskOccupiesResources( OsTaskVarType *pcb ) {
	return !(TAILQ_EMPTY(&pcb->resourceHead));
}

static inline _Bool Os_TaskOccupiesSpinlocks( OsTaskVarType *pcb ) {
	return !(TAILQ_EMPTY(&pcb->spinlockHead));
}



#endif /*TASK_I_H_*/
