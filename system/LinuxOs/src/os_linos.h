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




#ifndef OS_LINOS_H_
#define OS_LINOS_H_

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

void Os_IsrInit( void );

void StartOS( AppModeType Mode );
void ShutdownOS( StatusType );

StatusType GetResource( ResourceType ResID );
StatusType ReleaseResource( ResourceType ResID);

#endif /* OS_LINOS_H_ */
