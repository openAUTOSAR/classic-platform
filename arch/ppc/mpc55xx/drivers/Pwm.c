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
 * TODO: Implement DMA support for PWM
 * TODO: Test PWM for MPC5567 and MPC5554
 *
 *  Created on: 2009-jul-09
 *      Author: nian
 */

//#define USE_KERNEL

#include <assert.h>
#include <string.h>

#include "Pwm.h"
#include "MemMap.h"
//#include "SchM_Pwm.h"
#include "Det.h"
#ifdef CFG_MPC5516
#include "mpc5516.h"
#elif defined(CFG_MPC5567)
#include "mpc5567.h"
#elif defined(CFG_MPC5606S)
#include "mpc5606s.h"
#endif
#if defined(USE_KERNEL)
#include "Os.h"
#endif
#include "Mcu.h"
#if PWM_NOTIFICATION_SUPPORTED==STD_ON
#include "irq.h"
#include "arc.h"
#endif


#if defined(CFG_MPC5606S)
	#define PWM_RUNTIME_CHANNEL_COUNT	48
#else
	#define PWM_RUNTIME_CHANNEL_COUNT	16
#endif

#if PWM_DEV_ERROR_DETECT==STD_ON
	#define PWM_VALIDATE(_exp, _errid) \
		if (!(_exp)) { \
			Pwm_ReportError(_errid); \
			return; \
		}
    #if ( defined(CFG_MPC5516) || defined (CFG_MPC5567) )
		
		#define PWM_RUNTIME_CHANNEL_COUNT	16
		#define PWM_VALID_CHANNEL(_ch)		(_ch < 16)
		#define Pwm_VALIDATE_CHANNEL(_ch)	PWM_VALIDATE( PWM_VALID_CHANNEL(_ch), PWM_E_PARAM_CHANNEL )

	#elif  defined(CFG_MPC5606S)
	
		#define PWM_RUNTIME_CHANNEL_COUNT	48
		#define PWM_VALID_CHANNEL(_ch)		( ((_ch <= PWM_MAX_CHANNEL-1) && (_ch >= 40)) || ((_ch <= 23) && (_ch >= 16)) )
		#define Pwm_VALIDATE_CHANNEL(_ch)	PWM_VALIDATE( PWM_VALID_CHANNEL(_ch), PWM_E_PARAM_CHANNEL )
	
	#endif

	#define Pwm_VALIDATE_INITIALIZED() PWM_VALIDATE(Pwm_ModuleState == PWM_STATE_INITIALIZED, PWM_E_UNINIT)
	#define Pwm_VALIDATE_UNINITIALIZED() PWM_VALIDATE(Pwm_ModuleState != PWM_STATE_INITIALIZED, PWM_E_ALREADY_INITIALIZED)
#else
	#define PWM_VALIDATE(_exp, _errid)
	#define PWM_VALID_CHANNEL(_ch)	(1)
	#define Pwm_VALIDATE_CHANNEL(ch)
	#define Pwm_VALIDATE_INITIALIZED()
	#define Pwm_VALIDATE_UNINITIALIZED()
#endif

const Pwm_ConfigType* PwmConfigPtr = NULL;

typedef enum {
	PWM_STATE_UNINITIALIZED, PWM_STATE_INITIALIZED
} Pwm_ModuleStateType;

static Pwm_ModuleStateType Pwm_ModuleState = PWM_STATE_UNINITIALIZED;

// Run-time variables
typedef struct {
	Pwm_ChannelClassType Class;

	#if PWM_NOTIFICATION_SUPPORTED==STD_ON
		Pwm_NotificationHandlerType NotificationRoutine;
		Pwm_EdgeNotificationType NotificationState;
	#endif
} Pwm_ChannelStructType;

// We use Pwm_ChannelType as index here
Pwm_ChannelStructType ChannelRuntimeStruct[PWM_RUNTIME_CHANNEL_COUNT];

/* Local functions */
void inline Pwm_InitChannel(Pwm_ChannelType Channel);
void inline Pwm_DeInitChannel(Pwm_ChannelType Channel);

#if PWM_NOTIFICATION_SUPPORTED==STD_ON
static void Pwm_Isr(void);
#endif

#if ( defined(CFG_MPC5516) || defined (CFG_MPC5567) )
static void calcPeriodTicksAndPrescaler(
    const Pwm_ChannelConfigurationType* channelConfig,
    uint16_t* ticks, Pwm_ChannelPrescalerType* prescaler) {

  uint32_t f_in = McuE_GetPeripheralClock( PERIPHERAL_CLOCK_EMIOS );
  uint32_t f_target = channelConfig->frequency;
  uint32_t pre_global = EMIOS.MCR.B.GPRE;

  Pwm_ChannelPrescalerType pre;
  uint32_t ticks_temp;

  if (channelConfig->prescaler == PWM_CHANNEL_PRESCALER_AUTO) {
    // Go from lowest to highest prescaler
    for (pre = PWM_CHANNEL_PRESCALER_1; pre < PWM_CHANNEL_PRESCALER_4; ++pre) {
      ticks_temp = f_in / (f_target * (pre_global + 1) * (pre + 1)); // Calc ticks
      if (ticks_temp > 0xffff) {
        ticks_temp = 0xffff;  // Prescaler too low
      } else {
        break;                // Prescaler ok
      }
    }
  } else {
    pre = channelConfig->prescaler; // Use config setting
    ticks_temp = f_in / (f_target * pre_global * (pre+1)); // Calc ticks
    if (ticks_temp > 0xffff) {
      ticks_temp = 0xffff;  // Prescaler too low
    }
  }

  (*ticks) = (uint16_t) ticks_temp;
  (*prescaler) = pre;
}
#endif

void Pwm_Init(const Pwm_ConfigType* ConfigPtr) {
    Pwm_ChannelType channel_iterator;

    Pwm_VALIDATE_UNINITIALIZED();
    #if defined(CFG_MPC5606S)
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
        #if PWM_STATICALLY_CONFIGURED==OFF
            if (ConfigPtr == NULL) {
                Pwm_ReportError(PWM_E_PARAM_CONFIG);
                return;
            }
        #endif
    #endif

	#if defined(CFG_MPC5606S)

    	PwmConfigPtr = ConfigPtr;
        /* Clock scaler uses system clock (~64MHz) as source, so prescaler 64 => 1MHz. */
		EMIOS_0.MCR.B.GPRE = PWM_PRESCALER - 1;
		EMIOS_1.MCR.B.GPRE = PWM_PRESCALER - 1;

		/* Enable eMIOS clock */
		EMIOS_0.MCR.B.GPREN = 1;
		EMIOS_1.MCR.B.GPREN = 1;

		/* Stop channels when in debug mode */
		EMIOS_0.MCR.B.FRZ = PWM_FREEZE_ENABLE;
		EMIOS_1.MCR.B.FRZ = PWM_FREEZE_ENABLE;

		/* Use global time base */
		EMIOS_0.MCR.B.GTBE = 1;
		EMIOS_1.MCR.B.GTBE = 1;

		Pwm_ModuleState = PWM_STATE_INITIALIZED;

		for (channel_iterator = 0; channel_iterator < PWM_NUMBER_OF_CHANNELS; channel_iterator++)
		{
			Pwm_ChannelType channel = ConfigPtr->Channels[channel_iterator].channel;

			// Set up the registers in hw

			if(channel <= PWM_NUMBER_OF_EACH_EMIOS-1)
			{
				memcpy((void*) &EMIOS_0.CH[channel],
							(void*) &ConfigPtr->Channels[channel_iterator].r,
							sizeof(Pwm_ChannelRegisterType));
			}
			else
			{
				memcpy((void*) &EMIOS_1.CH[channel-PWM_NUMBER_OF_EACH_EMIOS],
										(void*) &ConfigPtr->Channels[channel_iterator].r,
										sizeof(Pwm_ChannelRegisterType));
			}

			ChannelRuntimeStruct[channel].Class = ConfigPtr->ChannelClass[channel_iterator];

        	#if PWM_NOTIFICATION_SUPPORTED==STD_ON
                /*
                 * PWM052: The function Pwm_Init shall disable all notifications.
                 *
                 * This is now implemented in the configuration macro.
                 */
                // Install ISR
			if(channel <= PWM_NUMBER_OF_EACH_EMIOS-1)
			{
				 Irq_AttachIsr2(tid, NULL, EMIOS_0_GFR_F16_F17 + (channel-16)/2);
			}
			else
			{
				 Irq_AttachIsr2(tid, NULL, EMIOS_1_GFR_F16_F17 + (channel-40)/2 );
			}

                ChannelRuntimeStruct[channel].NotificationRoutine
                        = ConfigPtr->NotificationHandlers[channel_iterator];
        	#endif
		}
	#else

    /* Clock scaler uses system clock (~64MHz) as source, so prescaler 64 => 1MHz. */
    EMIOS.MCR.B.GPRE = PWM_PRESCALER - 1;

    /* Enable eMIOS clock */
    EMIOS.MCR.B.GPREN = 1;

    /* Stop channels when in debug mode */
    EMIOS.MCR.B.FRZ = PWM_FREEZE_ENABLE;

    /* Use global time base */
    EMIOS.MCR.B.GTBE = 1;

    Pwm_ModuleState = PWM_STATE_INITIALIZED;

    for (channel_iterator = 0; channel_iterator < PWM_NUMBER_OF_CHANNELS; channel_iterator++) {
      const Pwm_ChannelConfigurationType* channelConfig = &ConfigPtr->Channels[channel_iterator];
      Pwm_ChannelType channel = channelConfig->channel;

        EMIOS.CH[channel].CCR.B.MODE = PWM_EMIOS_OPWM;
        EMIOS.CH[channel].CCR.B.DMA = 0;
        EMIOS.CH[channel].CCR.B.BSL = 3;
        EMIOS.CH[channel].CCR.B.ODIS = 0;

        Pwm_ChannelPrescalerType prescaler;  uint16_t period_ticks;
        calcPeriodTicksAndPrescaler( channelConfig, &period_ticks, &prescaler );

        EMIOS.CH[channel].CBDR.R = period_ticks;
        EMIOS.CH[channel].CCR.B.UCPRE = prescaler;
        EMIOS.CH[channel].CCR.B.UCPREN = 1;

        // 0 A match on comparator A clears the output flip-flop, while a match on comparator B sets it
        // 1 A match on comparator A sets the output flip-flop, while a match on comparator B clears it
        // A duty cycle of X % should give a signal with state 'channelConfig->polarity' during
        // X % of the period time.
        EMIOS.CH[channel].CCR.B.EDPOL = (channelConfig->polarity == PWM_LOW) ? 1 : 0;

        #if PWM_NOTIFICATION_SUPPORTED==STD_ON
                /*
                 * PWM052: The function Pwm_Init shall disable all notifications.
                 *
                 * This is now implemented in the configuration macro.
                 */
                // Pwm_DisableNotification(channel);

                // Install ISR
                switch (channel) {
					case 0x0: ISR_INSTALL_ISR2( "Pwm0",  Pwm_Isr, EMISOS200_FLAG_F0,  PWM_ISR_PRIORITY, 0 ); break;
					case 0x1: ISR_INSTALL_ISR2( "Pwm1",  Pwm_Isr, EMISOS200_FLAG_F1,  PWM_ISR_PRIORITY, 0 ); break;
					case 0x2: ISR_INSTALL_ISR2( "Pwm2",  Pwm_Isr, EMISOS200_FLAG_F2,  PWM_ISR_PRIORITY, 0 ); break;
					case 0x3: ISR_INSTALL_ISR2( "Pwm3",  Pwm_Isr, EMISOS200_FLAG_F3,  PWM_ISR_PRIORITY, 0 ); break;
					case 0x4: ISR_INSTALL_ISR2( "Pwm4",  Pwm_Isr, EMISOS200_FLAG_F4,  PWM_ISR_PRIORITY, 0 ); break;
					case 0x5: ISR_INSTALL_ISR2( "Pwm5",  Pwm_Isr, EMISOS200_FLAG_F5,  PWM_ISR_PRIORITY, 0 ); break;
					case 0x6: ISR_INSTALL_ISR2( "Pwm6",  Pwm_Isr, EMISOS200_FLAG_F6,  PWM_ISR_PRIORITY, 0 ); break;
					case 0x7: ISR_INSTALL_ISR2( "Pwm7",  Pwm_Isr, EMISOS200_FLAG_F7,  PWM_ISR_PRIORITY, 0 ); break;
					case 0x8: ISR_INSTALL_ISR2( "Pwm8",  Pwm_Isr, EMISOS200_FLAG_F8,  PWM_ISR_PRIORITY, 0 ); break;
					case 0x9: ISR_INSTALL_ISR2( "Pwm9",  Pwm_Isr, EMISOS200_FLAG_F9,  PWM_ISR_PRIORITY, 0 ); break;
					case 0xA: ISR_INSTALL_ISR2( "Pwm10", Pwm_Isr, EMISOS200_FLAG_F10, PWM_ISR_PRIORITY, 0 ); break;
					case 0xB: ISR_INSTALL_ISR2( "Pwm11", Pwm_Isr, EMISOS200_FLAG_F11, PWM_ISR_PRIORITY, 0 ); break;
					case 0xC: ISR_INSTALL_ISR2( "Pwm12", Pwm_Isr, EMISOS200_FLAG_F12, PWM_ISR_PRIORITY, 0 ); break;
					case 0xD: ISR_INSTALL_ISR2( "Pwm13", Pwm_Isr, EMISOS200_FLAG_F13, PWM_ISR_PRIORITY, 0 ); break;
					case 0xE: ISR_INSTALL_ISR2( "Pwm14", Pwm_Isr, EMISOS200_FLAG_F14, PWM_ISR_PRIORITY, 0 ); break;
					case 0xF: ISR_INSTALL_ISR2( "Pwm15", Pwm_Isr, EMISOS200_FLAG_F15, PWM_ISR_PRIORITY, 0 ); break;
					default: assert(0); break;
				}

                ChannelRuntimeStruct[channel].NotificationRoutine
                        = ConfigPtr->NotificationHandlers[channel_iterator];
        #endif
    }
	#endif
}

#if PWM_DEINIT_API==STD_ON

// TODO: Test that this function in fact turns the channel off.
void inline Pwm_DeInitChannel(Pwm_ChannelType Channel) {
	Pwm_VALIDATE_CHANNEL(Channel);
	Pwm_VALIDATE_INITIALIZED();

    Pwm_SetOutputToIdle(Channel);

    #ifdef CFG_MPC5516
        // Set the disable bit for this channel
        EMIOS.UCDIS.R |= (1 << (31 - Channel));

    #elif defined(CFG_MPC5606S)
        // Set the disable bit for this channel
        if(Channel <= PWM_NUMBER_OF_EACH_EMIOS-1)
        {
        	EMIOS_0.UCDIS.R |= (1 << (Channel));
        }
        else
        {
        	EMIOS_1.UCDIS.R |= (1 << (Channel-PWM_NUMBER_OF_EACH_EMIOS));
        }

	#endif

    /*
     * PWM052: The function Pwm_DeInit shall disable all notifications.
     */
    #if PWM_NOTIFICATION_SUPPORTED==STD_ON
        Pwm_DisableNotification(Channel);
    #endif
}

void Pwm_DeInit() {
	/* TODO: Implement Pwm_DeInit() */
	Pwm_ChannelType channel_iterator;

	Pwm_VALIDATE_INITIALIZED();

	for (channel_iterator = 0; channel_iterator < PWM_NUMBER_OF_CHANNELS; channel_iterator++) {
		Pwm_ChannelType channel = PwmConfigPtr->Channels[channel_iterator].channel;
		Pwm_DeInitChannel(channel);

	}

	// Disable module

	#ifdef CFG_MPC5516

		EMIOS.MCR.B.MDIS = 1;

    #elif defined(CFG_MPC5606S)

		EMIOS_0.MCR.B.MDIS = 1;
		EMIOS_1.MCR.B.MDIS = 1;

	#endif

	Pwm_ModuleState = PWM_STATE_UNINITIALIZED;
}
#endif


/*
 * PWM083: The function Pwm_SetPeriodAndDuty shall be pre compile time
 * changeable ON/OFF by the configuration parameter PwmSetPeriodAndDuty.
 */
#if PWM_SET_PERIOD_AND_DUTY==STD_ON
	void Pwm_SetPeriodAndDuty(Pwm_ChannelType Channel, Pwm_PeriodType Period,
			Pwm_DutyCycleType DutyCycle) {

		Pwm_VALIDATE_INITIALIZED();
		Pwm_VALIDATE_CHANNEL(Channel);
		PWM_VALIDATE(ChannelRuntimeStruct[Channel].Class == PWM_VARIABLE_PERIOD, PWM_E_PERIOD_UNCHANGEABLE);

		uint16 leading_edge_position = (uint16) (((uint32) Period
				* (uint32) DutyCycle) >> 15);

		#ifdef CFG_MPC5516

			/* Timer instant for leading edge */
			EMIOS.CH[Channel].CADR.R = leading_edge_position;

			/* Timer instant for the period to restart */
			EMIOS.CH[Channel].CBDR.R = Period;

		#elif defined(CFG_MPC5606S)

			if(Channel <= PWM_NUMBER_OF_EACH_EMIOS-1)
			{
				EMIOS_0.CH[Channel].CADR.R = leading_edge_position;
				EMIOS_0.CH[Channel].CBDR.R = Period;
			}
			else
			{
			 EMIOS_1.CH[Channel - PWM_NUMBER_OF_EACH_EMIOS].CADR.R = leading_edge_position;
			 EMIOS_1.CH[Channel - PWM_NUMBER_OF_EACH_EMIOS].CBDR.R = Period;
			}

		#endif
	}
#endif


/**
 * PWM013: The function Pwm_SetDutyCycle shall set the duty cycle of the PWM
 * channel.
 *
 * @TODO: How to conform with PWM018: "The driver shall forbid the spike on the PWM output signal"?
 *
 * @param Channel PWM channel to use. 0 <= Channel < PWM_NUMBER_OF_CHANNELS <= 16
 * @param DutyCycle 0 <= DutyCycle <= 0x8000
 */
#if PWM_SET_DUTYCYCLE==STD_ON

void Pwm_SetDutyCycle(Pwm_ChannelType Channel, Pwm_DutyCycleType DutyCycle)
{
	Pwm_VALIDATE_INITIALIZED();
	Pwm_VALIDATE_CHANNEL(Channel);

	#ifdef CFG_MPC5516

		uint16 leading_edge_position = (uint16) ((EMIOS.CH[Channel].CBDR.R
					* (uint32) DutyCycle) >> 15);


		Pwm_VALIDATE_INITIALIZED();
		Pwm_VALIDATE_CHANNEL(Channel);

		/* Timer instant for leading edge */

		/*
		 * PWM017: The function Pwm_SetDutyCycle shall update the duty cycle at
		 * the end of the period if supported by the implementation and configured
		 * with PwmDutycycleUpdatedEndperiod. [ This is achieved in hardware since
		 * the A and B registers are double buffered ]
		 *
		 * PWM014: The function Pwm_SetDutyCycle shall set the output state according
		 * to the configured polarity parameter [which is already set from
		 * Pwm_InitChannel], when the duty parameter is 0% [=0] or 100% [=0x8000].
		 */
		EMIOS.CH[Channel].CADR.R = leading_edge_position;

	#elif defined(CFG_MPC5606S)

		if(Channel <= PWM_NUMBER_OF_EACH_EMIOS-1)
		{
			uint16 leading_edge_position = (uint16) ((EMIOS_0.CH[Channel].CBDR.R* (uint32) DutyCycle) >> 15);

			EMIOS_0.CH[Channel].CADR.R = leading_edge_position;

		}
		else
		{
			uint16 leading_edge_position = (uint16) ((EMIOS_1.CH[Channel-PWM_NUMBER_OF_EACH_EMIOS].CBDR.R* (uint32) DutyCycle) >> 15);

			EMIOS_1.CH[Channel-PWM_NUMBER_OF_EACH_EMIOS].CADR.R = leading_edge_position;

		}
	#endif
}
#endif

#if  PWM_SET_OUTPUT_TO_IDLE == STD_ON
	void Pwm_SetOutputToIdle(Pwm_ChannelType Channel)
	{
		Pwm_VALIDATE_CHANNEL(Channel);
		Pwm_VALIDATE_INITIALIZED();

		/* TODO: Make Pwm_SetOutputToIdle sensitive to PwmIdleState (currently uses PwmPolarity) */

		#ifdef CFG_MPC5516

			EMIOS.CH[Channel].CADR.R = 0;

		#elif defined(CFG_MPC5606S)


			for (Pwm_ChannelType channel_iterator = 0; channel_iterator < PWM_NUMBER_OF_CHANNELS; channel_iterator++)
			{
				if(Channel == PwmConfigPtr->Channels[channel_iterator].channel)
				{
					Pwm_OutputStateType ChannelPol = PwmConfigPtr->Channels[channel_iterator].r.edgePolarity;
					if(ChannelPol == PwmConfigPtr->IdleState[channel_iterator])
					{
						Pwm_SetDutyCycle(Channel,0x0000);
					}
					else
					{
						Pwm_SetDutyCycle(Channel,0x8000);
					}
				}

			}

		#endif

}
#endif
/*
 * PWM085: The function Pwm_GetOutputState shall be pre compile configurable
 * ON/OFF by the configuration parameter PwmGetOutputState
 */
#if PWM_GET_OUTPUT_STATE==STD_ON
	/*
	 * PWM022: The function Pwm_GetOutputState shall read the internal state
	 * of the PWM output signal and return it.
	 */
	Pwm_OutputStateType Pwm_GetOutputState(Pwm_ChannelType Channel)
	{
		// We need to return something, even in presence of errors
		if ( ! PWM_VALID_CHANNEL(Channel) )
		{
			Pwm_ReportError(PWM_E_PARAM_CHANNEL);

			/*
			 * Accordingly to PWM025, we should return PWM_LOW on failure.
			 */
			return PWM_LOW;

		}
		else if (Pwm_ModuleState != PWM_STATE_INITIALIZED)
		{
			Pwm_ReportError(PWM_E_UNINIT);
			/*
			 * Accordingly to PWM025, we should return PWM_LOW on failure.
			 */
			return PWM_LOW;
		}

		#ifdef CFG_MPC5516

			return EMIOS.CH[Channel].CSR.B.UCOUT;

		#elif defined(CFG_MPC5606S)

			if(Channel <= PWM_NUMBER_OF_EACH_EMIOS-1)
			{
				return EMIOS_0.CH[Channel].CSR.B.UCOUT;
			}
			else
			{
				return EMIOS_1.CH[Channel-PWM_NUMBER_OF_EACH_EMIOS].CSR.B.UCOUT;
			}
		#endif

	}
#endif

#if PWM_NOTIFICATION_SUPPORTED==STD_ON

	void Pwm_DisableNotification(Pwm_ChannelType Channel)
	{
		Pwm_VALIDATE_CHANNEL(Channel);
		Pwm_VALIDATE_INITIALIZED();

		// Disable flags on this channel
		#if defined(CFG_MPC5516)

			EMIOS.CH[Channel].CCR.B.FEN = 0;

		#elif defined(CFG_MPC5606S)

			if(Channel <= PWM_NUMBER_OF_EACH_EMIOS-1)
			{
				EMIOS_0.CH[Channel].CCR.B.FEN = 0;
			}
			else
			{
				EMIOS_1.CH[Channel-PWM_NUMBER_OF_EACH_EMIOS].CCR.B.FEN = 0;
			}
		#endif

	}

	void Pwm_EnableNotification(Pwm_ChannelType Channel,Pwm_EdgeNotificationType Notification)
	{
		Pwm_VALIDATE_CHANNEL(Channel);
		Pwm_VALIDATE_INITIALIZED();

		ChannelRuntimeStruct[Channel].NotificationState = Notification;

		// Enable flags on this channel
		#if defined(CFG_MPC5516)

			EMIOS.CH[Channel].CCR.B.FEN = 1;

		#elif defined(CFG_MPC5606S)

			if(Channel <= PWM_NUMBER_OF_EACH_EMIOS-1)
			{
				EMIOS_0.CH[Channel].CCR.B.FEN = 1;
			}
			else
			{
				EMIOS_1.CH[Channel-PWM_NUMBER_OF_EACH_EMIOS].CCR.B.FEN = 1;
			}
		#endif
	}

	static void Pwm_Isr(void)
	{
		// Find out which channel that triggered the interrupt
		#if defined(CFG_MPC5516) || defined(CFG_MPC5567)
			#if defined(CFG_MPC5516)
				uint32_t flagmask = EMIOS.GFLAG.R;
			#elif defined(CFG_MPC5567)
				uint32_t flagmask = EMIOS.GFR.R;
			#endif

			// There are 24 channels specified in the global flag register, but
			// we only listen to the first 16 as only these support OPWfM
			for (Pwm_ChannelType emios_ch = 0; emios_ch < 16; emios_ch++) {
				if (flagmask & (1 << emios_ch)) {

					if (ChannelRuntimeStruct[emios_ch].NotificationRoutine != NULL && EMIOS.CH[emios_ch].CCR.B.FEN) {

						Pwm_EdgeNotificationType notification = ChannelRuntimeStruct[emios_ch].NotificationState;
						if (notification == PWM_BOTH_EDGES || notification == EMIOS.CH[emios_ch].CSR.B.UCOUT)
						{
								ChannelRuntimeStruct[emios_ch].NotificationRoutine();
						}
					}
					// Clear interrupt
					EMIOS.CH[emios_ch].CSR.B.FLAG = 1;
				}
			}
        #elif defined(CFG_MPC5606S)
			uint32_t flagmask_0 = EMIOS_0.GFR.R;
			uint32_t flagmask_1 = EMIOS_1.GFR.R;

			for (Pwm_ChannelType channel_iterator = 0; channel_iterator < 16; channel_iterator++)
			{

				Pwm_ChannelType emios_ch = PwmConfigPtr->Channels[channel_iterator].channel;

				if (flagmask_0 & (1 << emios_ch))
				{
					if (ChannelRuntimeStruct[channel_iterator].NotificationRoutine != NULL && EMIOS_0.CH[emios_ch].CCR.B.FEN)
					{
						Pwm_EdgeNotificationType notification = ChannelRuntimeStruct[channel_iterator].NotificationState;
						if (notification == PWM_BOTH_EDGES || notification == EMIOS_0.CH[emios_ch].CSR.B.UCOUT)
						{
							ChannelRuntimeStruct[channel_iterator].NotificationRoutine();
						}
					}

					// Clear interrupt
					EMIOS_0.CH[emios_ch].CSR.B.FLAG = 1;
				}
				else if (flagmask_1 & (1 << emios_ch - 24 ))
				{
					if (ChannelRuntimeStruct[channel_iterator].NotificationRoutine != NULL && EMIOS_1.CH[emios_ch - 24].CCR.B.FEN)
					{
						Pwm_EdgeNotificationType notification = ChannelRuntimeStruct[channel_iterator].NotificationState;
						if (notification == PWM_BOTH_EDGES ||
								notification == EMIOS_1.CH[emios_ch - 24].CSR.B.UCOUT)
							{
								ChannelRuntimeStruct[channel_iterator].NotificationRoutine();
							}
						}

					// Clear interrupt
					EMIOS_1.CH[emios_ch - 24].CSR.B.FLAG = 1;
				}
			}

		#endif
	}

#endif /* PWM_NOTIFICATION_SUPPORED == STD_ON */
