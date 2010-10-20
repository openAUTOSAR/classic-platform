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
#include <stdint.h>

#define DECLARE(_sym,_val) \
	__asm("#define " #_sym " %0 ": : "i" (_val))


#include "stack.h"

void  arch_foo(void) {
	/* StackNvgprType */
	DECLARE(STACK_NVGPR_R4,		offsetof(StackNvgprType, r4));
	DECLARE(STACK_NVGPR_R5,		offsetof(StackNvgprType, r5));
	DECLARE(STACK_NVGPR_R6,		offsetof(StackNvgprType, r6));
	DECLARE(STACK_NVGPR_R7,		offsetof(StackNvgprType, r7));
	DECLARE(STACK_NVGPR_R8,		offsetof(StackNvgprType, r8));
	DECLARE(STACK_NVGPR_R10,	offsetof(StackNvgprType, r10));
	DECLARE(STACK_NVGPR_R11,	offsetof(StackNvgprType, r11));
	DECLARE(STACK_NVGPR_VA,	offsetof(StackNvgprType, va));

	/* StackCallAndContextType */
}



