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

#include "Com.h"
#include "Os.h"
#include "Dio.h"
#include "EcuM.h"
#include "CanIf.h"

/*
 * Functions that must be supplied by the example
 */
void OsIdle( void ) {
	for(;;);
}

void StartupTask( void ) {

	// Call second phase of startup sequence.
	EcuM_StartupTwo();

	// Startup CanIf due to ComM is missing in this example
	CanIf_InitController(CANIF_Channel_1, CANIF_Channel_1_CONFIG_0);
	CanIf_SetControllerMode(CANIF_Channel_1, CANIF_CS_STARTED);

	// Make sure that the right PDU-groups are ready for communication.
	Com_IpduGroupStart(ComPduGroup, 0);

	// End of startup_task().
	TerminateTask();

}

void MainFunctionTask( void ) {
	Com_MainFunctionRx();
	Com_MainFunctionTx();
	TerminateTask();
}



// Task that toggles the LED
Dio_LevelType level = 1;
void BlinkerTask( void ) {
	level = !level;
	Dio_WriteChannel(DIO_CHANNEL_NAME_LED_CHANNEL, level);
	TerminateTask();
}
