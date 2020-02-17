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

#include "Fls.h"
#include "flash.h"
#include <stdlib.h>


#if defined(CFG_MPC5646B)

/*lint -e940 -e785 -e835 -e9027 -e845 -e651 To increase readability  */
const FlashType flashInfo[3] = {
    /* NO RWW */
    /* Bank 0, code flash memory */
    {
    .sectCnt = 16,
    .bankSize = 0x180000UL,
    .regBase = 0xC3F88000UL,
    .sectAddr[0] = 0, 	   /* 0, B0F0, LOW  */
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
    .sectAddr[8] = 0x80000, /* 8, B0F8, MID */
    .addrSpace[8] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 0,
    .sectAddr[9] = 0xA0000, /* 9, B0F9, HIGH */
    .addrSpace[9] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 1,
    .sectAddr[10] = 0xC0000, /* 10, B0FA, HIGH */
    .addrSpace[10] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 2,
    .sectAddr[11] = 0xE0000, /* 11, B0FB, HIGH */
    .addrSpace[11] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 3,
    .sectAddr[12] = 0x100000, /* 12, B0FC, HIGH */
    .addrSpace[12] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 4,
    .sectAddr[13] = 0x120000, /* 13, B0FD, HIGH */
    .addrSpace[13] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 5,
    .sectAddr[14] = 0x140000, /* 14, B0FE, HIGH */
    .addrSpace[14] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 6,
    .sectAddr[15] = 0x160000, /* 15, B0FF, HIGH */
    .addrSpace[15] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 7,
    .sectAddr[16] = 0x180000,	/* End, NOT a sector */
    },
     /* NO RWW */
    /* Bank 2, code flash memory */
    {
    .sectCnt = 16,
    .bankSize = 0x300000UL - 0x180000UL,
    .regBase = 0xC3FB0000UL,
    .sectAddr[0] = 0x180000, /* 0, B2F8, HIGH */
    .addrSpace[0] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 0,
    .sectAddr[1] = 0x1A0000, /* 1, B2F9, HIGH */
    .addrSpace[1] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 1,
    .sectAddr[2] = 0x1C0000, /* 2, B2FA, HIGH */
    .addrSpace[2] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 2,
    .sectAddr[3] = 0x1E0000, /* 3, B2FB, HIGH */
    .addrSpace[3] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 3,
    .sectAddr[4] = 0x200000, /* 4, B2FC, HIGH */
    .addrSpace[4] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 4,
    .sectAddr[5] = 0x220000, /* 5, B2FD, HIGH */
    .addrSpace[5] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 5,
    .sectAddr[6] = 0x240000, /* 6, B2FE, HIGH */
    .addrSpace[6] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 6,
    .sectAddr[7] = 0x260000, /* 7, B2FF, HIGH */
    .addrSpace[7] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 7,
    .sectAddr[8] = 0x280000, /* 8, B2F0, LOW  */
    .addrSpace[8] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 0,
    .sectAddr[9] = 0x288000, /* 9, B2F1, LOW */
    .addrSpace[9] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 1,
    .sectAddr[10] = 0x28C000, /* 10, B2F2, LOW */
    .addrSpace[10] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 2,
    .sectAddr[11] = 0x290000, /* 11, B2F3, LOW */
    .addrSpace[11] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 3,
    .sectAddr[12] = 0x298000, /* 12, B2F4, LOW */
    .addrSpace[12] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 4,
    .sectAddr[13] = 0x2A0000, /* 13, B2F5, LOW */
    .addrSpace[13] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 5,
    .sectAddr[14] = 0x2C0000, /* 14, B2F6, MID */
    .addrSpace[14] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 0,
    .sectAddr[15] = 0x2E0000, /* 15, B2F7, MID */
    .addrSpace[15] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 1,    
    .sectAddr[16] = 0x300000,	/* End, NOT a sector */
    },
    
    {
    /* Data flash memory */
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
  }
};
