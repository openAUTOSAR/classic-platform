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










#include "Os.h"
#include "types.h"
#include "counter_i.h"
#include "ext_config.h"
#include "alarm_i.h"
#include <assert.h>
#include <stdlib.h>
#include "hooks.h"
#include "internal.h"
#include "alarm_i.h"


/**
 * The system service  GetAlarmBase  reads the alarm base
 * characteristics. The return value <Info> is a structure in which
 * the information of data type AlarmBaseType is stored.
 *
 * @param alarm_id  Reference to alarm
 * @param info Reference to structure with constants of the alarm base.
 * @return
 */
StatusType GetAlarmBase( AlarmType AlarmId, AlarmBaseRefType Info ) {
    StatusType rv = Oil_GetAlarmBase(AlarmId,Info);
    if (rv != E_OK) {
        goto err;
    }
    OS_STD_END_2(OSServiceId_GetAlarmBase,AlarmId, Info);
}

StatusType GetAlarm(AlarmType AlarmId, TickRefType Tick) {
    StatusType rv = E_OK;

	(void)AlarmId;
	(void)Tick;
// TODO: What is this?

	// Prevent label warning. Remove when proper error handling is implemented.
	if (0) goto err;

	OS_STD_END_2(OSServiceId_GetAlarm,AlarmId, Tick);
}

#define COUNTER_MAX(x) 			(x)->counter->alarm_base.maxallowedvalue
#define COUNTER_MIN_CYCLE(x) 	(x)->counter->alarm_base.mincycle
#define ALARM_CHECK_ID(x) 				\
	if( (x) > Oil_GetAlarmCnt()) { \
		rv = E_OS_ID;					\
		goto err; 						\
	}





/**
 * The system service occupies the alarm <AlarmID> element.
 * After <increment> ticks have elapsed, the task assigned to the
 * alarm <AlarmID> is activated or the assigned event (only for
 * extended tasks) is set or the alarm-callback routine is called.
 *
 * @param alarm_id Reference to the alarm element
 * @param increment Relative value in ticks
 * @param cycle Cycle value in case of cyclic alarm. In case of single alarms, cycle shall be zero.
 * @return
 */

StatusType SetRelAlarm(AlarmType AlarmId, TickType Increment, TickType Cycle){
	StatusType rv = E_OK;
	alarm_obj_t *a_obj;

	ALARM_CHECK_ID(AlarmId);

	a_obj = Oil_GetAlarmObj(AlarmId);

	os_isr_printf(D_ALARM,"SetRelAlarm id:%d inc:%d cycle:%d\n",AlarmId,Increment,Cycle);


	if( (Increment == 0) ||
		(Increment > COUNTER_MAX(a_obj)) ||
		(Cycle < COUNTER_MIN_CYCLE(a_obj)) ||
		(Cycle > COUNTER_MAX(a_obj)) )
	{
		/* See SWS, OS304 */
		rv =  E_OS_VALUE;
		goto err;
	}

	{

		Irq_Disable();
		if( a_obj->active == 0 ) {
			a_obj->active = 1;
		} else {
			rv = E_OS_STATE;
			goto err;
		}

		TickType curr_val = a_obj->counter->val;
		TickType left = COUNTER_MAX(a_obj) - curr_val;

		a_obj->expire_val = (left < Increment ) ?
									(curr_val + Increment) :
									(Increment - curr_val);
		a_obj->cycletime = Cycle;

		Irq_Enable();
		os_isr_printf(D_ALARM,"  expire:%d cycle:%d\n",a_obj->expire_val,a_obj->cycletime);
	}

	OS_STD_END_3(OSServiceId_SetRelAlarm,AlarmId, Increment, Cycle);
}

StatusType SetAbsAlarm(AlarmType AlarmId, TickType Start, TickType Cycle) {

	alarm_obj_t *a_p;
	long flags;
	StatusType rv = E_OK;

	a_p = Oil_GetAlarmObj(AlarmId);

	if( a_p == NULL ) {
	    rv = E_OS_ID;
	    goto err;
	}

	if( (Start > COUNTER_MAX(a_p)) ||
		(Cycle < COUNTER_MIN_CYCLE(a_p)) ||
		(Cycle > COUNTER_MAX(a_p)) )
	{
		/* See SWS, OS304 */
	    rv = E_OS_VALUE;
	    goto err;
	}

	Irq_Save(flags);
	if( a_p->active == 1 ) {
		rv = E_OS_STATE;
		goto err;
	}

	a_p->active = 1;

	a_p->expire_val = Start;
	a_p->cycletime = Cycle;
	Irq_Restore(flags);

	os_isr_printf(D_ALARM,"  expire:%d cycle:%d\n",a_p->expire_val,a_p->cycletime);

	OS_STD_END_3(OSServiceId_SetAbsAlarm,AlarmId, Start, Cycle);
}

StatusType CancelAlarm(AlarmType AlarmId) {
	StatusType rv = E_OK;
	alarm_obj_t *a_obj;
	long flags;

	ALARM_CHECK_ID(AlarmId);

	a_obj = Oil_GetAlarmObj(AlarmId);

	Irq_Save(flags);
	if( a_obj->active == 0 ) {
		rv = E_OS_NOFUNC;
		Irq_Restore(flags);
		goto err;
	}

	a_obj->active = 0;

	Irq_Restore(flags);

	OS_STD_END_1(OSServiceId_CancelAlarm,AlarmId);
}


