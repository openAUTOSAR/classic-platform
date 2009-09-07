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
 * Pwm_Cfg.h
 *
 *  Created on: 2009-jul-09
 *      Author: nian
 */

#ifndef PWM_CFG_H_
#define PWM_CFG_H_

/****************************************************************************
 * Global configuration options and defines
 */

#define ON 1
#define OFF 0

/*
 * PWM003: The detection of development errors is configurable (ON/OFF) at
 * pre-compile time. The switch PwmDevErorDetect shall activate or disable
 * the detection of all development errors
 */
#define PWM_DEV_EROR_DETECT ON
#define PWM_GET_OUTPUT_STATE ON
#define PWM_STATICALLY_CONFIGURED OFF
#define PWM_NOTIFICATION_SUPPORTED ON

#define PWM_SET_PERIOD_AND_DUTY ON

/*
 * PWM132: Switch for enabling the update of duty cycle parameter at the end
 * of the current period.
 *
 * Note: Currently only ON mode is supported.
 */
#define PWM_DUTYCYCLE_UPDATED_ENDPERIOD ON

/****************************************************************************
 * Not defined in AUTOSAR.
 */
#define PWM_ISR_PRIORITY 1
#define PWM_PRESCALER 64
/*
 * Setting to ON freezes the current output state of a PWM channel when in
 * debug mode.
 */
#define PWM_FREEZE_ENABLE ON

/****************************************************************************
 * Enumeration of channels
 * Maps a symbolic name to a hardware channel
 */
typedef enum {
#if defined(CFG_BRD_MPC5516IT)
    PWM_CHANNEL_1 = 13, /* Emios channel 13 and 12 map to the */
    PWM_CHANNEL_2 = 12, /* LEDs LD4 and LD5 of MPC5516IT      */

#elif defined(CFG_BRD_MPC5567QRTECH)
    PWM_CHANNEL_1 = 10, /* Emios channel 10 maps to PCR189 which
                         * is available on pin 54 of the
                         * ERNI 154822 connector
                         */
    PWM_CHANNEL_2 = 12, /* Channel 12 goes to PCR191, also
                         * available on the ERNI 154822 connector
                         */
#else
#warning "Unknown board or CFG_BRD_* undefined"
#endif
    PWM_NUMBER_OF_CHANNELS = 2
} Pwm_NamedChannelsType;

#endif /* PWM_CFG_H_ */
