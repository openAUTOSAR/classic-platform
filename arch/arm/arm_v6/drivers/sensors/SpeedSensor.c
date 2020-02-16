/*
 * SpeedSensor.c
 *
 *  Created on:  Sep 4, 2014
 *      Author:  Zhang Shuzhou
 *  Reviewed on:
 *     Reviewer:
 *
 * This class implements a reflective sensor
 * (Photologic® Reflective Object Sensor OPB715Z, OPB716Z, OPB717Z, OPB718Z)
 * that is used to calculate wheel rotational speed.
 *
 */
#include <stdio.h>

#include "bcm2835.h"

#include "isr.h"
#include "irq_types.h"

#include "Sensors.h"

static uint32 pulse[2];

/**
 * Interrupt service routine that counts the number of detected pulses
 *
 * Each time a reflection signal is captured by the sensor, this interrupt
 * is activated and a counter (unique for each wheel pair) is increased.
 */
void SpeedSensor_Isr(void) {
	if (bcm2835_ReadGpioPin(&GPEDS0, GPIO_FRONT_SPEED)) {
		pulse[FRONT_WHEEL]++;
		bcm2835_ClearEventDetectPin(GPIO_FRONT_SPEED);
	}

	if (bcm2835_ReadGpioPin(&GPEDS0, GPIO_REAR_SPEED)) {
		pulse[REAR_WHEEL]++;
		bcm2835_ClearEventDetectPin(GPIO_REAR_SPEED);
	}
}

/**
 * Check if a wheel index is valid
 *
 * @param wheel		   		----- wheel type (typically rear or front)
 * @return 0/1				----- 1 if this is a valid wheel type
 * 								  0 otherwise
 */
static uint8 SpeedSensor_IsValidWheel(enum Wheel wheel) {
	if (wheel >= NO_WHEEL || wheel < 0) {
		printf("ERROR: There is no such wheel (%d). Was it a coding miss?\r\n", wheel);
		return 0;
	}

	return 1;
}

/**
 * Speed sensor initialization
 *
 * Basically, initialize GPIO pins connected to the speed sensor
 * to detect rising edges. Also, install an interrupt to count the
 * rising edges.
 */
void SpeedSensor_Init(void){

	/* Configure the pins connected to the wheel speed sensor as input pins */
	bcm2835_GpioFnSel(GPIO_FRONT_SPEED, GPFN_IN);
	bcm2835_GpioFnSel(GPIO_REAR_SPEED, GPFN_IN);

    /* Enable the pins to detect falling edge signals */
	bcm2835_SetReadWriteGpioReg(&GPFEN0, GPIO_FRONT_SPEED);
	bcm2835_SetReadWriteGpioReg(&GPFEN0, GPIO_REAR_SPEED);

	/* Reset edge statuses (sometimes they have been seen to be non-zero at start-up) */
	bcm2835_ClearEventDetectPin(GPIO_FRONT_SPEED);
	bcm2835_ClearEventDetectPin(GPIO_REAR_SPEED);

	/* Install an interrupt to handle falling edge signals
	 * Format: ISR_INSTALL(_name, _entryFunction, _irqVector, _priority, _appOwner)
	 * (for more details on irqVector, see the interrupts table on p.113 in BCM2835-ARM-Peripherals.pdf) */
    ISR_INSTALL_ISR2("GPIO0", bcm2835_GpioIsr, BCM2835_IRQ_ID_GPIO_0, 2, 0);

	/* Enable the installed interrupt by setting the appropriate HW register */
	bcm2835_SetReadWriteGpioReg(&IRQ_ENABLE1, BCM2835_IRQ_ID_GPIO_0);
}

/**
 * Get the number of detected pulses for a wheel pair, since the last counter reset
 *
 * @param wheel		    	----- wheel type (rear or front)
 * @return pulse			----- pulse counter
 */
uint32 Sensors_GetWheelPulse(enum Wheel wheel) {
	if (!SpeedSensor_IsValidWheel(wheel)) {
		return -1;
	}

	return pulse[wheel];
}

/**
 * Reset pulse counter for a wheel pair
 *
 * @param wheel		    	----- wheel type (rear or front)
 */
void Sensors_ResetWheelPulse(uint8 wheel) {
	if (SpeedSensor_IsValidWheel(wheel)) {
		pulse[wheel] = 0;
	}
}
