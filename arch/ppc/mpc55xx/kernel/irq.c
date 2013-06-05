
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
/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/


/* ----------------------------[includes]------------------------------------*/

#include "internal.h"
#include "irq.h"
#include "irq_types.h"
#include "mpc55xx.h"
//#include "pcb.h"
#include "sys.h"
#include "internal.h"
#include "task_i.h"
#include "hooks.h"
#include "debug.h"
#include "isr.h"
#include "Ramlog.h"
#include <stdint.h>

/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
typedef void (*f_t)( uint32_t *);

/* ----------------------------[private function prototypes]-----------------*/
//extern uintptr_t Irq_VectorTable[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS];
//extern uint8 Irq_IsrTypeTable[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS];
//extern const OsIsrConstType *Irq_Map[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS];

//static void dumpExceptionRegs( uint32_t *regs );

/* ----------------------------[private variables]---------------------------*/
extern void exception_tbl(void);

/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/

// write 0 to pop INTC stack
void Irq_Init( void ) {
	  // Check alignment for the exception table
	  assert(((uint32)exception_tbl & 0xfff)==0);
	  set_spr(SPR_IVPR,(uint32)exception_tbl);

	  // TODO: The 5516 simulator still thinks it's a 5554 so setup the rest
#if (defined(CFG_SIMULATOR) && defined(CFG_MPC5516)) || defined(CFG_MPC5567) || defined(CFG_MPC5554)  || defined(CFG_MPC5668) || defined(CFG_MPC563XM)
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
	    set_spr(SPR_IVOR15,((uint32_t)&exception_tbl+0xf0) );
#if defined(CFG_SPE)
	    // SPE exceptions
	    set_spr(SPR_IVOR32,((uint32_t)&exception_tbl+0x100) );
	    set_spr(SPR_IVOR33,((uint32_t)&exception_tbl+0x110) );
	    set_spr(SPR_IVOR34,((uint32_t)&exception_tbl+0x120) );
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

	#if defined(CFG_MPC5516) || defined(CFG_MPC5668)
	  INTC.MCR.B.HVEN_PRC0 = 0; // Soft vector mode
	  INTC.MCR.B.VTES_PRC0 = 0; // 4 byte offset between entries
	#elif defined(CFG_MPC5554) || defined(CFG_MPC5567) || defined(CFG_MPC560X) || defined(CFG_MPC563XM)
	  INTC.MCR.B.HVEN = 0; // Soft vector mode
	  INTC.MCR.B.VTES = 0; // 4 byte offset between entries
	#endif

	  // Pop the FIFO queue
	  for (int i = 0; i < 15; i++)
	  {
	#if defined(CFG_MPC5516) || defined(CFG_MPC5668)
	    INTC.EOIR_PRC0.R = 0;
	#elif defined(CFG_MPC5554) || defined(CFG_MPC5567) || defined(CFG_MPC560X) || defined(CFG_MPC563XM)
	    INTC.EOIR.R = 0;
	#endif
	  }

	  // Accept interrupts
	#if defined(CFG_MPC5516) || defined(CFG_MPC5668)
	  INTC.CPR_PRC0.B.PRI = 0;
	#elif defined(CFG_MPC5554) || defined(CFG_MPC5567) || defined(CFG_MPC560X) || defined(CFG_MPC563XM)
	  INTC.CPR.B.PRI = 0;
	#endif
}

void Irq_EOI( void ) {
#if defined(CFG_MPC5516) || defined(CFG_MPC5668)
	volatile struct INTC_tag *intc = &INTC;
	intc->EOIR_PRC0.R = 0;
#elif defined(CFG_MPC5554)||defined(CFG_MPC5567) || defined(CFG_MPC560X) || defined(CFG_MPC563XM)
	volatile struct INTC_tag *intc = &INTC;
	intc->EOIR.R = 0;
#else
#error No CPU defined
#endif
}

static inline int osPrioToCpuPio( uint8_t prio ) {
	assert(prio<32);
	assert(prio>1);
	return prio>>1;		// Os have 32 -> 16
}

void Irq_SetPriority( Cpu_t cpu,  IrqType vector, uint8_t prio ) {
#if defined(CFG_MPC5516) || defined(CFG_MPC5668)
	INTC.PSR[vector].B.PRC_SEL = cpu;
#else
	(void)cpu;
#endif
	INTC.PSR[vector].B.PRI = prio;
}




void Irq_EnableVector( int16_t vector, int priority, int core ) {

	if (vector < INTC_NUMBER_OF_INTERRUPTS) {
		Irq_SetPriority((Cpu_t)core, (IrqType)(vector + IRQ_INTERRUPT_OFFSET), osPrioToCpuPio(priority));
	} else if ((vector >= CRITICAL_INPUT_EXCEPTION)
			&& (vector<= DEBUG_EXCEPTION)) {
	} else {
		/* Invalid vector! */
		assert(0);
	}
}




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

#if defined(CFG_MPC5516) || defined(CFG_MPC5668)
	if( cpu == CPU_CORE0 ) {
		prio = INTC.CPR_PRC0.B.PRI;
	} else if ( cpu == CPU_CORE1 ) {
		prio = INTC.CPR_PRC1.B.PRI;
	 }
#elif defined(CFG_MPC5554)||defined(CFG_MPC5567) || defined(CFG_MPC560X) || defined(CFG_MPC563XM)
	(void)cpu;
	prio = INTC.CPR.B.PRI;
#else
#error No CPU defined
#endif

	return prio;
}


