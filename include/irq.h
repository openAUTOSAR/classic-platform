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


#ifndef IRQ_H_
#define IRQ_H_

#include <stdint.h>
#if defined(USE_KERNEL) || defined(USE_LINUXOS)
#include "Os.h"
#else
typedef uint32 CoreIDType; /* @req SWS_Os_00790 Type definition */
#endif
#include "irq_types.h"
#include "bit.h"

typedef void ( *IrqFuncType)(void);

#if (OS_SC2==STD_ON) || (OS_SC4==STD_ON)
#define HAVE_SC2_SC4(_value) _value
#else
#define HAVE_SC2_SC4(_value)
#endif

#define IRQ_NAME(_vector)		IrqVector_ ## _vector

/**
 * Init the interrupt controller
 */
void Irq_Init( void );


#if defined(CFG_HC1X)
/**
 *
 * @param stack Ptr to the current stack.
 * @param irq_nr The nr. of the interrupt being handled.
 *
 * The stack holds C, NVGPR, VGPR and the EXC frame.
 *
 */
void *Irq_Entry( uint8_t irq_nr, void *stack );

#else
/**
 *
 * @param stack_p Ptr to the current stack.
 *
 * The stack holds C, NVGPR, VGPR and the EXC frame.
 *
 */
void *Irq_Entry( void *stack_p );
#endif

struct OsIsrConst;


/**
 * Generates a soft interrupt
 * Used by the test-system
 *
 * @param vector
 */
void Irq_GenerateSoftInt( IrqType vector );

/**
 * Ack a software interrupt.
 * Used by the test-system
 *
 * @param vector
 */
void Irq_AckSoftInt( IrqType vector );

/**
 * Get the current priority from the interrupt controller.
 * @param cpu
 * @return
 */
uint8_t Irq_GetCurrentPriority( Cpu_t cpu);

/**
 * Set the priority in the interrupt controller for vector
 */
void Irq_SetPriority( CoreIDType cpu,  IrqType vector, uint8 prio );

/**
 * Enable a vector in the interrupt controller
 *
 * @param vector
 * @param priority
 * @param core
 */
void Irq_EnableVector( sint16 vector, uint8 priority, sint32 core );

/**
 *
 * @param func
 * @param vector
 * @param type
 * @param priority
 * @param core
 */
void Irq_EnableVector2( IrqFuncType func,  sint16 vector, uint16 type,  uint8 priority, sint32 core );

/**
 * Disable a vector so that Irq_EnableVector can be used again.
 * (used by the test-system for some archs)
 *
 * @param vector
 * @param core
 */
void Irq_DisableVector( sint16 vector, sint32 core );

#ifndef Irq_EOI
void Irq_EOI( sint16 vector );
#endif

#if defined (CFG_TMS570)
void Irq_SOI3(uint8 prio);
void setupPinsForIrq( void );
#endif

/**
 * Function to return the ISR id based on priority, if an ISR is already installed then
 * Irq_VectorTable[priority] will have its ISR id otherwise VECTOR_ILL.
 *
 * @param priority
 */
#if defined(CFG_TC2XX) || defined(CFG_TC3XX)
uint16 Irq_GetISRinstalledId(uint8 priority );
#endif

#endif /* IRQ_H_ */
