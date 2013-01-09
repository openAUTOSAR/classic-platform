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

#define CFG_MPC5XXX_TEST

//#if defined(CFG_MPC5XXX_TEST)
//#include "embUnit/embUnit.h"
//#endif
#include "mpc55xx.h"
#include "Mcu.h"
#include "asm_ppc.h"
#include "arch_stack.h"
#include "io.h"


/* ----------------------------[private define]------------------------------*/
#define TODO_NUMBER  0

#define EXC_NOT_HANDLED	1
#define EXC_HANDLED		2


/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/

#if defined(CFG_MPC5XXX_TEST)
uint32_t Mpc5xxx_vectorMask;
uint8_t Mpc5xxx_Esr;
uint8_t Mpc5xxx_Intc_Esr;
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
 *
 * The VLE detection is taken from AN4648 ( VLE 16-bit and 32-bit Instruction
 * Length Decode Algorithm )
 */
static uint32_t adjustReturnAddr( uint32_t instrAddr ) {

	uint32_t vleMode = (get_spr(SPR_ESR) & ESR_VLEMI);

	if( (!vleMode) ||
		(*(uint16_t *)(instrAddr) & 0x9000) == 0x1000 ) {
		instrAddr += 4;
	} else {
		/* Executing VLE and 16-bit instructions */
		instrAddr += 2;
	}
	return instrAddr;
}

static uint32_t handleEcc( uint16_t vector ) {
	uint8_t esr = READ8(ECSM_BASE+ECSM_ESR);
	uint32_t rv  = EXC_NOT_HANDLED;

	if( esr & (ESR_R1BC+ESR_RNCE) ) {
		/* ECC RAM problems */
		Mpc5xxx_Esr = esr;
		WRITE8(ECSM_BASE+ECSM_ESR,ESR_R1BC+ESR_RNCE);
		rv = EXC_HANDLED;

	} else if (esr & (ESR_F1BC+ESR_FNCE)) {
		/* ECC Flash problems */
		Mpc5xxx_Esr = esr;
		WRITE8(ECSM_BASE+ECSM_ESR,ESR_F1BC+ESR_FNCE);
		rv = EXC_HANDLED;
	} else  {
		Mpc5xxx_Esr = 0;
	}

	return rv;
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

	if( handleEcc(1) & EXC_NOT_HANDLED ) {
		/* Do nothing at this point */
	}

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

	if( handleEcc(2) & EXC_NOT_HANDLED ) {
		/* Do nothing at this point */
	}


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

#if defined(CFG_SPE)

/* SPE Unavailable:  SRR0, SRR1, ESR=SPE */
uint32_t Mpc5xxx_Exception_IVOR32( void ) {
	preHook();
	return get_spr(SPR_SRR0);
}

/* SPE Data:  SRR0, SRR1, ESR=SPE */
uint32_t Mpc5xxx_Exception_IVOR33( void ) {
	preHook();
	return get_spr(SPR_SRR0);
}

/* SPE round exception: SRR0, SRR1, ESR=SPE */
uint32_t Mpc5xxx_Exception_IVOR34( void ) {
	preHook();
	return get_spr(SPR_SRR0);
}

#endif




