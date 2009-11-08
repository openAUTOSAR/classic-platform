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









/*
 * Hold macros for the generator
 */
#ifndef _OS_CONFIG_MACROS_H
#define _OS_CONFIG_MACROS_H

#define false		0
#define true		1

// +1 here.. easy to have a reference..
#define GEN_TRUSTEDFUNCTIONS_LIST trusted_func_t os_cfg_trusted_list[SERVICE_CNT];

#define GEN_APPLICATION_HEAD rom_app_t rom_app_list[] =

#define GEN_APPLICATON(	_id,_name,_trusted,_startuphook,_shutdownhook, \
						_errorhook,_isr_mask,_scheduletable_mask, _alarm_mask, \
						_counter_mask,_resource_mask,_message_mask ) \
{												\
	.application_id = _id, 						\
	.name = _name,								\
	.trusted = _trusted,						\
	.StartupHook = _startuphook,				\
	.ShutdownHook = _shutdownhook,				\
	.ErrorHook = _errorhook,					\
	.isr_mask = _isr_mask,						\
	.scheduletable_mask  = _scheduletable_mask,	\
	.alarm_mask  = _alarm_mask,					\
	.counter_mask  = _counter_mask,				\
	.resource_mask  = _resource_mask,			\
	.message_mask  = _message_mask,				\
}


#define GEN_TASK_HEAD const rom_pcb_t rom_pcb_list[] =


#define GEN_ETASK( _id, _priority, 	_autostart, _timing_protection, _application_id, _resource_int_p ) \
{									\
	.pid = TASK_ID_##_id,           \
	.name = #_id,					\
	.entry = _id,				\
	.prio = _priority,				\
	.proc_type = PROC_EXTENDED,		\
	.stack.size = sizeof stack_##_id,	\
	.stack.top = stack_##_id,		\
	.autostart = _autostart,		\
	.timing_protection = _timing_protection,\
	.application_id = _application_id,		\
	.resource_int_p = _resource_int_p, \
}

#define GEN_BTASK( _id, _priority, 	_autostart, _timing_protection, _application_id, _resource_int_p ) \
{									\
	.pid = TASK_ID_##_id,           \
	.name = #_id,					\
	.entry = _id,				\
	.prio = _priority,				\
	.proc_type = PROC_BASIC,		\
	.stack.size = sizeof stack_##_id,	\
	.stack.top = stack_##_id,		\
	.autostart = _autostart,		\
	.timing_protection = _timing_protection,\
	.application_id = _application_id,		\
	.resource_int_p = _resource_int_p, \
}


#define GEN_TASK( _id, _name, _entry, _priority, _process_type, _stack_size, _stack_top, \
				_autostart, _timing_protection, _application_id, _resource_int_p ) \
{									\
	.pid = _id,						\
	.name = _name,					\
	.entry = _entry,				\
	.prio = _priority,				\
	.proc_type = _process_type,		\
	.stack.size = _stack_size,		\
	.stack.top = _stack_top,		\
	.autostart = _autostart,		\
	.timing_protection = _timing_protection,\
	.application_id = _application_id,		\
	.resource_int_p = _resource_int_p, \
}

#define GEN_ISR_2( _id, _name, _entry, _priority, _process_type, _vector,  _timing_protection, _application_id ) \
{									\
	.pid = _id,						\
	.name = _name,					\
	.entry = _entry,				\
	.prio = _priority,				\
	.proc_type = _process_type,		\
	.vector = _vector,              \
	.timing_protection = _timing_protection,\
	.application_id = _application_id,		\
}


#define GEN_ISR_1( _id, _name, _entry, _priority , _vector ) \
{									\
	.pid = _id,						\
	.name = _name,					\
	.entry = _entry,				\
	.prio = _priority,				\
	.proc_type = PROC_ISR1,		\
	.vector = _vector,              \
}

#define GEN_PCB_LIST()	uint8_t pcb_list[PCB_T_SIZE*ARRAY_SIZE(rom_pcb_list)];

#define GEN_RESOURCE_HEAD resource_obj_t resource_list[] =
#define GEN_RESOURCE( _id, _type, _ceiling_priority, _application_id, _task_mask) \
{												\
	.nr= _id,									\
	.type= _type,								\
	.ceiling_priority = _ceiling_priority,		\
	.application_owner_id = _application_id,	\
	.task_mask = _task_mask,					\
	.owner = (-1),								\
}

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

#define GEN_COUNTER_HEAD counter_obj_t counter_list[] =
#define GEN_COUNTER( _id, _name, _type, _unit, 	\
					_maxallowedvalue, 			\
					_ticksperbase, 				\
					_mincycle,          \
					_gpt_ch ) 				\
{												\
	.type = _type,								\
	.unit = _unit,								\
	.alarm_base.maxallowedvalue = _maxallowedvalue,	\
	.alarm_base.tickperbase = _ticksperbase,		\
	.alarm_base.mincycle = _mincycle,				\
	.driver.OsGptChannelRef = _gpt_ch  \
}

#define GEN_ALARM_HEAD alarm_obj_t alarm_list[] =

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
 * _autostart_active
 *     If the alarm should be auto-started.
 *
 * _autostart_alarmtime
 *     Only used if active = 1
 *
 * _autostart_cycletime
 *     Only used if active = 1
 *
 * _autostart_application_mode_mask
 *     Set to 0 for now
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
#define GEN_ALARM( _id, _name, _counter_id,	\
			_autostart_active,				\
			_autostart_alarmtime,			\
			_autostart_cycletime,			\
			_autostart_application_mode_mask,\
			_action_type,					\
			_action_task_id,				\
			_action_event_id,				\
			_action_counter_id )			\
{											\
	.name = _name,							\
	.counter = &counter_list[_counter_id],	\
	.counter_id = _counter_id,				\
	.autostart = {							\
		.active = _autostart_active,		\
		.alarmtime = _autostart_alarmtime,	\
		.cycletime = _autostart_cycletime,	\
		.appmode_mask = _autostart_application_mode_mask,	\
	},										\
	.action = {								\
		.type = _action_type,				\
		.task_id = _action_task_id,			\
		.event_id = _action_event_id,		\
		.counter_id = _action_counter_id 	\
	},										\
}

#define GEN_SCHEDULETABLE_HEAD sched_table_t sched_list[] =
#define GEN_SCHEDULETABLE(  _id, _name, _counter, _repeating, _length, _application_mask, \
							_action_cnt, _action_expire_ref, \
							_autostart_active, _autostart_type, _autostart_rel_offset, _autostart_appmode, \
							_sync_strategy, _sync_explicit_precision , \
							_adj_max_advance,_adj_max_retard  ) \
{												\
	.name = _name,						\
	.counter = &counter_list[_counter],	\
	.repeating = _repeating,			\
	.length = _length,					\
	.app_mask = _application_mask,		\
	.action_list = SA_LIST_HEAD_INITIALIZER(_action_cnt,_action_expire_ref), \
	.autostart = { \
		.active = _autostart_active,		\
		.type = _autostart_type,	\
		.relOffset = _autostart_rel_offset,	\
		.appModeRef = _autostart_appmode,	\
	}, \
	.sync = { \
		.syncStrategy = _sync_strategy,		\
		.explicitPrecision = _sync_explicit_precision,	\
	}, \
	.adjExpPoint = { \
		.maxAdvance = _adj_max_advance,		\
		.maxRetard = _adj_max_retard,	\
	} \
}

#define GEN_HOOKS( _startup, _protection, _shutdown, _error, _pretask, _posttask ) \
struct os_conf_global_hooks_s os_conf_global_hooks = { \
		.StartupHook = _startup, 		\
		.ProtectionHook = _protection, 	\
		.ShutdownHook = _shutdown,		\
		.ErrorHook = _error,			\
		.PreTaskHook = _pretask,		\
		.PostTaskHook = _posttask,		\
};

#define GEN_IRQ_VECTOR_TABLE_HEAD 	\
		 void * Irq_VectorTable[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS] =

#define GEN_IRQ_ISR_TYPE_TABLE_HEAD \
		 uint8_t Irq_IsrTypeTable[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS]  =

#define GEN_IRQ_PRIORITY_TABLE_HEAD \
		 uint8_t Irq_PriorityTable[NUMBER_OF_INTERRUPTS_AND_EXCEPTIONS]  =

#define ALIGN_16(x) (((x)>>4)<<4)

#define DECLARE_STACK(_name,_size) \
	uint8_t stack_##_name[_size]

#define SECTION_BSS_SUPER	__attribute__ ((aligned (16),section(".bss")))
#define SECTION_BSS_USER	__attribute__ ((aligned (16),section(".bss")))

#undef ALARM_CNT
#undef SCHEDULETABLE_CNT
#undef MESSAGE_CNT
#undef EVENT_CNT
#undef SERVICE_CNT

#endif







