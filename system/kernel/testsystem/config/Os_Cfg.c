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
 * Configuration
 * ------------------------------------------------
 * 3 Tasks
 *  - 3 extended ( do NOT terminate )
 *  - 3 basic ( terminate )
 *
 * 2 Counters
 *  - 1 system timer
 *  - 1 soft( incremented with IncrementCounter(X) )
 *
 *
 * 2 Alarms
 *  - 1 hardware alarm
 *  - 1 soft alarm
 *    - Running on the soft counter above
 *    - Triggers the basic task through ActivateTask() in alarm
 *
 * 2 ISRs
 *  - 1 ISR type 1
 *  - 1 ISR type, system counter
 *
 * 1 Schedule Table
 *  - Running on the system timer
 *  - Using 3 Tasks
 *
 *
 *
 * System timer
 * ----------------------------------
 * A system MUST have a system counter to function. This is almost
 * always a hardware timer. So where is this timer configured?
 *
 * decrementer exception
 * --> generate a prioritised interrupt through INTC soft interrupt
 *
 * Using the decrementer without generating the soft interrupt is not
 * recommended since it's not prioritised( it's and exception that blocks
 * interrupts from INTC )
 *
 * The timers are initialized in EcuM() using Gpt_Init(X). A StartupHook() must
 * be used to start the timer. Example
 *
 * 	Gpt_StartTimer( GPT_CHANNEL_DEC, 10000UL);
 *	Gpt_EnableNotification(GPT_CHANNEL_DEC);
 *
 *                                  Note!
 *       The counter configuration, e.g. timeout is not yet configured
 *       through the counter in the configuration.
 *
 * ISR's
 * ----------------------------------
 * dec_exception ( From Mcu_Exception.S )
 *  This exception installed HARD in the exception_tbl( Mcu_Exceptions )
 *
 *
 * Note!
 * - Do NOT use internal resource unless you are very sure that you to it right..
 *
 *
 *
 */

#include <stdlib.h>
#include <stdint.h>
#include "os_config_macros.h"
#include "Platform_Types.h"
#include "Os.h"				// includes Os_Cfg.h
#include "os_test.h"
#include "kernel.h"
#include "kernel_offset.h"
#include "alist_i.h"
#include "Mcu.h"

OsTickType OsTickFreq = 1000;

extern void dec_exception( void );

// atleast 1
#define SERVICE_CNT 1

GEN_TRUSTEDFUNCTIONS_LIST


//-------------------------------------------------------------------

GEN_APPLICATION_HEAD {
	GEN_APPLICATON(0,"application_1",true,NULL,NULL,NULL , 0,0,0,0,0,0 )
};
//-------------------------------------------------------------------

#define ALIGN_16(x) (((x)>>4)<<4)

//
uint8 stack_OsIdle[STACK_SIZE_OsIdle] SECTION_BSS_SUPER;
uint8 stack_etask_master[STACK_SIZE_etask_master] SECTION_BSS_SUPER;
uint8 stack_etask_sup_l[STACK_SIZE_etask_sup_l] SECTION_BSS_SUPER;
uint8 stack_etask_sup_m[STACK_SIZE_etask_sup_m] SECTION_BSS_SUPER;
uint8 stack_etask_sup_h[STACK_SIZE_etask_sup_h] SECTION_BSS_SUPER;
uint8 stack_btask_sup_l[STACK_SIZE_btask_sup_l] SECTION_BSS_SUPER;
uint8 stack_btask_sup_m[STACK_SIZE_btask_sup_m] SECTION_BSS_SUPER;
uint8 stack_btask_sup_h[STACK_SIZE_btask_sup_h] SECTION_BSS_SUPER;

#define INTC_VECTOR_EXCEPTION_DEC		(320+10)
#define INTC_VECTOR_SSCIR0				0
#define INTC_VECTOR_SSCIR7				7

//-------------------------------------------------------------------

GEN_RESOURCE_HEAD {
	GEN_RESOURCE(RES_SCHEDULER,RESOURCE_TYPE_STANDARD,0,0,0),		// Standard resource..
// Internal resources
	GEN_RESOURCE(1,RESOURCE_TYPE_INTERNAL,8, APPLICATION_ID_application_1,(1<<TASK_ID_etask_sup_l)),
// external resource
	GEN_RESOURCE(2,RESOURCE_TYPE_STANDARD,0,0,0),
};

//-------------------------------------------------------------------

GEN_TASK_HEAD {

	GEN_ETASK(OsIdle,0,true/*auto*/, NULL/*tm*/, APPLICATION_ID_application_1/*app*/,NULL/*rsrc*/),

/* extended */
	GEN_ETASK(etask_master,1,true/*auto*/, NULL/*tm*/, APPLICATION_ID_application_1/*app*/,NULL/*rsrc*/),

	GEN_ETASK(etask_sup_l,2,false/*auto*/, NULL/*tm*/, APPLICATION_ID_application_1/*app*/,NULL/*rsrc*/),
	GEN_ETASK(etask_sup_m,3,false/*auto*/, NULL/*tm*/, APPLICATION_ID_application_1/*app*/,NULL/*rsrc*/),
	GEN_ETASK(etask_sup_h,4,false/*auto*/, NULL/*tm*/, APPLICATION_ID_application_1/*app*/,NULL/*rsrc*/),

/* basic */
	GEN_BTASK(btask_sup_l,2,false/*auto*/, NULL/*tm*/, APPLICATION_ID_application_1/*app*/,NULL/*rsrc*/),
	GEN_BTASK(btask_sup_m,3,false/*auto*/, NULL/*tm*/, APPLICATION_ID_application_1/*app*/,NULL/*rsrc*/),
	GEN_BTASK(btask_sup_h,4,false/*auto*/, NULL/*tm*/, APPLICATION_ID_application_1/*app*/,NULL/*rsrc*/),

	GEN_ISR_2(  TASK_ID_os_tick, "dec", OsTick, /*prio*/ 11, /*type*/ PROC_ISR2,  INTC_VECTOR_EXCEPTION_DEC , NULL, APPLICATION_ID_application_1),
#if 0
	// Use the intc_vector tables for now
	GEN_ISR_2(  TASK_ID_isr_dec, "dec", my_dec, /*prio*/ 11, /*type*/ PROC_ISR2,  INTC_VECTOR_EXCEPTION_DEC , NULL, APPLICATION_ID_application_1),
	GEN_ISR_1(  TASK_ID_isr_soft7, "exception_sc", my_intc_soft7, /*prio*/ 12, INTC_VECTOR_SSCIR7 ),
#endif
};


GEN_PCB_LIST()

//-------------------------------------------------------------------
typedef void (*exc_func_t)(uint32_t *);

// Print all to stdout
uint32 os_dbg_mask = 0;

#if 0
uint32 os_dbg_mask = \
	D_MASTER_PRINT |\
	D_ISR_MASTER_PRINT |\
	D_STDOUT |\
	D_ISR_STDOUT;
#endif


//	D_ALARM | D_TASK;

// --- INTERRUPTS ---

uint8_t os_interrupt_stack[OS_INTERRUPT_STACK_SIZE] __attribute__ ((aligned (0x10)));

GEN_IRQ_VECTOR_TABLE_HEAD {};
GEN_IRQ_ISR_TYPE_TABLE_HEAD {};
GEN_IRQ_PRIORITY_TABLE_HEAD {};

//-------------------------------------------------------------------

// Generate as <type> <msg_name>_data
#ifdef ALARM_USE
int MsgRx_1_data;
int MsgTx_1_data;

message_obj_t message_list[] = {
{
	.property = RECEIVE_UNQUEUED_INTERNAL,
	.data = &MsgRx_1_data,
	.data_size = sizeof(MsgRx_1_data),

},
{
	.property = SEND_STATIC_INTERNAL,
	.data = &MsgTx_1_data,
	.data_size = sizeof(MsgTx_1_data),
},
};
#endif


#if 0
#define MESSAGE_CLASS(_name,_ctype ,_property,_queued ,_notification) \
	struct message ## _name ## _s {	\
		_ctype a;				\
	};

MESSAGE_CLASS(rx1,int,,,);
#endif

/*
typedef struct {
	message_type_t type;		// RECEIVE_UNQUEUED_INTERNAL, RECEIVE_QUEUE_INTERNAL
	MessageType send_id;
	message_notification_t *notification;
	void *queue_data;
	uint32 	queue_size;
} message_rx_t;

#define DeclareMessage(x)		(x)

#define msgTx_1			0
#define msgRx_1			1
*/

//-------------------------------------------------------------------
#if MESSAGE_USE!=0


//  SEND_STATIC_INTERNAL is the only message that needs an object



// RECEIVE_UNQUEUED_INTERNAL, RECEIVE_QUEUE_INTERNAL have notification and



#endif
//-------------------------------------------------------------------

//#define COUNTER_SOFT_1		1
//#define EVENT_1				1


GEN_COUNTER_HEAD {
	GEN_COUNTER(COUNTER_ID_os_tick,	"COUNTER_ID_OsTick",COUNTER_TYPE_HARD,
				COUNTER_UNIT_NANO, 0xffff,1,1,0 ),
	GEN_COUNTER(COUNTER_ID_soft_1,	"counter_soft_1",COUNTER_TYPE_SOFT,
				COUNTER_UNIT_NANO, 10,1,1,0),
	GEN_COUNTER(COUNTER_ID_soft_2,	"counter_soft_2",COUNTER_TYPE_SOFT,
				COUNTER_UNIT_NANO, 100,1,1,0),
};

CounterType Os_Arc_OsTickCounter = COUNTER_ID_OsTick;

//-------------------------------------------------------------------

#ifdef ALARM_USE
GEN_ALARM_HEAD {
	{
		.counter = &counter_list[COUNTER_ID_OsTick],
		.counter_id = COUNTER_ID_OsTick,
		.action =
		{
				.type = ALARM_ACTION_ACTIVATETASK,
				.task_id = TASK_ID_btask_sup_h,
				.event_id = 0,
		}
	},
		{
		/* Set EVENT_1 in etask_sup_m, driven by soft counter */
		.counter = &counter_list[COUNTER_ID_OsTick],
		.counter_id = COUNTER_ID_OsTick,
		.action = {
				.type = ALARM_ACTION_SETEVENT,
				.task_id = TASK_ID_etask_sup_m,
				.event_id = EVENT_1,
		}
	},{
		/* Set EVENT_1 in etask_sup_m, driven by counter_soft_1 */
		.counter = &counter_list[COUNTER_ID_soft_1],
		.counter_id = COUNTER_ID_soft_1,
		.action = {
			.type = ALARM_ACTION_SETEVENT,
			.task_id = TASK_ID_etask_sup_m,
			.event_id = EVENT_1,
		}
	},{
		.counter = &counter_list[COUNTER_ID_soft_1],
		.counter_id = COUNTER_ID_soft_1,
		.action = {
			.type = ALARM_ACTION_INCREMENTCOUNTER,
			.counter_id = COUNTER_ID_soft_2,
		}
	}
};
#endif

//-------------------------------------------------------------------


#if defined(SCHEDULETABLE_USE)


sched_action_t sched_expire_list_0[] = {
	{
		.type = SCHEDULE_ACTION_ACTIVATETASK,
		.offset = 5,
		.task_id = TASK_ID_etask_sup_m,
	},{
		.type = SCHEDULE_ACTION_SETEVENT,
		.offset = 7,
		.task_id = TASK_ID_etask_sup_m,
		.event_id = EVENT_2,
	}
};


sched_action_t sched_expire_list_1[] = {
	{
		.type = SCHEDULE_ACTION_ACTIVATETASK,
		.offset = 2,
		.task_id = TASK_ID_etask_sup_m,
	}
};


GEN_SCHEDULETABLE_HEAD {
	GEN_SCHEDULETABLE(
			0,						// id
			"stable0",				// name
		    COUNTER_ID_soft_2,		// counter
			1,						// periodic
			SCHEDULETABLE_DURATION_1,	// duration
			0,						// app_mask
			ARRAY_SIZE(sched_expire_list_0),				// action count
			sched_expire_list_0, 	// expire ref
			0,0,0,0, 				// autostart
			NONE,0,					// sync
			0,0 					// adjExpPoint
			),

	GEN_SCHEDULETABLE(
			1,						// id
			"stable1",				// name
			COUNTER_ID_soft_2,		// counter
			1,						// periodic
			SCHEDULETABLE_DURATION_2,	// duration
			0,						// app_mask
			ARRAY_SIZE(sched_expire_list_1),				// action count
			sched_expire_list_1,	// expire ref
			0,0,0,0, 				// autostart
			NONE,0,					// sync
			0,0 					// adjExpPoint
			),
};

#endif


// --- HOOKS ---

struct os_conf_global_hooks_s os_conf_global_hooks = {
		.StartupHook = StartupHook,
		.ProtectionHook = ProtectionHook,
		.ShutdownHook = ShutdownHook,
		.ErrorHook = ErrorHook,
		.PreTaskHook = PreTaskHook,
		.PostTaskHook = PostTaskHook,
};


#include "os_config_funcs.h"
