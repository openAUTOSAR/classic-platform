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
 * OSEK:
 * - GetResource     			E_OS_ID, E_OS_ACCESS
 * - ReleaseResource 			E_OS_ID, E_OS_ACCESS, E_OS_NOFUNC
 *
 * More tests here:
 * - GetResource(RES_SCHEDULER)
 * - Check the ceiling protocol
 * - Linked resources
 * - Nested allocation of the same resource is forbidden.
 *
 *
 * Limitations:
 * - Internal resources.
 */

#include "Os.h"
#include "os_test.h"
#include "arc.h"


void etask_sup_l_02(void) {
	_Bool done = 0;
	StatusType rv;

	while (!done) {
		TEST_RUN();
		switch (test_nr) {
		case 1:
			rv = GetResource(RES_ID_ext_prio_3);
			TEST_ASSERT(rv == E_OK);
			rv = ReleaseResource(RES_ID_ext_prio_3);
			TEST_ASSERT(rv == E_OK);
			test_nr++;
			break;
		case 2:
			/* Get the same resource twice */
			rv = GetResource(RES_ID_ext_prio_3);
			TEST_ASSERT(rv == E_OK);
			rv = GetResource(RES_ID_ext_prio_3);
			TEST_ASSERT(rv == E_OS_ACCESS);
			test_nr = 100;
			break;
		case 100:
			TerminateTask();
			break;
		}
	}
}

void btask_sup_l_02(void) {
}


DECLARE_TEST_ETASK(02, etask_sup_l_02, NULL, NULL );
DECLARE_TEST_BTASK(02, btask_sup_l_02, NULL, NULL );
