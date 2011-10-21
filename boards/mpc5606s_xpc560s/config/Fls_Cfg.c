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
    { // B0F0
    .FlsSectorStartaddress = (Fls_AddressType)0x00000000, // Start address of this sector
    .FlsPageSize = (Fls_LengthType)8, // Read page size of 128 bits (4 words), (8 bytes)
    .FlsSectorSize = (Fls_LengthType)(32 KB), // 32Kbyte
    // Number of continuous sectors with the above characteristics.
    .FlsNumberOfSectors = (uint32)1,// B0F0 , 1 sectors
    },
    { // B0F1, B0F2
    .FlsSectorStartaddress = (Fls_AddressType)0x00008000,
    .FlsPageSize = (Fls_LengthType)8, // Read page size of 128 bits (4 words), (8 bytes)
    .FlsSectorSize = (Fls_LengthType)(16 KB), // 16Kbyte
    .FlsNumberOfSectors = (uint32)2,// B0F1, B0F2, 2 sectors
    },
    { // B0F3, B0F4
    .FlsSectorStartaddress = (Fls_AddressType)0x00010000,
    .FlsPageSize = (Fls_LengthType)8, // Read page size of 128 bits (4 words), (8 bytes)
    .FlsSectorSize = (Fls_LengthType)(32 KB), // 32Kbyte
    .FlsNumberOfSectors = (uint32)2,// B0F3, B0F4, 2 sectors
    },
    { // B0F5, B0F6, B0F7
    .FlsSectorStartaddress = (Fls_AddressType)0x00020000,
    .FlsPageSize = (Fls_LengthType)8, // Read page size of 128 bits (4 words), (8 bytes)
    .FlsSectorSize = (Fls_LengthType)(128 KB), // 128Kbyte
    .FlsNumberOfSectors = (uint32)3,// B0F5, B0F6, B0F7, 3 sectors
    },
    { // B2F0, B2F1, B2F2, B2F3
    .FlsSectorStartaddress = (Fls_AddressType)0x00080000,
    .FlsPageSize = (Fls_LengthType)8, // Read page size of 128 bits (4 words), (8 bytes)
    .FlsSectorSize = (Fls_LengthType)(128 KB), // 128Kbyte
    .FlsNumberOfSectors = (uint32)4,// B2F0, B2F1, B2F2, B2F3, 4 sectors
    },
    { // B1F0, B1F1, B1F2, B1F3
    .FlsSectorStartaddress = (Fls_AddressType)0x00800000,
    .FlsPageSize = (Fls_LengthType)8, // Read page size of 128 bits (4 words), (8 bytes)
    .FlsSectorSize = (Fls_LengthType)(16 KB), // 16Kbyte
    .FlsNumberOfSectors = (uint32)4,// B1F0, B1F1, B1F2, B1F3, 4 sectors
    },

};

/*
 * Block to partition map
 */
uint8 Fls_BlockToPartitionMap[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };

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
