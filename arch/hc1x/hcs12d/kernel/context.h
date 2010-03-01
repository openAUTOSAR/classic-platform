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
 * context.h
 *
 * DESCRIPTION
 *   ARM Cortex-M3 (architecture ARMv7-M).
 *
 * REFERENCES
 * - Any ABI?
 *
 * REGISTER USE
 *   Args
 *     ?
 *
 * RETURN
 *   ?? r0?
 *
 *   Non-volatile regs (saved by function call)
 *     ?
 *
 *   Misc
 *     ?
 *
 * EXCEPTION FRAME
 *    ?
 *
 * EXCEPTION/IRQ TABLE
 *    ?
 *
 *   EXCEPTIONS
 *     ?
 *
 *   IRQ
 *     ?
 *
 * EXCEPTION/IRQ FLOW
 */

#ifndef CONTEXT_H_
#define CONTEXT_H_


#define SC_PATTERN		0xde
#define LC_PATTERN		0xad

/* Minimum alignment req */
#define ARCH_ALIGN		4

/* Small context (task swap==function call) */
#define SAVE_NVGPR(_x,_y)
#define RESTORE_NVGPR(_x,_y)

/* Save volatile regs, NOT preserved by function calls */
#define SAVE_VGPR(_x,_y)
#define RESTORE_VGPR(_x,_y)

/* Large context (interrupt) */
#define SAVE_ALL_GPR(_x,_y)
#define RESTORE_ALL_GPR(_x,_y)


// NVREGS: r4+r5+r6+r7+r8++r10+r11+lr = 9*4 = 36
#define NVGPR_SIZE		32
// VGPR: 9*4 = 36
#define VGPR_SIZE		36
// SP + context
#define C_SIZE			8

//...
#define VGPR_LR_OFF		(C_SIZE+NVGPR_SIZE-4)
#define C_CONTEXT_OFF   4
#define C_SP_OFF 		0
#define C_LR_OFF		16 // TODO: value is from ppc

#define SC_SIZE			(NVGPR_SIZE+C_SIZE)


#endif /* CONTEXT_H_ */
