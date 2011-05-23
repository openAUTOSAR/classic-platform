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
#include <stdio.h>
#include <assert.h>
#include "Com.h"
#include "Adc.h"

//#define USE_LDEBUG_PRINTF // Uncomment this to turn debug statements on.
#include "debug.h"

void OsIdle( void ) {
	for(;;);
}


void ComTask( void ) {
	// Run COM scheduled functions.
	Com_MainFunctionTx();
	Com_MainFunctionRx();

	TerminateTask();
}

void ReadSwitches( void ) {
	// Trigger update of switch readings
	Adc_StartGroupConversion(ADC_SWITCHES); // Read the switches
	Adc_StartGroupConversion(ADC_POTENTIOMETERS);

	TerminateTask();
}

/*
 * This is the startup task. It is activated once immediately after the OS i started.
 */
void StartupTask( void ) {

	// Call second phase of startup sequence.
	EcuM_StartupTwo();

        // Startup CanIf due to ComM is missing in this example
        CanIf_InitController(CANIF_CHANNEL_0, CANIF_CHANNEL_0_CONFIG_0);
        CanIf_SetControllerMode(CANIF_CHANNEL_0, CANIF_CS_STARTED);

	// Make sure that the right PDU-groups are ready for communication.
	Com_IpduGroupStart(TxGroup, 0);

	// End of startup_task().
	TerminateTask();
}


