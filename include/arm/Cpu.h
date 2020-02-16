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

#ifndef CPU_H_
#define CPU_H_

#include <stdint.h>

#if defined(CFG_ARM_CM3)
#include "stm32f10x.h"
#elif defined(CFG_ARM_CR4)
#include "core_cr4.h"
#elif defined(CFG_ARM_V6)
#include "bcm2835.h"
#endif

#include "Std_Types.h"
typedef uint32_t imask_t;

/* Call architecture specific code */
#define Irq_Disable()		__disable_irq()
#define Irq_Enable()		__enable_irq()

#define Irq_Save(_flags)     		_flags = _Irq_Save();
#define Irq_Restore(_flags)			_Irq_Restore(_flags);

#define Irq_SuspendAll() 	Irq_Disable()
#define Irq_ResumeAll() 	Irq_Enable()

#define Irq_SuspendOs() 	Irq_Disable()
#define Irq_ResumeOs() 		Irq_Enable()


#define CallService(index,param)

#define ilog2(x) (__builtin_ffs(x)-1)

#endif /* CPU_H_ */
