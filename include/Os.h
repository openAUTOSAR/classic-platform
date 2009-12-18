/* -------------------------------- Arctic Core ------------------------------
 * Arctic Core - the open source AUTOSAR platform http://arccore.com
 *
 * Copyright (C) 2009  ArcCore AB <contact@arccore.com>
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by the
 * Free Software Foundation; See <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * -------------------------------- Arctic Core ------------------------------*/



#ifndef OS_H_
#define OS_H_

#define OS_SW_MAJOR_VERSION    1
#define OS_SW_MINOR_VERSION    0
#define OS_SW_PATCH_VERSION    0

#include "Std_Types.h"
#if !defined(CC_KERNEL)
#include "Os_Cfg.h"
#endif
#include "MemMap.h"
#include "Cpu.h"

/* TODO: remove this... */
// #include "pcb.h"
/* 13.5.1 */
/*
typedef  procid_t		TaskType;
typedef  event_t	EventMaskType;
*/
typedef uint32 EventMaskType;
typedef EventMaskType *EventMaskRefType;
typedef uint16 TaskType;
typedef TaskType *TaskRefType;

typedef enum {
	TASK_STATE_WAITING,
	TASK_STATE_READY,
	TASK_STATE_SUSPENDED,
	TASK_STATE_RUNNING,
} TaskStateType;


/*
 * Macros for error handling
 * Registers service id of the erroneous function and the applicable parameters
 * to os_error. Functions that have less than three parameters do not touch
 * os_error.param3. Same rule follows for other parameter counts.
 */

/* Error handling for functions that take no arguments */
#define OS_STD_END(_service_id) \
        goto ok;        \
    err:                \
        os_error.serviceId=_service_id;\
        ERRORHOOK(rv);  \
    ok:                 \
        return rv;

/* Error handling for functions that take one argument */
#define OS_STD_END_1(_service_id, _p1) \
        goto ok;        \
    err:                \
    os_error.serviceId=_service_id;\
        os_error.param1 = (uint32_t) _p1; \
        ERRORHOOK(rv);  \
    ok:                 \
        return rv;

/* Error handling for functions that take two arguments */
#define OS_STD_END_2(_service_id, _p1,_p2) \
        goto ok;        \
    err:                \
        os_error.serviceId=_service_id;\
        os_error.param1 = (uint32_t) _p1; \
        os_error.param2 = (uint32_t) _p2; \
        ERRORHOOK(rv);  \
    ok:                 \
        return rv;

/* Error handling for functions that take three arguments */
#define OS_STD_END_3(_service_id,_p1,_p2,_p3) \
        goto ok;        \
    err:                \
        os_error.serviceId=_service_id;\
        os_error.param1 = (uint32_t) _p1; \
        os_error.param2 = (uint32_t) _p2; \
        os_error.param3 = (uint32_t) _p3; \
        ERRORHOOK(rv);  \
    ok:                 \
        return rv;


typedef TaskStateType *TaskStateRefType;

/* FIXME: OSMEMORY_IS__ , see 8.2*/


#define INVALID_OSAPPLICATION (-1)

/* TODO, I have no idea what this should be*/
typedef sint32 ApplicationType;

/* See oil config for defines */
typedef sint32 AppModeType;

/* FIXME: more types here */
typedef uint16 ScheduleTableType;
typedef uint16 GlobalTimeTickType;


typedef enum {
	SCHEDULETABLE_STOPPED,
	SCHEDULETABLE_NEXT,
	SCHEDULETABLE_WAITING,
	SCHEDULETABLE_RUNNING,
	SCHEDULETABLE_RUNNING_AND_SYNCHRONOUS
} ScheduleTableStatusType;

typedef ScheduleTableStatusType *ScheduleTableStatusRefType;

/* Flags for AccessType */
#define ACCESSTYPE_W	1
#define ACCESSTYPE_R	(1<<1)

typedef uint32 AccessType;

typedef enum {
	ACCESS,
	NO_ACCESS
} ObjectAccessType;

typedef enum {
	OBJECT_TASK,
	OBJECT_ISR,
	OBJECT_ALARM,
	OBJECT_RESOURCE,
	OBJECT_COUNTER,
	OBJECT_SCHEDULETABLE,
	OBJECT_MESSAGE
} ObjectTypeType;

typedef enum {
	PRO_KILLTASKISR,
	PRO_KILLAPPL,
	PRO_KILLAPPL_RESTART,
	PRO_SHUTDOWN
} ProtectionReturnType;

typedef enum {
	RESTART,
	NO_RESTART
} RestartType;


typedef ProtectionReturnType (*ProtectionHookType)( StatusType);
typedef void (*StartupHookType)( void );
typedef void (*ShutdownHookType)( StatusType );
typedef void (*ErrorHookType)( StatusType );
typedef void (*PreTaskHookType)( void );
typedef void (*PostTaskHookType)( void );

ProtectionReturnType ProtectionHook( StatusType FatalError );
void StartupHook( void );
void ShutdownHook( StatusType Error );
void ErrorHook( StatusType Error );
void PreTaskHook( void );
void PostTaskHook( void );

typedef uint16 TrustedFunctionIndexType;
typedef  void * TrustedFunctionParameterRefType;


/* See 8.3.9 */
#define INVALID_ISR		((sint16)(-1))
typedef	sint16 ISRType;

typedef void * MemoryStartAddressType;
typedef uint32 MemorySizeType;

//#define WaitEvent(...) _WaitEvent(__VA_ARGS__)
#if 0
#define WaitEvent(...) SC_CALL(WaitEvent,1,1,__VA_ARGS__)
#define SetEvent(...) SC_CALL(SetEvent,2,2,__VA_ARGS__)
#define ClearEvent(...) SC_CALL(ClearEvent,3,1,__VA_ARGS__)
#endif
StatusType WaitEvent( EventMaskType Mask );
StatusType SetEvent( TaskType TaskID, EventMaskType Mask );
StatusType ClearEvent( EventMaskType Mask);
StatusType GetEvent( TaskType TaskId, EventMaskRefType Mask);


ApplicationType GetApplicationID( void );
ISRType GetISRID( void );


#define  EnableAllInterrupts() 		Irq_Enable()
#define  DisableAllInterrupts()		Irq_Disable()
#define  ResumeAllInterrupts()		Irq_Enable()
#define  SuspendAllInterrupts()		Irq_Disable()
#define  ResumeOSInterrupts()		Irq_Enable()
#define  SuspendOSInterrupts()		Irq_Disable()

#if 0
void EnableAllInterrups( void )		Irq_Enable()
void DisableAllInterrupts( void )	Irq_Disable()
void ResumeAllInterrupts( void )	Irq_Enable()
void SuspendAllInterrupts( void )	Irq_Disable()
void ResumeOSInterrupts( void )		Irq_Enable()
void SuspendOSInterrupts( void )	Irq_Disable()
#endif

StatusType EnableInterruptSource( ISRType EnableISR );
StatusType DisableInterruptSource( ISRType EnableISR );


ApplicationType GetApplicationID( void );
ISRType GetISRID( void );
AccessType CheckISRMemoryAccess( ISRType ISRID,
								MemoryStartAddressType Address,
								MemorySizeType Size );

AccessType CheckTaskMemoryAccess( 	TaskType TaskID,
									MemoryStartAddressType Address,
									MemorySizeType Size );
void InitOS( void );
void StartOS( AppModeType Mode );

StatusType CallTrustedFunction(	TrustedFunctionIndexType FunctionIndex,
								TrustedFunctionParameterRefType FunctionParams );

StatusType GetTaskID( TaskRefType TaskID );
StatusType GetTaskState(TaskType task_id, TaskStateRefType state);

void ShutdownOS( StatusType );
StatusType ActivateTask( TaskType TaskID );
StatusType TerminateTask( void );
StatusType ChainTask( TaskType TaskID );
StatusType Schedule( void );

/* TODO: This def. is wrong wrong wrong */

/* Hmm, OS188 indicates that we must have a locking time here ..*/
/* and we at least have prio here */
//typedef const uint32 ResourceType;
#if 0

#define DeclareResource(x) extern ResourceType *(x);

StatusType GetResource( ResourceType *ResID );
StatusType ReleaseResource( ResourceType *ResID);

#else

typedef uint32 ResourceType;
#define DeclareResource(x) extern ResourceType (x);
StatusType GetResource( ResourceType ResID );
StatusType ReleaseResource( ResourceType ResID);

#endif

#define	RES_SCHEDULER 0
//DeclareResource(RES_SCHEDULER);
#define OS_TASK_PRIORITY_MIN	0
#define OS_TASK_PRIORITY_MAX	31

typedef struct OsDriver_s {
	int	OsGptChannelRef;
} OsDriver;

/*-------------------------------------------------------------------
 * Free running timer
 *-----------------------------------------------------------------*/
typedef const uint32 OsTickType;
void Frt_Init( void );
void Frt_Start(uint32_t period_ticks);
uint32_t Frt_GetTimeElapsed( void );

/*-------------------------------------------------------------------
 * Counters
 *-----------------------------------------------------------------*/
typedef sint16 CounterType;

typedef uint32 TickType;
typedef TickType *TickRefType;

StatusType IncrementCounter( CounterType );
StatusType GetCounterValue( CounterType, TickRefType );
StatusType GetElapsedCounterValue( CounterType, TickRefType val, TickRefType elapsed_val);

/*-------------------------------------------------------------------
 * Schedule Tables
 *-----------------------------------------------------------------*/

StatusType StartScheduleTableRel(ScheduleTableType sid, TickType offset);
StatusType StartScheduleTableAbs(ScheduleTableType sid, TickType val );
StatusType StartScheduleTableSynchron(ScheduleTableType sid );
StatusType StopScheduleTable(ScheduleTableType sid);
StatusType NextScheduleTable( ScheduleTableType sid_curr, ScheduleTableType sid_next);
StatusType SyncScheduleTable( ScheduleTableType sid, GlobalTimeTickType global_time  );
StatusType GetScheduleTableStatus( ScheduleTableType sid, ScheduleTableStatusRefType status );
StatusType SetScheduleTableAsync( ScheduleTableType sid );


/*-------------------------------------------------------------------
 * Alarms
 *-----------------------------------------------------------------*/

typedef uint16 AlarmType;

typedef struct {
	TickType maxallowedvalue;
	TickType tickperbase;
	TickType mincycle;
} AlarmBaseType;

typedef AlarmBaseType *AlarmBaseRefType;

StatusType GetAlarmBase( AlarmType AlarmId, AlarmBaseRefType Info );
StatusType GetAlarm(AlarmType AlarmId, TickRefType Tick);
StatusType SetRelAlarm(AlarmType AlarmId, TickType Increment, TickType Cycle);
StatusType SetAbsAlarm(AlarmType AlarmId, TickType Start, TickType Cycle);
StatusType CancelAlarm(AlarmType AlarmId);


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
} OsServiceIdType;;

typedef struct os_error_s {
	OsServiceIdType serviceId;
	uint32_t param1;
	uint32_t param2;
	uint32_t param3;
} os_error_t;

extern os_error_t os_error;

// TODO: Add the service id to all OS service methods.
static inline OsServiceIdType OSErrorGetServiceId(void)  {
	return os_error.serviceId;
}

extern os_error_t os_error;

#define OSError_ActivateTask_TaskID ((TaskType) os_error.param1)
#define OSError_ChainTask_TaskID ((TaskType) os_error.param1)
#define OSError_GetTaskID_TaskID ((TaskRefType) os_error.param1)
#define OSError_GetResource_ResID ((ResourceType) os_error.param1)
#define OSError_ReleaseResource_ResID ((ResourceType) os_error.param1)
#define OSError_SetEvent_TaskID ((TaskType) os_error.param1)
#define OSError_SetEvent_Mask ((EventMaskType) os_error.param2)
#define OSError_ClearEvent_Mask ((EventMaskType) os_error.param1)
#define OSError_GetEvent_TaskId ((TaskType) os_error.param1)
#define OSError_GetEvent_Mask ((EventMaskRefType) os_error.param2)
#define OSError_WaitEvent_Mask ((EventMaskType) os_error.param1)
#define OSError_GetAlarmBase_AlarmId ((AlarmType) os_error.param1)
#define OSError_GetAlarmBase_Info ((AlarmBaseRefType) os_error.param2)
#define OSError_GetAlarm_AlarmId ((AlarmType) os_error.param1)
#define OSError_GetAlarm_Tick ((TickRefType) os_error.param2)
#define OSError_SetRelAlarm_AlarmId ((AlarmType) os_error.param1)
#define OSError_SetRelAlarm_Increment ((TickType) os_error.param2)
#define OSError_SetRelAlarm_Cycle ((TickType) os_error.param3)
#define OSError_SetAbsAlarm_AlarmId ((AlarmType) os_error.param1)
#define OSError_SetAbsAlarm_Start ((TickType) os_error.param2)
#define OSError_SetAbsAlarm_Cycle ((TickType) os_error.param3)
#define OSError_CancelAlarm_AlarmId ((AlarmType) os_error.param1)
#define OSError_StartOS_Mode ((AppModeType) os_error.param1)
#define OSError_ErrorHook_Error ((StatusType) os_error.param1)
#define OSError_ShutdownHook_Error ((StatusType) os_error.param1)
#define OSError_GetTaskState_TaskId ((TaskType) os_error.param1)
#define OSError_GetTaskState_State ((TaskStateRefType) os_error.param2)

/*-------------------------------------------------------------------
 * COM ( TODO : move )
 *-----------------------------------------------------------------*/

/*
 * The only information about the COM that is valid is
 * in the COM specification ..SWS_COM.pdf.
 *
 * The most important requirements are COM010 and COM013
 *
 * Com_Init()
 * Com_DeInit()
 *
 * No error hooks..
 * No. GetMessageStatus()
 * No. SendZeroMessage()
 * No. SendDynamicMessage(), RecieveDynamicMessage()
 *
 * Yes. SendMessage()
 *
 * */


typedef uint32 MessageType;
typedef void *ApplicationDataRef;


StatusType SendMessage( MessageType message_id, ApplicationDataRef dataRef );
StatusType ReceiveMessage( MessageType message_id, ApplicationDataRef dataRef );

/*
 * ArcCore extensions
 */
TickType GetOsTick();
void OsTick(void);
void OsIdle(void);

// Generate conversion macro'
// Todo
#define OS_TICK2NS_OS_TICK_COUNTER(_x)
#define OS_TICK2US_OS_TICK_COUNTER(_x)
#define OS_TICK2MS_OS_TICK_COUNTER(_x)
#define OS_TICK2SEC_OS_TICK_COUNTER(_x)

#define OS_ISR_TYPE2	0
#define OS_ISR_TYPE1	1

union isr_attr {
	TaskType tid;
	void (*entry)(void);
};

typedef struct StackInfo_s {
	void *at_swap;	// This task was swapped in with this stack
	void *top;		// Top of the stack
	int size;		// Size of the stack

	void *curr;		// current stack ptr
	void *usage;	// Usage in %
} StackInfoType;

void Os_GetStackInfo( TaskType pid, StackInfoType *s );

#define OS_STACK_USAGE(_x) ((((_x)->size - (uint32_t)((_x)->usage - (_x)->top))*100)/(_x)->size)

int simple_printf(const char *format, ...);

#define ARRAY_SIZE(_x) sizeof(_x)/sizeof((_x)[0])

#define OS_STR__(x)	#x
#define OS_STRSTR__(x) OS_STR__(x)


TaskType Os_CreateIsr( void  (*entry)(void), uint8_t prio, const char *name );
#if 0
void IntCtrl_AttachIsr1( void (*entry)(void), void *int_ctrl, uint32_t vector,uint8_t prio);
void IntCtrl_AttachIsr2(TaskType tid,void *int_ctrl, IrqType vector );
#endif


#endif /*OS_H_*/
