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
#include "isr.h"
#include "irq_types.h"
#include "regs.h"

extern void * Irq_VectorTable[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS];
extern uint8 Irq_IsrTypeTable[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS];

extern void _start (void);


void Irq_Init( void ) {

}


// IRQ debug information
// Stores irq nr on erroneous interrupt
volatile sint16 bad_irq_nr = -1;

// Stores context info on erroneous interrupt
volatile uint8 bad_irq_context_bank = 0;
volatile void* bad_irq_context_address = 0;

void bad_irq(uint8_t irq_nr, void **stack) {

	// Save number of caught interrupt
	bad_irq_nr = irq_nr;

	// Fetch address and page of where we were interrupted, from context
	uint16 bank_and_ccr = (uint16)(*(stack + 4));
	bad_irq_context_bank = (bank_and_ccr & 0xFF00) >> 8;
	bad_irq_context_address = *(stack + 8);

	for (;;)
	  asm("BGND"); // Jump to debugger
}

void *Irq_Entry( uint8_t irq_nr, void *stack )
{
	//** Move to asm **/
	return Os_Isr(stack, irq_nr);
}

/**
 * Generates a soft interrupt, ie sets pending bit.
 * This could also be implemented using ISPR regs.
 *
 * @param vector
 */
void Irq_GenerateSoftInt( IrqType vector ) {
	if (vector == IRQ_TYPE_SWI)
	{
	    asm("swi");
	}

	if (vector == IRQ_TYPE_ILLEGAL)
	{
#if defined(__ICCHCS12__)
            asm("TRAP 0x30"); // Trap instruction
#else
            asm(".short 0x1830"); // Trap instruction
#endif
	}
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

void Irq_EnableVector( int16_t vector, int priority, int core ) {
}


// #####################  INTERRUPT TRANSLATE TABLE #######################
#define IRQ_MAP(x) irq_##x

#if defined(__ICCHCS12__)
/* MOVED TO crt0_iar.s instead */
#else
const struct interrupt_vectors __attribute__((section(".vectors"))) vectors =
    {
	  pwm_shutdown_handler:
		  IRQ_MAP(pwm_shutdown),
	  ptpif_handler:
		  IRQ_MAP(ptpif),
	  can4_tx_handler:
		  IRQ_MAP(can4_tx),
	  can4_rx_handler:
		  IRQ_MAP(can4_rx),
	  can4_err_handler:
		  IRQ_MAP(can4_err),
	  can4_wake_handler:
		  IRQ_MAP(can4_wake),
	  can3_tx_handler:
		  IRQ_MAP(can3_tx),
	  can3_rx_handler:
		  IRQ_MAP(can3_rx),
	  can3_err_handler:
		  IRQ_MAP(can3_err),
	  can3_wake_handler:
		  IRQ_MAP(can3_wake),
	  can2_tx_handler:
		  IRQ_MAP(can2_tx),
	  can2_rx_handler:
		  IRQ_MAP(can2_rx),
	  can2_err_handler:
		  IRQ_MAP(can2_err),
	  can2_wake_handler:
		  IRQ_MAP(can2_wake),
	  can1_tx_handler:
		  IRQ_MAP(can1_tx),
	  can1_rx_handler:
		  IRQ_MAP(can1_rx),
	  can1_err_handler:
		  IRQ_MAP(can1_err),
	  can1_wake_handler:
		  IRQ_MAP(can1_wake),
	  can0_tx_handler:
		  IRQ_MAP(can0_tx),
	  can0_rx_handler:
		  IRQ_MAP(can0_rx),
	  can0_err_handler:
		  IRQ_MAP(can0_err),
	  can0_wake_handler:
		  IRQ_MAP(can0_wake),
	  flash_handler:
		  IRQ_MAP(flash),
	  eeprom_handler:
		  IRQ_MAP(eeprom),
	  spi2_handler:
		  IRQ_MAP(spi2),
	  spi1_handler:
		  IRQ_MAP(spi1),
	  iic_handler:
		  IRQ_MAP(iic),
	  bdlc_handler:
		  IRQ_MAP(bdlc),
	  selfclk_mode_handler:
		  IRQ_MAP(selfclk_mode),
	  pll_lock_handler:
		  IRQ_MAP(pll_lock),
	  accb_overflow_handler:
		  IRQ_MAP(accb_overflow),
	  mccnt_underflow_handler:
		  IRQ_MAP(mccnt_underflow),
	  pthif_handler:
		  IRQ_MAP(pthif),
	  ptjif_handler:
		  IRQ_MAP(ptjif),
	  atd1_handler:
		  IRQ_MAP(atd1),
	  atd0_handler:
		  IRQ_MAP(atd0),
	  sci1_handler:
		  IRQ_MAP(sci1),
	  sci0_handler:
		  IRQ_MAP(sci0),
	  spi0_handler:
		  IRQ_MAP(spi0),

		  // Timer and Accumulator
	  acca_input_handler:
		  IRQ_MAP(acca_input),
	  acca_overflow_handler:
		  IRQ_MAP(acca_overflow),
	  timer_overflow_handler:
		  IRQ_MAP(timer_overflow),

		  // InputCapture/OutputCompare Timers
	  tc7_handler:
		  IRQ_MAP(tc7),
	  tc6_handler:
		  IRQ_MAP(tc6),
	  tc5_handler:
		  IRQ_MAP(tc5),
	  tc4_handler:
		  IRQ_MAP(tc4),
	  tc3_handler:
		  IRQ_MAP(tc3),
	  tc2_handler:
		  IRQ_MAP(tc2),
	  tc1_handler:
		  IRQ_MAP(tc1),
	  tc0_handler:
		  IRQ_MAP(tc0),

		  // External Interrupts
	  rtii_handler:
		  IRQ_MAP(rtii),
	  irq_handler:
		  IRQ_MAP(irq),
	  xirq_handler:
		  IRQ_MAP(xirq),

	  // Vectors in use
	  swi_handler:
		  IRQ_MAP(swi),

	  illegal_handler:
		  IRQ_MAP(illegal),
	  cop_fail_handler:
		  _start,
	  cop_clock_handler:
		  IRQ_MAP(cop_clock),

  reset_handler:
      _start,
    };
#endif
