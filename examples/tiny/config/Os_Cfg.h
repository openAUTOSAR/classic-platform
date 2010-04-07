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



#ifndef OS_CFG_H_
#define OS_CFG_H_


#if (OS_SW_MAJOR_VERSION != 2)
#error "Os: Configuration file version differs from BSW version."
#endif



/* os_config.h */

#define APPLICATION_ID_application_1	0
#define APPLICATION_CNT					1

#define TASK_ID_OsIdle					0
#define TASK_ID_etask_1					1
#define TASK_ID_etask_2					2
#define TASK_ID_btask_3					3

void etask_1( void );
void etask_2( void );
void btask_3( void );

//#define TASK_ID_os_tick					8

#define COUNTER_ID_OsTick				0

// NOT GENERATED( for test system only )
#define SYSTEM_COUNTER_PERIOD				100
#define OS_TICK_DURATION_IN_US				2000

#if defined(USE_SIMPLE_PRINTF)
#define PRIO_STACK_SIZE						1000
#else
#define PRIO_STACK_SIZE						200
#endif

#define OS_INTERRUPT_STACK_SIZE				200

#define EVENT_0	(1<<0)
#define EVENT_1	(1<<1)
#define EVENT_2 (1<<2)

#define OS_ALARM_CNT				1
#define OS_TASK_CNT				4
#define OS_COUNTER_CNT				1
#define OS_EVENTS_CNT				3
#define OS_ISRS_CNT				0
#define OS_RESOURCE_CNT			1
#define OS_LINKED_RESOURCE_CNT		0
#define OS_SCHTBL_CNT				0

#define CFG_OS_DEBUG				STD_ON


/*
 * OsOs container
 */
#define OS_SC1 					STD_ON 		/* | OS_SC2 | OS_SC3 | OS_SC4 */
#define OS_STACK_MONITORING		STD_ON
#define OS_STATUS_EXTENDED			STD_ON 		/* OS_STATUS_STANDARD */
#define OS_USE_GET_SERVICE_ID		STD_ON
#define OS_USE_PARAMETER_ACCESS	STD_ON
#define OS_RES_SCHEDULER			STD_ON


#endif /* OS_CFG_H_ */
