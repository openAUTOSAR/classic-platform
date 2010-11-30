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








#include "Fls.h"
#include "Fls_Cfg.h"
#include <stdlib.h>


#define KB *1024


const Fls_SectorType fls_evbSectorList[] = {
	{ // L0
	.FlsSectorStartaddress = (Fls_AddressType)0x00000000, // Start address of this sector
	.FlsPageSize = (Fls_LengthType)4, // Page size 8 words = 32bytes
	.FlsSectorSize = (Fls_LengthType)(16 KB), // 16Kbyte
	// Number of continuous sectors with the above characteristics.
	.FlsNumberOfSectors = (uint32)1,// L0 , 1 sectors
	},
	{ // L1-L2
	.FlsSectorStartaddress = (Fls_AddressType)0x00004000, // Start address of this sector
	.FlsPageSize = (Fls_LengthType)4, // Page size 8 words = 32bytes
	.FlsSectorSize = (Fls_LengthType)(48 KB), // 48Kbyte
	// Number of continuous sectors with the above characteristics.
	.FlsNumberOfSectors = (uint32)2,// L1->L2 , 2 sectors
	},
	{ // L3
	.FlsSectorStartaddress = (Fls_AddressType)0x0001C000, // Start address of this sector
	.FlsPageSize = (Fls_LengthType)4, // Page size 8 words = 32bytes
	.FlsSectorSize = (Fls_LengthType)(16 KB), // 16Kbyte
	// Number of continuous sectors with the above characteristics.
	.FlsNumberOfSectors = (uint32)8,// L0->L7 , 8 sectors
	},
	{ // L4-L5
	.FlsSectorStartaddress = (Fls_AddressType)0x00020000, // Start address of this sector
	.FlsPageSize = (Fls_LengthType)4, // Page size 8 words = 32bytes
	.FlsSectorSize = (Fls_LengthType)(128 KB), // 128Kbyte
	// Number of continuous sectors with the above characteristics.
	.FlsNumberOfSectors = (uint32)2,// L4->L5 , 8 sectors
	},
    { // H0->H11
    .FlsSectorStartaddress = (Fls_AddressType)0x00060000, // Start address of this sector
    .FlsPageSize = (Fls_LengthType)4, // Page size 8 words = 32bytes
    .FlsSectorSize = (Fls_LengthType)(128 KB), // 128Kbyte
    .FlsNumberOfSectors = (uint32)12,// H0->H11
    }
};

/*
 * Block to partition map
 */
uint8 Fls_BlockToPartitionMap[] = { 1,1,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9 };

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
