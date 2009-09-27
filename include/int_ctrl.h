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
 * int_ctrl.h
 *
 *  Created on: Jul 31, 2009
 *      Author: mahi
 */

#ifndef INT_CTRL_H_
#define INT_CTRL_H_

#include "irq.h"

typedef void ( * func_t)(void);


/**
 * Init the interrupt controller
 */
void IntCtrl_Init( void );

/**
 * End-Of-Interrupt. Called by the OS it wants to clear the interrupt.
 */
void IntCtrl_EOI( void );


/**
 *
 * @param stack_p Ptr to the current stack.
 *
 * The stack holds C, NVGPR, VGPR and the EXC frame.
 *
 */
void *IntCtrl_Entry( void *stack_p );

/**
 * Attach an ISR type 1 to the interrupt controller.
 *
 * @param entry
 * @param int_ctrl
 * @param vector
 * @param prio
 */
void IntCtrl_AttachIsr1( void (*entry)(void), void *int_ctrl, uint32_t vector, uint8_t prio);

/**
 * Attach a ISR type 2 to the interrupt controller.
 *
 * @param tid       The task id
 * @param int_ctrl  The interrupt controller, The is NULL for now.
 * @param vector 	The vector to attach to
 */
void IntCtrl_AttachIsr2(TaskType tid,void *int_ctrl,IrqType vector );

/**
 * Generates a soft interrupt
 * @param vector
 */
void IntCtrl_GenerateSoftInt( IrqType vector );
/**
 * Get the current priority from the interrupt controller.
 * @param cpu
 * @return
 */
uint8_t IntCtrl_GetCurrentPriority( Cpu_t cpu);

#if 0
typedef struct {
	uint32_t dummy;
} exc_stack_t;
#endif


#endif /* INT_CTRL_H_ */
