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

#include "Std_Types.h"
#include "Pwm.h"
#include "RTE_pwm_sine.h"


static uint8 index[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#define SINE_TABLE_SIZE 39
const Pwm_PeriodType sine_table[SINE_TABLE_SIZE] = {0x4a03, 0x53c6, 0x5d0e, 0x659e, 0x6d41, 0x73c6,
		0x7906, 0x7cde, 0x7f36, 0x7fff, 0x7f36, 0x7cde, 0x7906, 0x73c6,
		0x6d41, 0x659e, 0x5d0e, 0x53c6, 0x4a03, 0x4000, 0x35fc, 0x2c39,
		0x22f1, 0x1a61, 0x12be, 0xc39, 0x6f9, 0x321, 0xc9, 0x0, 0xc9, 0x321,
		0x6f9, 0xc39, 0x12be, 0x1a61, 0x22f1, 0x2c39, 0x35fc};

/*
 * Next sine sample
 */
int pwm_sine_main(Pwm_ChannelType channel) {
	uint8 cnt = index[channel];
	if (cnt >= SINE_TABLE_SIZE) {
		cnt = 0;
	} else {
		cnt++;
	}

	RTE_pwm_sine_update(channel, sine_table[cnt]);

	index[channel] = cnt;

	return 1;
}


/*
 * Initialize notifications
 */
int pwm_enable_notifications() {
	RTE_pwm_enable_notifications(PWM_CHANNEL_1, PWM_RISING_EDGE);
	return 1;
}
