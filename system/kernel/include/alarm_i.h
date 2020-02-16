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

#ifndef ALARM_I_H_
#define ALARM_I_H_

#include "counter_i.h"

/* STD container : OsAlarmSetEvent
 * OsAlarmSetEventRef:			1
 * OsAlarmSetEventTaskRef:		1
 * */

/* STD container : OsAlarmActivateTask
 * OsAlarmActivateTaskRef:		1
 * */

/* STD container : OsAlarmCallback
 * OsAlarmCallbackName:			1    Function Ptr
 * */

/* STD container : OsAlarmIncrementCounter
 * OsAlarmIncrementCounterRef:	1    Ref to counter
 * */


struct OsCounter;

typedef enum alarm_action_type_e {
	ALARM_ACTION_ACTIVATETASK=0,
	ALARM_ACTION_SETEVENT,
	ALARM_ACTION_ALARMCALLBACK,		/* Only class 1 */
	ALARM_ACTION_INCREMENTCOUNTER/* SWS OS302 */
} alarm_action_type_t;


/* STD container : OsAlarmAction
 * OsAlarmActivateTask:			0..1
 * OsAlarmCallback:				0..1
 * OsAlarmIncrementCounter:		0..1
 * OsAlarmSetEvent:				0..1
 * */
typedef struct OsAlarmAction {
	alarm_action_type_t type;
	TaskType 			task_id;
	EventMaskType 		event_id;
	CounterType 		counter_id;
} OsAlarmActionType;


enum OsAlarmAutostartTypeType {
	// Start with SetAbsAlarm()
	ALARM_AUTOSTART_ABSOLUTE,
	// Start with SetRelAlarm()
	ALARM_AUTOSTART_RELATIVE
};


/* STD container : OsAlarmAutostart
 * OsAlarmAlarmTime: 	 		1    Int
 * OsAlarmAutoStartType: 		1    Int, ABSOLUTE, RELATIVE
 * OsAlarmCycleTime: 	 		1    Int
 * OsAlarmAppModeRef: 	 		1..*
 */
typedef struct OsAlarmAutostart {
	uint32 alarmTime;
	enum OsAlarmAutostartTypeType autostartType;
	uint32 cycleTime;
	AppModeType appModeRef;
} OsAlarmAutostartType;

/* STD container : OsAlarm
 * OsAlarmAccessingApplication: 0..* Ref to OS application
 * OsAlamCounterRef:            1    Ref to counter
 * OsAlarmAction[C]             1    Action when alarm expires
 * OsAlarmAutostart[C]          0..1 Autostart
 */
typedef struct OsAlarm {
	char 	*name;
	struct OsCounter *counter;		/* Reference to counter */

#if	(OS_USE_APPLICATIONS == STD_ON)
	ApplicationType applOwnerId;
	uint32 			accessingApplMask;
#endif

	CounterType counter_id;
	uint32 		alarmtime;		/* cycle, 0 = no cycle */
	uint32 		cycletime;
	uint32 		app_mask;
	uint32 		expire_val;
	_Bool 		active;			/* if the alarm is active or not */

	OsAlarmActionType action;	/* Action attributes when alarm expires. */

	const struct OsAlarmAutostart *autostartPtr;

	SLIST_ENTRY(OsAlarm) alarm_list;		/* List of alarms connected to the same counter */

#if (OS_SC1 == STD_ON)
	/* TODO: OS242, callback in scalability class 1 only..*/
	/** Missing req OS242 */
#endif

} OsAlarmType;

#if OS_ALARM_CNT!=0
extern GEN_ALARM_HEAD;
#endif

void Os_AlarmCheck(OsCounterType *c_p);
void Os_AlarmAutostart(void);

static inline OsAlarmType *Os_AlarmGet( AlarmType alarm_id ) {
#if (OS_ALARM_CNT!=0)
	if( alarm_id < OS_ALARM_CNT) {
	  return &alarm_list[alarm_id];
	} else {
		return NULL;
	}
#else
	(void)alarm_id;
	return NULL;
#endif
}


#if OS_ALARM_CNT!=0
static inline StatusType Os_AlarmGetBase(AlarmType alarm_id, AlarmBaseRefType info) {

	StatusType rv = E_OK;

	if( alarm_id >= OS_ALARM_CNT ) {
		rv = E_OS_ID;
	} else {
#if (OS_ALARM_CNT!=0)
		*info = alarm_list[alarm_id].counter->alarm_base;
#endif
	}
	return rv;
}
#endif


static inline ApplicationType Os_AlarmGetApplicationOwner( AlarmType id ) {
	ApplicationType rv;
#if (OS_ALARM_CNT!=0)
	rv = (id < OS_ALARM_CNT) ? Os_AlarmGet(id)->applOwnerId : INVALID_OSAPPLICATION;
#else
	(void)id;
	rv = INVALID_OSAPPLICATION;
#endif
	return rv;
}

#endif /*ALARM_I_H_*/
