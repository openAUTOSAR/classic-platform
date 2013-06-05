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


/* ----------------------------[includes]------------------------------------*/

#include "internal.h"
#include "Cpu.h"
#include "sys.h"
#include "asm_ppc.h"
#include "mpc55xx.h"
#include "arch_stack.h"
#include "arch_offset.h"
#include "arch.h"


#define USE_LDEBUG_PRINTF
#include "debug.h"

/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/



/**
 * NOT complete yet.....
 *
 * @param err			The error code.
 * @param errFramePtr   Pointer to extra information about the error, if any.
 * @param excFramePtr   Pointer to the exception frame, that caused the error.
 */
void Os_ArchPanic( uint32_t err, void *errFramePtr , void *excFramePtr) {
	(void)excFramePtr;  // Really of type Os_ExceptionFrameType
	(void)errFramePtr;
	switch(err) {
	case OS_ERR_BAD_CONTEXT:
		while(1) {} ;
	case OS_ERR_SPURIOUS_INTERRUPT:
#if 0
		fputs("Spurious interrupt\n",stdout);
		printf(" vector : %02lx\n", excFramePtr->vector);
		printf(" srr0   : %08lx\n", excFramePtr->srr0);
		printf(" srr1   : %08lx\n", excFramePtr->srr1);
#endif
		while(1) {} ;
	default:
		while(1) {} ;
	}
}

/**
 * Function make sure that we switch to supervisor mode(rfi) before
 * we call a task for the first time.
 */
void Os_ArchFirstCall( void )
{
#if USE_MM_USER_MODE

	// Assume that regs[0] is setup before and contains the settings
	// to switch to user mode.
	register uint32_t msr asm("r3") = os_sys.curr_pcb->regs[0];
	register void *ea asm("r4") = (void *) os_sys.curr_pcb->entry;

	// Do the switch
	asm volatile(
			"mtspr 26,%0;\r\t"	// srr0
			"mtspr 27,%1;\r\t"	// srr1
			"rfi;\r\t"
			:
			: "r" (ea), "r" (msr)	);
#else
// TODO: This really depends on if scheduling policy
	Irq_Enable();
	Os_Sys.currTaskPtr->constPtr->entry();
	//os_sys.curr_pcb->entry();
#endif
}


unsigned int Os_ArchGetScSize( void ) {
	return FUNC_FRM_SIZE;
}


/**
 * Setup a context for a task.
 *
 * @param pcb Pointer to the pcb to setup
 */
void Os_ArchSetupContext( OsTaskVarType *pcbPtr ) {
	Os_FuncFrameType *cPtr = (Os_FuncFrameType *)pcbPtr->stack.curr;
	uint32_t msr;
	msr = MSR_EE;

#if defined(CFG_SPE)
	msr |= MSR_SPE;
#endif

#if (OS_USE_APPLICATIONS == STD_ON)
#if 0
	if( !pcb->application->trusted ) {
		// Non-trusted = User mode..
		msr |= MSR_PR | MSR_DS | MSR_IS;
	}
#endif
#endif
	pcbPtr->regs[0] = msr;

	cPtr->pattern = FUNC_PATTERN;
}

/**
 *
 * @param pcbPtr
 */

void Os_ArchSetTaskEntry(OsTaskVarType *pcbPtr ) {
	Os_FuncFrameType *cPtr = (Os_FuncFrameType *)pcbPtr->stack.curr;

	if( pcbPtr->constPtr->proc_type == PROC_EXTENDED ) {
		cPtr->lr = (uint32_t)Os_TaskStartExtended;
	} else if( pcbPtr->constPtr->proc_type == PROC_BASIC ) {
		cPtr->lr = (uint32_t)Os_TaskStartBasic;
	}

}

void Os_ArchInit( void ) {
	uint32_t msr = get_msr();
#if defined(CFG_SPE)
	msr |= MSR_SPE;
#endif
	msr |= MSR_ME;	/* We want IVOR1 instead of checkstop */
	set_msr(msr);

}


