/*
 * irq_types.h
 *
 *  Created on:  Mar 4, 2013
 *      Author:  Zhang Shuzhou
 *  Reviewed on:
 *     Reviewer:
 *
 */
#ifndef IRQ_TYPES_H
#define IRQ_TYPES_H

/* BCM2835 interrupts (see table on p.113 in BCM2835-ARM-Peripherals.pdf) */
typedef enum {
	BCM2835_IRQ_ID_SYSTEM_TIMER3 		= 3,
	BCM2835_IRQ_ID_USB 	         		= 9,
    BCM2835_IRQ_ID_AUX_UART      		= 29,
    BCM2835_IRQ_ID_GPIO_0 	     		= 49,
    BCM2835_IRQ_ID_GPIO_1 	     		= 50,
    BCM2835_IRQ_ID_GPIO_2 	     		= 51,
    BCM2835_IRQ_ID_GPIO_3 	     		= 52,
    BCM2835_IRQ_ID_I2C 	         		= 53,
    BCM2835_IRQ_ID_SPI 	         		= 54,
    BCM2835_IRQ_ID_TIMER_0       		= 64,
    NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS = 72,				// Total number of interrupts and exceptions
} IrqType;

/* Offset from start of exceptions to interrupts
 * Exceptions have negative offsets while interrupts have positive
 */
#define IRQ_INTERRUPT_OFFSET 0

typedef enum {
	 PERIPHERAL_CLOCK_AHB,
	 PERIPHERAL_CLOCK_APB1,
	 PERIPHERAL_CLOCK_APB2,
} McuE_PeriperalClock_t;

typedef enum {
	//what is this for?
	CPU_0=0,
} Cpu_t;

//add some function here for our projcet
typedef void (*FN_INTERRUPT_HANDLER)(int nIRQ, void *pParam);

typedef struct {
	FN_INTERRUPT_HANDLER 	pfnHandler;			///< Function that handles this IRQn
	void 				   *pParam;				///< A special parameter that the use can pass to the IRQ.
} INTERRUPT_VECTOR;

//int EnableInterrupts();
//int DisableInterrupts();

#endif /* IRQ_H_ */
