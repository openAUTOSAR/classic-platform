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


/* ----------------------------[includes]------------------------------------*/
#include "mpc55xx.h"

/* ----------------------------[private define]------------------------------*/
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/


static void Os_Panic( OS_SP ) {

}


/*
 * Gets the instruction length of instructions at an address.
 * Supports ONLY load/store instructions.
 *
 * Applies to load/store instructions ONLY
 * - does NOT support SPE instructions
 */
static uint32_t adjustReturnAddr( uint32_t instrAddr ) {

	/* ESR[VLEMI] - if set indicate VLE instruction */

	/* VLE supports ONLY big-endian */

	/* Can't find any way to determine if it's a 32-bit or 16-bit
	 * instructions that is run.
	 *
	 * OP-code encoding. From PowerISA_V2.06B_V2_PUBLIC.pdf Appendix A.
	 *
	 * se_lxxx   8,a,c
	 * e_lxxx    3,1,3,1,5,1,1,5,1
	 * se_stxx   9,b,d
	 * e_stxx    3,1,5,1,1,5
	 *
	 * -> first opcode byte > 7 --> 16-bit, ie (opcode&0x80)
	 *
	 */
	uint32_t ra;

	if( (get_spr(SPR_ESR) & ESR_VLEMI) &&
		(*(uint8_t *)(instrAddr) & 0x80u ) ) {
		/* Executing VLE and 16-bit instructions */
		ra += 2;
	} else {
		ra += 4;
	}
	return ra;
}

void Mpc5xxx_Exception_IVOR0( Mpc5xxx_ExceptionFrmType *frmPtr ) {

}


uint32_t Mpc5xxx_Exception_IVOR2( Mpc5xxx_ExceptionFrmType *frmPtr ) {
	(void)frmPtr;

	Os_Panic(OS_ERR_SPURIOUS_INTERRUPT);

	return adjustReturnAddr(frmPtr->srr0)
}

