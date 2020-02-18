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
/*lint -save -e451 MISRA:STANDARDIZED_INTERFACE:AUTOSAR need Inclusion:[MISRA 2012 Directive 4.10, required]*/
/*lint -save -e9021 MISRA:STANDARDIZED_INTERFACE:AUTOSAR need Inclusion:[MISRA 2012 Rule 20.5, advisory] */
#ifdef SCHM_START_SEC_VAR_CLEARED_ASIL_UNSPECIFIED
#include "MemMap.h"
#undef SCHM_MEMMAP_ERROR
#undef SCHM_START_SEC_VAR_CLEARED_ASIL_UNSPECIFIED
#endif

#ifdef SCHM_STOP_SEC_VAR_CLEARED_ASIL_UNSPECIFIED
#include "MemMap.h"
#undef SCHM_MEMMAP_ERROR
#undef SCHM_STOP_SEC_VAR_CLEARED_ASIL_UNSPECIFIED
#endif

#ifdef SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "MemMap.h"
#undef SCHM_MEMMAP_ERROR
#undef SCHM_START_SEC_VAR_CLEARED_QM_UNSPECIFIED
#endif

#ifdef SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#include "MemMap.h"
#undef SCHM_MEMMAP_ERROR
#undef SCHM_STOP_SEC_VAR_CLEARED_QM_UNSPECIFIED
#endif
/*lint -restore */

