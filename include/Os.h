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

#define OS_SW_MAJOR_VERSION    2
#define OS_SW_MINOR_VERSION    0
#define OS_SW_PATCH_VERSION    0

#include <assert.h>
#include "Std_Types.h"
#include "Os_Cfg.h"
#include "MemMap.h"
#include "Cpu.h"

typedef uint32_t 		EventMaskType;
typedef EventMaskType *	EventMaskRefType;
typedef uint16_t 		TaskType;
typedef TaskType *		TaskRefType;

typedef enum {
	TASK_STATE_WAITING,
	TASK_STATE_READY,
	TASK_STATE_SUSPENDED,
	TASK_STATE_RUNNING,
} TaskStateType;

#define INVALID_TASK	0xdeadU

typedef TaskStateType *TaskStateRefType;

/* FIXME: OSMEMORY_IS__ , see 8.2*/

#define OSDEFAULTAPPMODE  1

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

#if 0
#define WaitEvent(...) SC_CALL(WaitEvent,1,1,__VA_ARGS__)
#define SetEvent(...) SC_CALL(SetEvent,2,2,__VA_ARGS__)
#define ClearEvent(...) SC_CALL(ClearEvent,3,1,__VA_ARGS__)
#endif
StatusType WaitEvent( EventMaskType Mask );
StatusType SetEvent( TaskType TaskID, EventMaskType Mask );
StatusType ClearEvent( EventMaskType Mask);
StatusType GetEvent( TaskType TaskId, EventMaskRefType Mask);

void InitOS( void );
void StartOS( AppModeType Mode );



ApplicationType GetApplicationID( void );
ISRType GetISRID( void );
StatusType GetActiveApplicationMode( AppModeType* mode);

typedef int8_t Os_IntCounterType;

/* requirements here: OS368(ISR2), OS092 */

extern Os_IntCounterType Os_IntDisableAllCnt;
extern Os_IntCounterType Os_IntSuspendAllCnt;
extern Os_IntCounterType Os_IntSuspendOsCnt;



/** @req OS299 */
/* OSEK: States that " service  does  not  support  nesting", but says
 * nothing what to do about it.
 *
 * OS092: "  If EnableAllInterrupts()/ResumeAllInterrupts()/ResumeOSInterrupts()
 * are called and no corresponding DisableAllInterupts()/SuspendAllInterrupts()/
 * SuspendOSInterrupts()  was  done  before,  the Operating System shall not
 * perform this OS service.
 */
static inline void DisableAllInterrupts( void ) {
	Irq_Disable();
	Os_IntDisableAllCnt++;
	/* No nesting allowed */
	assert(Os_IntDisableAllCnt==1);
}

static inline void EnableAllInterrupts( void ) {

	if(Os_IntDisableAllCnt==0) {
		/** @req OS092 EnableAllInterrupts */
	} else {
		Os_IntDisableAllCnt--;
		Irq_Enable();
	}
}

static inline void SuspendAllInterrupts( void ) {
	Irq_SuspendAll();
	Os_IntSuspendAllCnt++;
}

static inline void ResumeAllInterrupts( void ) {

	if(Os_IntSuspendAllCnt==0) {
		/** @req OS092 ResumeAllInterrupts */
	} else {
		Os_IntSuspendAllCnt--;
		Irq_ResumeAll();
	}
}

/* Only ISR2 interrupts should be suspended but this can NEVER be
 * done in a more efficient way than to disable all, so let's
 * do that for all targets.
 */

static inline void SuspendOSInterrupts( void ) {
	Irq_SuspendOs();
	Os_IntSuspendOsCnt++;
}

static inline void ResumeOSInterrupts( void ) {

	if(Os_IntSuspendOsCnt==0) {
		/** @req OS092 ResumeOSInterrupts */
	} else {
		Os_IntSuspendOsCnt--;
		Irq_ResumeOs();
	}
}

/*
 * Class 2,3 and 4 API
 */
ApplicationType GetApplicationID( void );
AccessType 	CheckISRMemoryAccess( 	ISRType ISRID,
									MemoryStartAddressType Address,
									MemorySizeType Size );

AccessType 	CheckTaskMemoryAccess( 	TaskType TaskID,
									MemoryStartAddressType Address,
									MemorySizeType Size );

StatusType 	CallTrustedFunction(	TrustedFunctionIndexType FunctionIndex,
									TrustedFunctionParameterRefType FunctionParams );

StatusType 	GetTaskID(		TaskRefType TaskID );
StatusType 	GetTaskState(	TaskType task_id, TaskStateRefType state);

void 		ShutdownOS( StatusType );
StatusType 	ActivateTask( TaskType TaskID );
StatusType 	TerminateTask( void );
StatusType 	ChainTask( TaskType TaskID );
StatusType 	Schedule( void );

typedef uint8 ResourceType;
#define DeclareResource(x) extern ResourceType (x);
StatusType GetResource( ResourceType ResID );
StatusType ReleaseResource( ResourceType ResID);

/*
 * Define scheduler as topmost
 */
#define	RES_SCHEDULER 			OS_RESOURCE_CNT

/*
 * Priorities of tasks and resources
 */
#define OS_TASK_PRIORITY_MIN	0
#define OS_TASK_PRIORITY_MAX	31
/* Logical priority is higher higher than normal tasks */
#define OS_RES_SCHEDULER_PRIO	32

typedef struct OsDriver_s {
	int	OsGptChannelRef;
} OsDriver;



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
 * System timer
 *-----------------------------------------------------------------*/
typedef const uint32 OsTickType;
void Os_SysTickInit( void );
void Os_SysTickStart(TickType period_ticks);
TickType Os_SysTickGetValue( void );
TickType Os_SysTickGetElapsedValue( TickType preValue );

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

typedef struct OsError {
	OsServiceIdType serviceId;
	uint32_t param1;
	uint32_t param2;
	uint32_t param3;
} OsErrorType;

extern OsErrorType os_error;

// TODO: Add the service id to all OS service methods.
static inline OsServiceIdType OSErrorGetServiceId(void)  {
	return os_error.serviceId;
}

extern OsErrorType os_error;

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

/** @req OS398 */
#if defined(LOCALMESSAGESONLY)
#error LOCALMESSAGESONLY shall not be defined
#endif


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

#define OS_ISR_TYPE2	0
#define OS_ISR_TYPE1	1

union isr_attr {
	TaskType tid;
	void (*entry)(void);
};

#define TTY_NONE		1
#define TTY_T32		2
#define TTY_WINIDEA	4
#define TTY_SERIAL		8
#define RAMLOG		16

#endif /*OS_H_*/
