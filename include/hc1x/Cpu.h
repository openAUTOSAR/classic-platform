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

/*
 * ilog2 - return floor(log base 2 of x), where x > 0
 *   Example: ilog2(16) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 90
 *   Rating: 4
 */
// = n where the most significant non-zero bit is the nth bit
static inline uint32 ilog2(uint32 x) {

	// find out if the non-zero bit is in the first two words
	uint32 A = !(!(x >> 16));  // should be 1 if the n >= 16
	uint32 count = 0;
	uint32 x_copy = x;

	// if A is 1 add 16 to count
	count = count + (A<<4);

	// if A is 1 return x >> 15, else return x. This is a
	// modified version of my conditional code
	x_copy = (((~A + 1) & (x >> 16)) + (~(~A+1) & x));

	// repeat the process but rather than operating on the second half,
	// we can now only worry about the second quarter of the first half.
	A = !(!(x_copy >> 8));
	count = count + (A<<3);
	x_copy = (((~A + 1) & (x_copy >> 8)) + (~(~A+1) & x_copy));

	// continue this process until we have covered all bits.
	A = !(!(x_copy >> 4));
	count = count + (A<<2);
	x_copy = (((~A + 1) & (x_copy >> 4)) + (~(~A+1) & x_copy));

	A = !(!(x_copy >> 2));
	count = count + (A<<1);
	x_copy = (((~A + 1) & (x_copy >> 2)) + (~(~A+1) & x_copy));

	A = !(!(x_copy >> 1));
	count = count + A;

	return count;
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
