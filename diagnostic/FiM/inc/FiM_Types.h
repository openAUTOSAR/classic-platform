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


#ifndef FIM_TYPES_H_
#define FIM_TYPES_H_

#include "Std_Types.h"
#include "Rte_FiM_Type.h" /* @req SWS_Fim_00096 */
#include "Dem_Types.h"

#define FIM_LAST_FAILED 		0u
#define FIM_NOT_TESTED  		1u
#define FIM_TESTED				2u
#define FIM_TESTED_AND_FAILED	3u

typedef struct {
    const uint16 *EventIndexList;
    uint16 Index;
}FiM_SummaryEventCfgType;

typedef struct {
    const uint16 *EventIndexList;
    const uint16 *InhSumIndexList;
    const uint8 *InhibitionMask;
    FiM_FunctionIdType FID;
}FiM_InhibitionConfigurationType;

typedef struct {
    const uint16 *InhCfgIndexList;
    FiM_FunctionIdType FID;
}Fim_FIDCfgType;

typedef struct {
    const uint16 *AffectedInhCfgIndexList;
    Dem_EventIdType EventId;
}FiM_EventInhCfgType;

/* @req SWS_Fim_00092 */
typedef struct {
    const FiM_InhibitionConfigurationType *InhibitConfig;
    const Fim_FIDCfgType *FIDConfig;
    const FiM_EventInhCfgType *EventInhCfg;
    const FiM_SummaryEventCfgType *SummeryEventCfg;
    uint16 EventInhCfg_Size;
}FiM_ConfigType;

#endif /* FIM_TYPES_H_ */
