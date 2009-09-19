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









#include "types.h"
#include "Os.h"
#include "assert.h"
#include "sys.h"
#include "stdlib.h"
#include "kernel.h"
#include "internal.h"
#include "hooks.h"
#include "task_i.h"
#include "ext_config.h"


#define valid_standard_id() (rid->nr < Oil_GetResourceCnt()) //&& !(rid->type == RESOURCE_TYPE_INTERNAL) )
#define valid_internal_id() (rid->nr < Oil_GetResourceCnt()) //&& (rid->type == RESOURCE_TYPE_INTERNAL) )


static StatusType GetResource_( resource_obj_t * );
StatusType ReleaseResource_( resource_obj_t * );

StatusType GetResource( ResourceType ResID ) {
	resource_obj_t *rid = Oil_GetResource(ResID);
	StatusType rv = GetResource_(rid);

	if (rv != E_OK)
	    goto err;

	OS_STD_END_1(OSServiceId_GetResource,ResID);
}

#if 0
StatusType GetResourceInternal( ResourceType ResID ) {
	return GetResource_(ResID,1);
}
#endif

static StatusType GetResource_( resource_obj_t * rid ) {
	StatusType rv = E_OK;

	if( rid->nr == RES_SCHEDULER ) {
		// Lock the sheduler
		os_sys.scheduler_lock = 1;
		//simple_printf("RES_SCHEDULER, NOT supported yet\n");
		//while(1);
	}
	// Check if valid resource
	if( !valid_standard_id() ) {
		rv = E_OS_ID;
		goto err;
	}
	// Check that the resource does not belong to another application or task
	if(	( (os_task_nr_to_mask(get_curr_pid()) & rid->task_mask ) == 0 )
		|| ( get_curr_application_id() !=  rid->application_owner_id)
		|| ( rid->owner != (TaskType)(-1)))
	{
		rv = E_OS_ACCESS;
		goto err;
	}

	rid->owner = get_curr_pid();
	rid->old_task_prio = os_pcb_set_prio(os_get_curr_pcb() ,rid->ceiling_priority);
	goto ok;
err:
	ERRORHOOK(rv);
ok:
	return rv;
}

StatusType ReleaseResource( ResourceType ResID) {
    StatusType rv = E_OK;
	if( ResID == RES_SCHEDULER ) {
		os_sys.scheduler_lock=0;
	} else {
	    resource_obj_t *rid = Oil_GetResource(ResID);
	    rv = ReleaseResource_(rid);
	}

	if (rv != E_OK)
	    goto err;

	OS_STD_END_1(OSServiceId_ReleaseResource,ResID);
}

StatusType ReleaseResource_( resource_obj_t * rid ) {
	if (!valid_standard_id()) {
		return E_OS_ID;
	} else {

        // Release it...
        rid->owner = (TaskType) (-1);
        os_pcb_set_prio(os_get_curr_pcb(), rid->old_task_prio);

        return E_OK;
	}
}

// TODO: Remove this function later.. this is done in oil generator
//       instead.
void os_resource_calc_attributes( void ) {
	// Calc ceiling
//	ResourceType *rsrc;
	for( int i=0;i<Oil_GetResourceCnt();i++) {
//		rsrc = Oil_GetResource();
		/* TODO: Do this when there's more time */
//		rsrc
	}
}

//
void os_resource_get_internal( void ) {
	resource_obj_t *rt = os_get_resource_int_p();

	if( rt != NULL ) {
		//simple_printf("Get IR proc:%s prio:%d old_task_prio:%d\n",get_curr_pcb()->name, rt->ceiling_priority,rt->old_task_prio);
		GetResource_(rt);
	}
	//GetResourceInternal(os_get_curr_pcb()->resource_internal);
}

void os_resource_release_internal( void ) {
	resource_obj_t *rt = os_get_resource_int_p();

	if(  rt != NULL ) {
		//simple_printf("Rel IR proc:%s prio:%d old_task_prio:%d\n",get_curr_pcb()->name,rt->ceiling_priority,rt->old_task_prio);
		ReleaseResource_(rt);
	}
	//ReleaseResource(os_get_curr_pcb()->resource_internal);
}



