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

/*lint -save -e9019 MISRA:OTHER:suppressed to be able to get the memap work:[MISRA 2012 Rule 20.1, advisory] */
/*lint -save -e9021 MISRA:OTHER:suppressed to be able to get the memap work:[MISRA 2012 Rule 20.5, advisory]*/

#define DET_MEMMAP_ERROR

#ifdef DET_STATE_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "MemMap.h"
#undef DET_MEMMAP_ERROR
#undef DET_STATE_START_SEC_VAR_CLEARED_UNSPECIFIED
#endif

#ifdef DET_STATE_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "MemMap.h"
#undef DET_MEMMAP_ERROR
#undef DET_STATE_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#endif

#ifdef DET_MONITOR_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "MemMap.h"
#undef DET_MEMMAP_ERROR
#undef DET_MONITOR_START_SEC_VAR_CLEARED_UNSPECIFIED
#endif

#ifdef DET_MONITOR_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "MemMap.h"
#undef DET_MEMMAP_ERROR
#undef DET_MONITOR_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#endif

#ifdef DET_MEMMAP_ERROR
#error "DET error, section not mapped"
#endif
/*lint -restore */
