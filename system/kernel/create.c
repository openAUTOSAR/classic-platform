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


#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/queue.h>
#include <string.h>
#include "sys.h"
#include "pcb.h"
#include "Os.h"

extern caddr_t *sbrk(int);

#define os_alloc(_x)	sbrk(_x)

pcb_t * os_alloc_new_pcb( void ) {
	void *h = os_alloc(sizeof(pcb_t));
	memset(h,0,sizeof(pcb_t));
	assert(h!=NULL);
	return h;
}

#if 0
typedef void (*Os_IsrEntryType)(void);


typedef Os_IsrInfo_s {
	Os_IsrEntryType entry;
	uint32_t vector;
	uint8_t priority;
} Os_IsrInfoType;
#endif


extern TaskType os_add_task( pcb_t *pcb );

static uint8 stackTop = 0x42;

TaskType Os_CreateIsr( void (*entry)(void ), uint8_t prio, const char *name )
{
	pcb_t *pcb = os_alloc_new_pcb();
	strncpy(pcb->name,name,TASK_NAME_SIZE);
	pcb->vector = -1;
	pcb->prio = prio;
	/* TODO: map to interrupt controller priority */
	assert(prio<=OS_TASK_PRIORITY_MAX);
	pcb->proc_type  = PROC_ISR2;
	pcb->state = ST_SUSPENDED;
	pcb->entry = entry;
	pcb->stack.top = &stackTop;


	return os_add_task(pcb);
}

