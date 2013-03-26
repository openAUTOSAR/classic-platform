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
 *
 */


/* ----------------------------[includes]------------------------------------*/


#include "Std_Types.h"
#include "Mcu.h"
#include "io.h"
#include "mpc55xx.h"
#include "Mcu_Arc.h"
#if defined(USE_FEE)
#include "Fee_Memory_Cfg.h"
#endif
#if defined(USE_DMA)
#include "Dma.h"
#endif
#include "asm_ppc.h"
#include "Os.h"

/* ----------------------------[private define]------------------------------*/

#if defined (CFG_MPC5668)
#define ECSM_BASE 	0xfff40000
#define ECSM_ESR    0x47
#endif

/* ----------------------------[private macro]-------------------------------*/



/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/

#if defined(CFG_MPC5XXX_TEST)
uint32_t Mpc5xxx_vectorMask;
uint8_t Mpc5xxx_Esr;
uint8_t Mpc5xxx_Intc_Esr;
#endif

extern uint32 EccErrReg;

/* ----------------------------[private functions]---------------------------*/


/**
 * Function called by exception handlers when there is a fatal error.
 * Will also call ShutdownOs(E_OS_PANIC).
 *
 * @param error		The exception that caused the error
 * @param pData		Pointer to private data that may want to be dumped.
 */
void Mpc5xxx_Panic( uint32_t error, void *pData ) {

	(void)error;
	(void)pData;

	ShutdownOS(E_OS_PANIC);
}

/**
 * Function that checks for ECC errors
 *
 * @return
 */
static uint32_t checkEcc(void) {
	uint32_t rv = EXC_NOT_HANDLED;

#if defined(USE_FEE) || defined(CFG_MPC5XXX_TEST)

	uint8 esr;
	do {
		esr = READ8( ECSM_BASE + ECSM_ESR );
	} while( esr != READ8( ECSM_BASE + ECSM_ESR ) );

#endif

#if defined(USE_FLS)
#if defined(USE_FEE)
	uint32_t excAddr = READ32( ECSM_BASE + ECSM_FEAR );
#endif

	/* Find FLS errors */

	if (esr & ESR_FNCE) {

		/* Record that something bad has happened */
		EccErrReg = READ8( ECSM_BASE + ECSM_ESR );
		/* Clear the exception */
		WRITE8(ECSM_BASE+ECSM_ESR,ESR_F1BC+ESR_FNCE);
#if defined(USE_FEE)
		/* Check if we are in FEE range */
		if ( ((FEE_BANK1_OFFSET >= excAddr) &&
						(FEE_BANK1_OFFSET + FEE_BANK1_LENGTH < excAddr)) ||
				((FEE_BANK2_OFFSET >= excAddr) &&
						(FEE_BANK2_OFFSET + FEE_BANK2_LENGTH < excAddr)) )
		{
			rv = EXC_HANDLED | EXC_ADJUST_ADDR;
		}
#endif
	}
#endif	 /* USE_FLS */

#if defined(CFG_MPC5XXX_TEST)
	if( esr & (ESR_R1BC+ESR_RNCE) ) {
		/* ECC RAM problems */
		Mpc5xxx_Esr = esr;
		WRITE8(ECSM_BASE+ECSM_ESR,ESR_R1BC+ESR_RNCE);
		rv = (EXC_HANDLED | EXC_ADJUST_ADDR);
	} else if (esr & ESR_FNCE) {
		Mpc5xxx_Esr = esr;
		WRITE8(ECSM_BASE+ECSM_ESR,ESR_F1BC+ESR_FNCE);
		rv = (EXC_HANDLED | EXC_ADJUST_ADDR);
	}
#endif
	return rv;
}


/* ----------------------------[public functions]----------------------------*/

/**
 * Called by lower level exception handlers and tries to handle
 * the exception.
 *
 * @param exceptionVector The exception vector.
 * @return
 */
uint32_t Mpc5xxx_ExceptionHandler(uint32_t exceptionVector) {
	uint32_t rv = EXC_NOT_HANDLED;

#if defined(CFG_MPC5XXX_TEST)
	Mpc5xxx_vectorMask |= (1<<exceptionVector);
#endif

	switch (exceptionVector) {
	case 1:
		/* CSRR0, CSRR1, MCSR */
		/* ECC: MSR[EE] = 0 */
#if defined(CFG_MPC5XXX_TEST)
		if( get_spr(SPR_MCSR) & ( MCSR_BUS_DRERR | MCSR_BUS_WRERR )) {
			/* We have a bus error */
			rv = EXC_HANDLED | EXC_ADJUST_ADDR;
			break;
		}
#endif
		rv = checkEcc();
		break;
	case 2:
		/* SRR0, SRR1, ESR, DEAR */
		/* ECC: MSR[EE] = 1 */
#if defined(CFG_MPC5XXX_TEST)
		if( get_spr(SPR_ESR) &  ESR_XTE) {
			/* We have a external termination bus error */
			rv = EXC_HANDLED | EXC_ADJUST_ADDR;
			break;
		}
#endif
		rv = checkEcc();
		break;
	case 3:
	{
		/* SRR0, SRR1, ESR */
		rv = checkEcc();
		break;
	}

	default:
		break;
	}

	return rv;
}


