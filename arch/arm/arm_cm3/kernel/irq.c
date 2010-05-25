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
#include "task_i.h"
#include "hooks.h"
#include "stm32f10x.h"
#include "misc.h"
#include "irq.h"
#include "core_cm3.h"

extern void *Irq_VectorTable[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS];

void Irq_Init( void ) {
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
}

void Irq_EOI( void ) {
	/* Note!
	 * This is not applicable on the Cortex-M3 since we
	 * can't terminate the interrupt request without popping
	 * back registers..have to be solved in the context switches
	 * themselves.
	 */
}

#define ICSR_VECTACTIVE		0x1ff

/**
 * Get Active ISR number field.
 * You can subtract 16 from the VECTACTIVE field to index into the Interrupt
 * Clear/Set Enable, Interrupt Clear Pending/SetPending and Interrupt Priority
 * Registers. INTISR[0] has vector number 16.
 *
 */
static uint32_t NVIC_GetActiveVector( void) {
	return (SCB->ICSR &  ICSR_VECTACTIVE);
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
	uint32_t vector = 0;
	uint32_t *stack;

	Irq_Disable();
	stack = (uint32_t *)stack_p;

	/* 0. Set the default handler here....
	 * 1. Grab the vector from the interrupt controller
	 *    INT_CTRL_ST[VECTACTIVE]
	 * 2. Irq_VectorTable[vector] is odd -> ISR1
	 *    Irq_VectorTable[vector] is even-> ISR2
	 */


	vector = NVIC_GetActiveVector();

	stack = Os_Isr(stack, (void *)Irq_VectorTable[vector]);
	Irq_Enable();
	return stack;
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

	// TODO: Use NVIC_Init here
	/*
  NVIC_InitTypeDef NVIC_InitStructure;

  // Enable and configure RCC global IRQ channel
  NVIC_InitStructure.NVIC_IRQChannel = RCC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
   */
}

static inline int osPrioToCpuPio( uint8_t prio ) {
	assert(prio<32);
	return prio>>1;
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
	NVIC_InitTypeDef irqInit;

	pcb = os_find_task(tid);
	Irq_VectorTable[vector+16] = (void *)pcb;

	irqInit.NVIC_IRQChannel = vector;
	irqInit.NVIC_IRQChannelPreemptionPriority = osPrioToCpuPio(pcb->prio);
	irqInit.NVIC_IRQChannelSubPriority = 0;
	irqInit.NVIC_IRQChannelCmd = ENABLE;


	// TODO: Same as for AttachIsr1
	NVIC_Init(&irqInit);
}


/**
 * Generates a soft interrupt, ie sets pending bit.
 * This could also be implemented using ISPR regs.
 *
 * @param vector
 */
void Irq_GenerateSoftInt( IrqType vector ) {

	NVIC->STIR = (vector + 16);
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

typedef struct {
	uint32_t dummy;
} exc_stack_t;


