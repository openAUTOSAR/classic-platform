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


#if defined(CFG_JAC5)

const FlashType flashInfo[] = {
    [0] = {
        .sectCnt = 1,
        .bankSize =     0x00100000,
        .sectAddr[0] =  FLS_BASE_ADDRESS,
        .sectAddr[1] =  0xA0000000,   /* End, NOT a sector */
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
        .FlsInfo = flashInfo,

        .FlsMaxReadFastMode = 16,
        .FlsMaxReadNormalMode = 16,
        .FlsMaxWriteFastMode = 32,
        .FlsMaxWriteNormalMode = 32,
    }
};
