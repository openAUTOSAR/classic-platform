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

/* ----------------------------[includes]------------------------------------*/

#include <stdlib.h>
#include <stdint.h>
#include "Os.h"

#include "internal.h"
#include "task_i.h"
#include "alarm_i.h"
#include "counter_i.h"
#include "sched_table_i.h"
#include "application.h"
#include "isr.h"
#include "resource_i.h"
#include "sys.h"

/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
#if OS_APPLICATION_CNT!=0
OsAppVarType Os_AppVar[OS_APPLICATION_CNT];
#endif

/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/


/* @req OS547
 *   Availability of AllowAccess(): Available in Scalability Classes 3 and 4.
 * @req OS536
 *    Availability of TerminateApplication(): Available in Scalability Classes 3 and 4.
 * @req OS520
 *    Availability  of  CheckObjectOwnership():Available  in  Scalability  Classes 3 and 4.
 *
 */


#if	(OS_USE_APPLICATIONS == STD_ON)

/**
 * This service determines the currently running OS-Application (a unique
 * identifier has to be allocated to each application).
 *
 * @return <identifier of running OS-Application> or INVALID_OSAPPLICATION
 */

ApplicationType GetApplicationID( void ) {
	return OS_SYS_PTR->currApplId;
}


/**
 * A (trusted or non-trusted) OS-Application uses this service to call a trusted
 * function
 *
 * @param FunctionIndex		Index of the function to be called.
 * @param FunctionParams    Pointer to the parameters for the function -
 * 							specified by the FunctionIndex - to be called.
 * 							If no parameters are provided, a NULL pointer has
 * 							to be passed.
 * @return
 */
StatusType 	CallTrustedFunction(	TrustedFunctionIndexType FunctionIndex,
									TrustedFunctionParameterRefType FunctionParams ) {


	(void)FunctionIndex;
	(void)FunctionParams;
	return E_OK;
}



/**
 * This service checks if a memory region is write/read/execute accessible
 * and also returns information if the memory region is part of the stack
 * space.
 *
 * @param ISRID		ISR reference
 * @param Address   Start of memory area
 * @param Size      Size of memory area
 * @return
 */
AccessType CheckISRMemoryAccess( ISRType isrId,
								MemoryStartAddressType address,
								MemorySizeType size )
{
	ptrdiff_t addr = (ptrdiff_t)address;
	(void)addr;
	(void)size;

	if( isrId > OS_TASK_CNT ) {
		return 0;
	}
	return 0;
}
/**
 * This service checks if a memory region is write/read/execute accessible
 * and also returns information if the memory region is part of the stack
 * space.
 *
 * Check returned accesstype with:
 *   OSMEMORY_IS_READABLE(<AccessType>)
 *   OSMEMORY_IS_WRITEABLE(<AccessType>)
 *   OSMEMORY_IS_EXECUTABLE(<AccessType>)
 *   OSMEMORY_IS_STACKSPACE(<AccessType>)
 *
 * TODO: Not really sure what this function is actually good for? Add a use-case!
 *
 * @param TaskID   Task reference
 * @param Address  Start of memory area
 * @param Size     Size of memory area
 * @return
 */
AccessType CheckTaskMemoryAccess( 	TaskType taskId,
									MemoryStartAddressType address,
									MemorySizeType size )
{
	ptrdiff_t addr = (ptrdiff_t)address;
	(void)addr;
	(void)size;

	/* @req OS270:
	 * if the Task reference <TaskID> in a call of CheckTaskMemoryAccess() is
	 * not valid, CheckTaskMemoryAccess() shall yield no access rights.
	 */
	if( taskId > OS_TASK_CNT ) {
		return 0;
	}

	/* TODO: Add body :) */
	return 0;
}


/**
 * This service determines if the OS-Applications, given by ApplID,
 * is allowed to use the IDs of a Task, ISR, Resource, Counter,
 * Alarm or Schedule Table in API calls.
 *
 * @param ApplID      OS-Application identifier
 * @param ObjectType  Type of the following parameter
 * @param object      The object to be examined
 * @return ACCESS if the ApplID has access to the object
 * NO_ACCESS otherwise
 */
ObjectAccessType CheckObjectAccess( ApplicationType ApplId,
									ObjectTypeType ObjectType,
									uint32_t objectId )
{
	uint32 appMask = APPL_ID_TO_MASK(ApplId);
	ObjectAccessType rv = NO_ACCESS;
	_Bool rvMask = 0;

	/* @req OS423
	 * If in a call of CheckObjectAccess() the object to  be examined
	 * is not avalid object OR <ApplID> is invalid OR <ObjectType> is
	 * invalid THEN CheckObjectAccess() shall return NO_ACCESS.
	 */
	if( ApplId > OS_APPLICATION_CNT ) {
		goto err;
	}

	/* @req OS272
	 * If the OS-Application <ApplID> in a call of CheckObjectAccess() has no
	 * access to the queried object, CheckObjectAccess() shall return NO_ACCESS.
	 *
	 * TODO: Could be that OS450 comes into play here....and then this is wrong.
	 */
	if( Os_AppVar[ApplId].state != APPLICATION_ACCESSIBLE ) {
		goto err;
	}


	/* TODO: check id */
	switch( ObjectType ) {
	case OBJECT_ALARM:
		if( objectId < OS_ALARM_CNT ) {
			rvMask =  Os_AlarmGet((AlarmType)objectId)->accessingApplMask & (appMask);
		}
		break;
	case OBJECT_COUNTER:
		if( objectId < OS_COUNTER_CNT ) {
			rvMask =  Os_CounterGet((CounterType)objectId)->accessingApplMask & (appMask);
		}
		break;
	case OBJECT_ISR:
		/* An ISR do not have accessingApplicationMask, just check if owner */
		if( objectId < OS_ISR_CNT ) {
			rvMask =  (Os_IsrGet((ISRType)objectId)->constPtr->appOwner == ApplId);
		}
		break;
	case OBJECT_MESSAGE:
		break;
	case OBJECT_RESOURCE:
		if( objectId < OS_RESOURCE_CNT ) {
			rvMask =  Os_ResourceGet((ResourceType)objectId)->accessingApplMask & (appMask);
		}
	case OBJECT_SCHEDULETABLE:
		if( objectId < OS_SCHTBL_CNT ) {
			rvMask =  Os_SchTblGet((ScheduleTableType)objectId)->accessingApplMask & (appMask);
		}
		break;
	case OBJECT_TASK:
		if( objectId < OS_TASK_CNT ) {
			rvMask =  Os_TaskGet((TaskType)objectId)->constPtr->accessingApplMask & (appMask);
		}
		break;
	default:
		/* @req OS423 */
		// rvMask = NO_ACCESS;
		break;
	}

	rv = rvMask ? ACCESS : NO_ACCESS;

	OS_STD_END_3(OSServiceId_CheckObjectAccess,ApplId,ObjectType,objectId);
}

/**
 * This service determines to which OS-Application a given Task, ISR, Resource,
 * Counter, Alarm or Schedule Table belongs
 *
 * @param ObjectType Type of the following parameter
 * @param object     The object to be examined
 * @return The OS-Application to which the object ObjectType belongs or
 * INVALID_OSAPPLICATION if the object does not exists
 */
ApplicationType CheckObjectOwnership( ObjectTypeType ObjectType,
									uint32_t objectId )
{
	ApplicationType rv = INVALID_OSAPPLICATION;

	switch( ObjectType ) {
	case OBJECT_ALARM:
		rv = Os_AlarmGetApplicationOwner((TaskType)objectId);
		break;
	case OBJECT_COUNTER:
		rv = Os_CounterGetApplicationOwner((TaskType)objectId);
		break;
	case OBJECT_ISR:
		rv = Os_IsrGetApplicationOwner((TaskType)objectId);
		break;
	case OBJECT_MESSAGE:
#if 0
		if( objectId < OS_SCHTBL_CNT ) {
			rv = Os_SchTblkGetApplicationOwner((TaskType)objectId);
		}
#endif
		break;
	case OBJECT_RESOURCE:
		rv = Os_ResourceGetApplicationOwner((TaskType)objectId);
		break;
	case OBJECT_SCHEDULETABLE:
		rv = Os_SchTblGetApplicationOwner((TaskType)objectId);
		break;
	case OBJECT_TASK:
		rv = Os_TaskGetApplicationOwner((TaskType)objectId);
		break;
	default:
		break;
	}

	if( rv == INVALID_OSAPPLICATION ) {
		goto err;
	}

	OS_STD_END_2(OSServiceId_CheckObjectOwnership,ObjectType,objectId);
}


/**
 * This service terminates the OS-Application to which the calling Task/Category 2
 * ISR/application specific error hook belongs.
 *
 * @param 	Application - The identifier of the OS-Application to be terminated.
 * 			If the caller belongs to <Application> the call results in a
 * 			self termination.
 *
 * @param 	RestartOption - Either RESTART for doing a restart of the
 * 			OS-Application or NO_RESTART if OS-Application shall not be restarted.
 *
 * @return  E_OK: No errors
 * 			E_OS_ID: <Application> was not valid
 * 			E_OS_VALUE: <RestartOption> was neither RESTART nor NO_RESTART
 * 			E_OS_ACCESS: The caller does not have the right to terminate <Application>
 * 			E_OS_STATE: The state of <Application> does not allow terminating <Application>
 */
StatusType TerminateApplication(  ApplicationType applId, RestartType restartOption ) {
	StatusType rv = E_OK;

	(void)applId;
	(void)restartOption;

	/* @req OS493 */
	if( applId > OS_APPLICATION_CNT) {
		rv = E_OS_ID;
		goto err;
	}

	/* @req OS459 */
	if( restartOption > NO_RESTART ) {
		rv = E_OS_VALUE;
		goto err;
	}

	/* @req OS507 */
	if( Os_ApplGet(applId)->state == APPLICATION_TERMINATED ) {
		rv = E_OS_STATE;
		goto err;
	}

	/* @req OS508 */
	if( (Os_ApplGet(applId)->state == APPLICATION_RESTARTING ) &&
		 ( applId != OS_SYS_PTR->currApplId ) ) {
		rv = E_OS_STATE;
		goto err;
	}

	/* @req OS508 */
	if( (Os_ApplGet(applId)->state == APPLICATION_RESTARTING ) &&
		 ( applId != OS_SYS_PTR->currApplId ) ) {
		rv = E_OS_STATE;
		goto err;
	}

	/* @req OS548 */
	if( (Os_ApplGet(applId)->state == APPLICATION_RESTARTING ) &&
		 ( applId == OS_SYS_PTR->currApplId ) &&
		 ( restartOption == RESTART ) ) {
		rv = E_OS_STATE;
		goto err;
	}

	/* TODO: MISSING OS494, OS287, OS535, OS536 */

	OS_STD_END_2(OSServiceId_TerminateApplication,applId,restartOption);
}


/**
 * This service sets the own state of an OS-Application from
 * APPLICATION_RESTARTING to APPLICATION_ACCESSIBLE.
 *
 * @return  E_OK : No errors
 * 			E_OS_STATE : The OS-Application of the caller is in the wrong
state
 */
StatusType AllowAccess( void ) {
	StatusType rv = E_OK;
	ApplicationType applId = OS_SYS_PTR->currApplId;

	/* @req OS497 */
	if( Os_AppVar[applId].state != APPLICATION_RESTARTING ) {
		rv = E_OS_STATE;
		goto err;
	}

	/* @req OS498 */
	Os_AppVar[applId].state = APPLICATION_ACCESSIBLE;
	OS_STD_END(OSServiceId_AllowAccess);
}

/**
 * This service returns the current state of an OS-Application.
 * SC: SC3 and SC4
 *
 * @param ApplId 		The OS-Application from which the state is requested
 * @param Value 		The current state of the application
 * @return  E_OK: No errors, E_OS_ID: <Application> is not valid
 */
StatusType GetApplicationState(   ApplicationType applId,  ApplicationStateRefType value ) {
	StatusType rv = E_OK;

	if(applId > OS_APPLICATION_CNT ) {
		rv = E_OS_ID;
		goto err;
	}

	*value = Os_AppVar[applId].state;

	OS_STD_END_2(OSServiceId_GetApplicationID,applId,value);
}


/**
 * TODO: Move somewhere else
 * @param mode
 * @return
 */
StatusType GetActiveApplicationMode( AppModeType* mode) {
	 *mode = OS_SYS_PTR->appMode;
	 return E_OK;
}


/**
 *
 */
void Os_ApplStart( void ) {
	uint16_t i;

	/* Call startuphooks for all applications */
	for(i=0;i<OS_APPLICATION_CNT;i++) {

		Os_AppVar[i].state = APPLICATION_ACCESSIBLE;

		if( Os_AppConst[i].StartupHook != NULL ) {
			Os_AppConst[i].StartupHook();
		}
	}
}


#endif
