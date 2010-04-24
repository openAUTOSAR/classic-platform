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








/*
 * Pwm_Cfg.c
 *
 *      Author: jonte
 */

#include "Pwm.h"
#include "Pwm_Cfg.h"

const Pwm_ConfigType PwmConfig =
{
		.channels = {
			{
				.channel = PWM_CHANNEL_1,
				DUTY_AND_PERIOD(0x6000,100),
				.centerAlign = 0,
				.polarity = POLARITY_NORMAL,
				.scaledClockEnable = 0,
			},
			{
				.channel = PWM_CHANNEL_2,
				DUTY_AND_PERIOD(0x8000,255),
				.centerAlign = 1,
				.polarity = POLARITY_NORMAL,
				.scaledClockEnable = 1,
			},
		},
		.prescalerA = PRESCALER_128,
		.prescalerB = PRESCALER_8,
};
