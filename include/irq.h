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


#ifndef IRQ_H_
#define IRQ_H_

#include "irq_types.h"
#include "bit.h"

typedef void ( * func_t)(void);

extern uint8_t Irq_PriorityTable[];
extern uint8_t Irq_IsrTypeTable[];

#define ISR_TYPE_1			0
#define ISR_TYPE_2			1

typedef _Bool IsrType;

/**
 * Init the interrupt controller
 */
void Irq_Init( void );

/**
 * End-Of-Interrupt. Called by the OS it wants to clear the interrupt.
 */
void Irq_EOI( void );

#if defined(CFG_HC1X)
/**
 *
 * @param stack Ptr to the current stack.
 * @param irq_nr The nr. of the interrupt being handled.
 *
 * The stack holds C, NVGPR, VGPR and the EXC frame.
 *
 */
void *Irq_Entry( uint8_t irq_nr, void *stack );

#else
/**
 *
 * @param stack_p Ptr to the current stack.
 *
 * The stack holds C, NVGPR, VGPR and the EXC frame.
 *
 */
void *Irq_Entry( void *stack_p );
#endif
/**
 * Attach an ISR type 1 to the interrupt controller.
 *
 * @param entry
 * @param int_ctrl
 * @param vector
 * @param prio
 */
void Irq_AttachIsr1( void (*entry)(void), void *int_ctrl, uint32_t vector, uint8_t prio);

/**
 * Attach a ISR type 2 to the interrupt controller.
 *
 * @param tid       The task id
 * @param int_ctrl  The interrupt controller, The is NULL for now.
 * @param vector 	The vector to attach to
 */
void Irq_AttachIsr2(TaskType tid,void *int_ctrl,IrqType vector );

/**
 * Generates a soft interrupt
 * @param vector
 */
void Irq_GenerateSoftInt( IrqType vector );
/**
 * Get the current priority from the interrupt controller.
 * @param cpu
 * @return
 */
uint8_t Irq_GetCurrentPriority( Cpu_t cpu);


/**
 * Set the priority in the interrupt controller for vector
 */
void Irq_SetPriority( Cpu_t cpu,  IrqType vector, uint8_t prio );

/**
 *
 * @param vector
 * @param type
 */
static inline void Irq_SetIsrType( IrqType vector, IsrType type ) {
	Irq_IsrTypeTable[vector + IRQ_INTERRUPT_OFFSET ] = type;
}

/**
 *
 * @param vector
 * @return 0 - Isr1
 *         1 - Isr2
 */
static inline IsrType Irq_GetIsrType( IrqType vector )  {
	return Irq_IsrTypeTable[vector + IRQ_INTERRUPT_OFFSET ];
}


#if 0
typedef struct {
	uint32_t dummy;
} exc_stack_t;
#endif


#endif /* IRQ_H_ */
