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









#ifndef FLS_SST25XX_CFG_H_
#define FLS_SST25XX_CFG_H_

// Take in the original types


#include "Fls_Cfg.h"


extern const Fls_ConfigType FlsSST25xxConfigSet[];
/*
 * Fls General container
 */

//	The flash driver shall load the flash access code to RAM whenever an
//	erase or write job is started and unload (overwrite) it after that job has
//	been finished or canceled. true: Flash access code loaded on job start /
//	unloaded on job end or error. false: Flash access code not loaded to /
//	unloaded from RAM at all.
#define FLS_SST25XX_AC_LOAD_ON_JOB_START STD_OFF

// The flash memory start address (see also FLS118).
// FLS169: This parameter defines the lower boundary for read / write /
// erase and compare jobs.
#define FLS_SST25XX_BASE_ADDRESS 			0x00000000

//	Compile switch to enable and disable the Fls_Cancel function. true: API
//	supported / function provided. false: API not supported / function not pro-
//	vided
#define FLS_SST25XX_CANCEL_API				STD_OFF

//	Compile switch to enable and disable the Fls_Compare function. true: API
//	supported / function provided. false: API not supported / function not pro-
//	vided
#define FLS_SST25XX_COMPARE_API			STD_ON

// Pre-processor switch for enabling the development error detection and
// reporting (see FLS077).

#define FLS_SST25XX_DEV_ERROR_DETECT		STD_ON

// Index of the driver, used by FEE.
#define FLS_SST25XX_DRIVER_INDEX			100

// Compile switch to enable and disable the Fls_GetJobResult function. true:
// API supported / function provided. false: API not supported / function not
// provided
#define FLS_SST25XX_GET_JOB_RESULT_API		STD_OFF

//	Compile switch to enable and disable the Fls_GetStatus function. true: API
//	supported / function provided. false: API not supported / function not pro-
//	vided
#define FLS_SST25XX_GET_STATUS_API			STD_ON

// Compile switch to enable and disable the Fls_SetMode function. true: API
// supported / function provided. false: API not supported / function not pro-
// vided
#define FLS_SST25XX_SET_MODE_API			STD_ON

// The total amount of flash memory in bytes (see also FLS118).
// FLS170: This parameter in conjunction with FLS_SST25XX_BASE_ADDRESS
// defines the upper boundary for read / write / erase and compare jobs
#define FLS_SST25XX_TOTAL_SIZE				0x200000 // 16Mbit->2Mb, 0x0000_0000 to 0x0020_0000
#if 0
#define FLS_SST25XX_READ_PAGE_SIZE 0x8 // Read page size of 128 bits (4 words) (8 bytes)
#endif

// Job processing triggered by hardware interrupt.
// true: Job processing triggered by interrupt (hardware controlled).
// false: Job processing not triggered by interrupt (software controlled)

// NOT supported by Freescale hardware
#define FLS_SST25XX_USE_INTERRUPTS			STD_OFF

#define FLS_SST25XX_VERSION_INFO_API		STD_ON


#endif /* FLS_SST25XX_CFG_H_ */
