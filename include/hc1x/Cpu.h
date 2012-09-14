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

#include "Std_Types.h"
typedef uint32_t imask_t;

#define Irq_Disable()		asm volatile (" sei");
#define Irq_Enable()		asm volatile (" cli");

#define Irq_SuspendAll() 	Irq_Disable()
#define Irq_ResumeAll() 	Irq_Enable()

#define Irq_SuspendOs()		Irq_Disable()
#define Irq_ResumeOs()		Irq_Enable()

#define Irq_Save(flags)    flags = _Irq_Disable_save()
#define Irq_Restore(flags) _Irq_Disable_restore(flags)

/*-----------------------------------------------------------------*/

static inline unsigned long _Irq_Disable_save(void)
{
   unsigned long result;
   asm volatile ("tfr CCR, %0" : "=r" (result) : );
   Irq_Disable();
   return result;
}

/*-----------------------------------------------------------------*/

static inline void _Irq_Disable_restore(unsigned long flags)
{
   asm volatile ("tfr %0, CCR" : : "r" (flags) );
}

#define ilog2(x) __builtin_ffs(x)
#endif /* CPU_H_ */
