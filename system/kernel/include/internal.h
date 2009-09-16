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

#ifndef INTERNAL_H_
#define INTERNAL_H_

#include <assert.h>
#include "Ramlog.h"
#include "simple_printf.h"

extern uint32 os_dbg_mask;

/*
 * 0  master print normal, 1-print
 * 1  master print isr     1-print
 * 2  normal 0-stdout,1-ramlog
 * 3  isr    0-stdout,
 *
 * 16 task_low
 * 17 task high
 *
 * 20 alarm
 *
 *
 * So when debugging the kernel using the ISS you want to use:
 * 0xB
 *
 * Ramlog all the way:
 * 0x7
 */

extern uint32 os_dbg_mask;

#define STR_TASK		"OS_TASK"
#define STR_ALARM		"OS_ALARM"
#define STR_STBL		"OS_STBL"


#define os_dbg_printf(format,...) \
			if( os_dbg_mask & OS_DBG_MASTER_PRINT ) { \
					simple_printf(format,## __VA_ARGS__ );	\
			}

#define os_dbg_isr_printf(format,...) \
			if( os_dbg_mask & OS_DBG_ISR_MASTER_PRINT ) { \
					simple_printf(format,## __VA_ARGS__ );	\
			}

#define os_isr_printf(_mask,format,...) \
			if( (os_dbg_mask & OS_DBG_ISR_MASTER_PRINT) && ((_mask)>255 ) ) { \
				if( os_dbg_mask & D_ISR_STDOUT ) { \
					simple_printf("[%08d] : ",GetOsTick()); \
					simple_printf(format,## __VA_ARGS__ );	\
				} else {                                    \
					ramlog_printf("[%08d] : ",GetOsTick()); \
					ramlog_printf(format,## __VA_ARGS__ );	\
				}                                           \
			}

#define os_std_printf(_mask,format,...) \
		if( (os_dbg_mask & OS_DBG_MASTER_PRINT) && ((_mask)>255 ) ) { \
			if( os_dbg_mask & D_STDOUT) { \
				simple_printf("[%08d] : ",GetOsTick()); \
				simple_printf(format,## __VA_ARGS__ );	\
			} else {                                    \
				ramlog_printf("[%08d] : ",GetOsTick()); \
				ramlog_printf(format,## __VA_ARGS__ );	\
			}                                           \
		}

#if 0
#define os_isr_printf(D_TASK,format,...) \
			if( (os_dbg_mask | OS_DBG_ISR_MASTER_PRINT | OS_DBG_TASK) \
					== os_dbg_mask ) { \
					simple_printf("[%08d] %s: ",GetOsTick(), STR_TASK); \
					simple_printf(format,## __VA_ARGS__ );	\
			}

#define os_dbg_task_printf(format,...) \
			if( (os_dbg_mask | OS_DBG_MASTER_PRINT | OS_DBG_TASK) \
					== os_dbg_mask ) { \
					simple_printf("[%08d] %s: ",GetOsTick(), STR_TASK); \
					simple_printf(format,## __VA_ARGS__ );	\
			}
#endif

#if 0
static inline void Irq_Disable( void ) {
	Irq_Disable();
}

static inline void Irq_Enable( void ) {
	Irq_Enable();
}
#endif


#if 0
#define os_dbg_m_printf(_x,format,...) \
			if( (os_dbg_mask | OS_DBG_ISR_MASTER_PRINT | (_x)) == os_dbg_mask) { \
					simple_printf(format,## __VA_ARGS__ );	\
			}
#endif

/*
 * PCB manipulating functions
 */

static inline procid_t get_curr_pid( void ) {
	return os_sys.curr_pcb->pid;
}

static inline pcb_t *get_curr_pcb( void ) {
	return os_sys.curr_pcb;
}

static inline void set_curr_pcb( pcb_t *pcb ) {
	os_sys.curr_pcb = pcb;
}

static inline _Bool is_idle_task( pcb_t *pcb ){
	return (pcb->pid == 0);
}

static inline procid_t get_curr_prio( void ){
	return os_sys.curr_pcb->prio;
}

static inline TickType get_os_tick( void ) {
	return os_sys.tick;
}

static inline app_t *get_curr_application( void ) {
	return get_curr_pcb()->application;
}

static inline uint32_t get_curr_application_id( void ) {
	return get_curr_pcb()->application->application_id;
}

static inline struct resource_obj_s *os_get_resource_int_p( void ) {
	return get_curr_pcb()->resource_int_p;
}

/*
 * Misc
 */

static inline uint32_t os_task_nr_to_mask( uint32_t nr ) {
	return (1<<nr);
}

// task_i.c
pcb_t *os_find_top_prio_proc( void );
pcb_t *os_find_task( TaskType tid );

// resource.c
void os_resource_get_internal(void );
void os_resource_release_internal( void );
// Create.c
pcb_t * os_alloc_new_pcb( void );


void OsTick( void );

void *Os_Isr( void *stack, void *pcb_p );


#endif /*INTERNAL_H_*/
