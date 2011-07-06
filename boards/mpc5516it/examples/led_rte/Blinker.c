/*
 * Blinker.c
 *
 *  Created on: 6 jul 2011
 *      Author: tojo
 */

#import "Rte_Blinker.h"


BooleanType Value = FALSE;

void BlinkerRunnable() {

	Value = !Value;

	Rte_Call_Blinker_LED_Port_Write(Value);

}
