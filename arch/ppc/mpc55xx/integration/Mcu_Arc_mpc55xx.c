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
/* ----------------------------[private define]------------------------------*/

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

#include "Os.h"

/* ----------------------------[private macro]-------------------------------*/

#if defined (CFG_MPC5668)
#define ECSM_BASE 	0xfff40000
#define ECSM_ESR    0x47
#endif


/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/

#if defined(CFG_MPC5XXX_TEST)
uint32_t Mpc5xxx_vectorMask;
uint8_t Mpc5xxx_Esr;
uint8_t Mpc5xxx_Intc_Esr;
#endif

/* ----------------------------[private functions]---------------------------*/



/**
 *
 * @param error
 * @param pData
 */
void Os_Panic( uint32_t error, void *pData ) {

	(void)error;
	(void)pData;

	ShutdownOS(E_OS_PANIC);

}


uint32_t Mcu_Arc_ExceptionHook(uint32_t exceptionVector) {
	uint32_t rv = EXC_NOT_HANDLED;

#if defined(CFG_MPC5XXX_TEST)
	Mpc5xxx_vectorMask |= (1<<exceptionVector);
#endif

	switch (exceptionVector) {
	case 1:
		/* CSRR0, CSRR1, MCSR */
		/* ECC: MSR[EE] = 0 */

	case 2:
		/* SRR0, SRR1, ESR, DEAR */
		/* ECC: MSR[EE] = 1 */
	case 3:
	{
		/* SRR0, SRR1, ESR */

#if defined(USE_FEE) || defined(CFG_MPC5XXX_TEST)
		uint8 esr = READ8( ECSM_BASE + ECSM_ESR );
#endif
#if defined(USE_FEE)
		uint32_t excAddr = READ32( ECSM_BASE + ECSM_FEAR );

		/* Find FLS errors */

		if (esr & ESR_FNCE) {

			/* Check if we are in FEE range */
			if ( ((FEE_BANK1_OFFSET >= excAddr) &&
				  (FEE_BANK1_OFFSET + FEE_BANK1_LENGTH < excAddr)) ||
				 ((FEE_BANK2_OFFSET >= excAddr) &&
				  (FEE_BANK2_OFFSET + FEE_BANK2_LENGTH < excAddr)) )
			{
				/* Record that something bad has happend */
				EccErrReg = READ8( ECSM_BASE + ECSM_ESR );
				/* Clear the exception */
				WRITE8(ECSM_BASE+ECSM_ESR,ESR_F1BC+ESR_FNCE);
				rv = EXC_HANDLED | EXC_ADJUST_ADDR;
			}
		}
#endif
#if defined(CFG_MPC5XXX_TEST)
		if( esr & (ESR_R1BC+ESR_RNCE) ) {
			/* ECC RAM problems */
			Mpc5xxx_Esr = esr;
			WRITE8(ECSM_BASE+ECSM_ESR,ESR_R1BC+ESR_RNCE);
			rv = (EXC_HANDLED | EXC_ADJUST_ADDR);
		}
#endif
		break;
	}

	default:
		break;
	}

	return rv;
}

void Mcu_Arc_InitMM( void ) {
	/* User: Enable caches if any */

	/* User: Setup TLBs if needed  */
	MM_TlbSetup( Mcu_Arc_ConfigData.tblTable );
}


/*
 * Called at a very early stage...
 */
void Mcu_Arc_InitPost( void ) {
	Mcu_Arc_InitMM();
}


/* ----------------------------[public functions]----------------------------*/

void Mcu_Arc_InitClockPre( const Mcu_ClockType ClockSetting )
{
}


void Mcu_Arc_InitClockPost( const Mcu_ClockType ClockSetting )
{
}

/**
 *
 * Application Notes!
 * - AN3584, "MPC5510 Family Low Power Features"
 *   Since it's not complete also check MPC5668
 * - AN4150 , "Using Sleep Mode on the MPC5668x" and it's code
 *
 *
 * @param LPM
 */
void Mcu_Arc_SetModePre( Mcu_ModeType mcuMode)
{
	if( MCU_MODE_RUN == mcuMode ) {

		/* Get back to "normal" halt flags */
#if defined(CFG_MPC5516)
		Mcu_Arc_SetModePre2(mcuMode, Mcu_Arc_ConfigData.sleepConfig );
#elif defined(CFG_MPC5668)
		SIU.HLT0.R = Mcu_SavedHaltFlags[0];
		SIU.HLT1.R = Mcu_SavedHaltFlags[1];
#endif

	} else if( MCU_MODE_SLEEP == mcuMode ) {
		/*
		 * Follows the AN3548 from Freescale
		 *
		 */
		/* Set Recover Vector */
	#if defined(CFG_MPC5516)
		Mcu_Arc_SetModePre2(mcuMode, Mcu_Arc_ConfigData.sleepConfig);

	#elif defined(CFG_MPC5668)
#if defined(USE_DMA)
		Dma_DeInit();
#endif

		/* Set system clock to 16Mhz IRC */
		SIU.SYSCLK.B.SYSCLKSEL = R_SYSCLKSEL;

		/* Put flash in low-power mode */
		// TODO



		/* Put QQADC in low-power mode */
		// TODO

		/* Set us in SLEEP mode */
		CRP.PSCR.B.SLEEP = 1;


		uint32 timeout = 0;
		/* - Set the sleep bit; following a WAIT instruction, the device will go to sleep
		 * - enable the 1.2V internal regulator when in sleep mode only
		 */

		READWRITE32(CRP_PSCR, (PSCR_SLEEP | PSCR_SLP12EN | PCSR_RAMSEL(0x7)), (PSCR_SLEEP | PSCR_SLP12EN | PCSR_RAMSEL(RAMSEL_VAL)));
		WRITE32(CRP_Z6VEC, ((uint32)&McuE_LowPowerRecoverFlash) | VLE_VAL );
		READWRITE32(CRP_RECPTR,RECPTR_FASTREC,0 );

		Mcu_SavedHaltFlags[0] = SIU.HLT0.R;
		Mcu_SavedHaltFlags[1] = SIU.HLT1.R;
		/* Halt everything */
	    SIU.HLT0.R = R_HLT0;
	    SIU.HLT1.R = R_HLT1;
	    while((SIU.HLTACK0.R != R_HLT0) && (SIU.HLTACK1.R != R_HLT1) && (timeout<HLT_TIMEOUT)){}
	#else
	#error CPU not defined
	#endif
//
//		/* put Z0 in reset if not used for wakeup */
//		CRP.Z0VEC.B.Z0RST = 1;
//
//	    /* Save context and execute wait instruction.
//		 *
//		 * Things that matter here are
//		 * - Z1VEC, determines where TLB0 will point. TLB0 is written with a
//		 *   value at startup that 4K aligned to this address.
//		 * - LowPower_Sleep() will save a interrupt context so we will return
//		 *   intact.
//		 * - For devices with little RAM we don't want to impose the alignment
//		 *   requirements there. Almost as we have to occupy a 4K block for this..
//		 *   although the code does not take that much space.
//		 * */
//		Mcu_Arc_EnterLowPower(mcuMode);
//
//		/* Back from Sleep */
//
//		/* Setup exceptions and INTC again */
//		Os_IsrInit();
//
//		/* Clear sleep flags to allow pads to operate */
//	    CRP.PSCR.B.SLEEPF = 0x1;
//
//#if defined(USE_ECUM)
//		EcuM_CheckWakeup( 0x3fffffffUL );
//#endif

	}
}

void Mcu_Arc_SetModePost( Mcu_ModeType mcuMode)
{
	if( MCU_MODE_RUN == mcuMode ) {
		Mcu_Arc_SetModePost2(mcuMode,  Mcu_Arc_ConfigData.sleepConfig);
	} else if( MCU_MODE_SLEEP == mcuMode ) {
		Mcu_Arc_SetModePost2(mcuMode,  Mcu_Arc_ConfigData.sleepConfig);
	}
}



