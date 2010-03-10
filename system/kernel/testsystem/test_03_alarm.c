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
 * Tests:
 *
 * OSEK
 * - GetAlarmBase				E_OS_ID
 * - GetAlarm      				E_OS_ID, E_OS_NOFUNC
 * - SetRelAlarm                E_OS_ID, E_OS_STATE, E_OS_VALUE
 * - SetAbsAlarm                E_OS_ID, E_OS_STATE, E_OS_VALUE
 * - CancelAlarm                E_OS_ID, E_OS_NOFUNC
 *
 * Autosar
 * - IncrementCounter			E_OS_ID
 * - GetCounterValue            E_OS_ID
 * - GetElapsedCounterValue     E_OS_ID, E_OS_VALUE
 *
 * More tests here:
 *
 * Limitations:
 */


#include "Os.h"
#include "os_test.h"
#include "arc.h"


void etask_sup_l_03(void) {
	_Bool done = 0;
	StatusType rv;
	AlarmBaseType alarmBase;
	TickType tick;

	while (!done) {
		TEST_RUN();
		switch (test_nr) {
		case 1:
			/* GetAlarmBase E_OS_ID */
			rv = GetAlarmBase(ALARM_ID_c_sys_activate_btask_h, &alarmBase);
			TEST_ASSERT(rv == E_OK);
			rv = GetAlarmBase(ALARM_ID_ILL, &alarmBase);
			TEST_ASSERT(rv == E_OS_ID);
			test_nr++;
			break;
		case 2:
			/* GetAlarm E_OS_ID and E_OS_NO_FUNC */
			rv = GetAlarm(ALARM_ID_c_sys_activate_btask_h,&tick);
			TEST_ASSERT(rv == E_OS_NOFUNC);
			rv = GetAlarm(ALARM_ID_ILL,&tick);
			TEST_ASSERT(rv == E_OS_ID);
			test_nr=100;
		case 100:
			TerminateTask();
			break;
		}
	}
}

void btask_sup_l_03(void) {
}


DECLARE_TEST_ETASK(03, etask_sup_l_03, NULL, NULL );
DECLARE_TEST_BTASK(03, btask_sup_l_03, NULL, NULL );
