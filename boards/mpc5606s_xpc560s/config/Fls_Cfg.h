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

/** @addtogroup Fls Flash Driver
 *  @{ */

/** @file Fls_Cfg.h
 *  Definitions of configuration parameters for Flash Driver.
 */



#ifndef FLS_CFG_H_
#define FLS_CFG_H_

#define USE_FLS_INFO		STD_ON


/* STD container : Fls
 * FlsConfigSet 			1..*
 * FlsGeneral				1
 * FlsPublishedInformation 	1
 */

#include "MemIf_Types.h"

/* FlsGeneral, 3.0 OK */
#define FLS_VARIANT_PB				STD_OFF
#define FLS_AC_LOAD_ON_JOB_START 	STD_OFF			/* NO SUPPORT */
#define FLS_BASE_ADDRESS 			0x00000000
#define FLS_CANCEL_API				STD_OFF			/* NO SUPPORT */
#define FLS_COMPARE_API				STD_ON
#define FLS_DEV_ERROR_DETECT		STD_ON
#define FLS_DRIVER_INDEX			0				/* NO SUPPORT */
#define FLS_GET_JOB_RESULT_API		STD_ON
#define FLS_GET_STATUS_API			STD_ON
#define FLS_SET_MODE_API			STD_OFF			/* NO SUPPORT */
//#define FLS_TOTAL_SIZE
#define FLS_USE_INTERRUPTS			STD_OFF			/* NO SUPPORT */
#define FLS_VERSION_INFO_API		STD_ON

/* FlsPublishedInformation, 3.0 OK */
#define FLS_AC_LOCATION_ERASE		0 				/* NO SUPPORT */
#define FLS_AC_LOCATION_WRITE		0				/* NO SUPPORT */
#define FLS_AC_SIZE_ERASE			0				/* NO SUPPORT */
#define FLS_AC_SIZE_WRITE			0				/* NO SUPPORT */
#define FLS_ERASE_TIME				0				/* NO SUPPORT */
#define FLS_ERASED_VALUE			0xff			/* NO SUPPORT */
#define FLS_EXPECTED_HW_ID			0				/* NO SUPPORT */
#define FLS_SPECIFIED_ERASE_CYCLES	0				/* NO SUPPORT */
#define FLS_WRITE_TIME				0				/* NO SUPPORT */


/* MCU Specific */
#if defined(CFG_MPC5606S)

#define FLASH_BANK_CNT 				3
#define FLASH_PAGE_SIZE				8
#define FLASH_MAX_SECTORS			16
#define FLS_TOTAL_SIZE              ((16*4+512+512)*1024)

#else
#error CPU not supported
#endif

#if (USE_FLS_INFO==STD_ON)

typedef struct Flash {
    uint32_t size;
    uint32_t sectCnt;
    uint32_t bankSize;
    uint32_t regBase;
    uint32_t sectAddr[FLASH_MAX_SECTORS+1];
    uint16_t addrSpace[FLASH_MAX_SECTORS+1];
} FlashType;


#else
typedef struct {
  Fls_LengthType FlsNumberOfSectors;
  Fls_LengthType FlsPageSize;
  Fls_LengthType FlsSectorSize;
  Fls_AddressType FlsSectorStartaddress;
} Fls_SectorType;
#endif


struct Flash;

typedef struct {
	void (*FlsAcErase)();					/* NO SUPPORT */
	void (*FlsAcWrite)();					/* NO SUPPORT */
	// FlsCallCycle N/A in core.
	void (*FlsJobEndNotification)();
	void (*FlsJobErrorNotification)();
	uint32_t FlsMaxReadFastMode;				/* NO SUPPORT */
	uint32_t FlsMaxReadNormalMode;			/* NO SUPPORT */
	uint32_t FlsMaxWriteFastMode;				/* NO SUPPORT */
	uint32_t FlsMaxWriteNormalMode;			/* NO SUPPORT */
	uint32_t FlsProtection;					/* NO SUPPORT */
#if (USE_FLS_INFO==STD_ON)
	const struct Flash *FlsInfo;
#else
	const Fls_SectorType *FlsSectorList;
#endif
//	const uint32 FlsSectorListSize;			/* NO SUPPORT */
} Fls_ConfigSetType;

typedef Fls_ConfigSetType Fls_ConfigType;

extern const Fls_ConfigSetType FlsConfigSet[];

#endif /*FLS_CFG_H_*/
/** @} */
