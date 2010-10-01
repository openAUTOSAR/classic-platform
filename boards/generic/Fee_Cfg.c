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

#include "Fee.h"

/*
 * Extern declarations for callbacks
 */
extern void NvmJobEndCallbackNotificationCallback(void);
extern void NvmJobErrorCallbackNotificationCallback(void);

extern uint8 FlashMemory[];


/*********************
 * Fee Configuration *
 *********************/


const Fee_BlockConfigType BlockConfigList[] = {
	{
		.BlockNumber = 0,				// TODO: Remove due to that DeviceIndex corresponds to posistion in list
		.DeviceIndex = 0,				// TODO: Needed by NvM and MemIf or remove?
		.BlockSize = 10,
		.ImmediateData = FALSE,
		.NumberOfWriteCycles = 0,	// Not supported yet
		.PhysBaseAddress = (uint32)&FlashMemory[0 * FEE_VIRTUAL_PAGE_SIZE],
	},
	{
		.BlockNumber = 1,				// TODO: Remove due to that DeviceIndex corresponds to posistion in list
		.DeviceIndex = 0,				// TODO: Needed by NvM and MemIf or remove?
		.BlockSize = 8,
		.ImmediateData = FALSE,
		.NumberOfWriteCycles = 0,	// Not supported yet
		.PhysBaseAddress = (uint32)&FlashMemory[3 * FEE_VIRTUAL_PAGE_SIZE],
	},
};

/*
 * Fee's config
 */
const Fee_ConfigType Fee_Config = {
		.General = {
				.NvmJobEndCallbackNotificationCallback = NULL,
				.NvmJobErrorCallbackNotificationCallback = NULL,
		},
		.BlockConfig = BlockConfigList,
};

