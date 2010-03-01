/*
 * cpu.h
 *
 *  Created on: 8 feb 2010
 *      Author: mahi
 */

#ifndef CPU_H_
#define CPU_H_

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

#endif /* CPU_H_ */
