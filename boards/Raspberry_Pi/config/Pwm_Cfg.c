/*
 * Pwm_Cfg.c
 *
 *  Created on: Sep 17, 2013
 *      Author: Zhang Shuzhou
 */

#include "Pwm.h"


const Pwm_ConfigType PwmConfig =
{
	.channels = {
					{
						.channel =  PWM_CHANNEL0,
						DUTY_AND_PERIOD(0x4000, 20000.0),   //duty and cycle
						.centerAlign =  STD_OFF,
						.polarity = POLARITY_NORMAL,
						.scaledClockEnable = STD_ON,
						.idleState = PWM_LOW,
						.class = PWM_VARIABLE_PERIOD
					},

	            	{
						.channel = PWM_CHANNEL1,
						 DUTY_AND_PERIOD(0x2000, 20000.0), //duty and cycle
                        .centerAlign = STD_OFF,
                        .polarity = POLARITY_NORMAL,
		                .scaledClockEnable = STD_ON,
		                .idleState = PWM_LOW,
		                .class = PWM_VARIABLE_PERIOD
					},
	           },
};



