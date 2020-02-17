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

/** @reqSettings DEFAULT_SPECIFICATION_REVISION=4.3.0 */


#ifndef WDGM_RUNTIMETYPES_H
#define WDGM_RUNTIMETYPES_H

#include "WdgM_ConfigTypes.h"

typedef struct
{
    uint32          AliveCounter;
    uint16          SupervisionCycleCounter;
    boolean         wasEvaluated;
} WdgM_runtime_AliveSupervision;

typedef struct
{
    TickType          LastTickValue;
} WdgM_runtime_DeadlineSupervision;

typedef struct
{
    WdgM_LocalStatusType    LocalState;
    WdgM_Substate           SubstateAlive;
    WdgM_Substate           SubstateDeadline;
    WdgM_Substate           SubstateLogical;

    boolean                 IsInternalGraphActive; /* active flag for the internal graph */ /** @req SWS_WdgM_00271 */

    uint16                  PreviousCheckpointId_internalLogic;     /** @req SWS_WdgM_00246 */

    uint8                   FailedAliveCyclesCounter;
} WdgM_runtime_SupervisedEntity;


typedef struct
{
    WdgM_runtime_AliveSupervision      *AliveSupervisions;
    const uint16                        Length_AliveSupervisions;

    WdgM_runtime_DeadlineSupervision   *DeadlineSupervisions;
    const uint16                        Length_DeadlineSupervisions;
} WdgM_runtime_SupervisedEntityConfig;

typedef struct
{
    WdgM_runtime_SupervisedEntityConfig    *SE_Configs;
    const uint16                            Length_SEConfigs;

    uint16                                  ExpiredSupervisionCycleCounter;
} WdgM_runtime_Mode;

typedef struct
{
    WdgM_runtime_SupervisedEntity          *SEs;
    const uint16                            Length_SEs;

    WdgM_runtime_Mode                      *Modes;
    const uint16                            Length_Modes;

} WdgM_RuntimeData;


typedef struct
{
    const WdgM_ConfigType       *ConfigPtr;
    WdgM_RuntimeData            *runtimeDataPtr;
    boolean                     isInitiated;
    WdgM_GlobalStatusType       GlobalState;                    /** @req SWS_WdgM_00213 */
    const WdgM_Mode             *CurrentMode;
    boolean                     ResetInitiated;
    boolean                     isFirstExpiredSet;
} WdgM_debuggable_runtimeData;


#endif
