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

#define SYS_SWI_NR 21

#define IrqGetCurrentInterruptSource() \
	(((volatile sint8)vimREG->IRQIVEC) - 1)

/** IrqActivateChannel turns the selected channel on in the VIM */
#define IrqActivateChannel(_channel) \
	if (_channel > 31) { \
		vimREG->REQMASKSET1 |= (1 << (_channel - 32)); \
	} else { \
		vimREG->REQMASKSET0 |= (1 << _channel); \
	}


/** IrqDeactivateChannel turns the selected channel off in the VIM */
#define IrqDeactivateChannel(_channel) \
	if (_channel > 31) { \
		vimREG->REQMASKCLR1 = (1 << (_channel - 32)); \
	} else { \
		vimREG->REQMASKCLR0 = (1 << _channel); \
	}


#define Irq_SOI() \
	sint8 channel = IrqGetCurrentInterruptSource(); \
	IrqDeactivateChannel(channel)


#define Irq_EOI() \
	IrqActivateChannel(channel)



/* Total number of interrupts and exceptions
 */
#define NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS 65

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
