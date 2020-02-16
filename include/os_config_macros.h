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

/* Configure "rules"
 * - Don't pollute the namespace with the generator tools. The tools should
 *   ONLY know the GEN_xxx macros.
 * - If something is a container with multiplicity 1 and above, make it a pointer.
 * - ...
 */

#ifndef _OS_CONFIG_MACROS_H
#define _OS_CONFIG_MACROS_H

#include "Std_Types.h"

#define false		0
#define true		1

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(_x)		(sizeof(_x)/sizeof((_x)[0]))
#endif


// +1 here.. easy to have a reference..
#define GEN_TRUSTEDFUNCTIONS_LIST trusted_func_t os_cfg_trusted_list[OS_SERVICE_CNT];

/*
 *---------------------- APPLICATIONS --------------------------------------
 */

#define GEN_APPLICATION_HEAD const OsAppConstType Os_AppConst[OS_APPLICATION_CNT]

#define GEN_APPLICATION(	_id, _name, _trusted, _core, _startuphook, _shutdownhook, _errorhook, \
							_restart_task  ) \
{												\
	.appId = _id, 						\
	.name = _name,								\
	.core = _core,                              \
	.trusted = _trusted,						\
	.StartupHook = _startuphook,				\
	.ShutdownHook = _shutdownhook,				\
	.ErrorHook = _errorhook,					\
	.restartTaskId = _restart_task				\
}

/*
 *---------------------- TASKS ---------------------------------------------
 */

#define GEN_TASK_HEAD const OsTaskConstType  Os_TaskConstList[OS_TASK_CNT]

/**
 * _id
 * _priority 		The task priority
 * _autostart   	true/false
 * _resource_int_p  Pointer to internal resource.
 *                  NULL - if no internal resource or scheduling==NON
 *                  Pointer to
 * _scheduling      FULL or NON
 * _resource_mask   Mask of the resources used. Applies to STANDARD and LINKED (NOT INTERNAL)
 *                  For example if this task would use resource with id 2 and 4 the mask would
 *                  become (1<<2)|(1<<4) = 0x14 (limits resources to 32).
 *                  Currently used for calculating the ceiling priority.
 */
#define GEN_ETASK( 	_id, _name, _priority, _scheduling, \
					_autostart, _resource_int_p,  _resource_mask, _event_mask, \
					_appl_owner, _accessing_appl_mask ) \
{									\
	.pid = TASK_ID_ ## _id,           			\
	.name = _name,					\
	.entry = _id,				\
	.prio = _priority,				\
	.proc_type = PROC_EXTENDED,		\
	.stack.size = sizeof stack_##_id,	\
	.stack.top = stack_##_id,		\
	.autostart = _autostart,		\
	.resourceIntPtr = _resource_int_p, \
	.scheduling = _scheduling, \
	.resourceAccess = _resource_mask, \
	.eventMask = _event_mask, \
	.activationLimit = 1, \
	.applOwnerId = _appl_owner, \
	.accessingApplMask = _accessing_appl_mask, \
}

#define GEN_BTASK( 	_id, _name, _priority, _scheduling, \
					_autostart, _resource_int_p,  _resource_mask, \
					_activation_limit, _appl_owner, _accessing_appl_mask ) \
{									\
	.pid = TASK_ID_ ## _id,           			\
	.name = _name,					\
	.entry = _id,				\
	.prio = _priority,				\
	.proc_type = PROC_BASIC,		\
	.stack.size = sizeof stack_##_id,	\
	.stack.top = stack_##_id,		\
	.autostart = _autostart,		\
	.resourceIntPtr = _resource_int_p, \
	.scheduling = _scheduling, \
	.resourceAccess = _resource_mask, \
	.activationLimit = _activation_limit, \
	.applOwnerId = _appl_owner, \
	.accessingApplMask = _accessing_appl_mask, \
}

/*
 *---------------------- INTERRUPTS ----------------------------------------
 */

#define GEN_ISR_HEAD const OsIsrConstType Os_IsrConstList[OS_ISR_CNT]

#define GEN_ISR1( _name, _vector, _priority, _entry, _appOwner ) \
{                                      \
		.vector = _vector,             \
		.type = ISR_TYPE_1,            \
		.priority = _priority,         \
		.entry = _entry,               \
		.name = _name,                 \
		.resourceMask = 0,             \
		.appOwner = _appOwner,         \
}

#define GEN_ISR2( _name, _vector, _priority, _entry, _appOwner, _resourceMask ) \
{                                      \
		.vector = _vector,             \
		.type = ISR_TYPE_2,            \
		.priority = _priority,         \
		.entry = _entry,               \
		.name = _name,                 \
		.resourceMask = _resourceMask, \
		.appOwner = _appOwner,         \
}


#define GEN_ISR_MAP uint8_t Os_VectorToIsr[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS]


/*
 *---------------------- RESOURCES -----------------------------------------
 */

#define GEN_RESOURCE_HEAD OsResourceType resource_list[OS_RESOURCE_CNT]

/**
 * _id
 * _type              RESOURCE_TYPE_STANDARD, RESOURCE_TYPE_LINKED or RESOURCE_TYPE_INTERNAL
 * _ceiling_priority  The calculated ceiling priority
 */
#define GEN_RESOURCE( _id, _type, _ceiling_priority, _appl_owner, _accessing_appl_mask ) \
{												\
	.nr= _id,									\
	.type= _type,								\
	.ceiling_priority = _ceiling_priority,		\
	.owner = (-1),								\
	.applOwnerId = _appl_owner, 				\
	.accessingApplMask = _accessing_appl_mask 	\
}


/*
 *---------------------- COUNTERS ------------------------------------------
 */

#define GEN_COUNTER_HEAD OsCounterType counter_list[OS_COUNTER_CNT]

/**
 * _id
 *      NOT_USED
 * _name
 *    Name of the alarm, string
 *
 * _type
 *   COUNTER_TYPE_HARD or COUNTER_TYPE_SOFT
 *
 * _unit
 *   COUNTER_UNIT_TICKS or COUNTER_UNIT_NANO
 *
 * _maxallowedvalue
 *    0xffffffffUL
 *
 * _ticksperbase
 *    1
 *
 * _mincycle
 *    Say 10000

 * _gpt_ch
 *    NOT USED. Set to 0
 */

#define GEN_COUNTER( _id, _name, _type, _unit, 	\
					_maxallowedvalue, 			\
					_ticksperbase, 				\
					_mincycle,          \
					_gpt_ch,				\
					_appl_owner, \
					_accessing_appl_mask )  \
{												\
	.type = _type,								\
	.unit = _unit,								\
	.alarm_base.maxallowedvalue = _maxallowedvalue,	\
	.alarm_base.tickperbase = _ticksperbase,		\
	.alarm_base.mincycle = _mincycle,				\
	.applOwnerId = _appl_owner, \
	.accessingApplMask = _accessing_appl_mask, \
}

/*
 *---------------------- ALARMS --------------------------------------------
 */

#define GEN_ALARM_HEAD OsAlarmType alarm_list[OS_ALARM_CNT]

#define	GEN_ALARM_AUTOSTART_NAME(_id)    &(Os_AlarmAutoStart_ ## _id)

/**
 * _id
 * _type
 * _alarms_time
 * _cycle_time
 * _app_mode       Mask of the application modes.
 */
#define GEN_ALARM_AUTOSTART(_id, _type, _alarm_time, _cycle_time, _app_mode ) \
		const OsAlarmAutostartType Os_AlarmAutoStart_ ## _id = \
		{ \
			.autostartType = _type, \
			.alarmTime = _alarm_time, \
			.cycleTime = _cycle_time, \
			.appModeRef = _app_mode \
		}

/**
 * _id
 *    NOT USED
 *
 * _name
 *    Name of the alarm, string
 *
 * _counter_id
 *    The id of the counter to drive the alarm
 *
 * _autostart_ref
 *
 * _X_type       - Any of:
 * 					ALARM_ACTION_ACTIVATETASK
 * 					ALARM_ACTION_SETEVENT
 * 					ALARM_ACTION_ALARMCALLBACK
 * 					ALARM_ACTION_INCREMENTCOUNTER
 *
 * _X_task_id    - The task ID to activate if _X_type is:
 * 					ALARM_ACTION_ACTIVATETASK or
 * 					ALARM_ACTION_SETEVENT
 *
 * _X_event_id   - The event ID if type is ALARM_ACTION_SETEVENT
 *
 * _X_counter_id - The counter ID if type is ALARM_ACTION_INCREMENTCOUNTER
 *
 */
#define GEN_ALARM( _id, _name, _counter_id,		\
		    _autostart_ref,                 	\
			_action_type,						\
			_action_task_id,					\
			_action_event_id,					\
			_action_counter_id,					\
			_appl_owner, 						\
			_accessing_appl_mask ) 				\
{											\
	.name = _name,							\
	.counter = &counter_list[_counter_id],	\
	.counter_id = _counter_id,				\
	.autostartPtr = _autostart_ref,         \
	.action = {								\
		.type = _action_type,				\
		.task_id = _action_task_id,			\
		.event_id = _action_event_id,		\
		.counter_id = _action_counter_id 	\
	},										\
	.applOwnerId = _appl_owner, \
	.accessingApplMask = _accessing_appl_mask, \
}

/*
 *---------------------- SCHEDULE TABLES -----------------------------------
 */

#define GEN_SCHTBL_EXPIRY_POINT_HEAD(_id ) \
		OsScheduleTableExpiryPointType Os_SchTblExpPointList_##_id[] =

#define GEN_SCHTBL_EXPIRY_POINT_W_TASK_EVENT(_id, _offset ) 										\
	{																			\
		.offset      = _offset,													\
		.taskList    = Os_SchTblTaskList_ ## _id ## _ ## _offset,				\
		.taskListCnt = ARRAY_SIZE(Os_SchTblTaskList_ ## _id ## _ ## _offset),	\
		.eventList   = Os_SchTblEventList_ ## _id ## _ ## _offset,				\
		.eventListCnt    = ARRAY_SIZE(Os_SchTblEventList_ ## _id ## _ ## _offset)	\
	}

#define GEN_SCHTBL_EXPIRY_POINT_W_TASK(_id, _offset ) 							\
	{																			\
		.offset      = _offset,													\
		.taskList    = Os_SchTblTaskList_ ## _id ## _ ## _offset,				\
		.taskListCnt = ARRAY_SIZE(Os_SchTblTaskList_ ## _id ## _ ## _offset),	\
	}

#define GEN_SCHTBL_EXPIRY_POINT_W_EVENT(_id, _offset ) 							\
	{																			\
		.offset      = _offset,													\
		.eventList   = Os_SchTblEventList_ ## _id ## _ ## _offset,				\
		.eventListCnt    = ARRAY_SIZE(Os_SchTblEventList_ ## _id ## _ ## _offset)	\
	}

#define	GEN_SCHTBL_TASK_LIST_HEAD( _id, _offset ) \
		const TaskType Os_SchTblTaskList_ ## _id ## _ ## _offset[] =

#define	GEN_SCHTBL_EVENT_LIST_HEAD( _id, _offset ) \
		const OsScheduleTableEventSettingType Os_SchTblEventList_ ## _id ## _ ## _offset[] =

#define GEN_SCHTBL_AUTOSTART(_id, _type, _offset, _app_mode ) \
		const struct OsSchTblAutostart Os_SchTblAutoStart_ ## _id = \
		{ \
			.type = _type, \
			.offset = _offset, \
			.appMode = _app_mode, \
		}

#define GEN_SCHTBL_AUTOSTART_NAME(_id)	&(Os_SchTblAutoStart_ ## _id)

#define GEN_SCHTBL_HEAD OsSchTblType sched_list[OS_SCHTBL_CNT]

/**
 * _id
 *    NOT USED
 *
 * _name
 *    Name of the alarm, string
 *
 * _counter_ref
 *    Pointer to the counter that drives the table
 *
 * _repeating
 *   SINGLE_SHOT or REPEATING
 *
 * _duration
 *   The duration of the schedule table
 *
 * _autostart_ref
 *   Pointer to autostart configuration.
 *   If autostart is desired set name to GEN_SCHTBL_AUTOSTART_NAME(<id>). It also
 *   requires that GEN_SCHTBL_AUTOSTART(...) is set.
 *   Set to NULL if not autostart configuration is desired.
 *
 * The usage of the macro requires that GEN_SCHTBL_EXPIRY_POINT_HEAD(<id>) is also
 * set.
 */

#define GEN_SCHEDULETABLE(  _id, _name, _counter_id, _repeating, \
							_duration,              \
							_autostart_ref,         \
							_appl_owner, 	    	\
							_accessing_appl_mask )  \
{											 \
	.name = _name,						     \
	.counter = &counter_list[_counter_id],	     \
	.repeating = _repeating,			     \
	.duration = _duration,					 \
	.expirePointList = {                     \
	  .data = (void *)( Os_SchTblExpPointList_ ## _id ), \
	  .cnt = ARRAY_SIZE(Os_SchTblExpPointList_ ## _id), \
	 }, \
	 .autostartPtr = _autostart_ref,\
	.applOwnerId = _appl_owner, \
	.accessingApplMask = _accessing_appl_mask, \
}

/*
 *---------------------- SPINLOCKS -----------------------------------------
 */

#define GEN_SPINLOCK_HEAD OsSpinlockType spinlock_list[OS_SPINLOCK_CNT]

/**
 * _id
 *    Id of the spinlock
 * _name
 *    Name of the spinlock, string
 * _lock
 *    SPINLOCK_UNLOCKED, SPINLOCK_LOCKED
 */
#define GEN_SPINLOCK( _id, _name, _lock, _accessing_appl_mask )	\
{	                                                          	\
	.id   = _id,                                            	\
	.name = _name,                                            	\
	.lock = _lock,                                            	\
	.accessingApplMask = _accessing_appl_mask,                	\
}



#define GEN_HOOKS( _startup, _protection, _shutdown, _error, _pretask, _posttask ) \
const struct OsHooks os_conf_global_hooks = { \
		.StartupHook = _startup, 		\
		.ProtectionHook = _protection, 	\
		.ShutdownHook = _shutdown,		\
		.ErrorHook = _error,			\
		.PreTaskHook = _pretask,		\
		.PostTaskHook = _posttask		\
}

#define ALIGN_16(x) (((x)>>4)<<4)

#define DECLARE_STACK(_name,_size) \
	uint8_t stack_##_name[_size]

#define SECTION_BSS_SUPER	__attribute__ ((aligned (16),section(".bss")))
#define SECTION_BSS_USER	__attribute__ ((aligned (16),section(".bss")))

#endif







