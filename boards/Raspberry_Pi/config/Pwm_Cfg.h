/*
 * Pwm_Cfg.h
 *
 *  Created on: Sep 16, 2013
 *      Author: Zhang Shuzhou
 */

#ifndef PWM_CFG_H_
#define PWM_CFG_H_

//#include "bcm2835.h"
//#include "Dma.h"

#define PWM_DEV_ERROR_DETECT        STD_ON
#define PWM_STATICALLY_CONFIGURED   STD_OFF
#define PWM_NOTIFICATION_SUPPORTED  STD_OFF
#define PWM_FREEZE_ENABLE           STD_OFF

// Define what API functions to enable.
#define PWM_GET_OUTPUT_STATE_API    STD_OFF
#define PWM_SET_PERIOD_AND_DUTY_API STD_ON
#define PWM_DE_INIT_API             STD_OFF
#define PWM_SET_DUTY_CYCLE_API      STD_ON
#define PWM_SET_OUTPUT_TO_IDLE_API  STD_OFF
#define PWM_VERSION_INFO_API        STD_OFF

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
 ****************************************************************************/
typedef enum {
	PWM_CHANNEL0 = 0,
	PWM_CHANNEL1,
    PWM_NUMBER_OF_CHANNELS   //how many channels can we have? 2 for PWM and 15 for DMA
} Pwm_NamedChannelsType;

/* 1 =PWMchannel output is high at the beginning of the period, then goes low when the duty count
is reached. */
#define POLARITY_NORMAL   1
#define POLARITY_INVERSED 0

/**
 * Since the AUTOSAR PWM specification uses a different unit for the duty,
 * the following macro can be used to convert between that format and the
 * driver format.
 */
#define DUTY_AND_PERIOD(_duty,_period) .duty = (uint8)((uint32)((uint32)_duty*(uint32)_period)>>15), .period = _period

typedef struct {
	/** Number of duty ticks */
	uint32 duty;
	/** Length of period, in ticks */
	uint32 period;
	/** Center or left align */
	uint8 centerAlign;
	/** Polarity of the channel */
	uint8 polarity;
	/** Scaled clock enabled */
	uint8 scaledClockEnable;
	/** idle state */
	uint8 idleState;
	uint8 reserved;
	Pwm_ChannelClassType class;
	Pwm_ChannelType channel;
} Pwm_ChannelConfigurationType;

typedef struct {
	Pwm_ChannelConfigurationType channels[PWM_NUMBER_OF_CHANNELS];
} Pwm_ConfigType;

#endif /* PWM_CFG_H_ */
