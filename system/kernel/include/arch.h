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


#if 0
/**
 * Swap context with a context. The difference between this
 * function and Os_ArchSwapContext is that the later use
 * the current stack. This one uses stack from context.
 *
 * @param old     always NULL
 * @param new     Ptr to pcb.
 * @param context Ptr to context
 * @return
 */

void Os_ArchSwapContextToW(void *old,void *new, void *context );
#endif


/**
 * Set the stack pointer to sp and call function f.
 *
 * @param sp    Pointer to the stack
 * @param f     Pointer to the function to call.
 *
 */
void Os_ArchSetSpAndCall(void *sp, void (*f)(void) );

/**
 * Swap context.
 * Sets the current pcb
 *
 * @param old - old ptr to pcb
 * @return
 */
void Os_ArchSwapContext(void *old,void *new);

/**
 * Swap context for the first time for a task
 *
 * The only way to call this function right now is to:
 *   Os_TaskSwapContextTo(NULL,<pcb>);
 */
void Os_ArchSwapContextTo(void *old,void *new);

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
void Os_ArchSetupContext( OsPcbType *pcb );

/**
 * Get current stack pointer
 *
 * @return current stack pointer
 */
void *Os_ArchGetStackPtr( void );

/**
 * Initialize the hardware.
 * This could be initialization of:
 * - interrupt controller
 * - timers
 */
void Os_ArchInit( void );

/**
 * Function that is used when task entry is called for the
 * first time. For ports that don't support user mode it
 * should just call entry entry() function of the current task.
 *
 * When user mode is supported a switch to user mode must be done
 * in some way. A trap maybe?
 */
void Os_ArchFirstCall( void );


/**
 * Get the small context size
 *
 * @return The small context size in bytes
 */
unsigned int Os_ArchGetScSize( void );

void OsArch_SetTaskEntry(OsPcbType *pcbPtr );

#if 0

void *os_arch_get_stack_usage( OsPcbType * );

_Bool os_arch_stack_endmark_ok( OsPcbType *);
#endif



#endif /*ARCH_H_*/
