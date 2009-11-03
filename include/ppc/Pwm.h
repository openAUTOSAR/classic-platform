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
 * Pwm.h
 *
 *  Created on: 2009-jul-09
 *      Author: nian
 */

#ifndef PWM_H_
#define PWM_H_

// Channel configuration macro.
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

/*
 * PWM094: Std_VersionInfoType shall be imported from Std_Types.h
 */
#include "Std_Types.h"
#include "Pwm_Cfg.h"

/*
 * PWM003: The detection of development errors is configurable (ON/OFF) at pre-
 * compile time. The switch PwmDevErorDetect shall activate or deactivate the
 * detection of all development errors.
 *
 * PWM064: If the PwmDevErorDetect switch is enabled, API parameter checking is
 * enabled.
 *
 * PWM078: Detected development errors shall be reported to the Det_ReportError
 * service of the Development Error Tracer (DET) if the pre-processor
 * PwmDevErorDetect is set.
 */
#if PWM_DEV_EROR_DETECT==ON
#	define Pwm_ReportError(ErrorId) Det_ReportError( MODULE_ID_PWM, 0, 0, ErrorId);
#else
#   define Pwm_ReportError(ErrorId)
#endif

/**************************************************************
 *  Type definitions
 **************************************************************/
// PWM002: Development error values are of type uint8
typedef uint8 Pwm_ErrorType;

/*
 * PWM058: The width of the duty cycle parameter is 16 bits
 *
 * PWM059: The PWM module shall comply with the following scaling scheme
 * for the duty cycle:
 * 0x0    =   0%,
 * 0x8000 = 100% */
typedef uint16 Pwm_DutyCycleType;

#define Pwm_100_Procent 0x8000
#define Pwm_0_Procent 0

/*
 * PWM106: This is implementation specific but not all values may be valid
 * within the type. This shall be chosen in order to have the most efficient
 * implementation on a specific microcontroller platform.
 *
 * PWM106 => Pwm_ChannelType == eemios channel id.
 */
typedef uint8 Pwm_ChannelType;

/*
 * PWM070: All time units used within the API services of the PWM module shall
 * be of the unit ticks.
 */
typedef uint16 Pwm_PeriodType;

typedef enum {
	PWM_LOW, PWM_HIGH
} Pwm_OutputStateType;

typedef enum {
	PWM_FALLING_EDGE=PWM_LOW, PWM_RISING_EDGE = PWM_HIGH, PWM_BOTH_EDGES
} Pwm_EdgeNotificationType;
#define PWM_NO_EDGES (PWM_BOTH_EDGES + 1)

typedef enum {
	PWM_VARIABLE_PERIOD, PWM_FIXED_PERIOD, PWM_FIXED_PERIOD_SHIFTED
} Pwm_ChannelClassType;

typedef void (*Pwm_NotificationHandlerType)();

//#define Pwm_ChannelConfiguration(DefaultPeriod)

typedef enum {
	PWM_CHANNEL_PRESCALER_1=0,
	PWM_CHANNEL_PRESCALER_2,
	PWM_CHANNEL_PRESCALER_3,
	PWM_CHANNEL_PRESCALER_4,
} Pwm_ChannelPrescalerType;

/*
 * Since the AUTOSAR PWM specification uses a different unit for the duty,
 * the following macro can be used to convert between that format and the
 * mpc5516 format.
 */
#define DUTY_AND_PERIOD(_duty,_period) .duty = (_duty*_period)>>15, .period = _period

#if defined(CFG_MPC5516)
	/* Mode is buffered PWM output (OPWM) */
	/* Mode is buffered Output PW and frequency modulation mode */
#define PWM_EMIOS_OPWM 0x5A
#elif defined(CFG_MPC5567)
	/* Mode is buffered OPWM with frequency modulation (allows change of
	 * period) */
#define PWM_EMIOS_OPWM 0x19
#endif


typedef struct {
	/* Number of duty ticks */
	uint32_t                 duty:32;
	/* Length of period, in ticks */
	uint32_t                 period:32;
	/* Counter */
	uint32_t                 counter:32;
	/* Enable freezing the channel when in debug mode */
	uint32_t                 freezeEnable:1;
	/* Disable output */
	uint32_t                 outputDisable:1;
	/* Select which bus disables the bus
	 * TODO: Figure out how this works, i.e. what bus does it refer to? */
	uint32_t 				 outputDisableSelect:2;
	/* Prescale the emios clock some more? */
	Pwm_ChannelPrescalerType prescaler:2;
	/* Prescale the emios clock some more? */
	uint32_t				 usePrescaler:1;
	/* Whether to use DMA. Currently unsupported */
	uint32_t				 useDma:1;
	uint32_t				 reserved_2:1;
	/* Input filter. Ignored in output mode. */
	uint32_t				 inputFilter:4;
	/* Input filter clock source. Ignored in output mode */
	uint32_t				 filterClockSelect:1;
	/* Enable interrupts/flags on this channel? Required for DMA as well. */
	uint32_t				 flagEnable:1;
	uint32_t				 reserved_3:3;
	/* Trigger a match on channel A */
	uint32_t				 forceMatchA:1;
	/* Triggers a match on channel B */
	uint32_t				 forceMatchB:1;
	uint32_t				 reserved_4:1;
	/* We can use different buses for the counter. Use the internal counter */
	uint32_t				 busSelect:2;
	/* What edges to flag on? */
	uint32_t				 edgeSelect:1;
	/* Polarity of the channel */
	uint32_t				 edgePolarity:1;
	/* EMIOS mode. 0x58 for buffered output PWM */
	uint32_t				 mode:7;
} Pwm_ChannelRegisterType;

typedef struct {
	Pwm_ChannelRegisterType r;
	Pwm_ChannelType channel;
} Pwm_ChannelConfigurationType;

typedef struct {
	Pwm_ChannelConfigurationType Channels[PWM_NUMBER_OF_CHANNELS];
#if PWM_NOTIFICATION_SUPPORTED==ON
	Pwm_NotificationHandlerType NotificationHandlers[PWM_NUMBER_OF_CHANNELS];
#endif
} Pwm_ConfigType;

extern const Pwm_ConfigType PwmConfig;

/*
 * With MPC551x, only the first 16 eMIOS channels are of the usable class for
 * hardware PWM.
 */
#if PWM_NUMBER_OF_CHANNELS > 16
#error "PWM_NUMBER_OF_CHANNELS is larger than supported by the target platform."
#endif

#if PWM_DUTYCYCLE_UPDATED_ENDPERIOD!=ON
#error "Unbuffered PWM currently not supported by this driver."
#endif

/*
 * Constants
 *****************/

/* Pwm_Init called with the wrong parameter */
//const Pwm_ErrorType PWM_E_PARAM_CONFIG = 0x10;
#define PWM_E_PARAM_CONFIG 0x10

/* PWM is not initialized yet */
//const Pwm_ErrorType PWM_E_UNINIT = 0x11;
#define PWM_E_UNINIT 0x11

/* Invalid PWM channel identifier */
//const Pwm_ErrorType PWM_E_PARAM_CHANNEL = 0x12;
#define PWM_E_PARAM_CHANNEL 0x12

/* Use of unauthorized service on PWM channel configured fixed period */
//const Pwm_ErrorType PWM_E_PERIOD_UNCHANGEABLE = 0x13;
#define PWM_E_PERIOD_UNCHANGEABLE 0x13

/* Pwm_Init called when already initialized */
//const Pwm_ErrorType PWM_E_ALREADY_INITIALIZED = 0x14;
#define PWM_E_ALREADY_INITIALIZED 0x14

/*
 * Implemented functions
 ****************************/
void Pwm_Init(const Pwm_ConfigType* ConfigPtr);
void Pwm_DeInit();
void Pwm_GetVersionInfo(Std_VersionInfoType* VersionInfo);
#if PWM_SET_PERIOD_AND_DUTY==ON
void Pwm_SetPeriodAndDuty(Pwm_ChannelType Channel, Pwm_PeriodType Period,
		Pwm_DutyCycleType DutyCycle);
#endif

void Pwm_SetDutyCycle(Pwm_ChannelType Channel, Pwm_DutyCycleType DutyCycle);
void Pwm_SetOutputToIdle(Pwm_ChannelType ChannelNumber);
/*
 * PWM085: The function Pwm_GetOutputState shall be pre compile configurable
 * ON/OFF by the configuration parameter PwmGetOutputState
 */
#if PWM_GET_OUTPUT_STATE==ON
Pwm_OutputStateType Pwm_GetOutputState(Pwm_ChannelType Channel);
#endif

/*
 * PWM113: The function EnableNotification shall be pre compile configurable
 * ON/OFF by the configuration parameter PwmNotificationSupported
 *
 * PWM112: The function DisableNotification shall be pre compile configurable
 * ON/OFF by the configuration parameter PwmNotificationSupported
 */
#if PWM_NOTIFICATION_SUPPORTED==ON
void Pwm_DisableNotification(Pwm_ChannelType Channel);
void Pwm_EnableNotification(Pwm_ChannelType ChannelNumber,
		Pwm_EdgeNotificationType Notification);
#endif

#endif /* PWM_H_ */
