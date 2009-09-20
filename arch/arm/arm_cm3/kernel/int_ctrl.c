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
 * int_ctrl.c
 *
 *  Created on: Jul 13, 2009
 *      Author: mahi
 */

#include "pcb.h"
#include "sys.h"
#include "internal.h"
#include "task_i.h"
#include "hooks.h"
#include "swap.h"
#include "stm32f10x.h"
#include "misc.h"
#include "int_ctrl.h"

extern void * intc_vector_tbl[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS];
extern uint8 intc_type_tbl[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS];


void IntCtrl_Init( void ) {


}

void IntCtrl_EOI( void ) {

}

/**
 *
 * @param stack_p Ptr to the current stack.
 *
 * The stack holds C, NVGPR, VGPR and the EXC frame.
 *
 */
void *IntCtrl_Entry( void *stack_p )
{
	uint32_t vector = 0;
	uint32_t *stack = (uint32_t *)stack_p;
//	uint32_t exc_vector = (EXC_OFF_FROM_BOTTOM+EXC_VECTOR_OFF)  / sizeof(uint32_t);

	if( intc_type_tbl[vector] == PROC_ISR1 ) {
		// It's a function, just call it.
		((func_t)intc_vector_tbl[vector])();
		return stack;
	} else {
		// It's a PCB
		// Let the kernel handle the rest,
		return Os_Isr(stack, (void *)intc_vector_tbl[vector]);
	}
}

/**
 * Attach an ISR type 1 to the interrupt controller.
 *
 * @param entry
 * @param int_ctrl
 * @param vector
 * @param prio
 */
void IntCtrl_AttachIsr1( void (*entry)(void), void *int_ctrl, uint32_t vector, uint8_t prio) {

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

/**
 * Attach a ISR type 2 to the interrupt controller.
 *
 * @param tid
 * @param int_ctrl
 * @param vector
 */
void IntCtrl_AttachIsr2(TaskType tid,void *int_ctrl,uint32_t vector ) {
	pcb_t *pcb;
	NVIC_InitTypeDef irqInit;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

	irqInit.NVIC_IRQChannel = vector;
	irqInit.NVIC_IRQChannelPreemptionPriority = 4;
	irqInit.NVIC_IRQChannelSubPriority = 4;
	irqInit.NVIC_IRQChannelCmd = ENABLE;

	pcb = os_find_task(tid);

	// TODO: Same as for AttachIsr1
	NVIC_Init(&irqInit);
}


/**
 * Generates a soft interrupt
 * @param vector
 */
void IntCtrl_GenerateSoftInt( IrqType vector ) {

	// NVIC_STIR
}

/**
 * Get the current priority from the interrupt controller.
 * @param cpu
 * @return
 */
uint8_t IntCtrl_GetCurrentPriority( Cpu_t cpu) {

	uint8_t prio = 0;

	// SCB_ICSR contains the active vector
	return prio;
}

typedef struct {
	uint32_t dummy;
} exc_stack_t;


