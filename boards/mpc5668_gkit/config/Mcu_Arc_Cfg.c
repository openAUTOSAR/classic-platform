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

#include "Std_Types.h"
#include "Mcu.h"
#include "Mcu_Arc.h"
#include "asm_ppc.h"
#include "mm.h"
#include "mpc55xx.h"
#include "io.h"
#include "isr.h"
#include "EcuM.h"

#if defined(USE_DMA)
#include "Dma.h"
#endif
#include <assert.h>

#if defined(CFG_MCU_ARC_DEBUG)
#include "Ramlog.h"
#define LOG_HEX1(_str,_arg1) \
	ramlog_str(_str);ramlog_hex(_arg1);ramlog_str("\n")

#define LOG_HEX2(_str,_arg1,_str2,_arg2) \
	ramlog_str(_str);ramlog_hex(_arg1);ramlog_str(_str2);ramlog_hex(_arg2);ramlog_str("\n")

#define LOG_STR(_str) 	ramlog_str(_str)
#else
#define LOG_HEX1(_str,_arg1)
#define LOG_HEX2(_str,_arg1,_str2,_arg2)
#define LOG_STR(_str)
#endif

#if defined(CFG_VLE)
#define ADJUST_VECTOR	1
#else
#define ADJUST_VECTOR	0
#endif


struct TlbEntry TlbTable[]  = {
	// TLB Entry 0 =  1M Internal flash
	{
			0,
			(0x10000000 + (0<<16)),
			(0xC0000000 + MAS1_TSIZE_4M),
			(CFG_FLASH_START + VLE_MAS2_VAL),
			(CFG_FLASH_START + MAS3_FULL_ACCESS),
	},
	// TLB Entry 1 =  Peripheral bridge and BAM
	{
			1,
			(0x10000000 + (1<<16)),
			(0xC0000000 + MAS1_TSIZE_1M),
			(CFG_PERIPHERAL_START + VLE_MAS2_VAL +  MAS2_I),
			(CFG_PERIPHERAL_START + MAS3_FULL_ACCESS),
	},
	// TLB Entry 2 =  External RAM.
	{
			2,
			(0x10000000 + (2<<16)),
			(0xC0000000 + MAS1_TSIZE_4M),
			(CFG_EXT_RAM_START + VLE_MAS2_VAL),
			(CFG_EXT_RAM_START + MAS3_FULL_ACCESS),
	},
	// TLB Entry 3 =  Internal SRAM
	{
			3,
			(0x10000000+(3<<16)),
			(0xC0000000 + MAS1_TSIZE_256K),
			(CFG_SRAM_START + VLE_MAS2_VAL + MAS2_I),
			(CFG_SRAM_START + MAS3_FULL_ACCESS),
	},
	// TLB Entry 4 =  Peripheral bridge B
	{
			4,
			(0x10000000+(4<<16)),
			(0xC0000000 + MAS1_TSIZE_1M),
			(CFG_PERIPHERAL_B_START + VLE_MAS2_VAL + MAS2_I),
			(CFG_PERIPHERAL_B_START + MAS3_FULL_ACCESS),
	},
	{
			(-1UL)
	}
};


/*
 * Configuration only support:
 * - Flash recovery only
 * - RECPTR is used to hold stack pointer when getting back from sleep
 *
 *
  */
struct Mcu_Arc_SleepPrivData sleepPrivData;



const Mcu_Arc_SleepConfigType Mcu_Arc_SleepConfig =  {
	/* Run all */
	.hlt0_run   = 0x0UL,
	.hlt1_run   = 0x0UL,

	/* Halt all but reserved bits */
	.hlt0_sleep = 0x037FFF3DUL,
	.hlt1_sleep = 0x18000F3CUL,

	/* Goto sleep, enable all RAM
	 *     0x1 32k, 0x2 64k, 0x3 128k */
	.crp_pscr = PSCR_SLEEP | PSCR_SLP12EN | PCSR_RAMSEL(0x3),

	/* Point to recovery routine. If VLE is used this must be indicated */
	.z6vec = ((uint32)&Mcu_Arc_LowPowerRecoverFlash ),
	/* Not using Z0 so keep in reset */
	.z0vec = 2,
	.sleepSysClk = 0,	/* 0 - 16Mhz IRC , 1 - XOSC , 2 - PLL */
	.sleepFlags = 0,
	.pData = &sleepPrivData,
};

const struct Mcu_Arc_Config Mcu_Arc_ConfigData = {
		.sleepConfig = &Mcu_Arc_SleepConfig,
		.tblTable = TlbTable,
};


/**
 * @param sleepCfg
 */ 
void Mcu_Arc_SetModePre2( Mcu_ModeType mcuMode, const struct Mcu_Arc_SleepConfig *sleepCfg ) {
	uint32_t timeout = 0;

	if( mcuMode == MCU_MODE_NORMAL ) {
		mcuMode = MCU_MODE_RUN;
	}

	if( MCU_MODE_RUN == mcuMode ) {
		SIU.HLT0.R = sleepCfg->hlt0_run;
		SIU.HLT1.R = sleepCfg->hlt1_run;
	} else if( MCU_MODE_SLEEP == mcuMode  ) {

#if defined(USE_DMA)
		Dma_DeInit();
#endif

		LOG_HEX1("CRP Sleep clock select: ", sleepCfg->sleepSysClk);

		/* Set system clock to 16Mhz IRC */
		SIU.SYSCLK.B.SYSCLKSEL = sleepCfg->sleepSysClk;

		if( sleepCfg->sleepFlags & SLEEP_FLG_POWERDOWN_FLASH ) {
			/* Set MCR[STOP]  (should work if FASTREC if OFF */
			SET32( 0xffff8000UL + 0, (1<<(31-25)) );
		}

		Irq_Disable();

		/* Clear FLAGS first */
		LOG_STR("CRP: Clearing wakeup flags\n");
		SET32(CRP_PSCR, 0xc7ff0000UL );
		SET32(CRP_RTCSC, (1<<(31-2)) );	/* Clear RTCF */

		/* Write Sleep config */
		WRITE32(CRP_PSCR, sleepCfg->crp_pscr);

		LOG_HEX1("CRP: Z6VEC: ", sleepCfg->z6vec );
		WRITE32(CRP_Z1VEC, sleepCfg->z6vec | ADJUST_VECTOR );
		LOG_HEX1("CRP: Z0VEC: ", sleepCfg->z0vec );
		WRITE32(CRP_Z0VEC, sleepCfg->z0vec | ADJUST_VECTOR );

		assert( sleepCfg->pData != NULL );

		LOG_HEX1("HLT: ", sleepCfg->hlt0_sleep );
		sleepCfg->pData->hlt0 = SIU.HLT0.R;
		sleepCfg->pData->hlt1 = SIU.HLT1.R;

		// sleepCfg->pData->swt_cr = MCM.SWTCR.R;

		SIU.HLT0.R = sleepCfg->hlt0_sleep;
		SIU.HLT1.R = sleepCfg->hlt1_sleep;

		while((SIU.HLTACK0.R != sleepCfg->hlt0_sleep) && (SIU.HLTACK1.R != sleepCfg->hlt1_sleep) && (timeout<HLT_TIMEOUT)){}

		Mcu_Arc_EnterLowPower(mcuMode);
		/* back from sleep */

		/* Setup exceptions and INTC again */
		Os_IsrInit();

		/* Restore watchdog */
	 	SWT.SR.R = 0x0000c520;     /* Write keys to clear soft lock bit */
	 	SWT.SR.R = 0x0000d928;
	 	SWT.CR.R = 0x8000010A;

#if defined(USE_ECUM)
		EcuM_CheckWakeup( 0x3fffffffUL );
#endif

		/* Clear sleep flags to allow pads to operate */
	    CRP.PSCR.B.SLEEPF = 0x1;
	}
}


/**
 * @param sleepCfg
 */
void Mcu_Arc_SetModePost2( Mcu_ModeType mcuMode, const struct Mcu_Arc_SleepConfig *sleepCfg ) {

	if( mcuMode == MCU_MODE_NORMAL ) {
		mcuMode = MCU_MODE_RUN;
	}

	if( MCU_MODE_RUN == mcuMode ) {

	} else if( MCU_MODE_SLEEP == mcuMode  ) {
	}
}



