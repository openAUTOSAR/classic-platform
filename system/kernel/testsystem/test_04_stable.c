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
 * From Autosar 3.1
 *
 * Tests:
 * - StartScheduleTableRel          E_OS_ID, E_OS_VALUE, E_OS_STATE
 * - StartScheduleTableAbs          E_OS_ID, E_OS_VALUE, E_OS_STATE
 * - StopScheduleTable				E_OS_ID, E_OS_NO_FUNC
 * - NextScheduleTable				E_OS_ID, E_OS_NO_FUNC, E_OS_STATE
 * - GetScheduleTableStatus			E_OS_ID,
 *
 *   Class 2 and 4
 * - StartScheduleTableSynchrone    E_OS_ID, E_OS_STATE
 * - SyncScheduleTable              E_OS_ID, E_OS_VALUE, E_OS_STATE
 * - SetScheduleTableAsync			E_OS_ID,
 *
 * More tests here:
 *
 * Limitations:
 */

#include "Os.h"
#include "os_test.h"
#include "arc.h"


void etask_sup_l_04(void) {
	_Bool done = 0;
	StatusType rv;

	while (!done) {
		TEST_RUN();
		switch (test_nr) {
		case 1:
			/* StartScheduleTableRel E_OS_ID, E_OS_VALUE, E_OS_STATE */
			rv = StartScheduleTableRel(SCHEDULE_TABLE_ID_ILL,1);
			TEST_ASSERT(rv==E_OS_ID);
			rv = StartScheduleTableRel(SCHEDULE_TABLE_0,0);
			TEST_ASSERT(rv==E_OS_VALUE);
			rv = StartScheduleTableRel(SCHEDULE_TABLE_0,1);
			TEST_ASSERT(rv==E_OK);
			rv = StartScheduleTableRel(SCHEDULE_TABLE_0,1);
			TEST_ASSERT(rv==E_OS_STATE);
			rv = StopScheduleTable(SCHEDULE_TABLE_0);
			test_nr++;
			break;
		case 2:
			/* StartScheduleTableAbs E_OS_ID, E_OS_VALUE, E_OS_STATE */
			rv = StartScheduleTableAbs(SCHEDULE_TABLE_ID_ILL,1);
			TEST_ASSERT(rv==E_OS_ID);
#if 0
			/* TODO:Must supply an offset bigger than OsCounterMaxAllowedValue */
			StartScheduleTableAbs(SCHEDULE_TABLE_0,0);
			TEST_ASSERT(rv==E_OS_VALUE);
#endif
			rv = StartScheduleTableAbs(SCHEDULE_TABLE_0,1);
			TEST_ASSERT(rv==E_OK);
			rv = StartScheduleTableAbs(SCHEDULE_TABLE_0,1);
			TEST_ASSERT(rv==E_OS_STATE);
			rv = StopScheduleTable(SCHEDULE_TABLE_0);
			test_nr++;
			break;
		case 3:
			/* StopScheduleTable E_OS_ID, E_OS_NO_FUNC */
			rv = StopScheduleTable(SCHEDULE_TABLE_ID_ILL);
			TEST_ASSERT(rv==E_OS_ID);
			rv = StopScheduleTable(SCHEDULE_TABLE_0);
			TEST_ASSERT(rv==E_OS_NOFUNC);
			test_nr++;
			break;
		case 4:
			/* NextScheduleTable	E_OS_ID, E_OS_NO_FUNC, E_OS_STATE */
			rv = NextScheduleTable(SCHEDULE_TABLE_ID_ILL, SCHEDULE_TABLE_ID_ILL);
			TEST_ASSERT(rv==E_OS_ID);
			rv = NextScheduleTable(SCHEDULE_TABLE_ID_ILL, SCHEDULE_TABLE_0);
			TEST_ASSERT(rv==E_OS_ID);
			rv = NextScheduleTable(SCHEDULE_TABLE_0, SCHEDULE_TABLE_ID_ILL);
			TEST_ASSERT(rv==E_OS_ID);

			rv = NextScheduleTable(SCHEDULE_TABLE_0,SCHEDULE_TABLE_0);
			TEST_ASSERT(rv==E_OS_NOFUNC);

			rv = StartScheduleTableRel(SCHEDULE_TABLE_0,1);
			rv = StartScheduleTableRel(SCHEDULE_TABLE_1,1);
			rv = NextScheduleTable(SCHEDULE_TABLE_0,SCHEDULE_TABLE_1);
			TEST_ASSERT(rv==E_OS_STATE);

			/* Cleanup */
			rv = StopScheduleTable(SCHEDULE_TABLE_0);
			TEST_ASSERT(rv==E_OK);
			rv = StopScheduleTable(SCHEDULE_TABLE_1);
			TEST_ASSERT(rv==E_OK);
			test_nr = 100;
			break;
		case 100:
			TerminateTask();
			break;
		}
	}
}

void btask_sup_l_04(void) {

}


DECLARE_TEST_ETASK(04, etask_sup_l_04, NULL, NULL );
DECLARE_TEST_BTASK(04, btask_sup_l_04, NULL, NULL );


