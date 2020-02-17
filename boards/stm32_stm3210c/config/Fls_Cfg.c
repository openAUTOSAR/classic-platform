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
#include <stdlib.h>

#if defined(CFG_STM32F1X)

const FlashType flashInfo[] = {
    {
        /* Main memory use last 32 kB */
        .size = FLS_TOTAL_SIZE,
        .sectCnt = FLASH_MAX_PAGES,
        .sectAddr = { 0x08038000,
        /*.sectAddr[1] = */ 0x08038800,
        /*.sectAddr[2] = */ 0x08039000,
        /*.sectAddr[3] = */ 0x08039800,
        /*.sectAddr[4] = */ 0x0803A000,
        /*.sectAddr[5] = */ 0x0803A800,
        /*.sectAddr[6] = */ 0x0803B000,
        /*.sectAddr[7] = */ 0x0803B800,
        /*.sectAddr[8] = */ 0x0803C000,
        /*.sectAddr[9] = */ 0x0803C800,
        /*.sectAddr[10] = */ 0x0803D000,
        /*.sectAddr[11] = */ 0x0803D800,
        /*.sectAddr[12] = */ 0x0803E000,
        /*.sectAddr[13] = */ 0x0803E800,
        /*.sectAddr[14] = */ 0x0803F000,
        /*.sectAddr[15] = */ 0x0803F800,
        /*.sectAddr[16] = */ 0x08040000, }
    }
};

#else
#error CPU NOT supported
#endif


const Fls_ConfigType FlsConfigSet[]=
{
  {
     .FlsJobEndNotification = NULL,
     .FlsJobErrorNotification = NULL,
     .FlsInfo = flashInfo
  }
};
