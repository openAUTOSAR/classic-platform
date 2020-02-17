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


#define FLS_INCLUDE_FILE "Fls_SST25xx.h"
#include "Fls.h"
#include <stdlib.h>

#define KB *1024

const Fls_SectorType fls_SST25xx_SectorList[] = {
    {
    .FlsSectorStartaddress 	= (Fls_AddressType)0x00000000,
    .FlsPageSize 			= (Fls_LengthType)1,
    .FlsSectorSize 			= (Fls_LengthType)(4 KB),
    .FlsNumberOfSectors 	= (uint32)512,
    },
};


const Fls_ConfigType FlsSST25xxConfigSet[]=
{
  {
      .FlsMaxReadFastMode 	= 64,
      .FlsMaxReadNormalMode 	= 1,
      .FlsMaxWriteFastMode 	= 1,
      .FlsMaxWriteNormalMode 	= 1,

      .FlsJobEndNotification 	 = NULL,
      .FlsJobErrorNotification = NULL,

    .FlsSectorList 		= &fls_SST25xx_SectorList[0],
    .FlsSectorListSize 	= sizeof(fls_SST25xx_SectorList)/sizeof(Fls_SectorType),
  }
};


