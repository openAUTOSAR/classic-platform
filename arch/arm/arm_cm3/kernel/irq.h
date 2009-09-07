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








/*
 * irq.h
 *
 *  Created on: 4 aug 2009
 *      Author: mahi
 */

#ifndef IRQ_H_
#define IRQ_H_

#include "stm32f10x.h"

typedef IRQn_Type IrqType;

#define NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS (OTG_FS_IRQn+15)

typedef enum {
	CPU_0=0,
} Cpu_t;

#endif /* IRQ_H_ */
