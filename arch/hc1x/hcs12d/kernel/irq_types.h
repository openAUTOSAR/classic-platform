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


#ifndef IRQ_TYPES_H
#define IRQ_TYPES_H

#define Irq_SOI()
#define Irq_EOI()

#include "irq_defines.h"

#define DECLARE_IRQ_PROTOTYPE(x)			\
void irq_##x (void);


DECLARE_IRQ_PROTOTYPE(res0)
DECLARE_IRQ_PROTOTYPE(res1)
DECLARE_IRQ_PROTOTYPE(res2)
DECLARE_IRQ_PROTOTYPE(res3)
DECLARE_IRQ_PROTOTYPE(res4)
DECLARE_IRQ_PROTOTYPE(res5)
DECLARE_IRQ_PROTOTYPE(pwm_shutdown)
DECLARE_IRQ_PROTOTYPE(ptpif)
DECLARE_IRQ_PROTOTYPE(can4_tx)
DECLARE_IRQ_PROTOTYPE(can4_rx)
DECLARE_IRQ_PROTOTYPE(can4_err)
DECLARE_IRQ_PROTOTYPE(can4_wake)
DECLARE_IRQ_PROTOTYPE(can3_tx)
DECLARE_IRQ_PROTOTYPE(can3_rx)
DECLARE_IRQ_PROTOTYPE(can3_err)
DECLARE_IRQ_PROTOTYPE(can3_wake)
DECLARE_IRQ_PROTOTYPE(can2_tx)
DECLARE_IRQ_PROTOTYPE(can2_rx)
DECLARE_IRQ_PROTOTYPE(can2_err)
DECLARE_IRQ_PROTOTYPE(can2_wake)
DECLARE_IRQ_PROTOTYPE(can1_tx)
DECLARE_IRQ_PROTOTYPE(can1_rx)
DECLARE_IRQ_PROTOTYPE(can1_err)
DECLARE_IRQ_PROTOTYPE(can1_wake)
DECLARE_IRQ_PROTOTYPE(can0_tx)
DECLARE_IRQ_PROTOTYPE(can0_rx)
DECLARE_IRQ_PROTOTYPE(can0_err)
DECLARE_IRQ_PROTOTYPE(can0_wake)
DECLARE_IRQ_PROTOTYPE(flash)
DECLARE_IRQ_PROTOTYPE(eeprom)
DECLARE_IRQ_PROTOTYPE(spi2)
DECLARE_IRQ_PROTOTYPE(spi1)
DECLARE_IRQ_PROTOTYPE(iic)
DECLARE_IRQ_PROTOTYPE(bdlc)
DECLARE_IRQ_PROTOTYPE(selfclk_mode)
DECLARE_IRQ_PROTOTYPE(pll_lock)
DECLARE_IRQ_PROTOTYPE(accb_overflow)
DECLARE_IRQ_PROTOTYPE(mccnt_underflow)
DECLARE_IRQ_PROTOTYPE(pthif)
DECLARE_IRQ_PROTOTYPE(ptjif)
DECLARE_IRQ_PROTOTYPE(atd1)
DECLARE_IRQ_PROTOTYPE(atd0)
DECLARE_IRQ_PROTOTYPE(sci1)
DECLARE_IRQ_PROTOTYPE(sci0)
DECLARE_IRQ_PROTOTYPE(spi0)
DECLARE_IRQ_PROTOTYPE(acca_input)
DECLARE_IRQ_PROTOTYPE(acca_overflow)
DECLARE_IRQ_PROTOTYPE(timer_overflow)
DECLARE_IRQ_PROTOTYPE(tc7)
DECLARE_IRQ_PROTOTYPE(tc6)
DECLARE_IRQ_PROTOTYPE(tc5)
DECLARE_IRQ_PROTOTYPE(tc4)
DECLARE_IRQ_PROTOTYPE(tc3)
DECLARE_IRQ_PROTOTYPE(tc2)
DECLARE_IRQ_PROTOTYPE(tc1)
DECLARE_IRQ_PROTOTYPE(tc0)
DECLARE_IRQ_PROTOTYPE(rtii)
DECLARE_IRQ_PROTOTYPE(irq)
DECLARE_IRQ_PROTOTYPE(xirq)
DECLARE_IRQ_PROTOTYPE(swi)
DECLARE_IRQ_PROTOTYPE(illegal)
DECLARE_IRQ_PROTOTYPE(cop_fail)
DECLARE_IRQ_PROTOTYPE(cop_clock)

typedef enum
{
	IRQ_TYPE_RES0,
	IRQ_TYPE_RES1,
	IRQ_TYPE_RES2,
	IRQ_TYPE_RES3,
	IRQ_TYPE_RES4,
	IRQ_TYPE_RES5,
	IRQ_TYPE_PWM_SHUTDOWN,
	IRQ_TYPE_PTPIF,

	/* CAN */
	IRQ_TYPE_CAN4_TX,
	IRQ_TYPE_CAN4_RX,
	IRQ_TYPE_CAN4_ERR,
	IRQ_TYPE_CAN4_WAKE,
	IRQ_TYPE_CAN3_TX,
	IRQ_TYPE_CAN3_RX,
	IRQ_TYPE_CAN3_ERR,
	IRQ_TYPE_CAN3_WAKE,
	IRQ_TYPE_CAN2_TX,
	IRQ_TYPE_CAN2_RX,
	IRQ_TYPE_CAN2_ERR,
	IRQ_TYPE_CAN2_WAKE,
	IRQ_TYPE_CAN1_TX,
	IRQ_TYPE_CAN1_RX,
	IRQ_TYPE_CAN1_ERR,
	IRQ_TYPE_CAN1_WAKE,
	IRQ_TYPE_CAN0_TX,
	IRQ_TYPE_CAN0_RX,
	IRQ_TYPE_CAN0_ERR,
	IRQ_TYPE_CAN0_WAKE,

	IRQ_TYPE_FLASH,
	IRQ_TYPE_EEPROM,
	IRQ_TYPE_SPI2,
	IRQ_TYPE_SPI1,
	IRQ_TYPE_IIC,
	IRQ_TYPE_BDLC,
	IRQ_TYPE_SELFCLK_MODE,
	IRQ_TYPE_PLL_LOCK,
	IRQ_TYPE_ACCB_OVERFLOW,
	IRQ_TYPE_MCCNT_UNDERFLOW,
	IRQ_TYPE_PTHIF,
	IRQ_TYPE_PTJIF,
	IRQ_TYPE_ATD1,
	IRQ_TYPE_ATD0,
	IRQ_TYPE_SCI1,
	IRQ_TYPE_SCI0,
	IRQ_TYPE_SPI0,

	/* TIMER AND ACCUMULATOR */
	IRQ_TYPE_ACCA_INPUT,
	IRQ_TYPE_ACCA_OVERFLOW,
	IRQ_TYPE_TIMER_OVERFLOW,

	/* INPUTCAPTURE/OUTPUTCOMPARE TIMERS */
	IRQ_TYPE_TC7,
	IRQ_TYPE_TC6,
	IRQ_TYPE_TC5,
	IRQ_TYPE_TC4,
	IRQ_TYPE_TC3,
	IRQ_TYPE_TC2,
	IRQ_TYPE_TC1,
	IRQ_TYPE_TC0,

	/* EXTERNAL INTERRUPTS */
	IRQ_TYPE_RTII,
	IRQ_TYPE_IRQ,
	IRQ_TYPE_XIRQ,

	/* SOFTWARE INTERRUPT  */
	IRQ_TYPE_SWI,

	/* ILLEGAL INSTRUCTION RESET  */
	IRQ_TYPE_ILLEGAL,

	/* COP TIMEOUT RESET  */
	IRQ_TYPE_COP_FAIL,

	/* CLOCK MONITOR FAIL RESET  */
	IRQ_TYPE_COP_CLOCK,

	/* START OR RESET VECTOR  */
	IRQ_TYPE_RESET,

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
