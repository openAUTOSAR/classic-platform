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

/* NOTES
 * The PWMX is bound with submodule period. The PWM signals are generated in the following manner.
- The INIT and VAL1 registers define the PWM modulo/period. So the PWM counter counts from INIT to VAL1 and then reinit to INIT again etc.
- The VAL2 resp. VAL3 define the compare value when PWMA goes High resp. Low.
- The VAL4 resp. VAL5 define the compare value when PWMB goes High resp. Low.
- The VAL0 defines half cycle reload point and also define the time when PWMX signal is set and the local sync signal is reset.
- The VAL1 also causes PWMX reseting and asserting local sync.
So the usage of PMWA and PWMB signals is easy. The PWMX, if not use as input (for capture feature etc), can generate also the PWM signal but you have to take into account that it represents the local sync signal, which is usually selected as sync source by INIT_SEL bits of CTRL2.
 *
 */


#include <assert.h>
#include <string.h>

#include "Pwm.h"
#include "MemMap.h"
#if defined(USE_DET)
#include "Det.h"
#endif
#include "mpc55xx.h"

#include "Os.h"
#include "Mcu.h"
#if PWM_NOTIFICATION_SUPPORTED==STD_ON
#include "isr.h"
#include "irq.h"
#include "arc.h"
#endif

#define PWM_RUNTIME_CHANNEL_COUNT	12
#define CHANNELS_OK (Channel < 12)

#define FLEXPWM_SUB_MODULE_DIVIDER 3

const Pwm_ConfigType* PwmConfigPtr = NULL;

typedef enum {
	PWM_STATE_UNINITIALIZED, PWM_STATE_INITIALIZED
} Pwm_ModuleStateType;

static Pwm_ModuleStateType Pwm_ModuleState = PWM_STATE_UNINITIALIZED;

// Run-time variables
typedef struct {
	#if PWM_NOTIFICATION_SUPPORTED==STD_ON
		Pwm_EdgeNotificationType NotificationState;
	#endif
} Pwm_ChannelStructType;

// We use Pwm_ChannelType as index here
Pwm_ChannelStructType ChannelRuntimeStruct[PWM_RUNTIME_CHANNEL_COUNT];


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
    if( !CHANNELS_OK  || (Pwm_ArcChToIndex[Channel] == 0xff))
    {
#if PWM_DEV_ERROR_DETECT==STD_ON
    	Det_ReportError(PWM_MODULE_ID,0, apiId,PWM_E_PARAM_CHANNEL);
    	result = E_NOT_OK;
#endif
    }
    return result;
}

static void SetDutyCycle(Pwm_ChannelType Channel, Pwm_DutyCycleType DutyCycle);

#if PWM_DE_INIT_API==STD_ON
void Pwm_DeInitChannel(Pwm_ChannelType Channel);
#endif

#if PWM_NOTIFICATION_SUPPORTED==STD_ON
static void Pwm_Isr(uint8 sub);
static void Pwm_Isr_0(void){Pwm_Isr(0);}
static void Pwm_Isr_1(void){Pwm_Isr(1);}
static void Pwm_Isr_2(void){Pwm_Isr(2);}
static void Pwm_Isr_3(void){Pwm_Isr(3);}
#endif

static void calcPeriodTicksAndPrescaler(
				const Pwm_ChannelConfigurationType* channelConfig,
				uint16_t* ticks, Pwm_ChannelPrescalerType* prescaler) {

	uint32_t f_in = 0;

	f_in = McuE_GetPeripheralClock( PERIPHERAL_CLOCK_FLEXPWM_0 );

	uint32_t f_target = channelConfig->frequency;

	Pwm_ChannelPrescalerType pre;
	uint32_t ticks_temp;

	if (channelConfig->prescaler == PWM_CHANNEL_PRESCALER_AUTO) {
		// Go from lowest to highest prescaler
		for (pre = PWM_CHANNEL_PRESCALER_DIV_1; pre <= PWM_CHANNEL_PRESCALER_DIV_128; pre++) {
		  ticks_temp = f_in / (f_target * (1 << pre)); // Calc ticks
		  if (ticks_temp > 0x7fff) {
			ticks_temp = 0x7fff;  // Prescaler too low
		  } else {
			break;                // Prescaler ok
		  }
		}
	} else {
		pre = channelConfig->prescaler; // Use config setting
		ticks_temp = f_in / (f_target * (1 << pre)); // Calc ticks
		if (ticks_temp > 0x7fff) {
		  ticks_temp = 0x7fff;  // Prescaler too low
		}
	}

	(*ticks) = (uint16_t) ticks_temp;
	(*prescaler) = pre;
}


static void configureChannel(const Pwm_ChannelConfigurationType* channelConfig){

	Pwm_ChannelType channel = channelConfig->channel;
	volatile struct FLEXPWM_tag *flexHw;
	Pwm_ChannelPrescalerType prescaler;
	uint16_t period_ticks;

	flexHw = &FLEXPWM_0;

	/*  Remove submodule unique so that it is not configured for all channels */

	/* All channels on same submodule must have same period. Should be checked by validation */
	calcPeriodTicksAndPrescaler( channelConfig, &period_ticks, &prescaler );

	flexHw->MCTRL.B.CLDOK = 1 << channel / FLEXPWM_SUB_MODULE_DIVIDER;
	/* Prescaler */
	flexHw->SUB[channel / FLEXPWM_SUB_MODULE_DIVIDER].CTRL.B.PRSC = prescaler;

	/* Edge aligned output, modulo count */
	flexHw->SUB[channel / FLEXPWM_SUB_MODULE_DIVIDER].INIT.R =   0x0000; /* INIT value */
	flexHw->SUB[channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[1].R = period_ticks; /* maximum count i.e. period NOTE! Valid for whole subgroup */

	flexHw->SUB[channel / FLEXPWM_SUB_MODULE_DIVIDER].DISMAP.R   = 0x0000;	/* disable fault pin condition */
	flexHw->SUB[channel / FLEXPWM_SUB_MODULE_DIVIDER].CTRL2.B.INDEP = 1; /* Run as independent channels */

	/* Polarity */
	switch(channel % FLEXPWM_SUB_MODULE_DIVIDER)
	{
	case 0: /* PWMA */
		flexHw->SUB[channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[2].R = 0x0000; /* PWMA rising edge */
		flexHw->SUB[channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[3].R = period_ticks; /* PWMA falling edge */
		flexHw->SUB[channel / FLEXPWM_SUB_MODULE_DIVIDER].OCTRL.B.POLA = (channelConfig->polarity == PWM_LOW) ? 1 : 0;
		break;
	case 1: /* PWMB */
		flexHw->SUB[channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[4].R = 0x0000; /* PWMB rising edge */
		flexHw->SUB[channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[5].R = period_ticks; /* PWMB falling edge */
		flexHw->SUB[channel / FLEXPWM_SUB_MODULE_DIVIDER].OCTRL.B.POLB = (channelConfig->polarity == PWM_LOW) ? 1 : 0;
		break;
	case 2: /* PWMX */
		flexHw->SUB[channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[0].R = 0x0000; /* PWMX rising edge */
		flexHw->SUB[channel / FLEXPWM_SUB_MODULE_DIVIDER].OCTRL.B.POLX = (channelConfig->polarity == PWM_LOW) ? 1 : 0;
		break;
	default:
		break;
	}
	flexHw->MCTRL.B.LDOK = 1 << channel / FLEXPWM_SUB_MODULE_DIVIDER;

	/* PWM009: The function Pwm_Init shall start all PWM channels with the configured
		default values. If the duty cycle parameter equals:
		􀂃 0% or 100% : Then the PWM output signal shall be in the state according to
			the configured polarity parameter
		􀂃 >0% and <100%: Then the PWM output signal shall be modulated according
		to parameters period, duty cycle and configured polarity. */
	SetDutyCycle(channel, channelConfig->duty);
}

void Pwm_Init(const Pwm_ConfigType* ConfigPtr) {
    Pwm_ChannelType channel_iterator;
	volatile struct FLEXPWM_tag *flexHw;
	flexHw = &FLEXPWM_0;
    union out_t{
        vuint16_t R;
        struct {
            vuint16_t:4;
            vuint16_t PWMA_EN:4;
            vuint16_t PWMB_EN:4;
            vuint16_t PWMX_EN:4;
        } B;
    };
    union out_t mask;

    mask.R = 0;

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

    PwmConfigPtr = ConfigPtr;

    Pwm_ModuleState = PWM_STATE_INITIALIZED;

    for (channel_iterator = 0; channel_iterator < PWM_NUMBER_OF_CHANNELS; channel_iterator++) {
    	const Pwm_ChannelConfigurationType* channelConfig = &ConfigPtr->Channels[channel_iterator];
    	Pwm_ChannelType channel = channelConfig->channel;

    	configureChannel( channelConfig );

        #if PWM_NOTIFICATION_SUPPORTED==STD_ON
                /*
                 * PWM052: The function Pwm_Init shall disable all notifications.
                 *
                 * This is now implemented in the configuration macro.
                 */
                // Pwm_DisableNotification(channel);

                // Install ISR
				switch(channel)
				{
				case 0:
					mask.B.PWMA_EN |= 0b0001;
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr_0, (IrqType)(PWM0_COF0),PWM_ISR_PRIORITY, 0);
					break;
				case 1:
					mask.B.PWMB_EN |= 0b0001;
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr_0, (IrqType)(PWM0_COF0),PWM_ISR_PRIORITY, 0);
					break;
				case 2:
					mask.B.PWMX_EN |= 0b0001;
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr_0, (IrqType)(PWM0_COF0),PWM_ISR_PRIORITY, 0);
					break;
				case 3:
					mask.B.PWMA_EN |= 0b0010;
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr_1, (IrqType)(PWM0_COF1),PWM_ISR_PRIORITY, 0);
					break;
				case 4:
					mask.B.PWMX_EN |= 0b0010;
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr_1, (IrqType)(PWM0_COF1),PWM_ISR_PRIORITY, 0);
					break;
				case 5:
					mask.B.PWMB_EN |= 0b0010;
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr_1, (IrqType)(PWM0_COF1),PWM_ISR_PRIORITY, 0);
					break;
				case 6:
					mask.B.PWMA_EN |= 0b0100;
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr_2, (IrqType)(PWM0_COF2),PWM_ISR_PRIORITY, 0);
					break;
				case 7:
					mask.B.PWMB_EN |= 0b0100;
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr_2, (IrqType)(PWM0_COF2),PWM_ISR_PRIORITY, 0);
					break;
				case 8:
					mask.B.PWMX_EN |= 0b0100;
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr_2, (IrqType)(PWM0_COF2),PWM_ISR_PRIORITY, 0);
					break;
				case 9:
					mask.B.PWMA_EN |= 0b1000;
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr_3, (IrqType)(PWM0_COF3),PWM_ISR_PRIORITY, 0);
					break;
				case 10:
					mask.B.PWMB_EN |= 0b1000;
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr_3, (IrqType)(PWM0_COF3),PWM_ISR_PRIORITY, 0);
					break;
				case 11:
					mask.B.PWMX_EN |= 0b1000;
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr_3, (IrqType)(PWM0_COF3),PWM_ISR_PRIORITY, 0);
					break;
				default:
					break;
			    }

		#endif
    }

	flexHw->OUTEN.B.PWMA_EN = mask.B.PWMA_EN;
	flexHw->OUTEN.B.PWMB_EN = mask.B.PWMB_EN;
	flexHw->OUTEN.B.PWMX_EN = mask.B.PWMX_EN;
	flexHw->MCTRL.B.LDOK = mask.B.PWMA_EN | mask.B.PWMB_EN | mask.B.PWMX_EN;
	flexHw->MCTRL.B.RUN = mask.B.PWMA_EN | mask.B.PWMB_EN | mask.B.PWMX_EN;
}

#if PWM_DE_INIT_API==STD_ON

void inline Pwm_DeInitChannel(Pwm_ChannelType Channel) {
    Pwm_SetOutputToIdle(Channel);

    /*
     * PWM052: The function Pwm_DeInit shall disable all notifications.
     */
    #if PWM_NOTIFICATION_SUPPORTED==STD_ON
        Pwm_DisableNotification(Channel);
    #endif
}

void Pwm_DeInit() {
	Pwm_ChannelType channel_iterator;
	volatile struct FLEXPWM_tag *flexHw;
	flexHw = &FLEXPWM_0;

	if(E_OK != Pwm_ValidateInitialized(PWM_DEINIT_ID))
	{
		return;
	}

	for (channel_iterator = 0; channel_iterator < PWM_NUMBER_OF_CHANNELS; channel_iterator++) {
		Pwm_ChannelType channel = PwmConfigPtr->Channels[channel_iterator].channel;
		Pwm_DeInitChannel(channel);
	}

	// Disable module
	flexHw->OUTEN.B.PWMA_EN = 0b0000;
	flexHw->OUTEN.B.PWMB_EN = 0b0000;
	flexHw->OUTEN.B.PWMX_EN = 0b0000;
	flexHw->MCTRL.B.LDOK = 0b0000;
	flexHw->MCTRL.B.RUN = 0b0000;

	Pwm_ModuleState = PWM_STATE_UNINITIALIZED;
}
#endif


/*
 * PWM083: The function Pwm_SetPeriodAndDuty shall be pre compile time
 * changeable ON/OFF by the configuration parameter PwmSetPeriodAndDuty.
 */
#if PWM_SET_PERIOD_AND_DUTY_API==STD_ON
	void Pwm_SetPeriodAndDuty(Pwm_ChannelType Channel, Pwm_PeriodType Period,
			Pwm_DutyCycleType DutyCycle) {

		if ((E_OK != Pwm_ValidateInitialized(PWM_SETPERIODANDDUTY_ID)) ||
			(E_OK != Pwm_ValidateChannel(Channel, PWM_SETPERIODANDDUTY_ID)))
		{
			return;
		}

		if(PwmConfigPtr->ChannelClass[Pwm_ArcChToIndex[Channel]] != PWM_VARIABLE_PERIOD){
#if PWM_DEV_ERROR_DETECT==STD_ON
			Det_ReportError(PWM_MODULE_ID,0, PWM_SETPERIODANDDUTY_ID, PWM_E_PERIOD_UNCHANGEABLE);
#endif
			return;
		}

		volatile struct FLEXPWM_tag *flexHw;
		flexHw = &FLEXPWM_0;

		/* Note! Changing period of a channel means changing for whole submodule  */

		if(flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[1].R != Period) {
			flexHw->MCTRL.B.CLDOK = 1 << Channel / FLEXPWM_SUB_MODULE_DIVIDER;
			flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[1].R = Period;
			flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[3].R = Period;
			flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[5].R = Period;
			flexHw->MCTRL.B.LDOK = 1 << Channel / FLEXPWM_SUB_MODULE_DIVIDER;
		}

		Pwm_SetDutyCycle(Channel, DutyCycle);
	}
#endif

static void SetDutyCycle(Pwm_ChannelType Channel, Pwm_DutyCycleType DutyCycle)
{
	volatile struct FLEXPWM_tag *flexHw;
	flexHw = &FLEXPWM_0;

	uint16 leading_edge_position;

	/* Timer instant for leading edge */

	/*
	 * PWM017: The function Pwm_SetDutyCycle shall update the duty cycle at
	 * the end of the period if supported by the implementation and configured
	 * with PwmDutycycleUpdatedEndperiod. [ This is achieved in hardware since
	 * the A and B registers are double buffered ]
	 *
	 * PWM014: The function Pwm_SetDutyCycle shall set the output state according
	 * to the configured polarity parameter [which is already set from
	 * Pwm_Init], when the duty parameter is 0% [=0] or 100% [=0x8000].
	 */
	switch(Channel % FLEXPWM_SUB_MODULE_DIVIDER)
	{
	case 0: /* PWMA */
		leading_edge_position = (uint16) ((flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[3].R
					* (uint32) DutyCycle) >> 15);
		if(flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[2].R != leading_edge_position) {
			flexHw->MCTRL.B.CLDOK = 1 << Channel / FLEXPWM_SUB_MODULE_DIVIDER;
			flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[2].R = leading_edge_position;
			flexHw->MCTRL.B.LDOK = 1 << Channel / FLEXPWM_SUB_MODULE_DIVIDER;
		}
		break;
	case 1: /* PWMB */
		leading_edge_position = (uint16) ((flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[5].R
					* (uint32) DutyCycle) >> 15);
		if(flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[4].R != leading_edge_position) {
			flexHw->MCTRL.B.CLDOK = 1 << Channel / FLEXPWM_SUB_MODULE_DIVIDER;
			flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[4].R = leading_edge_position;
			flexHw->MCTRL.B.LDOK = 1 << Channel / FLEXPWM_SUB_MODULE_DIVIDER;
		}
		break;
	case 2: /* PWMX */
		leading_edge_position = (uint16) ((flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[1].R
					* (uint32) DutyCycle) >> 15);
		if(flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[0].R != leading_edge_position) {
			flexHw->MCTRL.B.CLDOK = 1 << Channel / FLEXPWM_SUB_MODULE_DIVIDER;
			flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[0].R = leading_edge_position;
			flexHw->MCTRL.B.LDOK = 1 << Channel / FLEXPWM_SUB_MODULE_DIVIDER;
		}
		break;
	default:
		break;
	}
}


/**
 * PWM013: The function Pwm_SetDutyCycle shall set the duty cycle of the PWM
 * channel.
 *
 * @param Channel PWM channel to use. 0 <= Channel < PWM_NUMBER_OF_CHANNELS <= 16
 * @param DutyCycle 0 <= DutyCycle <= 0x8000
 */
#if PWM_SET_DUTY_CYCLE_API==STD_ON
void Pwm_SetDutyCycle(Pwm_ChannelType Channel, Pwm_DutyCycleType DutyCycle)
{
	if ((E_OK != Pwm_ValidateInitialized(PWM_SETDUTYCYCLE_ID)) ||
		(E_OK != Pwm_ValidateChannel(Channel, PWM_SETDUTYCYCLE_ID)))
	{
		return;
	}
	SetDutyCycle(Channel, DutyCycle);
}
#endif

#if  PWM_SET_OUTPUT_TO_IDLE_API == STD_ON
	void Pwm_SetOutputToIdle(Pwm_ChannelType Channel)
	{
		if ((E_OK != Pwm_ValidateInitialized(PWM_SETOUTPUTTOIDLE_ID)) ||
			(E_OK != Pwm_ValidateChannel(Channel, PWM_SETOUTPUTTOIDLE_ID)))
		{
			return;
		}
		volatile struct FLEXPWM_tag *flexHw;
		flexHw = &FLEXPWM_0;

		switch(Channel % FLEXPWM_SUB_MODULE_DIVIDER)
		{
		case 0: /* PWMA */
			if(flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[2].R != 0) {
				flexHw->MCTRL.B.CLDOK = 1 << Channel / FLEXPWM_SUB_MODULE_DIVIDER;
				flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[2].R = 0;
				flexHw->MCTRL.B.LDOK = 1 << Channel / FLEXPWM_SUB_MODULE_DIVIDER;
			}
			break;
		case 1: /* PWMB */
			if(flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[4].R != 0) {
				flexHw->MCTRL.B.CLDOK = 1 << Channel / FLEXPWM_SUB_MODULE_DIVIDER;
				flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[4].R = 0;
				flexHw->MCTRL.B.LDOK = 1 << Channel / FLEXPWM_SUB_MODULE_DIVIDER;
			}
			break;
		case 2: /* PWMX */
			if(flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[0].R != 0) {
				flexHw->MCTRL.B.CLDOK = 1 << Channel / FLEXPWM_SUB_MODULE_DIVIDER;
				flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].VAL[0].R = 0;
				flexHw->MCTRL.B.LDOK = 1 << Channel / FLEXPWM_SUB_MODULE_DIVIDER;
			}
			break;
		default:
			break;
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
		Pwm_OutputStateType res = PWM_LOW;

		if ((E_OK == Pwm_ValidateInitialized(PWM_GETOUTPUTSTATE_ID)) &&
			(E_OK == Pwm_ValidateChannel(Channel, PWM_GETOUTPUTSTATE_ID)))
		{
			volatile struct FLEXPWM_tag *flexHw;
			flexHw = &FLEXPWM_0;

			switch(Channel % FLEXPWM_SUB_MODULE_DIVIDER)
			{
			case 0:
				res = (Pwm_OutputStateType)flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].OCTRL.B.PWMA_IN;
				break;
			case 1:
				res = (Pwm_OutputStateType)flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].OCTRL.B.PWMB_IN;
				break;
			case 2:
				res = (Pwm_OutputStateType)flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].OCTRL.B.PWMX_IN;
				break;
			default:
				break;
			}
		}

		return res;
	}
#endif

#if PWM_NOTIFICATION_SUPPORTED==STD_ON

	void Pwm_DisableNotification(Pwm_ChannelType Channel)
	{
		if ((E_OK != Pwm_ValidateInitialized(PWM_DISABLENOTIFICATION_ID)) ||
			(E_OK != Pwm_ValidateChannel(Channel, PWM_DISABLENOTIFICATION_ID)))
		{
			return;
		}

		volatile struct FLEXPWM_tag *flexHw;
		flexHw = &FLEXPWM_0;

		// Disable flags on this submodule
		flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].INTEN.B.CMPIE = 0;
	}

	void Pwm_EnableNotification(Pwm_ChannelType Channel,Pwm_EdgeNotificationType Notification)
	{
		if ((E_OK != Pwm_ValidateInitialized(PWM_ENABLENOTIFICATION_ID)) ||
			(E_OK != Pwm_ValidateChannel(Channel, PWM_ENABLENOTIFICATION_ID)))
		{
			return;
		}

		volatile struct FLEXPWM_tag *flexHw;
		flexHw = &FLEXPWM_0;

		ChannelRuntimeStruct[Channel].NotificationState = Notification;

		/* Enable flags on this submodule */
		flexHw->SUB[Channel / FLEXPWM_SUB_MODULE_DIVIDER].INTEN.B.CMPIE = 1;
	}

	static void Pwm_Isr(uint8 sub)
	{
		// Find out which channel that triggered the interrupt
		volatile struct FLEXPWM_tag *flexHw;
		flexHw = &FLEXPWM_0;
		uint8_t cmpflags = flexHw->SUB[sub].STS.B.CMPF;

		for(int i = 0; i < 6; i++)
		{
			uint8_t val = 1 << i;
			if(cmpflags & val){
				/* val[0] and val[1] is PWMX -> index = 2, val[2] and val[3] is PWMA -> index = 0,
				 * val[2] and val[3] is PWMB -> index = 1*/
				int index = 1;
				if(i < 2){
					index = 2;
				}else if(i < 4){
					index = 0;
				}
				Pwm_ChannelType channel_iterator = Pwm_ArcChToIndex[(sub*FLEXPWM_SUB_MODULE_DIVIDER)+index];
				if (PwmConfigPtr->NotificationHandlers[channel_iterator] != NULL && channel_iterator != 0xff)
				{
					Pwm_EdgeNotificationType notification = ChannelRuntimeStruct[(sub*FLEXPWM_SUB_MODULE_DIVIDER)+index].NotificationState;
					if ((notification == PWM_BOTH_EDGES) || (notification != i%2)) /* Falling edge = 0 */
					{
						PwmConfigPtr->NotificationHandlers[channel_iterator]();
					}
				}
			}
		}

		// Clear interrupt
		flexHw->SUB[sub].STS.B.CMPF = 0x3f;
	}

#endif /* PWM_NOTIFICATION_SUPPORED == STD_ON */
