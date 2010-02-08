/*
 * cpu.h
 *
 *  Created on: 8 feb 2010
 *      Author: mahi
 */

#ifndef CPU_H_
#define CPU_H_

#define Irq_Disable()
#define Irq_Enable()

#define Irq_SuspendAll() 	Irq_Disable()
#define Irq_ResumeAll() 	Irq_Enable()

#define Irq_SuspendOs() 	Irq_Disable()
#define Irq_ResumeOs() 	Irq_Enable()



#endif /* CPU_H_ */
