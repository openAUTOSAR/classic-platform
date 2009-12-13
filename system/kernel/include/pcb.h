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








#ifndef PCB_H
#define PCB_H

#include "types.h"
#include "sys/queue.h"
#include "Os.h"
#include "kernel.h"
#include "counter_i.h"
#include <stdio.h>
//#include "app_i.h"

#define PID_IDLE	0
#define PRIO_IDLE	0

#define ST_READY 		1
#define ST_WAITING		(1<<1)
#define ST_SUSPENDED	(1<<2)
#define ST_RUNNING		(1<<3)
#define ST_NOT_STARTED  (1<<4)

typedef uint16_t state_t;

/* from Os.h types */
typedef TaskType procid_t;
typedef EventMaskType event_t;
/* Lets have 32 priority levels
 * 0 - Highest prio
 * 31- Lowest
 */
typedef sint8 prio_t;

#define TASK_NAME_SIZE		16

#if 0
/* Application */
typedef struct {
	/* Application id */
	uint32_t application_id;
	/* Name of the application */
	char name[16];
	/* We let the application have a pool */
	void *pool;
	/* if this application is trusted or not */
	uint8 trusted:1;
	/* Attached counters */
	counter_obj_t *counter;
} app_t;
#endif
typedef rom_app_t app_t;


struct rom_pcb_s;

/* We do ISR and TASK the same struct for now */
typedef struct pcb_s {
	procid_t 	pid;					// TASK
	prio_t  	prio;
	ApplicationType application_id;
	uint32		app_mask;
	void 		(*entry)();
	proc_type_t proc_type;
	uint8		autostart:1;			// TASK
	stack_t		stack;					// TASK
	/* belongs to this application */
	app_t	   	*application;

	int			vector; 				// ISR
	char 		name[TASK_NAME_SIZE];

	timing_protection_t	*timing_protection;

	state_t 	state;					// TASK
	event_t 	ev_wait;				// TASK
	event_t 	ev_set;					// TASK

	enum OsTaskSchedule scheduling;	// TASK

	// OsTaskActivation
	// The limit from OsTaskActivation
	uint8_t activationLimit;
	// The number of queued activation of a task
	uint8_t activations;

	// A task can hold only one internal resource and only i f
	// OsTaskSchedule == FULL
	resource_obj_t *resource_int_p;		// TASK

	// What resources this task have access to
	// Typically (1<<RES_xxx) | (1<<RES_yyy)
	uint32_t resourceAccess;

	// What resource that are currently held by this task
	// Typically (1<<RES_xxx) | (1<<RES_yyy)
	uint32_t resourceHolds;

	TAILQ_HEAD(,resource_obj_s) resource_head; // TASK

	struct rom_pcb_s *pcb_rom_p;

	/* TODO: Arch specific regs .. make space for them later...*/
	uint32_t	regs[16]; 				// TASK
	/* List of PCB's */
	TAILQ_ENTRY(pcb_s) pcb_list;		// TASK
	/* ready list */
	TAILQ_ENTRY(pcb_s) ready_list;		// TASK
} pcb_t;

extern pcb_t pcb_list[];
extern rom_pcb_t rom_pcb_list[];

static inline pcb_t * os_get_pcb( procid_t pid ) {
	return &pcb_list[pid];
}

static inline rom_pcb_t * os_get_rom_pcb( procid_t pid ) {
	return &rom_pcb_list[pid];
}

static inline prio_t os_pcb_set_prio( pcb_t *pcb, prio_t new_prio ) {
	prio_t 	old_prio;
	old_prio = pcb->prio;
	pcb->prio = new_prio;
	//simple_printf("set_prio of %s to %d from %d\n",pcb->name,new_prio,pcb->prio);
	return old_prio;
}

#define os_pcb_get_state(pcb) ((pcb)->state)

#endif

