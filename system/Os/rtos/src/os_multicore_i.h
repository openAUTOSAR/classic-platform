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

#ifndef MULTICORE_H_
#define MULTICORE_H_

#if (OS_NUM_CORES > 1)


#include "application.h"
#include "sys.h"

typedef struct {
    OsServiceIdType op;
    uint32_t arg1;
    uint32_t arg2;
    uint32_t arg3;
    boolean opFinished;
    StatusType result;
} OsCoreMessageBoxType;

boolean Os_OnRunningCore(ObjectTypeType ObjectType, uint32_t objectId);
void Os_CoreNotificationInit();
StatusType Os_NotifyCore(CoreIDType coreId, OsServiceIdType op,
                         uint32_t arg1, uint32_t arg2, uint32_t arg3);

#else
/*lint -emacro(506, Os_OnRunningCore)  MISRA:FALSE_POSITIVE:For always evaluating as true:[MISRA 2012 Rule 2.1, required]*/
/*lint -emacro(774, Os_OnRunningCore) MISRA:FALSE_POSITIVE:For always evaluating as true:[MISRA 2012 Rule 14.3, required]*/
#define Os_OnRunningCore(x,y) true
#endif


#endif /* MULTICORE_H_ */

