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

#include "Fls.h"
#include "flash.h"
#include <stdlib.h>


#if defined(CFG_MPC5604B)

/* IMPROVEMENT: This can actually be read from the flash instead */
const FlashType flashInfo[] = {
    /* NO RWW */
    {
        /* Bank 0, Array 0 (LOW) */
        .sectCnt = 8,
        .bankSize = 0x80000,
//        .bankRange = BANK_RANGE_CODE_LOW,
        .regBase = 0xC3F88000UL,
        .sectAddr[0] = 0,       /* 0, B0F0, LOW  */
        .addrSpace[0] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 0,
        .sectAddr[1] = 0x08000, /* 1, B0F1, LOW */
        .addrSpace[1] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 1,
        .sectAddr[2] = 0x0c000, /* 2, B0F2, LOW */
        .addrSpace[2] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 2,
        .sectAddr[3] = 0x10000, /* 3, B0F3, LOW */
        .addrSpace[3] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 3,
        .sectAddr[4] = 0x18000, /* 4, B0F4, LOW */
        .addrSpace[4] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 4,
        .sectAddr[5] = 0x20000, /* 5, B0F5, LOW */
        .addrSpace[5] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 5,
        .sectAddr[6] = 0x40000, /* 6, B0F6, MID */
        .addrSpace[6] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 0,
        .sectAddr[7] = 0x60000, /* 7, B0F7, MID */
        .addrSpace[7] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 1,
        .sectAddr[8] = 0x80000,  /* End, NOT a sector */

    },
    {
        /* Bank 1, Data */
        .sectCnt = 4,
        .bankSize = 0x810000 - 0x800000,
        .regBase = 0xC3F8C000UL,
        .sectAddr[0] = 0x800000,  /* LOW */
        .addrSpace[0] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 0,
        .sectAddr[1] = 0x804000,  /* LOW */
        .addrSpace[1] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 1,
        .sectAddr[2] = 0x808000,  /* LOW */
        .addrSpace[2] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 2,
        .sectAddr[3] = 0x80c000,  /* LOW */
        .addrSpace[3] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 3,
        .sectAddr[4] = 0x810000, /* End, NOT a sector */
    }
};

#else
#error CPU NOT supported
#endif


const Fls_ConfigType FlsConfigSet[]=
{
  {
#if ( FLS_AC_LOAD_ON_JOB_START == STD_ON)
    .FlsAcWrite = __FLS_ERASE_RAM__,
    .FlsAcErase = __FLS_WRITE_RAM__,
#else
    .FlsAcWrite = NULL,
    .FlsAcErase = NULL,
#endif
    .FlsJobEndNotification = NULL,
    .FlsJobErrorNotification = NULL,
    .FlsInfo = flashInfo,

    .FlsMaxReadFastMode = 16,
    .FlsMaxReadNormalMode = 16,
    .FlsMaxWriteFastMode = 32,
    .FlsMaxWriteNormalMode = 32,

//    .FlsSectorList = &fls_evbSectorList[0],
//    .FlsSectorListSize = sizeof(fls_evbSectorList)/sizeof(Fls_SectorType),
//    .FlsBlockToPartitionMap = Fls_BlockToPartitionMap,
  }
};
