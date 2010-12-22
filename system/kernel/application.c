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

/* ----------------------------[includes]------------------------------------*/

#include <stdlib.h>
#include <stdint.h>
#include "Os.h"

#include "internal.h"
#include "arc.h"
#include "arch.h"

/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/
#define APPL_ID_TO_MASK(_x)   (1<<(_x))

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


#if ((OS_SC3==STD_ON)||(OS_SC4==STD_ON))

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
									void *object )
{
	uint32 appMask = APPL_ID_TO_MASK(ApplId);
	ObjectAccessType orv;
	_Bool rv;


	/* @req OS423
	 * If in a call of CheckObjectAccess() the object to  be examined
	 * is not avalid object OR <ApplID> is invalid OR <ObjectType> is
	 * invalid THEN CheckObjectAccess() shall return NO_ACCESS.
	 */
	if( ApplId > OS_APPLICATION_CNT ) {
		return NO_ACCESS;
	}

	/* @req OS272
	 * If the OS-Application <ApplID> in a call of CheckObjectAccess() has no
	 * access to the queried object, CheckObjectAccess() shall return NO_ACCESS.
	 *
	 * TODO: Could be that OS450 comes into play here....and then this is wrong.
	 */
	if( Os_AppVar[ApplId].state != APPLICATION_ACCESSIBLE ) {
		return NO_ACCESS;
	}

	/* TODO: check id */
	switch( ObjectType ) {
	case OBJECT_ALARM:
		rv =  ((OsAlarmType *)object)->accessingApplMask & (appMask);
		break;
	case OBJECT_COUNTER:
		rv =  ((OsCounterType *)object)->accessingApplMask & (appMask);
		break;
	case OBJECT_ISR:
		/* TODO: Add more things here for missing objects */
		break;
	case OBJECT_MESSAGE:
	case OBJECT_RESOURCE:
	case OBJECT_SCHEDULETABLE:
		break;
	case OBJECT_TASK:
		rv = ((OsCounterType *)object)->accessingApplMask & (appMask);
		break;
	default:
		/* @req OS423 */
		rv = NO_ACCESS;
		break;
	}

	orv = rv ? ACCESS : NO_ACCESS;

	return orv;
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
									void *object )
{
	switch( ObjectType ) {
	case OBJECT_ALARM:
	case OBJECT_COUNTER:
	case OBJECT_ISR:
	case OBJECT_MESSAGE:
	case OBJECT_RESOURCE:
	case OBJECT_SCHEDULETABLE:
	case OBJECT_TASK:
	default:
		break;
	}

	return INVALID_OSAPPLICATION;
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
	(void)applId;
	(void)restartOption;
	return E_OK;
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
	ApplicationType applId = Os_Sys.currApplId;

	/* @req OS497 */
	if( Os_AppVar[applId].state != APPLICATION_RESTARTING ) {
		return E_OS_STATE;
	}

	/* @req OS498 */
	Os_AppVar[applId].state = APPLICATION_ACCESSIBLE;
	return E_OK;
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

	if(applId > OS_APPLICATION_CNT ) {
		return E_OS_ID;
	}

	*value = Os_AppVar[applId].state;

	return E_OK;
}


/**
 * TODO: Move somewhere else
 * @param mode
 * @return
 */
StatusType GetActiveApplicationMode( AppModeType* mode) {
	 *mode = Os_Sys.appMode;
	 return E_OK;
}
#endif
