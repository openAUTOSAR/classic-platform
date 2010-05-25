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

#include "internal.h"
#include "asm_book_e.h"
#include "irq_types.h"
#include "mpc55xx.h"
#if !defined(USE_KERNEL)
#include "Mcu.h"
#endif

#if defined(USE_KERNEL)
#include "pcb.h"
#include "sys.h"
#include "internal.h"
#include "task_i.h"
#include "hooks.h"

#if 0
#define INTC_SSCIR0_CLR7					7
#define MLB_SERVICE_REQUEST					293
#define CRITICAL_INPUT_EXCEPTION			320
#define DEBUG_EXCEPTION						335
#define NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS	336
#endif

#include "debug.h"
#endif
#include "irq.h"

static void dump_exception_regs( uint32_t *regs );

typedef void (*f_t)( uint32_t *);
//typedef void (*func_t)();
//extern vfunc_t Irq_VectorTable[];
extern void exception_tbl(void);



#if defined(USE_KERNEL)
extern void * Irq_VectorTable[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS];
extern uint8 Irq_IsrTypeTable[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS];
#else
extern func_t Irq_VectorTable[];
#endif

// write 0 to pop INTC stack
void Irq_Init( void ) {
	  // Check alignment for the exception table
	  assert(((uint32)exception_tbl & 0xfff)==0);
	  set_spr(SPR_IVPR,(uint32)exception_tbl);

	  ramlog_str("Test\n");
	  ramlog_hex(0x10);
	  ramlog_dec(20);

	  // TODO: The 5516 simulator still thinks it's a 5554 so setup the rest
#if (defined(CFG_SIMULATOR) && defined(CFG_MPC5516)) || defined(CFG_MPC5567) || defined(CFG_MPC5554)
	    set_spr(SPR_IVOR0,((uint32_t)&exception_tbl+0x0) );
	    set_spr(SPR_IVOR1,((uint32_t)&exception_tbl+0x10) );
	    set_spr(SPR_IVOR2,((uint32_t)&exception_tbl+0x20) );
	    set_spr(SPR_IVOR3,((uint32_t)&exception_tbl+0x30) );
	    set_spr(SPR_IVOR4,((uint32_t)&exception_tbl+0x40) );
	    set_spr(SPR_IVOR5,((uint32_t)&exception_tbl+0x50) );
	    set_spr(SPR_IVOR6,((uint32_t)&exception_tbl+0x60) );
	    set_spr(SPR_IVOR7,((uint32_t)&exception_tbl+0x70) );
	    set_spr(SPR_IVOR8,((uint32_t)&exception_tbl+0x80) );
	    set_spr(SPR_IVOR9,((uint32_t)&exception_tbl+0x90) );
	    set_spr(SPR_IVOR10,((uint32_t)&exception_tbl+0xa0) );
	    set_spr(SPR_IVOR11,((uint32_t)&exception_tbl+0xb0) );
	    set_spr(SPR_IVOR12,((uint32_t)&exception_tbl+0xc0) );
	    set_spr(SPR_IVOR13,((uint32_t)&exception_tbl+0xd0) );
	    set_spr(SPR_IVOR14,((uint32_t)&exception_tbl+0xe0) );
#if defined(CFG_SPE)
	    // SPE exceptions...map to dummy
	    set_spr(SPR_IVOR32,((uint32_t)&exception_tbl+0xf0) );
	    set_spr(SPR_IVOR33,((uint32_t)&exception_tbl+0xf0) );
	    set_spr(SPR_IVOR34,((uint32_t)&exception_tbl+0xf0) );
#endif
#endif

	  //
	  // Setup INTC
	  //
	  // according to manual
	  //
	  // 1. configure VTES_PRC0,VTES_PRC1,HVEN_PRC0 and HVEN_PRC1 in INTC_MCR
	  // 2. configure VTBA_PRCx in INTC_IACKR_PRCx
	  // 3. raise the PRIx fields and set the PRC_SELx fields to the desired processor in INTC_PSRx_x
	  // 4. set the enable bits or clear the mask bits for the peripheral interrupt requests
	  // 5. lower PRI in INTC_CPR_PRCx to zero
	  // 6. enable processor(s) recognition of interrupts

	  // Z1 init

	#if defined(CFG_MPC5516)
	  INTC.MCR.B.HVEN_PRC0 = 0; // Soft vector mode
	  INTC.MCR.B.VTES_PRC0 = 0; // 4 byte offset between entries
	#elif defined(CFG_MPC5554) || defined(CFG_MPC5567)
	  INTC.MCR.B.HVEN = 0; // Soft vector mode
	  INTC.MCR.B.VTES = 0; // 4 byte offset between entries
	#endif


	  // Pointless in software vector more???
#if 0
	  // Check alignment requirements for the INTC table
	  assert( (((uint32_t)&Irq_VectorTable[0]) & 0x7ff) == 0 );
	#if defined(CFG_MPC5516)
	  INTC.IACKR_PRC0.R = (uint32_t) & Irq_VectorTable[0]; // Set INTC ISR vector table
	#elif defined(CFG_MPC5554) || defined(CFG_MPC5567)
	  INTC.IACKR.R = (uint32_t) & Irq_VectorTable[0]; // Set INTC ISR vector table
	#endif
#endif
	  // Pop the FIFO queue
	  for (int i = 0; i < 15; i++)
	  {
	#if defined(CFG_MPC5516)
	    INTC.EOIR_PRC0.R = 0;
	#elif defined(CFG_MPC5554) || defined(CFG_MPC5567)
	    INTC.EOIR.R = 0;
	#endif
	  }

	  // Accept interrupts
	#if defined(CFG_MPC5516)
	  INTC.CPR_PRC0.B.PRI = 0;
	#elif defined(CFG_MPC5554) || defined(CFG_MPC5567)
	  INTC.CPR.B.PRI = 0;
	#endif

}


void Irq_EOI( void ) {
#if defined(CFG_MPC5516)
	struct INTC_tag *intc = &INTC;
	intc->EOIR_PRC0.R = 0;
#elif defined(CFG_MPC5554)||defined(CFG_MPC5567)
	volatile struct INTC_tag *intc = &INTC;
	intc->EOIR.R = 0;
#endif
}


/**
 *
 * @param stack_p Ptr to the current stack.
 *
 * The stack holds C, NVGPR, VGPR and the EXC frame.
 *
 */
void *Irq_Entry( void *stack_p )
{
	uint32_t vector;
	uint32_t *stack = (uint32_t *)stack_p;
	uint32_t exc_vector = (EXC_OFF_FROM_BOTTOM+EXC_VECTOR_OFF)  / sizeof(uint32_t);

	// Check for exception
	if( stack[exc_vector]>=CRITICAL_INPUT_EXCEPTION )
	{
		vector = stack[exc_vector];
	}
	else
	{
#if defined(CFG_MPC5516)
		struct INTC_tag *intc = &INTC;
		vector = (intc->IACKR_PRC0.B.INTVEC_PRC0);
#elif defined(CFG_MPC5554)||defined(CFG_MPC5567)
		volatile struct INTC_tag *intc = &INTC;
		vector = (intc->IACKR.B.INTVEC);
#endif
		// save the vector for later
		stack[exc_vector] = vector;

		// Check for software interrupt
		if((uint32_t)vector<=INTC_SSCIR0_CLR7)
		{
			// Clear soft int
			intc->SSCIR[vector].B.CLR = 1;
			asm("mbar 0");
		}
	}

#if defined(USE_KERNEL)

	if( Irq_GetIsrType(vector) == ISR_TYPE_1 ) {
		// It's a function, just call it.
		((func_t)Irq_VectorTable[vector])();
		return stack;
	} else {
		// It's a PCB
		// Let the kernel handle the rest,
		return Os_Isr(stack, (void *)Irq_VectorTable[vector]);
	}


#else
		//read address
	t = (func_t)Irq_VectorTable[vector];

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



#if defined(USE_KERNEL)



static inline int osPrioToCpuPio( uint8_t prio ) {
	assert(prio<32);
	return prio>>1;		// Os have 32 -> 16
}

void Irq_SetPriority( Cpu_t cpu,  IrqType vector, uint8_t prio ) {
#if defined(CFG_MPC5516)
	INTC.PSR[vector].B.PRC_SEL = cpu;
#endif
	INTC.PSR[vector].B.PRI = prio;
}



/**
 * Attach an ISR type 1 to the interrupt controller.
 *
 * @param entry
 * @param int_ctrl
 * @param vector
 * @param prio
 */
void Irq_AttachIsr1( void (*entry)(void), void *int_ctrl, uint32_t vector,uint8_t prio) {
	Irq_VectorTable[vector] = (void *)entry;
	Irq_SetIsrType(vector, ISR_TYPE_1);

	if (vector < INTC_NUMBER_OF_INTERRUPTS) {
		Irq_SetPriority(CPU_CORE0,vector + IRQ_INTERRUPT_OFFSET, osPrioToCpuPio(prio));
	} else if ((vector >= CRITICAL_INPUT_EXCEPTION) && (vector
			<= DEBUG_EXCEPTION)) {
	} else {
		/* Invalid vector! */
		assert(0);
	}

}

/**
 * Attach a ISR type 2 to the interrupt controller.
 *
 * @param tid
 * @param int_ctrl
 * @param vector
 */
void Irq_AttachIsr2(TaskType tid,void *int_ctrl,IrqType vector ) {
	OsPcbType *pcb;

	pcb = os_find_task(tid);
	Irq_VectorTable[vector] = (void *)pcb;
	Irq_IsrTypeTable[vector] = PROC_ISR2;

	if (vector < INTC_NUMBER_OF_INTERRUPTS) {
		Irq_SetPriority(CPU_CORE0,vector + IRQ_INTERRUPT_OFFSET, osPrioToCpuPio(pcb->prio));
	} else if ((vector >= CRITICAL_INPUT_EXCEPTION) && (vector
			<= DEBUG_EXCEPTION)) {
	} else {
		/* Invalid vector! */
		assert(0);
	}
}

#endif /* defined(USE_KERNEL) */

#if !defined(USE_KERNEL)
/**
 * Installs a vector in intc vector table. It also sets the priority in the INTC
 * internal registers.
 *
 * This does NOT use the kernel
 *
 * @param func   The function to install
 * @param vector INTC vector to install it to
 * @param priority INTC priority. 0 - Low prio. 15- Highest( NMI )
 * @param cpu
 */

void Irq_InstallVector(void(*func)(), IrqType vector,
    uint8_t priority, Cpu_t cpu)
{
  VALIDATE( ( 1 == Mcu_Global.initRun ), MCU_INTCVECTORINSTALL_SERVICE_ID, MCU_E_UNINIT );
  DEBUG(DEBUG_LOW,"Installing INTC vector:%d,prio:%d,cpu,%d\n",vector,priority,cpu);
  Irq_VectorTable[vector] = func;

  if (vector <= MLB_SERVICE_REQUEST)
  {
    INTC.PSR[vector].B.PRC_SEL = cpu;
    INTC.PSR[vector].B.PRI = priority;

    Irq_VectorTable[vector] = func;
  } else if ((vector >= CRITICAL_INPUT_EXCEPTION)
      && (vector <= DEBUG_EXCEPTION))
  {
    Irq_VectorTable[vector] = func;
  } else
  {
    /* Invalid vector! */
    assert(0);
  }
}
#endif



/**
 * Generates a soft interrupt
 * @param vector
 */
void Irq_GenerateSoftInt( IrqType vector ) {
	if( vector > INTC_SSCIR0_CLR7 ) {
		assert(0);
	}

	INTC.SSCIR[vector].B.SET = 1;
}

/**
 * Get the current priority from the interrupt controller.
 * @param cpu
 * @return
 */
uint8_t Irq_GetCurrentPriority( Cpu_t cpu) {

	uint8_t prio = 0;

#if defined(CFG_MPC5516)
	if( cpu == CPU_Z1 ) {
		prio = INTC.CPR_PRC0.B.PRI;
	} else if ( cpu == CPU_Z0 ) {
		prio = INTC.CPR_PRC1.B.PRI;
	}
#elif defined(CFG_MPC5554)||defined(CFG_MPC5567)
	prio = INTC.CPR.B.PRI;
#endif

	return prio;
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

	LDEBUG_PRINTF("sp   %08x  srr0 %08x  srr1 %08x\n",r->sp,r->srr0,r->srr1);
	LDEBUG_PRINTF("lr   %08x  ctr  %08x  xer  %08x\n",r->lr,r->ctr,r->xer);
	LDEBUG_PRINTF("cr   %08x  esr  %08x  mcsr %08x\n",r->cr,r->esr,r->mcsr);
	LDEBUG_PRINTF("dear %08x  vec  %08x  r3   %08x\n",r->dear,r->vector,r->r3);
	LDEBUG_PRINTF("r4   %08x\n",r->r4);
}

#else
static void dump_exception_regs( uint32_t *regs ) {
}
#endif

#if !defined(USE_KERNEL)
func_t Irq_VectorTable[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS] __attribute__ ((aligned (0x800))) = {
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

