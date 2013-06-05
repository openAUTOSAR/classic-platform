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

#ifndef MCU_ARC_CFG_H_
#define MCU_ARC_CFG_H_


#define CFG_SRAM_START			0x40000000
#define CFG_FLASH_START			0x00000000
#define CFG_PERIPHERAL_START	0xfff00000
#define CFG_EXT_RAM_START		0x20000000
#define CFG_PERIPHERAL_B_START	0xc3f00000

/* Maximum time to wait before timout of HLT */
#define HLT_TIMEOUT				3000

#if !defined(_ASSEMBLER_)

#include "mm.h"

/* Set MCR[STOP] Flash block
 * (this assumes that you are running from RAM (NOT SUPPORTED */
#define SLEEP_FLG_POWERDOWN_FLASH 	(1<<0)


struct Mcu_Arc_SleepPrivData {
	uint32_t hlt0;
	uint32_t hlt1;
	uint32_t swt_cr;
};


typedef struct Mcu_Arc_SleepConfig {
	/* HLT bits to set when in entering RUN state */
	uint32_t hlt0_run;
	uint32_t hlt1_run;

	/* HLT bits to set when in entering SLEEP state */
	uint32_t hlt0_sleep;
	uint32_t hlt1_sleep;

	uint32_t crp_pscr;
	uint32_t crp_recptr;

	uint32_t z0vec;
	uint32_t z6vec;

	/* Register value of SIU.SYSCLK.B.SYSCLKSEL */
	uint32_t sleepSysClk;

	uint32_t sleepFlags;

	struct Mcu_Arc_SleepPrivData *pData;

} Mcu_Arc_SleepConfigType;


typedef struct Mcu_Arc_Config {
	const struct Mcu_Arc_SleepConfig 	*sleepConfig;
	const struct TlbEntry 				*tblTable;
} Mcu_Arc_ConfigType;


extern const struct Mcu_Arc_Config Mcu_Arc_ConfigData;

#endif


#endif /* MCU_ARC_CFG_H_ */
