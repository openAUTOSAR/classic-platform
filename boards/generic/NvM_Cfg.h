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


    
#ifndef NVM_CFG_H_
#define NVM_CFG_H_

#include "NvM_Types.h"
#include "NvM_ConfigTypes.h"

#define NVM_DEV_ERROR_DETECT			STD_ON
#define NVM_VERSION_INFO_API    		STD_ON
#define NVM_SET_RAM_BLOCK_STATUS_API	STD_OFF

#define NVM_API_CONFIG_CLASS			NVM_API_CONFIG_CLASS_1	// Class 1-3
#define NVM_COMPILED_CONFIG_ID			0						// 0..65535
#define NVM_CRC_NUM_OF_BYTES			0						// 1..65535
#define NVM_DATASET_SELECTION_BITS		0						// 0..8
#define NVM_DRV_MODE_SWITCH				STD_OFF					// OFF = SLOW, ON = FAST
#define NVM_DYNAMIC_CONFIGURATION		STD_OFF					// OFF..ON
#define NVM_JOB_PRIORIZATION			STD_OFF					// OFF..ON
#define NVM_MAX_NUMBER_OF_WRITE_RETRIES	2						// 0..7
#define NVM_POLLING_MODE				STD_OFF					// OFF..ON
#define NVM_SIZE_IMMEDIATE_JOB_QUEUE	0						// 1..255
#define NVM_SIZE_STANDARD_JOB_QUEUE		0						// 1..255

#define NVM_NUM_OF_NVRAM_BLOCKS			0


#endif /*NVM_CFG_H_*/
