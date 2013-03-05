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
