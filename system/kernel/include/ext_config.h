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

#ifndef EXT_CONFIG_H_
#define EXT_CONFIG_H_

/* Created in Os_CfgConfig */
struct OsTaskVar;
struct OsAlarm;
struct OsCounter;
struct OsSchTbl;
struct OsResource;
struct OsMessage;
#if	(OS_USE_APPLICATIONS == STD_ON)
struct OsRomApplication;
#endif

#if	(OS_USE_APPLICATIONS == STD_ON)
extern const OsAppConstType Os_AppConst[OS_APPLICATION_CNT];

struct OsRomApplication *Os_CfgGetApplObj( ApplicationType application_id );
#endif
void *Os_CfgGetIdleProcStack(void);
StatusType Os_CfgGetAlarmBase(AlarmType alarm_id, AlarmBaseRefType info);
struct OsAlarm *Os_CfgGetAlarmObj( AlarmType alarm_id );
struct OsCounter *Os_CfgGetCounter(CounterType);
struct OsSchTbl *Os_CfgGetSched( ScheduleTableType sched_id );
struct OsResource *Os_CfgGetResource( ResourceType resource );
struct OsMessage *Os_CfgGetMessage(MessageType message_id);
void Os_CfgValidate( void );

#endif /*EXT_CONFIG_H_*/
