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
 * Description
 *   A very simple example that uses a scheduletable with a duration of 10.
 *
 *   Activations
 *    2 - ActivateTask bTask3
 *    4 - SetEvent     eTask1
 *    8 - SetEvent     eTask2
 *    9 - ActivateTak bTask3 and SetEvent eTask1
 */


#include "Os.h"
#define USE_LDEBUG_PRINTF // Uncomment this to turn debug statements on.
#include "debug.h"

#define TABLE_DURATION     10

static int iteration = 0;

static unsigned getTick( void ) {
	TickType tick;
	(void)GetCounterValue(COUNTER_ID_Counter1,&tick);
	return tick;
}

/**
 * Task activated by Scheduletable/ActivateTask on 2/10 and 9/10
 */
void bTask3( void ) {

	LDEBUG_PRINTF("%d/%u btask3\n",iteration,getTick());
	TerminateTask();
}

/**
 * Task activated by Scheduletable/SetEvent on 4/10
 */

void eTask1( void ) {

	for(;;) {
		WaitEvent(EVENT_MASK_Event1);
		LDEBUG_PRINTF("%d/%u eTask1\n",iteration,getTick());
		ClearEvent(EVENT_MASK_Event1);
	}
}

/**
 * Task activated by Scheduletable/SetEvent on 8/10 and 9/10
 */
void eTask2( void ) {

	for(;;) {
		WaitEvent(EVENT_MASK_Event2);
		LDEBUG_PRINTF("%d/%u eTask2\n",iteration++,getTick());
		ClearEvent(EVENT_MASK_Event2);
	}
}

void OsIdle( void ) {
	while(1);
}
