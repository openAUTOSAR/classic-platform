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

/*
 * REF
 *   PowerPC e500 Application Binary Interface User’s Guide
 *
 * DESCRIPTION
 *
 * REGISTER USE
 *     #       V  Description
 *     ---------------------------------------
 *     r0         Volatile
 *     r1         Stackframe
 *     r2         Special
 *     r3-r10     Arguments
 *     r3,r4      Return values
 *     r13        Small data ptr
 *     r14-31  SC Local variables
 *     CR      SC Condition register
 *     LR         Link resister
 *     CTR        Counter registers
 *     XER        Integer exception register
 *     SPEFSCR    SPE status register *1
 *     ACC        SPE accumulator *1
 *
 *      *1 - Only for a MCU with SPE (e.g 5554)
 *      SC - Saved by callee
 *
 *
 * EXCEPTION
 *   USAGE
 *     IVOR4   External int  exceptions branches directly to the INTC handler
 *     IVOR8   System call  Is called directly with some kind of argument.
 *                           We need to save ISR context?
 *     IVOR10  Decrementer  Probably just a small assembler routine to trigger a
 *                           software interrupt.
 *     IVOR11  FIT          ?
 *     IVOR12  Watchdog     ?
 *     IVOR13  Data TLB     Small assembler routine..
 *     IVOR14  Instr TLB    Small assembler routine..
 *
 *     For IVOR11 and IVOR14 it would probably be nice with a
 *     INTC (ISR1) direct function call.
 *
 *     This leaves us with saving the lot for an IVOR4, for the rest we don't really
 *     care and just call Os_Panic( void *stack )
 *
 *   FRAME
 *
 *      Offs  Reg
 *      ...
 *      ....
 *      0x10  SCC1
 *      0x0c  SCC0
 *      0x08  padding
 *      0x04  Reserved  (backchain)
 *      0x00  SP
 *
 *
 *
 *
 * EXCEPTION/IRQ TABLE
 *
 * EXCEPTION/IRQ FLOW
 *
 */


#ifndef ARCH_STACK_H_
#define ARCH_STACK_H_


/*
 * FULL_PATTERN is used indicate that a full register context is on the stack.
 * FUNC_PATTERN indicates that a function register context is  on the stack
 */
#define ISR_SPE_PATTERN		0x12
#define FUNC_SPE_PATTERN	0x32
#define ISR_PATTERN			0xde
#define FUNC_PATTERN		0xad


/* An interrupt that is not mapped */
#define OS_ERR_SPURIOUS_INTERRUPT	1
/* The context indicator is missing */
#define OS_ERR_BAD_CONTEXT			2

#if !defined(_ASSEMBLER_)
#include <stdint.h>

typedef struct {
	uint32_t sp;
	uint32_t backchain;		/* TODO: Useless since we don't make any c-call from the frame? */
	uint32_t padding;		/* TODO: Useless since we don't make any c-call from the frame? */
	uint32_t srr0;
	uint32_t srr1;
	uint32_t lr;
	uint32_t ctr;
	uint32_t xer;
	uint32_t cr;
	uint32_t vector;
	uint32_t r3;			/*11*4 = 44 < 48*/
	uint32_t pad;
} Os_ExceptionFrameType;

typedef struct {
	uint32_t r0;
	uint32_t r3;
	uint32_t r4;
	uint32_t r5;

	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;

	uint32_t r10;
	uint32_t r11;
	uint32_t r12;
	uint32_t lr;

	uint32_t ctr;
	uint32_t xer;
	uint32_t cr;
	uint32_t pad;
} Os_VolatileFrameType;


typedef struct {
	uint32_t c_srr0;
	uint32_t c_srr1;
	uint32_t mcsr;
	uint32_t esr;
	uint32_t dear;
} Mpc5xxx_ExceptionInfoType;


#if defined(CFG_SPE)
typedef struct {
	uint32_t sp;
	uint32_t backchain;
	uint32_t padding;
	uint32_t pattern;
	uint32_t cr;
	uint32_t lr;
	uint64_t r14;
	uint64_t r15;
	uint64_t r16;
	uint64_t r17;
	uint64_t r18;
	uint64_t r19;
	uint64_t r20;
	uint64_t r21;
	uint64_t r22;
	uint64_t r23;
	uint64_t r24;
	uint64_t r25;
	uint64_t r26;
	uint64_t r27;
	uint64_t r28;
	uint64_t r29;
	uint64_t r30;
	uint64_t r31;
} Os_FuncFrameType;

typedef struct  {
	uint32_t sr;
	uint32_t backchain;
	uint32_t padding;
	uint32_t pattern;
	uint64_t fscr;
	uint64_t acc;
	uint64_t r0;
	/* r1 */
	/* r2 */
	uint64_t r3;
	uint64_t r4;
	uint64_t r5;
	uint64_t r6;
	uint64_t r7;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t r12;
	/* r13 */
	uint64_t r14;
	uint64_t r15;
	uint64_t r16;
	uint64_t r17;
	uint64_t r18;
	uint64_t r19;
	uint64_t r20;
	uint64_t r21;
	uint64_t r22;
	uint64_t r23;
	uint64_t r24;
	uint64_t r25;
	uint64_t r26;
	uint64_t r27;
	uint64_t r28;
	uint64_t r29;
	uint64_t r30;
	uint64_t r31;
} Os_IsrFrameType;
#else
typedef struct {
	uint32_t sp;
	uint32_t backchain;
	uint32_t padding;
	uint32_t pattern;
	uint32_t cr;
	uint32_t lr;
	uint32_t r14;
	uint32_t r15;
	uint32_t r16;
	uint32_t r17;
	uint32_t r18;
	uint32_t r19;
	uint32_t r20;
	uint32_t r21;
	uint32_t r22;
	uint32_t r23;
	uint32_t r24;
	uint32_t r25;
	uint32_t r26;
	uint32_t r27;
	uint32_t r28;
	uint32_t r29;
	uint32_t r30;
	uint32_t r31;
} Os_FuncFrameType;

typedef struct  {
	uint32_t sr;
	uint32_t backchain;
	uint32_t padding;
	uint32_t pattern;
	uint32_t r0;
	/* r1 */
	/* r2 */
	/* r3, already save in exception frame */
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t r12;
	/* r13 */
	uint32_t r14;
	uint32_t r15;
	uint32_t r16;
	uint32_t r17;
	uint32_t r18;
	uint32_t r19;
	uint32_t r20;
	uint32_t r21;
	uint32_t r22;
	uint32_t r23;
	uint32_t r24;
	uint32_t r25;
	uint32_t r26;
	uint32_t r27;
	uint32_t r28;
	uint32_t r29;
	uint32_t r30;
	uint32_t r31;	    /* 3+32-4 + 1= 31 */
} Os_IsrFrameType;

#endif /* defined(CFG_SPE) */

#endif /* !defined(_ASSEMLBER_) */



#endif /* ARCH_STACK_H_ */
