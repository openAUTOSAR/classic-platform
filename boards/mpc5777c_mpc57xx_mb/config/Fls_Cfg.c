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

#if defined(CFG_MPC5777C)

/* IMPROVEMENT: This can actually be read from the flash instead */
const FlashType flashInfo[] = {
    /* NO RWW */

    /*LOW : 64*2                =  128KB*/
    /*MID : 64*2                =  128KB*/
    /*HIGH : 16*2               =   32KB*/
    /*LARGE : 256*32            = 8192KB*/

    /* Bank 0, Array 0 (LOW) */
    [0].sectCnt = 40,
    [0].bankSize = 0x01000000UL,/* This is used only for address range validation
    so the bank size will be 0x01000000 - 0x00000000 even though available actual size is only 0x848000 as computed above */
    [0].regBase = 0xFFFE8000UL,/* C55FMC_MCR register */

    /*Flash memory Low and Mid Blocks (256K)*/
    /* Low*/
    [0].sectAddr[0] = 0x00000000,
    [0].addrSpace[0] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 0,
    [0].sectAddr[1] = 0x00010000,
    [0].addrSpace[1] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 1,
    /*Mid*/
    [0].sectAddr[2] = 0x00020000,
    [0].addrSpace[2] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 0,
    [0].sectAddr[3] = 0x00030000,
    [0].addrSpace[3] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 1,
    [0].sectAddr[4] = 0x00040000,/* end first chunk */
    [0].addrSpace[4] = SECTOR_EMPTY,

    /*Flash memory High Blocks (32K)*/
    /*High*/
    [0].sectAddr[5] = 0x00600000,
    [0].addrSpace[5] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 0,
    [0].sectAddr[6] = 0x00604000,
    [0].addrSpace[6] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 1,
    [0].sectAddr[7] = 0x00608000,/* end first chunk */
    [0].addrSpace[7] = SECTOR_EMPTY,

    /*Flash memory Large Blocks (8M)*/
    /*Large*/
    [0].sectAddr[8] = 0x00800000,
    [0].addrSpace[8] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 0,
    [0].sectAddr[9] = 0x00840000,
    [0].addrSpace[9] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 1,
    [0].sectAddr[10] = 0x00880000,
    [0].addrSpace[10] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 2,
    [0].sectAddr[11] = 0x008C0000,
    [0].addrSpace[11] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 3,
    [0].sectAddr[12] = 0x00900000,
    [0].addrSpace[12] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 4,
    [0].sectAddr[13] = 0x00940000,
    [0].addrSpace[13] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 5,
    [0].sectAddr[14] = 0x00980000,
    [0].addrSpace[14] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 6,
    [0].sectAddr[15] = 0x009C0000,
    [0].addrSpace[15] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 7,
    [0].sectAddr[16] = 0x00A00000,
    [0].addrSpace[16] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 8,
    [0].sectAddr[17] = 0x00A40000,
    [0].addrSpace[17] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 9,
    [0].sectAddr[18] = 0x00A80000,
    [0].addrSpace[18] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 10,
    [0].sectAddr[19] = 0x00AC0000,
    [0].addrSpace[19] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 11,
    [0].sectAddr[20] = 0x00B00000,
    [0].addrSpace[20] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 12,
    [0].sectAddr[21] = 0x00B40000,
    [0].addrSpace[21] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 13,
    [0].sectAddr[22] = 0x00B80000,
    [0].addrSpace[22] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 14,
    [0].sectAddr[23] = 0x00BC0000,
    [0].addrSpace[23] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 15,
    [0].sectAddr[24] = 0x00C00000,
    [0].addrSpace[24] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 16,
    [0].sectAddr[25] = 0x00C40000,
    [0].addrSpace[25] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 17,
    [0].sectAddr[26] = 0x00C80000,
    [0].addrSpace[26] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 18,
    [0].sectAddr[27] = 0x00CC0000,
    [0].addrSpace[27] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 19,
    [0].sectAddr[28] = 0x00D00000,
    [0].addrSpace[28] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 20,
    [0].sectAddr[29] = 0x00D40000,
    [0].addrSpace[29] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 21,
    [0].sectAddr[30] = 0x00D80000,
    [0].addrSpace[30] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 22,
    [0].sectAddr[31] = 0x00DC0000,
    [0].addrSpace[31] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 23,
    [0].sectAddr[32] = 0x00E00000,
    [0].addrSpace[32] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 24,
    [0].sectAddr[33] = 0x00E40000,
    [0].addrSpace[33] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 25,
    [0].sectAddr[34] = 0x00E80000,
    [0].addrSpace[34] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 26,
    [0].sectAddr[35] = 0x00EC0000,
    [0].addrSpace[35] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 27,
    [0].sectAddr[36] = 0x00F00000,
    [0].addrSpace[36] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 28,
    [0].sectAddr[37] = 0x00F40000,
    [0].addrSpace[37] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 29,
    [0].sectAddr[38] = 0x00F80000,
    [0].addrSpace[38] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 30,
    [0].sectAddr[39] = 0x00FC0000,
    [0].addrSpace[39] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 31,
    [0].sectAddr[40] = 0x01000000,
    [0].addrSpace[40] = SECTOR_EMPTY,
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
