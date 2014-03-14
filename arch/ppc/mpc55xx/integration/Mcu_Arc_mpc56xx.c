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
#include "EcuM.h"

/* ----------------------------[private define]------------------------------*/

#define MODE_RESET		0
#define MODE_STANDBY 	0xd

/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/

#if defined(USE_FLS)
extern uint32 EccErrReg;
#endif


/* ----------------------------[private functions]---------------------------*/

#if defined(CFG_MCU_ARC_LP)

static void enterStandby( void ) {

	/* ME_PCTL_x MUST be off */


	/* Configure STANDBY0 Mode for lowest consumption (only WUP Unit ON) */
	/* Please note, WKPU (Wakeup Unit) is always enabled */
	/* To generate an interrupt event triggered by a wakeup line, it is */
	/* necessary to enable WKPU */

	RGM.STDBY.B.BOOT_FROM_BKP_RAM = 1;
	ME.MER.B.STANDBY0 = 1;
	PCU.PCONF[2].B.STBY0 = 1;		/* Enable 32K RAM in stdby */

	/* Turn off ALL ME_PCTL
	 * All ME_PCTL point to LP config ME_LP_PC0 (LP_CFG=0)
	 */
	ME.LPPC[0].R = 0;
	ME.RUNPC[0].R = 0xfe;
	ME.RUNPC[1].R = 0xfe;


	/* Errata e3247, must write FRZ to all CAN devices. How do I know
	 * what ones to disable?
	 * */
	CAN_0.MCR.B.FRZ = 1;
	CAN_1.MCR.B.FRZ = 1;
	CAN_2.MCR.B.FRZ = 1;
	CAN_3.MCR.B.FRZ = 1;
	CAN_4.MCR.B.FRZ = 1;
	CAN_5.MCR.B.FRZ = 1;

	ME.STANDBY0.B.PDO=1;	  /* Keep output state of the Pad Power Driver */
	ME.STANDBY0.B.MVRON=0;	  /* Main Voltage regulator is off */
	ME.STANDBY0.B.DFLAON=1;   /* Data flash is in power-down mode */
	ME.STANDBY0.B.CFLAON=1;   /* Code flash is in power-down mode */
	ME.STANDBY0.B.FXOSC0ON=0;    /* Fast external crystal oscillator (4-16 MHz) off */
	ME.STANDBY0.B.FMPLLON=0;    /* FMPLL is off (default status during STOP0) */
	ME.STANDBY0.B.SYSCLK=0xF; /* System clock disabled BEFORE RC switching off */
	ME.STANDBY0.B.FIRCON=0;     /* Fast internal RC oscillator (16 MHz) off */


	/* Enter STANDBY Mode */
	ME.MCTL.R = ((vuint32_t)MODE_STANDBY<<28) | 0x00005AF0; /* Mode & Key */
	ME.MCTL.R = ((vuint32_t)MODE_STANDBY<<28) | 0x0000A50F; /* Mode & Key */

	/* Wait to enter sleep and come back in either flash or RAM */
	 while(1 == ME.GS.B.S_MTRANS)
	    {
	    }

	/* From "MC_ME Mode Diagram", page 136 in RM,  it seems as e return to DRUN after exit of STANDBY0 */
}
#endif

/* ----------------------------[public functions]----------------------------*/



void Mcu_Arc_InitPre( const Mcu_ConfigType *configPtr ) {
	(void)configPtr;
}

void Mcu_Arc_InitPost( const Mcu_ConfigType *configPtr ) {
	(void)configPtr;
}

void Mcu_Arc_InitClockPre( const Mcu_ClockSettingConfigType *clockSettingsPtr )
{
#if defined(CFG_MPC5604B) || defined(CFG_MPC5606B) || defined (CFG_MPC5602B)
    // Write pll parameters.
    CGM.FMPLL_CR.B.IDF = clockSettingsPtr->Pll1;
    CGM.FMPLL_CR.B.NDIV = clockSettingsPtr->Pll2;
    CGM.FMPLL_CR.B.ODF = clockSettingsPtr->Pll3;

    /* RUN0 cfg: 16MHzIRCON,OSC0ON,PLL0ON,syclk=PLL0 */
    ME.RUN[0].R = 0x001F0074;
    /* Peri. Cfg. 1 settings: only run in RUN0 mode */
    ME.RUNPC[1].R = 0x00000010;
    /* MPC56xxB/S: select ME.RUNPC[1] */
    ME.PCTL[68].R = 0x01; //SIUL control
    ME.PCTL[69].R = 0x01; //WKUP control
    ME.PCTL[91].R = 0x01; //RTC/API control
    ME.PCTL[92].R = 0x01; //PIT_RTI control
    ME.PCTL[72].R = 0x01; //eMIOS0 control
    ME.PCTL[73].R = 0x01; //eMIOS1 control
    ME.PCTL[16].R = 0x01; //FlexCAN0 control
    ME.PCTL[17].R = 0x01; //FlexCAN1 control
    ME.PCTL[4].R = 0x01;  /* MPC56xxB/P/S DSPI0  */
    ME.PCTL[5].R = 0x01;  /* MPC56xxB/P/S DSPI1:  */
    ME.PCTL[32].R = 0x01; //ADC0 control
#if defined(CFG_MPC5606B)
    ME.PCTL[33].R = 0x01; //ADC1 control
#endif
    ME.PCTL[23].R = 0x01; //DMAMUX control
    ME.PCTL[48].R = 0x01; /* MPC56xxB/P/S LINFlex  */
    ME.PCTL[49].R = 0x01; /* MPC56xxB/P/S LINFlex  */

    /* Mode Transition to enter RUN0 mode: */
    /* Enter RUN0 Mode & Key */
    ME.MCTL.R = 0x40005AF0;
    /* Enter RUN0 Mode & Inverted Key */
    ME.MCTL.R = 0x4000A50F;

    /* Wait for mode transition to complete */
    while (ME.GS.B.S_MTRANS) {}
    /* Verify RUN0 is the current mode */
    while(ME.GS.B.S_CURRENTMODE != 4) {}

    CGM.SC_DC[0].R = 0x80; /* MPC56xxB/S: Enable peri set 1 sysclk divided by 1 */
    CGM.SC_DC[1].R = 0x80; /* MPC56xxB/S: Enable peri set 2 sysclk divided by 1 */
    CGM.SC_DC[2].R = 0x80; /* MPC56xxB/S: Enable peri set 3 sysclk divided by 1 */

    SIU.PSMI[0].R = 0x01; /* CAN1RX on PCR43 */
    SIU.PSMI[6].R = 0x01; /* CS0/DSPI_0 on PCR15 */

#elif defined(CFG_MPC5606S)
    // Write pll parameters.
    CGM.FMPLL[0].CR.B.IDF = clockSettingsPtr->Pll1;
    CGM.FMPLL[0].CR.B.NDIV = clockSettingsPtr->Pll2;
    CGM.FMPLL[0].CR.B.ODF = clockSettingsPtr->Pll3;

    /* RUN0 cfg: 16MHzIRCON,OSC0ON,PLL0ON,syclk=PLL0 */
    ME.RUN[0].R = 0x001F0074;
    /* Peri. Cfg. 1 settings: only run in RUN0 mode */
    ME.RUNPC[1].R = 0x00000010;
    /* MPC56xxB/S: select ME.RUNPC[1] */
    ME.PCTL[68].R = 0x01; //SIUL control
    ME.PCTL[91].R = 0x01; //RTC/API control
    ME.PCTL[92].R = 0x01; //PIT_RTI control
    ME.PCTL[72].R = 0x01; //eMIOS0 control
    ME.PCTL[73].R = 0x01; //eMIOS1 control
    ME.PCTL[16].R = 0x01; //FlexCAN0 control
    ME.PCTL[17].R = 0x01; //FlexCAN1 control
    ME.PCTL[4].R = 0x01;  /* MPC56xxB/P/S DSPI0  */
    ME.PCTL[5].R = 0x01;  /* MPC56xxB/P/S DSPI1:  */
    ME.PCTL[32].R = 0x01; //ADC0 control
    ME.PCTL[23].R = 0x01; //DMAMUX control
    ME.PCTL[48].R = 0x01; /* MPC56xxB/P/S LINFlex  */
    ME.PCTL[49].R = 0x01; /* MPC56xxB/P/S LINFlex  */
    /* Mode Transition to enter RUN0 mode: */
    /* Enter RUN0 Mode & Key */
    ME.MCTL.R = 0x40005AF0;
    /* Enter RUN0 Mode & Inverted Key */
    ME.MCTL.R = 0x4000A50F;

    /* Wait for mode transition to complete */
    while (ME.GS.B.S_MTRANS) {}
    /* Verify RUN0 is the current mode */
    while(ME.GS.B.S_CURRENTMODE != 4) {}

    CGM.SC_DC[0].R = 0x80; /* MPC56xxB/S: Enable peri set 1 sysclk divided by 1 */
    CGM.SC_DC[1].R = 0x80; /* MPC56xxB/S: Enable peri set 2 sysclk divided by 1 */
    CGM.SC_DC[2].R = 0x80; /* MPC56xxB/S: Enable peri set 3 sysclk divided by 1 */

#elif defined(CFG_MPC5604P)
    // Write pll parameters.
    CGM.FMPLL[0].CR.B.IDF = clockSettingsPtr->Pll1;
    CGM.FMPLL[0].CR.B.NDIV = clockSettingsPtr->Pll2;
    CGM.FMPLL[0].CR.B.ODF = clockSettingsPtr->Pll3;
    // PLL1 must be higher than 120MHz for PWM to work */
    CGM.FMPLL[1].CR.B.IDF = clockSettingsPtr->Pll1_1;
    CGM.FMPLL[1].CR.B.NDIV = clockSettingsPtr->Pll2_1;
    CGM.FMPLL[1].CR.B.ODF = clockSettingsPtr->Pll3_1;

    /* RUN0 cfg: 16MHzIRCON,OSC0ON,PLL0ON, PLL1ON,syclk=PLL0 */
  	ME.RUN[0].R = 0x001F00F4;
    /* Peri. Cfg. 1 settings: only run in RUN0 mode */
    ME.RUNPC[1].R = 0x00000010;

    /* MPC56xxB/S: select ME.RUNPC[1] */
    ME.PCTL[68].R = 0x01; //SIUL control
    ME.PCTL[92].R = 0x01; //PIT_RTI control
    ME.PCTL[41].R = 0x01; //flexpwm0 control
    ME.PCTL[16].R = 0x01; //FlexCAN0 control
    ME.PCTL[26].R = 0x01; //FlexCAN1(SafetyPort) control
    ME.PCTL[4].R = 0x01;  /* MPC56xxB/P/S DSPI0  */
    ME.PCTL[5].R = 0x01;  /* MPC56xxB/P/S DSPI1:  */
    ME.PCTL[6].R = 0x01;  /* MPC56xxB/P/S DSPI2  */
    ME.PCTL[7].R = 0x01;  /* MPC56xxB/P/S DSPI3:  */
    ME.PCTL[32].R = 0x01; //ADC0 control
    ME.PCTL[33].R = 0x01; //ADC1 control
    ME.PCTL[48].R = 0x01; /* MPC56xxB/P/S LINFlex  */
    ME.PCTL[49].R = 0x01; /* MPC56xxB/P/S LINFlex  */
    /* Mode Transition to enter RUN0 mode: */
    /* Enter RUN0 Mode & Key */
    ME.MCTL.R = 0x40005AF0;
    /* Enter RUN0 Mode & Inverted Key */
    ME.MCTL.R = 0x4000A50F;

    /* Wait for mode transition to complete */
    while (ME.GS.B.S_MTRANS) {}
    /* Verify RUN0 is the current mode */
    while(ME.GS.B.S_CURRENTMODE != 4) {}

    /* Pwm, adc, etimer clock */
    CGM.AC0SC.R = 0x05000000;  /* MPC56xxP: Select FMPLL1 for aux clk 0  */
    CGM.AC0DC.R = 0x80000000;  /* MPC56xxP: Enable aux clk 0 div by 1 */

    /* Safety port clock */
    CGM.AC2SC.R = 0x04000000;  /* MPC56xxP: Select FMPLL0 for aux clk 2  */
    CGM.AC2DC.R = 0x80000000;  /* MPC56xxP: Enable aux clk 2 div by 1 */
#endif

}

void Mcu_Arc_InitClockPost( const Mcu_ClockSettingConfigType *clockSettingsPtr )
{
	(void)clockSettingsPtr;
}

#if defined(CFG_MCU_ARC_LP)

extern char __LP_TEXT_ROM[];
extern char __LP_TEXT_START[];
extern char __LP_TEXT_END[];

/* Context save area */
uint8_t context[32+5];

/**
 *
 * @param mcuMode The mcuMode from Mcu_SetMode()
 */
void Mcu_Arc_SetModePre( Mcu_ModeType mcuMode)
{
	uint32_t timeout = 0;
#if defined(CFG_ECUM_VIRTUAL_SOURCES)
	EcuM_WakeupSourceType pendWakeup;
#endif

	if( mcuMode == MCU_MODE_NORMAL ) {
		mcuMode = MCU_MODE_RUN;
	}

	if( MCU_MODE_RUN == mcuMode ) {

	} else if( MCU_MODE_SLEEP == mcuMode  ) {

		memcpy( __LP_TEXT_START, __LP_TEXT_ROM, __LP_TEXT_END - __LP_TEXT_START );


#if defined(CFG_ECUM_VIRTUAL_SOURCES)

		do {
			/* Go to sleep */
			if( Mcu_Arc_setjmp(context) == 0 ) {
				enterStandby();
			}

			/* Back from sleep!
			 * Now running in DRUN and on FIRC (16Mhz)
			 */

#if defined(USE_ECUM)
		EcuM_CheckWakeup( 0x3fffffffUL );
#endif

			pendWakeup = EcuM_GetPendingWakeupEvents();
			if( pendWakeup & (ECUM_WKSOURCE_WAKEUPSOURCEPIN | ECUM_WKSOURCE_WAKEUPSOURCEVIRTUALPIN) ) {
				break;
			}
		} while( pendWakeup & ECUM_WKSOURCE_WAKEUPSOURCERTC );
#else
		/* Go to sleep */
		if( Mcu_Arc_setjmp(context) == 0 ) {
			enterStandby();
		}

		/* Back from sleep!
		 * Now running in DRUN and on FIRC (16Mhz)
		 */

#if defined(USE_ECUM)
	EcuM_CheckWakeup( 0x3fffffffUL );
#endif
#endif

		/* Wdg OFF */
		SWT.SR.R = 0x0000c520;     /* Write keys to clear soft lock bit */
		SWT.SR.R = 0x0000d928;
		SWT.CR.R = 0x8000010A;

		/* Setup exceptions and INTC again */
		Os_IsrInit();


	}	/* MCU_MODE_SLEEP == mcuMode */


#if defined(CFG_MCU_ARC_CONFIG)
	Mcu_Arc_SetModePre2(mcuMode, Mcu_Arc_ConfigData.sleepConfig );
#endif
}

/**
 *
 * @param mcuMode The mcuMode from Mcu_SetMode()
 */
void Mcu_Arc_SetModePost( Mcu_ModeType mcuMode)
{
#if defined(CFG_MCU_ARC_CONFIG)
	Mcu_Arc_SetModePost2(mcuMode,  Mcu_Arc_ConfigData.sleepConfig);
#endif
}
#endif


