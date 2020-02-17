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


#if defined(CFG_MPC5643L)
#define FLASH_REGS_BASE 0xC3F88000UL
    /* RWW between partitions*
     *
     * LOW   2*16KB + + 2*48KB + 2*64KB = 256KB
     * MID   2*128KB                    = 256KB
     * HIGH  4*128KB                    = 512KB
     */

const FlashType flashInfo[] = {
    [0].sectCnt = 10,
    [0].bankSize = 0x100000,
    [0].regBase = FLASH_REGS_BASE,

    /* LOW */
    [0].sectAddr[0] = 0,
    [0].addrSpace[0] = ADDR_SPACE(0, ADDR_SPACE_LOW, 1 ),
    [0].sectAddr[1] = 0x00004000,
    [0].addrSpace[1] = ADDR_SPACE(1, ADDR_SPACE_LOW, 1 ),
    [0].sectAddr[2] = 0x00010000,
    [0].addrSpace[2] = ADDR_SPACE(2, ADDR_SPACE_LOW, 1 ),
    [0].sectAddr[3] = 0x0001C000,
    [0].addrSpace[3] = ADDR_SPACE(3, ADDR_SPACE_LOW, 1 ),
    [0].sectAddr[4] = 0x00020000,
    [0].addrSpace[4] = ADDR_SPACE(4, ADDR_SPACE_LOW, 2 ),
    [0].sectAddr[5] = 0x00030000,
    [0].addrSpace[5] = ADDR_SPACE(5, ADDR_SPACE_LOW, 2 ),

    /* MID */
    [0].sectAddr[6] = 0x00040000,
    [0].addrSpace[6] = ADDR_SPACE(0, ADDR_SPACE_MID, 3 ),
    [0].sectAddr[7] = 0x00060000,
    [0].addrSpace[7] = ADDR_SPACE(1, ADDR_SPACE_MID, 3 ),

    /* HIGH */
    [0].sectAddr[8] = 0x00080000,
    [0].addrSpace[8] = ADDR_SPACE(0, ADDR_SPACE_HIGH, 4 ),
    [0].sectAddr[9] = 0x000C0000,
    [0].addrSpace[9] = ADDR_SPACE(1, ADDR_SPACE_HIGH, 4 ),
    [0].sectAddr[10] = 0x00100000,  /* End, NOT a sector */
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
