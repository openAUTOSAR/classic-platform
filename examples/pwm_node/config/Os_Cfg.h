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

#include <limits.h>

//	APPS
#define BLINKER_APP_ID						122
#define APPLICATION_CNT						1

// TASKS
#define TASK_ID_OsIdle						0
#define TASK_ID_bTask25						1
#define TASK_ID_bTask100					2
#define TASK_ID_Startup						3

void OsIdle( void );
void Startup( void );
void bTask25( void );
void bTask100( void );

// COUNTERS
#define COUNTER_ID_OsTick					0

// ALARMS
#define ALARM_USE
#define ALARM_ID_bTask25					0
#define ALARM_ID_bTask100					1

// RESOURCES
#define RES_ID_BLINK						1 // Den mysko RES_SCHEDULER är ju nr 0

// MISC
#define USE_IDLE_TASK
#define PRIO_STACK_SIZE						4096
#define OS_INTERRUPT_STACK_SIZE				4096
#define EVENT_BLINK	(1<<0)

#endif /* OS_CFG_H_ */
