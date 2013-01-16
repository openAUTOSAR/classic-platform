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
/* ----------------------------[private define]------------------------------*/

#include "Mcu_Arc_Cfg.h"

/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
/* ----------------------------[private functions]---------------------------*/


#if defined(CFG_MPC5516) || defined(CFG_MPC5668)

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
#if defined(CFG_MPC5516) || defined(CFG_MPC5668)
	if( MCU_MODE_RUN == mcuMode ) {

		/* Get back to "normal" halt flags */
#if defined(CFG_MPC5516)
		SIU.HLT.R = Mcu_SavedHaltFlags;
#elif defined(CFG_MPC5668)
		SIU.HLT0.R = Mcu_SavedHaltFlags[0];
		SIU.HLT1.R = Mcu_SavedHaltFlags[1];
#endif

	} else if( MCU_MODE_SLEEP == mcuMode ) {
		/*
		 * Follows the AN3548 from Freescale
		 *
		 */
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

		/* Set Recover Vector */
	#if defined(CFG_MPC5516)
		WRITE32(CRP_PSCR, PSCR_SLEEP | PSCR_SLP12EN | PCSR_RAMSEL(RAMSEL_VAL));

		WRITE32(CRP_Z1VEC, ((uint32)&McuE_LowPowerRecoverFlash) | VLE_VAL );
		READWRITE32( CRP_RECPTR, RECPTR_FASTREC, 0 );

		Mcu_SavedHaltFlags = SIU.HLT.R;
		/* Halt everything */
		SIU.HLT.R = R_HLT0;
		while((SIU.HLTACK.R != 0x3FFFFFFF) && (timeout++<HLT_TIMEOUT)) {}

		/* put Z0 in reset if not used for wakeup */
		CRP.Z0VEC.B.Z0RST = 1;

	#elif defined(CFG_MPC5668)
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

		/* put Z0 in reset if not used for wakeup */
		CRP.Z0VEC.B.Z0RST = 1;

	    /* Save context and execute wait instruction.
		 *
		 * Things that matter here are
		 * - Z1VEC, determines where TLB0 will point. TLB0 is written with a
		 *   value at startup that 4K aligned to this address.
		 * - LowPower_Sleep() will save a interrupt context so we will return
		 *   intact.
		 * - For devices with little RAM we don't want to impose the alignment
		 *   requirements there. Almost as we have to occupy a 4K block for this..
		 *   although the code does not take that much space.
		 * */
		McuE_EnterLowPower(mcuMode);

	    /* Clear sleep flags to allow pads to operate */
	    CRP.PSCR.B.SLEEPF = 0x1;
	}
#else
	/* NOT SUPPORTED */
	(void) mcuMode;
#endif
}

void Mcu_Arc_SetModePost( Mcu_ModeType mcuMode)
{


}
}



