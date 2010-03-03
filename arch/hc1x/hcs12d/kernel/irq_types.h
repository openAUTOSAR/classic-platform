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


#ifndef IRQ_H
#define IRQ_H


typedef enum
{
	RESET,
	CLOCK_MONITOR_FAIL_RESET,
	/* TODO: */

	NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS,
}IrqType;

typedef enum {
	CPU_CORE0,
} Cpu_t;

#define IRQ_INTERRUPT_OFFSET		0

/* Interrupt handler prototype.  */
typedef void (* interrupt_t) (void);

/* Interrupt vectors as a struct.*/
typedef struct interrupt_vectors
{
	interrupt_t res0_handler;        /* 0x80 */
	interrupt_t res1_handler;        /* 0x82 */
	interrupt_t res2_handler;        /* 0x84 */
	interrupt_t res3_handler;        /* 0x86 */
	interrupt_t res4_handler;        /* 0x88 */
	interrupt_t res5_handler;        /* 0x8a */
	interrupt_t pwm_shutdown_handler; /* 0x8c */
	interrupt_t ptpif_handler;

	/* ControllerAreaNetworking */
	interrupt_t can4_tx_handler;
	interrupt_t can4_rx_handler;
	interrupt_t can4_err_handler;
	interrupt_t can4_wake_handler;
	interrupt_t can3_tx_handler;
	interrupt_t can3_rx_handler;
	interrupt_t can3_err_handler;
	interrupt_t can3_wake_handler;
	interrupt_t can2_tx_handler;
	interrupt_t can2_rx_handler;
	interrupt_t can2_err_handler;
	interrupt_t can2_wake_handler;
	interrupt_t can1_tx_handler;
	interrupt_t can1_rx_handler;
	interrupt_t can1_err_handler;
	interrupt_t can1_wake_handler;
	interrupt_t can0_tx_handler;
	interrupt_t can0_rx_handler;
	interrupt_t can0_err_handler;
	interrupt_t can0_wake_handler;

	interrupt_t flash_handler;
	interrupt_t eeprom_handler;
	interrupt_t spi2_handler;
	interrupt_t spi1_handler;
	interrupt_t iic_handler;
	interrupt_t bdlc_handler;
	interrupt_t selfclk_mode_handler;
	interrupt_t pll_lock_handler;
	interrupt_t accb_overflow_handler;
	interrupt_t mccnt_underflow_handler;
	interrupt_t pthif_handler;
	interrupt_t ptjif_handler;
	interrupt_t atd1_handler;
	interrupt_t atd0_handler;
	interrupt_t sci1_handler;
	interrupt_t sci0_handler;
	interrupt_t spi0_handler;

	/* Timer and Accumulator */
	interrupt_t acca_input_handler;
	interrupt_t acca_overflow_handler;
	interrupt_t timer_overflow_handler;

	/* InputCapture/OutputCompare Timers */
	interrupt_t tc7_handler;
	interrupt_t tc6_handler;
	interrupt_t tc5_handler;
	interrupt_t tc4_handler;
	interrupt_t tc3_handler;
	interrupt_t tc2_handler;
	interrupt_t tc1_handler;
	interrupt_t tc0_handler;

	/* External Interrupts */
	interrupt_t rtii_handler;
	interrupt_t irq_handler;
	interrupt_t xirq_handler;

	/* Software Interrupt  */
	interrupt_t swi_handler;

	/* Illegal Instruction Reset  */
	interrupt_t illegal_handler;

	/* COP Timeout Reset  */
	interrupt_t cop_fail_handler;

	/* Clock Monitor Fail Reset  */
	interrupt_t cop_clock_handler;

	/* Start or Reset vector  */
	interrupt_t reset_handler;

} interrupt_vectors_t;

#endif /* IRQ_H_ */
