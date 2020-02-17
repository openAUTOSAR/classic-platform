/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/



#ifndef CPU_H
#define CPU_H

#include "Std_Types.h"
#if defined(__ghs__)
#include "v800_ghs.h"
#endif

typedef uint32_t imask_t;

#define STR__(x)	#x
#define STRINGIFY__(x) STR__(x)

#if defined(__ghs__)
#define Irq_Save(flags)		((flags) = (imask_t)__DIR() )
#define Irq_Restore(flags)	(void)(__RIR(flags))

#define Irq_Disable()		__DI()
#define Irq_Enable()		__EI()
#define synci()             asm volatile(" synci" ::: "memory")
#else
#define Irq_Save(flags)		flags = _Irq_Disable_save()
#define Irq_Restore(flags)	_Irq_Disable_restore(flags)

#define Irq_Disable()		asm volatile(" DI" ::: "memory","cc")
#define Irq_Enable()		asm volatile(" EI" ::: "memory","cc")
#define synci()             asm volatile(" synci" ::: "memory")
#endif

#define Irq_SuspendAll() 	Irq_Disable()
#define Irq_ResumeAll() 	Irq_Enable()

#define Irq_SuspendOs() 	Irq_Disable()
#define Irq_ResumeOs() 		Irq_Enable()

/**
 * Return number of leading zero's from right + 1
 *
 * Examples:
 *  sch1r(0x8000_0000) = 32
 *  sch1r(0x0000_0000) = 0
 *  sch1r(0x0000_0001) = 1
 *
 * @param val
 * @return
 */
static inline unsigned int sch1r(unsigned int val)
{
   unsigned int result;
   asm volatile ("  SCH1R  %[val],%[rv]" : [rv] "=r" (result) : [val] "r" (val));
   return result;
}

/**
 * Integer log2
 *
 * Examples:
 * - ilog2(0x0) = -1
 * - ilog2(0x1) = 0
 * - ilog2(0x2) = 1
 * - ilog2(0x8000_0000)=31
 *
 * @param val
 * @return
 */
static inline uint8 ilog2( uint32 val ) {
    return sch1r(val) - 1;
}


/*
 * unsigned int get_sr(unsigned int, unsigned int);
 *
 * STSR  regID, reg2, selID
 */

#if defined(__ghs__)
#define get_sr(spr_nr,sel_id)	__STSR(spr_nr,sel_id)
#else
#define get_sr(spr_nr,sel_id)	 \
({\
    uint32 __val;\
    asm volatile (" stsr " STRINGIFY__(spr_nr) ",%0," STR__(sel_id) : "=r"(__val) : );\
    __val;\
})
#endif

static inline unsigned long get_psw( void ) {
    uint32 psw;
    asm volatile(" stsr PSW,%[psw]":[psw] "=r" (psw ) );
    return psw;
}


static inline unsigned long _Irq_Disable_save(void)
{
   unsigned long result = get_psw();
   Irq_Disable();
   return result;
}


static inline void _Irq_Disable_restore(unsigned long flags) {
    /* restore PSW */
    asm volatile(" ldsr %0,PSW" : : "r" (flags) );
}


#endif /* CPU_H */
