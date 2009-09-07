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

#ifndef CPU_H_
#define CPU_H_

#include <stdint.h>
#include "stm32f10x.h"
#include "core_cm3.h"

#define SIMULATOR() (0==0)


//#define isync()  		asm volatile(" isync");
//#define sync()   		asm volatile(" sync");
//#define msync() 		asm volatile(" msync");

/* Call intrinsic functions directly */
#define Irq_Disable()					__disable_irq()
#define Irq_Enable()					__enable_irq()

/* TODO: This is of course wrong */
#define Irq_Save(_flags)     			__disable_irq();
#define Irq_Restore(_flags)				__enable_irq();

#endif /* CPU_H_ */
