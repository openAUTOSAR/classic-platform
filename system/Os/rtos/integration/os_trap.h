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

#ifndef OS_TRAP_H_
#define OS_TRAP_H_

 /**
  * @file    os_trap.h
  * @brief   Internal trap interface for the OS used for OS_SC3 and OS_SC4.
  *          The intended use is to give supervisor access to the configured APIs
  *
  * @addtogroup os
  * @details
  * @{
  */

/* ----------------------------[includes]------------------------------------*/

#include "Os.h"
#include "irq.h"

/* ----------------------------[private define]------------------------------*/
#if (OS_SC3 == STD_ON ) || (OS_SC4 == STD_ON )

/*
 * In order to separate the different traps from each other the index is
 * split into two 16-bits fields:
 *
 *   3                    2                   1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * |         Trap Index            |        Service Index          |
 *
 *
 *   Prefix          Trap Index
 *   -------------------------------------------
 *   SYS_CALL_          0
 *   OS_TRAP_           1
 *
 *
 */

#define OS_TRAP_QTST_IDX    2
#define OS_TRAP_FLS_IDX     3

#define OS_TRAP_OS_IDX                          1u

#define OS_TRAP_OS_IDX_SHIFTED                  (OS_TRAP_OS_IDX << 16u)

/* Index to trap functions */
#if defined(CFG_PPC) || defined(CFG_TMS570)
#define OS_TRAP_INDEX_Os_SetPrivilegedMode      (0u + OS_TRAP_OS_IDX_SHIFTED)
#define OS_TRAP_INDEX_Irq_GenerateSoftInt       (2u + OS_TRAP_OS_IDX_SHIFTED)
#define OS_TRAP_INDEX_Irq_AckSoftInt            (3u + OS_TRAP_OS_IDX_SHIFTED)
#define OS_TRAP_INDEX_Os_IsrAdd                 (4u + OS_TRAP_OS_IDX_SHIFTED)
#else
#define OS_TRAP_INDEX_Reserved                  (0UL)
#define OS_TRAP_INDEX_Os_ArchToPrivilegedMode   (-1UL)
#define OS_TRAP_INDEX_Irq_GenerateSoftInt       (-2UL)
#define OS_TRAP_INDEX_Os_IsrAdd                 (-3UL)
#endif

/* ----------------------------[public define]------------------------------*/
/* OS trap internal functions */
#define OS_TRAP_Irq_GenerateSoftInt(_a1)        SYS_CALL_1( OS_TRAP_INDEX_Irq_GenerateSoftInt,_a1 )
#define OS_TRAP_Irq_AckSoftInt(_a1)             SYS_CALL_1( OS_TRAP_INDEX_Irq_AckSoftInt,_a1 )


/* PPC */
#if defined(CFG_PPC) || defined(CFG_TMS570)
#define OS_TRAP_Os_SetPrivilegedMode(_a1)      SYS_CALL_0( OS_TRAP_INDEX_Os_SetPrivilegedMode )
#elif defined(CFG_TC2XX)
#define OS_TRAP_Os_ArchToPrivilegedMode(_a1)   SYS_CALL_1( OS_TRAP_INDEX_Os_ArchToPrivilegedMode, _a1)
#endif

#define OS_TRAP_Os_IsrAdd(_a1)                  SYS_CALL_1( OS_TRAP_INDEX_Os_IsrAdd, _a1)

#else  /* for OS_SC1 or OS_SC2 */

#define OS_TRAP_Irq_GenerateSoftInt            Irq_GenerateSoftInt
#define OS_TRAP_Irq_AckSoftInt                 Irq_AckSoftInt
#define OS_TRAP_Os_IsrAdd                      Os_IsrAdd

#if defined(CFG_PPC)
#define OS_TRAP_Os_SetPrivilegedMode(_a1)
#endif // CFG_PPC

#if defined(CFG_TC2XX)
#define OS_TRAP_Os_ArchToPrivilegedMode(_a1)
#endif // CFG_TC2XX

#if defined(CFG_TMS570)
#define OS_TRAP_Os_ArchToPrivilegedMode()
#if defined(USE_WDG)
#define OS_TRAP_Wdg_Hw_KickWdg                  Wdg_Hw_KickWdg
#define OS_TRAP_Wdg_Hw_SetTriggerCondition      Wdg_Hw_SetTriggerCondition
#endif // USE_WDG
#endif // CFG_TMS570

#define OS_TRAP_Mcu_Hw_PerformReset             Mcu_Hw_PerformReset
#endif /* (OS_SC3 == STD_ON ) || (OS_SC4 == STD_ON ) */
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/
/** @} */

#endif
