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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.2.2 */
#ifndef STBM_TYPES_H_
#define STBM_TYPES_H_

#if defined(USE_NVM)
#include "NvM.h"
#endif

/* @req SWS_StbM_00142 */ /*  @req SWS_StbM_00150 */
typedef uint16 StbM_SynchronizedTimeBaseType;

/* @req SWS_StbM_00194 */
typedef uint32 StbM_TimeStampRawType;

typedef enum{
    NO_STORAGE,
    STORAGE_AT_SHUTDOWN
}StbM_StoreTimebaseNVType;


typedef struct {

    StbM_SynchronizedTimeBaseType   StbMOffsetTimeBaseID;   /* Reference to the Synchronized Time-Base  */
    uint32 				            StbMLocalTimeRef;     /* Reference to local time Hardware/OsCounterTicksPerBase  OS counter  */
    uint32 				            StbMEthGlobalTimeDomainRef; /* Reference to local time shall be accessed on an Ethernet bus*/
    boolean                         StbMIsHardwareTimersupported; /* used this for identify the hardware timer is enabled */
    float64                         StbMSyncLossThreshold; /* minimum delta between the time value in two sync messages for which the sync loss flag is set */
    float64                         StbMSyncLossTimeout; 	/* timeout for the situation that the time synchronization gets lost in the scope of the time domain*/
    StbM_SynchronizedTimeBaseType   StbMSyncTimeBaseId;	/* Synchronized time-base via a unique identifier */
    StbM_StoreTimebaseNVType        StbMStoreTimebaseNonVolatile; /* specifying that the time base shall be stored in the NvRam*/
    const boolean                   StbMIsSystemWideGlobalTimeMaster; /* global time master that acts as a system-wide source of time information with respect to global time.*/
#if defined(USE_NVM)
    NvM_BlockIdType StbMNvmUseBlockID;
#endif
}StbMSyncTimeBaseType;

typedef struct {

    const StbMSyncTimeBaseType  *StbMSynchronizedTimeBaseRef;  /* Reference to the required synchronized time-base */
    uint32 				  StbMTriggeredCustomerPeriod;  /* triggering period of the triggered customer in microseconds */
    uint16                StbMOSScheduleTableRef;   /* Reference Mandatory reference to synchronized OS ScheduleTable,*/
}StbMTriggeredCustomerType;


/** Top level config container for STBM implementation. */
typedef struct {
    const StbMSyncTimeBaseType* 	 StbMSyncTimeBase;         /* Reference to Synchronized time base  */
    const StbMTriggeredCustomerType* StbMTriggeredCustomer;    /* Reference to Synchronized Trigger Customers  */

}StbM_ConfigType; /* @req SWS_StbM_00249*/


#endif /* STBM_TYPES_H_ */
