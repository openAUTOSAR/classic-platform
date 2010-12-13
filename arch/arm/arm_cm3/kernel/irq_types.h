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

#include "stm32f10x.h"

typedef IRQn_Type IrqType;

/* Offset from start of exceptions to interrupts
 * Exceptions have negative offsets while interrupts have positive
 */
#define IRQ_INTERRUPT_OFFSET  16

#define Irq_SOI()

/* Total number of interrupts and exceptions
 */

#if   defined(STM32F10X_LD) || defined(STM32F10X_MD)
#define NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS (USBWakeUp_IRQn+IRQ_INTERRUPT_OFFSET)
#elif defined(STM32F10X_HD)
#define NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS (DMA2_Channel4_5_IRQn+IRQ_INTERRUPT_OFFSET)
#elif defined(STM32F10X_CL)
#define NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS (OTG_FS_IRQn+IRQ_INTERRUPT_OFFSET)
#else
#error No device selected
#endif


typedef enum {
	  PERIPHERAL_CLOCK_AHB,
	  PERIPHERAL_CLOCK_APB1,
	  PERIPHERAL_CLOCK_APB2,
} McuE_PeriperalClock_t;


typedef enum {
	CPU_0=0,
} Cpu_t;

#endif /* IRQ_H_ */
