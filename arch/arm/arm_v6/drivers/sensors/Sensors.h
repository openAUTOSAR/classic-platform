/*
 * Sensors.h
 *
 *  Created on: Nov 19, 2014
 *      Author: avenir
 */

#ifndef SENSORS_H_
#define SENSORS_H_

/* Channel on the */
#define MCP3008_BATTERY_CH 		1

/* Debug print-out flag */
#define WHEELSENS_PRINT_DEBUG  	0

/* Our MOPED-wheels are approximately 10 cm in diameter */
#define WHEEL_DIAMETER 			10
/* Nr of pulses in each wheel turn (360 degrees) */
#define PULSES_PER_WHEEL_TURN	5

/* Valid wheel types */
enum Wheel {REAR_WHEEL, FRONT_WHEEL, NO_WHEEL};

void SpeedSensor_Isr(void);
/** Speed sensor initialization */
void SpeedSensor_Init(void);
/** Get the number of (recently) detected pulses for a wheel pair */
uint32 Sensors_GetWheelPulse(enum Wheel wheel);
/** Reset pulse counter for a wheel pair */
void Sensors_ResetWheelPulse(uint8 wheel);

/** Read (raw) voltage value from the ADC sensor (MCP3008) */
void Mcp3008_Read(uint8 channel, uint32 *data);

/** Initialize the GPIO pins connected to the ultrasonic sensor */
void UltrasonicSensor_Init(void);
/** Measure the distance to an object in front of the ultrasonic sensor */
uint32 UltrasonicSensor_Read(void);

#endif /* SENSORS_H_ */
