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

extern uint32_t os_dbg_mask;


/*
 * Configuration tree:
 * USE_OS_DEBUG               - Turn on/off all Os_DbgPrintf()
 * SELECT_OS_CONSOLE          - Select console
 * USE_RAMLOG                 - Compile ramlog code...
 *
 *
 * Default is to print to RAMLOG.
 *
 *
 * Use cases:
 * 1. We don't have a RAMLOG (low on RAM) so we want to print to serial console:
 *     #define CFG_OS_DEBUG = STD_ON
 *     #define USE_SERIAL_PORT
 *     #define SELECT_OS_CONSOLE=TTY_SERIAL0
 * 2. We have a RAMLOG but we have a debugger connected and want the OS debug
 *    to go there instead:
 *     #define CFG_OS_DEBUG = STD_ON
 *     #define USE_RAMLOG
 *     #define USE_TTY_T32
 *     #define SELECT_OS_CONSOLE=TTY_T32
 * 3. We have only the ramlog:
 *     #define CFG_OS_DEBUG = STD_ON
 *     #define USE_RAMLOG
 *     #define SELECT_OS_CONSOLE=TTY_RAMLOG
 * 4. We use no debug.
 *    <empty>
  *
 */

#if (CFG_OS_DEBUG == STD_ON)
# if (SELECT_OS_CONSOLE==RAMLOG)
#  ifndef USE_RAMLOG
#  error  USE_RAMLOG must be defined.
#  endif

#  define OS_DEBUG(_mask,...) \
	do { \
		if( os_dbg_mask & (_mask) ) { \
			ramlog_printf("[%08u] : ",(unsigned)GetOsTick()); \
			ramlog_printf(__VA_ARGS__ );	\
		}; \
	} while(0);
# elif (SELECT_OS_CONSOLE==TTY_NONE)
#   define OS_DEBUG(_mask,...)
# else
#  define OS_DEBUG(_mask,...) \
	do { \
		if( os_dbg_mask & (_mask) ) { \
			printf("[%08u] : %s %d ",(unsigned)GetOsTick(), __FUNCTION__, __LINE__ ); \
			printf(__VA_ARGS__ );	\
		}; \
	} while(0);
# endif
#else
# define OS_DEBUG(_mask,...)
#endif


#if ( OS_SC1 == STD_ON ) || ( OS_SC4 == STD_ON )
typedef void ( * trusted_func_t)( TrustedFunctionIndexType , TrustedFunctionParameterRefType );
#endif


/* STD container : OsOs. OSEK properties
 * Class: ALL
 *
 * OsScalabilityClass:			0..1 SC1,SC2,SC3,SC4
 * OsStackMonitoring:			1    Stack monitoring of tasks/category 2
 * OsStatus                 	1    EXTENDED or STANDARD status
 * OsUseGetServiceId			1    We can use the  OSErrorGetServiceId() function
 * OsUseParameterAccess			1    We save the parameters in OSError_XX_YY()
 * OsUseResScheduler			1
 * OsHooks[C]               	1
 *
 * From OSEK/VDX oil:
 *
 * OS ExampleOS {
 *   STATUS = STANDARD;
 *   STARTUPHOOK = TRUE;
 *   ERRORHOOK = TRUE;
 *   SHUTDOWNHOOK = TRUE;
 *   PRETASKHOOK = FALSE;
 *   POSTTASKHOOK = FALSE;
 *   USEGETSERVICEID = FALSE;
 *   USEPARAMETERACCESS = FALSE;
 *   USERESSCHEDULER = TRUE;
 * };
 *
 * OS_SC1 | OS_SC2 | OS_SC3 | OS_SC4
 * OS_STACK_MONITORING
 * OS_STATUS_EXTENDED  / OS_STATUS_STANDARD
 * OS_USE_GET_SERVICE_ID
 * OS_USE_PARAMETER_ACCESS
 * OS_RES_SCHEDULER
 * */

/**
 * STD container: OsResource
 * OsResourceProperty:				1    INTERNAL, LINKED, STANDARD
 * OsResourceAccessingApplication:  0..*
 * OsResourceLinkedResourceRef:     0..1
 */

typedef enum {
	/* External resource */
	RESOURCE_TYPE_STANDARD,//!< RESOURCE_TYPE_STANDARD
	/* ?? */
	RESOURCE_TYPE_LINKED,  //!< RESOURCE_TYPE_LINKED
	/* Internal resource */
	RESOURCE_TYPE_INTERNAL //!< RESOURCE_TYPE_INTERNAL
} OsResourcePropertyType;

/*-----------------------------------------------------------------*/
typedef struct OsResource {
	char id[16];
	// The running number, starting at RES_SCHEDULER=0
	int nr;
	// The calculated ceiling prio
	uint32 ceiling_priority;
	// Stored prio of the owner oi the resource
	uint32 old_task_prio;

	// What application may access this resource. A resource may only be
	// accessed by one application
#if ( OS_SC3 == STD_ON ) || ( OS_SC4 == STD_ON )
	uint32 application_owner_id;
#endif
	// What tasks may access this resource. A resource may be be shared
	// several tasks.
//	uint32 task_mask;
	// Owner of the resource...
	TaskType owner;

	OsResourcePropertyType type;
	// used only if type is RESOURCE_TYPE_LINKED
	ResourceType    linked_resource;

	/* List of resources for each task. */
	TAILQ_ENTRY(OsResource) listEntry;

} OsResourceType;

typedef enum {
	LOCK_TYPE_RESOURCE,
	LOCK_TYPE_INTERRUPT,
} OsLocktypeType;

typedef struct OsLockingtime {
	OsLocktypeType type;
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
} OsLockingtimeType;

typedef struct OsTimingProtection {
	// ROM, worst case execution budget in ns
	uint64	execution_budget;
	// ROM, the frame in ns that timelimit may execute in.
	uint64 timeframe;
	// ROM, time in ns that the task/isr may with a timeframe.
	uint64 timelimit;
	// ROM, resource/interrupt locktimes
	OsLockingtimeType *lockingtime;
} OsTimingProtectionType;


#include "counter_i.h"
#include "alarm_i.h"
#include "sched_table_i.h"
#include "application.h"
#include "pcb.h"
#include "sys.h"

/*-----------------------------------------------------------------*/


/*-----------------------------------------------------------------*/
/*
 * debug settings for os_debug_mask
 *
 */

// print to STDOUT. If not set it prints to ramlog
#define D_STDOUT					0
#define D_RAMLOG					0
#define D_MASTER_PRINT				0
#define D_ISR_MASTER_PRINT			0
#define D_ISR_RAMLOG				0

#define D_TASK						(1<<0)
#define D_ALARM					(1<<1)
#define D_RESOURCE					(1<<2)
#define D_SCHTBL					(1<<3)
#define D_EVENT					(1<<4)
#define D_MESSAGE					(1<<5)


#endif /* KERNEL_H_ */
