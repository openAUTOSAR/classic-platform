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
 * @file    os_trap.c
 * @brief   Internal trap interface for the OS used for OS_SC3 and OS_SC4.
 *
 * @addtogroup os
 * @details
 * @{
 */

/* ----------------------------[includes]------------------------------------*/
#include "os_trap.h"


#if (OS_SC3 == STD_ON ) || (OS_SC4 == STD_ON )

#if defined(CFG_PPC)
extern void Os_SetPrivilegedMode( void );
#endif
#if defined(CFG_TMS570)
extern void Os_ArchToPrivilegedMode(void);
#endif
#if defined(CFG_TC2XX)
extern void Os_ArchToPrivilegedMode(uint32 pcxi);
#endif

/* ----------------------------[private define]------------------------------*/

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(_x)  sizeof(_x)/sizeof((_x)[0])
#endif

/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/
/* ----------------------------[private function prototypes]-----------------*/

#if defined(CFG_TMS570) && defined(USE_WDG)
extern void Wdg_Hw_KickWdg(void);
extern void Wdg_Hw_SetTriggerCondition(uint16 timeout);
extern void Mcu_Hw_PerformReset(void);
#endif


/* ----------------------------[private variables]---------------------------*/
/*lint -e611  MISRA:FALSE_POSITIVE:Array dimension:[MISRA 2004 Info, advisory] */
/*lint -e9054 MISRA:FALSE_POSITIVE:Array dimension:[MISRA 2012 Rule 9.5, required] */
const void * Os_TrapList[] = {
#if defined(CFG_PPC)
        [0] = (void *)Os_SetPrivilegedMode,   /* ASM call */
        [1] = (void *)NULL,
#else
        [0] = (void *)NULL,   /* ASM call */
        [1] = (void *)Os_ArchToPrivilegedMode,
#endif
		[2] = (void *)Irq_GenerateSoftInt,

#if defined(CFG_TMS570) && defined(USE_WDG)
		[3] = (void *)Wdg_Hw_KickWdg,
		[4] = (void *)Wdg_Hw_SetTriggerCondition,
		[5] = (void *)Mcu_Hw_PerformReset,
#endif
};
/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/


struct ServiceEntry {
    void    *entry;
    uint32  cnt;
};

extern void * Os_ServiceList[];


const struct ServiceEntry Os_GblServiceList[] = {
    {
        (void *)Os_ServiceList,
        SYS_CALL_CNT,
    },
    {
        (void *)Os_TrapList,
        ARRAY_SIZE(Os_TrapList),
    },
};

#endif /* (OS_SC3 == STD_ON ) || (OS_SC4 == STD_ON ) */


/** @} */
