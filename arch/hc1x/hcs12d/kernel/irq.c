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
extern void _start (void);

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

void bad_int()
{
  for (;;);
}


const struct interrupt_vectors __attribute__((section(".vectors"))) vectors =
    {
  pwm_shutdown_handler:
      bad_int,
  ptpif_handler:
      bad_int,
  can4_tx_handler:
      bad_int,
  can4_rx_handler:
      bad_int,
  can4_err_handler:
      bad_int,
  can4_wake_handler:
      bad_int,
  can3_tx_handler:
      bad_int,
  can3_rx_handler:
      bad_int,
  can3_err_handler:
      bad_int,
  can3_wake_handler:
      bad_int,
  can2_tx_handler:
      bad_int,
  can2_rx_handler:
      bad_int,
  can2_err_handler:
      bad_int,
  can2_wake_handler:
      bad_int,
  can1_tx_handler:
      bad_int,
  can1_rx_handler:
      bad_int,
  can1_err_handler:
      bad_int,
  can1_wake_handler:
      bad_int,
  can0_tx_handler:
      bad_int,
  can0_rx_handler:
      bad_int,
  can0_err_handler:
      bad_int,
  can0_wake_handler:
      bad_int,
  flash_handler:
      bad_int,
  eeprom_handler:
      bad_int,
  spi2_handler:
      bad_int,
  spi1_handler:
      bad_int,
  iic_handler:
      bad_int,
  bdlc_handler:
      bad_int,
  selfclk_mode_handler:
      bad_int,
  pll_lock_handler:
      bad_int,
  accb_overflow_handler:
      bad_int,
  mccnt_underflow_handler:
      bad_int,
  pthif_handler:
      bad_int,
  ptjif_handler:
      bad_int,
  atd1_handler:
      bad_int,
  atd0_handler:
      bad_int,
  sci1_handler:
      bad_int,
  sci0_handler:
      bad_int,
  spi0_handler:
      bad_int,

      // Timer and Accumulator
  acca_input_handler:
      bad_int,
  acca_overflow_handler:
      bad_int,
  timer_overflow_handler:
      bad_int,

      // InputCapture/OutputCompare Timers
  tc7_handler:
      bad_int,
  tc6_handler:
      bad_int,
  tc5_handler:
      bad_int,
  tc4_handler:
      bad_int,
  tc3_handler:
      bad_int,
  tc2_handler:
      bad_int,
  tc1_handler:
      bad_int,
  tc0_handler:
      bad_int,

      // External Interrupts
  rtii_handler:
      bad_int,
  irq_handler:
      bad_int,
  xirq_handler:
      bad_int,

  illegal_handler:
      bad_int,
  cop_fail_handler:
      bad_int,
  cop_clock_handler:
      bad_int,

      // Vectors in use
  swi_handler:
      bad_int,

  rtii_handler:
      bad_int,
  sci0_handler:
      bad_int,
  sci1_handler:
      bad_int,

  reset_handler:
      _start,
    };
