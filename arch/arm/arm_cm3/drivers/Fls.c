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

#include "stm32f10x_flash.h"
#include "Fls.h"

static Fls_ConfigType const *flsConfigPtr;

void Fls_Init(const Fls_ConfigType *ConfigPtr)
{
	flsConfigPtr = ConfigPtr;
}

Std_ReturnType Fls_Erase(Fls_AddressType TargetAddress, Fls_LengthType Length)
{
	Fls_AddressType pageStart = 0;
	Fls_AddressType erased = 0;
	u32 page = 0;
	u32 pageIndex;

	/* Unlock the Flash Program Erase controller */
	FLASH_Unlock();
	/* Clear All pending flags */
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

	/* Find first sector. */
	page = (TargetAddress - flsConfigPtr->FlsSectorList[0].FlsSectorStartaddress) / flsConfigPtr->FlsSectorList[0].FlsPageSize;
	pageStart = flsConfigPtr->FlsSectorList[0].FlsSectorStartaddress + page * flsConfigPtr->FlsSectorList[0].FlsPageSize;
	//page = Length / flsConfigPtr->FlsSectorList[0].FlsPageSize;

	/* Erase the pages... */
	for (pageIndex = page; erased < Length; pageIndex++)
	{
		volatile FLASH_Status status = FLASH_BUSY;
		while(status != FLASH_COMPLETE){
			status = FLASH_ErasePage((uint32_t)pageStart);
		}
		erased += flsConfigPtr->FlsSectorList[0].FlsPageSize;
		pageStart += flsConfigPtr->FlsSectorList[0].FlsPageSize;
	}

	return E_OK;
}

Std_ReturnType Fls_Write(Fls_AddressType TargetAddress, const uint8 *SourceAddressPtr, Fls_LengthType Length)
{
	FLASH_Status stResult;
	Fls_LengthType len = Length;
	Fls_AddressType addr = TargetAddress;
	const uint8 *srcPtr = SourceAddressPtr;

	while (len >= sizeof(uint32_t))
	{
		stResult = FLASH_ProgramWord((uint32_t)addr, (uint32_t)*(uint32_t *)srcPtr);
		srcPtr += sizeof(uint32_t);
		addr += sizeof(uint32_t);
		len -= sizeof(uint32_t);
	}

	if (len == sizeof(uint16_t))
	{
		FLASH_ProgramHalfWord((uint32_t)addr, (uint16_t)*(uint16_t *)srcPtr);
		srcPtr += sizeof(uint16_t);
		addr += sizeof(uint16_t);
		len -= sizeof(uint16_t);
	}

	return E_OK;
}
