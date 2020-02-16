/*
 * arch_stack.h
 *
 *  Created on:  Mar 4, 2013
 *      Author:  Zhang Shuzhou
 *  Reviewed on:
 *     Reviewer:
 *
 */
#ifndef ARCH_STACK_H_
#define ARCH_STACK_H_


#define SC_PATTERN		0xde
#define LC_PATTERN		0xad

/* Minimum alignment req */
#define ARCH_ALIGN		4

// NVREGS: r0+r1+r2+r3+r4+r5+r6+r7+r8+r9+r10+r11+r12+lr = 14*4 = 40
#define NVGPR_SIZE		56
// VGPR: 9*4 = 36
//#define VGPR_SIZE		36
// SP + context
#define C_SIZE			8
#define VGPR_LR_OFF		(C_SIZE+NVGPR_SIZE-4)
#define C_CONTEXT_OFFS  4
#define C_SP_OFF 		0
#define SC_SIZE			(NVGPR_SIZE+C_SIZE)

#if !defined(_ASSEMBLER_)
#include <stdint.h>
/**
 * @brief   Interrupt saved context.
 * @details This structure represents the stack frame saved during a
 *          preemption-capable interrupt handler.
 */
typedef struct {
	uint32_t      spsr_irq;
	uint32_t      lr_irq;
	uint32_t      r0;
	uint32_t      r1;
	uint32_t      r2;
	uint32_t      r3;
	uint32_t      r12;
	uint32_t      lr_usr;
}Os_ISRFrameType;

/**
 * @brief   System saved context.
 * @details This structure represents the inner stack frame during a context
 *          switching.
 */
typedef struct {
	uint32_t      r0;
	uint32_t      r1;
	uint32_t      r2;
	uint32_t      r3;
	uint32_t      r4;
	uint32_t      r5;
	uint32_t      r6;
	uint32_t      r7;
	uint32_t      r8;
	uint32_t      r9;
	uint32_t      r10;
	uint32_t      r11;
	uint32_t      r12;
	uint32_t      lr;
}StackNvgprType;

/**
 * @brief   Platform dependent part of the @p Thread structure.
 * @details In this port the structure just holds a pointer to the @p intctx
 *          structure representing the stack pointer at context switch time.
 */
struct context {
  struct StackNvgprType *r13;
};



#endif /* _ASSEMBLER_ */


#endif /* ARCH_STACK_H_ */
