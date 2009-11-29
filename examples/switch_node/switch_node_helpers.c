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









#include "Adc.h"
#include "Com.h"
#include "Trace.h"
#include "RTE_switch.h"

#define SWITCH_LOW_THRESHOLD 100

/*
 * ADC callback. This function is called each time the switches are read.
 */
static Adc_ValueGroupType switchValues[ADC_NBR_OF_SWITCH_CHANNELS];
void switch_node_switches_callback(void) {

	if (E_OK == Adc_ReadGroup(ADC_SWITCHES, switchValues)) {

		if (switchValues[ADC_SWITCH_RED] <= SWITCH_LOW_THRESHOLD) {
			switch_handle_switch1();
		} else if (switchValues[ADC_SWITCH_BLACK] <= SWITCH_LOW_THRESHOLD) {
			switch_handle_switch2();
		}

	} else {
		DEBUG(DEBUG_HIGH, "Switches failed!\n");
	}
}


/*
 * Read values from Adc channels connected to potentiometers.
 * Nothing is done with their values though.
 */
static Adc_ValueGroupType potentiometerValues[ADC_NBR_OF_POTENTIOMETER_CHANNELS];
void switch_node_potentiometers_callback(void){

	if (E_OK == Adc_ReadGroup(ADC_POTENTIOMETERS, potentiometerValues)) {
		DEBUG(DEBUG_MEDIUM, "Potentiometers read successfully! %d %d\n", potentiometerValues[ADC_POTENTIOMETER_0], potentiometerValues[ADC_POTENTIOMETER_1]);

	} else {
		DEBUG(DEBUG_HIGH, "Potentiometers failed!\n");
	}
}


