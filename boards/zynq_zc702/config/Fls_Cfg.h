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

#ifndef FLS_CFG_H_
#define FLS_CFG_H_

/** @tagSettings DEFAULT_ARCHITECTURE=ZYNQ */
/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.2 */

/** @file Fls_Cfg.h
 * Definitions of configuration parameters for Fls Driver.
 */

#if !(((FLS_SW_MAJOR_VERSION == 2) && (FLS_SW_MINOR_VERSION == 0)) )
#error Fls: Configuration file expected BSW module version to be 2.0.*
#endif

#if !(((FLS_AR_RELEASE_MAJOR_VERSION == 4) && (FLS_AR_RELEASE_MINOR_VERSION == 1)) )
#error Fls: Configuration file expected AUTOSAR version to be 4.1.*
#endif

#define USE_FLS_INFO		STD_ON

/* STD container : Fls
 * FlsConfigSet 			1..*
 * FlsGeneral				1
 * FlsPublishedInformation 	1
 */

/** @req SWS_Fls_00308 */
#include "MemIf_Types.h"
#include "QspiIf.h"
#if defined(USE_FEE)
/** @req SWS_Fls_00262 */
/** @req SWS_Fls_00263 */
#include "Fee_Cbk.h"
#endif

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
#define FLS_USE_INTERRUPTS			STD_OFF			/* NO SUPPORT */
#define FLS_VERSION_INFO_API		STD_ON

/* FlsPublishedInformation, 3.0 OK */
#define FLS_AC_LOCATION_ERASE		0 				/* NO SUPPORT */
#define FLS_AC_LOCATION_WRITE		0				/* NO SUPPORT */
#define FLS_AC_SIZE_ERASE			0				/* NO SUPPORT */
#define FLS_AC_SIZE_WRITE			0				/* NO SUPPORT */
#define FLS_ERASE_TIME				0				/* NO SUPPORT */
#define FLS_ERASED_VALUE			(uint8)0xff			/* NO SUPPORT */
#define FLS_EXPECTED_HW_ID			0				/* NO SUPPORT */
#define FLS_SPECIFIED_ERASE_CYCLES	0				/* NO SUPPORT */
#define FLS_WRITE_TIME				0				/* NO SUPPORT */

typedef struct {
    uint8 (*QspiIf_Arc_Erase)(uint32, uint32);
    uint8 (*QspiIf_Arc_Write)(uint32, uint32);
    uint8 (*QspiIf_Arc_Read)(uint32, uint32);
    uint8 (*QspiIf_Arc_Fast_Read)(uint32, uint32);
    void (*QspiIf_Arc_Init)(void);
    /** @req SWS_Fls_00109 */
    /** @req SWS_Fls_00110 */
    void (*FlsJobEndNotification)();
    void (*FlsJobErrorNotification)();
#if (USE_FLS_INFO==STD_ON)
    /** @req SWS_Fls_00355 */
    const struct Flash *FlsInfo;
#else
    const Fls_SectorType *FlsSectorList;
#endif
    uint8 *QspiIf_Arc_ReadBuffer;
    uint8 *QspiIf_Arc_WriteBuffer;
    uint32 FlsAcErase;                              /* NO SUPPORT */
    uint32 FlsAcWrite;                              /* NO SUPPORT */
    // FlsCallCycle N/A in core.
    uint32 FlsMaxReadFastMode;                      /* NO SUPPORT */
    uint32 FlsMaxReadNormalMode;                    /* NO SUPPORT */
    uint32 FlsMaxWriteFastMode;                     /* NO SUPPORT */
    uint32 FlsMaxWriteNormalMode;                   /* NO SUPPORT */
    uint32 FlsProtection;                           /* NO SUPPORT */

} Fls_ConfigSetType;
/** @req SWS_Fls_00368 */
/** @req SWS_Fls_00353 */
/** @req SWS_Fls_00354 */
typedef Fls_ConfigSetType Fls_ConfigType;

typedef struct Flash {
    uint32 FlsBaseAddress;
    uint32 FlsTotalSize;
    uint32 sectCnt;
    uint32 bankSize;
    uint32 regBase;
    uint32 sectAddr[FLASH_NUM_SECTORS];
} FlashType;

extern const Fls_ConfigType FlsConfigSet[1];

#endif /*FLS_CFG_H_*/
