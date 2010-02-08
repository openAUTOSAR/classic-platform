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
#include "irq.h"

extern void *Irq_VectorTable[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS];

void Irq_Init( void ) {

}

void Irq_EOI( void ) {

}


static uint32_t NVIC_GetActiveVector( void) {
	return 0;
}

void *Irq_Entry( void *stack_p )
{
	return NULL;
}

/**
 * Attach an ISR type 1 to the interrupt controller.
 *
 * @param entry
 * @param int_ctrl
 * @param vector
 * @param prio
 */
void Irq_AttachIsr1( void (*entry)(void), void *int_ctrl, uint32_t vector, uint8_t prio) {
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

}


/**
 * Generates a soft interrupt, ie sets pending bit.
 * This could also be implemented using ISPR regs.
 *
 * @param vector
 */
void Irq_GenerateSoftInt( IrqType vector ) {
	(void)vector;
}

/**
 * Get the current priority from the interrupt controller.
 * @param cpu
 * @return
 */
uint8_t Irq_GetCurrentPriority( Cpu_t cpu) {

	uint8_t prio = 0;

	// SCB_ICSR contains the active vector
	return prio;
}

