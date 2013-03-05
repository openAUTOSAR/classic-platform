/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/


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


