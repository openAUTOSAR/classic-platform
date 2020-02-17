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

#ifndef RTE_WDGM_TYPE_H_
#define RTE_WDGM_TYPE_H_

#define WDGM_NOT_SERVICE_COMPONENT
#ifdef USE_RTE
#warning This file should only be used when NOT using an WdgM Service Component.
#include "Rte_Type.h"
#else
/* Std_Types.h is normally included by Rte_Type.h */
#include "Std_Types.h"

typedef uint8 WdgM_LocalStatusType;
typedef uint8 WdgM_GlobalStatusType;
typedef uint8 WdgM_ModeType;


/* NOTE:    datatype depends on the maximal configured amount of supervised entities
 *          currently we assume that 16bit are necessary (alternative would be 8)
 */
typedef uint16 WdgM_SupervisedEntityIdType;


/* NOTE:    datatype depends on the maximal configured amount of supervised entities
 *          currently we assume that 16bit are necessary (alternative would be 8)
 */
typedef uint16 WdgM_CheckpointIdType;

/** --- ENUMERATION DATA TYPES ------------------------------------------------------------------ */

/** Enum literals for WdgM_GlobalStatusType */
#ifndef WDGM_GLOBAL_STATUS_OK
#define WDGM_GLOBAL_STATUS_OK 0U
#endif /* WDGM_GLOBAL_STATUS_OK */

#ifndef WDGM_GLOBAL_STATUS_FAILED
#define WDGM_GLOBAL_STATUS_FAILED 1U
#endif /* WDGM_GLOBAL_STATUS_FAILED */

#ifndef WDGM_GLOBAL_STATUS_EXPIRED
#define WDGM_GLOBAL_STATUS_EXPIRED 2U
#endif /* WDGM_GLOBAL_STATUS_EXPIRED */

#ifndef WDGM_GLOBAL_STATUS_STOPPED
#define WDGM_GLOBAL_STATUS_STOPPED 3U
#endif /* WDGM_GLOBAL_STATUS_STOPPED */

#ifndef WDGM_GLOBAL_STATUS_DEACTIVATED
#define WDGM_GLOBAL_STATUS_DEACTIVATED 4U
#endif /* WDGM_GLOBAL_STATUS_DEACTIVATED */

/** Enum literals for WdgM_LocalStatusType */
#ifndef WDGM_LOCAL_STATUS_OK
#define WDGM_LOCAL_STATUS_OK 0U
#endif /* WDGM_LOCAL_STATUS_OK */

#ifndef WDGM_LOCAL_STATUS_FAILED
#define WDGM_LOCAL_STATUS_FAILED 1U
#endif /* WDGM_LOCAL_STATUS_FAILED */

#ifndef WDGM_LOCAL_STATUS_EXPIRED
#define WDGM_LOCAL_STATUS_EXPIRED 2U
#endif /* WDGM_LOCAL_STATUS_EXPIRED */

#ifndef WDGM_LOCAL_STATUS_DEACTIVATED
#define WDGM_LOCAL_STATUS_DEACTIVATED 4U
#endif /* WDGM_LOCAL_STATUS_DEACTIVATED */

/** Enum literals for WdgM_ModeType */
#ifndef SUPERVISION_DEACTIVATED
#define SUPERVISION_DEACTIVATED 0U
#endif /* SUPERVISION_DEACTIVATED */

#ifndef SUPERVISION_FAILED
#define SUPERVISION_FAILED 1U
#endif /* SUPERVISION_FAILED */

#ifndef SUPERVISION_EXPIRED
#define SUPERVISION_EXPIRED 2U
#endif /* SUPERVISION_EXPIRED */

#ifndef SUPERVISION_STOPPED
#define SUPERVISION_STOPPED 3U
#endif /* SUPERVISION_STOPPED */

#ifndef SUPERVISION_OK
#define SUPERVISION_OK 4U
#endif /* SUPERVISION_OK */

/** --- MODE TYPES ------------------------------------------------------------------------------ */

#ifndef RTE_MODETYPE_WdgMMode
#define RTE_MODETYPE_WdgMMode
typedef uint8 Rte_ModeType_WdgMMode;
#endif

#ifndef RTE_TRANSITION_WdgMMode
#define RTE_TRANSITION_WdgMMode 255U
#endif

#ifndef RTE_MODE_WdgMMode_SUPERVISION_DEACTIVATED
#define RTE_MODE_WdgMMode_SUPERVISION_DEACTIVATED 0U
#endif

#ifndef RTE_MODE_WdgMMode_SUPERVISION_EXPIRED
#define RTE_MODE_WdgMMode_SUPERVISION_EXPIRED 2U
#endif

#ifndef RTE_MODE_WdgMMode_SUPERVISION_FAILED
#define RTE_MODE_WdgMMode_SUPERVISION_FAILED 1U
#endif

#ifndef RTE_MODE_WdgMMode_SUPERVISION_OK
#define RTE_MODE_WdgMMode_SUPERVISION_OK 4U
#endif

#ifndef RTE_MODE_WdgMMode_SUPERVISION_STOPPED
#define RTE_MODE_WdgMMode_SUPERVISION_STOPPED 3U
#endif

#ifndef RTE_MODETYPE_WdgMMode
#define RTE_MODETYPE_WdgMMode
typedef uint8 Rte_ModeType_WdgMMode;
#endif

#ifndef RTE_TRANSITION_WdgMMode
#define RTE_TRANSITION_WdgMMode 255U
#endif

#ifndef RTE_MODE_WdgMMode_SUPERVISION_DEACTIVATED
#define RTE_MODE_WdgMMode_SUPERVISION_DEACTIVATED 0U
#endif

#ifndef RTE_MODE_WdgMMode_SUPERVISION_EXPIRED
#define RTE_MODE_WdgMMode_SUPERVISION_EXPIRED 2U
#endif

#ifndef RTE_MODE_WdgMMode_SUPERVISION_FAILED
#define RTE_MODE_WdgMMode_SUPERVISION_FAILED 1U
#endif

#ifndef RTE_MODE_WdgMMode_SUPERVISION_OK
#define RTE_MODE_WdgMMode_SUPERVISION_OK 4U
#endif

#ifndef RTE_MODE_WdgMMode_SUPERVISION_STOPPED
#define RTE_MODE_WdgMMode_SUPERVISION_STOPPED 3U
#endif

#ifndef RTE_MODETYPE_WdgMMode
#define RTE_MODETYPE_WdgMMode
typedef uint8 Rte_ModeType_WdgMMode;
#endif

#ifndef RTE_TRANSITION_WdgMMode
#define RTE_TRANSITION_WdgMMode 255U
#endif

#ifndef RTE_MODE_WdgMMode_SUPERVISION_DEACTIVATED
#define RTE_MODE_WdgMMode_SUPERVISION_DEACTIVATED 0U
#endif

#ifndef RTE_MODE_WdgMMode_SUPERVISION_EXPIRED
#define RTE_MODE_WdgMMode_SUPERVISION_EXPIRED 2U
#endif

#ifndef RTE_MODE_WdgMMode_SUPERVISION_FAILED
#define RTE_MODE_WdgMMode_SUPERVISION_FAILED 1U
#endif

#ifndef RTE_MODE_WdgMMode_SUPERVISION_OK
#define RTE_MODE_WdgMMode_SUPERVISION_OK 4U
#endif

#ifndef RTE_MODE_WdgMMode_SUPERVISION_STOPPED
#define RTE_MODE_WdgMMode_SUPERVISION_STOPPED 3U
#endif
#endif

#endif /* RTE_WDGM_TYPE_H_ */
