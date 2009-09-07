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








#include "typedefs.h"
#include "asm_book_e.h"
#include "mpc55xx.h"
#if !defined(USE_KERNEL)
#include "Mcu.h"
#endif
#include <assert.h>
//#include <stdio.h>

#if defined(USE_KERNEL)
#include "pcb.h"
#include "sys.h"
#include "internal.h"
#include "task_i.h"
#include "hooks.h"
#include "swap.h"

#define INTC_SSCIR0_CLR7					7
#define MLB_SERVICE_REQUEST					293
#define CRITICAL_INPUT_EXCEPTION			320
#define DEBUG_EXCEPTION						335
#define NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS	336

#include "Trace.h"
#endif

static void dump_exception_regs( uint32_t *regs );

typedef void (*f_t)( uint32_t *);
typedef void (*func_t)();

#if defined(USE_KERNEL)
extern void * intc_vector_tbl[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS];
extern uint8 intc_type_tbl[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS];
#else
extern func_t intc_vector_tbl[];
#endif

/* Handle INTC
 *
 * When we get here we have saved
 * - exception frame
 * - VGPR
 * - ?
 *
 * */



void IntCtrl_AttachIsr1( void (*entry)(void), void *int_ctrl, uint32_t vector,uint8_t prio) {
#if defined(CFG_MPC5516)
	uint8_t cpu = 0; /* 0- cpu Z1, 1-CPU Z0(slave) */
#endif

	intc_vector_tbl[vector] = (void *)entry;
	intc_type_tbl[vector] = PROC_ISR1;

	if (vector <= MLB_SERVICE_REQUEST) {
#if defined(CFG_MPC5516)
		INTC.PSR[vector].B.PRC_SEL = cpu;
#endif
		INTC.PSR[vector].B.PRI = prio;

	} else if ((vector >= CRITICAL_INPUT_EXCEPTION) && (vector
			<= DEBUG_EXCEPTION)) {
	} else {
		/* Invalid vector! */
		assert(0);
	}

}

void IntCtrl_AttachIsr2(TaskType tid,void *int_ctrl,uint32_t vector ) {
#if defined(CFG_MPC5516)
	uint8_t cpu = 0; /* 0- cpu Z1, 1-CPU Z0(slave) */
#endif
	pcb_t *pcb;

	pcb = os_find_task(tid);
	intc_vector_tbl[vector] = (void *)pcb;
	intc_type_tbl[vector] = PROC_ISR2;

	if (vector <= MLB_SERVICE_REQUEST) {
#if defined(CFG_MPC5516)
		INTC.PSR[vector].B.PRC_SEL = cpu;
#endif
		INTC.PSR[vector].B.PRI = pcb->prio;

	} else if ((vector >= CRITICAL_INPUT_EXCEPTION) && (vector
			<= DEBUG_EXCEPTION)) {
	} else {
		/* Invalid vector! */
		assert(0);
	}
}


/**
 *
 * USE_KERNEL
 * Following must be done before coming here:
 * - Swapped to interrupt stack, or???
 *
 * @param stack Ptr to current stack. The context just saved
 * can be access with positive offsets.
 */

void *mcu_intc( uint32_t *stack ) {
	uint32_t vector;

#if defined(CFG_MPC5516)
	struct INTC_tag *intc = &INTC;
#else
	volatile struct INTC_tag *intc = &INTC;
#endif

#if defined(USE_KERNEL)
	struct pcb_s *pcb;
	struct pcb_s *preempted_pcb;
#else
	func_t t;
#endif

	// Check for exception
	if( stack[EXC_VECTOR_OFF/sizeof(uint32_t)]>=CRITICAL_INPUT_EXCEPTION )
	{
		vector = stack[EXC_VECTOR_OFF/sizeof(uint32_t)];
	}
	else
	{
#if defined(CFG_MPC5516)
		vector = (intc->IACKR_PRC0.B.INTVEC_PRC0);
#else
		vector = (intc->IACKR.B.INTVEC);
#endif
		// save the vector for later
		stack[EXC_VECTOR_OFF/sizeof(uint32_t)] = vector;

		// Check for software interrupt
		if((uint32_t)vector<=INTC_SSCIR0_CLR7)
		{
			// Clear soft int
			intc->SSCIR[vector].B.CLR = 1;
		}
	}

#if defined(USE_KERNEL)

	os_sys.int_nest_cnt++;

	if( intc_type_tbl[vector] == PROC_ISR1 ) {
		// It's a function, just call it.
		((func_t)intc_vector_tbl[vector])();
	} else {
		// It's a PCB

		// Save info for preemted pcb
		preempted_pcb = get_curr_pcb();
		preempted_pcb->stack.curr = stack;
		preempted_pcb->state = ST_READY;
		os_isr_printf(D_TASK,"Preempted %s\n",preempted_pcb->name);

		POSTTASKHOOK();

		pcb = intc_vector_tbl[vector];
		pcb->state = ST_RUNNING;
		set_curr_pcb(pcb);

		PRETASKHOOK();

		// We should not get here if we're SCHEDULING_NONE
		if( pcb->scheduling == SCHEDULING_NONE) {
			// TODO:
			// assert(0);
			while(1);
		}
		//Irq_Enable(); // Added by Mattias
		//Irq_Enable();
		pcb->entry();
		Irq_Disable();
	}

	pcb->state = ST_SUSPENDED;
	POSTTASKHOOK();

	// write 0 to pop INTC stack

#if defined(CFG_MPC5516)
	intc->EOIR_PRC0.R = 0;
#else
	intc->EOIR.R = 0;
#endif
	--os_sys.int_nest_cnt;

	// TODO: Check stack check marker....
	// We have preempted a task
	if( (os_sys.int_nest_cnt == 0) ) { //&& is_idle_task() ) {
		/* If we get here:
		 * - the preempted task is saved with large context.
		 * - We are on interrupt stack..( this function )
		 *
		 * if we find a new task:
		 * - just switch in the new context( don't save the old because
		 *   its already saved )
		 *
		 */
		pcb_t *new_pcb;
		new_pcb = os_find_top_prio_proc();
		if( new_pcb != preempted_pcb ) {
			os_isr_printf(D_TASK,"Found candidate %s\n",new_pcb->name);
//#warning os_swap_context_to should call the pretaskswaphook
			os_swap_context_to(NULL,new_pcb);
		} else {
			if( new_pcb == NULL ) {
				assert(0);
			}
			preempted_pcb->state = ST_RUNNING;
			set_curr_pcb(preempted_pcb);
		}
	}

	return stack;

#else
	//read address
	t = (func_t)intc_vector_tbl[vector];

	if( t == ((void *)0) )
	{
		while(1);
	}

	// Enable nestling interrupts
	Irq_Enable();
	t();
	Irq_Disable();

	if( vector < INTC_NUMBER_OF_INTERRUPTS )
	{
		// write 0 to pop INTC stack
		intc->EOIR_PRC0.R = 0;
	}
	return NULL;
#endif
}

void dummy (void);

// Critical Input Interrupt
void IVOR0Exception (uint32_t *regs)
{
//	srr0 = get_spr(SPR_SRR0);
//	srr1 = get_spr(SPR_SRR0);
//	ExceptionSave(srr0,srr1,esr,mcsr,dear;)
	// CSRR0, CSSR1
	// Nothing more
	dump_exception_regs(regs);
	while (1);
}

// Machine check
void IVOR1Exception (uint32_t *regs)
{
	// CSRR0, CSSR1
	// MCSR - Source of machine check
	dump_exception_regs(regs);
   while (1);
}
// Data Storage Interrupt
void IVOR2Exception (uint32_t *regs)
{
	// SRR0, SRR1
	// ESR - lots of stuff
	dump_exception_regs(regs);
   while (1);
}

// Instruction Storage Interrupt
void IVOR3Exception (uint32_t *regs)
{
	// SRR0, SRR1
	// ESR - lots of stuff
	dump_exception_regs(regs);
   while (1);
}

// Alignment Interrupt
void IVOR5Exception (uint32_t *regs)
{
	// SRR0, SRR1
	// ESR - lots of stuff
	// DEAR - Address of load store that caused the exception
	dump_exception_regs(regs);
   while (1);
}

// Program Interrupt
void IVOR6Exception (uint32_t *regs)
{
	// SRR0, SRR1
	// ESR - lots of stuff
	dump_exception_regs(regs);
	while (1);
}

// Floating point unavailable
void IVOR7Exception (uint32_t *regs)
{
	// SRR0, SRR1
	dump_exception_regs(regs);
   while (1);
}

// System call
void IVOR8Exception (uint32_t *regs)
{
	// SRR0, SRR1
	// ESR
	dump_exception_regs(regs);
	while (1);
}

// Aux processor Unavailable
void IVOR9Exception (uint32_t *regs)
{
	// Does not happen on e200
	dump_exception_regs(regs);
	while (1);
}
#if 0
// Decrementer
void IVOR10Exception (uint32_t *regs)
{
	// SRR0, SRR1
	while (1);
}
#endif

// FIT
void IVOR11Exception (uint32_t *regs)
{
	// SRR0, SRR1
	dump_exception_regs(regs);
	while (1);
}

// Watchdog Timer
void IVOR12Exception (uint32_t *regs)
{
	// SRR0, SRR1
	dump_exception_regs(regs);
	while (1);
}

// Data TLB Error Interrupt
void IVOR13Exception (uint32_t *regs)
{
#if 0
	uint32_t srr0,srr1,esr,dear;

	srr0 = regs[SC_GPRS_SIZE+(EXC_SRR0_OFF/4)];
	srr1 = regs[SC_GPRS_SIZE+(EXC_SRR1_OFF/4)];
	esr = regs[SC_GPRS_SIZE+(EXC_ESR_OFF/4)];
	dear = regs[SC_GPRS_SIZE+(EXC_DEAR_OFF/4)];
	dump_exception_regs(regs);
#endif

	// SRR0, SRR1
	// ESR - lots
	// DEAR -
	while (1);
}

// Instruction TLB Error Interupt
void IVOR14Exception (uint32_t *regs)
{
	// SRR0, SRR1
	// ESR - MIF set, All others cleared
	dump_exception_regs(regs);
	while (1);
}

void IVOR15Exception (uint32_t *regs)
{
	// Debug
	dump_exception_regs(regs);
	while (1);
}

#if defined(CFG_CONSOLE_T32) || defined(CFG_CONSOLE_WINIDEA)

typedef struct {
	uint32_t sp;
	uint32_t bc;  // backchain
	uint32_t pad;
	uint32_t srr0;
	uint32_t srr1;
	uint32_t lr;
	uint32_t ctr;
	uint32_t xer;
	uint32_t cr;
	uint32_t esr;
	uint32_t mcsr;
	uint32_t dear;
	uint32_t vector;
	uint32_t r3;
	uint32_t r4;
} exc_stack_t;



static void dump_exception_regs( uint32_t *regs ) {
	exc_stack_t *r = (exc_stack_t *)regs;

dbg_printf("sp   %08x  srr0 %08x  srr1 %08x\n",r->sp,r->srr0,r->srr1);
dbg_printf("lr   %08x  ctr  %08x  xer  %08x\n",r->lr,r->ctr,r->xer);
dbg_printf("cr   %08x  esr  %08x  mcsr %08x\n",r->cr,r->esr,r->mcsr);
dbg_printf("dear %08x  vec  %08x  r3   %08x\n",r->dear,r->vector,r->r3);
dbg_printf("r4   %08x\n",r->r4);
}

#else
static void dump_exception_regs( uint32_t *regs ) {
}
#endif

#if !defined(USE_KERNEL)
func_t intc_vector_tbl[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS] __attribute__ ((aligned (0x800))) = {
 dummy, dummy, dummy, dummy, dummy, /* ISRs 00 - 04 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 05 - 09 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 10 - 14 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 15 - 19 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 20 - 24 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 25 - 29 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 30 - 34 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 35 - 39 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 40 - 44 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 45 - 49 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 50 - 54 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 55 - 59 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 60 - 64 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 55 - 69 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 70 - 74 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 75 - 79 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 80 - 84 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 85 - 89 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 90 - 94 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 95 - 99 */

 dummy, dummy, dummy, dummy, dummy, /* ISRs 100 - 104 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 105 - 109 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 110 - 114 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 115 - 119 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 120 - 124 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 125 - 129 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 130 - 134 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 135 - 139 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 140 - 144 */
 dummy, dummy, dummy, dummy, dummy /* PIT1 */, /* ISRs 145 - 149 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 150 - 154 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 155 - 159 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 160 - 164 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 155 - 169 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 170 - 174 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 175 - 179 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 180 - 184 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 185 - 189 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 190 - 194 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 195 - 199 */

 dummy, dummy, dummy, dummy, dummy, /* ISRs 200 - 204 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 205 - 209 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 210 - 214 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 215 - 219 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 220 - 224 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 225 - 229 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 230 - 234 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 235 - 239 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 240 - 244 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 245 - 249 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 250 - 254 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 255 - 259 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 260 - 264 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 255 - 269 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 270 - 274 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 275 - 279 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 280 - 284 */
 dummy, dummy, dummy, dummy, dummy, /* ISRs 285 - 289 */
 dummy, dummy, dummy, dummy,        /* ISRs 290 - 293 */

 /* Some reserved vectors between INC interrupts and exceptions. */
 dummy,                             /* INTC_NUMBER_OF_INTERRUPTS */

 dummy, dummy, dummy, dummy, dummy,
 dummy, dummy, dummy, dummy, dummy,
 dummy, dummy, dummy, dummy, dummy,
 dummy, dummy, dummy, dummy, dummy,
 dummy, dummy, dummy, dummy, dummy,

 IVOR0Exception,                    /* CRITICAL_INPUT_EXCEPTION, */
 IVOR1Exception,                    /* MACHINE_CHECK_EXCEPTION */
 IVOR2Exception,                    /* DATA_STORAGE_EXCEPTION */
 IVOR3Exception,                    /* INSTRUCTION_STORAGE_EXCEPTION */
 dummy,                             /* EXTERNAL_INTERRUPT */
                                    /* This is the place where the "normal" interrupts will hit the CPU... */
 IVOR5Exception,                    /* ALIGNMENT_EXCEPTION */
 IVOR6Exception,                    /* PROGRAM_EXCEPTION */
 IVOR7Exception,                    /* FLOATING_POINT_EXCEPTION */
 IVOR8Exception,                    /* SYSTEM_CALL_EXCEPTION */
 dummy,                             /* AUX_EXCEPTION Not implemented in MPC5516. */
 dummy,                             /* DECREMENTER_EXCEPTION */
 IVOR11Exception,                   /* FIXED_INTERVAL_TIMER_EXCEPTION */
 IVOR12Exception,                   /* WATCHDOG_TIMER_EXCEPTION */
 IVOR13Exception,                   /* DATA_TLB_EXCEPTION */
 IVOR14Exception,                   /* INSTRUCTION_TLB_EXCEPTION */
 IVOR15Exception,                   /* DEBUG_EXCEPTION */
};

void dummy (void) {
   while (1){
	   /* TODO: Rename and check for what spurious interrupt have happend */
   };
 }

#endif
