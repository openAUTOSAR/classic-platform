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


#ifndef IRQ_H_
#define IRQ_H_

#include <stdint.h>
#include "Os.h"
#include "irq_types.h"
#include "bit.h"

typedef void ( * func_t)(void);



#if (OS_SC2==STD_ON) || (OS_SC4==STD_ON)
#define HAVE_SC2_SC4(_value) _value
#else
#define HAVE_SC2_SC4(_value)
#endif

#define IRQ_NAME(_vector)		IrqVector_ ## _vector

#define IRQ_DECL_ISR2_TIMING_PROT(	\
				_name, \
				_max_all_interrupt_lock_time, \
				_exeution_budget, \
				_os_interrupt_lock_budget, \
				_time_frame, \
				_resource_lock_list )


#define IRQ_DECL_ISR1(_name, _vector, _core, _prio, _entry  ) \
		const OsIsrConstType Irq_VectorConst_ ## _vector = { 	\
		.name = _name,						\
		.vector = (_vector),				\
		.core = (_core),					\
		.prio = (_prio),					\
		.entry = (_entry),					\
		.type = ISR_TYPE_1,					\
	}


#define IRQ_DECL_ISR2(_name,_vector, _core, _prio, _entry, _resource_mask, _timing_prot) \
		const OsIsrConstType Irq_VectorConst_ ## _vector = { 	\
		.name = _name,						\
		.vector = (_vector),				\
		.core = (_core),					\
		.prio = (_prio),					\
		.entry = (_entry),					\
		.type = ISR_TYPE_2,					\
		.resourceMask = (_resource_mask),	\
		.timingProtPtr = (_timing_prot)     \
	}


#define IRQ_ATTACH(_vector)	Irq_Attach(&Irq_VectorConst_ ## _vector)

/* Example:
 * IRQ_DECL_ISR2_RESOURCE(res2, RES_ID_2, 500000U );   			// Max 50us
 * IRQ_DECL_ISR2_TIMING_PROT(timing,0,0,0,0,res2);
 * IRQ_DECL_ISR2("MyIsr",10,10,MyIsr,HAVE_SC2_SC4(RES_ID_1),HAVE_SC2_SC4(timing));
 *
 */

// typedef _Bool IsrType;

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

//ISRType Irq_Attach( const OsIsrConstType *isrPtr );
ISRType Irq_Attach( int vector );
ISRType Irq_Attach2( const OsIsrConstType * );

/**
 * Attach an ISR type 1 to the interrupt controller.
 *
 * @param entry
 * @param int_ctrl
 * @param vector
 * @param prio
 */
void Irq_AttachIsr1( void (*entry)(void), void *int_ctrl, uint32_t vector, uint8_t prio);

/**
 * Attach a ISR type 2 to the interrupt controller.
 *
 * @param tid       The task id
 * @param int_ctrl  The interrupt controller, The is NULL for now.
 * @param vector 	The vector to attach to
 */
void Irq_AttachIsr2(TaskType tid,void *int_ctrl,IrqType vector );

/**
 * Generates a soft interrupt
 * @param vector
 */
void Irq_GenerateSoftInt( IrqType vector );
/**
 * Get the current priority from the interrupt controller.
 * @param cpu
 * @return
 */
uint8_t Irq_GetCurrentPriority( Cpu_t cpu);

/**
 * Set the priority in the interrupt controller for vector
 */
void Irq_SetPriority( Cpu_t cpu,  IrqType vector, uint8_t prio );

void Irq_EnableVector( int16_t vector, int priority, int core );

/**
 *
 * @param vector
 * @param type
 */
#if 0
static inline void Irq_SetIsrType( IrqType vector, int8_t type ) {
	Irq_IsrTypeTable[vector] = type;
}

/**
 *
 * @param vector
 * @return 0 - Isr1
 *         1 - Isr2
 */
static inline int8_t Irq_GetIsrType( IrqType vector )  {
	return Irq_IsrTypeTable[vector];
}
#endif


#if 0
typedef struct {
	uint32_t dummy;
} exc_stack_t;
#endif


#endif /* IRQ_H_ */
