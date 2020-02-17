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

#define OS_TRAP_OS_IDX          1ul

#define OS_TRAP_OS_IDX_SHIFTED          (OS_TRAP_OS_IDX << 16u)

/* Index to trap functions */
#define OS_TRAP_INDEX_Reserved                  (0UL)
#define OS_TRAP_INDEX_Os_ArchToPrivilegedMode   (-1UL)
#define OS_TRAP_INDEX_Irq_GenerateSoftInt       (-2UL)
#if defined(CFG_PPC)
#define OS_TRAP_INDEX_Os_SetPrivilegedMode      (0u + OS_TRAP_OS_IDX_SHIFTED)
#endif

#if defined(CFG_TMS570)
#define OS_ARM_SERVICE_EXCEPTION_MAP            (0x80UL) // Masking variable with 8th bit set to differentiate array index between Os_TrapList and Os_ServiceList
#if defined(USE_WDG)
#define OS_TRAPLIST_INDEX_Wdg_Hw_KickWdg                    (0x3UL)  // Array index for Wdg_Hw_KickWdg function in Os_TrapList
#define OS_TRAPLIST_INDEX_Wdg_Hw_SetTriggerCondition        (0x4UL)
#define OS_TRAPLIST_INDEX_Mcu_Hw_PerformReset               (0x5UL)
#define OS_TRAP_INDEX_Wdg_Hw_KickWdg                        (OS_ARM_SERVICE_EXCEPTION_MAP | OS_TRAPLIST_INDEX_Wdg_Hw_KickWdg)
#define OS_TRAP_INDEX_Wdg_Hw_SetTriggerCondition            (OS_ARM_SERVICE_EXCEPTION_MAP | OS_TRAPLIST_INDEX_Wdg_Hw_SetTriggerCondition)
#define OS_TRAP_INDEX_Mcu_Hw_PerformReset                   (OS_ARM_SERVICE_EXCEPTION_MAP | OS_TRAPLIST_INDEX_Mcu_Hw_PerformReset)
#endif
#endif
/* ----------------------------[public define]------------------------------*/
/* OS trap internal functions */
#define OS_TRAP_Os_ArchToPrivilegedMode()      SYS_CALL_0( OS_TRAP_INDEX_Os_ArchToPrivilegedMode )
#define OS_TRAP_Irq_GenerateSoftInt(_a1)       SYS_CALL_1( OS_TRAP_INDEX_Irq_GenerateSoftInt,_a1 )
#if defined(CFG_PPC)
#define OS_TRAP_Os_SetPrivilegedMode(_a1)      SYS_CALL_0( OS_TRAP_INDEX_Os_SetPrivilegedMode )
#endif

#if defined(CFG_TMS570) && defined(USE_WDG)
#define OS_TRAP_Wdg_Hw_KickWdg()                    SYS_CALL_0( OS_TRAP_INDEX_Wdg_Hw_KickWdg )
#define OS_TRAP_Wdg_Hw_SetTriggerCondition(_a1)     SYS_CALL_1( OS_TRAP_INDEX_Wdg_Hw_SetTriggerCondition, _a1 )
#define OS_TRAP_Mcu_Hw_PerformReset()               SYS_CALL_0( OS_TRAP_INDEX_Mcu_Hw_PerformReset )
#endif

#else  /* for OS_SC1 or OS_SC2 */
#if defined(CFG_TMS570) && defined(USE_WDG)
#define OS_TRAP_Wdg_Hw_KickWdg                  Wdg_Hw_KickWdg
#define OS_TRAP_Wdg_Hw_SetTriggerCondition      Wdg_Hw_SetTriggerCondition
#endif /* CFG_TMS570 and USE_WDG */
#define OS_TRAP_Mcu_Hw_PerformReset             Mcu_Hw_PerformReset
#endif /* (OS_SC3 == STD_ON ) || (OS_SC4 == STD_ON ) */
/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/
/** @} */
