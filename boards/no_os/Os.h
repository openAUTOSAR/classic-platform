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


#ifndef _OS_H_
#define _OS_H_

#define OS_AR_RELEASE_MAJOR_VERSION           4u
#define OS_AR_RELEASE_MINOR_VERSION           0u
#define OS_AR_RELEASE_REVISION_VERSION        3u

/* @req SWS_BSW_00059 Published information */
#define OS_SW_MAJOR_VERSION   1u
#define OS_SW_MINOR_VERSION   0u
#define OS_SW_PATCH_VERSION   1u


#include "Std_Types.h"
#include "MemMap.h"
#include "Os_Cfg.h"

//#define OSTICKDURATION 1000000UL


#define E_OS_EXIT_ABORT              (StatusType)30    /* ArcCore */
#define E_OS_ASSERT                  (StatusType)37    /* ArcCore, call to ASSERT() or CONFIG_ASSERT() */

typedef uint32 ApplicationType;
typedef uint32 CounterType;
typedef uint8 StatusType;
typedef    sint16 ISRType;
typedef uint32  AreaIdType;

typedef uint32 *TickRefType;
typedef uint32 TickType;
typedef uint64            EventMaskType;
typedef uint16            TaskType;
typedef sint8 Os_IntCounterType;

StatusType GetElapsedValue( CounterType CounterID, TickRefType Value, TickRefType ElapsedValue );
extern uint16 EA_count;

static inline void SuspendOSInterrupts( void ) {
/* EA_count is incremented to get the count of number of times OS interrupts are suspended*/
    EA_count++;
}

static inline void ResumeOSInterrupts( void ) {
/* EA_count is decremented to get the count of number of times OS interrupts are resumed*/
    EA_count--;
}

void        ShutdownOS( StatusType );

#define SYS_CALL_EnterUserMode()
#define SYS_CALL_EnterSupervisorMode()
#define SYS_CALL_ResumeAllInterrupts()      ResumeAllInterrupts()
#define SYS_CALL_SuspendAllInterrupts()     SuspendAllInterrupts()

#define SYS_CALL_EnableAllInterrupts()      EnableAllInterrupts()
#define SYS_CALL_DisableAllInterrupts()     DisableAllInterrupts()

#define SYS_CALL_ResumeOSInterrupts()       ResumeOSInterrupts()
#define SYS_CALL_SuspendOSInterrupts()      SuspendOSInterrupts()

#define SYS_CALL_ActivateTask                ActivateTask
#define SYS_CALL_TerminateTask               TerminateTask
#define SYS_CALL_ChainTask                   ChainTask
#define SYS_CALL_Schedule                    Schedule
#define SYS_CALL_GetTaskID                   GetTaskID
#define SYS_CALL_GetResource                 GetResource
#define SYS_CALL_ReleaseResource             ReleaseResource
#define SYS_CALL_SetEvent                    SetEvent
#define SYS_CALL_ClearEvent                  ClearEvent
#define SYS_CALL_GetEvent                    GetEvent
#define SYS_CALL_WaitEvent                   WaitEvent
#define SYS_CALL_GetAlarmBase                GetAlarmBase
#define SYS_CALL_GetAlarm                    GetAlarm
#define SYS_CALL_SetRelAlarm                 SetRelAlarm
#define SYS_CALL_SetAbsAlarm                 SetAbsAlarm
#define SYS_CALL_CancelAlarm                 CancelAlarm
#define SYS_CALL_ShutdownOS                  ShutdownOS
#define SYS_CALL_GetTaskState                GetTaskState
#define SYS_CALL_GetISRID                    GetISRID
#define SYS_CALL_CheckISRMemoryAccess        CheckISRMemoryAccess
#define SYS_CALL_CheckTaskMemoryAccess       CheckTaskMemoryAccess
#define SYS_CALL_CheckObjectOwnership        CheckObjectOwnership
#define SYS_CALL_StartScheduleTableRel       StartScheduleTableRel
#define SYS_CALL_StartScheduleTableAbs       StartScheduleTableAbs
#define SYS_CALL_StopScheduleTable           StopScheduleTable
#define SYS_CALL_NextScheduleTable           NextScheduleTable
#define SYS_CALL_GetScheduleTableStatus      GetScheduleTableStatus
#define SYS_CALL_IncrementCounter            IncrementCounter
#define SYS_CALL_GetCounterValue             GetCounterValue
#define SYS_CALL_GetElapsedValue             GetElapsedValue
#define ISR(_isr)    						 void _isr( void )
StatusType SetEvent( TaskType TaskID, EventMaskType Mask );
#define SYS_CALL_AtomicCopy16(to, from) (to) = (from)

static inline StatusType  WritePeripheral32( AreaIdType  Area,  uint32  *Address, uint32 WriteValue) {
    *Address = WriteValue;
    return E_OK;
}



#endif /* _OS_H_ */
