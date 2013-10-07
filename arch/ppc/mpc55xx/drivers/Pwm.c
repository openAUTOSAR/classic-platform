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

#if defined(CFG_MPC5604B) || defined (CFG_MPC5602B)
	#define PWM_RUNTIME_CHANNEL_COUNT	56
    #define CHANNELS_OK (Channel <= PWM_MAX_CHANNEL-1)
#elif defined(CFG_MPC5606B)
	#define PWM_RUNTIME_CHANNEL_COUNT	64
    #define CHANNELS_OK (Channel <= PWM_MAX_CHANNEL-1)
#elif defined(CFG_MPC5606S)
	#define PWM_RUNTIME_CHANNEL_COUNT	48
    #define CHANNELS_OK (((Channel <= PWM_MAX_CHANNEL-1) && (Channel >= 40)) || ((Channel <= 23) && (Channel >= 16)))
#elif defined(CFG_MPC563XM)
	#define PWM_RUNTIME_CHANNEL_COUNT	24
	#define CHANNELS_OK (Channel < 24)
#else
	#define PWM_RUNTIME_CHANNEL_COUNT	16
	#define CHANNELS_OK (Channel < 16)
#endif

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
    if( !CHANNELS_OK  )
    {
#if PWM_DEV_ERROR_DETECT==STD_ON
    	Det_ReportError(PWM_MODULE_ID,0, apiId,PWM_E_PARAM_CHANNEL);
    	result = E_NOT_OK;
#endif
    }
    return result;
}

#if PWM_DE_INIT_API==STD_ON
void Pwm_DeInitChannel(Pwm_ChannelType Channel);
#endif

#if PWM_NOTIFICATION_SUPPORTED==STD_ON
static void Pwm_Isr(void);
#endif

static void calcPeriodTicksAndPrescaler(
				const Pwm_ChannelConfigurationType* channelConfig,
				uint16_t* ticks, Pwm_ChannelPrescalerType* prescaler) {

	uint32_t pre_global = 0;
	uint32_t f_in = 0;

#if defined(CFG_MPC560X)
	Pwm_ChannelType channel = channelConfig->channel;

	if(channel <= PWM_NUMBER_OF_EACH_EMIOS-1) {
		f_in = McuE_GetPeripheralClock( PERIPHERAL_CLOCK_EMIOS_0 );
		pre_global = EMIOS_0.MCR.B.GPRE;
	} else {
		f_in = McuE_GetPeripheralClock( PERIPHERAL_CLOCK_EMIOS_1 );
		pre_global = EMIOS_1.MCR.B.GPRE;
	}
#else
	f_in = McuE_GetPeripheralClock( PERIPHERAL_CLOCK_EMIOS );
	pre_global = EMIOS.MCR.B.GPRE;
#endif

  uint32_t f_target = channelConfig->frequency;

  Pwm_ChannelPrescalerType pre;
  uint32_t ticks_temp;

  if (channelConfig->prescaler == PWM_CHANNEL_PRESCALER_AUTO) {
    // Go from lowest to highest prescaler
    for (pre = PWM_CHANNEL_PRESCALER_1; pre <= PWM_CHANNEL_PRESCALER_4; pre++) {
      ticks_temp = f_in / (f_target * (pre_global + 1) * (pre + 1)); // Calc ticks
      if (ticks_temp > 0xffff) {
        ticks_temp = 0xffff;  // Prescaler too low
      } else {
        break;                // Prescaler ok
      }
    }
  } else {
    pre = channelConfig->prescaler; // Use config setting
    ticks_temp = f_in / (f_target * (pre_global + 1) * (pre+1)); // Calc ticks
    if (ticks_temp > 0xffff) {
      ticks_temp = 0xffff;  // Prescaler too low
    }
  }

  (*ticks) = (uint16_t) ticks_temp;
  (*prescaler) = pre;
}


static void configureChannel(const Pwm_ChannelConfigurationType* channelConfig){

	Pwm_ChannelType channel = channelConfig->channel;
	volatile struct EMIOS_tag *emiosHw;

#if defined(CFG_MPC560X)
	if(channel <= PWM_NUMBER_OF_EACH_EMIOS-1) {
		emiosHw = &EMIOS_0;
	} else {
		emiosHw = &EMIOS_1;
		channel -= PWM_NUMBER_OF_EACH_EMIOS;
	}
#else
	emiosHw = &EMIOS;
#endif

#if defined(CFG_MPC560X) || defined(CFG_MPC563XM)
	emiosHw->CH[channel].CCR.B.MODE = channelConfig->mode;
	emiosHw->CH[channel].CCR.B.DMA = 0;
	emiosHw->CH[channel].CCR.B.BSL = channelConfig->clksrc;
	emiosHw->CH[channel].CCR.B.ODIS = 0;
#else
	emiosHw->CH[channel].CCR.B.MODE = PWM_EMIOS_OPWM;
	emiosHw->CH[channel].CCR.B.DMA = 0;
	emiosHw->CH[channel].CCR.B.BSL = 3;
	emiosHw->CH[channel].CCR.B.ODIS = 0;
#endif

	Pwm_ChannelPrescalerType prescaler;  uint16_t period_ticks;
	calcPeriodTicksAndPrescaler( channelConfig, &period_ticks, &prescaler );

	emiosHw->CH[channel].CBDR.R = period_ticks;
	emiosHw->CH[channel].CCR.B.UCPRE = prescaler;
#if defined(CFG_MPC560X)
	emiosHw->CH[channel].CCR.B.UCPEN = 1;
#else
	emiosHw->CH[channel].CCR.B.UCPREN = 1;
#endif

	/* PWM009: The function Pwm_Init shall start all PWM channels with the configured
		default values. If the duty cycle parameter equals:
		􀂃 0% or 100% : Then the PWM output signal shall be in the state according to
			the configured polarity parameter
		􀂃 >0% and <100%: Then the PWM output signal shall be modulated according
		to parameters period, duty cycle and configured polarity. */
	emiosHw->CH[channel].CADR.R = (uint16) (((uint32) period_ticks * (uint32) channelConfig->duty) >> 15);

	// 0 A match on comparator A clears the output flip-flop, while a match on comparator B sets it
	// 1 A match on comparator A sets the output flip-flop, while a match on comparator B clears it
	// A duty cycle of X % should give a signal with state 'channelConfig->polarity' during
	// X % of the period time.
	emiosHw->CH[channel].CCR.B.EDPOL = (channelConfig->polarity == PWM_LOW) ? 1 : 0;
}

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

    #if defined(CFG_MPC5606S) && !defined(CFG_MPC560XB)
		CGM.AC1_SC.R = 0x03000000; /* MPC56xxS: Select aux. set 1 clock to be FMPLL0 */
		CGM.AC2_SC.R = 0x03000000; /* MPC56xxS: Select aux. set 2 clock to be FMPLL0 */
	#endif
	
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

	#if defined(CFG_MPC560X)
		/* Clock scaler uses system clock (~64MHz) as source, so prescaler 64 => 1MHz. */
		EMIOS_0.MCR.B.GPRE = PWM_PRESCALER_EMIOS_0 - 1;
		EMIOS_1.MCR.B.GPRE = PWM_PRESCALER_EMIOS_1 - 1;

		/* Enable eMIOS clock */
		EMIOS_0.MCR.B.GPREN = 1;
		EMIOS_1.MCR.B.GPREN = 1;

		/* Stop channels when in debug mode */
		EMIOS_0.MCR.B.FRZ = PWM_FREEZE_ENABLE;
		EMIOS_1.MCR.B.FRZ = PWM_FREEZE_ENABLE;

		/* Use global time base */
		EMIOS_0.MCR.B.GTBE = 1;
		EMIOS_1.MCR.B.GTBE = 1;
	#else
		/* Clock scaler uses system clock (~64MHz) as source, so prescaler 64 => 1MHz. */
		EMIOS.MCR.B.GPRE = PWM_PRESCALER - 1;

		/* Enable eMIOS clock */
		EMIOS.MCR.B.GPREN = 1;

		/* Stop channels when in debug mode */
		EMIOS.MCR.B.FRZ = PWM_FREEZE_ENABLE;

		/* Use global time base */
		EMIOS.MCR.B.GTBE = 1;
	#endif

    Pwm_ModuleState = PWM_STATE_INITIALIZED;

    for (channel_iterator = 0; channel_iterator < PWM_NUMBER_OF_CHANNELS; channel_iterator++) {
    	const Pwm_ChannelConfigurationType* channelConfig = &ConfigPtr->Channels[channel_iterator];
    	Pwm_ChannelType channel = channelConfig->channel;
    	volatile struct EMIOS_tag *emiosHw;

#if defined(CFG_MPC560X)
		if(channel <= PWM_NUMBER_OF_EACH_EMIOS-1) {
			emiosHw = &EMIOS_0;
		} else {
			emiosHw = &EMIOS_1;
			channel -= PWM_NUMBER_OF_EACH_EMIOS;
		}
#else
		emiosHw = &EMIOS;
#endif

#if !defined(CFG_MPC5567)
		// Clear the disable bit for this channel
		emiosHw->UCDIS.R &= ~(1 << channel);
#endif

    	configureChannel( channelConfig );

        #if PWM_NOTIFICATION_SUPPORTED==STD_ON
                /*
                 * PWM052: The function Pwm_Init shall disable all notifications.
                 *
                 * This is now implemented in the configuration macro.
                 */
                // Pwm_DisableNotification(channel);

                // Install ISR
			#if defined(CFG_MPC560XB) || defined(CFG_MPC5604P)
				switch(channel)
				{
				case 0:
				case 1:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_0_GFR_F0_F1),PWM_ISR_PRIORITY, 0);
					break;
				case 2:
				case 3:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_0_GFR_F2_F3),PWM_ISR_PRIORITY, 0);
					break;
				case 4:
				case 5:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_0_GFR_F4_F5),PWM_ISR_PRIORITY, 0);
					break;
				case 6:
				case 7:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_0_GFR_F6_F7),PWM_ISR_PRIORITY, 0);
					break;
				case 8:
				case 9:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_0_GFR_F8_F9),PWM_ISR_PRIORITY, 0);
					break;
				case 10:
				case 11:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_0_GFR_F10_F11),PWM_ISR_PRIORITY, 0);
					break;
				case 12:
				case 13:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_0_GFR_F12_F13),PWM_ISR_PRIORITY, 0);
					break;
				case 14:
				case 15:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_0_GFR_F14_F15),PWM_ISR_PRIORITY, 0);
					break;
				case 16:
				case 17:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_0_GFR_F16_F17),PWM_ISR_PRIORITY, 0);
					break;
				case 18:
				case 19:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_0_GFR_F18_F19),PWM_ISR_PRIORITY, 0);
					break;
				case 20:
				case 21:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_0_GFR_F20_F21),PWM_ISR_PRIORITY, 0);
					break;
				case 22:
				case 23:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_0_GFR_F22_F23),PWM_ISR_PRIORITY, 0);
					break;
				case 24:
				case 25:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_0_GFR_F24_F25),PWM_ISR_PRIORITY, 0);
					break;
				case 26:
				case 27:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_0_GFR_F26_F27),PWM_ISR_PRIORITY, 0);
					break;
				case 28:
				case 29:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_1_GFR_F0_F1),PWM_ISR_PRIORITY, 0);
					break;
				case 30:
				case 31:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_1_GFR_F2_F3),PWM_ISR_PRIORITY, 0);
					break;
				case 32:
				case 33:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_1_GFR_F4_F5),PWM_ISR_PRIORITY, 0);
					break;
				case 34:
				case 35:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_1_GFR_F6_F7),PWM_ISR_PRIORITY, 0);
					break;
				case 36:
				case 37:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_1_GFR_F8_F9),PWM_ISR_PRIORITY, 0);
					break;
				case 38:
				case 39:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_1_GFR_F10_F11),PWM_ISR_PRIORITY, 0);
					break;
				case 40:
				case 41:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_1_GFR_F12_F13),PWM_ISR_PRIORITY, 0);
					break;
				case 42:
				case 43:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_1_GFR_F14_F15),PWM_ISR_PRIORITY, 0);
					break;
				case 44:
				case 45:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_1_GFR_F16_F17),PWM_ISR_PRIORITY, 0);
					break;
				case 46:
				case 47:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_1_GFR_F18_F19),PWM_ISR_PRIORITY, 0);
					break;
				case 48:
				case 49:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_1_GFR_F20_F21),PWM_ISR_PRIORITY, 0);
					break;
				case 50:
				case 51:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_1_GFR_F22_F23),PWM_ISR_PRIORITY, 0);
					break;
				case 52:
				case 53:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_1_GFR_F24_F25),PWM_ISR_PRIORITY, 0);
					break;
				case 54:
				case 55:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_1_GFR_F26_F27),PWM_ISR_PRIORITY, 0);
					break;
				default:
					break;
				}
			#elif defined(CFG_MPC5606S)
				switch(channel)
				{
				case 16:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_0_GFR_F16_F17),PWM_ISR_PRIORITY, 0);
					break;
				case 17:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_0_GFR_F16_F17),PWM_ISR_PRIORITY, 0);
			    break;
				case 18:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_0_GFR_F18_F19),PWM_ISR_PRIORITY, 0);
					break;
				case 19:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_0_GFR_F18_F19),PWM_ISR_PRIORITY, 0);
			    break;
				case 20:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_0_GFR_F20_F21),PWM_ISR_PRIORITY, 0);
					break;
				case 21:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_0_GFR_F20_F21),PWM_ISR_PRIORITY, 0);
			    break;
				case 22:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_0_GFR_F22_F23),PWM_ISR_PRIORITY, 0);
					break;
				case 23:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_0_GFR_F22_F23),PWM_ISR_PRIORITY, 0);
			    break;
				case 40:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_1_GFR_F16_F17),PWM_ISR_PRIORITY, 0);
					break;
				case 41:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_1_GFR_F16_F17),PWM_ISR_PRIORITY, 0);
			    break;
				case 42:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_1_GFR_F18_F19),PWM_ISR_PRIORITY, 0);
					break;
				case 43:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_1_GFR_F18_F19),PWM_ISR_PRIORITY, 0);
			    break;
				case 44:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_1_GFR_F20_F21),PWM_ISR_PRIORITY, 0);
					break;
				case 45:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_1_GFR_F20_F21),PWM_ISR_PRIORITY, 0);
			    break;
				case 46:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_1_GFR_F22_F23),PWM_ISR_PRIORITY, 0);
					break;
				case 47:
					ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMIOS_1_GFR_F22_F23),PWM_ISR_PRIORITY, 0);
			    break;
				default:
					break;
			    }
			#else
				switch(channel)
				{
				case 0:
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMISOS200_FLAG_F0),PWM_ISR_PRIORITY, 0);
					break;
				case 1:
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMISOS200_FLAG_F1),PWM_ISR_PRIORITY, 0);
					break;
				case 2:
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMISOS200_FLAG_F2),PWM_ISR_PRIORITY, 0);
					break;
				case 3:
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMISOS200_FLAG_F3),PWM_ISR_PRIORITY, 0);
					break;
				case 4:
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMISOS200_FLAG_F4),PWM_ISR_PRIORITY, 0);
					break;
				case 5:
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMISOS200_FLAG_F5),PWM_ISR_PRIORITY, 0);
					break;
				case 6:
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMISOS200_FLAG_F6),PWM_ISR_PRIORITY, 0);
					break;
				case 7:
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMISOS200_FLAG_F7),PWM_ISR_PRIORITY, 0);
					break;
				case 8:
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMISOS200_FLAG_F8),PWM_ISR_PRIORITY, 0);
					break;
				case 9:
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMISOS200_FLAG_F9),PWM_ISR_PRIORITY, 0);
					break;
				case 10:
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMISOS200_FLAG_F10),PWM_ISR_PRIORITY, 0);
					break;
				case 11:
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMISOS200_FLAG_F11),PWM_ISR_PRIORITY, 0);
					break;
				case 12:
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMISOS200_FLAG_F12),PWM_ISR_PRIORITY, 0);
					break;
				case 13:
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMISOS200_FLAG_F13),PWM_ISR_PRIORITY, 0);
					break;
				case 14:
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMISOS200_FLAG_F14),PWM_ISR_PRIORITY, 0);
					break;
				case 15:
	            	ISR_INSTALL_ISR2("PwmIsr", Pwm_Isr, (IrqType)(EMISOS200_FLAG_F15),PWM_ISR_PRIORITY, 0);
					break;
				default:
					break;
			    }
        	#endif

		#endif
    }

	/* Enable module */
    #if defined(CFG_MPC560X)
		EMIOS_0.MCR.B.MDIS = 0;
		EMIOS_1.MCR.B.MDIS = 0;
	#else
		EMIOS.MCR.B.MDIS = 0;
	#endif
}

#if PWM_DE_INIT_API==STD_ON

void inline Pwm_DeInitChannel(Pwm_ChannelType Channel) {
	volatile struct EMIOS_tag *emiosHw;
#if defined(CFG_MPC560X)
	if(Channel <= PWM_NUMBER_OF_EACH_EMIOS-1) {
		emiosHw = &EMIOS_0;
	} else {
		emiosHw = &EMIOS_1;
		Channel -= PWM_NUMBER_OF_EACH_EMIOS;
	}
#else
	emiosHw = &EMIOS;
#endif

	emiosHw->CH[Channel].CADR.R = 0;

#if !defined(CFG_MPC5567)
	// Set the disable bit for this channel
	emiosHw->UCDIS.R |= (1 << Channel);
#endif
    /*
     * PWM052: The function Pwm_DeInit shall disable all notifications.
     */
    #if PWM_NOTIFICATION_SUPPORTED==STD_ON
        Pwm_DisableNotification(Channel);
    #endif
}

void Pwm_DeInit() {
	Pwm_ChannelType channel_iterator;

	if(E_OK != Pwm_ValidateInitialized(PWM_DEINIT_ID))
	{
		return;
	}

	for (channel_iterator = 0; channel_iterator < PWM_NUMBER_OF_CHANNELS; channel_iterator++) {
		Pwm_ChannelType channel = PwmConfigPtr->Channels[channel_iterator].channel;
		Pwm_DeInitChannel(channel);
	}

	// Disable module
    #if defined(CFG_MPC560X)
		EMIOS_0.MCR.B.MDIS = 1;
		EMIOS_1.MCR.B.MDIS = 1;
	#else
		EMIOS.MCR.B.MDIS = 1;
	#endif

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

		for (Pwm_ChannelType channel_iterator = 0; channel_iterator < PWM_NUMBER_OF_CHANNELS; channel_iterator++)
		{
			if(Channel == PwmConfigPtr->Channels[channel_iterator].channel){
				if(PwmConfigPtr->ChannelClass[channel_iterator] != PWM_VARIABLE_PERIOD){
		#if PWM_DEV_ERROR_DETECT==STD_ON
					Det_ReportError(PWM_MODULE_ID,0, PWM_SETPERIODANDDUTY_ID, PWM_E_PERIOD_UNCHANGEABLE);
		#endif
					return;
				}
				break;
			}
		}

		volatile struct EMIOS_tag *emiosHw;
#if defined(CFG_MPC560X)
		if(Channel <= PWM_NUMBER_OF_EACH_EMIOS-1) {
			emiosHw = &EMIOS_0;
		} else {
			emiosHw = &EMIOS_1;
			Channel -= PWM_NUMBER_OF_EACH_EMIOS;
		}
#else
		emiosHw = &EMIOS;
#endif

		uint16 leading_edge_position = (uint16) (((uint32) Period * (uint32) DutyCycle) >> 15);

		/* Timer instant for leading edge */
		emiosHw->CH[Channel].CADR.R = leading_edge_position;

		/* Timer instant for the period to restart */
		emiosHw->CH[Channel].CBDR.R = Period;
	}
#endif


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
	volatile struct EMIOS_tag *emiosHw;
#if defined(CFG_MPC560X)
	if(Channel <= PWM_NUMBER_OF_EACH_EMIOS-1) {
		emiosHw = &EMIOS_0;
	} else {
		emiosHw = &EMIOS_1;
		Channel -= PWM_NUMBER_OF_EACH_EMIOS;
	}
#else
	emiosHw = &EMIOS;
#endif

	uint16 leading_edge_position = (uint16) ((emiosHw->CH[Channel].CBDR.R
				* (uint32) DutyCycle) >> 15);

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
	emiosHw->CH[Channel].CADR.R = leading_edge_position;
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
		volatile struct EMIOS_tag *emiosHw;
	#if defined(CFG_MPC560X)
		if(Channel <= PWM_NUMBER_OF_EACH_EMIOS-1) {
			emiosHw = &EMIOS_0;
		} else {
			emiosHw = &EMIOS_1;
			Channel -= PWM_NUMBER_OF_EACH_EMIOS;
		}
	#else
		emiosHw = &EMIOS;
	#endif

		emiosHw->CH[Channel].CADR.R = 0;
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

		volatile struct EMIOS_tag *emiosHw;
	#if defined(CFG_MPC560X)
		if(Channel <= PWM_NUMBER_OF_EACH_EMIOS-1) {
			emiosHw = &EMIOS_0;
		} else {
			emiosHw = &EMIOS_1;
			Channel -= PWM_NUMBER_OF_EACH_EMIOS;
		}
	#else
		emiosHw = &EMIOS;
	#endif

		return (Pwm_OutputStateType)emiosHw->CH[Channel].CSR.B.UCOUT;
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

		volatile struct EMIOS_tag *emiosHw;
	#if defined(CFG_MPC560X)
		if(Channel <= PWM_NUMBER_OF_EACH_EMIOS-1) {
			emiosHw = &EMIOS_0;
		} else {
			emiosHw = &EMIOS_1;
			Channel -= PWM_NUMBER_OF_EACH_EMIOS;
		}
	#else
		emiosHw = &EMIOS;
	#endif

		// Disable flags on this channel
		emiosHw->CH[Channel].CCR.B.FEN = 0;
	}

	void Pwm_EnableNotification(Pwm_ChannelType Channel,Pwm_EdgeNotificationType Notification)
	{
		if ((E_OK != Pwm_ValidateInitialized(PWM_ENABLENOTIFICATION_ID)) ||
			(E_OK != Pwm_ValidateChannel(Channel, PWM_ENABLENOTIFICATION_ID)))
		{
			return;
		}

		volatile struct EMIOS_tag *emiosHw;
	#if defined(CFG_MPC560X)
		if(Channel <= PWM_NUMBER_OF_EACH_EMIOS-1) {
			emiosHw = &EMIOS_0;
		} else {
			emiosHw = &EMIOS_1;
			Channel -= PWM_NUMBER_OF_EACH_EMIOS;
		}
	#else
		emiosHw = &EMIOS;
	#endif

		ChannelRuntimeStruct[Channel].NotificationState = Notification;

		emiosHw->CH[Channel].CCR.B.FEN = 1;
	}

	static void Pwm_Isr(void)
	{
		// Find out which channel that triggered the interrupt
		#if defined (CFG_MPC5516)
			uint32_t flagmask = EMIOS.GFLAG.R;
		#elif defined(CFG_MPC5567)
			uint32_t flagmask = EMIOS.GFR.R;
        #endif

        #if defined(CFG_MPC5516) || defined(CFG_MPC5567)
			// There are 24 channels specified in the global flag register, but
			// we only listen to the first 16 as only these support OPWfM
			for (Pwm_ChannelType channel_iterator = 0; channel_iterator < PWM_NUMBER_OF_CHANNELS; channel_iterator++)
			{
				Pwm_ChannelType emios_ch = PwmConfigPtr->Channels[channel_iterator].channel;

				if (flagmask & (1 << emios_ch))
				{
					if (PwmConfigPtr->NotificationHandlers[channel_iterator] != NULL && EMIOS.CH[emios_ch].CCR.B.FEN)
					{
						Pwm_EdgeNotificationType notification = ChannelRuntimeStruct[emios_ch].NotificationState;
						if (notification == PWM_BOTH_EDGES ||

								notification == EMIOS.CH[emios_ch].CSR.B.UCOUT)
						{
							PwmConfigPtr->NotificationHandlers[channel_iterator]();
						}
					}

					// Clear interrupt
					EMIOS.CH[emios_ch].CSR.B.FLAG = 1;
				}
			}
        #elif defined(CFG_MPC560X)
			uint32_t flagmask_0 = EMIOS_0.GFR.R;
			uint32_t flagmask_1 = EMIOS_1.GFR.R;

			for (Pwm_ChannelType channel_iterator = 0; channel_iterator < PWM_NUMBER_OF_CHANNELS; channel_iterator++)
			{

				Pwm_ChannelType emios_ch = PwmConfigPtr->Channels[channel_iterator].channel;

				if (flagmask_0 & (1 << emios_ch))
				{
					if (PwmConfigPtr->NotificationHandlers[channel_iterator] != NULL && EMIOS_0.CH[emios_ch].CCR.B.FEN)
					{
						Pwm_EdgeNotificationType notification = ChannelRuntimeStruct[emios_ch].NotificationState;
						if (notification == PWM_BOTH_EDGES ||

								notification == EMIOS_0.CH[emios_ch].CSR.B.UCOUT)
						{
							PwmConfigPtr->NotificationHandlers[channel_iterator]();
						}
					}

					// Clear interrupt
					EMIOS_0.CH[emios_ch].CSR.B.FLAG = 1;
				}
				else if (flagmask_1 & (1 << emios_ch - PWM_NUMBER_OF_EACH_EMIOS ))
				{
					if (PwmConfigPtr->NotificationHandlers[channel_iterator] != NULL && EMIOS_1.CH[emios_ch - PWM_NUMBER_OF_EACH_EMIOS].CCR.B.FEN)
					{
						Pwm_EdgeNotificationType notification = ChannelRuntimeStruct[emios_ch].NotificationState;
						if (notification == PWM_BOTH_EDGES ||
								notification == EMIOS_1.CH[emios_ch - PWM_NUMBER_OF_EACH_EMIOS].CSR.B.UCOUT)
							{
							PwmConfigPtr->NotificationHandlers[channel_iterator]();
							}
						}

					// Clear interrupt
					EMIOS_1.CH[emios_ch - PWM_NUMBER_OF_EACH_EMIOS].CSR.B.FLAG = 1;
				}
			}

		#endif
	}

#endif /* PWM_NOTIFICATION_SUPPORED == STD_ON */
