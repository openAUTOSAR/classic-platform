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

#ifndef FLS_CFG_H_
#define FLS_CFG_H_

#include "Std_Types.h"

#define FLS_BASE_ADDRESS 			0x08000000
#define FLS_TOTAL_SIZE				0x40000 // from addr 0x0800_0000 to 0x0804_0000

// Configuration description of a flashable sector
typedef struct {
  // Number of continuous sectors with the above characteristics.
  Fls_LengthType FlsNumberOfSectors;

  // Size of one page of this sector. Implementation Type: Fls_LengthType.
  Fls_LengthType FlsPageSize;

  // Size of this sector. Implementation Type: Fls_LengthTyp
  Fls_LengthType FlsSectorSize;

  // Start address of this sector
  Fls_AddressType FlsSectorStartaddress;

} Fls_SectorType;


typedef struct {
  // List of flash:able sectors and pages
  const Fls_SectorType *FlsSectorList;

  // Size of List of the FlsSectorList
  const uint32 FlsSectorListSize;

  uint8 *FlsBlockToPartitionMap;

} Fls_ConfigType;

extern const Fls_ConfigType FlsConfigSet[];


#endif /* FLS_CFG_H_ */
