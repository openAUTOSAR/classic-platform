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

#endif /* IRQ_H_ */
