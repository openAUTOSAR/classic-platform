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
#include "os_config_macros.h"

#include "EcuM.h"
#include <stdio.h>
#include <assert.h>
#include "pwm_sine_main.h"
#include "debug.h"
//#include "WdgM.h"


void OsIdle( void ) {
	for(;;);
}

/*
 * This basic task should execute every 25 OS ticks
 */
void bTask25( void ) {

	// Update PWM_CHANNEL_1 every 25th OS tick
	pwm_sine_main(PWM_CHANNEL_1);

	TerminateTask();
}

/*
 * This basic task should execute every 100 OS ticks
 */

void bTask100( void ) {

	// Update PWM_CHANNEL_2 every 100th OS tick
	pwm_sine_main(PWM_CHANNEL_2);

	TerminateTask();
}

/*
 * This is the startup task. It is activated once immediately after the OS i started.
 */
void Startup( void ) {

	// Call second phase of startup sequence.
	EcuM_StartupTwo();

	pwm_enable_notifications();

	/*
	 * Activate scheduled tasks. OS tick is 1ms.
	 */
	SetRelAlarm(ALARM_ID_Alarm25, 25, 25);    // Task for pwm channel 1
	SetRelAlarm(ALARM_ID_Alarm100, 100, 100); // Task for pwm channel 2


	// End of startup_task().
	TerminateTask();
}


