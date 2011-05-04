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








#warning "This default file may only be used as an example!"

#include "Fls.h"
#include <stdlib.h>


#define KB *1024


const Fls_SectorType fls_evbSectorList[] = {
    { // L0->L7
    .FlsSectorStartaddress = (Fls_AddressType)0x00000000, // Start address of this sector
    .FlsPageSize = (Fls_LengthType)8, // Read page size of 128 bits (4 words), (8 bytes)
    .FlsSectorSize = (Fls_LengthType)(16 KB), // 16Kbyte
    // Number of continuous sectors with the above characteristics.
    .FlsNumberOfSectors = (uint32)8,// L0->L7 , 8 sectors
    },
    { // L8,L9
    .FlsSectorStartaddress = (Fls_AddressType)0x00020000,
    .FlsPageSize = (Fls_LengthType)8, // Read page size of 128 bits (4 words), (8 bytes)
    .FlsSectorSize = (Fls_LengthType)(64 KB), // 64Kbyte
    .FlsNumberOfSectors = (uint32)2,// L8,L9 , 2 sectors
    },
    { // M0->H7
    .FlsSectorStartaddress = (Fls_AddressType)0x00040000,
    .FlsPageSize = (Fls_LengthType)8, // Read page size of 128 bits (4 words), (8 bytes)
    .FlsSectorSize = (Fls_LengthType)(128 KB), // 128Kbyte
    .FlsNumberOfSectors = (uint32)6,// M0->H7
    }
};

/*
 * Block to partition map
 */
uint8 Fls_BlockToPartitionMap[] = { 1,1,1,1,2,2,2,2,3,3,4,4,5,5,6,6,7,7,8,8 };

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

    .FlsSectorList = &fls_evbSectorList[0],
    .FlsSectorListSize = sizeof(fls_evbSectorList)/sizeof(Fls_SectorType),
    .FlsBlockToPartitionMap = Fls_BlockToPartitionMap,
  }
};
