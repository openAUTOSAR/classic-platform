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








/*
 * Porting interface. Should really be called port.h or something.

 * The routines below + interrupt handing in Mcu_IntXX.c, should
 * be all that needs to be implemented for a new port.
 */

#ifndef ARCH_H_
#define ARCH_H_

#define STACK_PATTERN	0x42

/**
 * Swap context.
 * Sets the current pcb
 *
 * @param old - old ptr to pcb
 * @return
 */
void os_arch_swap_context(void *old,void *new);

/**
 * Swap context for the first time for a task
 *
 * The only way to call this function right now is to:
 *   os_swap_context_to(NULL,<pcb>);
 */
void os_arch_swap_context_to(void *old,void *new);

/**
 * Setup a pcb before use.
 *
 * This function should setup:
 * - context marker( SC_PATTERN, LC_PATTERN )
 * - Arch specific register setup
 * - Check that the stack is suffient for the arch
 * - More...
 *
 * @param pcb Ptr to pcb
 */
void os_arch_setup_context( pcb_t *pcb );

/**
 * Get current stack pointer
 *
 * @return current stack pointer
 */
void *os_arch_get_stackptr( void );

/**
 * Initialize the hardware.
 * This could be initialization of:
 * - interrupt controller
 * - timers
 */
void os_arch_init( void );

/**
 * Function that is used when task entry is called for the
 * first time. For ports that don't support user mode it
 * should just call entry entry() function of the current task.
 *
 * When user mode is supported a switch to user mode must be done
 * in some way. A trap maybe?
 */
void os_arch_first_call( void );


/**
 * Get the small context size
 *
 * @return The small context size in bytes
 */
unsigned int os_arch_get_sc_size( void );

#if 0

void *os_arch_get_stack_usage( pcb_t * );

_Bool os_arch_stack_endmark_ok( pcb_t *);
#endif

static inline void os_arch_stack_set_endmark( pcb_t *pcb ) {
	uint8_t *end = pcb->stack.top;
	*end = STACK_PATTERN;
}

static inline _Bool os_arch_stack_endmark_ok( pcb_t *pcb ) {
	uint8_t *end = pcb->stack.top;
	return ( *end == STACK_PATTERN);
}


static inline void *os_arch_get_stack_usage( pcb_t *pcb ) {

	uint8_t *p = pcb->stack.curr;
	uint8_t *end = pcb->stack.top;

	while( (*end == STACK_PATTERN) && (end<p)) {
			end++;
		}
	return (void *)end;
}



#endif /*ARCH_H_*/
