
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

#include "linos_logger.h" /* Logger functions */
#include "Os.h"
#include <os_sys.h>  /* OS_SYS_PTR */
#include <os_i.h>	/* GEN_TASK_HEAD  */


void Os_AlarmAutostart(void) {
	AlarmType j;
	logger_mod((LOGGER_MOD_LINOS|LOGGER_SUB_OS_ALARM), LOG_INFO,"Os_AlarmAutostart");
	for (j = 0; j < OS_ALARM_CNT; j++) {
		if (Os_OnRunningCore(OBJECT_ALARM,j)) {
			OsAlarmType *alarmPtr;
			alarmPtr = Os_AlarmGet(j);
			if (alarmPtr->autostartPtr != NULL) {
				const OsAlarmAutostartType *autoPtr = alarmPtr->autostartPtr;
				logger_mod((LOGGER_MOD_LINOS|LOGGER_SUB_OS_ALARM), LOG_INFO, "Setting Alarm %s", alarmPtr->name);

				if (OS_SYS_PTR->appMode & autoPtr->appModeRef) {
					if (autoPtr->autostartType == ALARM_AUTOSTART_ABSOLUTE) {
						logger_mod((LOGGER_MOD_LINOS|LOGGER_SUB_OS_ALARM), LOG_INFO,
								"Os_AlarmAutostart. Setting ABS Alarm %s alarmTime: %u cycleTime: %u", 
								alarmPtr->name, autoPtr->alarmTime, autoPtr->cycleTime);
						(void)SetAbsAlarm(j, autoPtr->alarmTime, autoPtr->cycleTime);
					} else {
						logger_mod((LOGGER_MOD_LINOS|LOGGER_SUB_OS_ALARM), LOG_INFO,
								"Os_AlarmAutostart. Setting Rel Alarm %s alarmTime: %u cycleTime: %u", 
								alarmPtr->name, autoPtr->alarmTime, autoPtr->cycleTime);
						(void)SetRelAlarm(j, autoPtr->alarmTime, autoPtr->cycleTime);
					}
				}
				else
				{
					logger(LOG_ERR, "Os_AlarmAutostart OS_SYS_PTR->appMode %u & autoPtr->appModeRef %u FALSE. (sint32)",
							OS_SYS_PTR->appMode,
							autoPtr->appModeRef);
				}
			}
		}
	}
}


/**
 *
 * @param a_obj
 */
static void AlarmProcess( OsAlarmType *aPtr ) {
        if( aPtr->cycletime == 0 ) {
                aPtr->active = 0;
        } else {
                // Calc new expire value..
                aPtr->expire_val = Os_CounterAdd( Os_CounterGetValue(aPtr->counter),
                                                                                        Os_CounterGetMaxValue(aPtr->counter),
                                                                                        aPtr->cycletime);
        }
}


void Os_AlarmCheck( const OsCounterType *c_p ) {
	OsAlarmType *a_obj;
	StatusType rv;

	SLIST_FOREACH(a_obj,&c_p->alarm_head,alarm_list) {
		// logger(LOG_INFO,"Os_AlarmCheck a_obj->active %d  c_p->val %d a_obj->expire_val %d", a_obj->active, c_p->val, a_obj->expire_val);
		if( a_obj->active && (c_p->val == a_obj->expire_val) ) {
			/* Check if the alarms have expired */
			logger_mod((LOGGER_MOD_LINOS|LOGGER_SUB_OS_ALARM), LOG_INFO,"Os_AlarmCheck expired %s(%d) cycletime (%d)\n", a_obj->name, a_obj->action.task_id, a_obj->cycletime);

			switch( a_obj->action.type ) {
				case ALARM_ACTION_ACTIVATETASK:
					logger_mod((LOGGER_MOD_LINOS|LOGGER_SUB_OS_ALARM), LOG_INFO, "Os_AlarmCheck ActivateTask (%d)", a_obj->action.task_id);
					if( ActivateTask(a_obj->action.task_id) != E_OK ) {
						/* We actually do thing here, See 0S321 */
					}
					AlarmProcess(a_obj);
					break;
				case ALARM_ACTION_SETEVENT:
					rv =  SetEvent(a_obj->action.task_id,a_obj->action.event_id);
					if( rv != E_OK ) {
						Os_CallErrorHook(rv);
					}
					AlarmProcess(a_obj);
					break;
				case ALARM_ACTION_ALARMCALLBACK:
					/* IMPROVEMENT: not done */
					break;

				case ALARM_ACTION_INCREMENTCOUNTER:
					/** @req OS301 */
					logger_mod((LOGGER_MOD_LINOS|LOGGER_SUB_OS_ALARM), LOG_INFO, "Os_AlarmCheck 			ALARM_ACTION_INCREMENTCOUNTER");
					(void)IncrementCounter(a_obj->action.counter_id);
					AlarmProcess(a_obj);
					break;
				default:
					ASSERT(0);
			}
		}
	}
}

