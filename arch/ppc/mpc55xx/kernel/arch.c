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



#include "types.h"
#include "Cpu.h"
#include "asm_ppc.h"
#include "asm_book_e.h"
#include "sys.h"
#include "arch.h"
#include "task_i.h"
#include "internal.h"
#include "mpc55xx.h"
#include <assert.h>

#define USE_DEBUG
#include "Trace.h"

/**
 * Function make sure that we switch to supervisor mode(rfi) before
 * we call a task for the first time.
 */
void os_arch_first_call( void )
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
	os_sys.curr_pcb->entry();
#endif
}


/* TODO: This actually gives the stack ptr here...not the callers stack ptr
 * Should probably be a macro instead..... in some arch part..
 */
void *os_arch_get_stackptr( void ) {
	void *stackp;
	// Get stack ptr(r1) from current context
	asm volatile(" mr %0,1":"=r" (stackp));

	return stackp;
}

unsigned int os_arch_get_sc_size( void ) {
	return SC_SIZE;
}

extern void os_arch_setup_context_asm( void *context,unsigned int msr);

#define STACK_PATTERN	0x42

#if 0

_Bool os_arch_stack_endmark_ok( pcb_t *pcb ) {
	uint8_t *end = pcb->stack.top;
	return ( *end == STACK_PATTERN);
}


void *os_arch_get_stack_usage( pcb_t *pcb ) {

	uint8_t *p = pcb->stack.curr;
	uint8_t *end = pcb->stack.top;

	while( (*end == STACK_PATTERN) && (end<p)) {
			end++;
		}
	return (void *)end;
}
#endif


// TODO: I have no clue why I wrote this????
void os_arch_stack_to_small(pcb_t *pcb ,uint32_t size_min) {
	pcb_t *t;
	uint32_t min;

	while(1) {
		t = pcb;
		min = size_min;
	}
}

/*
 * Stack grows from high -> low addresses
 *

 *
 *


 * ----------------  bottom of the stack( high address )
 *  small context
 * ----------------
 *
 * ----------------  top of the stack( low address )
 *
 */

void os_arch_setup_context( pcb_t *pcb ) {
	uint32_t msr;
	// Note! stack.curr already points to where to save the context

	// Check that the stack size is enough
	#define STACK_SIZE_MIN	(SC_SIZE + 16*2 )

	if( pcb->stack.size < (STACK_SIZE_MIN) ) {
		os_arch_stack_to_small(pcb, STACK_SIZE_MIN);
	}

#if 0
	// Fill stack with a nice pattern STACK_PATTERN
	{
		uint8_t *p = pcb->stack.curr;

		assert(pcb->stack.curr>pcb->stack.top);

		while(p > (uint8_t *)pcb->stack.top) {
			--p;
			*p = STACK_PATTERN;
		}
	}
#endif

	msr = MSR_EE;

#if defined(CFG_SPE)
	msr |= MSR_SPE;
#endif

	if( !pcb->application->trusted ) {
		// Non-trusted = User mode..
		msr |= MSR_PR | MSR_DS | MSR_IS;
	}
	pcb->regs[0] = msr;

	{
		uint32_t *context = (uint32_t *)pcb->stack.curr;

		context[C_CONTEXT_OFF/4] = SC_PATTERN;

		/* Set LR to start function */
		if( pcb->proc_type == PROC_EXTENDED ) {
			context[C_LR_OFF/4] = (uint32_t)os_proc_start_extended;
		} else if( pcb->proc_type == PROC_BASIC ) {
			context[C_LR_OFF/4] = (uint32_t)os_proc_start_basic;
		}
	}

	os_arch_stack_set_endmark(pcb);
}


#define C_CONTEXT_OFF	12
#define C_LR_OFF		16
#define C_CR_OFF		20

void os_arch_print_context( char *str, pcb_t *pcb ) {
	uint32_t *stack;

	dbg_printf("%s CONTEXT: %d\n",str, pcb->pid);
	dbg_printf("  stack: curr=%08x top=%08x bottom=%08x\n",pcb->stack.curr, pcb->stack.top, pcb->stack.top+ pcb->stack.size);
	stack = pcb->stack.curr;
	dbg_printf("  val  : context=%08x LR=%08x CR=%08x\n",
					stack[C_CONTEXT_OFF/4],
					stack[C_LR_OFF/4],
					stack[C_CR_OFF/4]
					);
}


void os_arch_init( void ) {
#if defined(CFG_SPE)
	uint32_t msr = get_msr();
	msr |= MSR_SPE;
	set_msr(msr);
#endif
}

#define EXC_VECTOR_CRITICAL_INPUT_OFF	0
#define EXC_VECTOR_MACHINE_CHECK_OFF	1
#define EXC_VECTOR_PROGRAM_OFF			6
#define EXC_VECTOR_DECREMENTER_OFF		10
#define EXC_VECTOR_DATA_TLB_OFF			13
#define EXC_VECTOR_INSTRUCTION_TLB_OFF	14


typedef void (*exc_func_t)(uint32_t *);

/* How do I construct the PTE table ??
 *
 * 1. Calculate the section sizes for each application
 *    ( objdump on the object file)
 * 2. Generate a Table for each application
 *
 * OS207
 * Trusted OS-applications CAN write to
 *
 * What do I do with the global data ???
 *
 *
 */


/* Move these somewhere else if we need the speed */
void os_arch_data_tlb( uint32_t *regs ) {
	uint32_t dear = regs[EXC_DEAR_OFF];
	(void)dear;
}

void os_arch_instruction_tlb( uint32_t *regs ) {
	uint32_t srr0 = regs[EXC_SRR0_OFF];
	(void)srr0;
	/* What information can I get here??
	 * - The pcb to MMU mapping ???
	 */

	/* TODO: How do I construct the PTE(Page Table Entry) ??*/

}

void os_arch_exc_program( uint32_t *regs ) {
	uint32_t esr = regs[EXC_ESR_OFF];

	if( esr & ESR_PTR ) {
		// Trap
		if( !(regs[EXC_SRR1_OFF] & MSR_PR) ) {
			// User -> Supervisor
			regs[EXC_SRR1_OFF] |= MSR_PR;
		}
	}
}


