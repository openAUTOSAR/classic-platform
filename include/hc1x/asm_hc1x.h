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



#ifndef ASM_HC1X_H_
#define ASM_HC1X_H_


/*
 * Function argument macros
 */

#define ARG_1_STACK_OFFS	3
#define ARG_2_STACK_OFFS	5

// Arg 0 is already in D
#define GET_ARG_0_TO_D

// Arg 0 (D) -> X
#define GET_ARG_0_TO_X		\
		tfr		d,x;

// Arg 0 (D) -> Y
#define GET_ARG_0_TO_Y		\
		tfr		d,y;

// Arg 1 (from stack) -> reg
#define GET_ARG_1_TO(reg)	\
		ld##reg		ARG_1_STACK_OFFS,sp;

// Arg 2 (from stack) -> reg
#define GET_ARG_2(reg) 		\
		ld##reg		ARG_2_STACK_OFFS,x;

// Macro used to create the IRQ mapping table
#define DECLARE_IRQ_MAP(x)    \
irq_##x:					  \
		movb	##x, CSize;SP \
		bra		Irq_PreEntry  \

#endif /*ASM_HC1X_H_*/
