/*
 * irq.c
 *
 *  Created on:  Mar 4, 2013
 *      Author:  Zhang Shuzhou
 *  Reviewed on:
 *     Reviewer:
 *
 */

#include "internal.h"
#include "task_i.h"
#include "isr.h"
#include "irq_types.h"
#include "led.h"
#include "bcm2835.h"
#include "Uart.h"
uint32 irq_mask = 0;

#define clz(a) \
 ({ unsigned long __value, __arg = (a); \
     asm ("clz\t%0, %1": "=r" (__value): "r" (__arg)); \
     __value; })

void Irq_SOI( void ) {
	//start of interrupt
	//turns the selected interrupt off
	if (irq_mask == BCM2835_IRQ_ID_TIMER_0) {
		ARM_TIMER_CTL = ARM_TIMER_DISABLE;
	}
}

void Irq_EOI( void ) {
	//clear the flag of the timer interrupt
	if (irq_mask == BCM2835_IRQ_ID_TIMER_0) {
		irq_mask = 0;
		ARM_TIMER_CLI = 0;
		ARM_TIMER_CTL = ARM_TIMER_ENABLE;
	} else if (irq_mask == BCM2835_IRQ_ID_USB){
		irq_mask = 0;
		//IRQ_ENABLE1 = 1;
	}

}


//int DisableInterrupts(void) {
//	irqDisable();
//	return 0;
//}

void Irq_Init( void ) {

}

void *Irq_Entry(void *stack_p) { //
	uint32 *stack;
	stack = (uint32 *) stack_p;

	if (IRQ_BASIC & ARM_TIMER_IRQ) { // Note how we mask out the GPU interrupt Aliases. IRQ_BASIC & ARM_TIMER_IRQ
		irq_mask = BCM2835_IRQ_ID_TIMER_0;
		stack = Os_Isr(stack, BCM2835_IRQ_ID_TIMER_0);
	} else if (IRQ_PEND1 & USB_IRQ) {
		irq_mask = BCM2835_IRQ_ID_USB;
		stack = Os_Isr(stack, BCM2835_IRQ_ID_USB);
	} else if (IRQ_PEND1 & AUX_IRQ) {
		stack = Os_Isr(stack, BCM2835_IRQ_ID_AUX_UART);
	} else if (IRQ_PEND2 & I2C_IRQ) {
		irq_mask = BCM2835_IRQ_ID_I2C;
		stack = Os_Isr(stack, BCM2835_IRQ_ID_I2C);
	} else if (IRQ_PEND2 & GPIO_IRQ0) {
		stack = Os_Isr(stack, BCM2835_IRQ_ID_GPIO_0);
	} else if (IRQ_PEND2 & SPI_IRQ) {
		stack = Os_Isr(stack, BCM2835_IRQ_ID_SPI);
	}

	return stack;
}

void Irq_EnableVector( int16_t vector, int priority, int core ) {
}

/**
 * NVIC prio have priority 0-31, 0-highest priority.
 * Autosar does it the other way around, 0-Lowest priority
 * NOTE: prio 255 is reserved for SVC and PendSV
 *
 * Autosar    NVIC
 *   31        0
 *   30        1
 *   ..
 *   0         31
 * @param prio
 * @return
 */
static inline int osPrioToCpuPio( uint8_t prio ) {
	assert(prio<32);
	prio = 31 - prio;
	return 0;
}

/**
 * Generates a soft interrupt, ie sets pending bit.
 * This could also be implemented using ISPR regs.
 *
 * @param vector
 */
void Irq_GenerateSoftInt( IrqType vector ) {


}

/**
 * Get the current priority from the interrupt controller.
 * @param cpu
 * @return
 */
uint8_t Irq_GetCurrentPriority( Cpu_t cpu) {

	 //uint8_t prio = 0;

	// SCB_ICSR contains the active vector
	return 0;
}




