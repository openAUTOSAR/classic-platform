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

#error Do not use this file, SC3 or SC4 is not supported yet.


#include "Os.h"


/* See 8.4.4 */
AccessType CheckISRMemoryAccess( ISRType ISRID,
								MemoryStartAddressType Address,
								MemorySizeType Size )
{
	// get hold of application memory space


}

AccessType CheckTaskMemoryAccess( 	TaskType TaskID,
									MemoryStartAddressType Address,
									MemorySizeType Size )
{


}

/* Object access
 *
 *
 * resource
 *
 *
 */




/* This is probably macros */
ObjectAccessType CheckObjectAccess( ApplicationType ApplID,
									ObjectTypeType ObjectType,
									void *object )
{
	uint32 app_mask = (1<<ApplID);
	uint32 rv;

	/* TODO: check id */
	switch( ObjectType ) {
	case OBJECT_ALARM:
		rv =  ((OsAlarmType *)object)->app_mask & (app_mask);
		break;
	case OBJECT_COUNTER:
		rv =  ((OsCounterType *)object)->app_mask & (app_mask);
		break;
	case OBJECT_ISR:
		break;
	case OBJECT_MESSAGE:
	case OBJECT_RESOURCE:
	case OBJECT_SCHEDULETABLE:
		break;
	case OBJECT_TASK:
		rv = ((OsCounterType *)object)->app_mask & (app_mask);
		break;
	default:
		break;
	}




	return ACCESS;
}

/* return application id for object */
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

	return (-1);
}
