
/*
 * multicore.h
 *
 *  Created on: 1 apr 2011
 *      Author: Niclas
 */

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
#define Os_OnRunningCore(x,y) true
#endif


#endif /* MULTICORE_H_ */

