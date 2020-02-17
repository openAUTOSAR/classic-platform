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

#if defined(CFG_MPC5777M)

/* IMPROVEMENT: This can actually be read from the flash instead */
const FlashType flashInfo[] = {
    /* NO RWW */

    /* Bank 0, Array 0 (LOW) */
    [0].sectCnt = 54,
    [0].bankSize = 0x1174000,
    [0].regBase = 0xFFFE0000UL,
    // HSM Code—no overlay
    [0].sectAddr[0] = 0x0060C000,
    [0].addrSpace[0] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 5,
    [0].sectAddr[1] = 0x00610000,
    [0].addrSpace[1] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 2,
    [0].sectAddr[2] = 0x00620000,
    [0].addrSpace[2] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 3,
    [0].sectAddr[3] = 0x00630000, /* end first chunk */
    [0].addrSpace[3] = SECTOR_EMPTY,
    // HSM Data—no overlay
    [0].sectAddr[4] = 0x00680000, /* 0x800000, B0F0, LOW  */
    [0].addrSpace[4] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 0,
    [0].sectAddr[5] = 0x00684000,
    [0].addrSpace[5] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 1,
    [0].sectAddr[6] = 0x00688000, /* end first chunk */
    [0].addrSpace[6] = SECTOR_EMPTY,

    // Data Flash–no overlay
    [0].sectAddr[7] = 0x800000,
    [0].addrSpace[7] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 0,
    [0].sectAddr[8] = 0x810000,
    [0].addrSpace[8] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 1,
    [0].sectAddr[9] = 0x820000,
    [0].addrSpace[9] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 2,
    [0].sectAddr[10] = 0x830000,
    [0].addrSpace[10] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 3,
    [0].sectAddr[11] = 0x840000,
    [0].addrSpace[11] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 4,
    [0].sectAddr[12] = 0x850000,
    [0].addrSpace[12] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 5,
    [0].sectAddr[13] = 0x860000,
    [0].addrSpace[13] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 6,
    [0].sectAddr[14] = 0x870000,
    [0].addrSpace[14] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 7,
    [0].sectAddr[15] = 0x880000, /* end first chunk */
    [0].addrSpace[15] = SECTOR_EMPTY,
    // Low & Mid & Large Flash Blocks—no overlay
    [0].sectAddr[16] = 0x00FC0000,
    [0].addrSpace[16] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 1,
    [0].sectAddr[17] = 0x00FC4000,
    [0].addrSpace[17] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 2,
    [0].sectAddr[18] = 0x00FC8000,
    [0].addrSpace[18] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 3,
    [0].sectAddr[19] = 0x00FCC000,
    [0].addrSpace[19] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 4,
    [0].sectAddr[20] = 0x00FD0000,
    [0].addrSpace[20] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 0,
    [0].sectAddr[21] = 0x00FD8000,
    [0].addrSpace[21] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 1,
    [0].sectAddr[22] = 0x00FE0000,
    [0].addrSpace[22] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 0,
    [0].sectAddr[23] = 0x0FF0000,
    [0].addrSpace[23] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 1,
    [0].sectAddr[24] = 0x1000000,
    [0].addrSpace[24] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 0,
    [0].sectAddr[25] = 0x1040000,
    [0].addrSpace[25] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 1,
    [0].sectAddr[26] = 0x1080000,
    [0].addrSpace[26] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 2,
    [0].sectAddr[27] = 0x10C0000,
    [0].addrSpace[27] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 3,
    [0].sectAddr[28] = 0x1100000,
    [0].addrSpace[28] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 4,
    [0].sectAddr[29] = 0x1140000,
    [0].addrSpace[29] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 5,
    [0].sectAddr[30] = 0x1180000,
    [0].addrSpace[30] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 6,
    [0].sectAddr[31] = 0x11C0000,
    [0].addrSpace[31] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 7,
    [0].sectAddr[32] = 0x1200000,
    [0].addrSpace[32] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 8,
    [0].sectAddr[33] = 0x1240000,
    [0].addrSpace[33] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 9,
    [0].sectAddr[34] = 0x1280000,
    [0].addrSpace[34] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 10,
    [0].sectAddr[35] = 0x12C0000,
    [0].addrSpace[35] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 11,
    [0].sectAddr[36] = 0x1300000,
    [0].addrSpace[36] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 12,
    [0].sectAddr[37] = 0x1340000,
    [0].addrSpace[37] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 13,
    [0].sectAddr[38] = 0x1380000,
    [0].addrSpace[38] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 14,
    [0].sectAddr[39] = 0x13C0000,
    [0].addrSpace[39] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 15,
    [0].sectAddr[40] = 0x1400000,
    [0].addrSpace[40] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 16,
    [0].sectAddr[41] = 0x1440000,
    [0].addrSpace[41] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 17,
    [0].sectAddr[42] = 0x1480000,
    [0].addrSpace[42] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 18,
    [0].sectAddr[43] = 0x14C0000,
    [0].addrSpace[43] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 19,
    [0].sectAddr[44] = 0x1500000,
    [0].addrSpace[44] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 20,
    [0].sectAddr[45] = 0x1540000,
    [0].addrSpace[45] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 21,
    [0].sectAddr[46] = 0x1580000,
    [0].addrSpace[46] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 22,
    [0].sectAddr[47] = 0x15C0000,
    [0].addrSpace[47] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 23,
    [0].sectAddr[48] = 0x1600000,
    [0].addrSpace[48] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 24,
    [0].sectAddr[49] = 0x1640000,
    [0].addrSpace[49] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 25,
    [0].sectAddr[50] = 0x1680000,
    [0].addrSpace[50] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 26,
    [0].sectAddr[51] = 0x16C0000,
    [0].addrSpace[51] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 27,
    [0].sectAddr[52] = 0x1700000,
    [0].addrSpace[52] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 28,
    [0].sectAddr[53] = 0x1740000,
    [0].addrSpace[53] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 29,
    [0].sectAddr[54] = 0x1780000,
    [0].addrSpace[54] = SECTOR_EMPTY,
};

#else
#error CPU NOT supported
#endif

const Fls_ConfigType FlsConfigSet[] = { {
#if ( FLS_AC_LOAD_ON_JOB_START == STD_ON)
        .FlsAcWrite = __FLS_ERASE_RAM__,
        .FlsAcErase = __FLS_WRITE_RAM__,
#else
        .FlsAcWrite = NULL, .FlsAcErase = NULL,
#endif
        .FlsJobEndNotification = NULL, .FlsJobErrorNotification = NULL,
        .FlsInfo = flashInfo,

        .FlsMaxReadFastMode = 16, .FlsMaxReadNormalMode = 16,
        .FlsMaxWriteFastMode = 32, .FlsMaxWriteNormalMode = 32,

        } };
