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



#warning "This default file may only be used as an example!"

#ifndef NVM_CFG_H_
#define NVM_CFG_H_

#include "NvM_Types.h"
#include "NvM_ConfigTypes.h"

#define NVM_DEV_ERROR_DETECT			STD_ON		// Activate/Deactivate Dev Error Detection and Notification.
#define NVM_VERSION_INFO_API    		STD_ON		// Activate/Deactivate ver info API.
#define NVM_SET_RAM_BLOCK_STATUS_API	STD_ON		// Activate/Deactivate Nvm_SetRamBlockStatus() API.

#define NVM_API_CONFIG_CLASS			NVM_API_CONFIG_CLASS_1	// Class 1-3
#define NVM_COMPILED_CONFIG_ID			0						// 0..65535
#define NVM_CRC_NUM_OF_BYTES			1						// 1..65535
#define NVM_DATASET_SELECTION_BITS		2						// 0..8
#define NVM_DRV_MODE_SWITCH				STD_OFF					// OFF = SLOW, ON = FAST
#define NVM_DYNAMIC_CONFIGURATION		STD_OFF					// OFF..ON
#define NVM_JOB_PRIORIZATION			STD_OFF					// OFF..ON
#define NVM_MAX_NUMBER_OF_WRITE_RETRIES	2						// 0..7
#define NVM_POLLING_MODE				STD_OFF					// OFF..ON
#define NVM_SIZE_IMMEDIATE_JOB_QUEUE	1						// 1..255
#define NVM_SIZE_STANDARD_JOB_QUEUE		1						// 1..255

#define NVM_NUM_OF_NVRAM_BLOCKS			4		// Needs to be derived from NvM_Cfg.c


#endif /*NVM_CFG_H_*/
