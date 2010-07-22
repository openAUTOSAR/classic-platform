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









#include <stdlib.h>

#include "RTE_blinker.h"

static uint8 blinkerStatus = STD_LOW;

/*
 * Updates the blinker.
 */
int blinker_component_main(void) {
	if (blinkerStatus == STD_LOW) {
		blinkerStatus = STD_HIGH;
	}
	else {
		blinkerStatus = STD_LOW;
	}
	RTE_blinker_blink(blinkerStatus);
	return 1;
}
