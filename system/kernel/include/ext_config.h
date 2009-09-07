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

/* Created in Oil_Config */
struct pcb_s;
struct alarm_obj_s;
struct counter_obj_s;
struct sched_table_s;
//struct app_s;
struct rom_app_s;
struct resource_obj_s;
struct rom_app_s;
struct message_obj_s;
//extern struct pcb_s pcb_list[];
//extern struct app_s app_list[];
//extern struct rom_app_s rom_app_list[];

int Oil_GetApplCnt(void);
struct rom_app_s *Oil_GetApplObj( ApplicationType application_id );
int Oil_GetTaskCnt(void);
void *Oil_GetIdleProcStack(void);
int Oil_GetResourceCnt(void);
StatusType Oil_GetAlarmBase(AlarmType alarm_id, AlarmBaseRefType info);
uint32 Oil_GetAlarmCnt(void);
struct alarm_obj_s *Oil_GetAlarmObj( AlarmType alarm_id );
struct counter_obj_s *Oil_GetCounter(CounterType);
uint32 Oil_GetCounterCnt(void );
uint32 Oil_GetSchedCnt( void );
struct sched_table_s *Oil_GetSched( ScheduleTableType sched_id );
uint32 Oil_GetServiceCnt( void ) ;
struct resource_obj_s *Oil_GetResource( ResourceType resource );

struct message_obj_s *Oil_GetMessage(MessageType message_id);
uint32 Oil_GetMessageCnt(void );

#endif /*EXT_CONFIG_H_*/
