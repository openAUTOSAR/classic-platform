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

#if defined(__IAR_SYSTEMS_ICC__)
#include <../intrinsics.h>
#define Irq_Disable()		__disable_interrupt();
#define Irq_Enable()		__enable_interrupt();
#else
#define Irq_Disable()		asm volatile (" sei");
#define Irq_Enable()		asm volatile (" cli");
#endif

#define Irq_SuspendAll() 	Irq_Disable()
#define Irq_ResumeAll() 	Irq_Enable()

#define Irq_SuspendOs()		Irq_Disable()
#define Irq_ResumeOs()		Irq_Enable()

#define Irq_Save(flags)    flags = _Irq_Disable_save()
#define Irq_Restore(flags) _Irq_Disable_restore(flags)


#if defined(__IAR_SYSTEMS_ICC__)
static inline unsigned long _Irq_Disable_save(void)
{
	  __istate_t old;
	  old = __get_interrupt_state();
	  __disable_interrupt();
	  return old;
}

static inline void _Irq_Disable_restore(unsigned long flags) {
	__set_interrupt_state(flags);
}

#elif defined(__GNUC__)

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
#else
#error Compiler not defined

#endif


#endif /* CPU_H_ */
