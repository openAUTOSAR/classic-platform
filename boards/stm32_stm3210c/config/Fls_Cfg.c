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
#include <stdlib.h>


#define KB *1024

const Fls_SectorType fls_SectorList[] = {
    {
    .FlsSectorStartaddress = (Fls_AddressType)0x08000000, // Start address of this sector
    .FlsPageSize = (Fls_LengthType)(1 KB), // Page size of 1k
    .FlsSectorSize = (Fls_LengthType)(1 KB),
    // Number of continuous sectors with the above characteristics.
    .FlsNumberOfSectors = (uint32)255
    }
};


const Fls_ConfigType FlsConfigSet[]=
{
  {
    .FlsSectorList = &fls_SectorList[0],
    .FlsSectorListSize = sizeof(fls_SectorList)/sizeof(Fls_SectorType),
    .FlsBlockToPartitionMap = NULL,
  }
};
