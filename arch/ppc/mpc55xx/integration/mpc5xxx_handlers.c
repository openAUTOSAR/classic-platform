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
#if defined(CFG_MPC5XXX_TEST)
#include "embUnit/embUnit.h"
#endif
#include "mpc55xx.h"
#include "Mcu.h"
#include "asm_ppc.h"
#include "arch_stack.h"


/* ----------------------------[private define]------------------------------*/
#define TODO_NUMBER  0

/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/

#if defined(CFG_MPC5XXX_TEST)
uint32_t Mpc5xxx_vectorMask;
#endif

/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/


static void Os_Panic( uint32_t error ) {

	(void)error;
#if ( MCU_PERFORM_RESET_API == STD_ON )
	Mcu_PerformReset();
#else
	while(1) {};
#endif


}

static void preHook( void ) {

}


/*
 * Gets the instruction length of instructions at an address.
 * Supports ONLY load/store instructions.
 *
 * Applies to load/store instructions ONLY
 * - does NOT support SPE instructions
 */
static uint32_t adjustReturnAddr( uint32_t instrAddr ) {

	/* ESR[VLEMI] - if set indicate VLE instruction
	 * Supports
	 * - Data Storage
	 * - Data TLB
	 * - Instruction Storage
	 * - Alignment
	 * - Program
	 * - System Call
	 * */

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

	if( (get_spr(SPR_ESR) & ESR_VLEMI) &&
		(*(uint8_t *)(instrAddr) & 0x80u ) ) {
		/* Executing VLE and 16-bit instructions */
		instrAddr += 2;
	} else {
		instrAddr += 4;
	}
	return instrAddr;
}

/* Critical Input:  CSRR0, CSRR1 */
void Mpc5xxx_Exception_IVOR0( void  ) {
	preHook();

}

/* Machine Check:   CSRR0, CSRR1, MCSR */
uint32_t Mpc5xxx_Exception_IVOR1( void ) {
	preHook();

#if defined(CFG_MPC5XXX_TEST)
	Mpc5xxx_vectorMask |= (1<<1);
#else
	Os_Panic(TODO_NUMBER);
#endif
	return adjustReturnAddr(get_spr(SPR_CSRR0));
}

/* Data Storage:    SRR0, SRR1, ESR, DEAR */
uint32_t Mpc5xxx_Exception_IVOR2( void ) {
	preHook();
#if defined(CFG_MPC5XXX_TEST)
	Mpc5xxx_vectorMask |= (1<<2);
#else
	Os_Panic(TODO_NUMBER);
#endif

	return adjustReturnAddr(get_spr(SPR_SRR0));
}

/* Inst. Storage:   SRR0, SRR1, ESR */
uint32_t Mpc5xxx_Exception_IVOR3( void ) {
	preHook();
	return get_spr(SPR_SRR0);
}


/* Alignment:       SRR0, SRR1, ESR, DEAR */
uint32_t Mpc5xxx_Exception_IVOR5( void ) {
	preHook();

	return get_spr(SPR_SRR0);
}

/* Program:         SRR0, SRR1, ESR */
uint32_t Mpc5xxx_Exception_IVOR6( void ) {
	preHook();

	return get_spr(SPR_SRR0);
}

/* Floating Point:  SRR0, SRR1 */
uint32_t Mpc5xxx_Exception_IVOR7( void ) {
	preHook();
	return get_spr(SPR_SRR0);
}

/* System Call:     SRR0, SRR1, ESR */
uint32_t Mpc5xxx_Exception_IVOR8( void ) {
	preHook();
	return get_spr(SPR_SRR0);
}

/* FIT:             SRR0, SRR1 */
uint32_t Mpc5xxx_Exception_IVOR11( void ) {
	preHook();
	return get_spr(SPR_SRR0);
}

/* Watchdog:        CSRR0, CSRR1 */
uint32_t Mpc5xxx_Exception_IVOR12( void ) {
	preHook();
	return get_spr(SPR_CSRR0);
}

/* Data TLB:        SRR0, SRR1, ESR, DEAR */
uint32_t Mpc5xxx_Exception_IVOR13( void ) {
	preHook();
	return get_spr(SPR_SRR0);
}

/* Intstr TLB:      SRR0, SRR1, ESR */
uint32_t Mpc5xxx_Exception_IVOR14( void ) {
	preHook();
	return get_spr(SPR_SRR0);
}

