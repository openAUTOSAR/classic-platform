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

/* ----------------------------[information]----------------------------------*/
/*
 * Author: mahi
 *
 * Description:
 *   Implements exceptions handlers for the mpc5xxx series.
 *   Vectors that are not used should probably report to some non-init RAM
 *   area and reset.
 *
 *   Comments about the vectors.
 *   IVOR0    - Critical interrupt
 *   IVOR1    - Machine check
 *   IVOR2    - Data Storage
 *   IVOR3    - Inst. Storage
 *   IVOR4    - External Input (INTC)
 *   IVOR5    - Alignment
 *   IVOR6    - Program
 *   IVOR7    - Floating point
 *   IVOR8    - System Call
 *   IVOR9    - Aux Processor (NOT USED)
 *   IVOR10   - Decrementer
 *   IVOR11   - FIT
 *   IVOR12   - Watchdog
 *   IVOR13   - Data TLB
 *   IVOR14   - Instr TLB
 *   IVOR15
 *   IVOR32
 *   IVOR33
 *   IVOR34
 *
 *   For *users* of the system the following could be of use:
 *     IVOR0  - If you use NMI pin and want to do something really critical.
 *     IVOR11 - The FIT is really bad timer (binary timeout) and since it's an exception
 *              it has higher priority than the INTC
 *     IVOR12 - Watchdog
 *
 *   and then we have ECC errors that affect IVOR1, IVOR2 and IVOR3.
 *   The exceptions can be divided into:
 *   - FEE related. Within this memory range the FEE should handle the errors.
 *     (Reset during program/erase should be considered normal behavior )
 *   - Non-FEE related. Something is very wrong with the flash.. record error and reset?
 *
 *  Design decision:
 *  - Call Os_Panic() for all exceptions not setup by the system.
 *  - Place Os_Panic() in Mcu_<cpu>.c
 *  - TODO: FEE memory ranges must be set somewhere?
 *  - Mcu_AddExceptionHook()
 *
 */


/* ----------------------------[includes]------------------------------------*/

//#if defined(CFG_MPC5XXX_TEST)
//#include "embUnit/embUnit.h"
//#endif
#include "mpc55xx.h"
#include "Mcu.h"
#include "asm_ppc.h"
#include "arch_stack.h"
#include "io.h"
#include "Os.h"

/* ----------------------------[private define]------------------------------*/
#define TODO_NUMBER  0

/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/

extern uint32_t Mcu_Arc_ExceptionHook(uint32_t exceptionVector);


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

#if 0
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
#endif


static uint32_t handleException( uint32_t exception , uint32_t spr ) {
	uint32_t rv;
	uint32_t _spr;

	rv = Mcu_Arc_ExceptionHook( exception );
	if( rv & EXC_NOT_HANDLED ) {
		Os_Panic(exception,NULL);
	}

	_spr = (spr==SPR_CSRR0) ? get_spr(SPR_CSRR0) : get_spr(SPR_SRR0);

	if( rv & EXC_ADJUST_ADDR ) {
		rv = adjustReturnAddr( _spr );
	} else {
		rv = _spr;
	}
	return rv;
}


/* Critical Input:  CSRR0, CSRR1 */
uint32_t Mpc5xxx_Exception_IVOR0( void  ) {
	return handleException(0, SPR_CSRR0 );
}

/* Machine Check:   CSRR0, CSRR1, MCSR */
uint32_t Mpc5xxx_Exception_IVOR1( void ) {
	return handleException(1, SPR_CSRR0 );
}

/* Data Storage:    SRR0, SRR1, ESR, DEAR */
uint32_t Mpc5xxx_Exception_IVOR2( void ) {
	return handleException(2, SPR_SRR0 );
}

/* Inst. Storage:   SRR0, SRR1, ESR */
uint32_t Mpc5xxx_Exception_IVOR3( void ) {
	return handleException(3, SPR_SRR0 );
}

/* Alignment:       SRR0, SRR1, ESR, DEAR */
uint32_t Mpc5xxx_Exception_IVOR5( void ) {
	return handleException(5, SPR_SRR0 );
}

/* Program:         SRR0, SRR1, ESR */
uint32_t Mpc5xxx_Exception_IVOR6( void ) {
	return handleException(6, SPR_SRR0 );
}

/* Floating Point:  SRR0, SRR1 */
uint32_t Mpc5xxx_Exception_IVOR7( void ) {
	return handleException(7, SPR_SRR0 );
}

/* System Call:     SRR0, SRR1, ESR */
uint32_t Mpc5xxx_Exception_IVOR8( void ) {
	return handleException(8, SPR_SRR0 );
}

/* FIT:             SRR0, SRR1 */
uint32_t Mpc5xxx_Exception_IVOR11( void ) {
	return handleException(11, SPR_SRR0 );
}

/* Watchdog:        CSRR0, CSRR1 */
uint32_t Mpc5xxx_Exception_IVOR12( void ) {
	return handleException(12, SPR_SRR0 );
}

/* Data TLB:        SRR0, SRR1, ESR, DEAR */
uint32_t Mpc5xxx_Exception_IVOR13( void ) {
	return handleException(13, SPR_SRR0 );
}

/* Intstr TLB:      SRR0, SRR1, ESR */
uint32_t Mpc5xxx_Exception_IVOR14( void ) {
	return handleException(14, SPR_SRR0 );
}

#if defined(CFG_SPE)

/* SPE Unavailable:  SRR0, SRR1, ESR=SPE */
uint32_t Mpc5xxx_Exception_IVOR32( void ) {
	Mcu_Arc_ExceptionHook(32);
	return get_spr(SPR_SRR0);
}

/* SPE Data:  SRR0, SRR1, ESR=SPE */
uint32_t Mpc5xxx_Exception_IVOR33( void ) {
	Mcu_Arc_ExceptionHook(33);
	return get_spr(SPR_SRR0);
}

/* SPE round exception: SRR0, SRR1, ESR=SPE */
uint32_t Mpc5xxx_Exception_IVOR34( void ) {
	Mcu_Arc_ExceptionHook(34);
	return get_spr(SPR_SRR0);
}

#endif




