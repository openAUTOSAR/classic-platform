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

/* ----------------------------[includes]------------------------------------*/

#include "irq.h"
#include "irq_types.h"
#include "os_i.h"

#if defined(CFG_TC2XX) || defined(CFG_TC3XX)
#include "IfxCpu.h"
#endif

/* ----------------------------[private define]------------------------------*/

#define VECTOR_ILL		0xff
#define ACK_INTERRUPT(_x) Irq_EOI(_x);

/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/

#if defined(CFG_OS_ISR_HOOKS)
static inline void Os_PreIsrHookStartI( OsIsrVarType *isrP );
static inline void Os_PreIsrHookStartPreemptI( OsIsrVarType *isrP );
static inline void Os_PostIsrHookPreemptI( OsIsrVarType *isrP );
static inline void Os_PostIsrHookTerminateI( OsIsrVarType *isrP );
#endif
static inline void Os_IsrResourceFreeAll( OsIsrVarType *isrPtr );
static inline boolean Os_IsrOccupiesResources(const  OsIsrVarType *isrPtr );

/* ----------------------------[private variables]---------------------------*/

extern uint8 Os_VectorToIsr[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS];

#if defined(CFG_ARMV7_M)
extern uint32 Os_ArchNestedIsr;
#endif

ISRType isrCnt = 0;

#if OS_ISR_CNT!=0
extern const OsIsrConstType Os_IsrConstList[OS_ISR_CNT];
#endif

#if OS_ISR_MAX_CNT!=0
OsIsrVarType Os_IsrVarList[OS_ISR_MAX_CNT];
#endif

uint8 Os_IsrStack[OS_NUM_CORES][OS_INTERRUPT_STACK_SIZE] __balign(0x10);

// IMPROVEMENT: remove. Make soft links or whatever
#if defined(CFG_ARM_CM3)
#include "irq_types.h"
#endif

/* ----------------------------[private functions]---------------------------*/

#if defined(CFG_OS_ISR_HOOKS)
/**
 * Call external hook function.
 *
 * @param isrP Pointer to Isr
 */
static inline void Os_PreIsrHookStartI( OsIsrVarType *isrP ) {
    Os_PreIsrHook(isrP->id);
}

static inline void Os_PreIsrHookStartPreemptI( OsIsrVarType *isrP ) {
#if defined(CFG_T1_ENABLE)
    /* T1 figures out this state, so it does not want it */
#else
    Os_PreIsrHook(isrP->id);
#endif
}

/**
 * Call external hook function.
 *
 * @param isrP Pointer to Isr
 */
static inline void Os_PostIsrHookPreemptI( OsIsrVarType *isrP ) {
#if defined(CFG_T1_ENABLE)
    /* T1 figures out this state, so it does not want it */
#else
    Os_PostIsrHook(isrP->id);
#endif
}

/** */
static inline void Os_PostIsrHookTerminateI( OsIsrVarType *isrP ) {
    Os_PostIsrHook(isrP->id);
}


#endif

static inline void Os_IsrResourceFreeAll( OsIsrVarType *isrPtr ) {
    OsResourceType *rPtr;

    /* Pop the queue */
    /*lint -e{9036} MISRA:EXTERNAL_FILE::[MISRA 2012 Rule 14.4, required] */
    TAILQ_FOREACH(rPtr, &isrPtr->resourceHead, listEntry ) {
        Os_IsrResourceRemove(rPtr,isrPtr);
    }
}

static inline boolean Os_IsrOccupiesResources(const  OsIsrVarType *isrPtr ) {
    return (boolean)(!(TAILQ_EMPTY(&isrPtr->resourceHead)));
}

/* ----------------------------[public functions]----------------------------*/

void Os_IsrResourceAdd( struct OsResource *rPtr, OsIsrVarType *isrPtr) {
    /* A resource can be owned by either a Task or an Isr. The owner element
     * is of TaskType, but let the Isr re-use the same. Before entering this
     * function, the resource should have no registered used anyway.
     */
    rPtr->owner = (TaskType)isrPtr->id;

    if( rPtr->type != RESOURCE_TYPE_INTERNAL ) {
        /*lint -e{9012, 9036} MISRA:EXTERNAL_FILE::[MISRA 2012 Rule 15.6, required], [MISRA 2012 Rule 14.4, required] */
        TAILQ_INSERT_TAIL(&isrPtr->resourceHead, rPtr, listEntry);
    }
}

void Os_IsrResourceRemove( struct OsResource *rPtr , OsIsrVarType *isrPtr) {
    ASSERT( rPtr->owner == isrPtr->id );
    rPtr->owner = NO_TASK_OWNER;

    if( rPtr->type != RESOURCE_TYPE_INTERNAL ) {
        /* The list can't be empty here */
        ASSERT( !TAILQ_EMPTY(&isrPtr->resourceHead) );

        /* The list should be popped in LIFO order */
        /*lint -e{929, 740, 826} MISRA:EXTERNAL_FILE::[MISRA 2012 Rule 11.3, required], [MISRA 2012 Rule 11.5, advisory],
         * 											  [MISRA 2012 Rule 15.6, required], [MISRA 2012 Rule 14.4, required], [MISRA 2012 Rule 1.3, required]*/
        ASSERT( TAILQ_LAST(&isrPtr->resourceHead, head) == rPtr );

        /* Remove the entry */
        /*lint -e{9012, 9036} MISRA:EXTERNAL_FILE::[MISRA 2012 Rule 15.6, required], [MISRA 2012 Rule 14.4, required] */
        TAILQ_REMOVE(&isrPtr->resourceHead, rPtr, listEntry);
    }
}

void Os_IsrAddWithId( const OsIsrConstType * isrPtr, ISRType id ) {
    Os_IsrVarList[id].constPtr = isrPtr;
    Os_IsrVarList[id].id = id;
#if defined(CFG_OS_ISR_HOOKS)
    Os_IsrVarList[id].preemtedId = INVALID_ISR;
#endif
    Os_VectorToIsr[isrPtr->vector + IRQ_INTERRUPT_OFFSET ] = (uint8)id;
#if defined(CFG_TMS570) || defined(CFG_ARMV7_M) || defined(CFG_TRAVEO)
    Irq_EnableVector2( isrPtr->entry, isrPtr->vector, isrPtr->type,  isrPtr->priority, Os_ApplGetCore(isrPtr->appOwner) );
#elif defined(CFG_TC2XX) || defined(CFG_TC3XX)
    Irq_EnableVector2( isrPtr->entry, isrPtr->vector, isrPtr->type,  isrPtr->priority, (int)IfxCpu_getCoreId() );
#else
    Irq_EnableVector( isrPtr->vector, isrPtr->priority, Os_ApplGetCore(isrPtr->appOwner )  );
#endif

}

void Os_IsrInit( void ) {

    Irq_Init();

    isrCnt = OS_ISR_CNT;
    /* May be possible to do this in another way*/
    memset(Os_VectorToIsr,VECTOR_ILL,(uint32)NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS);

#if OS_ISR_CNT != 0
    /* Attach the interrupts */
    GetSpinlock(OS_SPINLOCK);
    for (ISRType i = 0; i < isrCnt; i++) {
        Os_IsrAddWithId(&Os_IsrConstList[i],i);
        /* Initialize the Isr linked list for resource handling */
        /*lint -e{9036} MISRA:EXTERNAL_FILE::[MISRA 2012 Rule 14.4, required] */
        TAILQ_INIT(&Os_IsrVarList[i].resourceHead);
    }
    ReleaseSpinlock(OS_SPINLOCK);
#endif
}

/**
 * Adds an ISR to a list of Isr's. The ISRType (id) is returned
 * for the "created" ISR.
 *
 * @param isrPtr Pointer to const data holding ISR information.
 * @return
 */
ISRType Os_IsrAdd( const OsIsrConstType * isrPtr ) {
    ISRType id;
    ISRType installedId;

    ASSERT( isrPtr != NULL_PTR );
    ASSERT( (isrPtr->vector + IRQ_INTERRUPT_OFFSET) < NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS );

    /* Check if we already have installed it */
    installedId = (ISRType)Os_VectorToIsr[isrPtr->vector + IRQ_INTERRUPT_OFFSET ];

    if( installedId != VECTOR_ILL ) {
        /* The vector is already installed */
        id = installedId;
    } else {
        /* It a new vector */
#if (OS_NUM_CORES > 1)
        GetSpinlock(OS_RTE_SPINLOCK);
#endif
        id = isrCnt;
        isrCnt++;
#if (OS_NUM_CORES > 1)
        ReleaseSpinlock(OS_RTE_SPINLOCK);
#endif
		/* Since OS_ISR_MAX_CNT defines the allocation limit for Os_IsrVarList,
		 * we must not allocate more IDs than that */
        ASSERT(id<(ISRType)OS_ISR_MAX_CNT);

        Os_IsrAddWithId(isrPtr,id);
    }

    return id;
}

/*
 * Resources:
 *   Irq_VectorTable[]
 *   Irq_IsrTypeTable[]
 *   Irq_PriorityTable[]
 *
 *   exception table
 *   interrupt table
 *
 * Usual HW resources.
 * - prio in HW (ppc and arm (even cortex m4))
 *
 *
 * TOOL GENERATES ALL
 *   Irq_VectorTable   CONST
 *   Irq_IsrTypeTable  CONST
 *   Irq_PriorityTable CONST  Can probably be a table with ISR_MAX number
 *                            of for a CPU with prio registers.  For masking
 *                            CPUs it's better to keep an array to that indexing
 *                            can be used.
 *
 *   The problem with this approach is that the tool needs to know everything.
 *
 * TOOL GENERATES PART
 *   Irq_VectorTable   VAR     Since we must add vectors later
 *   Irq_IsrTypeTable  VAR     Since we must add vectors later
 *   Irq_PriorityTable VAR
 *
 */

/*-----------------------------------------------------------------*/

void Os_IsrGetStackInfo( OsIsrStackType *stack ) {
    stack->top = Os_IsrStack[GetCoreID()];
    stack->size = sizeof(Os_IsrStack[GetCoreID()]);
}

OsIsrVarType *Os_IsrGet( ISRType id ) {

    OsIsrVarType *rv = NULL_PTR;

#if OS_ISR_MAX_CNT != 0
    if( id < isrCnt ) {
        rv = &Os_IsrVarList[id];
    }
#else
    (void)id;
#endif

return rv;
}

ApplicationType Os_IsrGetApplicationOwner( ISRType id ) {
    /** @req SWS_Os_00274 */
    ApplicationType rv = (uint32)INVALID_OSAPPLICATION;

#if (OS_ISR_MAX_CNT!=0)
    if( id < isrCnt ) {
        rv = Os_IsrGet(id)->constPtr->appOwner;
    }
#else
    (void)id;
#endif
    return rv;
}

void *Os_Isr( void *stack, uint16 vector ) {

    OsIsrVarType *isrPtr =  &Os_IsrVarList[Os_VectorToIsr[vector]];
    OsTaskVarType *taskPtr = NULL_PTR;
    OsIsrVarType *oldIsrPtr = NULL_PTR;
#if defined(CFG_TC2XX) || defined(CFG_TC3XX)
    uint32 *CSAPtr = NULL_PTR;
#endif
    ASSERT( isrPtr != NULL_PTR );

    if( isrPtr->constPtr->type == ISR_TYPE_1) {
        oldIsrPtr = OS_SYS_PTR->currIsrPtr;
        OS_SYS_PTR->currIsrPtr = isrPtr;
        OS_SYS_PTR->intNestCnt++;
        Irq_Enable();
        isrPtr->constPtr->entry();
        Irq_Disable();
        OS_SYS_PTR->intNestCnt--;
        OS_SYS_PTR->currIsrPtr = oldIsrPtr;
#if !defined(CFG_TMS570)
        ACK_INTERRUPT(vector);
#endif
        return stack; /*lint !e904 OTHER return since the rest of the function is applicable for ISR_TYPE_2*/
    } else {
        ASSERT( isrPtr->constPtr->type == ISR_TYPE_2 );
    }

    /* Check if we interrupted a task or ISR */
#if defined(CFG_ARMV7_M)
    if( (OS_SYS_PTR->intNestCnt + Os_ArchNestedIsr) == 0 ) {
#else
    if( OS_SYS_PTR->intNestCnt == 0 ) {
#endif
        /* We interrupted a task */
        POSTTASKHOOK();

        /* Save info for preempted pcb */
        taskPtr = Os_SysTaskGetCurr();
#ifndef CFG_ARMV7_M
        taskPtr->stack.curr = stack;
#endif
        taskPtr->state = ST_READY;
        OS_DEBUG(D_TASK,"Preempted %s\n",taskPtr->constPtr->name);

        Os_StackPerformCheck(taskPtr);

#if defined (CFG_ARMV7_M)
    } else if (Os_ArchNestedIsr == 1) {
        /* Do nothing.
         * We have a run the ISR but it's not terminated yet. */
#endif
    } else {
        /* We interrupted an ISR, save it */
        oldIsrPtr = OS_SYS_PTR->currIsrPtr;
#if defined(CFG_OS_ISR_HOOKS)
        isrPtr->preemtedId = oldIsrPtr->id;
        Os_PostIsrHookPreemptI(oldIsrPtr);
#endif
    }

    OS_SYS_PTR->intNestCnt++;

    isrPtr->state = ST_ISR_RUNNING;
    OS_SYS_PTR->currIsrPtr = isrPtr;

    Irq_SOI();
#if defined(CFG_ARM_CR4) || defined(CFG_TMS570LC43X)
    Irq_SOI3(isrPtr->constPtr->priority);
#endif

#if defined(CFG_OS_ISR_HOOKS)
    Os_PreIsrHookStartI(isrPtr);
#endif

#if defined(CFG_TRAVEO)
    isrPtr->constPtr->entry();
#else
#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
    {
        OsAppVarType *aP = Os_ApplGet(isrPtr->constPtr->appOwner);

        if( aP->trusted == 0) {
            /* Os_ArchCallIsrEntry will lock and unlock interrupts (so don't do that here )
             *
             */
            aP->nestCnt++;
            Os_ArchCallIsrEntry(aP,isrPtr->constPtr->entry);
            Os_AppIsrStackPerformCheck( isrPtr->constPtr->appOwner );
            aP->nestCnt--;
        } else {
            Irq_Enable();
            isrPtr->constPtr->entry();
            Irq_Disable();
        }
    }
#else
    Irq_Enable();
    isrPtr->constPtr->entry();
    Irq_Disable();
#endif
#endif

#if defined(CFG_OS_ISR_HOOKS)
    Os_PostIsrHookTerminateI(isrPtr);
    if( isrPtr->preemtedId != INVALID_ISR ) {
        OS_SYS_PTR->currIsrPtr = &Os_IsrVarList[isrPtr->preemtedId];
        Os_PreIsrHookStartPreemptI( OS_SYS_PTR->currIsrPtr );
        isrPtr->preemtedId = INVALID_ISR;
    }
#endif

    /* Check so that ISR2 haven't disabled the interrupts */
    /** @req SWS_Os_00368 */
    if( Os_SysIntAnyDisabled() ) {
        Os_SysIntClearAll();
        Os_CallErrorHook(E_OS_DISABLEDINT);
    }

    /* Check so that the ISR2 have called ReleaseResource() for each GetResource() */
    /** @req SWS_Os_00369 */
    if( Os_IsrOccupiesResources(isrPtr) == TRUE) {
        Os_IsrResourceFreeAll(isrPtr);
        Os_CallErrorHook(E_OS_RESOURCE);
    }

    isrPtr->state = ST_ISR_NOT_RUNNING;
    OS_SYS_PTR->currIsrPtr = isrPtr;

    ACK_INTERRUPT(vector);

    --OS_SYS_PTR->intNestCnt;

#if defined (CFG_ARMV7_M)
    return NULL_PTR;
#else
    // We have preempted a task
    if( (OS_SYS_PTR->intNestCnt == 0) ) {
        OsTaskVarType *new_pcb  = Os_TaskGetTop();

        Os_StackPerformCheck(new_pcb);

        //lint -e{9007} MISRA False positive. No side effects of Os_SchedulerResourceIsFree
        if(     (new_pcb == OS_SYS_PTR->currTaskPtr) ||
                (OS_SYS_PTR->currTaskPtr->constPtr->scheduling == NON) ||
                (!Os_SchedulerResourceIsFree()) )
        {
            /* Just bring the preempted task back to running */
            OS_SYS_PTR->currTaskPtr->state = ST_RUNNING;
            PRETASKHOOK();
#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
            // Set MPU memory regions for the resuming application
            Os_MMSetUserMode(new_pcb);
#endif
#ifdef CFG_T1_ENABLE
            Os_StartTaskHook(OS_SYS_PTR->currTaskPtr->constPtr->pid);
#endif
        } else {
            OS_DEBUG(D_TASK,"Found candidate %s\n",new_pcb->constPtr->name);
#if defined(CFG_TC2XX) || defined(CFG_TC3XX)
            /* Mark context with 0, so that we can free it later */
            CSAPtr =  (uint32 *)CSA_TO_ADDRESS(_mfcr(CPU_PCXI));
            CSAPtr[0] = 0;
#endif
            Os_TaskSwapContextTo(NULL_PTR,new_pcb);
        }
    } else {
        /* We have a nested interrupt */
#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
        Os_MMSetUserModeIsr(oldIsrPtr);
#endif

        /* Restore current running ISR from stack */
        OS_SYS_PTR->currIsrPtr = oldIsrPtr;
    }

    return stack;
#endif /*CFG_ARMV7_M*/
}

void Os_Arc_GetIsrInfo( Arc_PcbType *pcbPtr, ISRType isrId ) {
    const OsIsrVarType *isrPtr = Os_IsrGet(isrId);

    if( isrPtr != NULL_PTR ) {
        strncpy(pcbPtr->name,Os_IsrGet(isrId)->constPtr->name,OS_ARC_PCB_NAME_SIZE);
    }

}

ISRType Os_Arc_GetIsrCount( void ) {
    return isrCnt;
}

/**
 * This service returns the identifier of the currently executing ISR
 *
 * If its caller is not a category 2 ISR (or Hook routines called
 * inside a category 2 ISR), GetISRID() shall return INVALID_ISR.
 *
 * @return
 */
/** @req SWS_Os_00511 OS provides the service GetISRID() */
/** @req SWS_Os_00515 GetISRID() available in all Scalability Classes. */
ISRType GetISRID( void ) {

	ISRType id = INVALID_ISR;
	/** @req SWS_Os_00264 */
	if(OS_SYS_PTR->intNestCnt == 0 ) {
	  id = INVALID_ISR;
	}
	else if( Os_SysIsrGetCurr()->constPtr->type == ISR_TYPE_1 ) {
	  id = INVALID_ISR;
	}
	else {
	  id = (ISRType)Os_SysIsrGetCurr()->id;
	}
	/** @req SWS_Os_00263 */
	return id;
}
