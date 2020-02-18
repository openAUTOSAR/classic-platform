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

#include "Arc_Types.h"

/* Default backchain space */
#define ARCH_BACKCHAIN_SIZE             (16U)

/* Pattern for startup stack */
#define START_STACK_PATTERN             0x42u


/**
 * @brief   Set the stack pointer to sp and call function f.
 *
 * @param sp    Pointer to the stack
 * @param f     Pointer to the function to call.
 *
 */
void Os_ArchSetSpAndCall(void *sp, void (*f)(void) );

/**
 * @brief  Return the current stack pointer.
 *
 * @param sp
 */
void Os_ArchSetStackPointer(void *sp);

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
void Os_ArchSetupContext( OsTaskVarType *pcb );

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
void Os_ArchFirstCall( void  );

#if (OS_SC3==STD_ON) || (OS_SC4==STD_ON)
/**
 * @brief   Call the ISR entry.
 * @details This function should
 *          * Set the MPU entries for ISR application
 *          * Load the application interrupt stack
 *          * Swap to user mode
 *          * Enable interrupts
 *          * Call the ISR entry
 *          * Swap back to privileged mode (trap into the kernel)
 *          * return
 *
 * @param   aP   Pointer to application
 * @param   func Function pointer to the ISR.
 * @param   paP  Pointer to preemted application
 */
void Os_ArchCallIsrEntry( OsAppVarType *aP, void (*func)(void), OsAppVarType *paP);

#endif

/**
 * Get the small context size
 *
 * @return The small context size in bytes
 */
uint32 Os_ArchGetScSize( void );

/**
 *
 * @param id
 * @return
 */
boolean Os_StartCore(CoreIDType id);


/**
 * Function to get map from architecture specific exception
 * to a protection type
 * @param exception
 *
 * @return
 */
StatusType Os_ArchGetProtectionType( uint32 exception );

/**
 * Function to get PCXI. Used only by Tricore
 * @return PCXI
 */
uint32 Os_GetCurrentPcxi(void);



/**
 * Function to swap to privileged mode.
 *
 * @param pcxi - The value of PCXI
 */
void Os_ArchToPrivilegedMode( uint32 pcxi);



/**
 * Check if initial stack
 * @return
 */
Std_ReturnType Os_ArchCheckStartStackMarker( void );



#endif /*ARCH_H_*/
