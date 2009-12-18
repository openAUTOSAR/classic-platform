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
 * Os_Cfg.h
 *
 *  Created on: 2008-dec-22
 *      Author: mahi
 */

#ifndef OS_CFG_H_
#define OS_CFG_H_

#define COUNTER_ID_OsTick				0

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


// COUNTER_ID_OsTick located in Os.h

// NOT GENERATED( for test system only )
#define SYSTEM_COUNTER_PERIOD				100

#define PRIO_STACK_SIZE						2600
#define OS_INTERRUPT_STACK_SIZE				2048

// Just define them if you want to use them.
#undef ALARM_USE
#undef SCHEDULETABLE_USE
#undef MESSAGE_USE
#undef EVENT_USE
#undef SERVICE_USE

#define EVENT_0	(1<<0)
#define EVENT_1	(1<<1)
#define EVENT_2 (1<<2)


#endif /* OS_CFG_H_ */
