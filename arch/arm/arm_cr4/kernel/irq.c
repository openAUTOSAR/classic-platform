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
#include "isr.h"
#include "irq_types.h"
#include "core_cr4.h"

extern TaskType Os_Arc_CreateIsr( void (*entry)(void ), uint8_t prio, const char *name );
extern void *Irq_VectorTable[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS];


static inline void Irq_Setup() {
	vimREG->FIRQPR0 = 0x0;
	vimREG->FIRQPR1 = 0x0;
}

void Irq_Init( void ) {
	Irq_Setup();
	//Irq_Enable();
}


/**
 *
 * @param stack_p Ptr to the current stack.
 *
 * The stack holds C, NVGPR, VGPR and the EXC frame.
 *
 */
#define MAX_WAIT_COUNT 1000
void *Irq_Entry( void *stack_p )
{
	uint32_t *stack;

	// This is the current hardware interrupt channel that we are processing.
	volatile sint8 channel;

	// This is the current OS-interrupt vector that we are processing.
	volatile sint8 virtualChannel;

	// Get the highest pending interrupt.
	volatile uint32 c = 0;
	do {
		channel = IrqGetCurrentInterruptSource();
		c++;
	} while (channel < 0 && c < MAX_WAIT_COUNT);

	if (c >= MAX_WAIT_COUNT) {
		// No interrupt is pending
		return stack_p;
	}

	// In most cases the OS-channel is the same as the hardware channel.
	virtualChannel = channel;

	// Special case for software interrupts.
	if (channel == SSI) {
		// Get the emulated interrupt channel.
		virtualChannel = systemREG1->SSISR1;
	}

	stack = (uint32_t *)stack_p;
	stack = Os_Isr_cr4(stack, virtualChannel, channel);

	//Irq_Enable();
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

	// TODO: Use NVIC_InitVector(vector, osPrioToCpuPio(pcb->prio)); here
}

/**
 * NVIC prio have priority 0-15, 0-highest priority.
 * Autosar does it the other way around, 0-Lowest priority
 *
 * Autosar    NVIC
 *   31        0
 *   30        0
 *   ..
 *   0         15
 *   0         15
 * @param prio
 * @return
 */
static inline int osPrioToCpuPio( uint8_t prio ) {
	assert(prio<32);
	prio = 31 - prio;
	return (prio>>1);
}

void Irq_EnableVector( int16_t vector, int priority, int core ) {

	if (vector < NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS) {
		IrqActivateChannel(vector);
	} else {
		/* Invalid vector! */
		assert(0);
	}
}

/**
 * Generates a soft interrupt, ie sets pending bit.
 * This could also be implemented using ISPR regs.
 *
 * @param vector
 */
void Irq_GenerateSoftInt( IrqType vector ) {
	IrqActivateChannel(SSI);
	systemREG1->SSISR1 = (0x75 << 8) | vector;
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


