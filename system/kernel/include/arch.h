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

/* See Os_ArchReboot() */
#define OS_REBOOT_WARM		0
#define OS_REBOOT_COLD		1


/**
 * Set the stack pointer to sp and call function f.
 *
 * @param sp    Pointer to the stack
 * @param f     Pointer to the function to call.
 *
 */
__simple void Os_ArchSetSpAndCall(void *sp, void (*f)(void) );

/**
 * Swap context.
 * Sets the current pcb
 *
 * @param old - old ptr to pcb
 * @return
 */
__simple void Os_ArchSwapContext(void *old,void *new);


/**
 * Swap context for the first time for a task
 *
 * The only way to call this function right now is to:
 *   Os_TaskSwapContextTo(NULL,<pcb>);
 */
__simple void Os_ArchSwapContextTo(void *old,void *new);

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
void Os_ArchSetupContext( OsTaskVarType *pcb );

/**
 * Get current stack pointer
 *
 * @return current stack pointer
 */
__simple void *Os_ArchGetStackPtr( void );

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
 * Reboot the OS. Can be used as soft reset but also to get a controlled
 * reboot of the OS, but saving state.
 *
 * This should probably:
 * - Setup the stack pointer to same as in crt0.
 * - Jump to main?!
 * - De-init for some devices?
 *
 * @param type OS_REBOOT_COLD - Reboot cold. Soft reset.
 *             OS_REBOOT_WARM - Reboot warm. Does not run init on sections (crt0)
 */
void Os_ArchReboot( int type );


/**
 * Get the small context size
 *
 * @return The small context size in bytes
 */
unsigned int Os_ArchGetScSize( void );

void Os_ArchSetTaskEntry(OsTaskVarType *pcbPtr );

void Os_ArchPanic( uint32_t err, void *errFramePtr , void *excFramePtr);

#endif /*ARCH_H_*/
