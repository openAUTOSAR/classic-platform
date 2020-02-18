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

/** @req SWS_BSW_00020 The header file structure shall contain one or more files 
 * that provide the declaration of functions from the BSW Module API
 */

#ifndef OS_H_
#define OS_H_

#define OS_AR_RELEASE_MAJOR_VERSION           4u
#define OS_AR_RELEASE_MINOR_VERSION           0u
#define OS_AR_RELEASE_REVISION_VERSION        3u

/* @req SWS_BSW_00059 Published information */
#define OS_SW_MAJOR_VERSION   1u
#define OS_SW_MINOR_VERSION   0u
#define OS_SW_PATCH_VERSION   1u

/* @req SWS_BSW_00024 Include AUTOSAR Standard Types Header in implementation header */
#include "Std_Types.h"
#include "arc_assert.h"
#if defined(CFG_OS_TEST)
#include "os_test.h"
#endif
#include "Os_Cfg.h"
#include "MemMap.h"
#include "Cpu.h"
#include "limits.h"
#include <sys/queue.h>
#include "Rte_Os_Type.h"

typedef uint8 StatusType;

#if (OS_ISR_CNT > OS_ISR_MAX_CNT)
#error OS configuration error. OS_ISR_MAX_CNT must be bigger or equal to OS_ISR_CNT
#endif

#define E_OS_ACCESS     (StatusType)1             /**< STD OSEK */
#define E_OS_CALLEVEL   (StatusType)2             /**< STD OSEK */
#define E_OS_ID         (StatusType)3             /**< STD OSEK */
#define E_OS_LIMIT      (StatusType)4             /**< STD OSEK */
#define E_OS_NOFUNC     (StatusType)5             /**< STD OSEK */
#define E_OS_RESOURCE   (StatusType)6             /**< STD OSEK */
#define E_OS_STATE      (StatusType)7             /**< STD OSEK */
#define E_OS_VALUE      (StatusType)8             /**< STD OSEK */

#define E_OS_SERVICEID              (StatusType)9      /**< AUTOSAR, see 7.10 */
#define E_OS_RATE                   (StatusType)10     /**< AUTOSAR, see 7.10 */
#define E_OS_ILLEGAL_ADDRESS        (StatusType)11     /**< AUTOSAR, see 7.10 */
#define E_OS_MISSINGEND             (StatusType)12     /**< AUTOSAR, see 7.10 */
#define E_OS_DISABLEDINT            (StatusType)13     /**< AUTOSAR, see 7.10 */
#define E_OS_STACKFAULT             (StatusType)14     /**< AUTOSAR, see 7.10 */
#define E_OS_PROTECTION_MEMORY      (StatusType)15     /**< AUTOSAR, see 7.10 */
#define E_OS_PROTECTION_TIME        (StatusType)16     /**< AUTOSAR, see 7.10 */
#define E_OS_PROTECTION_LOCKED      (StatusType)17     /**< AUTOSAR, see 7.10 */
#define E_OS_PROTECTION_EXCEPTION   (StatusType)18     /**< AUTOSAR, see 7.10 */
#define E_OS_PROTECTION_RATE        (StatusType)19     /**< AUTOSAR, see 7.10 */
#define E_OS_CORE                   (StatusType)20     /**< AUTOSAR 4.0.3, see 7.13 */
#define E_OS_SPINLOCK               (StatusType)21     /**< AUTOSAR 4.0.3, see 7.13 */
#define E_OS_INTERFERENCE_DEADLOCK  (StatusType)22     /**< AUTOSAR 4.0.3, see 7.13 */
#define E_OS_NESTING_DEADLOCK       (StatusType)23     /**< AUTOSAR 4.0.3, see 7.13 */
#define E_OS_PARAM_POINTER           (StatusType)24    /**< AUTOSAR 4.3.0, see 7.1.2.3 */

#define E_OS_EXIT_ABORT              (StatusType)30    /* ArcCore */
#define E_OS_PANIC                   (StatusType)31    /* ArcCore */
#define E_OS_RTE                     (StatusType)32    /* ArcCore */
#define E_OS_TRUSTED_INDEX           (StatusType)33    /* ArcCore, index out of bounds */
#define E_OS_TRUSTED_FUNC            (StatusType)34    /* ArcCore, called function is NULL */
#define E_OS_BAD_CONTEXT             (StatusType)35    /* ArcCore, called function is NULL */
#define E_OS_TIMEOUT                 (StatusType)36    /* ArcCore, Semaphore have timeout */
#define E_OS_ASSERT                  (StatusType)37    /* ArcCore, call to ASSERT() or CONFIG_ASSERT() */
#define E_OS_ISR_RESOURCE            (StatusType)38    /* ArcCore, ISR does not support Get/ReleaseResource() */
#define E_OS_FULL                    (StatusType)39    /* ArcCore, mbox full */

#define E_OS_ILLEGAL                 (StatusType)255   /* ArcCore*/



#define E_COM_ID 255 // NOTE: The value of E_COM_ID is not known"

#if (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)
/* 32bit: Keeping Load/Store and register manipulation simple
 * during SysCalls when EventMaskType as fun argument  */
typedef uint32            EventMaskType;
#else
typedef uint64            EventMaskType;
#endif
typedef EventMaskType *   EventMaskRefType;
typedef uint16            TaskType;
typedef TaskType *        TaskRefType;
typedef uint16            AreaIdType;

typedef enum {
    TASK_STATE_WAITING,
    TASK_STATE_READY,
    TASK_STATE_SUSPENDED,
    TASK_STATE_RUNNING,
    TASK_STATE_WAITING_SEM
} TaskStateType;

#define INVALID_TASK    0xdeadU

typedef TaskStateType *TaskStateRefType;

#define OSMEMORY_IS_READABLE(_access)
#define OSMEMORY_IS_WRITEABLE(_access)
#define OSMEMORY_IS_EXECUTABLE(_access)
#define OSMEMORY_IS_STACKSPACE(_access)

#define OSDEFAULTAPPMODE  1

#define INVALID_OSAPPLICATION (0xFFFFFFFFUL)

#define TASK(_task)        void _task( void )

//lint -esym(123,ISR)  Inhibit lint warning for ISR being defined elsewhere without being an error
#define ISR(_isr)     void _isr( void )


#if (OS_USE_APPLICATIONS == STD_ON)

/* @req SWS_Os_00772 Type definition */
typedef uint32 ApplicationType;

/* @req SWS_Os_00773 Type definition */
typedef enum {
    APPLICATION_ACCESSIBLE,
    APPLICATION_RESTARTING,
    APPLICATION_TERMINATED
} ApplicationStateType;

/* @req SWS_Os_00774 Type definition */
typedef ApplicationStateType *ApplicationStateRefType;
#endif

/* See oil config for defines */
typedef uint32 AppModeType; /* @req SWS_Os_91007 Type definition */
typedef uint16 ScheduleTableType; /* @req SWS_Os_00783 Type definition */
typedef uint16 GlobalTimeTickType;

/* @req SWS_Os_00784 Type definition */
typedef enum {
    SCHEDULETABLE_STOPPED,
    SCHEDULETABLE_NEXT,
    SCHEDULETABLE_WAITING,
    SCHEDULETABLE_RUNNING,
    SCHEDULETABLE_RUNNING_AND_SYNCHRONOUS
} ScheduleTableStatusType;

/* @req SWS_Os_00785 Type definition */
typedef ScheduleTableStatusType *ScheduleTableStatusRefType;

/* Flags for AccessType */
#define ACCESSTYPE_W    1
#define ACCESSTYPE_R    (1<<1)

/* @req SWS_Os_00777 Type definition */
typedef uint32 AccessType;

/* @req SWS_Os_00778 Type definition */
typedef enum {
    ACCESS,
    NO_ACCESS
} ObjectAccessType;

/* @req SWS_Os_00779 Type definition */
typedef enum {
    OBJECT_TASK,
    OBJECT_ISR,
    OBJECT_ALARM,
    OBJECT_RESOURCE,
    OBJECT_COUNTER,
    OBJECT_SCHEDULETABLE,
    OBJECT_MESSAGE
} ObjectTypeType;

/* @req SWS_Os_00787 Type definition */
typedef enum {
    PRO_IGNORE,
    PRO_KILLTASKISR,
    PRO_KILLAPPL,
    PRO_KILLAPPL_RESTART,
    PRO_SHUTDOWN
} ProtectionReturnType;

/* @req SWS_Os_00788 Type definition */
typedef uint8 RestartType;

#define RESTART         0
#define NO_RESTART        1

#if (OS_SC2 == STD_ON) || (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)
/** @req SWS_Os_00542 */ /* ProtectionHook( ) available in Scalability Classes 2, 3 and 4. */
/** @req SWS_Os_00538 */
ProtectionReturnType ProtectionHook( StatusType FatalError );
#endif
void StartupHook( void );
void ShutdownHook( StatusType Error );
void ErrorHook( StatusType Error );
void PreTaskHook( void );
void PostTaskHook( void );

/* @req SWS_Os_00775 Type definition */
typedef uint16 TrustedFunctionIndexType;

/* @req SWS_Os_00776 Type definition */
typedef  void * TrustedFunctionParameterRefType;

/* See 8.3.9 */
#define INVALID_ISR        ((sint16)(-1))

/* @req SWS_Os_00782 Type definition */
typedef    sint16 ISRType;

#define APP_NO_OWNER    (-1UL)

/* @req SWS_Os_00780 Type definition */
typedef void * MemoryStartAddressType;

/* @req SWS_Os_00781 Type definition */
typedef uint32 MemorySizeType;

#include "os_peripheral_i.h"

StatusType WaitEvent( EventMaskType Mask );
StatusType SetEvent( TaskType TaskID, EventMaskType Mask );
StatusType ClearEvent( EventMaskType Mask);
StatusType GetEvent( TaskType TaskId, EventMaskRefType Mask);

void InitOS( void );
void StartOS( AppModeType Mode );

ISRType GetISRID( void );

#if !defined(CFG_SAFETY_PLATFORM)
StatusType GetActiveApplicationMode( AppModeType* mode);
#endif //CFG_SAFETY_PLATFORM

typedef sint8 Os_IntCounterType;

/* requirements here: OS368(ISR2), OS092 */

extern Os_IntCounterType Os_IntDisableAllCnt;
extern Os_IntCounterType Os_IntSuspendAllCnt;
/*lint -esym(9003,Os_Arc_OsTickCounter) MISRA:OTHER:variable defined in generated code so it cannot be defined in block scope:[MISRA 2012 Rule 8.9, advisory]*/
extern CounterType Os_Arc_OsTickCounter; 

/** @req SWS_Os_00299 */
/* OSEK: States that " service  does  not  support  nesting", but says
 * nothing what to do about it.
 *
 * OS092: "  If EnableAllInterrupts()/ResumeAllInterrupts()/ResumeOSInterrupts()
 * are called and no corresponding DisableAllInterupts()/SuspendAllInterrupts()/
 * SuspendOSInterrupts()  was  done  before,  the Operating System shall not
 */

/* @req OSEK_SWS_ISR_00002 */
static inline void DisableAllInterrupts( void ) {
    Irq_Disable();

#if defined(CFG_JAC6_IPC)
    /* We have to allow disabling interrupts several times from the IPC library */
    Os_IntDisableAllCnt=1;
#else
    /* No nesting allowed */
    Os_IntDisableAllCnt++;
    ASSERT(Os_IntDisableAllCnt==1);
#endif
}

/* @req OSEK_SWS_ISR_00001 */
static inline void EnableAllInterrupts( void ) {

    if(Os_IntDisableAllCnt==0) {
        /** @req SWS_Os_00092 EnableAllInterrupts */
    } else {
        Os_IntDisableAllCnt--;
#if defined(CFG_JAC6_IPC)
        Os_IntDisableAllCnt=0;
        __asm volatile ("CPSIE if");
#else
        Irq_Enable();
#endif
    }
}

/* @req OSEK_SWS_ISR_00004 */
static inline void SuspendAllInterrupts( void ) {
    Irq_SuspendAll();
    Os_IntSuspendAllCnt++;
}

/* @req OSEK_SWS_ISR_00003 */
static inline void ResumeAllInterrupts( void ) {

    if(Os_IntSuspendAllCnt==0) {
        /** @req SWS_Os_00092 ResumeAllInterrupts */
    } else {
        Os_IntSuspendAllCnt--;
        if( Os_IntSuspendAllCnt == 0 ) {
            Irq_ResumeAll();
        }
    }
}

/* Only ISR2 interrupts should be suspended but this can NEVER be
 * done in a more efficient way than to disable all, so let's
 * do that for all targets.
 */

/* @req OSEK_SWS_ISR_00006 */
static inline void SuspendOSInterrupts( void ) {
    Irq_SuspendOs();
    Os_IntSuspendAllCnt++;
}

/* @req OSEK_SWS_ISR_00005 */
static inline void ResumeOSInterrupts( void ) {

    if(Os_IntSuspendAllCnt==0) {
        /** @req SWS_Os_00092 ResumeOSInterrupts */
    } else {
        Os_IntSuspendAllCnt--;
        if(Os_IntSuspendAllCnt==0) {
            Irq_ResumeOs();
        }
    }
}

/*
 * Class 2,3 and 4 API
 */


#if (OS_SC1 == STD_ON) || (OS_SC2 == STD_ON)

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

StatusType  Os_ReadPeripheral8(    AreaIdType  Area,  const  uint8  * Address, uint8  * ReadValue);
StatusType  Os_ReadPeripheral16(   AreaIdType  Area,  const  uint16 * Address, uint16 * ReadValue);
StatusType  Os_ReadPeripheral32(   AreaIdType  Area,  const  uint32 * Address, uint32 * ReadValue);

StatusType  Os_WritePeripheral8 (  AreaIdType  Area,  uint8   *Address, uint8  WriteValue);
StatusType  Os_WritePeripheral16(  AreaIdType  Area,  uint16  *Address, uint16 WriteValue);
StatusType  Os_WritePeripheral32(  AreaIdType  Area,  uint32  *Address, uint32 WriteValue);

StatusType  Os_ModifyPeripheral8(  AreaIdType  Area,  uint8   *Address, uint8  ClearMask, uint8 SetMask);
StatusType  Os_ModifyPeripheral16(  AreaIdType  Area,  uint16   *Address, uint16  ClearMask, uint16 SetMask);
StatusType  Os_ModifyPeripheral32(  AreaIdType  Area,  uint32   *Address, uint32  ClearMask, uint32 SetMask);


#define ReadPeripheral8(_a1,_a2,_a3)        Os_ReadPeripheral8( _a1, _a2, _a3 )
#define ReadPeripheral16(_a1,_a2,_a3)       Os_ReadPeripheral16(_a1, _a2, _a3 )
#define ReadPeripheral32(_a1,_a2,_a3)       Os_ReadPeripheral32( _a1, _a2, _a3 )
#define WritePeripheral8(_a1,_a2,_a3)       Os_WritePeripheral8( _a1, _a2, _a3 )
#define WritePeripheral16(_a1,_a2,_a3)      Os_WritePeripheral16(_a1, _a2, _a3 )
#define WritePeripheral32(_a1,_a2,_a3)      Os_WritePeripheral32( _a1, _a2, _a3 )
#define ModifyPeripheral8(_a1,_a2,_a3,_a4)  Os_ModifyPeripheral8( _a1, _a2, _a3, _a4 )
#define ModifyPeripheral16(_a1,_a2,_a3,_a4) Os_ModifyPeripheral16(_a1, _a2, _a3, _a4 )
#define ModifyPeripheral32(_a1,_a2,_a3,_a4) Os_ModifyPeripheral32( _a1, _a2, _a3, _a4 )

#elif (OS_SC3 == STD_ON) || (OS_SC4 == STD_ON)
/* The RTE uses SYS_CALL_xxx always, so map them to the OS functions */

extern StatusType Os_ArchSysCall();
extern StatusType Os_ArchSysCall_4(uint32 a, uint32 b, uint32 c, uint32 d);
extern StatusType Os_ArchSysCall_5(uint32 a, uint32 b, uint32 c, uint32 d, uint32 e);
#if defined(CFG_TC2XX)
extern StatusType Os_ArchSysCall_4_P1(uint32 * a, uint32 b, uint32 c, uint32 d);
extern StatusType Os_ArchSysCall_4_P2(uint32 a, uint32 * b, uint32 c, uint32 d);
extern StatusType Os_ArchSysCall_4_P2_3(uint32 a, uint32 * b, uint32 * c, uint32 d);
extern StatusType Os_ArchSysCall_4_P2_3(uint32 a, uint32 * b, uint32 * c, uint32 d);
extern StatusType Os_ArchSysCall_4_S_P_P(uint32 a, uint32 * b, uint32 *c, uint32 d);
extern StatusType Os_ArchSysCall_4_S_P_S(uint32 a, uint32 * b, uint32 c, uint32 d);
extern StatusType Os_ArchSysCall_4_S_P_S_S(uint32 a, uint32 * b, uint32 c, uint32 d, uint32 e);

extern int Os_ArchSysCall_4_P_S(uint32 *a, uint32 b, uint32 c, uint32 d);
#endif
extern StatusType Os_ArchSysCall_4_S(uint32 a, uint64, uint32 d);
extern StatusType Os_SysCall_0(uint32 index);

#if defined(CFG_ARM)
extern StatusType Os_ArchSysCall_8(uint32 a1, uint32 a2, uint32 a3, uint32 a4, uint32 a5, uint32 a6,uint32 a7,uint32 a8 );
#endif


#define SYS_CALL_0(index)                            Os_ArchSysCall_4(0,0,0,index)
#define SYS_CALL_1(index,_a1)                        Os_ArchSysCall_4((uint32)(_a1),0,0,index)
#define SYS_CALL_2(index,_a1,_a2)                    Os_ArchSysCall_4((uint32)_a1,(uint32)_a2,0,index)
#define SYS_CALL_3(index,_a1,_a2,_a3)                Os_ArchSysCall_4((uint32)_a1,(uint32)_a2,(uint32)_a3,index)
#define SYS_CALL_4(index,_a1,_a2,_a3,_a4)            Os_ArchSysCall_5((uint32)_a1,(uint32)_a2,(uint32)_a3,(uint32)_a4,index)
#if defined(CFG_ARM)
#define SYS_CALL_8(index,_a1,_a2,_a3,_a4,_a5,_a6,_a7 )   \
            Os_ArchSysCall_8((uint32)_a1,(uint32)_a2,(uint32)_a3,(uint32)_a4,(uint32)_a5,(uint32)_a6,(uint32)_a7,index)
#endif

#if defined(CFG_TC2XX)
#define SYS_CALL_1_P1(index,_a1)                     Os_ArchSysCall_4_P1((uint32 *)_a1,0,0,index)
#define SYS_CALL_2_P2(index,_a1,_a2)                 Os_ArchSysCall_4_P2((uint32)_a1,(uint32 *)_a2,0,index)
#define SYS_CALL_3_P2_3(index,_a1,_a2,_a3)           Os_ArchSysCall_4_P2_3((uint32)_a1,(uint32 *)_a2,(uint32 *)_a3,index)
#define SYS_CALL_3_S_P_S(index,_a1,_a2,_a3)          Os_ArchSysCall_4_S_P_S((uint32)_a1,(uint32 *)_a2,(uint32)_a3,index)
#define SYS_CALL_3_S_P_P(index,_a1,_a2,_a3)          Os_ArchSysCall_4_S_P_P((uint32)_a1,(uint32 *)_a2,(uint32 *)_a3,index)
#define SYS_CALL_4_S_P_S_S(index,_a1,_a2,_a3,_a4)    Os_ArchSysCall_4_S_P_S_S((uint32)_a1,(uint32 *)_a2,(uint32)_a3,(uint32)_a4, index)
#endif

#define SYS_CALL_INDEX_EnterSupervisorMode           0
#define SYS_CALL_INDEX_ResumeAllInterrupts           1
#define SYS_CALL_INDEX_SuspendAllInterrupts          2
#define SYS_CALL_INDEX_EnableAllInterrupts           3
#define SYS_CALL_INDEX_DisableAllInterrupts          4
#define SYS_CALL_INDEX_ResumeOSInterrupts            5
#define SYS_CALL_INDEX_SuspendOSInterrupts           6
#define SYS_CALL_INDEX_ActivateTask                  7
#define SYS_CALL_INDEX_TerminateTask                 8
#define SYS_CALL_INDEX_ChainTask                     9
#define SYS_CALL_INDEX_Schedule                      10
#define SYS_CALL_INDEX_GetTaskID                     11
#define SYS_CALL_INDEX_GetResource                   12
#define SYS_CALL_INDEX_ReleaseResource               13
#define SYS_CALL_INDEX_SetEvent                      14
#define SYS_CALL_INDEX_ClearEvent                    15
#define SYS_CALL_INDEX_GetEvent                      16
#define SYS_CALL_INDEX_WaitEvent                     17
#define SYS_CALL_INDEX_GetAlarmBase                  18
#define SYS_CALL_INDEX_GetAlarm                      19
#define SYS_CALL_INDEX_SetRelAlarm                   20
#define SYS_CALL_INDEX_SetAbsAlarm                   21
#define SYS_CALL_INDEX_CancelAlarm                   22
#define SYS_CALL_INDEX_StartOS                       23
#define SYS_CALL_INDEX_ShutdownOS                    24
#define SYS_CALL_INDEX_GetTaskState                  25
#define SYS_CALL_INDEX_GetApplicationID              26
#define SYS_CALL_INDEX_GetISRID                      27
#define SYS_CALL_INDEX_CheckISRMemoryAccess          28
#define SYS_CALL_INDEX_CheckTaskMemoryAccess         29
#define SYS_CALL_INDEX_CheckObjectAccess             30
#define SYS_CALL_INDEX_CheckObjectOwnership          31
#define SYS_CALL_INDEX_StartScheduleTableRel         32
#define SYS_CALL_INDEX_StartScheduleTableAbs         33
#define SYS_CALL_INDEX_StopScheduleTable             34
#define SYS_CALL_INDEX_NextScheduleTable             35
#define SYS_CALL_INDEX_StartScheduleTableSynchron    36
#define SYS_CALL_INDEX_SyncScheduleTable             37
#define SYS_CALL_INDEX_GetScheduleTableStatus        38
#define SYS_CALL_INDEX_SetScheduleTableAsync         39
#define SYS_CALL_INDEX_IncrementCounter              40
#define SYS_CALL_INDEX_GetCounterValue               41
#define SYS_CALL_INDEX_GetElapsedValue               42
#define SYS_CALL_INDEX_ReadPeripheral8               43
#define SYS_CALL_INDEX_ReadPeripheral16              44
#define SYS_CALL_INDEX_ReadPeripheral32              45
#define SYS_CALL_INDEX_WritePeripheral8              46
#define SYS_CALL_INDEX_WritePeripheral16             47
#define SYS_CALL_INDEX_WritePeripheral32             48
#define SYS_CALL_INDEX_ModifyPeripheral8             49
#define SYS_CALL_INDEX_ModifyPeripheral16            50
#define SYS_CALL_INDEX_ModifyPeripheral32            51

#define SYS_CALL_INDEX_write                         52

#define SYS_CALL_CNT                                 53

#define SYS_CALL_EnterSupervisorMode()              (void)SYS_CALL_0( SYS_CALL_INDEX_EnterSupervisorMode )
#define SYS_CALL_ResumeAllInterrupts()              (void)SYS_CALL_0( SYS_CALL_INDEX_ResumeAllInterrupts )
#define SYS_CALL_SuspendAllInterrupts()             (void)SYS_CALL_0( SYS_CALL_INDEX_SuspendAllInterrupts )
#define SYS_CALL_EnableAllInterrupts()              (void)SYS_CALL_0( SYS_CALL_INDEX_EnableAllInterrupts )
#define SYS_CALL_DisableAllInterrupts()             (void)SYS_CALL_0( SYS_CALL_INDEX_DisableAllInterrupts )
#define SYS_CALL_ResumeOSInterrupts()               (void)SYS_CALL_0( SYS_CALL_INDEX_ResumeOSInterrupts )
#define SYS_CALL_SuspendOSInterrupts()              (void)SYS_CALL_0( SYS_CALL_INDEX_SuspendOSInterrupts )

#define SYS_CALL_ActivateTask(_a1)                  SYS_CALL_1( SYS_CALL_INDEX_ActivateTask,_a1 )
#define SYS_CALL_TerminateTask()                    SYS_CALL_0( SYS_CALL_INDEX_TerminateTask)
#define SYS_CALL_ChainTask(_a1)                     SYS_CALL_1( SYS_CALL_INDEX_ChainTask,_a1)
#define SYS_CALL_Schedule()                         SYS_CALL_0( SYS_CALL_INDEX_Schedule)
#if defined(CFG_TC2XX)
#define SYS_CALL_GetTaskID(_a1)                     SYS_CALL_1_P1( SYS_CALL_INDEX_GetTaskID,_a1)
#else
#define SYS_CALL_GetTaskID(_a1)                     SYS_CALL_1( SYS_CALL_INDEX_GetTaskID,_a1)
#endif
#define SYS_CALL_GetResource(_a1)                   SYS_CALL_1( SYS_CALL_INDEX_GetResource,_a1)
#define SYS_CALL_ReleaseResource(_a1)               SYS_CALL_1( SYS_CALL_INDEX_ReleaseResource,_a1)
#define SYS_CALL_SetEvent(_a1,_a2)                  SYS_CALL_2( SYS_CALL_INDEX_SetEvent,_a1, _a2)
#define SYS_CALL_ClearEvent(_a1)                    SYS_CALL_1( SYS_CALL_INDEX_ClearEvent,_a1)
#if defined(CFG_TC2XX)
#define SYS_CALL_GetEvent(_a1, _a2)                 SYS_CALL_2_P2( SYS_CALL_INDEX_GetEvent,_a1,_a2)
#else
#define SYS_CALL_GetEvent(_a1, _a2)                 SYS_CALL_2( SYS_CALL_INDEX_GetEvent,_a1,_a2)
#endif
#define SYS_CALL_WaitEvent(_a1)                     SYS_CALL_1( SYS_CALL_INDEX_WaitEvent,_a1)
#if defined(CFG_TC2XX)
#define SYS_CALL_GetAlarmBase(_a1, _a2)             SYS_CALL_2_P2( SYS_CALL_INDEX_GetAlarmBase,_a1, _a2)
#define SYS_CALL_GetAlarm(_a1, _a2)                 SYS_CALL_2_P2( SYS_CALL_INDEX_GetAlarm,_a1, _a2)
#else
#define SYS_CALL_GetAlarmBase(_a1, _a2)             SYS_CALL_2( SYS_CALL_INDEX_GetAlarmBase,_a1, _a2)
#define SYS_CALL_GetAlarm(_a1, _a2)                 SYS_CALL_2( SYS_CALL_INDEX_GetAlarm,_a1, _a2)
#endif
#define SYS_CALL_SetRelAlarm(_a1,_a2,_a3)           SYS_CALL_3( SYS_CALL_INDEX_SetRelAlarm,_a1, _a2, _a3)
#define SYS_CALL_SetAbsAlarm(_a1,_a2,_a3)           SYS_CALL_3( SYS_CALL_INDEX_SetAbsAlarm,_a1, _a2, _a3)
#define SYS_CALL_CancelAlarm(_a1)                   SYS_CALL_1( SYS_CALL_INDEX_CancelAlarm,_a1)
#define SYS_CALL_StartOS(_a1)                       SYS_CALL_1( SYS_CALL_INDEX_StartOS,_a1)
#define SYS_CALL_ShutdownOS(_a1)                    SYS_CALL_1( SYS_CALL_INDEX_ShutdownOS,_a1)
#if defined(CFG_TC2XX)
#define SYS_CALL_GetTaskState(_a1, _a2)             SYS_CALL_2_P2( SYS_CALL_INDEX_GetTaskState,_a1, _a2)
#else
#define SYS_CALL_GetTaskState(_a1, _a2)             SYS_CALL_2( SYS_CALL_INDEX_GetTaskState,_a1, _a2)
#endif
#if (( OS_SC3 == STD_ON ) || ( OS_SC4 == STD_ON ))
#define SYS_CALL_GetApplicationID()                 SYS_CALL_0( SYS_CALL_INDEX_GetApplicationID)
#endif
#define SYS_CALL_GetISRID()                         SYS_CALL_0( SYS_CALL_INDEX_GetISRID)
#define SYS_CALL_CheckISRMemoryAccess(_a1,_a2,_a3)  SYS_CALL_3( SYS_CALL_INDEX_CheckISRMemoryAccess,_a1, _a2, _a3)
#define SYS_CALL_CheckTaskMemoryAccess(_a1,_a2,_a3) SYS_CALL_3( SYS_CALL_INDEX_CheckTaskMemoryAccess,_a1, _a2, _a3)
#if (( OS_SC3 == STD_ON ) || ( OS_SC4 == STD_ON ))
#define SYS_CALL_CheckObjectAccess(_a1,_a2,_a3)     SYS_CALL_3( SYS_CALL_INDEX_CheckObjectAccess,_a1,_a2,_a3)
#endif
#define SYS_CALL_CheckObjectOwnership(_a1,_a2)      SYS_CALL_2( SYS_CALL_INDEX_CheckObjectOwnership,_a1,_a2)
#define SYS_CALL_StartScheduleTableRel(_a1,_a2)     SYS_CALL_2( SYS_CALL_INDEX_StartScheduleTableRel,_a1,_a2)
#define SYS_CALL_StartScheduleTableAbs(_a1,_a2)     SYS_CALL_2( SYS_CALL_INDEX_StartScheduleTableAbs,_a1,_a2)
#define SYS_CALL_StopScheduleTable(_a1)             SYS_CALL_1( SYS_CALL_INDEX_StopScheduleTable,_a1)
#define SYS_CALL_NextScheduleTable(_a1,_a2)         SYS_CALL_2( SYS_CALL_INDEX_NextScheduleTable,_a1,_a2)
#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
#define SYS_CALL_StartScheduleTableSynchron(_a1)    SYS_CALL_1( SYS_CALL_INDEX_StartScheduleTableSynchron,_a1)
#define SYS_CALL_SyncScheduleTable(_a1,_a2)         SYS_CALL_2( SYS_CALL_INDEX_SyncScheduleTable,_a1,_a2)
#define SYS_CALL_SetScheduleTableAsync(_a1)         SYS_CALL_1( SYS_CALL_INDEX_SetScheduleTableAsync,_a1)
#endif
#define SYS_CALL_GetScheduleTableStatus(_a1,_a2)    SYS_CALL_2( SYS_CALL_INDEX_GetScheduleTableStatus,_a1,_a2)
#define SYS_CALL_IncrementCounter(_a1)              SYS_CALL_1( SYS_CALL_INDEX_IncrementCounter,_a1)
#if defined(CFG_TC2XX)
#define SYS_CALL_GetCounterValue(_a1,_a2)           SYS_CALL_2_P2( SYS_CALL_INDEX_GetCounterValue,_a1,_a2)
#define SYS_CALL_GetElapsedValue(_a1,_a2,_a3)       SYS_CALL_3_P2_3( SYS_CALL_INDEX_GetElapsedValue,_a1,_a2,_a3)
#else
#define SYS_CALL_GetCounterValue(_a1,_a2)           SYS_CALL_2( SYS_CALL_INDEX_GetCounterValue,_a1,_a2)
#define SYS_CALL_GetElapsedValue(_a1,_a2,_a3)       SYS_CALL_3( SYS_CALL_INDEX_GetElapsedValue,_a1,_a2,_a3)
#endif

#if defined(CFG_TC2XX)
#define ReadPeripheral8(_a1,_a2,_a3)       SYS_CALL_3_S_P_P( SYS_CALL_INDEX_ReadPeripheral8, _a1, _a2, _a3 )
#define ReadPeripheral16(_a1,_a2,_a3)      SYS_CALL_3_S_P_P( SYS_CALL_INDEX_ReadPeripheral16, _a1, _a2, _a3 )
#define ReadPeripheral32(_a1,_a2,_a3)      SYS_CALL_3_S_P_P( SYS_CALL_INDEX_ReadPeripheral32, _a1, _a2, _a3 )
#define WritePeripheral8(_a1,_a2,_a3)      SYS_CALL_3_S_P_S( SYS_CALL_INDEX_WritePeripheral8, _a1, _a2, _a3 )
#define WritePeripheral16(_a1,_a2,_a3)     SYS_CALL_3_S_P_S( SYS_CALL_INDEX_WritePeripheral16, _a1, _a2, _a3 )
#define WritePeripheral32(_a1,_a2,_a3)     SYS_CALL_3_S_P_S( SYS_CALL_INDEX_WritePeripheral32, _a1, _a2, _a3 )
#else
#define ReadPeripheral8(_a1,_a2,_a3)       SYS_CALL_3( SYS_CALL_INDEX_ReadPeripheral8, _a1, _a2, _a3 )
#define ReadPeripheral16(_a1,_a2,_a3)      SYS_CALL_3( SYS_CALL_INDEX_ReadPeripheral16, _a1, _a2, _a3 )
#define ReadPeripheral32(_a1,_a2,_a3)      SYS_CALL_3( SYS_CALL_INDEX_ReadPeripheral32, _a1, _a2, _a3 )
#define WritePeripheral8(_a1,_a2,_a3)      SYS_CALL_3( SYS_CALL_INDEX_WritePeripheral8, _a1, _a2, _a3 )
#define WritePeripheral16(_a1,_a2,_a3)     SYS_CALL_3( SYS_CALL_INDEX_WritePeripheral16, _a1, _a2, _a3 )
#define WritePeripheral32(_a1,_a2,_a3)     SYS_CALL_3( SYS_CALL_INDEX_WritePeripheral32, _a1, _a2, _a3 )
#endif

#if defined(CFG_ARM)
#define ModifyPeripheral8(_a1,_a2,_a3,_a4)  SYS_CALL_8( SYS_CALL_INDEX_ModifyPeripheral8, _a1, _a2, _a3, _a4, 0u, 0u, 0u)
#define ModifyPeripheral16(_a1,_a2,_a3,_a4)    SYS_CALL_8( SYS_CALL_INDEX_ModifyPeripheral16, _a1, _a2, _a3, _a4, 0u, 0u, 0u)
#define ModifyPeripheral32(_a1,_a2,_a3,_a4)    SYS_CALL_8( SYS_CALL_INDEX_ModifyPeripheral32, _a1, _a2, _a3, _a4, 0u, 0u, 0u)
#elif defined(CFG_TC2XX)
#define ModifyPeripheral8(_a1,_a2,_a3,_a4)      SYS_CALL_4_S_P_S_S( SYS_CALL_INDEX_ModifyPeripheral8, _a1, _a2, _a3, _a4 )
#define ModifyPeripheral16(_a1,_a2,_a3,_a4)     SYS_CALL_4_S_P_S_S( SYS_CALL_INDEX_ModifyPeripheral16, _a1, _a2, _a3, _a4 )
#define ModifyPeripheral32(_a1,_a2,_a3,_a4)     SYS_CALL_4_S_P_S_S( SYS_CALL_INDEX_ModifyPeripheral32, _a1, _a2, _a3, _a4 )
#else
#define ModifyPeripheral8(_a1,_a2,_a3,_a4)  SYS_CALL_4( SYS_CALL_INDEX_ModifyPeripheral8, _a1, _a2, _a3, _a4 )
#define ModifyPeripheral16(_a1,_a2,_a3,_a4)    SYS_CALL_4( SYS_CALL_INDEX_ModifyPeripheral16, _a1, _a2, _a3, _a4 )
#define ModifyPeripheral32(_a1,_a2,_a3,_a4)    SYS_CALL_4( SYS_CALL_INDEX_ModifyPeripheral32, _a1, _a2, _a3, _a4 )
#endif


#if defined(CFG_TC2XX)
#define SYS_CALL_write(_a1,_a2,_a3)                 SYS_CALL_3_S_P_S( SYS_CALL_INDEX_write,_a1, _a2, _a3 )
#else
#define SYS_CALL_write(_a1,_a2,_a3)                 SYS_CALL_3( SYS_CALL_INDEX_write,_a1, _a2, _a3 )
#endif

#endif    /* (OS_SC1 == STD_ON) || (OS_SC2 == STD_ON) */


#define SYS_CALL_AtomicCopy8(to, from)  (to) = (from)

#define SYS_CALL_AtomicCopy16(to, from) (to) = (from)

#define SYS_CALL_AtomicCopy32(to, from) (to) = (from)

#define SYS_CALL_AtomicCopy64(to, from) SYS_CALL_SuspendAllInterrupts(); \
                                        (to) = (from); \
                                        SYS_CALL_ResumeAllInterrupts()
#define SYS_CALL_AtomicCopyBoolean(to, from)  (to) = (from)



#if (OS_USE_APPLICATIONS == STD_ON)
#if (( OS_SC3 == STD_ON ) || ( OS_SC4 == STD_ON ))
ApplicationType GetApplicationID( void );
AccessType     CheckISRMemoryAccess(     ISRType ISRID,
                                    MemoryStartAddressType Address,
                                    MemorySizeType Size );

AccessType     CheckTaskMemoryAccess(     TaskType TaskID,
                                    MemoryStartAddressType Address,
                                    MemorySizeType Size );

ObjectAccessType CheckObjectAccess( ApplicationType ApplId,
                                    ObjectTypeType ObjectType,
                                    uint32 objectId );
#endif // #if (( OS_SC3 == STD_ON ) || ( OS_SC4 == STD_ON ))

ApplicationType CheckObjectOwnership( ObjectTypeType ObjectType,
                                    uint32 objectId );

#endif //#if (OS_USE_APPLICATIONS == STD_ON)

StatusType  GetTaskID(        TaskRefType TaskID );
StatusType  GetTaskState(    TaskType task_id, TaskStateRefType state);

void        ShutdownOS( StatusType );
StatusType  ActivateTask( TaskType TaskID );
StatusType  TerminateTask( void );
StatusType  ChainTask( TaskType TaskID );
StatusType  Schedule( void );

typedef uint8 ResourceType;
StatusType GetResource( ResourceType ResID );
StatusType ReleaseResource( ResourceType ResID);

/*
 * Define scheduler as topmost
 */
#define    RES_SCHEDULER             OS_RESOURCE_CNT

/*
 * Priorities of tasks and resources
 */
#define OS_TASK_PRIORITY_MIN    0
#define OS_TASK_PRIORITY_MAX    UINT32_MAX
/* Logical priority is higher higher than normal tasks */
#define OS_RES_SCHEDULER_PRIO    UINT32_MAX

typedef struct OsDriver_s {
    sint32    OsGptChannelRef;
} OsDriver;



/*-------------------------------------------------------------------
 * Counters
 *-----------------------------------------------------------------*/


#ifdef USE_RTE

/* The OS service interface does not match the return type specified in the
 * OS specification and therefore the following wrap functions are introduced.
 * Also note there is an argument mismatch in the specification which suggests
 * the type is TimeInMicrosecondsType while the description describes that
 * ticks are used.
 */

/*******************************************************************************
 * @brief  This service reads the current count value of a counter (returning
 *         either the hardware timer ticks if counter is driven by hardware or
 *         the software ticks when user drives counter).
 * @param[in] counterId        Counter id
 * @param[out] value            Contains the current tick value of the counter
 * @return                     Status of the execution success of the function
 * @retval E_OK                 The function was excuted successfully
 * @retval E_OS_ID              Function error, the input counter value was
 *                             invalid
 * @retval E_OS_PARAM_POINTER  Function error, the value argument was NULL_PTR
 * @retval E_OS_DISABLEDINT    Function error, the interrupts were disabled
 ******************************************************************************/
Std_ReturnType Os_GetCounterValue(CounterType counterId, TickType * value);


/*******************************************************************************
 * @brief  This service gets the number of ticks between the current tick value
 *         and a previously read tick value
 * @param[in] counterId        Counter id
 * @param[inout] value            in: the previously read tick value of the counter
 *                             out: the current tick value of the counter
 * @param[out] elapsedTime     The difference to the previous read value
 * @return                     Status of the execution success of the function
 * @retval E_OK                 The function was excuted successfully
 * @retval E_OS_ID              Function error, the input counter value was
 *                             invalid
 * @retval E_OS_PARAM_POINTER  Function error, the value argument was NULL_PTR
 * @retval E_OS_DISABLEDINT    Function error, the interrupts were disabled
 ******************************************************************************/
Std_ReturnType Os_GetElapsedValue(CounterType counterId, TickType * value, TickType * elapsedValue);

#endif

StatusType IncrementCounter( CounterType );
StatusType GetCounterValue( CounterType, TickRefType );
StatusType GetElapsedCounterValue( CounterType, TickRefType val, TickRefType elapsed_val);
StatusType GetElapsedValue( CounterType CounterID, TickRefType Value, TickRefType ElapsedValue );


/*-------------------------------------------------------------------
 * System timer
 *-----------------------------------------------------------------*/
typedef const uint32 OsTickType;

/*lint -esym(9003,OsTickFreq) MISRA:EXTERNAL_FILE:Inhibit lint error for external object OsTickFreq:[MISRA 2012 Rule 8.9, advisory]*/
extern OsTickType OsTickFreq;
void Os_SysTickInit( void );
#if defined(CFG_OS_SYSTICK2)
void Os_SysTickStart2( uint32 frequency );
#else
void Os_SysTickStart(TickType period_ticks);
#endif

/* Return a value that is always a free running timer */
TickType GetOsTick( void );

/* Since this is based on SW ticks, no use to have ns resolution */
#define OSTICKDURATION_US        (OSTICKDURATION/1000UL)

/* Have special macros to always get the OsTick */
#define OS_TICKS2SEC_OS_TICK(_ticks)       ( (OSTICKDURATION_US * (_ticks))/1000000UL)
#define OS_TICKS2MS_OS_TICK(_ticks)        ( (OSTICKDURATION_US * (_ticks))/1000UL )
#define OS_TICKS2US_OS_TICK(_ticks)        (OSTICKDURATION_US * (_ticks))
#define OS_TICKS2NS_OS_TICK(_ticks)        (OSTICKDURATION * (_ticks))

/*-------------------------------------------------------------------
 * Kernel extra
 *-----------------------------------------------------------------*/

#if defined(CFG_KERNEL_EXTRA)


#define TMO_MIN           0
#define TMO_INFINITE      (0xffffffffUL)

#include "os_sem.h"
#include "os_mbox.h"

#define TICK_MAX     UINT_MAX

typedef struct OsSemaphore {
    volatile sint32 count;
    sint32 maxCount;
    STAILQ_HEAD(,OsTaskVar) taskHead;
} OsSemaphoreType;

typedef OsSemaphoreType OsMutexType;

StatusType Sleep( TickType ticks );
/* Semaphores */
OsSemaphoreType *   Semaphore_Create( sint32 initialCount, sint32 maxCount );
void       Semaphore_Destroy( OsSemaphoreType *semPtr );
StatusType Semaphore_Init( OsSemaphoreType *semPtr, sint32 initialCount, sint32 maxCount );
StatusType Semaphore_Wait( OsSemaphoreType *semPtr, TickType tmo );
void       Semaphore_Signal(OsSemaphoreType *semPtr);

StatusType WaitMutex( OsMutexType *mutexPtr, TickType tmo );
StatusType ReleaseMutex( OsMutexType *mutexPtr );
#endif

#if defined(CFG_OS_ISR_HOOKS)
void Os_PreIsrHook(ISRType);
void Os_PostIsrHook(ISRType);
#endif
#if defined(CFG_T1_ENABLE)
void Os_WaitEventHook(TaskType taskId);
void Os_ResumeEventHook(TaskType taskId);
void Os_ReleaseEventHook(TaskType taskId);

void Os_ActivateTaskHook(TaskType taskId);
void Os_StopTaskHook(TaskType taskId);
void Os_StartTaskHook(TaskType taskId);
void Os_StartStopTaskHook(TaskType taskIdStop, TaskType taskIdStart);
#endif



/*-------------------------------------------------------------------
 * Schedule Tables
 *-----------------------------------------------------------------*/

StatusType StartScheduleTableRel(ScheduleTableType sid, TickType offset);
StatusType StartScheduleTableAbs(ScheduleTableType sid, TickType val );
#if ( OS_SC2 == STD_ON ) || ( OS_SC4 == STD_ON )
StatusType StartScheduleTableSynchron(ScheduleTableType sid );
StatusType SyncScheduleTable( ScheduleTableType ScheduleTableID, TickType Value  );
StatusType SetScheduleTableAsync( ScheduleTableType sid );
#endif
StatusType StopScheduleTable(ScheduleTableType sid);
StatusType NextScheduleTable( ScheduleTableType sid_curr, ScheduleTableType sid_next);
StatusType GetScheduleTableStatus( ScheduleTableType sid, ScheduleTableStatusRefType status );


/*-------------------------------------------------------------------
 * Alarms
 *-----------------------------------------------------------------*/

typedef uint16 AlarmType;

typedef struct {
    TickType maxallowedvalue;
    TickType tickperbase;
    TickType mincycle;
} AlarmBaseType; /*@req OSEK_SWS_AL_00024*/

typedef AlarmBaseType *AlarmBaseRefType; /*@req OSEK_SWS_AL_00024*/

StatusType GetAlarmBase( AlarmType AlarmId, AlarmBaseRefType Info );
StatusType GetAlarm(AlarmType AlarmId, TickRefType Tick);
StatusType SetRelAlarm(AlarmType AlarmId, TickType Increment, TickType Cycle);
StatusType SetAbsAlarm(AlarmType AlarmId, TickType Start, TickType Cycle);
StatusType CancelAlarm(AlarmType AlarmId);

/*-------------------------------------------------------------------
 * Spinlocks
 *-----------------------------------------------------------------*/
/* @req SWS_Os_00792 Type definition */
typedef enum {
    TRYTOGETSPINLOCK_SUCCESS,
    TRYTOGETSPINLOCK_NOSUCCESS
} TryToGetSpinlockType;

/* @req SWS_Os_00791 Type definition */
typedef uint16 SpinlockIdType;

#if (OS_NUM_CORES > 1)
StatusType GetSpinlock( SpinlockIdType SpinlockId );
StatusType ReleaseSpinlock( SpinlockIdType SpinlockId );
StatusType TryToGetSpinlock( SpinlockIdType SpinlockId, TryToGetSpinlockType* Success);
#else
#define GetSpinlock( x )
#define ReleaseSpinlock( x )
#define TryToGetSpinlock( x,y )
#endif


/*-------------------------------------------------------------------
 * Error's
 *-----------------------------------------------------------------*/
typedef enum {
    OSServiceId_ActivateTask,
    OSServiceId_TerminateTask,
    OSServiceId_ChainTask,
    OSServiceId_Schedule,
    OSServiceId_GetTaskID,
    OSServiceId_GetResource,
    OSServiceId_ReleaseResource,
    OSServiceId_SetEvent,
    OSServiceId_ClearEvent,
    OSServiceId_GetEvent,
    OSServiceId_WaitEvent,
    OSServiceId_GetAlarmBase,
    OSServiceId_GetAlarm,
    OSServiceId_SetRelAlarm,
    OSServiceId_SetAbsAlarm,
    OSServiceId_CancelAlarm,
    OSServiceId_StartOS,
    OSServiceId_ShutdownOS,
    OSServiceId_ErrorHook,
    OSServiceId_PreTaskHook,
    OSServiceId_PostTaskHook,
    OSServiceId_StartupHook,
    OSServiceId_ShutdownHook,
    OSServiceId_GetTaskState,
    OSServiceId_GetApplicationID,
    OSServiceId_GetISRID,
    OSServiceId_CallTrustedFunction,
    OSServiceId_CheckISRMemoryAccess,
    OSServiceId_TaskMemoryAccess,
    OSServiceId_CheckObjectAccess,
    OSServiceId_CheckObjectOwnership,
    OSServiceId_StartScheduleTableRel,
    OSServiceId_StartScheduleTableAbs,
    OSServiceId_StopScheduleTable,
    OSServiceId_NextScheduleTable,
    OSServiceId_StartScheduleTableSynchron,
    OSServiceId_SyncScheduleTable,
    OSServiceId_GetScheduleTableStatus,
    OSServiceId_SetScheduleTableAsync,
    OSServiceId_IncrementCounter,
    OSServiceId_GetCounterValue,
    OSServiceId_GetElapsedValue,
    OSServiceId_TerminateApplication,
    OSServiceId_AllowAccess,
    OSServiceId_GetApplicationState,
    OSServiceId_ReadPeripheral8,
    OSServiceId_ReadPeripheral16,
    OSServiceId_ReadPeripheral32,
    OSServiceId_WritePeripheral8,
    OSServiceId_WritePeripheral16,
    OSServiceId_WritePeripheral32,
    OSServiceId_ModifyPeripheral8,
    OSServiceId_ModifyPeripheral16,
    OSServiceId_ModifyPeripheral32,

#if defined(CFG_KERNEL_EXTRA)
    OSServiceId_Sleep,
    OSServiceId_SemWaitChecks,
    OSServiceId_SemSignal,
    OSServiceId_BSemSignal,
#endif
} OsServiceIdType;

typedef struct OsError {
    OsServiceIdType serviceId;
    uint32 param1;
    uint32 param2;
    uint32 param3;
} OsErrorType;

extern OsErrorType os_error;

// IMPROVMENT: Add the service id to all OS service methods.
static inline OsServiceIdType OSErrorGetServiceId(void)  {
    return os_error.serviceId;
}

/* @req SWS_Os_00439
 * The Operating System module shall provide the OSEK error macros (OSError…())
 * to all configured error hooks AND there shall be two (like in OIL)
 * global configuration parameters to switch these macros on or off.
 *
 * NOTE: As Os has only extended mode, not standard mode, this requirement is considered for extended mode only,
 * that means no macro is implemented to switch off OSError_...()
 */
//Task
#define OSError_ActivateTask_TaskID         ((TaskType) os_error.param1)
#define OSError_ChainTask_TaskID            ((TaskType) os_error.param1)
#define OSError_GetTaskID_TaskID            ((TaskRefType) os_error.param1)
#define OSError_GetTaskState_TaskId         ((TaskType) os_error.param1)
#define OSError_GetTaskState_State          ((TaskStateRefType) os_error.param2)
//Resource
#define OSError_GetResource_ResID           ((ResourceType) os_error.param1)
#define OSError_ReleaseResource_ResID       ((ResourceType) os_error.param1)
//Event
#define OSError_SetEvent_TaskID             ((TaskType) os_error.param1)
#define OSError_SetEvent_Mask               ((EventMaskType) os_error.param2)
#define OSError_ClearEvent_Mask             ((EventMaskType) os_error.param1)
#define OSError_GetEvent_TaskId             ((TaskType) os_error.param1)
#define OSError_GetEvent_Mask               ((EventMaskRefType) os_error.param2)
#define OSError_WaitEvent_Mask              ((EventMaskType) os_error.param1)
//Alarm
#define OSError_GetAlarmBase_AlarmId        ((AlarmType) os_error.param1)
#define OSError_GetAlarmBase_Info           ((AlarmBaseRefType) os_error.param2)
#define OSError_GetAlarm_AlarmId            ((AlarmType) os_error.param1)
#define OSError_GetAlarm_Tick               ((TickRefType) os_error.param2)
#define OSError_SetRelAlarm_AlarmId         ((AlarmType) os_error.param1)
#define OSError_SetRelAlarm_Increment       ((TickType) os_error.param2)
#define OSError_SetRelAlarm_Cycle           ((TickType) os_error.param3)
#define OSError_SetAbsAlarm_AlarmId         ((AlarmType) os_error.param1)
#define OSError_SetAbsAlarm_Start           ((TickType) os_error.param2)
#define OSError_SetAbsAlarm_Cycle           ((TickType) os_error.param3)
#define OSError_CancelAlarm_AlarmId         ((AlarmType) os_error.param1)
//Application
#define OSError_GetApplicationState_ApplId  ((ApplicationType) os_error.param1)
#define OSError_GetApplicationState_Value   ((ApplicationStateRefType) os_error.param2)
//Counter
#define OSError_IncrementCounter_CounterId  ((CounterType) os_error.param1)
#define OSError_GetCounterValue_CounterId   ((CounterType) os_error.param1)
#define OSError_GetCounterValue_RickRef     ((TickRefType) os_error.param2)
#define OSError_GetElapsedValue_CounterId   ((CounterType) os_error.param1)
#define OSError_GetElapsedValue_Value       ((TickRefType) os_error.param2)
#define OSError_GetElapsedValue_ElapsedVal  ((TickRefType) os_error.param3)
//Schedule Table
#define OSError_StartScheduleTableRel_ScheduleTableID         ((ScheduleTableType) os_error.param1)
#define OSError_StartScheduleTableRel_Offset                  ((TickType) os_error.param2)
#define OSError_StartScheduleTableAbs_ScheduleTableID         ((ScheduleTableType) os_error.param1)
#define OSError_StartScheduleTableAbs_Start                   ((TickType) os_error.param2)
#define OSError_StartScheduleTableSynchron_ScheduleTableID    ((ScheduleTableType) os_error.param1)
#define OSError_StopScheduleTable_ScheduleTableID             ((ScheduleTableType) os_error.param1)
#define OSError_NextScheduleTable_ScheduleTableID_From        ((ScheduleTableType) os_error.param1)
#define OSError_NextScheduleTable_ScheduleTableID_To          ((ScheduleTableType) os_error.param2)
#define OSError_SyncScheduleTable_ScheduleTableID             ((ScheduleTableType) os_error.param1)
#define OSError_SyncScheduleTable_Value                       ((TickType) os_error.param2)
#define OSError_GetScheduleTableStatus_ScheduleTableID        ((ScheduleTableType) os_error.param1)
#define OSError_GetScheduleTableStatus_Status                 ((ScheduleTableStatusRefType)  os_error.param2)
#define OSError_SetScheduleTableAsync_ScheduleTableID         ((ScheduleTableType) os_error.param1)
//Init
#define OSError_StartOS_Mode           ((AppModeType) os_error.param1)
//Hook
#define OSError_ErrorHook_Error        ((StatusType) os_error.param1)
#define OSError_ShutdownHook_Error     ((StatusType) os_error.param1)

#if defined(LOCALMESSAGESONLY)
#error LOCALMESSAGESONLY shall not be defined
#endif


/*-------------------------------------------------------------------
 * COM
 *-----------------------------------------------------------------*/

typedef void *ApplicationDataRef;


/*-------------------------------------------------------------------
 * Multicore OS
 *-----------------------------------------------------------------*/
#define OS_CORE_ID_INVALID     65535
#define OS_CORE_ID_MASTER    0

/*lint -emacro(506, OS_CORE_IS_ID_MASTER) MISRA:OTHER:Exception, single core configuration result is always true:[MISRA 2004 Info,advisory]*/
#define OS_CORE_IS_ID_MASTER(_id)    (OS_CORE_ID_MASTER == (_id))

typedef uint32 CoreIDType; /* @req SWS_Os_00790 Type definition */
#if defined(CFG_MPC5777M)
/* No support for multicore, use core2 */
#define MPC5777M_ACTUAL_CORE_ID_MASTER 2
#define MPC5777M_INTC_REG_EOIR EOIR_PRC2
#define MPC5777M_INTC_REG_IACKR IACKR_PRC2
/* But we say that we use core0 since os assumes this for singlecore */
/*lint -emacro(506, GetCoreID) MISRA:OTHER:Exception, for optimization:[MISRA 2004 Info,advisory]*/

#define  GetCoreID()    0
#else
#if (OS_NUM_CORES > 1)
CoreIDType GetCoreID(void);
#else
/*lint -emacro(506, GetCoreID) MISRA:OTHER:Exception, for optimization:[MISRA 2004 Info,advisory]*/
#define  GetCoreID()    0
#endif
#endif
void StartCore(CoreIDType coreId, StatusType* status);

/*
 * ArcCore extensions
 */
void OsTick(void);

#define OS_ISR_TYPE2    0
#define OS_ISR_TYPE1    1

union isr_attr {
    TaskType tid;
    void (*entry)(void);
};

#define TTY_NONE        1
#define TTY_T32         2
#define TTY_WINIDEA     4
#define TTY_SERIAL      8
#define RAMLOG          16

#endif /*OS_H_*/
