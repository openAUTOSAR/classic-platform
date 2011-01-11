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

#ifndef INTERNAL_H_
#define INTERNAL_H_

/* Using internal.h
 *
 * Os.h
 *  |
 *  |--- Os_Cfg.h
 *  |
 *  |--- Std_Types.h
 *  |    |--- Platform_Types.h (std?)
 *  |    |--- Compiler.h       (std?)
 *  |
 *  |--- MemMap.h
 *
 *
 * task.c
 *  |--- Os.h
 *  |--- internal.h
 *
 *
 *
 *           kernel.h (shared types between config and OS source... pointless?)
 *              |
 *     |------------------|
 *  Os_Cfg.c          internal.h
 *
 *
 * API's
 * -------------------------------------
 * Os.h   				- OS API and types
 * os_config_func.h     - Inline API for Os_Cfg.c
 * ext_config.h         - API for os_config_func.h, used by kernel
 * internal.h 			- Internal API for kernel, do NOT expose outside kernel
 *
 * os_types.h           - Internal types for the kernel
 *
 *
 */




#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include "kernel.h"
#include "task_i.h"
#include "ext_config.h"



/*
 * Macros for error handling
 * Registers service id of the erroneous function and the applicable parameters
 * to os_error. Functions that have less than three parameters do not touch
 * os_error.param3. Same rule follows for other parameter counts.
 */

/* Error handling for functions that take no arguments */
#define OS_STD_END(_service_id) \
        goto ok;        \
    err:                \
        os_error.serviceId=_service_id;\
        ERRORHOOK(rv);  \
    ok:                 \
        return rv;

/* Error handling for functions that take one argument */
#define OS_STD_END_1(_service_id, _p1) \
        goto ok;        \
    err:                \
    os_error.serviceId=_service_id;\
        os_error.param1 = (uint32_t) _p1; \
        ERRORHOOK(rv);  \
    ok:                 \
        return rv;

/* Error handling for functions that take two arguments */
#define OS_STD_END_2(_service_id, _p1,_p2) \
        goto ok;        \
    err:                \
        os_error.serviceId=_service_id;\
        os_error.param1 = (uint32_t) _p1; \
        os_error.param2 = (uint32_t) _p2; \
        ERRORHOOK(rv);  \
    ok:                 \
        return rv;

/* Error handling for functions that take three arguments */
#define OS_STD_END_3(_service_id,_p1,_p2,_p3) \
        goto ok;        \
    err:                \
        os_error.serviceId=_service_id;\
        os_error.param1 = (uint32_t) _p1; \
        os_error.param2 = (uint32_t) _p2; \
        os_error.param3 = (uint32_t) _p3; \
        ERRORHOOK(rv);  \
    ok:                 \
        return rv;



/* Called for sequence of error hook calls in case a service
 * does not return with E_OK. Note that in this case the general error hook and the OS-
 * Application specific error hook are called.
 */

#define ERRORHOOK(x) \
	if( os_sys.hooks->ErrorHook != NULL  ) { \
		os_sys.hooks->ErrorHook(x); \
	}


#define PRETASKHOOK() \
	assert( os_sys.curr_pcb->state & ST_RUNNING ); \
	assert( os_sys.curr_pcb->flags == SYS_FLAG_HOOK_STATE_EXPECTING_PRE );  \
	os_sys.curr_pcb->flags = SYS_FLAG_HOOK_STATE_EXPECTING_POST;   \
	if( os_sys.hooks->PreTaskHook != NULL ) { \
		os_sys.hooks->PreTaskHook(); \
	}

#define POSTTASKHOOK() \
	assert( os_sys.curr_pcb->state & ST_RUNNING ); \
	assert( os_sys.curr_pcb->flags == SYS_FLAG_HOOK_STATE_EXPECTING_POST );  \
	os_sys.curr_pcb->flags = SYS_FLAG_HOOK_STATE_EXPECTING_PRE;   \
	if( os_sys.hooks->PostTaskHook != NULL ) { 	\
		os_sys.hooks->PostTaskHook();			\
	}

/*
 * PCB manipulating functions
 */

static inline OsTaskidType get_curr_pid( void ) {
	return os_sys.curr_pcb->pid;
}

static inline OsPcbType *get_curr_pcb( void ) {
	return os_sys.curr_pcb;
}

static inline void set_curr_pcb( OsPcbType *pcb ) {
	os_sys.curr_pcb = pcb;
}

static inline _Bool is_idle_task( OsPcbType *pcb ){
	return (pcb->pid == 0);
}

static inline OsTaskidType get_curr_prio( void ){
	return os_sys.curr_pcb->prio;
}

static inline TickType get_os_tick( void ) {
	return os_sys.tick;
}

#if ( OS_SC3 == STD_ON ) || ( OS_SC4 == STD_ON )
static inline OsApplicationType *get_curr_application( void ) {
	return get_curr_pcb()->application;
}

static inline uint32_t get_curr_application_id( void ) {
	return get_curr_pcb()->application->application_id;
}
#endif

static inline struct OsResource *os_get_resource_int_p( void ) {
	return get_curr_pcb()->resource_int_p;
}

/*
 * Misc
 */

static inline uint32_t os_task_nr_to_mask( uint32_t nr ) {
	return ((uint32_t) 1 << nr); // 701 PC-lint [10.5]: OK om skriver 1u... får då istället: 960 PC-lint [10.5]: Båda ok om skriver 1ul eller castar till uint32_t
}

// task_i.c
OsPcbType *Os_TaskGetTop( void );
OsPcbType *os_find_task( TaskType tid );

// resource.c
void Os_ResourceGetInternal(void );
void Os_ResourceReleaseInternal( void );
void Os_ResourceAlloc( OsResourceType *rPtr, OsPcbType *pcbPtr);
void Os_ResourceFree( OsResourceType *rPtr , OsPcbType *pcbPtr);

void Os_ResourceInit( void );


static inline void Os_ResourceFreeAll( OsPcbType *pcbPtr ) {
	OsResourceType *rPtr;

	/* Pop the queue */
	TAILQ_FOREACH(rPtr, &pcbPtr->resource_head, listEntry ) {
		Os_ResourceFree(rPtr,pcbPtr);
	}
}

#if 0
/**
 *
 * @return 1 - if any resources were found.
 */
static inline _Bool Os_ResourceCheckAndRelease( OsPcbType *pcb )  {
	_Bool rv = 0;
	if( !TAILQ_EMPTY(&pcb->resource_head) ) {
		OsResourceType *rPtr;

		TAILQ_FOREACH(rPtr, &pcb->resource_head, listEntry ) {
			ReleaseResource(rPtr->nr);
			/* Requirements are a little fuzzy here, no explicit
			 * requirement for this.
			 *
			 * For OSEK this is a req.
			 */
			ERRORHOOK(E_OS_RESOURCE);
			rv = 1;
		}
	}
	return rv;
}
#endif

static inline _Bool Os_TaskOccupiesResources( OsPcbType *pcb ) {
	return !(TAILQ_EMPTY(&pcb->resource_head));
}

/*
static inline void Os_GetSchedulerResource() {
	os_sys.scheduler_lock = 1;
}

static inline void Os_ReleaseSchedulerResource() {
	os_sys.scheduler_lock = 0;
}
*/
/*
static inline _Bool Os_SchedulerResourceIsOccupied() {
#if 0
	return (os_sys.resScheduler.owner != NO_TASK_OWNER );
#else
	return (os_sys.scheduler_lock == 1);
#endif
}
*/
#define NO_TASK_OWNER 	(TaskType)(~0)

static inline _Bool Os_SchedulerResourceIsFree() {
#if 1
	return (os_sys.resScheduler.owner == NO_TASK_OWNER );
#else
	return (os_sys.scheduler_lock == 0);
#endif
}

// Create.c
OsPcbType * os_alloc_new_pcb( void );

void os_dispatch(void);

void OsTick( void );

#if defined(CFG_ARM_CM3)
void Os_Isr_cm3( void *isr_p );
void TailChaining(void *stack);
#endif

void *Os_Isr( void *stack, void *pcb_p );
void Os_Dispatch( uint32_t op );

#define STACK_PATTERN	0x42

static inline void *Os_StackGetUsage( OsPcbType *pcb ) {

	uint8_t *p = pcb->stack.curr;
	uint8_t *end = pcb->stack.top;

	while( (*end == STACK_PATTERN) && (end<p)) {
			end++;
		}
	return (void *)end;
}

static inline void Os_StackSetEndmark( OsPcbType *pcbPtr ) {
	uint8_t *end = pcbPtr->stack.top;
	*end = STACK_PATTERN;
}

static inline _Bool Os_StackIsEndmarkOk( OsPcbType *pcbPtr ) {
	_Bool rv;
	uint8_t *end = pcbPtr->stack.top;
	rv =  ( *end == STACK_PATTERN);
	if( !rv ) {
		OS_DEBUG(D_TASK,"Stack End Mark is bad for %s curr: %p curr: %p\n",
				pcbPtr->name,
				pcbPtr->stack.curr,
				pcbPtr->stack.top );
	}
	return rv;
}

static inline void Os_StackPerformCheck( OsPcbType *pcbPtr ) {
#if (OS_STACK_MONITORING == 1)
		if( !Os_StackIsEndmarkOk(pcbPtr) ) {
#if (  OS_SC1 == 1) || (  OS_SC2 == 1)
			/** @req OS068 */
			ShutdownOS(E_OS_STACKFAULT);
#else
#error SC3 or SC4 not supported. Protection hook should be called here
#endif
		}
#endif
}


int Os_CfgGetTaskCnt(void);
void Os_ContextReInit( OsPcbType *pcbPtr );


static inline _Bool Os_IrqAnyDisabled( void ) {
	return ((Os_IntDisableAllCnt | Os_IntSuspendAllCnt | Os_IntSuspendOsCnt) != 0);
}

static inline void Os_IrqClearAll( void ) {
	Os_IntDisableAllCnt = 0;
	Os_IntSuspendAllCnt = 0;
	Os_IntSuspendOsCnt = 0;
}


#endif /*INTERNAL_H_*/
