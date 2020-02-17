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

const FlashType flashInfo[] = {
    [0].sectCnt = 32,
    [0].bankSize = 0x20000, // 128 kB
    [0].regBase = 0xF0200000UL,
    [0].sectAddr[0] = 0x00000000UL,
    [0].sectAddr[1] = 0x00001000UL,
    [0].sectAddr[2] = 0x00002000UL,
    [0].sectAddr[3] = 0x00003000UL,
    [0].sectAddr[4] = 0x00004000UL,
    [0].sectAddr[5] = 0x00005000UL,
    [0].sectAddr[6] = 0x00006000UL,
    [0].sectAddr[7] = 0x00007000UL,
    [0].sectAddr[8] = 0x00008000UL,
    [0].sectAddr[9] = 0x00009000UL,
    [0].sectAddr[10] = 0x0000A000UL,
    [0].sectAddr[11] = 0x0000B000UL,
    [0].sectAddr[12] = 0x0000C000UL,
    [0].sectAddr[13] = 0x0000D000UL,
    [0].sectAddr[14] = 0x0000E000UL,
    [0].sectAddr[15] = 0x0000F000UL,
    [0].sectAddr[16] = 0x00010000UL,
    [0].sectAddr[17] = 0x00011000UL,
    [0].sectAddr[18] = 0x00012000UL,
    [0].sectAddr[19] = 0x00013000UL,
    [0].sectAddr[20] = 0x00014000UL,
    [0].sectAddr[21] = 0x00015000UL,
    [0].sectAddr[22] = 0x00016000UL,
    [0].sectAddr[23] = 0x00017000UL,
    [0].sectAddr[24] = 0x00018000UL,
    [0].sectAddr[25] = 0x00019000UL,
    [0].sectAddr[26] = 0x0001A000UL,
    [0].sectAddr[27] = 0x0001B000UL,
    [0].sectAddr[28] = 0x0001C000UL,
    [0].sectAddr[29] = 0x0001D000UL,
    [0].sectAddr[30] = 0x0001E000UL,
    [0].sectAddr[31] = 0x0001F000UL,
    [0].sectAddr[32] = 0x00020000UL, // end
};

const Fls_ConfigType FlsConfigSet[]=
{
  {
    .FlsAcWrite = NULL,
    .FlsAcErase = NULL,
    .FlsJobEndNotification = NULL,
    .FlsJobErrorNotification = NULL,
#if (STD_ON == USE_FLS_INFO)
    .FlsInfo = flashInfo,
#endif

    .FlsMaxReadFastMode = 16,
    .FlsMaxReadNormalMode = 16,
    .FlsMaxWriteFastMode = 32,
    .FlsMaxWriteNormalMode = 32,

//    .FlsSectorList = &fls_evbSectorList[0],
//    .FlsSectorListSize = sizeof(fls_evbSectorList)/sizeof(Fls_SectorType),
//    .FlsBlockToPartitionMap = Fls_BlockToPartitionMap,
  }
};
