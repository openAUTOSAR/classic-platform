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
#include "irq.h"
#include "core_cr4.h"

extern TaskType Os_Arc_CreateIsr( void (*entry)(void ), uint8_t prio, const char *name );
extern void *Irq_VectorTable[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS];

/*
* PRIGROUP[2:0] 	Group prios 	Sub prios
* 0b011 			16 				None
* 0b100 			8 				2
* 0b101 			4 				4
* 0b110 			2 				8
* 0b111 			None 			16
*/
#define AIRCR_VECTKEY    ((uint32_t)0x05FA0000)

/** Set NVIC prio group */
/* TODO remove
static void NVIC_SetPrioGroup(uint32_t prioGroup)
{
  SCB->AIRCR = AIRCR_VECTKEY | (prioGroup<<8);
}
*/



static inline void Irq_Setup() {
	vimREG->FIRQPR0 = 0x0;
	vimREG->FIRQPR1 = 0x0;
}

void Irq_Init( void ) {
	Irq_Setup();
	Irq_Enable();
}


#define ICSR_VECTACTIVE		0x1ff

/**
 * Get Active ISR number field.
 * You can subtract 16 from the VECTACTIVE field to index into the Interrupt
 * Clear/Set Enable, Interrupt Clear Pending/SetPending and Interrupt Priority
 * Registers. INTISR[0] has vector number 16.
 *
 */
/* TODO remove
static uint32_t NVIC_GetActiveVector( void) {
	return (SCB->ICSR &  ICSR_VECTACTIVE);
}
*/

/**
 * Init NVIC vector. We do not use subriority
 *
 */

/**
 * Init NVIC vector. We do not use subpriority
 *
 * @param vector	The IRQ number
 * @param prio      NVIC priority, 0-15, 0-high prio
 */
/* TODO remove
static void NVIC_InitVector(IRQn_Type vector, uint32_t prio)
{
	// Set prio
	NVIC_SetPriority(vector,prio);
	//NVIC->IP[vector] = prio;

	// Enable
	//NVIC_EnableIRQ(vector);
    NVIC->ISER[vector >> 5] = (uint32_t)1 << (vector & (uint8_t)0x1F);
}
*/

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

	// Get the active interrupt channel
	volatile sint8 channel;
	volatile uint32 c = 0;
	do {
		channel = IrqGetCurrentInterruptSource();
		c++;
	} while (channel < 0 && c < MAX_WAIT_COUNT);

	if (c >= MAX_WAIT_COUNT) {
		// No interrupt is pending
		return stack_p;

		// This irq is a result of a svc call
		//register uint32 irq_vector asm("r1");
		//channel = irq_vector;
		//channel = IrqGetCurrentInterruptSource();
	}

	//Irq_Disable();
	stack = (uint32_t *)stack_p;

	stack = Os_Isr(stack, (void *)Irq_VectorTable[channel]);

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
	IrqActivateChannel(vector);

	// TOdo replace NVIC_InitVector(vector, osPrioToCpuPio(pcb->prio));
}


void SoftIrqRunner() {
	uint32 vector = systemREG1->SSISR1 & 0xFF;
	struct OsPcb *isr_p = Irq_VectorTable[SYS_SWI_NR];
}

/**
 * Generates a soft interrupt, ie sets pending bit.
 * This could also be implemented using ISPR regs.
 *
 * @param vector
 */
void Irq_GenerateSoftInt( IrqType vector ) {
	if (Irq_VectorTable[SYS_SWI_NR] == NULL) {
		TaskType tid = Os_Arc_CreateIsr(SoftIrqRunner,6,"SoftIrq");
		Irq_AttachIsr2(tid,NULL, SYS_SWI_NR);
	}
	systemREG1->SSISR1 = 0x00007500 + vector;
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


