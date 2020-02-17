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

#ifndef FRSM_TYPES_H_
#define FRSM_TYPES_H_

/* @req FrSm054 */
#if defined(USE_DEM)
#include "Dem.h"
#endif

#include "Fr_GeneralTypes.h"

/* @req FrSm180 */
typedef enum {
    FRSM_BSWM_READY,
    FRSM_BSWM_READY_ECU_PASSIVE,
    FRSM_BSWM_STARTUP,
    FRSM_BSWM_STARTUP_ECU_PASSIVE,
    FRSM_BSWM_WAKEUP,
    FRSM_BSWM_WAKEUP_ECU_PASSIVE,
    FRSM_BSWM_HALT_REQ,
    FRSM_BSWM_HALT_REQ_ECU_PASSIVE,
    FRSM_BSWM_KEYSLOT_ONLY,
    FRSM_BSWM_KEYSLOT_ONLY_ECU_PASSIVE,
    FRSM_BSWM_ONLINE,
    FRSM_BSWM_ONLINE_ECU_PASSIVE,
    FRSM_BSWM_ONLINE_PASSIVE,
    FRSM_BSWM_ONLINE_PASSIVE_ECU_PASSIVE,
    FRSM_BSWM_LOW_NUMBER_OF_COLDSTARTERS,
    FRSM_BSWM_LOW_NUMBER_OF_COLDSTARTERS_ECU_PASSIVE,
}FrSM_BswM_StateType;

typedef struct {
    const uint8                  FrIfControllerId;
    const Fr_ChannelType         FrSMChannel;
    const boolean                FrSMIsDualChannelNode;
    const boolean                FrTrcvAvailable;
} FrSM_ControllerType;

typedef struct
{
    const uint32                 FrSMDurationT1;
    const uint32                 FrSMDurationT2;
    const uint32                 FrSMDurationT3;
    const uint32                 FrSMMainFunctionCycleTime;
    const uint32                 FrSMNumWakeupPatterns;
    const uint32                 FrSMStartupRepetitions;
    const uint32                 FrSMStartupRepetitionsWithWakeup;
    const FrSM_ControllerType*   FrSMControllerRef;
    const uint8                  FrSMClusterId;
    const uint8                  FrSMControllerCount;
    const uint8                  FrSMMinNumberOfColdstarter;
    const uint8                  FrSMComMNetworkHandle;
    const uint8                  FrSMFrIfClusterRef;
#if (USE_DEM)
    Dem_EventIdType              FrSMClusterStartUpDemEventId;
    Dem_EventIdType              FrSMClusterSyncLossDemEventId;
#endif
    const boolean                FrSMCheckWakeupReason;
    const boolean                FrSMDelayStartupWithoutWakeup;
    const boolean                FrSMIsColdstartEcu;
    const boolean                FrSMIsWakeupEcu;
    const boolean                FrSMStartupRepetitionsType;
    const boolean                FrSMStartupRepetitionsWithWakeupType;
}FrSM_ClusterType;

typedef struct
{
    const FrSM_ClusterType*      FrSMClusters; /* configuration structure that is for FrSM_Init */
    const uint8*                 FrSMClusterLookups; /* Pointer to map from NetworkHandle to Frif Cluster IDs */
    const uint8                  FrSMClusterCount; /* No. of FrSM clusters */
    const uint8                  FrSMTotalCtrlCount; /* Total num of Controllers */
}FrSM_ConfigType;

#endif /* FRSM_TYPES_H_ */
