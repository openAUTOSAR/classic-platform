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

/** @addtogroup Pwm PWM Driver
 *  @{ */

/** @file Pwm_Cfg.h
 *  Definitions of configuration parameters for PWM Driver.
 */

#ifndef PWM_CFG_H_
#define PWM_CFG_H_

#warning "This default file may only be used as an example!"

/*
 * PwmGeneral
 */

/*
 * PWM003: The detection of development errors is configurable (ON/OFF) at
 * pre-compile time. The switch PwmDevErorDetect shall activate or disable
 * the detection of all development errors
 */
/** Enable Development Error Trace */
#define PWM_DEV_ERROR_DETECT STD_ON
/** Build GetOutputState()
 * Note: Not supported since pwm module doesn't support reading output.
 */
#define PWM_GET_OUTPUT_STATE STD_OFF
/** Determines if module is configurable at runtime */
#define PWM_STATICALLY_CONFIGURED STD_OFF
/** Use notifications
 * Note: Only OFF mode available due to hw limitations.
 */
#define PWM_NOTIFICATION_SUPPORTED STD_OFF
/** Build period and duty API */
#define PWM_SET_PERIOD_AND_DUTY STD_ON

/**
 * Setting to ON freezes the current output state of a PWM channel when in
 * debug mode.
 */
#define PWM_FREEZE_ENABLE STD_ON

/**
 * PWM132: Switch for enabling the update of duty cycle parameter at the end
 * of the current period.
 *
 * Note: Currently only ON mode is supported.
 */
#define PWM_DUTYCYCLE_UPDATED_ENDPERIOD STD_ON

/**
 * PWM106: This is implementation specific but not all values may be valid
 * within the type. This shall be chosen in order to have the most efficient
 * implementation on a specific microcontroller platform.
 *
 * PWM106 => Pwm_ChannelType == channel id.
 */
typedef uint8 Pwm_ChannelType;

/**
 * PWM070: All time units used within the API services of the PWM module shall
 * be of the unit ticks.
 */
typedef uint16 Pwm_PeriodType;

/****************************************************************************
 * Enumeration of channels
 * Maps a symbolic name to a hardware channel
 */
typedef enum {
    PWM_CHANNEL_1 = 0,
    PWM_CHANNEL_2 = 7,
    PWM_NUMBER_OF_CHANNELS = 2
} Pwm_NamedChannelsType;

/* 1 =PWMchannel output is high at the beginning of the period, then goes low when the duty count
is reached. */
#define POLARITY_NORMAL 1
#define POLARITY_INVERSED 0

/**
 * Since the AUTOSAR PWM specification uses a different unit for the duty,
 * the following macro can be used to convert between that format and the
 * driver format.
 */
#define DUTY_AND_PERIOD(_duty,_period) .duty = (uint8)((uint32)((uint32)_duty*(uint32)_period)>>15), .period = _period


typedef struct {
	/** Number of duty ticks */
	uint8_t duty;
	/** Length of period, in ticks */
	uint8_t period;
	/** Center or left align */
	uint8_t centerAlign:1;
	/** Polarity of the channel */
	uint8_t polarity:1;
	/** Scaled clock enabled */
	uint8_t scaledClockEnable:1;
	/** idle state */
	uint8_t idleState:1;

	uint8_t reserved:4;

	Pwm_ChannelClassType class;

	Pwm_ChannelType channel;
} Pwm_ChannelConfigurationType;

typedef enum{
	PRESCALER_1 = 0,
	PRESCALER_2 = 1,
	PRESCALER_4 = 2,
	PRESCALER_8 = 3,
	PRESCALER_16 = 4,
	PRESCALER_32 = 5,
	PRESCALER_64 = 6,
	PRESCALER_128 = 7,
}Pwm_PrescalerType;

typedef struct {
	Pwm_ChannelConfigurationType channels[PWM_NUMBER_OF_CHANNELS];
	Pwm_PrescalerType busPrescalerA;
	Pwm_PrescalerType busPrescalerB;
	uint8_t prescalerA;
	uint8_t prescalerB;
} Pwm_ConfigType;

#endif
/** @} */
