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
#ifndef STBM_INTERNAL_TYPES_H_
#define STBM_INTERNAL_TYPES_H_


#include "Rte_StbM_Type.h"
#include "StbM_Types.h"


#define VALID_TIME_DOMAIN_MAX 15
#define VALID_TIME_DOMAIN_MIN 0
#define OFFSET_TIME_DOMAIN_MAX 31
#define OFFSET_TIME_DOMAIN_MIN 16

typedef enum {
    STBM_STATUS_UNINIT,
    STBM_STATUS_INIT

}StbM_Internal_InitStatusType;



typedef struct{
    float64                    syncLosstimer;
    StbM_TimeStampRawType      rawTimeDiff;
    StbM_TimeStampRawType      synchronisedRawtime;
    TickType                   stbmInternalOsCounter;
    StbM_UserDataType          StbMCurrentUserData;
    StbM_TimeStampExtendedType StbMCurrentTimeStampExtended;
    StbM_TimeStampType         StbMCurrentTimeStamp;
}StbM_Internal_TimebaseType;


typedef struct {
    StbM_Internal_InitStatusType StbMInitStatus;
    StbM_Internal_TimebaseType* timeBase;
#if (STBM_TRIGGERED_CUSTOMER_COUNT > 0)
    uint8 StbmTriggeredPeriod[STBM_TRIGGERED_CUSTOMER_COUNT];
#endif
}StbM_Internal_RunTimeType;


#endif /* STBM_INTERNAL_TYPES_H_ */
