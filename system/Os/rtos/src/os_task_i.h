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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.3.0 */

#ifndef TASK_I_H_
#define TASK_I_H_

/* ----------------------------[includes]------------------------------------*/
/* ----------------------------[define]--------------------------------------*/


#define PID_IDLE                0U

#define ST_READY                (uint32)1U
#define ST_WAIT_EVENT           ((uint32)1U<<(uint32)1U)
#define ST_SUSPENDED            ((uint32)1U<<(uint32)2U)
#define ST_RUNNING              ((uint32)1U<<(uint32)3U)
#define ST_NOT_STARTED          ((uint32)1U<<(uint32)4U)
#define ST_SLEEPING             ((uint32)1U<<(uint32)5U)
#define ST_WAIT_SEM             ((uint32)1U<<(uint32)6U)
#define ST_WAIT_MUTEX           ((uint32)1U<<(uint32)7U)

#define ST_ISR_RUNNING          (uint32)1U
#define ST_ISR_NOT_RUNNING      (uint32)2U



/* ----------------------------[macro]---------------------------------------*/

#define TASK_CHECK_ID(x)    ((x) < OS_TASK_CNT)


/* ----------------------------[typedef]-------------------------------------*/


struct OsTaskConst;

typedef uint16 state_t;

/* from Os.h types */
typedef TaskType		OsTaskidType;
typedef EventMaskType 	OsEventType;

/* Autosar want relative priorities */
typedef uint32 OsPriorityType;

/* OsTask/OsTaskSchedule */
enum OsTaskSchedule {
    FULL,
    NON
};

/*-----------------------------------------------------------------*/

typedef uint8 proc_type_t;

#define PROC_BASIC				0x1
#define PROC_EXTENDED			0x3

typedef struct {
    void   		*curr;	/* Current stack ptr( at swap time ) */
    void   		*top;	/* Top of the stack( low address )   */
    uint32		size;	/* The size of the stack             */
} OsStackType;


#define SYS_FLAG_HOOK_STATE_EXPECTING_PRE		0
#define SYS_FLAG_HOOK_STATE_EXPECTING_POST   	1

#define REACTIVATE_NORMAL                       (0)
#define REACTIVATE_FROM_CHAINTASK               (1)

typedef struct OsTaskVar {
    OsStackType		stack;

    state_t 		state;
    OsEventType 	ev_wait;			/* Events the task wait for
                                         * ( what events WaitEvent() was called with) */
    OsEventType 	ev_set;				/* Events that are set by SetEvent() on the task */
    OsEventType     ev_react;			/* The events the task may react on */
    uint32   		flags;

    OsPriorityType  activePriority;  	/* Priority of the task, this can be different depening on if the
                                           * task hold resources. Related to priority inversion */
    uint8   		activations;		/* The number of queued activation of a task */
    uint8           reactivation;       /* Retains the history for, after chaining a task the current task shall start from the beginning (when activation count > 1) */
    uint32   		resourceMaskTaken;	/* What resource that are currently held by this task
                                         * Typically (1<<RES_xxx) | (1<<RES_yyy) */

    TAILQ_HEAD(head,OsResource) 	resourceHead;
#if	(OS_NUM_CORES > 1)
    TAILQ_HEAD(shead,OsSpinlock) 	spinlockHead;	/* Occupied spinlocks list */
#endif
    const struct OsTaskConst *constPtr;
    uint32       	regs[16]; 					/* NOTE: Arch specific regs .. make space for them later...*/
#if defined(CFG_KERNEL_EXTRA)
    TAILQ_ENTRY(OsTaskVar) timerEntry;
    /* Absolute timeout in ticks */
    TickType        tmo;
    /* Set timeout value */
    TickType        tmoVal;

    /* return value back to scheduler */
    StatusType  rv;

    OsSemType *semPtr;

    /* Semaphore list */
    STAILQ_ENTRY(OsTaskVar) semEntry;
    /* Mutex list */
    STAILQ_ENTRY(OsTaskVar) mutexEntry;

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
    const char 		 	*name;
    enum OsTaskSchedule scheduling;
    uint32  		    resourceAccess;
    EventMaskType 		eventMask;
    struct OsResource	*resourceIntPtr;	/* pointer to internal resource, NULL if none */
    uint8               activationLimit;

} OsTaskConstType;


/* ----------------------------[function prototypes]-------------------------*/

extern OsTaskVarType Os_TaskVarList[OS_TASK_CNT];
extern GEN_TASK_HEAD;

void Os_Dispatch( OpType op );
StatusType Os_DispatchToSleepWithTmo( OpType op, uint32 tmo);

void Os_TaskMakeReady( 	OsTaskVarType *pcb );
void Os_TaskMakeWaiting( OsTaskVarType *pcb );

void Os_TaskSwapContext  ( OsTaskVarType *old_pcb, OsTaskVarType *new_pcb );
void Os_TaskSwapContextTo( OsTaskVarType *old_pcb, OsTaskVarType *new_pcb );
void Os_TaskStartFromBeginning( OsTaskVarType *pcbPtr );

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

static inline OsTaskVarType * Os_TaskGet( TaskType pid ) {
    return &Os_TaskVarList[pid];
}

static inline ApplicationType Os_TaskGetApplicationOwner( TaskType id ) {
    ApplicationType rv;
    if( id < OS_TASK_CNT ) {
        rv = Os_TaskConstList[id].applOwnerId;
    } else {
        /** @req SWS_Os_00274 */
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
    ASSERT( rPtr->owner == pcbPtr->constPtr->pid );
    rPtr->owner = NO_TASK_OWNER;
    pcbPtr->activePriority = rPtr->old_task_prio;

    if( rPtr->type != RESOURCE_TYPE_INTERNAL ) {
        /* The list can't be empty here */
        ASSERT( !TAILQ_EMPTY(&pcbPtr->resourceHead) );

        /* The list should be popped in LIFO order */
        ASSERT( TAILQ_LAST(&pcbPtr->resourceHead, head) == rPtr );

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

#if !(defined(CFG_SAFETY_PLATFORM) || defined(BUILD_OS_SAFETY_PLATFORM))
static inline void *Os_StackGetUsage( OsTaskVarType *pcb ) {

    uint8 *p = pcb->stack.curr;
    uint8 *end = pcb->stack.top;

    while( (*end == STACK_PATTERN) && (end<p)) {
            end++;
        }
    return (void *)end;
}
#endif

static inline void Os_StackSetStartmark( OsTaskVarType *pcbPtr ) {
	uint8 *start = (uint8 *)pcbPtr->stack.top + pcbPtr->stack.size;
    /* For Tricore architecture NULL in the start of the stack is used for CSA linked list.
     * Hence setting STACK_PATTERN in start-1.
     */
    *(start-1) = STACK_PATTERN;
}
static inline void Os_StackSetEndmark( OsTaskVarType *pcbPtr ) {
	uint8 *end = pcbPtr->stack.top;
    *end = STACK_PATTERN;
}

static inline boolean Os_StackIsEndmarkOk( OsTaskVarType *pcbPtr ) {
	boolean rv = FALSE;
    uint8 *end = pcbPtr->stack.top;
    rv =  ( *end == STACK_PATTERN);
    if( !rv ) {
    	// stack overflow occurred
        OS_DEBUG(D_TASK,"Stack End Mark is bad for %s curr: %p curr: %p\n",
                pcbPtr->constPtr->name,
                pcbPtr->stack.curr,
                pcbPtr->stack.top );
    }
    return rv;
}
static inline boolean Os_StackIsStartmarkOk( OsTaskVarType *pcbPtr ) {
	boolean rv = FALSE;
	uint8 *start = (uint8 *)pcbPtr->stack.top + pcbPtr->stack.size;
    rv =  ( *(start-1) == STACK_PATTERN);
    if( !rv ) {
    	// stack underflow occurred
        OS_DEBUG(D_TASK,"Stack Start Mark is bad for %s curr: %p curr: %p\n",
                pcbPtr->constPtr->name,
                pcbPtr->stack.curr,
                pcbPtr->stack.top );
    }
    return rv;
}

void Os_StackPerformCheck( OsTaskVarType *pcbPtr );

static inline _Bool Os_TaskOccupiesResources( OsTaskVarType *pcb ) {
    return !(TAILQ_EMPTY(&pcb->resourceHead));
}

#if	(OS_NUM_CORES > 1)
static inline _Bool Os_TaskOccupiesSpinlocks( OsTaskVarType *pcb ) {
    return !(TAILQ_EMPTY(&pcb->spinlockHead));
}
#endif //OS_NUM_CORES > 1

void Os_TimerQInsertSorted(OsTaskVarType *tPtr);
boolean Os_TimerQIsPresent(OsTaskVarType *tPtr);

/**
 * returns true if list is empty
 * @return
 */
boolean Os_TimerQIsEmpty( void );


/**
 * returns first task in timer queue (it does NOT remove it)
 * @return
 */
OsTaskVarType * Os_TimerQFirst( void );
/**
 * Remove task from timer queue.
 * @param tPtr
 */
void Os_TimerQRemove(OsTaskVarType *tPtr);


#endif /*TASK_I_H_*/
