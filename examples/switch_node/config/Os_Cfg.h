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
* Module vendor:  Autocore
* Module version: 1.0.0
* Specification: Autosar v3.0.1, Final
*
*/


#ifndef OS_CFG_H_
#define OS_CFG_H_

#define ALARM_USE

// Application
#define APPLICATION_ID_application_1	0
#define APPLICATION_CNT					1

// Alarm Id's
#define ALARM_ID_ComAlarm	0
#define ALARM_ID_ReadSwAlarm	1

// Counter Id's
#define COUNTER_ID_OsTick	0

// Event Id's

// Event masks

// Isr Id's

// Resource Id's

// Task Id's
#define TASK_ID_ComTask	0
#define TASK_ID_OsIdle	1
#define TASK_ID_ReadSwitches	2
#define TASK_ID_StartupTask	3

// Task entry points
void ComTask( void );
void OsIdle( void );
void ReadSwitches( void );
void StartupTask( void );

// Stack sizes
#define PRIO_STACK_SIZE				2048
#define OS_INTERRUPT_STACK_SIZE		2048

// Hooks
#define USE_ERRORHOOK
#define USE_POSTTASKHOOK
#define USE_PRETASKHOOK
#define USE_PROTECTIONHOOK
#define USE_SHUTDOWNHOOK
#define USE_STARTUPHOOK

#endif /*OS_CFG_H_*/
