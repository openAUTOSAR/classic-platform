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


#include "Os.h"

#include "EcuM.h"
#include "blinker_main.h"

//#define USE_LDEBUG_PRINTF // Uncomment this to turn debug statements on.
#include "debug.h"


void OsIdle( void ) {
	for(;;);
}

/*
 * This basic task should execute every 10 OS ticks
 */

void bTask10( void ) {

	TerminateTask();
}

/*
 * This basic task should execute every 25 OS ticks
 */
void bTask25( void ) {

	blinker_component_main();

	TerminateTask();
}

/*
 * This basic task should execute every 100 OS ticks
 */

void bTask100( void ) {

	TerminateTask();
}
/*
 * This is the startup task. It is activated once immediately after the OS i started.
 */
void Startup( void ) {

	// Call second phase of startup sequence.
	EcuM_StartupTwo();

	/*
	 * Activate scheduled tasks. OS tick is 1ms.
	 * The Blink is run every 25 ms with an offset of 25ms.
	 */
	SetRelAlarm(ALARM_ID_alarm10, 10, 10); // ADC data acquisition
	SetRelAlarm(ALARM_ID_alarm25, 25, 25); // ADC data acquisition
	SetRelAlarm(ALARM_ID_alarm100, 100, 100); // ADC data acquisition

	// End of startup_task().
	TerminateTask();
}


