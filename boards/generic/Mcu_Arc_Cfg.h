/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/

#ifndef MCU_ARC_CFG_H_
#define MCU_ARC_CFG_H_

#if !defined(_ASSEMBLER_)

struct Mcu_Arc_SleepPrivData {
	uint32_t dummy;
};

typedef struct Mcu_Arc_SleepConfig {
	uint32_t dummy;
	struct Mcu_Arc_SleepPrivData *pData;
} Mcu_Arc_SleepConfigType;

typedef struct Mcu_Arc_Config {
	const struct Mcu_Arc_SleepConfig 	*sleepConfig;
	const struct TlbEntry 				*tblTable;
} Mcu_Arc_ConfigType;


extern const struct Mcu_Arc_Config Mcu_Arc_ConfigData;
#endif

#endif /* MCU_ARC_CFG_H_ */
