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
 *  Created on: 2009-okt-02
 *      Author: jonte
 */


#include "Pwm.h"
#include "Pwm_Cfg.h"

extern void MyPwmNotificationRoutine(void);

// TODO remove, just put here for lazyness
void MyPwmNotificationRoutine(void){
}
/* -----------------------------------------------------------------------
   TIM4CLK = 72 MHz, Prescaler = 7199, TIM4 counter clock = 72 MHz
   TIM4 ARR Register = 10000 => TIM4 Frequency = TIM4 counter clock/(ARR*(PSC + 1)
   TIM4 Frequency = 1 Hz.
   TIM4 Channel1 duty cycle = (TIM4_CCR1/ TIM4_ARR)* 100 = 12.5%


   NOTE!!! All channels on one TIM uses the same Time base. The last configured will
   set the Time base.
   ----------------------------------------------------------------------- */
const Pwm_ConfigType PwmConfig = {
	.Channels = {
	        PWM_CHANNEL_CONFIG(PWM_CHANNEL_1, 10000, 0x4000/*50%*/, 29, PWM_HIGH),
	        PWM_CHANNEL_CONFIG(PWM_CHANNEL_2, 10000, 0x4000/*50%*/, 29, PWM_LOW)
	},
#if PWM_NOTIFICATION_SUPPORTED==ON
	.NotificationHandlers = {
			MyPwmNotificationRoutine, // PWM_CHANNEL_1
			NULL                      // PWM_CHANNEL_2
	}
#endif
};


