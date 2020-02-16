/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/

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

int Os_Test_And_Set(vuint8_t* lock, vuint8_t value);
boolean Os_StartCore(CoreIDType id);


#endif /*ARCH_H_*/
