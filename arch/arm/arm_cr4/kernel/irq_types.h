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


#define IrqGetCurrentInterruptSource() \
	(((volatile sint8)vimREG->IRQIVEC) - 1)

/** IrqActivateChannel turns the selected channel on in the VIM */
#define IrqActivateChannel(_c) \
	uint8 _ch1 = _c; \
	if (_ch1 > 31) { \
		vimREG->REQMASKSET1 |= (1 << (_ch1 - 32)); \
	} else { \
		vimREG->REQMASKSET0 |= (1 << _ch1); \
	}


/** IrqDeactivateChannel turns the selected channel off in the VIM */
#define IrqDeactivateChannel(_c) \
	uint8 _ch2 = _c; \
	if (_ch2 > 31) { \
		vimREG->REQMASKCLR1 = (1 << (_ch2 - 32)); \
	} else { \
		vimREG->REQMASKCLR0 = (1 << _ch2); \
	}


#define Irq_SOI() \
	IrqDeactivateChannel(isrPtr->activeVector)


#define Irq_EOI() \
	IrqActivateChannel(isrPtr->activeVector)


typedef enum {
  ESM_LEVEL_0 = 0,
  RESERVED = 1,
  RTI_COMPARE_0,
  RTI_COMPARE_1,
  RTI_COMPARE_2,
  RTI_COMPARE_3,
  RTI_OVERFLOW_0,
  RTI_OVERFLOW_1,
  RTI_TIMEBASE,
  DIO_LEVEL_0,
  HET_LEVEL_0,
  HET_TU_LEVEL_0,
  MIBSP1_LEVEL_0,
  LIN1_LEVEL_0,
  MIBADC1_EVENT,
  MIBADC1_GROUP_1,
  CAN1_LEVEL_0,
  RESERVED_0,
  FR_LEVEL_0,
  CRC_1,
  ESM_LEVEL_1,
  SSI,
  PMU,
  DIO_LEVEL_1,
  HET_LEVEL_1,
  HET_TU_LEVEL_1,
  MIBSP1_LEVEL_1,
  LIN1_LEVEL_1,
  MIBADC1_GROUP_2,
  CAN1_LEVEL_1,
  RESERVED_1,
  MIBADC1_MAG,
  FR_LEVEL_1,
  DMA_FTCA,
  DMA_LFSA,
  CAN2_LEVEL_0,
  DMM_LEVEL_0,
  MIBSPI3_LEVEL_0,
  MIBSPI3_LEVEL_1,
  DMA_HBDC,
  DMA_BTCA,
  RESERVED_2,
  CAN2_LEVEL_1,
  DMM_LEVEL_1,
  CAN1_IF_3,
  CAN3_LEVEL_0,
  CAN2_IF_3,
  FPU,
  FR_TU_STATUS,
  LIN2_LEVEL_0,
  MIBADC2_EVENT,
  MIBADC2_GROUP_1,
  FR_TOC,
  MIBSPIP5_LEVEL_0,
  LIN2_LEVEL_1,
  CAN3_LEVEL_1,
  MIBSPI5_LEVEL_1,
  MIBADC2_GROUP_2,
  FR_TU_ERROR,
  MIBADC2_MAG,
  CAN3_IF_3,
  FR_TU_MPU,
  FR_T1C,
  NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS,
} IrqType;

/* Total number of interrupts and exceptions
 */
#define NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS 65

/* Offset from start of exceptions to interrupts
 * Exceptions have negative offsets while interrupts have positive
 */
#define IRQ_INTERRUPT_OFFSET 0

typedef enum {
	  PERIPHERAL_CLOCK_AHB,
	  PERIPHERAL_CLOCK_APB1,
	  PERIPHERAL_CLOCK_APB2,
	  PERIPHERAL_CLOCK_CAN,
	  PERIPHERAL_CLOCK_DCAN1,
	  PERIPHERAL_CLOCK_DCAN2,
	  PERIPHERAL_CLOCK_DCAN3
} McuE_PeriperalClock_t;

typedef enum {
	CPU_0=0,
} Cpu_t;

#endif /* IRQ_H_ */
