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

#define ACK_INTERRUPT(_x) Irq_EOI(_x);


 #if defined(CFG_LOG) && defined(LOG_OS_ISR)
     #define _LOG_NAME_ "Os_Isr"
 #endif
 #include "log.h"

/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/

#if defined(CFG_OS_ISR_HOOKS)
static inline void Os_PreIsrHookStartI( OsIsrVarType *isrP );
static inline void Os_PreIsrHookStartPreemptI( OsIsrVarType *isrP );
static inline void Os_PostIsrHookPreemptI( OsIsrVarType *isrP );
static inline void Os_PostIsrHookTerminateI( OsIsrVarType *isrP );
#endif

static inline ISRType Os_CheckISRinstalled( const OsIsrConstType * isrPtr );


/* ----------------------------[private variables]---------------------------*/
#if (!defined(CFG_TC2XX) && !defined(CFG_TC3XX))
extern uint8 Os_VectorToIsr[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS];
#endif

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


static inline ISRType Os_CheckISRinstalled( const OsIsrConstType * isrPtr ) {
	ISRType id;
#if defined(CFG_TC2XX) || defined(CFG_TC3XX)
	id = (ISRType)Irq_GetISRinstalledId((uint8) isrPtr->priority);
#else
	id = (ISRType)Os_VectorToIsr[isrPtr->vector + IRQ_INTERRUPT_OFFSET ];
#endif
	return id;
}

static inline OsIsrVarType * Os_GetIsrVarType(uint16 isrIndex, uint16 *isrVector ) {
	OsIsrVarType *isrVPtr = NULL_PTR;
	/*lint -e578 MISRA:CONFIGURATION:initialising index based on configuration:[MISRA 2012 Rule 5.8, required]*/
	uint16 index;
#if defined(CFG_TC2XX) || defined(CFG_TC3XX)
    index = isrIndex;
#else
    index = Os_VectorToIsr[isrIndex];
    *isrVector = isrIndex;
#endif
    ASSERT(index != VECTOR_ILL);
    ASSERT(index < OS_ISR_MAX_CNT);

    isrVPtr = &Os_IsrVarList[index]; /*lint !e838, OK redefinition */
#if defined(CFG_TC2XX) || defined(CFG_TC3XX)
    *isrVector = (uint16)isrVPtr->constPtr->vector;
#endif

    return isrVPtr;
}

/* ----------------------------[public functions]----------------------------*/


void Os_IsrAddWithId( const OsIsrConstType * isrPtr, ISRType id ) {
    Os_IsrVarList[id].constPtr = isrPtr;
    Os_IsrVarList[id].id = id;
#if defined(CFG_OS_ISR_HOOKS)
    Os_IsrVarList[id].preemtedId = INVALID_ISR;
#endif
#if (!defined(CFG_TC2XX) && !defined(CFG_TC3XX))
    Os_VectorToIsr[isrPtr->vector + IRQ_INTERRUPT_OFFSET ] = (uint8)id;
#endif
#if defined(CFG_TMS570) || defined(CFG_ARMV7_M) || defined(CFG_TRAVEO)
    /*lint -e732 MISRA:OTHER:loss of sign:[MISRA 2004 Info, advisory] */
    Irq_EnableVector2( isrPtr->entry, isrPtr->vector, isrPtr->type,  isrPtr->priority, Os_ApplGetCore(isrPtr->appOwner) );
#elif defined(CFG_TC2XX) || defined(CFG_TC3XX)
    Irq_EnableVector2( isrPtr->entry, isrPtr->vector, (uint16)id,  isrPtr->priority, (uint8)IfxCpu_getCoreId() );
#else
    Irq_EnableVector( isrPtr->vector, isrPtr->priority, Os_ApplGetCore(isrPtr->appOwner )  );
#endif

}

void Os_IsrInit( void ) {

    Irq_Init();

    isrCnt = OS_ISR_CNT;
    /* May be possible to do this in another way*/
#if (!defined(CFG_TC2XX) && !defined(CFG_TC3XX))
    memset(Os_VectorToIsr,VECTOR_ILL,(uint32)NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS);
#endif
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
    installedId = Os_CheckISRinstalled( isrPtr );

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


#if !defined(BUILD_OS_SAFETY_PLATFORM)
void Os_IsrRemove( sint16 vector, sint16 type, uint8 priority, ApplicationType app ) {
#if defined(CFG_TC2XX) || defined(CFG_TC3XX)
    /* Not suppored yet */
#else
    (void)app;
    (void)type;
    (void)priority;

    Os_VectorToIsr[vector + IRQ_INTERRUPT_OFFSET ] = VECTOR_ILL;
#endif
}
#endif


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


#if ((OS_SC3 == STD_ON) || (OS_SC4 == STD_ON))
/**
   @brief  Check stackmarks for interrupt stack are ok.
 * @return FALSE if the end-mark is not ok.
 */
static boolean Os_IsIsrStackmarkOk( void  )
{
    boolean rv = FALSE;
    const uint8 *top = Os_IsrStack[GetCoreID()];
    const uint8 *bottom = top + sizeof(Os_IsrStack[GetCoreID()]);     /*lint !e9016 MISRA:PERFORMANCE:Calculation size:[MISRA 2012 Rule 18.4, advisory] */

    if( ( *top == STACK_PATTERN ) && ( *(bottom - 1UL) == STACK_PATTERN ) ) {
        rv = TRUE;
    }

    return rv;
}


/**
 * @brief   Perform stack check on system ISR
 *
 * @param id  The ID of the application
 */
void Os_IsrStackPerformCheck( void ) {
#if (OS_STACK_MONITORING == 1)
    /*lint --e{9036} MISRA:OTHER:Os_IsIsrStackmarkOk() return boolean value:[MISRA 2012 Rule 14.4, required]*/
        if( Os_IsIsrStackmarkOk() == FALSE ) {
            /** @req SWS_Os_00396
             * If a stack fault is detected by stack monitoring AND the configured scalability
             * class is 3 or 4, the Operating System module shall call the ProtectionHook() with
             * the status E_OS_STACKFAULT.
             * */
            Os_CallProtectionHook(E_OS_STACKFAULT);
        }
#endif  /* (OS_STACK_MONITORING == 1) */
}

#endif

void *Os_Isr( void *stack, uint16 isrTableIndex)
{
    OsIsrVarType *isrPtr = NULL_PTR;
    OsTaskVarType *taskPtr = NULL_PTR;
    OsIsrVarType *oldIsrPtr = NULL_PTR;
    uint16 vector = 0;
#if defined(CFG_TC2XX) || defined(CFG_TC3XX)
    uint32 *CSAPtr = NULL_PTR;
#endif

    /*lint -e838 MISRA:OTHER:default value:[MISRA 2004 Info, advisory] */
    isrPtr = Os_GetIsrVarType(isrTableIndex, &vector);
    ASSERT( isrPtr != NULL_PTR );


#if defined(CFG_TMS570)
    ASSERT( isrPtr->constPtr->type == ISR_TYPE_2 );
#else
    if( isrPtr->constPtr->type == ISR_TYPE_1) {
        oldIsrPtr = OS_SYS_PTR->currIsrPtr;
        OS_SYS_PTR->currIsrPtr = isrPtr;
        OS_SYS_PTR->intNestCnt++;
        Irq_Enable();
        isrPtr->constPtr->entry();
        Irq_Disable();
        OS_SYS_PTR->intNestCnt--;
        OS_SYS_PTR->currIsrPtr = oldIsrPtr;
        /*lint -e713 MISRA:OTHER:casting unsigned short to short:[MISRA 2004 Info, advisory]*/
        ACK_INTERRUPT(vector);
        return stack; /*lint !e904 MISRA:OTHER:return since the rest of the function is applicable for ISR_TYPE_2:[MISRA 2012 Rule 15.5, advisory]*/
    }else{
        ASSERT( isrPtr->constPtr->type == ISR_TYPE_2 );
    }
#endif

    /* Check if we interrupted a task or ISR */
#if defined(CFG_ARMV7_M)
    if( (OS_SYS_PTR->intNestCnt + Os_ArchNestedIsr) == 0 ) {
#else
    if( OS_SYS_PTR->intNestCnt == 0 ) {
#endif
        /* We interrupted a task */
        Os_CallPostTaskHook();

        /* Save info for preempted pcb */
        taskPtr = Os_SysTaskGetCurr();
#ifndef CFG_ARMV7_M
        taskPtr->stack.curr = stack;
#endif
        taskPtr->state = ST_READY;
        OS_DEBUG(D_TASK,"Preempted %s\n",taskPtr->constPtr->name);

#if ((OS_SC3 == STD_ON) || (OS_SC4 == STD_ON))
        /* Save current application */
        ApplicationType curAppl = OS_SYS_PTR->currApplId;
        OS_SYS_PTR->currApplId = INVALID_OSAPPLICATION;
#endif

        Os_StackPerformCheck(taskPtr);

#if ((OS_SC3 == STD_ON) || (OS_SC4 == STD_ON))
        OS_SYS_PTR->currApplId = curAppl;
#endif

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
    LOG_S_U32("entry: ",vector);
    {

        OsAppVarType *aP = Os_ApplGet(isrPtr->constPtr->appOwner);

        if( aP->trusted == 0) {
            /* Os_ArchCallIsrEntry will lock and unlock interrupts (so don't do that here ) */

            OsAppVarType *paP = NULL_PTR;

            LOG_S_U32("app: ",isrPtr->constPtr->appOwner);

            /* Get preemted application */
            if( oldIsrPtr != NULL_PTR ) {
                paP = Os_ApplGet(oldIsrPtr->constPtr->appOwner);
                LOG_S_U32("papp: ",oldIsrPtr->constPtr->appOwner);
            } else {
                ASSERT(taskPtr != NULL_PTR );
                paP = Os_ApplGet(taskPtr->constPtr->applOwnerId);
                LOG_S_U32("papp: ",taskPtr->constPtr->applOwnerId);
            }

            aP->nestCnt++;
            OS_SYS_PTR->currApplId = aP->appId;
            Os_ArchCallIsrEntry(aP,isrPtr->constPtr->entry,paP);
            Os_AppIsrStackPerformCheck( isrPtr->constPtr->appOwner );
            aP->nestCnt--;
        } else {

            OS_SYS_PTR->currApplId = aP->appId;
            Irq_Enable();
            isrPtr->constPtr->entry();
            Os_IsrStackPerformCheck();
            Irq_Disable();
        }
    }
    LOG_S_U32("exit: ",vector);
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

    isrPtr->state = ST_ISR_NOT_RUNNING;
    OS_SYS_PTR->currIsrPtr = isrPtr;
    /*lint -e713 MISRA:OTHER:casting unsigned short to short:[MISRA 2004 Info, advisory]*/
    ACK_INTERRUPT(vector);

    --OS_SYS_PTR->intNestCnt;

#if defined (CFG_ARMV7_M)
    return NULL_PTR;
#else
    // We have preempted a task
    if( (OS_SYS_PTR->intNestCnt == 0) ) {
        OsTaskVarType *new_pcb  = Os_TaskGetTop();

        Os_StackPerformCheck(new_pcb);

        /*lint -e{9007} MISRA:FALSE_POSITIVE:No side effects of Os_SchedulerResourceIsFree:[MISRA 2012 Rule 13.5, required]*/
        if(     (new_pcb == OS_SYS_PTR->currTaskPtr) ||
                (OS_SYS_PTR->currTaskPtr->constPtr->scheduling == NON) )
        {
            /* Just bring the preempted task back to running */
            OS_SYS_PTR->currTaskPtr->state = ST_RUNNING;
            OS_SYS_PTR->currApplId = new_pcb->constPtr->applOwnerId;
            Os_CallPreTaskHook();
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
            /*lint -e{9027} MISRA:FALSE_POSITIVE:ASM and bitwise operations:[MISRA 2012 Rule 10.1, required]*/
            /*lint -e{9033} MISRA:FALSE_POSITIVE:ASM and bitwise operations:[MISRA 2012 Rule 10.8, required]*/
            /*lint -e{931} MISRA:FALSE_POSITIVE:ASM and bitwise operations:[MISRA 2012 Rule 13.2, required]*/
            /*lint -e{923} MISRA:FALSE_POSITIVE:ASM and bitwise operations:[MISRA 2012 Rule 11.6, required]*/
            CSAPtr =  (uint32 *)CSA_TO_ADDRESS(_mfcr(CPU_PCXI));
            CSAPtr[0] = 0;
#endif
            Os_TaskSwapContextTo(NULL_PTR,new_pcb);
        }
    } else {
        /* We have a nested interrupt */
        OS_SYS_PTR->currApplId = oldIsrPtr->constPtr->appOwner;
        OS_SYS_PTR->currIsrPtr = oldIsrPtr;

#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
        // Set MPU memory regions for the resuming application
    	Os_MMSetUserModeIsr(oldIsrPtr);
#endif
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
#if defined(CFG_ARM_CR4) || defined(CFG_ARM_CR5)
	  /* The pre-/post on cortex-r is very short so we don't record any information */
#else
	} else if( Os_SysIsrGetCurr()->constPtr->type == ISR_TYPE_1 ) {
	  id = INVALID_ISR;
#endif
	} else {
	  id = (ISRType)Os_SysIsrGetCurr()->id;
	}
	/** @req SWS_Os_00263 */
	return id;
}
