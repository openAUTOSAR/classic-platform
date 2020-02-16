/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/

/** @addtogroup Pwm PWM Driver
 *  @{ */

/** @file Pwm.h
 * API and type definitions for PWM Driver.
 */

#ifndef PWM_H_
#define PWM_H_

#include "Modules.h"

#define PWM_SW_MAJOR_VERSION	1
#define PWM_SW_MINOR_VERSION	0
#define PWM_SW_PATCH_VERSION	0

#define PWM_MODULE_ID			MODULE_ID_PWM
#define PWM_VENDOR_ID			VENDOR_ID_ARCCORE

#define PWM_AR_MAJOR_VERSION	3
#define PWM_AR_MINOR_VERSION	1
#define PWM_AR_PATCH_VERSION	5

/*
 * PWM094: Std_VersionInfoType shall be imported from Std_Types.h
 */
#include "Std_Types.h"

#include "Dma.h"

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
#if PWM_DEV_ERROR_DETECT==STD_ON
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

/* PWM channel information and control structures */
struct PwmChannel {
	Dma_CBType*  	cb_base; 			// The address of the first DMA control block for this channel
	uint32*			data_base;			// The address of the first data sample for this channel
	uint32 			cycle_time_us;		// Cycle time [microsec]
	uint16 			prev_pulse_start;	// The sample number (relative to cycle start) of the previous pulse start
	uint16 			prev_pulse_width;	// Number of samples in the previous pulse
};

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

/* DET errors that the PWM can produce. */
typedef enum
{
  PWM_E_PARAM_CONFIG              = 0x10,
  PWM_E_UNINIT,
  PWM_E_PARAM_CHANNEL,
  PWM_E_PERIOD_UNCHANGEABLE,
  PWM_E_ALREADY_INITIALIZED
} Pwm_DetErrorType;

/** API service ID's */
typedef enum
{
	  PWM_INIT_ID = 0x00,
	  PWM_DEINIT_ID,
	  PWM_SETDUTYCYCLE_ID,
	  PWM_SETPERIODANDDUTY_ID,
	  PWM_SETOUTPUTTOIDLE_ID,
	  PWM_GETOUTPUTSTATE_ID,
	  PWM_DISABLENOTIFICATION_ID,
	  PWM_ENABLENOTIFICATION_ID,
	  PWM_GETVERSIONINFO_ID,
} Pwm_APIServiceIDType;

/*
 * Implemented functions
 ****************************/

/* [SWS_Pwm_00095] - Service for PWM initialization. */
void Pwm_Init(const Pwm_ConfigType* ConfigPtr);
/* [SWS_Pwm_00096] - Service for PWM De-Initialization. */
void Pwm_DeInit(void);
/* [SWS_Pwm_00097] - Service sets the duty cycle of a PWM channel. */
void Pwm_SetDutyCycle(Pwm_ChannelType ChannelNumber,
					  Pwm_DutyCycleType DutyCycle);
/* [SWS_Pwm_00098] - Service sets the period and the duty cycle of a PWM channel. */
void Pwm_SetPeriodAndDuty(Pwm_ChannelType ChannelNumber,
						  Pwm_PeriodType Period,
						  Pwm_DutyCycleType DutyCycle);
/* [SWS_Pwm_0099] - Service sets the PWM output to the configured Idle state. */
void Pwm_SetOutputToIdle(Pwm_ChannelType ChannelNumber);
/* [SWS_Pwm_00100] - Service to read the internal state of the PWM output signal. */
Pwm_OutputStateType Pwm_GetOutputState(Pwm_ChannelType ChannelNumber);
/* [SWS_Pwm_00101] - Service to disable the PWM signal edge notification. */
void Pwm_DisableNotification(Pwm_ChannelType ChannelNumber);
/* [SWS_Pwm_00102] - Service to enable the PWM signal edge notification according to notification parameter. */
void Pwm_EnableNotification(Pwm_ChannelType ChannelNumber, Pwm_EdgeNotificationType Notification);
/* SWS_Pwm_00103] - Service returns the version information of this module. */
#define Pwm_GetVersionInfo(_vi) STD_GET_VERSION_INFO(_vi,PWM)

#endif /* PWM_H_ */
/** @} */
