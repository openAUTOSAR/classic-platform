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
 *  Created on: 2009-jul-09
 *      Author: nian
 */

#warning "This default file may only be used as an example!"

#include "Pwm.h"
#include "Pwm_Cfg.h"

extern void MyPwmNotificationRoutine(void);

const Pwm_ConfigType PwmConfig = {
	.Channels = {
	        PWM_CHANNEL_CONFIG(PWM_CHANNEL_1, 3000, 0x6000, PWM_CHANNEL_PRESCALER_4, PWM_HIGH),
	        PWM_CHANNEL_CONFIG(PWM_CHANNEL_2, 2000, 0x2000, PWM_CHANNEL_PRESCALER_2, PWM_LOW)
	},
#if PWM_NOTIFICATION_SUPPORTED==STD_ON
	.NotificationHandlers = {
			MyPwmNotificationRoutine, // PWM_CHANNEL_1
			NULL                      // PWM_CHANNEL_2
	}
#endif
};
