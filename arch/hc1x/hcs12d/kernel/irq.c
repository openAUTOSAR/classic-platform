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

extern void * Irq_VectorTable[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS];
extern uint8 Irq_IsrTypeTable[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS];

extern void _start (void);


void Irq_Init( void ) {

}

void Irq_EOI( void ) {

}

void *Irq_Entry( uint8 irq_nr )
{
	if( Irq_GetIsrType(vector) == ISR_TYPE_1 ) {
		// It's a function, just call it.
		((func_t)Irq_VectorTable[vector])();
		return stack;
	} else {
		// It's a PCB
		// Let the kernel handle the rest,
		return Os_Isr(stack, (void *)Irq_VectorTable[vector]);
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
void Irq_AttachIsr1( void (*entry)(void), void *int_ctrl, uint32_t vector, uint8_t prio) {
	Irq_VectorTable[vector] = (void *)entry;
	Irq_SetIsrType(vector, ISR_TYPE_1);
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
	Irq_IsrTypeTable[vector] = PROC_ISR2;

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


// #####################  INTERRUPT TRANSLATE TABLE #######################
#define IRQ_MAP(x) irq_##x

const struct interrupt_vectors __attribute__((section(".vectors"))) vectors =
    {
	  pwm_shutdown_handler:
		  IRQ_MAP(IRQ_NR_PWM_SHUTDOWN),
	  ptpif_handler:
		  IRQ_MAP(IRQ_NR_PTPIF),
	  can4_tx_handler:
		  IRQ_MAP(IRQ_NR_CAN4_TX),
	  can4_rx_handler:
		  IRQ_MAP(IRQ_NR_CAN4_RX),
	  can4_err_handler:
		  IRQ_MAP(IRQ_NR_CAN4_ERR),
	  can4_wake_handler:
		  IRQ_MAP(IRQ_NR_CAN4_WAKE),
	  can3_tx_handler:
		  IRQ_MAP(IRQ_NR_CAN3_TX),
	  can3_rx_handler:
		  IRQ_MAP(IRQ_NR_CAN3_RX),
	  can3_err_handler:
		  IRQ_MAP(IRQ_NR_CAN3_ERR),
	  can3_wake_handler:
		  IRQ_MAP(IRQ_NR_CAN3_WAKE),
	  can2_tx_handler:
		  IRQ_MAP(IRQ_NR_CAN2_TX),
	  can2_rx_handler:
		  IRQ_MAP(IRQ_NR_CAN2_RX),
	  can2_err_handler:
		  IRQ_MAP(IRQ_NR_CAN2_ERR),
	  can2_wake_handler:
		  IRQ_MAP(IRQ_NR_CAN2_WAKE),
	  can1_tx_handler:
		  IRQ_MAP(IRQ_NR_CAN1_TX),
	  can1_rx_handler:
		  IRQ_MAP(IRQ_NR_CAN1_RX),
	  can1_err_handler:
		  IRQ_MAP(IRQ_NR_CAN1_ERR),
	  can1_wake_handler:
		  IRQ_MAP(IRQ_NR_CAN1_WAKE),
	  can0_tx_handler:
		  IRQ_MAP(IRQ_NR_CAN0_TX),
	  can0_rx_handler:
		  IRQ_MAP(IRQ_NR_CAN0_RX),
	  can0_err_handler:
		  IRQ_MAP(IRQ_NR_CAN0_ERR),
	  can0_wake_handler:
		  IRQ_MAP(IRQ_NR_CAN0_WAKE),
	  flash_handler:
		  IRQ_MAP(IRQ_NR_FLASH),
	  eeprom_handler:
		  IRQ_MAP(IRQ_NR_EEPROM),
	  spi2_handler:
		  IRQ_MAP(IRQ_NR_SPI2),
	  spi1_handler:
		  IRQ_MAP(IRQ_NR_SPI1),
	  iic_handler:
		  IRQ_MAP(IRQ_NR_IIC),
	  bdlc_handler:
		  IRQ_MAP(IRQ_NR_BDLC),
	  selfclk_mode_handler:
		  IRQ_MAP(IRQ_NR_SELFCLK_MODE),
	  pll_lock_handler:
		  IRQ_MAP(IRQ_NR_PLL_LOCK),
	  accb_overflow_handler:
		  IRQ_MAP(IRQ_NR_ACCB_OVERFLOW),
	  mccnt_underflow_handler:
		  IRQ_MAP(IRQ_NR_MCCNT_UNDERFLOW),
	  pthif_handler:
		  IRQ_MAP(IRQ_NR_PTHIF),
	  ptjif_handler:
		  IRQ_MAP(IRQ_NR_PTJIF),
	  atd1_handler:
		  IRQ_MAP(IRQ_NR_ATD1),
	  atd0_handler:
		  IRQ_MAP(IRQ_NR_ATD0),
	  sci1_handler:
		  IRQ_MAP(IRQ_NR_SCI1),
	  sci0_handler:
		  IRQ_MAP(IRQ_NR_SCI0),
	  spi0_handler:
		  IRQ_MAP(IRQ_NR_SPI0),

		  // Timer and Accumulator
	  acca_input_handler:
		  IRQ_MAP(IRQ_NR_ACCA_INPUT),
	  acca_overflow_handler:
		  IRQ_MAP(IRQ_NR_ACCA_OVERFLOW),
	  timer_overflow_handler:
		  IRQ_MAP(IRQ_NR_TIMER_OVERFLOW),

		  // InputCapture/OutputCompare Timers
	  tc7_handler:
		  IRQ_MAP(IRQ_NR_TC7),
	  tc6_handler:
		  IRQ_MAP(IRQ_NR_TC6),
	  tc5_handler:
		  IRQ_MAP(IRQ_NR_TC5),
	  tc4_handler:
		  IRQ_MAP(IRQ_NR_TC4),
	  tc3_handler:
		  IRQ_MAP(IRQ_NR_TC3),
	  tc2_handler:
		  IRQ_MAP(IRQ_NR_TC2),
	  tc1_handler:
		  IRQ_MAP(IRQ_NR_TC1),
	  tc0_handler:
		  IRQ_MAP(IRQ_NR_TC0),

		  // External Interrupts
	  rtii_handler:
		  IRQ_MAP(IRQ_NR_RTII),
	  irq_handler:
		  IRQ_MAP(IRQ_NR_IRQ),
	  xirq_handler:
		  IRQ_MAP(IRQ_NR_XIRQ),

	  // Vectors in use
	  swi_handler:
		  IRQ_MAP(IRQ_NR_SWI),

	  illegal_handler:
		  IRQ_MAP(IRQ_NR_ILLEGAL),
	  cop_fail_handler:
		  IRQ_MAP(IRQ_NR_COP_FAIL),
	  cop_clock_handler:
		  IRQ_MAP(IRQ_NR_COP_CLOCK),

  reset_handler:
      _start,
    };
