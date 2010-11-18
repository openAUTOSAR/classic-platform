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

#ifndef FEE_CFG_H_
#define FEE_CFG_H_

#include "MemIf_Types.h"
#include "Fee_ConfigTypes.h"

#define FEE_DEV_ERROR_DETECT			STD_ON		// Activate/Deactivate Dev Error Detection and Notification.
#define FEE_VERSION_INFO_API    		STD_ON		// Activate/Deactivate ver info API.
#define FEE_POLLING_MODE				STD_OFF		// Activate/Deactivate polling mode

#define FEE_INDEX							0
#define FEE_VIRTUAL_PAGE_SIZE			   16		// 0..65535

#define FEE_NUM_OF_BLOCKS					3		// Needs to be derived from Fee_Cfg.c

// Published information
#define FEE_BLOCK_OVERHEAD					5		// 1 Status + 4 Magic bytes
#define FEE_PAGE_OVERHEAD					0		// Currently none
#define FEE_MAXIMUM_BLOCKING_TIME			?		// TBD
#endif /*FEE_CFG_H_*/
