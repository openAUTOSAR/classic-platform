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


#include <stddef.h>
#include "Os.h"
#include "internal.h"
#include "arch_stack.h"
#include "Compiler.h"




#if defined(__GNUC__)
#define DECLARE(sym,val) \
	__asm("#define " #sym " %0" : : "i" ((val)))

#elif defined(__CWCC__)

#pragma section ".apa" ".apa"

#define DECLARE(_var,_offset) \
    __declspec(section ".apa") char _var[100+_offset]
#elif (defined(__DCC__))
#define DECLARE(_sym,_val) \
	const int arc_dummy_ ## _sym = _val
#endif

#if defined(__GNUC__)
void  oil_foo(void);

void  oil_foo(void) {
#endif

/* Exceptions */
	DECLARE(EXC_FRM_SIZE,	sizeof(Os_ExceptionFrameType));
	DECLARE(EXC_FRM_SP,		offsetof(Os_ExceptionFrameType, sp));
	DECLARE(EXC_FRM_SRR0,	offsetof(Os_ExceptionFrameType, srr0));
	DECLARE(EXC_FRM_SRR1,	offsetof(Os_ExceptionFrameType, srr1));
	DECLARE(EXC_FRM_LR,		offsetof(Os_ExceptionFrameType, lr));
	DECLARE(EXC_FRM_CTR,	offsetof(Os_ExceptionFrameType, ctr));
	DECLARE(EXC_FRM_XER,	offsetof(Os_ExceptionFrameType, xer));
	DECLARE(EXC_FRM_CR,		offsetof(Os_ExceptionFrameType, cr));
	DECLARE(EXC_FRM_R3,		offsetof(Os_ExceptionFrameType, r3));
	DECLARE(EXC_FRM_VECTOR,	offsetof(Os_ExceptionFrameType, vector));

/* Volatile frame */
	DECLARE(VOLATILE_FRM_SIZE,	sizeof(Os_VolatileFrameType)+16);
	DECLARE(VOLATILE_FRM_R0,	offsetof(Os_VolatileFrameType, r0));
	DECLARE(VOLATILE_FRM_R3,	offsetof(Os_VolatileFrameType, r3));
	DECLARE(VOLATILE_FRM_R4,	offsetof(Os_VolatileFrameType, r4));
	DECLARE(VOLATILE_FRM_R5,	offsetof(Os_VolatileFrameType, r5));
	DECLARE(VOLATILE_FRM_R6,	offsetof(Os_VolatileFrameType, r6));
	DECLARE(VOLATILE_FRM_R7,	offsetof(Os_VolatileFrameType, r7));
	DECLARE(VOLATILE_FRM_R8,	offsetof(Os_VolatileFrameType, r8));
	DECLARE(VOLATILE_FRM_R9,	offsetof(Os_VolatileFrameType, r9));
	DECLARE(VOLATILE_FRM_R10,	offsetof(Os_VolatileFrameType, r10));
	DECLARE(VOLATILE_FRM_R11,	offsetof(Os_VolatileFrameType, r11));
	DECLARE(VOLATILE_FRM_R12,	offsetof(Os_VolatileFrameType, r12));
	DECLARE(VOLATILE_FRM_LR,	offsetof(Os_VolatileFrameType, lr));
	DECLARE(VOLATILE_FRM_CTR,	offsetof(Os_VolatileFrameType, ctr));
	DECLARE(VOLATILE_FRM_XER,	offsetof(Os_VolatileFrameType, xer));
	DECLARE(VOLATILE_FRM_CR,	offsetof(Os_VolatileFrameType, cr));

	/* Exception info */
	DECLARE(MPC5XXX_INFO_SIZE,	sizeof(Mpc5xxx_ExceptionInfoType));
	DECLARE(MPC5XXX_INFO_C_SRR0,offsetof(Mpc5xxx_ExceptionInfoType, c_srr0));
	DECLARE(MPC5XXX_INFO_C_SRR1,offsetof(Mpc5xxx_ExceptionInfoType, c_srr1));
	DECLARE(MPC5XXX_INFO_MCSR,	offsetof(Mpc5xxx_ExceptionInfoType, mcsr));
	DECLARE(MPC5XXX_INFO_ESR,	offsetof(Mpc5xxx_ExceptionInfoType, esr));
	DECLARE(MPC5XXX_INFO_DEAR,	offsetof(Mpc5xxx_ExceptionInfoType, dear));

	DECLARE(FUNC_FRM_SIZE,	sizeof(Os_FuncFrameType));
	DECLARE(FUNC_FRM_CR, offsetof(Os_FuncFrameType,cr));
	DECLARE(FUNC_FRM_LR, offsetof(Os_FuncFrameType,lr));
	DECLARE(FUNC_FRM_PATTERN, offsetof(Os_FuncFrameType,pattern));
	DECLARE(FUNC_FRM_R14, offsetof(Os_FuncFrameType,r14));
	DECLARE(FUNC_FRM_R15, offsetof(Os_FuncFrameType,r15));
	DECLARE(FUNC_FRM_R16, offsetof(Os_FuncFrameType,r16));
	DECLARE(FUNC_FRM_R17, offsetof(Os_FuncFrameType,r17));
	DECLARE(FUNC_FRM_R18, offsetof(Os_FuncFrameType,r18));
	DECLARE(FUNC_FRM_R19, offsetof(Os_FuncFrameType,r19));
	DECLARE(FUNC_FRM_R20, offsetof(Os_FuncFrameType,r20));
	DECLARE(FUNC_FRM_R21, offsetof(Os_FuncFrameType,r21));
	DECLARE(FUNC_FRM_R22, offsetof(Os_FuncFrameType,r22));
	DECLARE(FUNC_FRM_R23, offsetof(Os_FuncFrameType,r23));
	DECLARE(FUNC_FRM_R24, offsetof(Os_FuncFrameType,r24));
	DECLARE(FUNC_FRM_R25, offsetof(Os_FuncFrameType,r25));
	DECLARE(FUNC_FRM_R26, offsetof(Os_FuncFrameType,r26));
	DECLARE(FUNC_FRM_R27, offsetof(Os_FuncFrameType,r27));
	DECLARE(FUNC_FRM_R28, offsetof(Os_FuncFrameType,r28));
	DECLARE(FUNC_FRM_R29, offsetof(Os_FuncFrameType,r29));
	DECLARE(FUNC_FRM_R30, offsetof(Os_FuncFrameType,r30));
	DECLARE(FUNC_FRM_R31, offsetof(Os_FuncFrameType,r31));

#if defined(CFG_SPE)
	DECLARE(ISR_FRM_SPE_FSCR, offsetof(Os_IsrFrameType,fscr));
	DECLARE(ISR_FRM_SPE_ACC, offsetof(Os_IsrFrameType,acc));
	DECLARE(ISR_FRM_R3, offsetof(Os_IsrFrameType,acc));
#endif

	DECLARE(ISR_FRM_SIZE,	sizeof(Os_IsrFrameType));
	DECLARE(ISR_FRM_R0, offsetof(Os_IsrFrameType,r0));
	DECLARE(ISR_FRM_R4, offsetof(Os_IsrFrameType,r4));
	DECLARE(ISR_FRM_R5, offsetof(Os_IsrFrameType,r5));
	DECLARE(ISR_FRM_R6, offsetof(Os_IsrFrameType,r6));
	DECLARE(ISR_FRM_R7, offsetof(Os_IsrFrameType,r7));
	DECLARE(ISR_FRM_R8, offsetof(Os_IsrFrameType,r8));
	DECLARE(ISR_FRM_R9, offsetof(Os_IsrFrameType,r9));
	DECLARE(ISR_FRM_R10, offsetof(Os_IsrFrameType,r10));
	DECLARE(ISR_FRM_R11, offsetof(Os_IsrFrameType,r11));
	DECLARE(ISR_FRM_R12, offsetof(Os_IsrFrameType,r12));
	DECLARE(ISR_FRM_R14, offsetof(Os_IsrFrameType,r14));
	DECLARE(ISR_FRM_R15, offsetof(Os_IsrFrameType,r15));
	DECLARE(ISR_FRM_R16, offsetof(Os_IsrFrameType,r16));
	DECLARE(ISR_FRM_R17, offsetof(Os_IsrFrameType,r17));
	DECLARE(ISR_FRM_R18, offsetof(Os_IsrFrameType,r18));
	DECLARE(ISR_FRM_R19, offsetof(Os_IsrFrameType,r19));
	DECLARE(ISR_FRM_R20, offsetof(Os_IsrFrameType,r20));
	DECLARE(ISR_FRM_R21, offsetof(Os_IsrFrameType,r21));
	DECLARE(ISR_FRM_R22, offsetof(Os_IsrFrameType,r22));
	DECLARE(ISR_FRM_R23, offsetof(Os_IsrFrameType,r23));
	DECLARE(ISR_FRM_R24, offsetof(Os_IsrFrameType,r24));
	DECLARE(ISR_FRM_R25, offsetof(Os_IsrFrameType,r25));
	DECLARE(ISR_FRM_R26, offsetof(Os_IsrFrameType,r26));
	DECLARE(ISR_FRM_R27, offsetof(Os_IsrFrameType,r27));
	DECLARE(ISR_FRM_R28, offsetof(Os_IsrFrameType,r28));
	DECLARE(ISR_FRM_R29, offsetof(Os_IsrFrameType,r29));
	DECLARE(ISR_FRM_R30, offsetof(Os_IsrFrameType,r30));
	DECLARE(ISR_FRM_R31, offsetof(Os_IsrFrameType,r31));
	DECLARE(ISR_FRM_PATTERN, offsetof(Os_IsrFrameType,pattern));

#if defined(__GNUC__)
}
#endif

