/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/








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
