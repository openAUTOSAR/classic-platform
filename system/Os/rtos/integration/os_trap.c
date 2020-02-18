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
#include <stdio.h>
#include "os_peripheral_i.h"
#include "isr.h"


#if (OS_SC3 == STD_ON ) || (OS_SC4 == STD_ON )

#if defined(CFG_PPC) || defined(CFG_TMS570)
extern void Os_SetPrivilegedMode( void );
#elif defined(CFG_TC2XX)
extern void Os_ArchToPrivilegedMode(uint32 pcxi);
#endif

/* ----------------------------[private define]------------------------------*/

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(_x)  (sizeof(_x)/sizeof((_x)[0]))
#endif

/* ----------------------------[private macro]-------------------------------*/
/* ----------------------------[private typedef]-----------------------------*/


struct ServiceEntry {
    /*lint -e970 MISRA:OTHER:type int declared outside:[MISRA 2012 Directive 4.6, advisory] */
    int* (*entry)[];
    uint32  cnt;
};

/* ----------------------------[private function prototypes]-----------------*/
/* ----------------------------[private variables]---------------------------*/
/*lint -save -e9054 MISRA:FALSE_POSITIVE:Array dimension:[MISRA 2012 Rule 9.5, required] */
/*lint -e611 MISRA:OTHER:file inclusion:[MISRA 2004 Info, advisory] */
void * Os_ServiceList[] = {
        /* void and context altering functions */
        [SYS_CALL_INDEX_EnterSupervisorMode] = (void *)NULL,
        [SYS_CALL_INDEX_ResumeAllInterrupts] = (void *)ResumeAllInterrupts ,
        [SYS_CALL_INDEX_SuspendAllInterrupts] = (void *)SuspendAllInterrupts ,
        [SYS_CALL_INDEX_EnableAllInterrupts] = (void *)EnableAllInterrupts ,
        [SYS_CALL_INDEX_DisableAllInterrupts] = (void *)DisableAllInterrupts,
        [SYS_CALL_INDEX_ResumeOSInterrupts] = (void *)ResumeOSInterrupts ,
        [SYS_CALL_INDEX_SuspendOSInterrupts] = (void *)SuspendOSInterrupts ,
        /* StatusType returning functions */
        [SYS_CALL_INDEX_ActivateTask] = (void *)ActivateTask,
        [SYS_CALL_INDEX_TerminateTask] = (void *)TerminateTask,
        [SYS_CALL_INDEX_ChainTask] = (void *)ChainTask,
        [SYS_CALL_INDEX_Schedule] = (void *)Schedule,
        [SYS_CALL_INDEX_GetTaskID] = (void *)GetTaskID,
        [SYS_CALL_INDEX_GetResource] = (void *)GetResource,
        [SYS_CALL_INDEX_ReleaseResource] = (void *)ReleaseResource,
        [SYS_CALL_INDEX_SetEvent] = (void *)SetEvent,
        [SYS_CALL_INDEX_ClearEvent] = (void *)ClearEvent,
        [SYS_CALL_INDEX_GetEvent] = (void *)GetEvent,
        [SYS_CALL_INDEX_WaitEvent] = (void *)WaitEvent,
#if (OS_ALARM_CNT != 0)
        [SYS_CALL_INDEX_GetAlarmBase] = (void *)GetAlarmBase,
        [SYS_CALL_INDEX_GetAlarm] = (void *)GetAlarm,
        [SYS_CALL_INDEX_SetRelAlarm] = (void *)SetRelAlarm,
        [SYS_CALL_INDEX_SetAbsAlarm] = (void *)SetAbsAlarm,
        [SYS_CALL_INDEX_CancelAlarm] = (void *)CancelAlarm,
#endif
        [SYS_CALL_INDEX_StartOS] = (void *)StartOS,
        [SYS_CALL_INDEX_ShutdownOS] = (void *)ShutdownOS,
        [SYS_CALL_INDEX_GetTaskState] = (void *)GetTaskState,
#if ( OS_SC3 == STD_ON ) || ( OS_SC4 == STD_ON )
        [SYS_CALL_INDEX_GetApplicationID] = (void *)GetApplicationID,
#endif
        [SYS_CALL_INDEX_GetISRID] = (void *)GetISRID,
        [SYS_CALL_INDEX_CheckISRMemoryAccess] = (void *)CheckISRMemoryAccess,
        [SYS_CALL_INDEX_CheckTaskMemoryAccess] = (void *)CheckTaskMemoryAccess,
#if ( OS_SC3 == STD_ON ) || ( OS_SC4 == STD_ON )
        [SYS_CALL_INDEX_CheckObjectAccess] = (void *)CheckObjectAccess,
#endif
        [SYS_CALL_INDEX_CheckObjectOwnership] = (void *)CheckObjectOwnership,
        [SYS_CALL_INDEX_StartScheduleTableRel] = (void *)StartScheduleTableRel,
        [SYS_CALL_INDEX_StartScheduleTableAbs] = (void *)StartScheduleTableAbs,
        [SYS_CALL_INDEX_StopScheduleTable] = (void *)StopScheduleTable,
        [SYS_CALL_INDEX_NextScheduleTable] = (void *)NextScheduleTable,
#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
        [SYS_CALL_INDEX_StartScheduleTableSynchron] = (void *)StartScheduleTableSynchron,
        [SYS_CALL_INDEX_SyncScheduleTable] = (void *)SyncScheduleTable,
        [SYS_CALL_INDEX_SetScheduleTableAsync] = (void *)SetScheduleTableAsync,
#endif
        [SYS_CALL_INDEX_GetScheduleTableStatus] = (void *)GetScheduleTableStatus,
        [SYS_CALL_INDEX_IncrementCounter] = (void *)IncrementCounter,
        [SYS_CALL_INDEX_GetCounterValue] = (void *)GetCounterValue,
        [SYS_CALL_INDEX_GetElapsedValue] = (void *)GetElapsedValue,

        [SYS_CALL_INDEX_ReadPeripheral8] = (void *)Os_ReadPeripheral8,
        [SYS_CALL_INDEX_ReadPeripheral16] = (void *)Os_ReadPeripheral16,
        [SYS_CALL_INDEX_ReadPeripheral32] = (void *)Os_ReadPeripheral32,
        [SYS_CALL_INDEX_WritePeripheral8] = (void *)Os_WritePeripheral8,
        [SYS_CALL_INDEX_WritePeripheral16] = (void *)Os_WritePeripheral16,
        [SYS_CALL_INDEX_WritePeripheral32] = (void *)Os_WritePeripheral32,
        [SYS_CALL_INDEX_ModifyPeripheral8] = (void *)Os_ModifyPeripheral8,
        [SYS_CALL_INDEX_ModifyPeripheral16] = (void *)Os_ModifyPeripheral16,
        [SYS_CALL_INDEX_ModifyPeripheral32] = (void *)Os_ModifyPeripheral32,

        /* Clib functions */
        [SYS_CALL_INDEX_write] = (void *)write ,
};


/*lint -e611  MISRA:FALSE_POSITIVE:Array dimension:[MISRA 2004 Info, advisory] */
/*lint -e9054 MISRA:FALSE_POSITIVE:Array dimension:[MISRA 2012 Rule 9.5, required] */
const void * Os_TrapList[] = {
#if defined(CFG_PPC) || defined(CFG_TMS570)
        [0] = (void *)Os_SetPrivilegedMode,   /* ASM call */
        [1] = (void *)NULL,
#else
        [0] = (void *)NULL,   /* ASM call */
        [1] = (void *)Os_ArchToPrivilegedMode,
#endif
        [2] = (void *)Irq_GenerateSoftInt,
#if defined(CFG_TMS570)
        [3] = (void *)Irq_AckSoftInt,
#endif
        [4] = (void *)Os_IsrAdd,

};

/*lint -esym(9003, Os_GblServiceList) MISRA:OTHER:Readability:[MISRA 2012 Rule 8.9, advisory] */
#if defined(CFG_QTST_TRAP)
#define QTST_TRAP_IMPL
#include "qtst_trap.h"
#endif

#if defined(CFG_SAFETY_PLATFORM) && defined(CFG_TMS570) && defined(USE_FLS)
#define FLS_TMS570_TRAP_IMPL
#include "Fls_tms570_trap.h"
#endif


const struct ServiceEntry Os_GblServiceList[] = {
    {
        (void *)Os_ServiceList,
        SYS_CALL_CNT,
    },
    {
        (void *)Os_TrapList,
        ARRAY_SIZE(Os_TrapList),
    },
#if defined(CFG_QTST_TRAP)
    [ OS_TRAP_QTST_IDX ] = {
        (void *)QTst_TrapList,
        ARRAY_SIZE(QTst_TrapList),
    },
#endif
#if defined(CFG_SAFETY_PLATFORM) && defined(CFG_TMS570) && defined(USE_FLS)
    [ OS_TRAP_FLS_IDX ] = {
        (void *)Fls_tms570_TrapList,
        ARRAY_SIZE(Fls_tms570_TrapList),
    },
#endif
};

/*lint -restore */
/* ----------------------------[private functions]---------------------------*/
/* ----------------------------[public functions]----------------------------*/

#if defined(CFG_TMS570)

void * Os_GetService( uint32 idx ) {
    uint32 tIdx = (idx >> 16u);
    uint32 sIdx = idx & 0xffffu;

    const struct ServiceEntry *se = &Os_GblServiceList[tIdx];

    ASSERT( (sIdx < se->cnt) && (tIdx < ARRAY_SIZE(Os_GblServiceList)) );

    return(*(se->entry))[sIdx];
}

#endif


#endif /* (OS_SC3 == STD_ON ) || (OS_SC4 == STD_ON ) */


/** @} */
