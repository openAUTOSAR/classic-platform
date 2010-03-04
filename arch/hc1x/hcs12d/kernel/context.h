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
 *
 * PPAGE
 * PC
 * PC
 *
 */

#ifndef CONTEXT_H_
#define CONTEXT_H_

#define OS_KERNEL_CODE_PPAGE	0x38

#define SC_PATTERN				0xde
#define LC_PATTERN				0xad

/*
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
*/

/* Context layout, sizes in 16 bit words */
// -- STACK TOP, CONTEXT START
#define C_REG_CCR_OFF			0
// -- CCR
#define C_REG_D_OFF				1
// -- D
#define C_REG_X_OFF				2
// -- X
#define C_REG_Y_OFF				3
// -- Y
#define C_REG_TMP_OFF			4
// -- _.tmp
#define C_REG_XY_OFF			5
// -- _.xy
#define C_REG_Z_OFF				6
// -- _.z
#define C_REG_FRAME_OFF			7
// -- _.frame
#define C_RETURN_PPAGE_OFF		8
// -- PPAGE
#define C_RETURN_ADDR_OFF		9
// -- ADDR
#define SC_SIZE					10
// -- CONTEXT END

#endif /* CONTEXT_H_ */
