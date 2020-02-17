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


#if defined(CFG_MPC5644A)

/* IMPROVEMENT: This can actually be read from the flash instead */
const FlashType flashInfo[] = {
    /* NO RWW */

    /* Bank 0, Array 0 (LOW + MID)  */
    {
    .sectCnt = 12,
    .bankSize = 0x80000,
    .regBase = 0xC3F88000UL,
    .sectAddr[0]   = 0, 	         /* 0, B0F0, LOW  */
    .addrSpace[0]  = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 0,
    .sectAddr[1]   = 0x04000, /* 1, B0F1, LOW */
    .addrSpace[1]  = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 1,
    .sectAddr[2]   = 0x08000, /* 2, B0F2, LOW */
    .addrSpace[2]  = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 2,
    .sectAddr[3]   = 0x0C000, /* 3, B0F3, LOW */
    .addrSpace[3]  = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 3,
    .sectAddr[4]   = 0x10000, /* 4, B0F4, LOW */
    .addrSpace[4]  = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 4,
    .sectAddr[5]   = 0x14000, /* 5, B0F5, LOW */
    .addrSpace[5]  = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 5,
    .sectAddr[6]   = 0x18000, /* 6, B0F6, LOW */
    .addrSpace[6]  = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 6,
    .sectAddr[7]   = 0x1C000, /* 7, B0F7, LOW */
    .addrSpace[7]  = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 7,
    .sectAddr[8]   = 0x20000, /* 8, B0F8, LOW */
    .addrSpace[8]  = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 8,
    .sectAddr[9]   = 0x30000, /* 9, B0F9, LOW */
    .addrSpace[9]  = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 9,
    .sectAddr[10]  = 0x40000, /* 10, B0F10, MID */
    .addrSpace[10] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 0,
    .sectAddr[11]  = 0x60000, /* 11, B0F11, MID */
    .addrSpace[11] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 1,
    .sectAddr[12]  = 0x80000, /* End, NOT a sector */
    },
    {
    /* Bank 0, Array 1(HIGH) */
    .sectCnt = 6,
    .bankSize = 0x400000 - 0x100000,
    .regBase =  0xC3F88000UL,
    .sectAddr[0]  = 0x100000,  /* High */
    .addrSpace[0] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 0,
    .sectAddr[1]  = 0x180000,  /* High */
    .addrSpace[1] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 1,
    .sectAddr[2]  = 0x200000,  /* High */
    .addrSpace[2] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 2,
    .sectAddr[3]  = 0x280000,  /* High */
    .addrSpace[3] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 3,
    .sectAddr[4]  = 0x300000,  /* High */
    .addrSpace[4] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 4,
    .sectAddr[5]  = 0x380000,  /* High */
    .addrSpace[5] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 5,
    .sectAddr[6]  = 0x400000, /* End, NOT a sector */
    },

    {
    /* Bank 1, Array 0(HIGH) */
    .sectCnt = 2,
    .bankSize = 0x100000-0x80000,
    .regBase = 0xC3F8C000UL,
    .sectAddr[0]  = 0x80000,  /* LOW */
    .addrSpace[0] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 0,
    .sectAddr[1]  = 0xC0000,  /* MID */
    .addrSpace[1] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 0,
    .sectAddr[2]  = 0x100000, /* End, NOT a sector */
    },


    {
    // The high address space for the controller is a mix of controller a and b. They overlap each
    // other every 16 byte. That is why this double declaration is required.
    /* Bank 1, Array 1(HIGH) */
    .sectCnt = 6,
    .bankSize = 0, //This is intended.
    .regBase =  0xC3F8C000UL,
    .sectAddr[0]  = 0x100000,  /* High */
    .addrSpace[0] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 0,
    .sectAddr[1]  = 0x180000,  /* High */
    .addrSpace[1] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 1,
    .sectAddr[2]  = 0x200000,  /* High */
    .addrSpace[2] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 2,
    .sectAddr[3]  = 0x280000,  /* High */
    .addrSpace[3] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 3,
    .sectAddr[4]  = 0x300000,  /* High */
    .addrSpace[4] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 4,
    .sectAddr[5]  = 0x380000,  /* High */
    .addrSpace[5] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 5,
    .sectAddr[6]  = 0x400000, /* End, NOT a sector */
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
