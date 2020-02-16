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
#include "kernel.h"
#include "arch.h"
#include "application.h"
#include "Os_Cfg.h"
#include "multicore_i.h"

void StartCore(CoreIDType CoreID, StatusType* Status) {
	if (CoreID >= OS_NUM_CORES) {
		*Status = E_OS_ID;
	}
	else if (Os_Sys[CoreID].status.os_started) {
		*Status = E_OS_ACCESS;
	}
	else if (Os_Sys[CoreID].status.activated) {
		*Status = E_OS_STATE;
	} else {
		boolean validId = Os_StartCore(CoreID);
		if (!validId) {
			*Status = E_OS_ID;
		} else {
			Os_Sys[CoreID].status.activated = true;
			*Status = E_OK;
		}
	}
}

boolean Os_OnRunningCore(ObjectTypeType ObjectType, uint32_t objectId) {
	ApplicationType app = CheckObjectOwnership(ObjectType,objectId);
	if (Os_ApplGetCore(app) == GetCoreID()) {
		return true;
	}
	else {
		return false;
	}
}
