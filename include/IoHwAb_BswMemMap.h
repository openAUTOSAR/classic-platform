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

/*lint -e451 -e9021 AUTOSAR API SWS_MemMap_00003 */

#define IOHWAB_MEMMAP_ERROR

#ifdef IOHWAB_START_SEC_VAR_INIT_UNSPECIFIED
#include "MemMap.h" /*lint !e451 !e9019 MISRA:STANDARDIZED_INTERFACE:AUTOSAR Require Inclusion:[MISRA 2012 Directive 4.10, required] [MISRA 2012 Rule 20.1, advisory] */
#undef IOHWAB_MEMMAP_ERROR
#undef IOHWAB_START_SEC_VAR_INIT_UNSPECIFIED
#endif

#ifdef IOHWAB_STOP_SEC_VAR_INIT_UNSPECIFIED
#include "MemMap.h" /*lint !e451 !e9019 MISRA:STANDARDIZED_INTERFACE:AUTOSAR Require Inclusion:[MISRA 2012 Directive 4.10, required] [MISRA 2012 Rule 20.1, advisory] */
#undef IOHWAB_MEMMAP_ERROR
#undef IOHWAB_STOP_SEC_VAR_INIT_UNSPECIFIED
#endif


#ifdef IOHWAB_START_SEC_VAR_CLEARED_UNSPECIFIED
#include "MemMap.h" /*lint !e451 !e9019 MISRA:STANDARDIZED_INTERFACE:AUTOSAR Require Inclusion:[MISRA 2012 Directive 4.10, required] [MISRA 2012 Rule 20.1, advisory] */
#undef IOHWAB_MEMMAP_ERROR
#undef IOHWAB_START_SEC_VAR_CLEARED_UNSPECIFIED
#endif

#ifdef IOHWAB_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#include "MemMap.h" /*lint !e451 !e9019 MISRA:STANDARDIZED_INTERFACE:AUTOSAR Require Inclusion:[MISRA 2012 Directive 4.10, required] [MISRA 2012 Rule 20.1, advisory] */
#undef IOHWAB_MEMMAP_ERROR
#undef IOHWAB_STOP_SEC_VAR_CLEARED_UNSPECIFIED
#endif


#ifdef IOHWAB_MEMMAP_ERROR
#error "EcuM_BswMemMap.h error, section not mapped"
#endif
