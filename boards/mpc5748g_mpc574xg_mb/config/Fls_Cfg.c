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
/** @tagSettings DEFAULT_ARCHITECTURE= MPC5748G */
/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.1.2 */

#include "Fls.h"
#include "flash.h"
#include <stdlib.h>

#if defined(CFG_MPC5748G)

/*lint -e940 -e785 -e835 -e9027 -e845 -e651 OTHER To increase readability  */
const FlashType flashInfo[] = {

    /*
     * Resources:
     * - 3.3 NVM memory map
     * - 74.1.1.1 C55FMC_LOCK0 and C55FMC_SEL0 register bit mapping
     *    - LOW  - partition 0 and 1
     *    - MID  - partition 2 and 3
     *    - HIGH - partition 4 and 5
     *    - 256  - partition 6 to 9
     *
     */


    /* Bank 0, Array 0 (LOW) */
    [0].sectCnt = 45,
    [0].bankSize = 0x019F0000,
    [0].regBase = 0xFFFE0000UL,

    /* HSM Code */
    [0].sectAddr[0] = 0x00610000,
    [0].addrSpace[0] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 7,
    [0].sectAddr[1] = 0x00620000,
    [0].addrSpace[1] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 9,
    [0].sectAddr[2] = 0x00630000, /* end first chunk */
    [0].addrSpace[2] = SECTOR_EMPTY,

    /* HSM Data */
    [0].sectAddr[3] = 0x00F80000,
    [0].addrSpace[3] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 0,
    [0].sectAddr[4] = 0x00F84000,
    [0].addrSpace[4] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 1,
    [0].sectAddr[5] = 0x00F88000, /* end first chunk */
    [0].addrSpace[5] = SECTOR_EMPTY,

    /* Small HSM Code Block */
    [0].sectAddr[6] = 0x00F8C000,
    [0].addrSpace[6] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 0,
    /* Small and Medium Flash Blocks */
    /* 16KB blocks */
    [0].sectAddr[7] = 0x00F90000,                                   /* partition 2 */
    [0].addrSpace[7] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 0,
    [0].sectAddr[8] = 0x00F94000,                                   /* partition 2 */
    [0].addrSpace[8] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 1,
    [0].sectAddr[9] = 0x00F98000,                                   /* partition 2 */
    [0].addrSpace[9] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 2,
    [0].sectAddr[10] = 0x00F9C000,                                  /* partition 2 */
    [0].addrSpace[10] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 3,
    [0].sectAddr[11] = 0x00FA0000,                                  /* partition 3 */
    [0].addrSpace[11] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 4,
    [0].sectAddr[12] = 0x00FA4000,                                  /* partition 3 */
    [0].addrSpace[12] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 5,
    [0].sectAddr[13] = 0x00FA8000,                                  /* partition 3 */
    [0].addrSpace[13] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 6,
    [0].sectAddr[14] = 0x00FAC000,                                  /* partition 3 */
    [0].addrSpace[14] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 7,
    [0].sectAddr[15] = 0x00FB0000,                                  /* partition 2 */
    [0].addrSpace[15] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 8,
    [0].sectAddr[16] = 0x00FB8000,                                  /* partition 3 */
    [0].addrSpace[16] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 9,
    /* LOW */
    [0].sectAddr[17] = 0x00FC0000,                                  /* partition 0 */
    [0].addrSpace[17] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 2,
    [0].sectAddr[18] = 0x00FC8000,                                  /* partition 0 */
    [0].addrSpace[18] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 3,
    [0].sectAddr[19] = 0x00FD0000,                                  /* partition 1 */
    [0].addrSpace[19] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 4,
    [0].sectAddr[20] = 0x00FD8000,                                  /* partition 1 */
    [0].addrSpace[20] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 5,
    [0].sectAddr[21] = 0x00FE0000,                                  /* partition 0 */
    [0].addrSpace[21] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 6,
    [0].sectAddr[22] = 0x00FF0000,                                  /* partition 1 */
    [0].addrSpace[22] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 8,

    /* Large Flash Blocks */
    [0].sectAddr[23] = 0x01000000,                                  /* partition 6 */
    [0].addrSpace[23] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 0,
    [0].sectAddr[24] = 0x01040000,
    [0].addrSpace[24] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 1,
    [0].sectAddr[25] = 0x01080000,
    [0].addrSpace[25] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 2,
    [0].sectAddr[26] = 0x010C0000,
    [0].addrSpace[26] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 3,
    [0].sectAddr[27] = 0x01100000,
    [0].addrSpace[27] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 4,
    [0].sectAddr[28] = 0x01140000,
    [0].addrSpace[28] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 5,
    [0].sectAddr[29] = 0x01180000,
    [0].addrSpace[29] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 6,
    [0].sectAddr[30] = 0x011C0000,
    [0].addrSpace[30] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 7,
    [0].sectAddr[31] = 0x01200000,                                  /* partition 7 */
    [0].addrSpace[31] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 8,
    [0].sectAddr[32] = 0x01240000,
    [0].addrSpace[32] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 9,
    [0].sectAddr[33] = 0x01280000,
    [0].addrSpace[33] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 10,
    [0].sectAddr[34] = 0x012C0000,
    [0].addrSpace[34] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 11,
    [0].sectAddr[35] = 0x01300000,
    [0].addrSpace[35] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 12,
    [0].sectAddr[36] = 0x01340000,
    [0].addrSpace[36] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 13,
    [0].sectAddr[37] = 0x01380000,
    [0].addrSpace[37] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 14,
    [0].sectAddr[38] = 0x013C0000,
    [0].addrSpace[38] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 15,
    [0].sectAddr[39] = 0x01400000,                                   /* partition 8 */
    [0].addrSpace[39] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 16,
    [0].sectAddr[40] = 0x01440000,
    [0].addrSpace[40] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 17,
    [0].sectAddr[41] = 0x01480000,
    [0].addrSpace[41] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 18,
    [0].sectAddr[42] = 0x014C0000,                                   /* partition 9 */
    [0].addrSpace[42] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 19,
    [0].sectAddr[43] = 0x01500000,
    [0].addrSpace[43] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 20,
    [0].sectAddr[44] = 0x01540000,
    [0].addrSpace[44] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 21,
    [0].sectAddr[45] = 0x01580000,
    [0].addrSpace[45] = SECTOR_EMPTY,

};

#else
#error CPU NOT supported
#endif


/* @req SWS_Fls_00262 */
/* @req SWS_Fls_00263 */
/* @req SWS_Fls_00368 */

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
        .FlsMaxWriteFastMode = 32, .FlsMaxWriteNormalMode = 32, } };
