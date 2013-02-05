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
 * Pwm.c
 *
 * TODO Only supports 8 8-bit channels. Could also be 4 16-bit.
 * Author: jonte
 */

#include <assert.h>
#include <string.h>

#include "Pwm.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#include "Os.h"
#include "Mcu.h"
#include "regs.h"
#include "arc.h"

typedef enum {
	PWM_STATE_UNINITIALIZED,
	PWM_STATE_INITIALIZED
} Pwm_ModuleStateType;

static void SetRegChannelBit(volatile uint8 *reg, uint8 channel, uint8 value)
{
	if(value==0)
	{
		*reg &= ~(1<<channel);
	}
	else
	{
		*reg |= (1<<channel);
	}
}

static Pwm_ModuleStateType Pwm_ModuleState = PWM_STATE_UNINITIALIZED;

// Run-time variables
typedef struct {
	Pwm_ChannelClassType Class;
	Pwm_OutputStateType IdleState;
} Pwm_ChannelStructType;

/* Local functions */
Std_ReturnType Pwm_ValidateInitialized(Pwm_APIServiceIDType apiId)
{
	Std_ReturnType result = E_OK;
    if( Pwm_ModuleState == PWM_STATE_UNINITIALIZED )
    {
#if PWM_DEV_ERROR_DETECT==STD_ON
    	Det_ReportError(PWM_MODULE_ID,0, apiId,PWM_E_UNINIT);
    	result = E_NOT_OK;
#endif
    }
    return result;
}

Std_ReturnType Pwm_ValidateChannel(Pwm_ChannelType Channel,Pwm_APIServiceIDType apiId)
{
	Std_ReturnType result = E_OK;
    if( Channel > 7  )
    {
#if PWM_DEV_ERROR_DETECT==STD_ON
    	Det_ReportError(PWM_MODULE_ID,0, apiId,PWM_E_PARAM_CHANNEL);
    	result = E_NOT_OK;
#endif
    }
    return result;
}

// We use Pwm_ChannelType as index here
static Pwm_ChannelStructType ChannelRuntimeStruct[8];

void Pwm_Init(const Pwm_ConfigType* ConfigPtr) {
    Pwm_ChannelType channel_iterator;

    /** @req PWM118 */
    /** @req PWM121 */
    if( Pwm_ModuleState == PWM_STATE_INITIALIZED ) {
#if PWM_DEV_ERROR_DETECT==STD_ON
    	Det_ReportError(PWM_MODULE_ID,0,PWM_INIT_ID,PWM_E_ALREADY_INITIALIZED);
#endif
    	return;
    }

#if PWM_DEV_ERROR_DETECT==STD_ON
        /*
         * PWM046: If development error detection is enabled for the Pwm module,
         * the function Pwm_Init shall raise development error PWM_E_PARAM_CONFIG
         * if ConfigPtr is a null pointer.
         *
         * PWM120: For pre-compile and link-time configuration variants, a NULL
         * pointer shall be passed to the initialization routine. In this case the
         * check for this NULL pointer has to be omitted.
         */
        #if PWM_STATICALLY_CONFIGURED==STD_OFF
            if (ConfigPtr == NULL) {
            	Det_ReportError(PWM_MODULE_ID,0,PWM_INIT_ID,PWM_E_PARAM_CONFIG);
                return;
            }
        #endif
    #endif

    Pwm_ModuleState = PWM_STATE_INITIALIZED;

	PWMCTL = 0; // We only support 8 8bit channels
    if(PWM_FREEZE_ENABLE == STD_ON){
    	PWMCTL |= 0x04;
    }else{
    	PWMCTL &= ~0x04;
    }

    PWMPRCLK = ConfigPtr->busPrescalerA | (ConfigPtr->busPrescalerB << 4);

    PWMSCLA = ConfigPtr->prescalerA;
    PWMSCLB = ConfigPtr->prescalerB;

    for (channel_iterator = 0; channel_iterator < PWM_NUMBER_OF_CHANNELS; channel_iterator++) {
    	const Pwm_ChannelConfigurationType *chCfgPtr = &ConfigPtr->channels[channel_iterator];
    	Pwm_ChannelType channel = ConfigPtr->channels[channel_iterator].channel;

        ChannelRuntimeStruct[channel].Class = ConfigPtr->channels[channel_iterator].class;
        ChannelRuntimeStruct[channel].IdleState = ConfigPtr->channels[channel_iterator].idleState;

        // Set up the registers in hw
    	SetRegChannelBit(&PWMCAE, channel, chCfgPtr->centerAlign);
    	SetRegChannelBit(&PWMCLK, channel, chCfgPtr->scaledClockEnable);
    	SetRegChannelBit(&PWMPOL, channel, chCfgPtr->polarity);

    	volatile uint8 *pDuty = &PWMDTY0+channel;
    	*pDuty = chCfgPtr->duty;
    	volatile uint8 *pPer = &PWMPER0+channel;
    	*pPer = chCfgPtr->period;

    	SetRegChannelBit(&PWME, channel, 1); // Enable Channel
    }
}

#if PWM_DE_INIT_API==STD_ON

void Pwm_DeInit() {

	if(E_OK != Pwm_ValidateInitialized(PWM_DEINIT_ID))
	{
		return;
	}

	PWME = 0;

	Pwm_ModuleState = PWM_STATE_UNINITIALIZED;
}
#endif

/*
 * PWM083: The function Pwm_SetPeriodAndDuty shall be pre compile time
 * changeable ON/OFF by the configuration parameter PwmSetPeriodAndDuty.
 */
#if PWM_SET_PERIOD_AND_DUTY_API==STD_ON
void Pwm_SetPeriodAndDuty(Pwm_ChannelType Channel, Pwm_PeriodType Period,
		Pwm_DutyCycleType DutyCycle)
{
	if ((E_OK != Pwm_ValidateInitialized(PWM_SETPERIODANDDUTY_ID)) ||
		(E_OK != Pwm_ValidateChannel(Channel, PWM_SETPERIODANDDUTY_ID)))
	{
		return;
	}
	if(ChannelRuntimeStruct[Channel].Class != PWM_VARIABLE_PERIOD)
	{
#if PWM_DEV_ERROR_DETECT==STD_ON
		Det_ReportError(PWM_MODULE_ID,0, PWM_SETPERIODANDDUTY_ID, PWM_E_PERIOD_UNCHANGEABLE);
#endif
		return;
	}

	volatile uint8 *pDuty = &PWMDTY0+Channel;
	*pDuty = (uint8)((uint32)((uint32)DutyCycle*(uint32)Period)>>15);
	volatile uint8 *pPer = &PWMPER0+Channel;
	*pPer = Period;

	if(PWM_DUTYCYCLE_UPDATED_ENDPERIOD != STD_ON){
		//Reset Counter to update directly
		volatile uint8 *pCtr = &PWMCNT0+Channel;
		*pCtr = 0;
	}
}
#endif


/**
 * PWM013: The function Pwm_SetDutyCycle shall set the duty cycle of the PWM
 * channel.
 *
 * @param Channel PWM channel to use. 0 <= Channel < PWM_NUMBER_OF_CHANNELS <= 7
 * @param DutyCycle 0 <= DutyCycle <= 0x8000
 */
#if PWM_SET_DUTY_CYCLE_API==STD_ON
void Pwm_SetDutyCycle(Pwm_ChannelType Channel, Pwm_DutyCycleType DutyCycle) {
	if ((E_OK != Pwm_ValidateInitialized(PWM_SETDUTYCYCLE_ID)) ||
		(E_OK != Pwm_ValidateChannel(Channel, PWM_SETDUTYCYCLE_ID)))
	{
		return;
	}

	volatile uint8 *pDuty = &PWMDTY0+Channel;

	/*
	 * PWM017: The function Pwm_SetDutyCycle shall update the duty cycle at
	 * the end of the period if supported by the implementation and configured
	 * with PwmDutycycleUpdatedEndperiod.
	 *
	 * PWM014: The function Pwm_SetDutyCycle shall set the output state according
	 * to the configured polarity parameter [which is already set from
	 * Pwm_InitChannel], when the duty parameter is 0% [=0] or 100% [=0x8000].
	 */
	if (DutyCycle == Pwm_100_Procent || DutyCycle == Pwm_0_Procent) {
		*pDuty = 0;
	} else {
		volatile uint8 *pPer = &PWMPER0+Channel;
		*pDuty = (uint8)((uint32)((uint32)DutyCycle*(uint32)*pPer)>>15);
	}

	if(PWM_DUTYCYCLE_UPDATED_ENDPERIOD != STD_ON){
		//Reset Counter to update directly
		volatile uint8 *pCtr = &PWMCNT0+Channel;
    	*pCtr = 0;
 	}
}
#endif

#if PWM_SET_OUTPUT_TO_IDLE_API==STD_ON
void Pwm_SetOutputToIdle(Pwm_ChannelType Channel) {
	if ((E_OK != Pwm_ValidateInitialized(PWM_SETOUTPUTTOIDLE_ID)) ||
		(E_OK != Pwm_ValidateChannel(Channel, PWM_SETOUTPUTTOIDLE_ID)))
	{
		return;
	}

	volatile uint8 *pDuty = &PWMDTY0+Channel;
	if(ChannelRuntimeStruct[Channel].IdleState == PWM_HIGH)
	{
		// Output shall be high
		if((PWMPOL & (1 << Channel)) == (1 << Channel)){
			*pDuty = 100;//Polarity high at beginning and goes low when duty is reached
		}else{
			*pDuty = 0;
		}
	}
	else
	{
		// Output shall be low
		if((PWMPOL & (1 << Channel)) == (1 << Channel)){
			*pDuty = 0;//Polarity high at beginning and goes low when duty is reached
		}else{
			*pDuty = 100;
		}
	}
}
#endif

/*
 * PWM085: The function Pwm_GetOutputState shall be pre compile configurable
 * ON/OFF by the configuration parameter PwmGetOutputState
 */
#if PWM_GET_OUTPUT_STATE_API==STD_ON
/*
 * PWM022: The function Pwm_GetOutputState shall read the internal state
 * of the PWM output signal and return it.
 */
Pwm_OutputStateType Pwm_GetOutputState(Pwm_ChannelType Channel)
{
	if ((E_OK != Pwm_ValidateInitialized(PWM_GETOUTPUTSTATE_ID)) ||
		(E_OK != Pwm_ValidateChannel(Channel, PWM_GETOUTPUTSTATE_ID)))
	{
		return PWM_LOW;
	}

	// TODO Is there an easy way to read from the port
	return PWM_LOW;
}
#endif

