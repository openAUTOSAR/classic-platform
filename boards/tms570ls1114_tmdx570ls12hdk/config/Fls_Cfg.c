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
    [0].sectCnt = 4,
    [0].bankSize = 0x10000, //64 kB
    [0].regBase = 0xF0200000UL,
    [0].sectAddr[0] = 0,
    [0].sectAddr[1] = 0x00004000,
    [0].sectAddr[2] = 0x00008000,
    [0].sectAddr[3] = 0x0000C000,
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
