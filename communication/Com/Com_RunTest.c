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








#include "Com_RunTest.h"
#include "Com.h"
#include "debug.h"
#include <stdlib.h>

//#define DEBUG_LVL DEBUG_MEDIUM

static uint8 RTE_ReceivedData[8] = {9,9,9,9,9,9,9,9};

static void PrintReceivedData(int nBytes) {
	DEBUG(DEBUG_HIGH, "Received data: ");
	for (int i = 0; i < nBytes; i++) {
		DEBUG(DEBUG_HIGH, "%d ", RTE_ReceivedData[i]);
	}
	DEBUG(DEBUG_HIGH,"\n");
}

void RTE_Notification(void) {
	DEBUG(DEBUG_HIGH, "RTE received notification.");

	Com_ReceiveSignal(11, (void *)RTE_ReceivedData);

	PrintReceivedData(8);

	DEBUG(DEBUG_HIGH, "Received data (signal %d): %d\n", 11, (sint8)RTE_ReceivedData[0]);
}

uint8 SIL2value;
uint8 SIL2quality;
void RTE_SIL2MESSAGE() {
	Com_ReceiveSignalGroup(2);
	Com_ReceiveShadowSignal(0, &SIL2value);
	Com_ReceiveShadowSignal(1, &SIL2quality);

	DEBUG(DEBUG_HIGH, "SIL2 message received! value: %d, quality: %d\n", SIL2value, SIL2quality);
}

static uint16 newSpeed;
static uint16 setNewSpeed = 0;
void RTE_EngineChangeSpeed() {
	setNewSpeed = 1;
}

void RTE_EngineMain() {
	if (setNewSpeed) {
		Com_ReceiveSignal(1, (void *)&newSpeed);

	LDEBUG_PRINTF("Setting engine speed to %d rpm\n", newSpeed);
		/*
		static uint16 sig;
		sig = rand() % 10000;
		*/
		Com_SendSignal(0, (void *)&newSpeed);
		setNewSpeed = 0;
	}
}
