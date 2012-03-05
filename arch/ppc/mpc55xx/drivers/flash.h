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

/*
 * flash.h
 *
 *  Created on: 29 aug 2011
 *      Author: mahi
 */

#ifndef FLASH_H_
#define FLASH_H_

#include "flash_ll_h7f_c90.h"

#define FLASH_OP_LOCK			0
#define FLASH_OP_UNLOCK			1
#define FLASH_OP_MASK			1

#define ADDR_SPACE_CNT			3
#define ADDR_SPACE_LOW			0
#define ADDR_SPACE_MID			1
#define ADDR_SPACE_HIGH			2
#define ADDR_SPACE_GET(_x)			(((_x)>>8)&3)
#define ADDR_SPACE_SET(_x)			(((_x)&3)<<8)
#define ADDR_SPACE_GET_SECTOR(_x)   ((_x) & 0xff )
#define ADDR_SPACE_GET_PART(_x)		(((_x)>>10)&3)
#define ADDR_SPACE_SET_PART(_x)		(((_x)&3)<<10)

#define ADDR_SPACE(_sector,_space,_partition)	     \
					( ADDR_SPACE_SET_PART(_partition) | \
					 ADDR_SPACE_SET(_space ) | (_sector) )

struct Flash;

//extern const FlashType flashInfo[];


typedef void (*flashCbType)( void );

void Flash_Init( void );
uint32_t Flash_Lock(const struct Flash *fPtr, uint32_t op, uintptr_t from, uint32_t size);
uint32_t Flash_Erase(const struct Flash *fPtr, uintptr_t dest, uint32_t size, flashCbType sb);
//uint32_t Flash_Program(const struct Flash *fPtr, uintptr_t to, uintptr_t from,uint32_t size, flashCbType sb);
uint32_t Flash_ProgramPageStart(const struct Flash *fPtr, uint32_t *to, uint32_t *from,uint32_t *size, flashCbType sb);
uint32_t Flash_CheckStatus( const struct Flash *fPtr, uint32_t *to, uint32_t size );
uint32_t Flash_SectorAligned( const struct Flash *fPtr, uintptr_t addr );

#endif /* FLASH_H_ */
