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

/** @file Pwm.h
 * API and type definitions for PWM Driver.
 */

#ifndef PWM_H_
#define PWM_H_

#define PWM_SW_MAJOR_VERSION	1
#define PWM_SW_MINOR_VERSION	0
#define PWM_SW_PATCH_VERSION	0

/*
 * PWM094: Std_VersionInfoType shall be imported from Std_Types.h
 */
#include "Std_Types.h"

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
#if PWM_DEV_EROR_DETECT==STD_ON
#	define Pwm_ReportError(ErrorId) Det_ReportError( MODULE_ID_PWM, 0, 0, ErrorId);
#else
#   define Pwm_ReportError(ErrorId)
#endif

/**************************************************************
 *  Type definitions
 **************************************************************/
/** PWM002: Development error values are of type uint8 */
typedef uint8 Pwm_ErrorType;

/**
 * PWM058: The width of the duty cycle parameter is 16 bits
 *
 * PWM059: The PWM module shall comply with the following scaling scheme
 * for the duty cycle:
 * 0x0    =   0%,
 * 0x8000 = 100% */
typedef uint16 Pwm_DutyCycleType;

#define Pwm_100_Procent 	0x8000
#define Pwm_0_Procent		0


typedef enum {
	PWM_LOW,
	PWM_HIGH
} Pwm_OutputStateType;

typedef enum {
	PWM_FALLING_EDGE = PWM_LOW,
	PWM_RISING_EDGE =  PWM_HIGH,
	PWM_BOTH_EDGES
} Pwm_EdgeNotificationType;

#define PWM_NO_EDGES (PWM_BOTH_EDGES + 1)

typedef enum {
	PWM_VARIABLE_PERIOD,
	PWM_FIXED_PERIOD,
	PWM_FIXED_PERIOD_SHIFTED
} Pwm_ChannelClassType;

typedef void (*Pwm_NotificationHandlerType)();

#include "Pwm_Cfg.h"

extern const Pwm_ConfigType PwmConfig;

/*
 * With MPC551x, only the first 16 eMIOS channels are of the usable class for
 * hardware PWM.
 */
#if PWM_NUMBER_OF_CHANNELS > 16
#error "PWM_NUMBER_OF_CHANNELS is larger than supported by the target platform."
#endif

#if PWM_DUTYCYCLE_UPDATED_ENDPERIOD!=STD_ON
#error "Unbuffered PWM currently not supported by this driver."
#endif

/*
 * Constants
 *****************/
/** @name Error Codes */
//@{
/** Pwm_Init called with the wrong parameter */
//const Pwm_ErrorType PWM_E_PARAM_CONFIG = 0x10;
#define PWM_E_PARAM_CONFIG 			0x10

/** PWM is not initialized yet */
//const Pwm_ErrorType PWM_E_UNINIT = 0x11;
#define PWM_E_UNINIT 					0x11

/** Invalid PWM channel identifier */
//const Pwm_ErrorType PWM_E_PARAM_CHANNEL = 0x12;
#define PWM_E_PARAM_CHANNEL 			0x12

/** Use of unauthorized service on PWM channel configured fixed period */
//const Pwm_ErrorType PWM_E_PERIOD_UNCHANGEABLE = 0x13;
#define PWM_E_PERIOD_UNCHANGEABLE 		0x13

/** Pwm_Init called when already initialized */
//const Pwm_ErrorType PWM_E_ALREADY_INITIALIZED = 0x14;
#define PWM_E_ALREADY_INITIALIZED 		0x14
//@}

/*
 * Implemented functions
 ****************************/
void Pwm_Init(const Pwm_ConfigType* ConfigPtr);
void Pwm_DeInit();
void Pwm_GetVersionInfo(Std_VersionInfoType* VersionInfo);
#if PWM_SET_PERIOD_AND_DUTY==STD_ON
void Pwm_SetPeriodAndDuty(Pwm_ChannelType Channel, Pwm_PeriodType Period,
		Pwm_DutyCycleType DutyCycle);
#endif

void Pwm_SetDutyCycle(Pwm_ChannelType Channel, Pwm_DutyCycleType DutyCycle);
void Pwm_SetOutputToIdle(Pwm_ChannelType ChannelNumber);
/*
 * PWM085: The function Pwm_GetOutputState shall be pre compile configurable
 * ON/OFF by the configuration parameter PwmGetOutputState
 */
#if PWM_GET_OUTPUT_STATE==STD_ON
Pwm_OutputStateType Pwm_GetOutputState(Pwm_ChannelType Channel);
#endif

/*
 * PWM113: The function EnableNotification shall be pre compile configurable
 * ON/OFF by the configuration parameter PwmNotificationSupported
 *
 * PWM112: The function DisableNotification shall be pre compile configurable
 * ON/OFF by the configuration parameter PwmNotificationSupported
 */
#if PWM_NOTIFICATION_SUPPORTED==STD_ON
void Pwm_DisableNotification(Pwm_ChannelType Channel);
void Pwm_EnableNotification(Pwm_ChannelType ChannelNumber,
		Pwm_EdgeNotificationType Notification);
#endif

#endif /* PWM_H_ */
/** @} */
