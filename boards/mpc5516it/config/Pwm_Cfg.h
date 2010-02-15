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

/*
 * PwmGeneral
 */

/*
 * PWM003: The detection of development errors is configurable (ON/OFF) at
 * pre-compile time. The switch PwmDevErorDetect shall activate or disable
 * the detection of all development errors
 */
/** Enable Development Error Trace */
#define PWM_DEV_EROR_DETECT STD_ON
/** Build GetOutputState() */
#define PWM_GET_OUTPUT_STATE STD_ON
/** Determines if module is configurable at runtime */
#define PWM_STATICALLY_CONFIGURED STD_OFF
/** Use notifications */
#define PWM_NOTIFICATION_SUPPORTED STD_ON
/** Build period and duty API */
#define PWM_SET_PERIOD_AND_DUTY STD_ON

/**
 * PWM106: This is implementation specific but not all values may be valid
 * within the type. This shall be chosen in order to have the most efficient
 * implementation on a specific microcontroller platform.
 *
 * PWM106 => Pwm_ChannelType == eemios channel id.
 */
typedef uint8 Pwm_ChannelType;

/**
 * PWM070: All time units used within the API services of the PWM module shall
 * be of the unit ticks.
 */
typedef uint16 Pwm_PeriodType;


/**
 * PWM132: Switch for enabling the update of duty cycle parameter at the end
 * of the current period.
 *
 * Note: Currently only ON mode is supported.
 */
#define PWM_DUTYCYCLE_UPDATED_ENDPERIOD STD_ON

/****************************************************************************
 * Not defined in AUTOSAR.
 */
#define PWM_ISR_PRIORITY 1
#define PWM_PRESCALER 64
/**
 * Setting to ON freezes the current output state of a PWM channel when in
 * debug mode.
 */
#define PWM_FREEZE_ENABLE STD_ON

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

typedef enum {
	PWM_CHANNEL_PRESCALER_1=0,
	PWM_CHANNEL_PRESCALER_2,
	PWM_CHANNEL_PRESCALER_3,
	PWM_CHANNEL_PRESCALER_4,
} Pwm_ChannelPrescalerType;

/**
 * Since the AUTOSAR PWM specification uses a different unit for the duty,
 * the following macro can be used to convert between that format and the
 * mpc5516 format.
 */
#define DUTY_AND_PERIOD(_duty,_period) .duty = (_duty*_period)>>15, .period = _period

#if defined(CFG_MPC5516)
	/** Mode is buffered PWM output (OPWM)
	 *  Mode is buffered Output PW and frequency modulation mode */
#define PWM_EMIOS_OPWM 0x5A
#elif defined(CFG_MPC5567)
	/** Mode is buffered OPWM with frequency modulation (allows change of
	 *  period) */
#define PWM_EMIOS_OPWM 0x19
#endif


typedef struct {
	/** Number of duty ticks */
	uint32_t                 duty:32;
	/** Length of period, in ticks */
	uint32_t                 period:32;
	/** Counter */
	uint32_t                 counter:32;
	/** Enable freezing the channel when in debug mode */
	uint32_t                 freezeEnable:1;
	/** Disable output */
	uint32_t                 outputDisable:1;
	/** Select which bus disables the bus
	 *  TODO: Figure out how this works, i.e. what bus does it refer to? */
	uint32_t 				 outputDisableSelect:2;
	/** Prescale the emios clock some more? */
	Pwm_ChannelPrescalerType prescaler:2;
	/** Prescale the emios clock some more? */
	uint32_t				 usePrescaler:1;
	/** Whether to use DMA. Currently unsupported */
	uint32_t				 useDma:1;
	uint32_t				 reserved_2:1;
	/** Input filter. Ignored in output mode. */
	uint32_t				 inputFilter:4;
	/** Input filter clock source. Ignored in output mode */
	uint32_t				 filterClockSelect:1;
	/** Enable interrupts/flags on this channel? Required for DMA as well. */
	uint32_t				 flagEnable:1;
	uint32_t				 reserved_3:3;
	/** Trigger a match on channel A */
	uint32_t				 forceMatchA:1;
	/** Triggers a match on channel B */
	uint32_t				 forceMatchB:1;
	uint32_t				 reserved_4:1;
	/** We can use different buses for the counter. Use the internal counter */
	uint32_t				 busSelect:2;
	/** What edges to flag on? */
	uint32_t				 edgeSelect:1;
	/** Polarity of the channel */
	uint32_t				 edgePolarity:1;
	/** EMIOS mode. 0x58 for buffered output PWM */
	uint32_t				 mode:7;
} Pwm_ChannelRegisterType;

typedef struct {
	Pwm_ChannelRegisterType r;
	Pwm_ChannelType channel;
} Pwm_ChannelConfigurationType;


typedef struct {
	Pwm_ChannelConfigurationType Channels[PWM_NUMBER_OF_CHANNELS];
#if PWM_NOTIFICATION_SUPPORTED==STD_ON
	Pwm_NotificationHandlerType NotificationHandlers[PWM_NUMBER_OF_CHANNELS];
#endif
} Pwm_ConfigType;

/** Channel configuration macro. */
#define PWM_CHANNEL_CONFIG(_hwchannel, _period, _duty, _prescaler, _polarity) \
    {\
        .channel = _hwchannel,\
        .r = {\
            DUTY_AND_PERIOD(_duty, _period),\
            .freezeEnable = 1,\
            .outputDisable = 0,\
            .usePrescaler = 1,\
            .prescaler = _prescaler,\
            .useDma = 0,\
            .flagEnable = 0, /* See PWM052 */ \
            .busSelect = 3, /* Use the internal counter bus */\
            .edgePolarity = _polarity,\
            .mode = PWM_EMIOS_OPWM\
        }\
    }

#endif /* PWM_CFG_H_ */
/** @} */
