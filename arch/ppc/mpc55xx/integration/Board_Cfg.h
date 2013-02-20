/*
 * Mcu_Arc_Cfg.h
 *
 *  Created on: 16 jan 2013
 *      Author: mahi
 */

#ifndef MCU_ARC_CFG_H_
#define MCU_ARC_CFG_H_


/*-----------------------------[ MEMORY ]-----------------------------------*/

#if defined(CFG_MPC5516) || defined(CFG_MPC5668)
#define CFG_SRAM_START			0x40000000
#define CFG_FLASH_START			0x00000000
#define CFG_PERIPHERAL_START	0xfff00000
#else
#error No support for this MCU
#endif


/*-----------------------------[ SLEEP ]------------------------------------*/

/* HLT values */
#if defined(CFG_MPC5516)
#define R_HLT0		0x3FFFFFFF
#define R_HLT1		0			/* Not used */
#elif defined(CFG_MPC5668)
#define R_HLT0		0x037FFF3D
#define R_HLT1		0x18000F3C
#endif

/* How long to wait (=loops) for the system to halt */
#define HLT_TIMEOUT			3000

/* The clock we want to sleep on */
#define SYSCLK_16MHZ_IRC		0
#define SYSCLK_XOSC				1
#define SYSCLK_PLL				2

#define R_SYSCLKSEL		SYSCLK_16MHZ_IRC

#if defined(CFG_MPC5516)
#define RAMSEL_VAL		0x7		/* 0x1 8k, 0x2 16k, 0x3 32k, 0x6 64k, 0x7 - 80K */
#elif defined(CFG_MPC5668)
#define RAMSEL_VAL		0x3		/* 0x1 32k, 0x2 64k, 0x3 128k */
#else
#error  Please define RAMSEL_VAL
#endif


/*-----------------------------[ MMU ]------------------------------------*/

// TLB Entry 0 =  1M Internal flash
#define TLB0_MAS0		(0x10000000 + (0<<16))
#define TLB0_MAS1		(0xC0000000 + MAS1_TSIZE_4M)
#define TLB0_MAS2		(FLASH_START + VLE_VAL)
#define TLB0_MAS3		(FLASH_START + MAS3_FULL_ACCESS)

// TLB Entry 1 =  Peripheral bridge and BAM
#define TLB1_MAS0		(0x10000000 + (1<<16))
#define TLB1_MAS1		(0xC0000000 + MAS1_TSIZE_1M)
#define TLB1_MAS2		(PERIPHERAL_START + VLE_VAL +  MAS2_I)
#define TLB1_MAS3		(PERIPHERAL_START + MAS3_FULL_ACCESS)

// TLB Entry 2 =  External RAM. Skip this.
//#define TLB2_MAS0		(0x10000000 + (0<<16))
#define TLB2_MAS1		(0xC0000000 + MAS1_TSIZE_4M)
#define TLB2_MAS2		(FLASH_START + VLE_VAL)
#define TLB2_MAS3		(FLASH_START + MAS3_FULL_ACCESS)

// TLB Entry 2 =  Internal SRAM
#define TLB3_MAS0		(0x10000000+(3<<16))
#define TLB3_MAS1		(0xC0000000 + MAS1_TSIZE_256K)
#define TLB3_MAS2		(SRAM_START + VLE_VAL + MAS2_I)
#define TLB3_MAS3		(SRAM_START + MAS3_FULL_ACCESS)



#endif /* MCU_ARC_CFG_H_ */
