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


typedef enum {
	/* External resource */
	RESOURCE_TYPE_STANDARD,
	/* ?? */
	RESOURCE_TYPE_LINKED,
	/* Internal resource */
	RESOURCE_TYPE_INTERNAL
} OsResourceTypeType;

typedef struct  {

	OsResourceTypeType type;
	/* used only if type is RESOURCE_TYPE_LINKED */
	ResourceType    linked_resource;
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
	uint32 application_owner_id;
	// What tasks may access this resource. A resource may be be shared
	// several tasks.
	uint32 task_mask;
	// Owner of the resource...
	TaskType owner;

	OsResourceTypeType type;
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

#define OS_DBG_MASTER_PRINT		(1<<0)
#define OS_DBG_ISR_MASTER_PRINT	(1<<1)
#define OS_DBG_STDOUT				(1<<2)
#define OS_DBG_ISR_STDOUT			(1<<3)

// Enable print dbg_XXXX (not dbg_isr_XXX though)
#define D_MASTER_PRINT				(1<<0)
// Enable print for all dbg_isr_XXX
#define D_ISR_MASTER_PRINT			(1<<1)
// print to STDOUT. If not set it prints to ramlog
#define D_STDOUT					(1<<2)
#define D_RAMLOG					0
// print to STDOUT, If not set print to ramlog
#define D_ISR_STDOUT				(1<<3)
#define D_ISR_RAMLOG				0

#define D_TASK						(1<<16)
#define D_ALARM					(1<<18)

#define OS_DBG_TASK				(1<<16)
#define OS_DBG_ALARM				(1<<18)

#endif /* KERNEL_H_ */
