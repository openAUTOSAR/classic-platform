/* -------------------------------- Arctic Core ------------------------------
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

/** @addtogroup Fls Flash Driver
 *  @{ */

/** @file Fls_Cfg.h
 *  Definitions of configuration parameters for Flash Driver.
 */

#ifndef FLS_CFG_H_
#define FLS_CFG_H_

#include "MemIf_Types.h"

#define FLS_GET_JOB_RESULT_API		STD_ON
#define FLS_GET_STATUS_API			STD_ON
#define FLS_CANCEL_API              STD_OFF
#define FLS_SET_MODE_API			STD_OFF
#define FLS_VERSION_INFO_API		STD_OFF
#define FLS_COMPARE_API				STD_ON
#define FLS_DEV_ERROR_DETECT		STD_ON

#define FLS_ERASED_VALUE			(uint8)0xff

/* MCU Specific */
#if defined(CFG_STM32F1X)
#define FLS_TOTAL_SIZE				0x40000
#define FLASH_PAGE_SIZE				2048
#define FLASH_MAX_PAGES           	16  // Main memory sectors == pages, only first 16 used (32K)

#else
#error CPU not supported
#endif


typedef struct Flash {
    uint32 size;
    uint32 sectCnt;
    uint32 sectAddr[FLASH_MAX_PAGES+1]; // Main memory sectors == pages
} FlashType;

typedef struct {
    // FlsCallCycle N/A in core.
    void (*FlsJobEndNotification)();
    void (*FlsJobErrorNotification)();
    const struct Flash *FlsInfo;
} Fls_ConfigType;

extern const Fls_ConfigType FlsConfigSet[];

#endif /*FLS_CFG_H_*/
/** @} */

