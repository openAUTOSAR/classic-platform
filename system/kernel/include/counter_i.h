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








#ifndef COUNTER_I_H_
#define COUNTER_I_H_

#include "Os.h"
#include "kernel.h"
#include <sys/queue.h>

#define COUNTER_TYPE_HARD 	0
#define COUNTER_TYPE_SOFT	1

#define COUNTER_UNIT_TICKS	0
#define COUNTER_UNIT_NANO	1

/*-----------------------------------------------------------------*/
typedef struct counter_obj_s {
	// counter id( TODO: remove ?? I use index here )
//	uint32_t cid;
	char name[16];
	// hardware or software counter, SWS OS255
	_Bool type;
	// Ticks or nano, SWS OS331
	_Bool unit;
	// The counter value ( if software counter )
	uint32_t val;
	// Application mask, SWS OS317
	uint32_t app_mask;
	//  hmm, strange to call it alarm base.... but see spec.
	AlarmBaseType alarm_base;
	/* Used only if we configure a GPT timer as os timer */
	OsDriver driver;
	/* List of alarms this counter is connected to
	 * Overkill ??? Could have list of id's here, but easier to debug this way*/
	SLIST_HEAD(slist,alarm_obj_s) alarm_head;
	/* List of scheduletable connected to this counter */
	SLIST_HEAD(sclist,sched_table_s) sched_head;
} counter_obj_t;

TickType GetCounterValue_( counter_obj_t *c_p );

#endif /*COUNTER_I_H_*/
