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

#ifndef MEMIF_CFG_H_
#define MEMIF_CFG_H_

#include "MemIf_Types.h"

#if defined(USE_FEE)
#include "Fee.h"
#endif

#if defined(USE_EA)
#include "Ea.h"
#endif


#define MEMIF_VERSION_INFO_API    		STD_ON		// Activate/Deactivate ver info API.
#define MEMIF_DEV_ERROR_DETECT			STD_ON		// Activate/Deactivate Dev Error Detection and Notification.

#define MEMIF_NUMBER_OF_DEVICES			1			// Number of underlying memory abstraction modules.

#if (MEMIF_NUMBER_OF_DEVICES == 1)
#define MEMIF_DEVICE_TO_USE				FLS_DRIVER_INDEX
#endif
#endif /*MEMIF_CFG_H_*/
