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


#if defined(CFG_MPC5744P)

/* IMPROVEMENT: This can actually be read from the flash instead */
const FlashType flashInfo[] = {
    /* NO RWW */

    /* Bank 0, Array 0 (LOW) */
    [0].sectCnt = 21,
    [0].bankSize = 0xA00000,
//	[0].bankRange = BANK_RANGE_CODE_LOW,
    [0].regBase = 0xFFFE0000UL,
    [0].sectAddr[0] = 0x800000, /* 0x800000, B0F0, LOW  */
    [0].addrSpace[0] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 0,
    [0].sectAddr[1] = 0x804000, /* 1, B0F1, LOW */
    [0].addrSpace[1] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 1,
    [0].sectAddr[2] = 0x808000, /* 2, B0F2, LOW */
    [0].addrSpace[2] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 0,
    [0].sectAddr[3] = 0x810000, /* 3, B0F3, LOW */
    [0].addrSpace[3] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 1,
    [0].sectAddr[4] = 0x818000, /* end first chunk */

    [0].sectAddr[5] = 0x0F98000, /* 5, B0F4, MID */
    [0].addrSpace[5] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 2,
    [0].sectAddr[6] = 0x0F9C000, /* 6, B0F5, MID */
    [0].addrSpace[6] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 3,
    [0].sectAddr[7] = 0x0FA0000,	/* 7, B0F6, HIGH */
    [0].addrSpace[7] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 0,
    [0].sectAddr[8] = 0x0FB0000, /* 8, B0F7, HIGH */
    [0].addrSpace[8] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 1,
    [0].sectAddr[9] = 0x0FC0000, /* 9, B0F8, HIGH */
    [0].addrSpace[9] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 2,
    [0].sectAddr[10] = 0x0FD0000, /* 10, B0F9, HIGH */
    [0].addrSpace[10] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 3,
    [0].sectAddr[11] = 0x0FE0000, /* 11, B0F10, HIGH */
    [0].addrSpace[11] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 4,
    [0].sectAddr[12] = 0x0FF0000, /* 12, B0F11, HIGH */
    [0].addrSpace[12] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 5,
    [0].sectAddr[13] = 0x1000000, /* 13, B0F12, HIGH */
    [0].addrSpace[13] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 0,
    [0].sectAddr[14] = 0x1040000, /* 14, B0F13, HIGH */
    [0].addrSpace[14] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 1,
    [0].sectAddr[15] = 0x1080000, /* 15, B0F14, HIGH */
    [0].addrSpace[15] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 2,
    [0].sectAddr[16] = 0x10C0000, /* 16, B0F15, HIGH */
    [0].addrSpace[16] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 3,
    [0].sectAddr[17] = 0x1100000, /* 17, B0F16, HIGH */
    [0].addrSpace[17] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 4,
    [0].sectAddr[18] = 0x1140000, /* 18, B0F17, HIGH */
    [0].addrSpace[18] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 5,
    [0].sectAddr[19] = 0x1180000, /* 19, B0F18, HIGH */
    [0].addrSpace[19] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 6,
    [0].sectAddr[20] = 0x11C0000, /* 20, B0F19, HIGH */
    [0].addrSpace[20] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 7,
    [0].sectAddr[21] = 0x1200000, /* end of flash */
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
