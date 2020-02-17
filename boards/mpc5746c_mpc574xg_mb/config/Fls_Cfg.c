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

/*lint -save -e9054 OTHER Not limiting the array dimension */
#if defined(CFG_MPC5746C)

/*lint -e940 -e785 -e835 -e9027 -e845 -e651 To increase readability  */
const FlashType flashInfo[] = {

    /* NO RWW */

    /*LOW : 16*1 + 32*4 + 64*4  =  400KB*/
    /*MID : 16*8                =  128KB*/
    /*HIGH : 16*2               =   32KB*/
    /*LARGE : 256*10            = 2560KB*/

    /* Bank 0, Array 0 */
    [0].sectCnt = 32,
    [0].bankSize = 0xC70000UL, /* This is used only for address range validation
    so the bank size will be 0x01280000 - 0x00610000 even though available actual size is only 0x30C000 as computed above */
    [0].regBase = 0xFFFE0000UL, /* C55FMC_MCR register */

    /*HSM code*/ /* LOW */
    [0].sectAddr[0] = 0x00610000,
    [0].addrSpace[0] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 7,
    [0].sectAddr[1] = 0x00620000,
    [0].addrSpace[1] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 9,
    [0].sectAddr[2] = 0x00630000, /* end first chunk */
    [0].addrSpace[2] = SECTOR_EMPTY,

    /*HSM Data*/ /*HIGH*/
    [0].sectAddr[3] = 0x00F80000,
    [0].addrSpace[3] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 0,
    [0].sectAddr[4] = 0x00F84000,
    [0].addrSpace[4] = ADDR_SPACE_SET(ADDR_SPACE_HIGH) + 1,
    [0].sectAddr[5] = 0x00F88000, /* end first chunk */
    [0].addrSpace[5] = SECTOR_EMPTY,

    /*Small HSM Code Block*//* LOW */
    [0].sectAddr[6] = 0x00F8C000,
    [0].addrSpace[6] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 0,

    /*Small and Medium Flash Blocks*/
    /*MID*/
    [0].sectAddr[7] = 0x00F90000,
    [0].addrSpace[7] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 0,
    [0].sectAddr[8] = 0x00F94000,
    [0].addrSpace[8] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 1,
    [0].sectAddr[9] = 0x00F98000,
    [0].addrSpace[9] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 2,
    [0].sectAddr[10] = 0x00F9C000,
    [0].addrSpace[10] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 3,
    [0].sectAddr[11] = 0x00FA0000,
    [0].addrSpace[11] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 4,
    [0].sectAddr[12] = 0x00FA4000,
    [0].addrSpace[12] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 5,
    [0].sectAddr[13] = 0x00FA8000,
    [0].addrSpace[13] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 6,
    [0].sectAddr[14] = 0x00FAC000,
    [0].addrSpace[14] = ADDR_SPACE_SET(ADDR_SPACE_MID) + 7,
    [0].sectAddr[15] = 0x00FB0000,
    [0].addrSpace[15] = SECTOR_EMPTY, /* Reserved area 0x00FB0000 to 0x00FBFFFF */

    /*LOW*/
    [0].sectAddr[16] = 0x00FC0000,
    [0].addrSpace[16] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 2,
    [0].sectAddr[17] = 0x00FC8000,
    [0].addrSpace[17] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 3,
    [0].sectAddr[18] = 0x00FD0000,
    [0].addrSpace[18] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 4,
    [0].sectAddr[19] = 0x00FD8000,
    [0].addrSpace[19] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 5,
    [0].sectAddr[20] = 0x00FE0000,
    [0].addrSpace[20] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 6,
    [0].sectAddr[21] = 0x00FF0000,
    [0].addrSpace[21] = ADDR_SPACE_SET(ADDR_SPACE_LOW) + 8,

    /*Large Flash Blocks*/ /*LARGE*/
    [0].sectAddr[22] = 0x01000000,
    [0].addrSpace[22] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 0,
    [0].sectAddr[23] = 0x01040000,
    [0].addrSpace[23] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 1,
    [0].sectAddr[24] = 0x01080000,
    [0].addrSpace[24] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 2,
    [0].sectAddr[25] = 0x010C0000,
    [0].addrSpace[25] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 3,
    [0].sectAddr[26] = 0x01100000,
    [0].addrSpace[26] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 4,
    [0].sectAddr[27] = 0x01140000,
    [0].addrSpace[27] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 5,
    [0].sectAddr[28] = 0x01180000,
    [0].addrSpace[28] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 6,
    [0].sectAddr[29] = 0x011C0000,
    [0].addrSpace[29] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 7,
    [0].sectAddr[30] = 0x01200000,
    [0].addrSpace[30] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 8,
    [0].sectAddr[31] = 0x01240000,
    [0].addrSpace[31] = ADDR_SPACE_SET(ADDR_SPACE_LARGE) + 9,
    [0].sectAddr[32] = 0x01280000,
    [0].addrSpace[32] =SECTOR_EMPTY,
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
};/*lint -restore */

